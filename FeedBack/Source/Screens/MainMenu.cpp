#include "FeedBack.h"
#include "Screen.h"
#include "MainMenu.h"

#include "Screens/Editor.h"

extern const char * gDifficultyStrings[GHS_NumDifficulties];
extern const char * gTrackStrings[GHS_NumTracks];
extern const char * gTrackCoopStrings[4];
extern const char * player2Strings[2];

MainMenuScreen::MainMenuScreen()
{
	gDifficultyStrings[0] = MFTranslation_GetString(pStrings, DIFFICULTY_EASY);
	gDifficultyStrings[1] = MFTranslation_GetString(pStrings, DIFFICULTY_MEDIUM);
	gDifficultyStrings[2] = MFTranslation_GetString(pStrings, DIFFICULTY_HARD);
	gDifficultyStrings[3] = MFTranslation_GetString(pStrings, DIFFICULTY_EXPERT);

	gTrackStrings[0] = MFTranslation_GetString(pStrings, STREAM_GUITAR);
	gTrackStrings[1] = MFTranslation_GetString(pStrings, STREAM_LEAD);
	gTrackStrings[2] = MFTranslation_GetString(pStrings, STREAM_BASS);
	gTrackStrings[3] = MFTranslation_GetString(pStrings, STREAM_ENHANCED_GUITAR);
	gTrackStrings[4] = MFTranslation_GetString(pStrings, STREAM_ENHANCED_LEAD);
	gTrackStrings[5] = MFTranslation_GetString(pStrings, STREAM_ENHANCED_BASS);
	gTrackStrings[6] = MFTranslation_GetString(pStrings, STREAM_10_KEY);
	gTrackStrings[7] = MFTranslation_GetString(pStrings, STREAM_DRUMS);
	gTrackStrings[8] = MFTranslation_GetString(pStrings, STREAM_DOUBLE_DRUMS);
	gTrackStrings[9] = MFTranslation_GetString(pStrings, STREAM_VOCALS);
	gTrackStrings[10] = MFTranslation_GetString(pStrings, STREAM_KEYBOARD);

	gTrackCoopStrings[0] = MFTranslation_GetString(pStrings, STREAM_BASS);
	gTrackCoopStrings[1] = MFTranslation_GetString(pStrings, STREAM_RHYTHM);
	gTrackCoopStrings[2] = MFTranslation_GetString(pStrings, STREAM_ENHANCED_BASS);
	gTrackCoopStrings[3] = MFTranslation_GetString(pStrings, STREAM_ENHANCED_RHYTHM);

	player2Strings[0] = MFTranslation_GetString(pStrings, MENU_BASS);
	player2Strings[1] = MFTranslation_GetString(pStrings, MENU_RHYTHM);
}

MainMenuScreen::~MainMenuScreen()
{
}

void MainMenuScreen::Select()
{
	item = 0; // start game
}

int MainMenuScreen::Update()
{
	dBScreen::SetNext(new EditorScreen);

	return 0;
}

void MainMenuScreen::Draw()
{
	
}

void MainMenuScreen::Deselect()
{
	delete this;
}
