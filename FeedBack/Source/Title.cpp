#include "GHEd.h"
#include "Title.h"

void TitleScreen::Update()
{

}

void TitleScreen::UpdateInput()
{
	// update menu
}

void TitleScreen::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = MFDisplay_IsWidescreen() ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = MFDisplay_IsWidescreen() ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	MFPrimitive_DrawUntexturedQuad(20, 20, 640-40, 480-40, MakeVector(0,0,0,0.8f));

	CenterText(30, 34, MFVector::yellow, "Main Menu", pHeading);

	float height = TEXT_HEIGHT;
	float y = 90-height;
	float x = 55.0f;

//	MFFont_DrawText(pText, , y+=height, 20, MFVector::white, "H - Enter help screen");

	CenterText(rect.height - 20 - 54, 44, MFVector::red, "Press ESC to continue", pFancy);

	MFView_Pop();
}

void TitleScreen::Activate()
{
	Pop();

#if defined(_PSP)
	gListBox.Show("Main Menu", ListCallback, 420.0f, 300.0f);
#else
	gListBox.Show("Main Menu", ListCallback);
#endif
	gListBox.AddItem("New Chart");
	gListBox.AddItem("Load Chart");
	gListBox.AddItem("Save Chart");
	gListBox.AddItem("Chart Settings");
	gListBox.AddItem("Program Settings");
	gListBox.AddItem("Show Help");
	gListBox.AddItem("Quit");
}
