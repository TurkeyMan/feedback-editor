#if !defined(_GHARCHIVE_H)
#define _GHARCHIVE_H

#include "MFMaterial.h"

// GH Ark file structures
struct ARKHeader
{
	int unknown, unknown2, unknown3;
	unsigned int arkSize;
	int stringCacheLength;
};

struct ARKStringCache
{
	int numStrings;
	char *pStrings[1];
};

struct ARKEntry
{
	unsigned int arkOffset;
	char *pFilename;
	char *pDirectory;
	unsigned int fileSize;
	int unknown;
};

struct ARKDataCache
{
	int numFiles;
	ARKEntry files[1];
};

// and the GH texture
struct TEXColour
{
	unsigned char r, g, b, a;
};

#pragma pack(1)
struct TEXHeader
{
	char magic[3];
	int something;
	short width, height;
	short numColours;

	char res[19];

	TEXColour colourTable[256];
};
#pragma pack()

// GH Archive struct
struct GHArchive
{
	ARKHeader *pHeader;
	ARKStringCache *pStrings;
	ARKDataCache *pFiles;

	HANDLE hArk;
};

// GH archive related functions
const char *FindGHDVD();

GHArchive *LoadGHArchive(const char *pHeaderFilename);
void FreeGHArchive(GHArchive *pArchive);

char *LoadFileFromGHArchive(GHArchive *pArchive, const char *pFilename, uint32 *pSize = NULL, bool path = true);

// this will allow to load textures from the Guitar Hero DVD
MFMaterial *LoadMaterialFromGHArchive(GHArchive *pArchive, const char *pFilename);

#endif
