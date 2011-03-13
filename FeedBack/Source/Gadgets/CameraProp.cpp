#include "FeedBack.h"
#include "CameraProp.h"

CameraProp *CameraProp::Create(const char *pName)
{
	CameraProp *pProp = new CameraProp;

	MFString_Copy(pProp->name, pName);

	return pProp;
}

void CameraProp::Destroy()
{
	delete this;
}

void CameraProp::Update()
{

}

void CameraProp::Draw()
{
}
