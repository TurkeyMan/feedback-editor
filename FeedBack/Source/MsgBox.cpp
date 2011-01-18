#include "FeedBack.h"
#include "Control.h"

MessageBoxScreen::MessageBoxScreen()
{
	frame.SetMaterial("Images/window");
}

MessageBoxScreen::~MessageBoxScreen()
{
}

void MessageBoxScreen::Show(const char *pMessage, MessageCallback pCallback)
{
	MFString_Copy(gMessage, pMessage);
	gpCompleteCallback = pCallback;

	Push();
}

void MessageBoxScreen::Update()
{
}

void MessageBoxScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once) || TestControl(dBCtrl_Menu_Accept, GHCT_Once))
	{
		Pop();

		if(gpCompleteCallback)
			gpCompleteCallback();
	}
}

void MessageBoxScreen::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = MFDisplay_IsWidescreen() ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = MFDisplay_IsWidescreen() ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	float textHeight = 16;
	float w = 400.0f, h, x, y;
	MFFont_GetStringWidth(pText, gMessage, textHeight*1.5f, w, -1, &h);

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	MFRect rect2 = { x, y, w, h };
	frame.SetFrame(&rect2);
	frame.Draw();

	MFFont_DrawTextAnchored(pText, gMessage, MakeVector(x + w*0.5f, y, 0.0f), MFFontJustify_Top_Center, w, textHeight*1.5f, MFVector::yellow);

	MFView_Pop();
}
