#include "FeedBack.h"
#include "Fuji/MFHeap.h"
#include "Control.h"

FingerEditScreen::FingerEditScreen()
{
}

FingerEditScreen::~FingerEditScreen()
{
}

void FingerEditScreen::Show()
{
	Push();
}

void FingerEditScreen::Update()
{
}

void FingerEditScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		Pop();
	}
	else if(TestControl(dBCtrl_Menu_Accept, GHCT_Once))
	{
		Pop();
	}
}

void FingerEditScreen::Draw()
{
	GHScreen::Draw();
/*
	MFView_Push();

	MFRect rect;
	rect.x = MFDisplay_IsWidescreen() ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = MFDisplay_IsWidescreen() ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	float textHeight = TEXT_HEIGHT;

	float w = width, h, x, y, lh = height - fmodf(height, textHeight);
	MFFont_GetStringWidth(pHeading, gMessage, textHeight*2.f, w, -1, &h);
	h += lh;

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	MFPrimitive_DrawUntexturedQuad(x-10, y-10, w+20, h + 20.0f, MakeVector(0,0,0.8f,0.8f));
	MFPrimitive_DrawUntexturedQuad(x-5, y-5 + (h-lh), w+10, lh + 10, MakeVector(0,0,0,1));

	MFFont_DrawTextAnchored(pHeading, gMessage, MakeVector(x, y-5, 0.0f), MFFontJustify_Top_Left, w, textHeight*2.f, MFVector::yellow);

	y = y + (h-lh);

	ListItem *pI = pFirst;
	for(int a=0; a<listOffset; a++)
		pI = pI->pNext;

	for(int a=0; a<pageMax && pI; a++, pI = pI->pNext)
	{
		bool hilite = a == (selected - listOffset);

		if(hilite)
			MFPrimitive_DrawUntexturedQuad(x-5, y, w+10, textHeight, MakeVector(0,0,0.5f,1));

		MFFont_DrawText(pText, x, y, textHeight, hilite ? MFVector::yellow : MFVector::white, pI->pString);
		y += textHeight;
	}

	MFView_Pop();
*/
}
