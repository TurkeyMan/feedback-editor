#include "FeedBack.h"
#include "TextProp.h"
#include "Action.h"
#include "Theme.h"
#include "Tools/ResourceCache.h"

static const char * gJustifyStrings[MFFontJustify_Max] =
{
	"top_left",
	"top_center",
	"top_right",
	"top_full",
	"center_left",
	"center",
	"center_right",
	"center_full",
	"bottom_left",
	"bottom_center",
	"bottom_right",
	"bottom_full"
};

int GetJustificationFromString(const char *pString)
{
	for(int a=0; a<sizeof(gJustifyStrings) / sizeof(gJustifyStrings[0]); ++a)
	{
		if(!MFString_CaseCmp(pString, gJustifyStrings[a]))
			return a;
	}

	return 0;
}

void dBTextProp::RegisterEntity()
{
	dBFactoryType *pType = dBEntityManager::RegisterEntityType("Text", Create, "Entity");

	dBActionManager::RegisterInstantAction("setfont", SetFont, pType);
	dBActionManager::RegisterInstantAction("settext", SetText, pType);
	dBActionManager::RegisterInstantAction("settextheight", SetTextHeight, pType);
	dBActionManager::RegisterInstantAction("setjustification", SetJustification, pType);
}

dBTextProp::dBTextProp()
{
	pFont = MFFont_Create("Arial");
	textHeight = 50.0f;
	justification = MFFontJustify_Top_Left;
	boundingRect.x = 0;
	boundingRect.y = 0;
	boundingRect.width = 1920;
	boundingRect.height = 1080;
}

dBTextProp::~dBTextProp()
{
	if(pFont)
		MFFont_Destroy(pFont);
}

void dBTextProp::Update()
{
	dBEntity::Update();
}

void dBTextProp::Draw()
{
	if(text.NumBytes() > 0)
		MFFont_DrawTextAnchored(pFont, text.CStr(), MakeVector(boundingRect.x, boundingRect.y), justification, boundingRect.width, textHeight, colour, -1, GetMatrix());

	dBEntity::Draw();
}

const char *dBTextProp::GetProperty(const char *pPropertyName)
{
	//...

	return dBEntity::GetProperty(pPropertyName);
}

void dBTextProp::SetFont(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	dBTextProp *pTextProp = (dBTextProp*)pEntity;
	dBResourceCache *pResourceCache = pEntity->GetManager()->GetScreen()->GetResourceCache();
	pTextProp->pFont = pResourceCache->FindFont(pArguments->GetString(0).CStr());

	// hack for now!
	if(!pTextProp->pFont)
		pTextProp->pFont = MFFont_Create("Arial");
}

void dBTextProp::SetText(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	dBTextProp *pTextProp = (dBTextProp*)pEntity;
	pTextProp->text = pArguments->GetString(0);
}

void dBTextProp::SetTextHeight(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	dBTextProp *pTextProp = (dBTextProp*)pEntity;
	pTextProp->textHeight = pArguments->GetFloat(0);
}

void dBTextProp::SetJustification(dBEntity *pEntity, dBRuntimeArgs *pArguments)
{
	dBTextProp *pTextProp = (dBTextProp*)pEntity;
	MFString arg = pArguments->GetString(0);
	const char *pStr = arg.CStr();

	if(!MFString_CaseCmp(pStr, "top_left"))
		pTextProp->justification = MFFontJustify_Top_Left;
	else if(!MFString_CaseCmp(pStr, "top_center"))
		pTextProp->justification = MFFontJustify_Top_Center;
	else if(!MFString_CaseCmp(pStr, "top_right"))
		pTextProp->justification = MFFontJustify_Top_Right;
	else if(!MFString_CaseCmp(pStr, "top_full"))
		pTextProp->justification = MFFontJustify_Top_Full;
	else if(!MFString_CaseCmp(pStr, "center_left"))
		pTextProp->justification = MFFontJustify_Center_Left;
	else if(!MFString_CaseCmp(pStr, "center"))
		pTextProp->justification = MFFontJustify_Center;
	else if(!MFString_CaseCmp(pStr, "center_right"))
		pTextProp->justification = MFFontJustify_Center_Right;
	else if(!MFString_CaseCmp(pStr, "center_full"))
		pTextProp->justification = MFFontJustify_Center_Full;
	else if(!MFString_CaseCmp(pStr, "bottom_Left"))
		pTextProp->justification = MFFontJustify_Bottom_Left;
	else if(!MFString_CaseCmp(pStr, "bottom_center"))
		pTextProp->justification = MFFontJustify_Bottom_Center;
	else if(!MFString_CaseCmp(pStr, "bottom_right"))
		pTextProp->justification = MFFontJustify_Bottom_Right;
	else if(!MFString_CaseCmp(pStr, "bottom_full"))
		pTextProp->justification = MFFontJustify_Bottom_Full;
}
