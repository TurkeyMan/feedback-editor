#include "FeedBack.h"
#include "ModelProp.h"
#include "Action.h"
#include "Theme.h"
#include "Tools/ResourceCache.h"

void dBModelProp::RegisterEntity()
{
	dBFactoryType *pType = dBEntityManager::RegisterEntityType("Model", Create, "Entity");

	dBActionManager::RegisterInstantAction("setmodel", SetModel, pType);
}

dBModelProp::dBModelProp()
{
	pModel = NULL;
}

dBModelProp::~dBModelProp()
{
	if(pModel)
		MFModel_Release(pModel);
}

void dBModelProp::Update()
{
	dBEntity::Update();
}

void dBModelProp::Draw()
{
	MFModel_SetWorldMatrix(pModel, GetMatrix());
	MFRenderer_AddModel(pModel, nullptr, MFView_GetViewState());

	dBEntity::Draw();
}

const char *dBModelProp::GetProperty(const char *pPropertyName)
{
	//...

	return dBEntity::GetProperty(pPropertyName);
}

void dBModelProp::SetModel(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	dBModelProp *pModelProp = (dBModelProp*)pEntity;
	dBResourceCache *pResourceCache = pEntity->GetManager()->GetScreen()->GetResourceCache();
	pModelProp->pModel = pResourceCache->FindModel(pArguments->GetString(0).CStr());
}
