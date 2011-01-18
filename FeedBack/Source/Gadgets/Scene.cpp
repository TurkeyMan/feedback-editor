#include "GHEd.h"
#include "Scene.h"
#include "Util/F3D.h"

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
		(*ppP)->Destroy();
		++ppP;
	}

	entities.Deinit();

	delete this;
}

void dBScene::AddProp(dBProp *pProp)
{
	entities.Create(pProp);
}

dBEntity *dBScene::FindProp(const char *pName)
{
	dBEntity **ppI = entities.Begin();

	while(*ppI)
	{
		if(!MFString_CaseCmp((*ppI)->name, pName))
			return *ppI;
		++ppI;
	}

	return NULL;
}

void dBScene::RemoveProp(dBProp *pProp)
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

