#if !defined(_SCENE_H)
#define _SCENE_H

#include "MFPtrList.h"
#include "Entity.h"

class dBScene
{
public:
	static dBScene *Create(const char *pSceneFile);
	void Destroy();

	void AddEntity(dBEntity *pProp, int layer);
	dBEntity *FindEntity(const char *pName, int layer = -1);
	void RemoveEntity(dBEntity *pProp, int layer = -1);

	void Update();
	void Draw();

	int layerCount;
	MFPtrList<dBEntity> entities;
};

#endif
