#if !defined(_SETTINGS_H)
#define _SETTINGS_H

#include "MFTranslation.h"
#include "Control.h"

struct dBSettings
{
	struct General
	{
		MFLanguage language;
		const char *pTheme;
	} general;

	struct Screen
	{
		int xRes, yRes;
		bool fullscreen;
		bool lowDetail;		// enable this to use low detail assets
	} screen;

	struct Sound
	{
		float masterLevel;
		float musicLevel;
		float guitarLevel;
		float bassLevel;
		float fxLevel;
		float tickLevel;
		float clapLevel;
	} sound;
/*
	struct Gameplay
	{

	} gameplay;
*/
	struct Editor
	{
		char lastOpenedChart[1024];
		char saveAction[1024];
		char editorFretboard[256];
	} editor;

	struct Controls
	{
		bool leftyFlip[2];
		GHControl controls[dBCtrl_Max];
	} controls;
};

void LoadConfigFile(const char *pConfig);
void SaveConfigFile(const char *pConfig);

#endif
