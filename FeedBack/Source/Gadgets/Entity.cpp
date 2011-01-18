#include "FeedBack.h"
#include "Entity.h"
#include "Action.h"
#include "Theme.h"

#include "ImageProp.h"
#include "ModelProp.h"
#include "TextProp.h"

dBFactory<dBEntity> dBEntityManager::entityFactory;

void dBEntity::RegisterEntity()
{
	dBFactoryType *pType = dBEntityManager::RegisterEntityType("Entity", NULL);

	dBActionManager::RegisterInstantAction("setposition", SetPos, pType);
	dBActionManager::RegisterInstantAction("setrotation", SetRot, pType);
	dBActionManager::RegisterInstantAction("setsize", SetSize, pType);
	dBActionManager::RegisterInstantAction("setcolour", SetColour, pType);
	dBActionManager::RegisterInstantAction("enable", SetEnable, pType);
	dBActionManager::RegisterInstantAction("show", SetVisible, pType);
}

dBEntity::dBEntity()
{
	name[0] = 0;
	name[sizeof(name)-1] = 0;
	pos = startPos = MFVector::zero;
	rot = startRot = MFVector::zero;
	scale = startScale = MFVector::one;
	bEnabled = true;

	pManager = NULL;
	pActionManager = NULL;
}

dBEntity::~dBEntity()
{
}

void dBEntity::Init(dBActionManager *_pActionManager, MFIniLine *pEntityData)
{
	pActionManager = _pActionManager;
	while(pEntityData)
	{
		if(pEntityData->IsSection("Events"))
		{
			MFIniLine *pEvents = pEntityData->Sub();
			while(pEvents)
			{
				dBEvent *pEvent = &events.push();
				pEvent->pEvent = pEvents->GetString(0);
				pEvent->pScript = _pActionManager->ParseScript(pEvents->GetString(pEvents->GetStringCount()-1));

				pEvents = pEvents->Next();
			}
		}

		pEntityData = pEntityData->Next();
	}
}

void dBEntity::Update()
{
}

void dBEntity::Draw()
{
}

bool dBEntity::IsType(dBFactoryType *pExpectedType)
{
	dBFactoryType *pT = pType;
	while(pT)
	{
		if(pT == pExpectedType)
			return true;
		pT = pT->pParent;
	}
	return false;
}

const char *dBEntity::GetProperty(const char *pPropertyName)
{
	if(!MFString_CaseCmp(pPropertyName, "position"))
		return MFStr("%g, %g, %g", pos.x, pos.y, pos.z);
	else if(!MFString_CaseCmp(pPropertyName, "rotation"))
		return MFStr("%g, %g, %g", rot.x, rot.y, rot.z);
	else if(!MFString_CaseCmp(pPropertyName, "scale"))
		return MFStr("%g, %g, %g", scale.x, scale.y, scale.z);
	else if(!MFString_CaseCmp(pPropertyName, "name"))
		return name;
	return NULL;
}

void dBEntity::SignalEvent(const char *pEvent, const char *pParams)
{
	for(int a=0; a<events.size(); ++a)
	{
		if(!MFString_CaseCmp(pEvent, events[a].pEvent))
			pActionManager->RunScript(events[a].pScript, this);
	}
}

MFMatrix dBEntity::GetMatrix()
{
	MFMatrix mat;
	mat.SetRotationYPR(rot.y, rot.x, rot.z);
	mat.SetXAxis4(mat.GetXAxis() * scale.x);
	mat.SetYAxis4(mat.GetYAxis() * scale.y);
	mat.SetZAxis4(mat.GetZAxis() * scale.z);
	mat.SetTrans3(pos);
	return mat;
}


void dBEntityManager::InitManager()
{
	dBEntity::RegisterEntity();
	dBImageProp::RegisterEntity();
	dBModelProp::RegisterEntity();
	dBTextProp::RegisterEntity();
}

void dBEntityManager::DeinitManager()
{
}

dBFactoryType *dBEntityManager::RegisterEntityType(const char *pEntityTypeName, dBFactory_CreateFunc *pCreateFunc, const char *pParentType)
{
	return entityFactory.RegisterType(pEntityTypeName, pCreateFunc, entityFactory.FindType(pParentType));
}

void dBEntityManager::Init(dBThemeScreen *_pThemeScreen)
{
	pThemeScreen = _pThemeScreen;

	entityPool.Create(256, 2048, 256);
}

void dBEntityManager::Deinit()
{
	entityPool.Destroy();
}

dBEntity *dBEntityManager::Create(const char *pTypeName, const char *pName)
{
	dBFactoryType *pType;
	dBEntity *pEntity = entityFactory.Create(pTypeName, &pType);
	if(pEntity)
	{
		// assign its type
		pEntity->pType = pType;

		// give it a name
		MFString_Copy(pEntity->name, pName);

		// add it to the entity registry
		entityPool.Add(pEntity, pName);
		pEntity->pManager = this;
	}
	return pEntity;
}

dBEntity *dBEntityManager::Find(const char *pName)
{
	return entityPool.Find(pName);
}

dBEntity *dBEntityManager::Iterate(dBEntity *pLast)
{
	return entityPool.Next(pLast);
}

void dBEntityManager::Destroy(dBEntity *pEntity)
{
	// remove any actions from the action list that belong to this entity

	// destroy it
	entityPool.Remove(pEntity);
	delete pEntity;
}

void dBEntity::SetPos(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	pEntity->pos = pArguments->GetVector(0);
}

void dBEntity::SetRot(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	pEntity->rot = pArguments->GetVector(0);
}

void dBEntity::SetSize(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	pEntity->scale = pArguments->GetVector(0);
}

void dBEntity::SetColour(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	pEntity->colour = pArguments->GetVector(0);
}

void dBEntity::SetEnable(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	pEntity->bEnabled = pArguments->GetBool(0);
}

void dBEntity::SetVisible(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	pEntity->bVisible = pArguments->GetBool(0);
}
