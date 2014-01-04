#include "FeedBack.h"
#include "Scene.h"
#include "Fuji/Util/F3D.h"

#include "ModelProp.h"

dBScene *dBScene::Create(const char *pSceneFile)
{
	dBScene *pScene = new dBScene;

	return pScene;
}

void dBScene::Destroy()
{
	dBEntity **ppP = entities.Begin();
	while(*ppP)
	{
		delete *ppP;
		++ppP;
	}

	entities.Deinit();

	delete this;
}

void dBScene::AddEntity(dBEntity *pProp, int layer)
{
	entities.Create(pProp);
}

dBEntity *dBScene::FindEntity(const char *pName, int layer)
{
	dBEntity **ppI = entities.Begin();

	while(*ppI)
	{
		if(!MFString_CaseCmp((*ppI)->GetName(), pName))
			return *ppI;
		++ppI;
	}

	return NULL;
}

void dBScene::RemoveEntity(dBEntity *pProp, int layer)
{
	entities.Destroy(pProp);
}

void dBScene::Update()
{
	dBEntity **ppI = entities.Begin();

	while(*ppI)
	{
		(*ppI)->Update();
		++ppI;
	}
}

void dBScene::Draw()
{
	dBEntity **ppI = entities.Begin();

	while(*ppI)
	{
		(*ppI)->Draw();
		++ppI;
	}
}

