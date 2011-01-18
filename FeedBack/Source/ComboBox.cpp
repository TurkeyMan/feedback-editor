#include "FeedBack.h"
#include "MFHeap.h"
#include "MFInput.h"
#include "Control.h"

#if defined(MF_WINDOWS)
	#include <Windows.h>
	extern HWND apphWnd;
#endif

static float gBlinkTime = 0.4f;

void StringCopyOverlap(char *pDest, const char *pSrc);

ComboBoxScreen::ComboBoxScreen()
 : stringLogic(256)
{
	stringLogic.SetChangeCallback(StringChangeCallback, this);
	pFirst = NULL;

	frame.SetMaterial("Images/window");
}

ComboBoxScreen::~ComboBoxScreen()
{
	Clear();
}

void ComboBoxScreen::Clear()
{
	while(pFirst)
	{
		ComboItem *pNext = pFirst->pNext;
		MFHeap_Free(pFirst);
		pFirst = pNext;
	}

	numItems = 0;
	selected = 0;
	listOffset = 0;
}

void ComboBoxScreen::Show(const char *pMessage, const char *pDefaultString, ComboCallback pCallback, ComboCallback _pChangeCallback, float _width, float _height)
{
	MFString_Copy(message, pMessage);
	pCompleteCallback = pCallback;
	pChangeCallback = _pChangeCallback;

	Clear();

	numItems = 0;
	selected = 0;
	listOffset = 0;

	width = _width;
	height = _height;

	const float textHeight = TEXT_HEIGHT;

	pageMax = (int)(height/textHeight);

	Push();

	stringLogic.SetString(pDefaultString);
}

void ComboBoxScreen::AddItem(const char *pItem, void *pData)
{
	ComboItem *pI = (ComboItem*)MFHeap_Alloc(sizeof(ComboItem) + MFString_Length(pItem)+1);
	pI->pString = (char*)&pI[1];
	MFString_Copy(pI->pString, pItem);
	pI->pData = pData;
	pI->pNext = NULL;

	if(!pFirst)
		pFirst = pI;
	else
	{
		ComboItem *pT = pFirst;
		while(pT->pNext)
			pT = pT->pNext;
		pT->pNext = pI;
	}

	++numItems;
}

void ComboBoxScreen::Update()
{
}

void ComboBoxScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		Pop();

		if(pCompleteCallback)
			pCompleteCallback(1, selected, NULL, NULL);
	}
	else if(TestControl(dBCtrl_Menu_Accept, GHCT_Once) || MFInput_WasPressed(Key_Return, IDD_Keyboard) || MFInput_WasPressed(Key_NumPadEnter, IDD_Keyboard))
	{
		Pop();

		if(pCompleteCallback)
		{
			ComboItem *pI = pFirst;
			for(int i=0; i<selected; ++i)
				pI = pI->pNext;

			pCompleteCallback(0, selected, stringLogic.GetString(), pI ? pI->pData : NULL);
		}
	}
	else if(MFInput_WasPressed(Key_Tab, IDD_Keyboard))
	{
		if(numItems)
		{
			ComboItem *pI = pFirst;
			for(int i=0; i<selected; ++i)
				pI = pI->pNext;

			stringLogic.SetString(pI->pString);

			if(pChangeCallback)
				pChangeCallback(0, selected, stringLogic.GetString(), pI->pData);
		}
	}
	else if(TestControl(dBCtrl_Menu_Up, GHCT_Delay))
	{
		selected = MFMax(selected-1, 0);
		listOffset = MFMin(selected, listOffset);
	}
	else if(TestControl(dBCtrl_Menu_Down, GHCT_Delay))
	{
		selected = MFMin(selected+1, numItems-1);
		listOffset = MFMax(listOffset, selected-(pageMax-1));
	}
	else
	{
		stringLogic.Update();
	}
}

void ComboBoxScreen::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = MFDisplay_IsWidescreen() ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = MFDisplay_IsWidescreen() ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	const char *pString = stringLogic.GetString();
	int cursorPos = stringLogic.GetCursorPos();
	int selectionStart, selectionEnd;
	stringLogic.GetSelection(&selectionStart, &selectionEnd);

	float textHeight = TEXT_HEIGHT;

	float w = width, h, x, y, lh = height - fmodf(height, textHeight);
	MFFont_GetStringWidth(pHeading, message, textHeight*2.f, w, -1, &h);
	h += lh + textHeight + 25;

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	float stringY = y+textHeight*1.5f + 10;

	// draw string box
	MFRect rect2 = { x, y, w, h-8 };
	frame.SetFrame(&rect2);
	frame.Draw();

//	MFPrimitive_DrawUntexturedQuad(x-10, y-10, w+20, h + 12.0f, MakeVector(0,0,0.8f,0.8f));
	MFPrimitive_DrawUntexturedQuad(x-5, stringY - 5, w+10, textHeight + 10, MakeVector(0,0,0,1));
	MFPrimitive_DrawUntexturedQuad(x-5, stringY + textHeight + 10, w+10, lh + 10, MakeVector(0,0,0,1));

	// draw title
	MFFont_DrawTextAnchored(pHeading, message, MakeVector(x, y-5, 0.0f), MFFontJustify_Top_Left, w, textHeight*2.f, MFVector::yellow);

	// draw selection (if selected)
	if(selectionStart != selectionEnd)
	{
		int selMin = MFMin(selectionStart, selectionEnd);
		int selMax = MFMax(selectionStart, selectionEnd);

		float selMinX = MFFont_GetStringWidth(pText, pString, textHeight, 10000, selMin);
		float selMaxX = MFFont_GetStringWidth(pText, pString, textHeight, 10000, selMax);
		MFPrimitive_DrawUntexturedQuad(x+selMinX, stringY, selMaxX-selMinX, textHeight, MakeVector(0,0,0.6f,1));
	}

	// draw text
	MFFont_DrawText(pText, x, stringY, textHeight, MFVector::white, pString);

	// blink cursor
	gBlinkTime -= MFSystem_TimeDelta();
	if(gBlinkTime < -0.4f) gBlinkTime += 0.8f;
	bool bCursor = gBlinkTime > 0.0f;

	// draw cursor
	if(bCursor)
	{
		// render cursor
		float cursorX = MFFont_GetStringWidth(pText, pString, textHeight, 10000, cursorPos);
		MFPrimitive_DrawUntexturedQuad(x+cursorX, stringY, 2, textHeight, MFVector::white);
	}

	y = stringY + textHeight + 15;

	ComboItem *pI = pFirst;
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
}

void ComboBoxScreen::StringChangeCallback(const char *pString, void *pUserData)
{
	gBlinkTime = 0.4f;

	ComboBoxScreen *pCB = (ComboBoxScreen*)pUserData;
	if(pCB->pChangeCallback)
		pCB->pChangeCallback(0, 0, pString, NULL);
}
