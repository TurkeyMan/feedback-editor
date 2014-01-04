#include "FeedBack.h"
#include "Control.h"

#include "Fuji/MFTexture.h"
#include "Fuji/Materials/MFMat_Standard.h"

HelpScreen::HelpScreen()
{
	frame.SetMaterial("Images/window");
}

HelpScreen::~HelpScreen()
{
}

void HelpScreen::Update()
{
}

void HelpScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
		Pop();
}

void HelpScreen::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = MFDisplay_IsWidescreen() ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = MFDisplay_IsWidescreen() ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	MFPrimitive_DrawUntexturedQuad(0, 0, 640, 480, MakeVector(0,0,0,0));
	MFRect rect2 = { 45, 35, 640-90, 480-70 };
	frame.SetFrame(&rect2);
	frame.Draw();

	MFMaterial *pMat = MFMaterial_GetStockMaterial(MFMat_White);

	uintp old = MFMaterial_GetParameterI(pMat, MFMatStandard_ZRead, 0);
	MFMaterial_SetParameterI(pMat, MFMatStandard_ZRead, 0, 0);
	MFPrimitive_DrawUntexturedQuadV(MakeVector(40, 75, 0.1f), MakeVector(640-40, 480-70, 0.1f), MakeVector(0,0,0,1));
	MFMaterial_SetParameterI(pMat, MFMatStandard_ZRead, 0, old);

	float height = HELP_TEXT_HEIGHT;

	float boxHeight = 325.0f;
	float totalHeight = 0.0f;

	int a;
	for(a=dBCtrl_EditStart; a<dBCtrl_Max; ++a)
	{
		totalHeight += height;
		if(gConfig.controls.controls[a].data[1].data.trigger.button != -1)
			totalHeight += height;
	}

	// update scrollbar
	static float yOffset = 0.0f;
	if(TestControl(dBCtrl_Menu_Up, GHCT_Hold))
	{
		yOffset = MFMax(yOffset - MFSystem_TimeDelta() * 500.0f, 0.0f);
	}
	if(TestControl(dBCtrl_Menu_Down, GHCT_Hold))
	{
		yOffset = MFMin(yOffset + MFSystem_TimeDelta() * 500.0f, MFMax(totalHeight - boxHeight, 0.0f));
	}

	// draw scrollbar
	float barHeight = (boxHeight / totalHeight) * (boxHeight - 10);
	float barOffset = (yOffset / totalHeight) * (boxHeight - 10);

	MFPrimitive_DrawUntexturedQuad(640-73, 82, 26, boxHeight-4, MakeVector(0.3f,0.3f,0.3f,1));
	MFPrimitive_DrawUntexturedQuad(640-70, 85 + barOffset, 20, barHeight, MakeVector(1,1,0,1));

	// draw headings
	CenterText(28, 44, MFVector::yellow, MFStr("%s - %s", MFTranslation_GetString(pStrings, TITLE), MFTranslation_GetString(pStrings, MENU_HELP)), pHeading);
	CenterText(rect.height - 66, 36, MFVector::red, MFTranslation_GetString(pStrings, MENU_PRESS_ESC), pHeading);

	// draw text
	float y = 80.0f - yOffset;
	float x = 60.0f;

	for(a=dBCtrl_EditStart; a<dBCtrl_Max && y<480-70; ++a)
	{
		GHControl *pI = &gConfig.controls.controls[a];
		if(y > 75-height)
		{
			MFFont_DrawText(pText, MakeVector(x, y, 0.1f), height, MFVector::white, &GHControl::GetControlString(a)[5]);
			MFFont_DrawText(pText, MakeVector(x + 250, y, 0.1f), height, MFVector::white, MFStr("- %s", pI->data[0].data.GetString(false)));
		}
		y += height;
		if(pI->data[1].data.trigger.button != -1)
		{
			if(y > 75-height)
			MFFont_DrawText(pText, MakeVector(x + 250, y, 0.1f), height, MFVector::white, MFStr("- %s", pI->data[1].data.GetString(false)));
			y += height;
		}
	}

	MFView_Pop();
}
