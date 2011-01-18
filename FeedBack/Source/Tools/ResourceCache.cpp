#include "Fuji.h"
#include "MFMaterial.h"
#include "MFSound.h"
#include "MFModel.h"
#include "MFFont.h"
#include "HashTable.h"
#include "ResourceCache.h"

void dBResourceCache::Init()
{
	materialList.Create(256, 256, 64);
	soundList.Create(256, 256, 64);
	modelList.Create(256, 256, 64);
	fontList.Create(256, 256, 64);
}

void dBResourceCache::Deinit()
{
	// free the resources

	materialList.Destroy();
	soundList.Destroy();
	modelList.Destroy();
	fontList.Destroy();
}

void dBResourceCache::LoadMaterial(const char *pName, const char *pFilename)
{
	MFMaterial *pMaterial = MFMaterial_Create(pFilename);
	if(pMaterial)
		materialList.Add(pMaterial, pName);
}

void dBResourceCache::LoadSound(const char *pName, const char *pFilename)
{
	MFSound *pSound = MFSound_Create(pFilename);
	if(pSound)
		soundList.Add(pSound, pName);
}

void dBResourceCache::LoadModel(const char *pName, const char *pFilename)
{
	MFModel *pModel = MFModel_Create(pFilename);
	if(pModel)
		modelList.Add(pModel, pName);
}

void dBResourceCache::LoadFont(const char *pName, const char *pFilename)
{
	MFFont *pFont = MFFont_Create(pFilename);
	if(pFont)
		fontList.Add(pFont, pName);
}

MFMaterial *dBResourceCache::FindMaterial(const char *pName)
{
	return materialList.Find(pName);
}

MFSound *dBResourceCache::FindSound(const char *pName)
{
	return soundList.Find(pName);
}

MFModel *dBResourceCache::FindModel(const char *pName)
{
	return modelList.Find(pName);
}

MFFont *dBResourceCache::FindFont(const char *pName)
{
	return fontList.Find(pName);
}
