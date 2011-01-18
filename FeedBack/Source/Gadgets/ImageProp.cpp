#include "FeedBack.h"
#include "ImageProp.h"
#include "Action.h"
#include "Theme.h"
#include "Tools/ResourceCache.h"

int GetJustificationFromString(const char *pString);

void dBImageProp::RegisterEntity()
{
	dBFactoryType *pType = dBEntityManager::RegisterEntityType("Image", Create, "Entity");

	dBActionManager::RegisterInstantAction("setimage", SetImage, pType);
}

dBImageProp::dBImageProp()
{
	pImage = NULL;
	imageRect.x = imageRect.y = 0;
	imageRect.width = imageRect.height = 128;
}

dBImageProp::~dBImageProp()
{
	if(pImage)
		MFMaterial_Destroy(pImage);
}

void dBImageProp::Update()
{
	dBEntity::Update();
}

void dBImageProp::Draw()
{
	dBEntity::Draw();

	MFMaterial_SetMaterial(pImage);

	MFPrimitive(PT_TriStrip | PT_Prelit);
	MFSetMatrix(GetMatrix());
	MFBegin(4);

	MFSetColour(colour);

	MFSetTexCoord1(0, 0);
	MFSetPosition(imageRect.x, imageRect.y, 0);

	MFSetTexCoord1(1, 0);
	MFSetPosition(imageRect.x+imageRect.width, imageRect.y, 0);

	MFSetTexCoord1(0, 1);
	MFSetPosition(imageRect.x, imageRect.y+imageRect.height, 0);

	MFSetTexCoord1(1, 1);
	MFSetPosition(imageRect.x+imageRect.width, imageRect.y+imageRect.height, 0);

	MFEnd();
}

const char *dBImageProp::GetProperty(const char *pPropertyName)
{
	//...

	return dBEntity::GetProperty(pPropertyName);
}

void dBImageProp::SetImage(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	dBImageProp *pImageProp = (dBImageProp*)pEntity;
	dBResourceCache *pResourceCache = pEntity->GetManager()->GetScreen()->GetResourceCache();
	pImageProp->pImage = pResourceCache->FindMaterial(pArguments->GetString(0).CStr());
}
