#include "FeedBack.h"
#include "Control.h"

static float gBlinkTime = 0.4f;

StringBoxScreen::StringBoxScreen()
: stringLogic(256)
{
	stringLogic.SetChangeCallback(StringChangeCallback, this);

	frame.SetMaterial("Images/window");
}

StringBoxScreen::~StringBoxScreen()
{
}

void StringBoxScreen::Show(const char *pMessage, const char *pDefaultString, StringCallback pCallback)
{
	MFString_Copy(message, pMessage);
	stringLogic.SetString(pDefaultString ? pDefaultString : "");
	pCompleteCallback = pCallback;

	Push();
}

void StringBoxScreen::Update()
{
}

void StringBoxScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		Pop();

		if(pCompleteCallback)
			pCompleteCallback(1, stringLogic.GetString());
	}
	else if(TestControl(dBCtrl_Menu_Accept, GHCT_Once) || MFInput_WasPressed(Key_Return, IDD_Keyboard) || MFInput_WasPressed(Key_NumPadEnter, IDD_Keyboard))
	{
		Pop();

		if(pCompleteCallback)
			pCompleteCallback(0, stringLogic.GetString());
	}
	else
	{
		stringLogic.Update();
	}
}

void StringBoxScreen::Draw()
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

	float textHeight = 16;
	float w = 400.0f, h, x, y;
	MFFont_GetStringWidth(pHeading, message, textHeight*2.f, w, -1, &h);
	h += textHeight;

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	float stringY = y+h-textHeight;

	// draw string box
	MFRect rect2 = { x, y, w, h };
	frame.SetFrame(&rect2);
	frame.Draw();

	MFPrimitive_DrawUntexturedQuad(x-5, y-5 + (h-textHeight), w+10, textHeight + 10, MakeVector(0,0,0,1));

	// draw selection (if selected)
	if(selectionStart != selectionEnd)
	{
		int selMin = MFMin(selectionStart, selectionEnd);
		int selMax = MFMax(selectionStart, selectionEnd);

		float selMinX = MFFont_GetStringWidth(pText, pString, textHeight, 10000, selMin);
		float selMaxX = MFFont_GetStringWidth(pText, pString, textHeight, 10000, selMax);
		MFPrimitive_DrawUntexturedQuad(x+selMinX, stringY, selMaxX-selMinX, textHeight, MakeVector(0,0,0.6f,1));
	}

	// draw title
	MFFont_DrawTextAnchored(pHeading, message, MakeVector(x, y-5, 0.0f), MFFontJustify_Top_Left, w, textHeight*2.f, MFVector::yellow);

	// draw text
	MFFont_DrawText2(pText, x, stringY, textHeight, MFVector::white, pString);

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

	MFView_Pop();
}

void StringBoxScreen::StringChangeCallback(const char *pString, void *pUserData)
{
	gBlinkTime = 0.4f;
}
