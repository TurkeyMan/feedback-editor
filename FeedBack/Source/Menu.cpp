#include "FeedBack.h"
#include "Fuji/MFFileSystem.h"
#include "ListBox.h"
#include "Control.h"

#include "Screens/Editor.h"

extern int gQuit;

void MenuScreen::Update()
{

}

void MenuScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
		Pop();
}

void MenuScreen::Draw()
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

	CenterText(30, 34, MFVector::yellow, MFTranslation_GetString(pStrings, MENU_MAIN_MENU), pHeading);

	float height = TEXT_HEIGHT;
//	float y = 90-height;
//	float x = 55.0f;

	CenterText(rect.height - 20 - 54, 44, MFVector::red, MFTranslation_GetString(pStrings, MENU_PRESS_ESC), pHeading);

	MFView_Pop();
}

void MenuScreen::Activate()
{
	Pop();

#if defined(_PSP)
	gpListBox->Show(MFTranslation_GetString(pStrings, MENU_MAIN_MENU), ListCallback, 420.0f, 300.0f);
#else
	gpListBox->Show(MFTranslation_GetString(pStrings, MENU_MAIN_MENU), ListCallback);
#endif
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_NEW_CHART));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_LOAD_CHART));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_SAVE_CHART));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_CHART_SETTINGS));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_PROGRAM_SETTINGS));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_SHOW_HELP));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_QUIT));
}

void ScanFolder(const char *pFindPattern, const char *pExtention, const char *pPrompt, ListCallback pCallback, void *pUserData, bool &foundFiles)
{
	MFFindData fd;
	bool more = true;

	MFFind *pFind = MFFileSystem_FindFirst(MFStr("songs:%s*", pFindPattern), &fd);

	if(pFind)
	{
		int extLen = MFString_Length(pExtention);

		while(pFind && more)
		{
			if(MFString_Compare(fd.pFilename, ".") && MFString_Compare(fd.pFilename, "..") && MFString_Compare(fd.pFilename, ".svn"))
			{
				if(fd.attributes & MFFA_Directory)
				{
					ScanFolder(MFStr("%s%s/", pFindPattern, fd.pFilename), pExtention, pPrompt, pCallback, pUserData, foundFiles);
				}
				else
				{
					int len = MFString_Length(fd.pFilename);

					if(len >= extLen && !MFString_CaseCmp(fd.pFilename + len - extLen, pExtention))
					{
						if(!foundFiles)
						{
							gpListBox->Show(pPrompt, pCallback, 550, 350);
							foundFiles = true;
						}

						gpListBox->AddItem(MFStr("%s%s", pFindPattern, fd.pFilename), pUserData);
					}
				}
			}

			more = MFFileSystem_FindNext(pFind, &fd);
		}

		MFFileSystem_FindClose(pFind);
	}
}

void SelectFile(const char *pExtention, const char *pPrompt, ListCallback pCallback, void *pUserData = NULL)
{
	bool foundFiles = false;

	const char *pExt = pExtention;
	while(pExt)
	{
		char *pDelim = MFString_Chr(pExt, '|');
		if(pDelim)
		{
			pExt = MFStrN(pExt, (int)(pDelim - pExt));
			++pDelim;
		}

		ScanFolder("", pExt, pPrompt, pCallback, pUserData, foundFiles);

		pExt = pDelim;
	}

	if(!foundFiles)
		gpMsgBox->Show(MFStr(MFTranslation_GetString(pStrings, MENU_NOT_FOUND), pExtention));
}

void MenuScreen::ListCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
		return;

	switch(listID)
	{
		case 0:
		{
			// new song..
			gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_CHOOSE_AUDIO), "songs:", "*.mp3|*.ogg|*.wav", NewCallback, 0);
			break;
		}
		case 1:
		{
			// load song..
			gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_LOAD_CHART), "songs:", "*.chart|*.mid|*.midi", LoadCallback, 0);
			break;
		}
		case 2:
		{
			gEditor.pSong->SaveChart();

			MFVoice *pVoice = MFSound_Play(gEditor.pSaveSound, MFPF_BeginPaused);
			MFSound_SetVolume(pVoice, gConfig.sound.fxLevel);
			MFSound_Pause(pVoice, false);

			if(gConfig.editor.saveAction[0])
				system(gConfig.editor.saveAction);
			break;
		}
		case 3:
			ShowChartSettings(true);
			break;
		case 4:
			gpListBox->Show(MFTranslation_GetString(pStrings, MENU_PROGRAM_SETTINGS), ConfigCallback, 550, 350);
			gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_LEFTY_FLIP_1), MFTranslation_GetString(pStrings, gConfig.controls.leftyFlip[0] ? MENU_ON : MENU_OFF)));
			gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_LEFTY_FLIP_2), MFTranslation_GetString(pStrings, gConfig.controls.leftyFlip[1] ? MENU_ON : MENU_OFF)));
			gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_FRETBOARD), gConfig.editor.editorFretboard[0] ? gConfig.editor.editorFretboard : MFStr("[%s]", MFTranslation_GetString(pStrings, MENU_RANDOM))));
			gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_VOLUME));
			gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_MAIN_MENU));
			break;
		case 5:
			gpHelp->Push();
			break;
		case 6:
			// offer to save...

			gQuit = 1;
			break;
	}
}

void MenuScreen::NewCallback(int cancel, const char *pFilename, const char *pPath, void *pData)
{
	if(cancel)
		return;

	if(gEditor.pSong)
		gEditor.pSong->Destroy();

	const char *pColon;
	if(pColon = MFString_Chr(pPath, ':'))
		pPath = pColon + 1;

	gEditor.pSong = dBChart::Create(pPath);
	gEditor.offset = gEditor.measure = gEditor.beat = 0;
	gEditor.currentBPM = gEditor.pSong->GetStartBPM();

	ShowChartSettings(false);
}

void MenuScreen::LoadCallback(int cancel, const char *pFilename, const char *pPath, void *pData)
{
	if(cancel)
		return;

	const char *pColon;
	if(pColon = MFString_Chr(pPath, ':'))
		pPath = pColon + 1;

	char *pExt = MFString_RChr(pPath, '.');

	if(!MFString_CaseCmp(pExt, ".chart"))
	{
		if(gEditor.pSong)
			gEditor.pSong->Destroy();

		*pExt = 0;
		gEditor.pSong = dBChart::LoadChart(pPath);
		gEditor.offset = gEditor.measure = gEditor.beat = 0;
		gEditor.currentBPM = gEditor.pSong->GetStartBPM();
	}
	else if(!MFString_CaseCmp(pExt, ".mid") || !MFString_CaseCmp(pExt, ".midi"))
	{
		if(gEditor.pSong)
			gEditor.pSong->Destroy();

		gEditor.pSong = dBChart::LoadMidi(pPath);
		if(gEditor.pSong)
		{
			gEditor.offset = gEditor.measure = gEditor.beat = 0;
			gEditor.currentBPM = gEditor.pSong->GetStartBPM();
		}
		else
		{
			gEditor.pSong = dBChart::Create();
			gpMsgBox->Show(MFStr(MFTranslation_GetString(pStrings, MENU_MIDI_LOAD_FAILED), pPath));
		}
	}
	else
	{
		// unsupported file...
	}
}

void MenuScreen::ShowChartSettings(bool returnToMenu)
{
	gpListBox->Show(MFTranslation_GetString(pStrings, MENU_CHART_SETTINGS), SettingsCallback, 550, 350);
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_SONG_NAME), gEditor.pSong->songName));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_ARTISTS_NAME), gEditor.pSong->artistName));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_CHARTERS_NAME), gEditor.pSong->charterName));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_PLAYER_2), MFTranslation_GetString(pStrings, gEditor.pSong->player2Type == GHP2T_Bass ? MENU_BASS : MENU_RHYTHM)));
	gpListBox->AddItem(MFStr("%s: %d", MFTranslation_GetString(pStrings, MENU_DIFFICULTY), gEditor.pSong->difficulty));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_GENRE), gEditor.pSong->genre));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_MEDIA), gEditor.pSong->mediaType));
	gpListBox->AddItem(MFStr("%s: %.2f - %.2f", MFTranslation_GetString(pStrings, MENU_PREVIEW), gEditor.pSong->previewStart, gEditor.pSong->previewEnd));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_MUSIC_URL), gEditor.pSong->musicURL[0] ? gEditor.pSong->musicURL : MFTranslation_GetString(pStrings, MENU_NONE)));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_PREVIEW_URL), gEditor.pSong->previewURL[0] ? gEditor.pSong->previewURL : MFTranslation_GetString(pStrings, MENU_NONE)));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_AUDIO), gEditor.pSong->musicFilename[0] ? gEditor.pSong->musicFilename : MFTranslation_GetString(pStrings, MENU_NONE)));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_GUITAR), gEditor.pSong->guitarFilename[0] ? gEditor.pSong->guitarFilename : MFTranslation_GetString(pStrings, MENU_NONE)));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_RHYTHM_BASS), gEditor.pSong->bassFilename[0] ? gEditor.pSong->bassFilename : MFTranslation_GetString(pStrings, MENU_NONE)));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, MENU_FRETBOARD), gEditor.pSong->fretboard[0] ? gEditor.pSong->fretboard : MFTranslation_GetString(pStrings, MENU_NONE)));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, returnToMenu ? MENU_MAIN_MENU : MENU_EXIT), returnToMenu ? (void*)1 : (void*)0);
}

void MenuScreen::SettingsCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
	{
		gMenu.Push();
	}
	else
	{
		switch(listID)
		{
			case 0:
				gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_ENTER_SONG_NAME), gEditor.pSong->songName, NameCallback);
				break;
			case 1:
				gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_ENTER_ARTISTS_NAME), gEditor.pSong->artistName, ArtistCallback);
				break;
			case 2:
				gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_ENTER_CHARTERS_NAME), gEditor.pSong->charterName, CharterCallback);
				break;
			case 3:
				gpListBox->Show(MFTranslation_GetString(pStrings, MENU_PLAYER_2), Player2Callback, 300, 50);
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_BASS));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_RHYTHM));
				break;
			case 4:
				gpListBox->Show(MFTranslation_GetString(pStrings, MENU_SELECT_DIFFICULTY), DifficultyCallback, 300, 200);
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_1));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_2));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_3));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_4));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_5));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_6));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_7));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_8));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_9));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_10));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_11));
				break;
			case 5:
				gpListBox->Show(MFTranslation_GetString(pStrings, MENU_SELECT_GENRE), GenreCallback, 300, 100);
				gpListBox->AddItem("Rock");
				gpListBox->AddItem("Metal");
				gpListBox->AddItem("Black Metal");
				gpListBox->AddItem("Punk");
				gpListBox->AddItem("Acid Rock");
				gpListBox->AddItem("Alternative");
				break;
			case 6:
				gpListBox->Show(MFTranslation_GetString(pStrings, MENU_SELECT_MEDIA), MediaCallback, 300, 100);
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MEDIA_CD));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MEDIA_VINYL));
				gpListBox->AddItem(MFTranslation_GetString(pStrings, MEDIA_CASETTE));
				break;
			case 7:
				gMenu.ShowChartSettings(true);
				break;
			case 8:
				gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_ENTER_MUSIC_URL), gEditor.pSong->musicURL, URLCallback);
				break;
			case 9:
				gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_ENTER_PREVIEW_URL), gEditor.pSong->previewURL, PreviewCallback);
				break;
			case 10:
				gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_CHOOSE_MUSIC_STREAM), MFStr("songs:%s", MFStr_GetFilePath(gEditor.pSong->songPath)), "*.mp3|*.ogg|*.wav", SetMusicCallback, FileSelectorScreen::SF_ShowNone | FileSelectorScreen::SF_NoFolders, gEditor.pSong->musicFilename);
				break;
			case 11:
				gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_CHOOSE_GUITAR_STREAM), MFStr("songs:%s", MFStr_GetFilePath(gEditor.pSong->songPath)), "*.mp3|*.ogg|*.wav", SetMusicCallback, FileSelectorScreen::SF_ShowNone | FileSelectorScreen::SF_NoFolders, gEditor.pSong->guitarFilename);
				break;
			case 12:
				gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_CHOOSE_BASS_STREAM), MFStr("songs:%s", MFStr_GetFilePath(gEditor.pSong->songPath)), "*.mp3|*.ogg|*.wav", SetMusicCallback, FileSelectorScreen::SF_ShowNone | FileSelectorScreen::SF_NoFolders, gEditor.pSong->bassFilename);
				break;
			case 13:
				gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_CHOOSE_FRETBOARD), MFStr("songs:%s", MFStr_GetFilePath(gEditor.pSong->songPath)), "*.png|*.tga|*.bmp", SetFretboardCallback, FileSelectorScreen::SF_ShowNone | FileSelectorScreen::SF_NoFolders);
				break;
			case 14:
				if(pData)
					gMenu.Push();
				break;
		}
	}
}

void MenuScreen::NameCallback(int cancel, const char *pString)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->songName, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::ArtistCallback(int cancel, const char *pString)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->artistName, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::CharterCallback(int cancel, const char *pString)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->charterName, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::Player2Callback(int cancel, int listID, const char *pString, void *pData)
{
	if(!cancel)
		gEditor.pSong->player2Type = listID;

	gMenu.ShowChartSettings(true);
}

void MenuScreen::DifficultyCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(!cancel)
		gEditor.pSong->difficulty = atoi(pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::GenreCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->genre, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::MediaCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->mediaType, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::URLCallback(int cancel, const char *pString)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->musicURL, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::PreviewCallback(int cancel, const char *pString)
{
	if(!cancel)
		MFString_Copy(gEditor.pSong->previewURL, pString);

	gMenu.ShowChartSettings(true);
}

void MenuScreen::SetMusicCallback(int cancel, const char *pFilename, const char *pPath, void *pData)
{
	if(!cancel)
	{
		// destroy existing stream
		if(pData == gEditor.pSong->musicFilename && gEditor.pSong->pStream)
		{
			MFSound_DestroyStream(gEditor.pSong->pStream);
			gEditor.pSong->pStream = NULL;
		}
		else if(pData == gEditor.pSong->guitarFilename && gEditor.pSong->pGuitar)
		{
			MFSound_DestroyStream(gEditor.pSong->pGuitar);
			gEditor.pSong->pGuitar = NULL;
		}
		else if(pData == gEditor.pSong->bassFilename && gEditor.pSong->pBass)
		{
			MFSound_DestroyStream(gEditor.pSong->pBass);
			gEditor.pSong->pBass = NULL;
		}

		if(!MFString_Compare(pFilename, MFStr("[%s]", MFTranslation_GetString(pStrings, MENU_NONE))))
		{
			// we set it to 'None'..
			*(char*)pData = 0;
		}
		else
		{
			// skip volume name
			const char *pColon;
			if(pColon = MFString_Chr(pPath, ':'))
				pPath = pColon + 1;

			// skip past song path
			const char *pSongPath = MFStr_GetFilePath(gEditor.pSong->songPath);
			int len = MFString_Length(pSongPath);
			if(!MFString_CompareN(pPath, pSongPath, len))
				pPath += len;

			// load the the track.
			MFString_Copy((char*)pData, pPath);

			if(pData == gEditor.pSong->musicFilename)
			{
				if(*gEditor.pSong->musicFilename)
					gEditor.pSong->pStream = gEditor.pSong->PlayStream(gEditor.pSong->musicFilename);
			}
			else if(pData == gEditor.pSong->guitarFilename)
			{
				if(*gEditor.pSong->guitarFilename)
					gEditor.pSong->pGuitar = gEditor.pSong->PlayStream(gEditor.pSong->guitarFilename);
			}
			else if(pData == gEditor.pSong->bassFilename)
			{
				if(*gEditor.pSong->bassFilename)
					gEditor.pSong->pBass = gEditor.pSong->PlayStream(gEditor.pSong->bassFilename);
			}
		}
	}

	gMenu.ShowChartSettings(true);
}

void MenuScreen::SetFretboardCallback(int cancel, const char *pFilename, const char *pPath, void *pData)
{
	if(!cancel)
	{
		if(gEditor.pSong->pFretboard)
		{
			MFMaterial_Release(gEditor.pSong->pFretboard);
			gEditor.pSong->pFretboard = NULL;
		}

		if(!MFString_Compare(pFilename, MFStr("[%s]", MFTranslation_GetString(pStrings, MENU_NONE))))
		{
			gEditor.pSong->fretboard[0] = 0;
		}
		else
		{
			MFString_Copy(gEditor.pSong->fretboard, MFStr_GetFileNameWithoutExtension(pFilename));
			gEditor.pSong->pFretboard = MFMaterial_Create(gEditor.pSong->fretboard);
		}
	}

	gMenu.ShowChartSettings(true);
}

void MenuScreen::ConfigCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
	{
		gMenu.Push();
	}
	else
	{
		switch(listID)
		{
			case 0:
				gConfig.controls.leftyFlip[0] = !gConfig.controls.leftyFlip[0];
				ListCallback(0, 4, NULL, NULL);
				break;

			case 1:
				gConfig.controls.leftyFlip[1] = !gConfig.controls.leftyFlip[1];
				ListCallback(0, 4, NULL, NULL);
				break;

			case 2:
				gpFileSelector->Show(MFTranslation_GetString(pStrings, MENU_CHOOSE_FRETBOARD), "theme:Fretboards/", "*.png|*.tga|*.bmp", SetFretboard, FileSelectorScreen::SF_ShowNone | FileSelectorScreen::SF_NoFolders);
				break;

			case 3:
				((EditorScreen*)dBScreen::GetCurrent())->gMixer.Push();
				break;

			case 4:
				gMenu.Push();
				break;
		}
	}
}

void MenuScreen::SetFretboard(int cancel, const char *pFilename, const char *pPath, void *pData)
{
	if(!cancel)
	{
		if(!MFString_Compare(pFilename, MFStr("[%s]", MFTranslation_GetString(pStrings, MENU_NONE))))
			gConfig.editor.editorFretboard[0] = 0;
		else
		{
			MFString_Copy(gConfig.editor.editorFretboard, MFStr_GetFileNameWithoutExtension(pPath));
			gGame.pTrack->LoadFretboard(gConfig.editor.editorFretboard);
			gGame.pTrack2->LoadFretboard(gConfig.editor.editorFretboard);
		}
	}

	ListCallback(0, 4, NULL, NULL);
}
