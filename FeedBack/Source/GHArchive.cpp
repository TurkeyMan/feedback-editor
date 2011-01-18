#include "GHEd.h"
#include "MFFileSystem.h"
#include "MFHeap.h"
#include "MFTexture.h"

#if !defined(MAKE_LEGAL)

#include "GHArchive.h"
#include <stdio.h>

char *LoadFile(const char *pFilename, int *pSize)
{
	char *pBuffer = NULL;

	FILE *pFile = fopen(pFilename, "rb");

	if(pFile)
	{
		fseek(pFile, 0, SEEK_END);
		int size = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		pBuffer = (char*)malloc(size);
		fread(pBuffer, 1, size, pFile);
		
		fclose(pFile);

		if(pSize)
			*pSize = size;
	}

	return pBuffer;
}

const char *FindGHDVD()
{
	return "F:\\";
}

GHArchive *LoadGHArchive(const char *pHeaderFilename)
{
	char *pBuffer = LoadFile(pHeaderFilename, NULL);

	if(!pBuffer)
		return NULL;

	char ark[256];
	strcpy(ark, pHeaderFilename);
	ark[strlen(ark)-4] = 0;
	strcat(ark, "_0.ARK");

	HANDLE hArk = CreateFile(ark, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hArk == INVALID_HANDLE_VALUE)
	{
		MFHeap_Free(pBuffer);
		return NULL;
	}

	GHArchive *pArchive = (GHArchive*)MFHeap_Alloc(sizeof(GHArchive));

	pArchive->pHeader = (ARKHeader*)pBuffer;
	pArchive->pStrings = (ARKStringCache*)(pBuffer + sizeof(ARKHeader) + pArchive->pHeader->stringCacheLength);
	pArchive->pFiles = (ARKDataCache*)((char*)pArchive->pStrings + sizeof(int)*(pArchive->pStrings->numStrings + 1));
	pArchive->hArk = hArk;

	for(int a=0; a<pArchive->pStrings->numStrings; a++)
	{
		if(pArchive->pStrings->pStrings[a])
			pArchive->pStrings->pStrings[a] += (size_t)(pBuffer + sizeof(ARKHeader));
	}

	for(int a=0; a<pArchive->pFiles->numFiles; a++)
	{
		pArchive->pFiles->files[a].pFilename = pArchive->pStrings->pStrings[(int&)pArchive->pFiles->files[a].pFilename];
		pArchive->pFiles->files[a].pDirectory = pArchive->pStrings->pStrings[(int&)pArchive->pFiles->files[a].pDirectory];
	}

	return pArchive;
}

void FreeGHArchive(GHArchive *pArchive)
{
	CloseHandle(pArchive->hArk);
	MFHeap_Free(pArchive->pHeader);
}

char *LoadFileFromGHArchive(GHArchive *pArchive, const char *pFilename, uint32 *pSize, bool path)
{
	// find file
	int file = 0;
	for(; file<pArchive->pFiles->numFiles; file++)
	{
		const char *pArkFile;
		if(path)
			pArkFile = MFStr("%s/%s", pArchive->pFiles->files[file].pDirectory, pArchive->pFiles->files[file].pFilename);
		else
			pArkFile = pArchive->pFiles->files[file].pFilename;


		// maybe we should force that all '\' characters are converted to foreward slashes

		if(!MFString_CaseCmp(pFilename, pArkFile))
			break;
	}

	if(file == pArchive->pFiles->numFiles)
		return NULL;

	// extract the file
	char *pFile = (char*)MFHeap_Alloc(pArchive->pFiles->files[file].fileSize);
	if(pSize)
		*pSize = pArchive->pFiles->files[file].fileSize;

	DWORD read;
	DWORD fp = SetFilePointer(pArchive->hArk, pArchive->pFiles->files[file].arkOffset, NULL, FILE_BEGIN);
	BOOL rd = ReadFile(pArchive->hArk, pFile, pArchive->pFiles->files[file].fileSize, &read, NULL);

	MFDebug_Assert(read == pArchive->pFiles->files[file].fileSize, MFStr("Failed reading file '%s' from the ark", pFilename));

	return pFile;
}

MFMaterial *LoadMaterialFromGHArchive(GHArchive *pArchive, const char *pFilename)
{
	char *pImage = LoadFileFromGHArchive(pArchive, pFilename);

	if(!pImage)
		return NULL;

	TEXHeader *pTex = (TEXHeader*)pImage;

	char *pImageBuffer = (char*)MFHeap_Alloc(pTex->width * pTex->height * 4);
	uint32 *pPixel = (uint32*)pImageBuffer;

	// convert image
	unsigned char *pImageData = (unsigned char*)&pTex->colourTable[256];

	for(int a=0; a<pTex->height; a++)
	{
		for(int b=0; b<pTex->width; b++)
		{
			// capculate the pixel
			int pixel = a*pTex->width + b;

			// get the colour index
			int index = pImageData[pixel];

			// swizzle the colour id (because the PS2 palette is swizzled)
			index = (index & 0xE7) | ((index & 0x08) << 1) | ((index & 0x10) >> 1);

			// and get the colour
			TEXColour *pCol = &pTex->colourTable[index];

			// write the pixel into our image
			*pPixel = (((unsigned int)pCol->r) << 16) | (((unsigned int)pCol->g) << 8) | ((unsigned int)pCol->b) | ((unsigned int)(((float)pCol->a)*(1.0f/128.0f) * 255.0f) << 24);
			++pPixel;
		}
	}

	// create the material
    MFTexture *pTexture = MFTexture_CreateFromRawData(pFilename, pImageBuffer, pTex->width, pTex->height, TexFmt_A8R8G8B8, TEX_CopyMemory, true);
	MFMaterial *pMat = MFMaterial_Create(pFilename);
	MFTexture_Destroy(pTexture);
	MFHeap_Free(pImageBuffer);

	return pMat;
}

#endif
