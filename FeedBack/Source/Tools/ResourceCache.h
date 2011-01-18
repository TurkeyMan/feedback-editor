#if !defined(_DB_RESOURCE_CACHE)
#define _DB_RESOURCE_CACHE

#include "HashTable.h"

struct MFMaterial;
struct MFSound;
struct MFModel;
struct MFFont;

class dBResourceCache
{
public:
	void Init();
	void Deinit();

	void LoadMaterial(const char *pName, const char *pFilename);
	void LoadSound(const char *pName, const char *pFilename);
	void LoadModel(const char *pName, const char *pFilename);
	void LoadFont(const char *pName, const char *pFilename);

	MFMaterial *FindMaterial(const char *pName);
	MFSound *FindSound(const char *pName);
	MFModel *FindModel(const char *pName);
	MFFont *FindFont(const char *pName);

protected:
	dBHashList<MFMaterial> materialList;
	dBHashList<MFSound> soundList;
	dBHashList<MFModel> modelList;
	dBHashList<MFFont> fontList;
};

#endif
