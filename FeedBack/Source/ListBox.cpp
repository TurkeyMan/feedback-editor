#include "FeedBack.h"
#include "Fuji/MFHeap.h"
#include "Control.h"

ListBoxScreen::ListBoxScreen()
{
	pFirst = NULL;

	frame.SetMaterial("Images/window");
}

ListBoxScreen::~ListBoxScreen()
{
	Destroy();
}

void ListBoxScreen::Destroy()
{
	while(pFirst)
	{
		ListItem *pNext = pFirst->pNext;
		MFHeap_Free(pFirst);
		pFirst = pNext;
	}

	numItems = 0;
	selected = 0;
	listOffset = 0;
}

void ListBoxScreen::Show(const char *pMessage, ListCallback pCallback, float _width, float _height)
{
	MFString_Copy(gMessage, pMessage);
	gpCompleteCallback = pCallback;

	Destroy();

	numItems = 0;
	selected = 0;
	listOffset = 0;

	width = _width;
	height = _height;

	const float textHeight = TEXT_HEIGHT;

	pageMax = (int)(height/textHeight);

	typeBuffer[0] = 0;
	typeTimeout = 0.0f;

	Push();
}

void ListBoxScreen::AddItem(const char *pItem, void *pData)
{
	ListItem *pI = (ListItem*)MFHeap_Alloc(sizeof(ListItem) + MFString_Length(pItem)+1);
	pI->pString = (char*)&pI[1];
	MFString_Copy(pI->pString, pItem);
	pI->pData = pData;
	pI->pNext = NULL;

	if(!pFirst)
		pFirst = pI;
	else
	{
		ListItem *pT = pFirst;
		while(pT->pNext)
			pT = pT->pNext;
		pT->pNext = pI;
	}

	++numItems;
}

void ListBoxScreen::Update()
{
}

void ListBoxScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		Pop();

		if(gpCompleteCallback)
			gpCompleteCallback(1, selected, NULL, NULL);
	}
	else if(TestControl(dBCtrl_Menu_Accept, GHCT_Once))
	{
		Pop();

		if(gpCompleteCallback)
		{
			ListItem *pI = pFirst;
			for(int i=0; i<selected; ++i)
				pI = pI->pNext;

			gpCompleteCallback(0, selected, pI->pString, pI->pData);
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

	// check for alpha-numeric keypress
	int pressed = 0;
	bool shift = MFInput_Read(Key_LShift, IDD_Keyboard) || MFInput_Read(Key_RShift, IDD_Keyboard);
	bool caps  = MFInput_GetKeyboardStatusState(KSS_CapsLock);

	for(int a=0; a<30; ++a)
	{
		if(MFInput_WasPressed(Key_A + a, IDD_Keyboard))
		{
			pressed = MFInput_KeyToAscii(Key_A + a, shift, caps);
			break;
		}
	}
	if(!pressed)
	{
		for(int a=0; a<14; ++a)
		{
			if(MFInput_WasPressed(Key_Comma + a, IDD_Keyboard))
			{
				pressed = MFInput_KeyToAscii(Key_Comma + a, shift, caps);
				break;
			}
		}
	}

	if(pressed)
	{
		// append keystroke
		size_t len = MFString_Length(typeBuffer);
		if(len < 255)
		{
			typeBuffer[len++] = pressed;
			typeBuffer[len] = 0;
			typeTimeout = 0.75f;

			// find first matching item in the list
			ListItem *pI = pFirst;
			int index = 0;
			while(pI)
			{
				if(!MFString_CaseCmpN(typeBuffer, pI->pString, len))
				{
					selected = index;
					listOffset = MFClamp(selected-(pageMax-1), listOffset, selected);
				}

				++index;
				pI = pI->pNext;
			}
		}
	}
	else if(typeTimeout > 0.0f)
	{
		typeTimeout -= MFSystem_GetTimeDelta();
		if(typeTimeout <= 0.0f)
			typeBuffer[0] = 0;
	}
}

void ListBoxScreen::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = (MFDisplay_GetAspectRatio() >= 1.5) ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = (MFDisplay_GetAspectRatio() >= 1.5) ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	float textHeight = TEXT_HEIGHT;

	float w = width, h, x, y, lh = height - fmodf(height, textHeight);
	MFFont_GetStringWidth(pHeading, gMessage, textHeight*2.f, w, -1, &h);
	h += lh;

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	MFRect rect2 = { x, y, w, h };
	frame.SetFrame(&rect2);
	frame.Draw();

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

		MFFont_DrawText2(pText, x, y, textHeight, hilite ? MFVector::yellow : MFVector::white, pI->pString);
		y += textHeight;
	}

	MFView_Pop();
}
