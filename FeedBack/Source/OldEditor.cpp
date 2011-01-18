#include "FeedBack.h"

#include "MFSystem.h"
#include "MFDisplay.h"
#include "MFFileSystem.h"
#include "FileSystem/MFFileSystemNative.h"
#include "MFView.h"
#include "DebugMenu.h"
#include "MFPrimitive.h"
#include "MFRenderer.h"

#include "Control.h"
#include "Menu.h"

#include "Screens/Editor.h"

extern MenuItemBool bMetronome;
extern MenuItemBool bClaps;
extern MenuItemIntString bHalfSpeed;

bool bSelecting = false;

int gQuantiseSteps[] = { 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64 };

int gButtons[] =
{
	Button_GH_Green,
	Button_GH_Red,
	Button_GH_Yellow,
	Button_GH_Blue,
	Button_GH_Orange
};

float gSpeeds[] = { 1.0f, 0.85f, 0.75f, 0.5f };

// **** Funcitons ****


GHEditor::GHEditor()
{
	pSong = NULL;

	selectedStream = 0;
	currentStream[0] = GHS_ExpertSingle;
	currentStream[1] = GHS_Unknown;

	state = GHPS_Stopped;
	measure = beat = offset = 0;
	currentBPM = 120000;
	currentTimeSignature = 4;
	lastTimeSignature = 0;
	speedMultiplier = 1.0f;
	quantiseStep = 3;
	quantisation = gQuantiseSteps[quantiseStep];
	metronome = false;
	clap = false;
	selectStart = 0;
	selectEnd = 0;
	copyLen = 0;

	selectEvents.Init();

	musicPan = 0.0f;
}

#define MakeFixed(x, f) ((int64)(x) << (f))
#define GetFixed(x, f) ((int)(x) >> (f))
#define FixedMul(x, y, f) (((x)*(y)) >> (f))
#define FixedDiv(x, y, f) (((x) << (f)) / (y))

#define MakeF16(x) MakeFixed(x, 16)
#define GetF16(x) GetFixed(x, 16)
#define F16Mul(x, y) FixedMul(x, y, 16)
#define F16Div(x, y) FixedDiv(x, y, 16)

void OffsetToMeasureAndBeat(int offset, int *pMeasure, int *pBeat)
{
	int measureLength = gEditor.pSong->resolution * 4;
	*pMeasure = offset / measureLength;
	int64 beatOffset = MakeF16(offset % measureLength);
	int64 quantLength = F16Div(MakeF16(measureLength), MakeF16(gEditor.quantisation));
	int64 beat = F16Div(beatOffset, quantLength);
	*pBeat = GetF16(beat);
}

int MeasureAndBeatToOffset(int measure, int beat)
{
	int measureLength = gEditor.pSong->resolution * 4;
	int offset = gEditor.measure * measureLength;
	int64 beatOffset = F16Div(MakeF16(measureLength * gEditor.beat), MakeF16(gEditor.quantisation));
	return offset + GetF16(beatOffset);
}

void SetCustomQuantisation(int cancel, const char *pString)
{
	if(cancel || !*pString) return;

	int quant = MFClamp(0, atoi(pString), gEditor.pSong->resolution);
	if(quant)
	{
		gEditor.quantisation = quant;
		OffsetToMeasureAndBeat(gEditor.offset, &gEditor.measure, &gEditor.beat);
	}
}

void PlaceEvent(int cancel, int item, const char *pString, void *pUserData)
{
	if(cancel || !*pString) return;

	GHEvent *pEv = gEditor.pSong->events.AddStringEvent(GHE_Event, gEditor.offset, pString);
	pEv->time = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);		
}

void PlaceTrackEvent(int cancel, int item, const char *pString, void *pUserData)
{
	if(cancel || !*pString) return;

	GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].AddStringEvent(GHE_Event, gEditor.offset, pString);
	pEv->time = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);		
}

void PlaceSection(int cancel, const char *pString)
{
	if(cancel || !*pString) return;

	GHEvent *pEv = gEditor.pSong->events.AddStringEvent(GHE_Event, gEditor.offset, MFStr("section %s", pString));
	pEv->time = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);
}

void GotoSection(int cancel, const char *pString)
{
	if(cancel || !*pString) return;

	const char *pTest = pString;
	while(*pTest && (MFIsNumeric(*pTest) || *pTest == '.'))
		++pTest;

	int res = gEditor.pSong->GetRes();

	if(pTest > pString)
	{
		if(!*pTest)
		{
			OffsetToMeasureAndBeat((int)((float)atof(pString) * res), &gEditor.measure, &gEditor.beat);
			return;
		}
		if(!MFString_CaseCmp(pTest, "s"))
		{
			float time = (float)atof(pString);
			OffsetToMeasureAndBeat(gEditor.pSong->CalculateTickAtTime((int64)(time*1000000.0f)), &gEditor.measure, &gEditor.beat);
			return;
		}
		else if(!MFString_CaseCmp(pTest, "ms"))
		{
			float time = (float)atof(pString);
			OffsetToMeasureAndBeat(gEditor.pSong->CalculateTickAtTime((int64)(time*1000.0f)), &gEditor.measure, &gEditor.beat);
			return;
		}
	}

	// find section by name
	int len = MFString_Length(pString);

	GHEvent *pSE = gEditor.pSong->events.First();

	while(pSE)
	{
		if(!MFString_CaseCmpN(pSE->GetString(), "section ", 8) && !MFString_CaseCmpN(&pSE->GetString()[8], pString, len))
		{
			OffsetToMeasureAndBeat(pSE->tick, &gEditor.measure, &gEditor.beat);
			return;
		}

		pSE = pSE->Next();
	}
}

void ScanAndSortEvents(int cancel, int item, const char *pString, void *pUserData)
{
	gpComboBox->Clear();

	int numSuggestions = EditorScreen::eventSuggestions.CollectAndSortEventSuggestions(pString);

	for(int a=0; a<numSuggestions; ++a)
	{
		gpComboBox->AddItem(EditorScreen::eventSuggestions.GetSuggestion(a));
	}
}

void ListSelect(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel) return;

	if(!pData)
		gpComboBox->Show(MFTranslation_GetString(pStrings, ENTER_EVENT), "", PlaceEvent, ScanAndSortEvents);
	else
		gEditor.pSong->events.RemoveEvent((GHEvent*)pData);
}

void TrackListSelect(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel) return;

	if(!pData)
		gpComboBox->Show(MFTranslation_GetString(pStrings, ENTER_TRACK_EVENT), "", PlaceTrackEvent, ScanAndSortEvents);
	else
		gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].RemoveEvent((GHEvent*)pData);
}

void PlaceEvent()
{
	// check if theres 
	GHEvent *pEv = gEditor.pSong->events.FindEvent(GHE_Event, gEditor.offset);
	while(pEv && !MFString_CompareN(pEv->GetString(), "section ", 8))
	{
		pEv = pEv->NextSibling();
	}

	if(pEv)
	{
		gpListBox->Show(MFTranslation_GetString(pStrings, ADD_EVENT_OR), ListSelect);
		gpListBox->AddItem(MFTranslation_GetString(pStrings, ADD_EVENT));
		while(pEv)
		{
			if(MFString_CompareN(pEv->GetString(), "section ", 8))
				gpListBox->AddItem(pEv->GetString(), pEv);

			pEv = pEv->NextSibling();
		}
	}
	else
		gpComboBox->Show(MFTranslation_GetString(pStrings, ENTER_EVENT), "", PlaceEvent, ScanAndSortEvents);
}

void PlaceTrackEvent()
{
	// check if theres 
	GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].FindEvent(GHE_Event, gEditor.offset);

	if(pEv)
	{
		gpListBox->Show(MFTranslation_GetString(pStrings, ADD_TRACK_EVENT_OR), TrackListSelect);
		gpListBox->AddItem(MFTranslation_GetString(pStrings, ADD_TRACK_EVENT));
		while(pEv)
		{
			gpListBox->AddItem(pEv->GetString(), pEv);

			pEv = pEv->NextSibling();
		}
	}
	else
		gpComboBox->Show(MFTranslation_GetString(pStrings, ENTER_TRACK_EVENT), "", PlaceTrackEvent, ScanAndSortEvents);
}

void PlaceSection()
{
	GHEvent *pEv = gEditor.pSong->events.FindEvent(GHE_Event, gEditor.offset);

	while(pEv && MFString_CompareN(pEv->GetString(), "section ", 8))
	{
		pEv = pEv->NextSibling();
	}

	if(pEv)
	{
		gEditor.pSong->events.RemoveEvent(pEv);
	}
	else
	{
		gpStringBox->Show(MFTranslation_GetString(pStrings, ENTER_SECTION), "", PlaceSection);
	}
}

void CopyTrackData();

static int gTargetTrack = 0;
static bool gbNotes = true;
static bool gbSpecials = true;
static bool gbEvents = true;

void DoCopyTrackData()
{
	GHEventManager &sourceStream = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]];
	GHEventManager &targetStream = gEditor.pSong->notes[gTargetTrack];

	for(GHEvent *pEv = sourceStream.First(); pEv; pEv = pEv->Next())
	{
		if((gbNotes && pEv->event == GHE_Note) || (gbSpecials && pEv->event == GHE_Special))
		{
			targetStream.AddEvent(pEv->event, pEv->tick, pEv->key, pEv->parameter);
		}

		if(gbEvents && pEv->event == GHE_Event)
		{
			targetStream.AddStringEvent(pEv->event, pEv->tick, pEv->GetString());
		}
	}

	gEditor.pSong->CalculateNoteTimes(gTargetTrack, 0);
}

void TargetSelect(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel) return;

	gTargetTrack = listID;
	CopyTrackData();
}

void CopyDataCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel) return;

	switch(listID)
	{
		case 0:
		{
			gpListBox->Show(MFTranslation_GetString(pStrings, SELECT_TARGET_TRACK), TargetSelect);

			for(int a=0; a<GHS_NumDifficulties; ++a)
			{
				for(int b=0; b<GHS_NumTracks; ++b)
				{
					const char *pTrackName = MFStr("%s - %s", gEditor.pSong->GetDifficultyName(a), gEditor.pSong->GetTrackName(b));
					gpListBox->AddItem(pTrackName);
				}
			}
			break;
		}
		case 1:
			gbNotes = !gbNotes;
			CopyTrackData();
			break;
		case 2:
			gbSpecials = !gbSpecials;
			CopyTrackData();
			break;
		case 3:
			gbEvents = !gbEvents;
			CopyTrackData();
			break;
		case 4:
			DoCopyTrackData();
			break;
	}
}

void CopyTrackData()
{
	gpListBox->Show(MFTranslation_GetString(pStrings, COPY_TRACK_DATA), CopyDataCallback);

	int difficulty = gTargetTrack / GHS_NumTracks;
	int track = gTargetTrack % GHS_NumTracks;
	const char *pTargetTrack = MFStr("%s - %s", gEditor.pSong->GetDifficultyName(difficulty), gEditor.pSong->GetTrackName(track));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, TARGET_TRACK), pTargetTrack));

	const char *pYes = MFTranslation_GetString(pStrings, MENU_YES);
	const char *pNo = MFTranslation_GetString(pStrings, MENU_NO);
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, COPY_NOTES), gbNotes ? pYes : pNo));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, COPY_SPECIALS), gbSpecials ? pYes : pNo));
	gpListBox->AddItem(MFStr("%s: %s", MFTranslation_GetString(pStrings, COPY_EVENTS), gbEvents ? pYes : pNo));

	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_COPY));
}

void SelectDifficulty(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
		return;

	gEditor.currentStream[gEditor.selectedStream] = ((int&)pData * GHS_NumTracks) + (gEditor.currentStream[gEditor.selectedStream] % GHS_NumTracks);
}

void SelectTrack(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
		return;

	gEditor.currentStream[gEditor.selectedStream] = (gEditor.currentStream[gEditor.selectedStream] - (gEditor.currentStream[gEditor.selectedStream] % GHS_NumTracks)) + (int&)pData;
}

void SelectRefTrack(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
		return;

	int track = (int&)pData;
	if(track == -1)
	{
		gEditor.currentStream[1] = -1;
		gEditor.selectedStream = 0;
	}
	else
	{
		gEditor.currentStream[1] = (gEditor.currentStream[gEditor.selectedStream] - (gEditor.currentStream[gEditor.selectedStream] % GHS_NumTracks)) + track;
	}
}

void QuickMenuCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
		return;

	switch(listID)
	{
		case 0:
			bMetronome = !bMetronome;
			break;
		case 1:
			bClaps = !bClaps;
			break;
		case 2:
			PlaceEvent();
			break;
		case 3:
			PlaceTrackEvent();
			break;
		case 4:
			PlaceSection();
			break;
		case 5:
			gpListBox->Show(MFTranslation_GetString(pStrings, MENU_SELECT_DIFFICULTY), SelectDifficulty, 150.0f);
			for(int a=0; a<GHS_NumDifficulties; ++a)
				gpListBox->AddItem(gEditor.pSong->GetDifficultyName(a), (void*&)a);
			break;
		case 6:
			gpListBox->Show(MFTranslation_GetString(pStrings, SELECT_TRACK), SelectTrack, 200.0f, 190.0f);
			for(int a=0; a<GHS_NumTracks; ++a)
				gpListBox->AddItem(gEditor.pSong->GetTrackName(a), (void*&)a);
			break;
		case 7:
			gpListBox->Show(MFTranslation_GetString(pStrings, SHOW_REFERENCE), SelectRefTrack, 200.0f, 190.0f);
			gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_NONE), (void*)-1);
			for(int a=0; a<GHS_NumTracks; ++a)
				gpListBox->AddItem(gEditor.pSong->GetTrackName(a), (void*&)a);
			break;
		case 8:
			CopyTrackData();
			break;
		case 9:
			MenuScreen::ShowChartSettings(false);
			break;
		case 10:
			((EditorScreen*)dBScreen::GetCurrent())->gMixer.Push();
			break;
		case 11:
			gMenu.Push();
			break;
		case 12:
			gpHelp->Push();
			break;
	}
}

void RemoveEventCallback(int cancel, int listID, const char *pString, void *pData)
{
	if(cancel)
		return;

	size_t eventType = (size_t)pData;

	GHEventManager &notes = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]];

	GHEvent *pEvent = notes.GetNextEvent(gEditor.offset);
	while(pEvent && pEvent->tick == gEditor.offset)
	{
		if(pEvent->event == eventType)
			pEvent = notes.RemoveEvent(pEvent);
		else
			pEvent = pEvent->Next();
	}

	pEvent = gEditor.pSong->events.GetNextEvent(gEditor.offset);
	while(pEvent && pEvent->tick == gEditor.offset)
	{
		if(pEvent->event == eventType)
			pEvent = gEditor.pSong->events.RemoveEvent(pEvent);
		else
			pEvent = pEvent->Next();
	}

	pEvent = gEditor.pSong->sync.GetNextEvent(gEditor.offset);
	while(pEvent && pEvent->tick == gEditor.offset)
	{
		if(pEvent->event == eventType)
			pEvent = gEditor.pSong->sync.RemoveEvent(pEvent);
		else
			pEvent = pEvent->Next();
	}
}


void ShowPopupMenu()
{
#if defined(MF_PSP)
	float w = 300.f;
#else
	float w = 200.f;
#endif
	gpListBox->Show(MFTranslation_GetString(pStrings, MENU_QUICK_MENU), QuickMenuCallback, w, 198.0f);
	gpListBox->AddItem(MFStr("%s %s", MFTranslation_GetString(pStrings, bMetronome ? MENU_DISABLE : MENU_ENABLE), MFTranslation_GetString(pStrings, MENU_METRONOME)));
	gpListBox->AddItem(MFStr("%s %s", MFTranslation_GetString(pStrings, bClaps ? MENU_DISABLE : MENU_ENABLE), MFTranslation_GetString(pStrings, MENU_CLAPS)));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_PLACE_EVENT));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_PLACE_TRACK_EVENT));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_PLACE_SECTION));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_SELECT_DIFFICULTY));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, SELECT_TRACK));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, SHOW_REFERENCE));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_COPY_CHART));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_CHART_SETTINGS));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_VOLUME));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_MAIN_MENU));
	gpListBox->AddItem(MFTranslation_GetString(pStrings, MENU_HELP));
}

void Editor::Update()
{
}

void UpdateEditor();

void Editor::UpdateInput()
{
	GHScreen::UpdateInput();

	// check for main menu
	if(TestControl(dBCtrl_Edit_Menu, GHCT_Once))
		gMenu.Push();

	if(TestControl(dBCtrl_Edit_PopupMenu, GHCT_Once))
		ShowPopupMenu();

	// check for section jump
	if(TestControl(dBCtrl_Edit_Goto, GHCT_Once))
		gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_GOTO), "", GotoSection);

	// update the editor.
	UpdateEditor();

	// enable/disable half time
	if(TestControl(dBCtrl_Edit_PlaybackRate, GHCT_Once))
	{
		bHalfSpeed.data = (bHalfSpeed.data + 1) % (int)(sizeof(gSpeeds) / sizeof(gSpeeds[0]));

		if(gEditor.pSong->pStream)
			MFSound_SetPlaybackRate(MFSound_GetStreamVoice(gEditor.pSong->pStream), gSpeeds[bHalfSpeed]);
		if(gEditor.pSong->pGuitar)
			MFSound_SetPlaybackRate(MFSound_GetStreamVoice(gEditor.pSong->pGuitar), gSpeeds[bHalfSpeed]);
		if(gEditor.pSong->pBass)
			MFSound_SetPlaybackRate(MFSound_GetStreamVoice(gEditor.pSong->pBass), gSpeeds[bHalfSpeed]);
	}

	// swap modes
	if(TestControl(dBCtrl_Edit_StartPlayback, GHCT_Once) || TestControl(dBCtrl_Edit_RestartPlayback, GHCT_Once))
	{
		gEditor.state = GHPS_Playing;

		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], 0);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], 0);

		if(TestControl(dBCtrl_Edit_RestartPlayback, GHCT_Once))
		{
			gEditor.playbackStartOffset = 0;
			gEditor.playingTime = 0;
		}
		else
		{
			gEditor.playbackStartOffset = gEditor.offset;
			gEditor.playingTime = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);
		}

		gEditor.pSong->Play(gEditor.playingTime);

		gPlay.scoreKeeper[0].Begin(gEditor.pSong, (GHStreams)gEditor.currentStream[0], 0, gEditor.playingTime);
		if(gEditor.currentStream[1] != -1)
			gPlay.scoreKeeper[1].Begin(gEditor.pSong, (GHStreams)gEditor.currentStream[1], 1, gEditor.playingTime);

		if(gEditor.pSong->pStream || gEditor.pSong->pGuitar || gEditor.pSong->pBass)
		{
			gEditor.playStart = 0;
			gEditor.lastTimestamp = 0;
		}
		else
		{
			gEditor.playStart = MFSystem_ReadRTC();
		}
		gEditor.startTime = gEditor.playingTime;

		bSelecting = false;

		Pop();
		gPlay.Push();
	}
}

void UpdateEditor()
{
	static GHEvent *pHold[8] = { (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1, (GHEvent*)(size_t)-1 };

	bool ctrlState = MFInput_Read(Key_LControl, IDD_Keyboard) || MFInput_Read(Key_RControl, IDD_Keyboard);
	bool shiftState = MFInput_Read(Key_LShift, IDD_Keyboard) || MFInput_Read(Key_LShift, IDD_Keyboard);
	bool altState = MFInput_Read(Key_LAlt, IDD_Keyboard) || MFInput_Read(Key_RAlt, IDD_Keyboard);

	int res = gEditor.pSong->GetRes();

	if(TestControl(dBCtrl_Edit_Save, GHCT_Once))
	{
		gEditor.pSong->SaveChart();
		MFVoice *pVoice = MFSound_Play(gEditor.pSaveSound, MFPF_BeginPaused);
		MFSound_SetVolume(pVoice, gConfig.sound.fxLevel);
		MFSound_Pause(pVoice, false);

		if(gConfig.editor.saveAction[0])
			system(gConfig.editor.saveAction);
	}

	if(TestControl(dBCtrl_Edit_Event, GHCT_Once))
		PlaceEvent();
	else if(TestControl(dBCtrl_Edit_TrackEvent, GHCT_Once))
		PlaceTrackEvent();
	else if(TestControl(dBCtrl_Edit_Section, GHCT_Once))
		PlaceSection();
	else if(TestControl(dBCtrl_Edit_Quantise, GHCT_Once))
		gpStringBox->Show(MFTranslation_GetString(pStrings, MENU_SETQUANTISE), "", SetCustomQuantisation);
	else if(TestControl(dBCtrl_Edit_Mixer, GHCT_Once))
		((EditorScreen*)dBScreen::GetCurrent())->gMixer.Push();

	// check selection
	if(bSelecting && !TestControl(dBCtrl_Edit_RangeSelect, GHCT_Hold))
	{
		gEditor.selectEnd = gEditor.offset;
		bSelecting = false;
	}
	else if(!bSelecting && TestControl(dBCtrl_Edit_RangeSelect, GHCT_Hold))
	{
		gEditor.selectStart = gEditor.selectEnd = gEditor.offset;
		bSelecting = true;
	}

	if(TestControl(dBCtrl_Edit_Cut, GHCT_Once) || TestControl(dBCtrl_Edit_Copy, GHCT_Once))
	{
		if(gEditor.selectStart != gEditor.selectEnd)
		{
			gEditor.copyLen = gEditor.selectEnd - gEditor.selectStart;

			gEditor.selectEvents.Clear();

			GHEventManager &noteStream = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]];
			GHEvent *pEv = noteStream.GetNextEvent(gEditor.selectStart);

			while(pEv && pEv->tick < gEditor.selectEnd)
			{
				// copy the next pointer incase we cut the note.
				GHEvent *pNext = pEv->Next();

				// just cut/copy notes
				if(pEv->event == GHE_Note)
				{
					// copy to clipboard
					gEditor.selectEvents.AddEvent(pEv->event, pEv->tick - gEditor.selectStart, pEv->key, pEv->parameter);

					// if we cut
					if(TestControl(dBCtrl_Edit_Cut, GHCT_Once))
						pNext = noteStream.RemoveEvent(pEv);
				}

				pEv = pNext;
			}
		}
	}
	else if(TestControl(dBCtrl_Edit_Paste, GHCT_Once))
	{
		GHEvent *pEv = gEditor.selectEvents.First();

		if(pEv)
		{
			int curStream = gEditor.currentStream[gEditor.selectedStream];
			GHEventManager &noteStream = gEditor.pSong->notes[curStream];

			// delete notes in paste range
			GHEvent *pDel = noteStream.GetNextEvent(gEditor.offset);
			while(pDel && pDel->tick < gEditor.offset + gEditor.copyLen)
			{
				if(pDel->event == GHE_Note)
					pDel = noteStream.RemoveEvent(pDel);
				else
					pDel = pDel->Next();
			}

			// paste notes
			while(pEv)
			{
				noteStream.AddEvent(pEv->event, pEv->tick + gEditor.offset, pEv->key, pEv->parameter);
				pEv = pEv->Next();
			}

			gEditor.pSong->CalculateNoteTimes(curStream, gEditor.offset);
		}
	}

	if(TestControl(dBCtrl_Edit_Delete, GHCT_Once))
	{
		GHEventManager &notes = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]];
		if(gEditor.selectStart != gEditor.selectEnd)
		{
			// delete notes in selected range
			GHEvent *pDel = notes.GetNextEvent(gEditor.selectStart);
			while(pDel && pDel->tick < gEditor.selectEnd)
			{
				if(pDel->event == GHE_Note)
					pDel = notes.RemoveEvent(pDel);
				else
					pDel = pDel->Next();
			}
		}
		else
		{
			int numEvents = 0;
			uint32 eventTypes = 0;

			// find note events
			GHEvent *pEvent = notes.GetNextEvent(gEditor.offset);
			while(pEvent && pEvent->tick == gEditor.offset)
			{
				uint32 bit = 1 << pEvent->event;
				if(!(eventTypes & bit))
				{
					++numEvents;
					eventTypes |= bit;
				}
				pEvent = pEvent->Next();
			}

			// find sync events
			if(gEditor.offset > 0)
			{
				pEvent = gEditor.pSong->sync.GetNextEvent(gEditor.offset);
				while(pEvent && pEvent->tick == gEditor.offset)
				{
					uint32 bit = 1 << pEvent->event;
					if(!(eventTypes & bit))
					{
						++numEvents;
						eventTypes |= bit;
					}
					pEvent = pEvent->Next();
				}
			}

			// find global events
			pEvent = gEditor.pSong->events.GetNextEvent(gEditor.offset);
			while(pEvent && pEvent->tick == gEditor.offset)
			{
				uint32 bit = 1 << pEvent->event;
				if(!(eventTypes & bit))
				{
					++numEvents;
					eventTypes |= bit;
				}
				pEvent = pEvent->Next();
			}

			// if there are multiple event types to remove, show a list box
			if(numEvents > 1)
				gpListBox->Show(MFTranslation_GetString(pStrings, SELECT_EVENT_REMOVE), RemoveEventCallback, 200.0f, 100.0f);

			for(int a=0; a<GHE_Max; ++a)
			{
				if(eventTypes & (1 << a))
				{
					if(numEvents > 1)
						gpListBox->AddItem(MFTranslation_GetString(pStrings, EVENT_TYPE_UNKNOWN+a), (void*)(size_t)a);
					else
						RemoveEventCallback(false, 0, NULL, (void*)(size_t)a);
				}
			}
		}
	}

	// shift notes left or right
	int selStart, selEnd;

	if(gEditor.selectStart != gEditor.selectEnd)
	{
		selStart = gEditor.selectStart;
		selEnd = gEditor.selectEnd;
	}
	else
	{
		selStart = gEditor.offset;
		selEnd = gEditor.pSong->GetLastNoteTick();
	}

	if(TestControl(dBCtrl_Edit_ShiftForwards, GHCT_Delay))
	{
		int offset = 4*res / gEditor.quantisation;

		// TODO: gotta remove notes after the selection that will be overwritten as the selection shifts..

		if(altState)
		{
			// shift events, sync and other tracks too
			GHEvent *pEv = gEditor.pSong->sync.GetNextEvent(selStart);
			while(pEv && pEv->tick < selEnd)
			{
				if(pEv->tick != 0)
					pEv->tick += offset;
				pEv = pEv->Next();
			}

			pEv = gEditor.pSong->events.GetNextEvent(selStart);
			while(pEv && pEv->tick < selEnd)
			{
				pEv->tick += offset;
				pEv = pEv->Next();
			}

			for(int i=0; i<GHS_Max; ++i)
			{
				pEv = gEditor.pSong->notes[i].GetNextEvent(selStart);
				while(pEv && pEv->tick < selEnd)
				{
					pEv->tick += offset;
					pEv = pEv->Next();
				}
			}
		}
		else
		{
			GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].GetNextEvent(selStart);
			while(pEv && pEv->tick < selEnd)
			{
				pEv->tick += offset;
				pEv = pEv->Next();
			}
		}

		gEditor.selectStart += offset;
		gEditor.selectEnd += offset;
		gEditor.offset += offset;

		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[gEditor.selectedStream], gEditor.offset);
	}
	if(TestControl(dBCtrl_Edit_ShiftBackwards, GHCT_Delay))
	{
		int offset = MFMin(4*res / gEditor.quantisation, gEditor.selectStart);

		// TODO: gotta remove notes before the selection that will be overwritten as the selection shifts..

		if(altState)
		{
			// shift events, sync and other tracks too
			GHEvent *pEv = gEditor.pSong->sync.GetNextEvent(selStart);
			while(pEv && pEv->tick < selEnd)
			{
				if(pEv->tick != 0)
					pEv->tick -= offset;
				pEv = pEv->Next();
			}

			pEv = gEditor.pSong->events.GetNextEvent(selStart);
			while(pEv && pEv->tick < selEnd)
			{
				pEv->tick -= offset;
				pEv = pEv->Next();
			}

			for(int i=0; i<GHS_Max; ++i)
			{
				pEv = gEditor.pSong->notes[i].GetNextEvent(selStart);
				while(pEv && pEv->tick < selEnd)
				{
					pEv->tick -= offset;
					pEv = pEv->Next();
				}
			}
		}
		else
		{
			GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].GetNextEvent(selStart);
			while(pEv && pEv->tick < selEnd)
			{
				pEv->tick -= offset;
				pEv = pEv->Next();
			}
		}

		gEditor.selectStart -= offset;
		gEditor.selectEnd -= offset;
		gEditor.offset -= offset;

		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[gEditor.selectedStream], gEditor.offset);
	}
	if(TestControl(dBCtrl_Edit_ShiftLeft, GHCT_Once))
	{
		GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].GetNextEvent(selStart);

		while(pEv && pEv->tick < selEnd)
		{
			if(pEv->event == GHE_Note)
				pEv->key = MFMax(0, pEv->key - 1);

			pEv = pEv->Next();
		}
	}
	if(TestControl(dBCtrl_Edit_ShiftRight, GHCT_Once))
	{
		GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].GetNextEvent(selStart);

		while(pEv && pEv->tick < selEnd)
		{
			if(pEv->event == GHE_Note)
				pEv->key = MFMin(4, pEv->key + 1);

			pEv = pEv->Next();
		}
	}

	// change quantisation
	if(TestControl(dBCtrl_Edit_QuantiseDown, GHCT_Delay))
	{
		gEditor.quantiseStep = MFMax(0, gEditor.quantiseStep-1);
		gEditor.quantisation = gQuantiseSteps[gEditor.quantiseStep];
		OffsetToMeasureAndBeat(gEditor.offset, &gEditor.measure, &gEditor.beat);

		MFVoice *pVoice = MFSound_Play(gEditor.pChangeSound, MFPF_BeginPaused);
		MFSound_SetVolume(pVoice, gConfig.sound.fxLevel);
		MFSound_Pause(pVoice, false);
	}
	if(TestControl(dBCtrl_Edit_QuantiseUp, GHCT_Delay))
	{
		gEditor.quantiseStep = MFMin((int)(sizeof(gQuantiseSteps)/sizeof(gQuantiseSteps[0]))-1, gEditor.quantiseStep+1);
		gEditor.quantisation = gQuantiseSteps[gEditor.quantiseStep];
		OffsetToMeasureAndBeat(gEditor.offset, &gEditor.measure, &gEditor.beat);

		MFVoice *pVoice = MFSound_Play(gEditor.pChangeSound, MFPF_BeginPaused);
		MFSound_SetVolume(pVoice, gConfig.sound.fxLevel);
		MFSound_Pause(pVoice, false);
	}

	// move the track
	if(TestControl(dBCtrl_Edit_Forward, GHCT_Delay))
	{
		// forward one step
		++gEditor.beat;
		if(gEditor.beat == gEditor.quantisation)
		{
			++gEditor.measure;
			gEditor.beat = 0;
		}
	}
	if(TestControl(dBCtrl_Edit_Back, GHCT_Delay))
	{
		// back one step
		if(gEditor.measure || gEditor.beat)
		{
			--gEditor.beat;
			if(gEditor.beat < 0)
			{
				--gEditor.measure;
				gEditor.beat += gEditor.quantisation;
			}
		}
	}
	if(TestControl(dBCtrl_Edit_Start, GHCT_Once))
	{
		// go to start
		gEditor.measure = gEditor.beat = 0;
	}
	if(TestControl(dBCtrl_Edit_End, GHCT_Once))
	{
		// go to the last note...
		OffsetToMeasureAndBeat(gEditor.pSong->GetLastNoteTick(), &gEditor.measure, &gEditor.beat);
	}
	if(TestControl(dBCtrl_Edit_UpMeasure, GHCT_Delay))
	{
		// forward one measure
		// TODO: consider bar lengths while moving
		++gEditor.measure;
	}
	if(TestControl(dBCtrl_Edit_DownMeasure, GHCT_Delay))
	{
		// back one measure
		// TODO: consider bar lengths while moving
		if(gEditor.measure < 1)
			gEditor.measure = gEditor.beat = 0;
		else
			--gEditor.measure;
	}
	if(TestControl(dBCtrl_Edit_NextSection, GHCT_Delay))
	{
		GHEvent *pEv = gEditor.pSong->events.GetNextEvent(gEditor.offset);

		while(pEv)
		{
			if(pEv->tick >= gEditor.offset + gEditor.pSong->resolution*4 / gEditor.quantisation && !MFString_CompareN(pEv->GetString(), "section ", 8))
			{
				OffsetToMeasureAndBeat(pEv->tick, &gEditor.measure, &gEditor.beat);
				break;
			}

			pEv = pEv->Next();
		}

		if(!pEv)
			OffsetToMeasureAndBeat(gEditor.pSong->GetLastNoteTick(), &gEditor.measure, &gEditor.beat);
	}
	if(TestControl(dBCtrl_Edit_PrevSection, GHCT_Delay))
	{
		GHEvent *pMostRecent = NULL;
		GHEvent *pEv = gEditor.pSong->events.First();

		while(pEv && pEv->tick < gEditor.offset)
		{
			if(!MFString_CompareN(pEv->GetString(), "section ", 8))
				pMostRecent = pEv;

			pEv = pEv->Next();
		}

		if(pMostRecent)
			OffsetToMeasureAndBeat(pMostRecent->tick, &gEditor.measure, &gEditor.beat);
		else
			gEditor.measure = gEditor.beat = 0;
	}

	int newOffset = MeasureAndBeatToOffset(gEditor.measure, gEditor.beat);
	int shift = newOffset - gEditor.offset;
	shift = MFMax(gEditor.offset + shift, 0) - gEditor.offset;

	if(shift)
	{
		// update BPM if applicable
		int shiftStart, shiftEnd;

		if(shift > 0)
		{
			shiftStart = gEditor.offset;
			shiftEnd = gEditor.offset+shift + 1;
		}
		else
		{
			shiftStart = 0;
			shiftEnd = gEditor.offset+shift + 1;
		}

		gEditor.offset += shift;

		if(bSelecting)
			gEditor.selectEnd = gEditor.offset;

		GHEvent *pEv = gEditor.pSong->sync.GetNextEvent(shiftStart);

		while(pEv && pEv->tick < shiftEnd)
		{
			if(pEv->event == GHE_BPM || pEv->event == GHE_Anchor)
				gEditor.currentBPM = pEv->parameter;
			else if(pEv->event == GHE_TimeSignature)
			{
				gEditor.currentTimeSignature = pEv->parameter;
				gEditor.lastTimeSignature = pEv->tick;
			}

			pEv = pEv->Next();
		}

		gEditor.playingTime = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);

		MFVoice *pVoice = MFSound_Play(gEditor.pStepSound, MFPF_BeginPaused);
		if(pVoice)
		{
			MFSound_SetVolume(pVoice, gConfig.sound.fxLevel);
			MFSound_Pause(pVoice, false);
		}
	}

	// increase/decrease BPM
	if(gEditor.currentBPM > 1000 && TestControl(dBCtrl_Edit_DecreaseBPM, GHCT_Delay))
	{
		// reduce BPM
		int inc = 1000;

		if(shiftState)
			inc /= 10;
		if(ctrlState)
			inc /= 100;
		if(altState)
			inc *= 10;

		GHEvent *pEv = gEditor.pSong->sync.FindEvent(GHE_BPM, gEditor.offset, 0);
		if(!pEv) pEv = gEditor.pSong->sync.FindEvent(GHE_Anchor, gEditor.offset, 0);

		if(!pEv)
			pEv = gEditor.pSong->sync.AddEvent(GHE_BPM, gEditor.offset, 0, MFMax(gEditor.currentBPM - inc, 1000));
		else
			pEv->parameter = MFMax(pEv->parameter - inc, 1000);

		gEditor.currentBPM = pEv->parameter;

		// remove this BPM marker if its the same as the previous one..
		if(pEv->event != GHE_Anchor)
		{
			GHEvent *pPrev = gEditor.pSong->sync.GetMostRecentEvent(GHE_BPM, gEditor.offset);
			if(pPrev && pPrev->parameter == pEv->parameter)
				gEditor.pSong->sync.RemoveEvent(pEv);
		}

		// recalculate the note times from this point on
		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], gEditor.offset);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], gEditor.offset);
	}

	if(gEditor.currentBPM < 9999000 && TestControl(dBCtrl_Edit_IncreaseBPM, GHCT_Delay))
	{
		// increase BPM
		int inc = 1000;

		if(shiftState)
			inc /= 10;
		if(ctrlState)
			inc /= 100;
		if(altState)
			inc *= 10;

		GHEvent *pEv = gEditor.pSong->sync.FindEvent(GHE_BPM, gEditor.offset, 0);
		if(!pEv) pEv = gEditor.pSong->sync.FindEvent(GHE_Anchor, gEditor.offset, 0);

		if(!pEv)
			pEv = gEditor.pSong->sync.AddEvent(GHE_BPM, gEditor.offset, 0, MFMin(gEditor.currentBPM + inc, 9999000));
		else
			pEv->parameter = MFMin(pEv->parameter + inc, 9999000);

		gEditor.currentBPM = pEv->parameter;

		// remove this BPM marker if its the same as the previous one..
		if(pEv->event != GHE_Anchor)
		{
			GHEvent *pPrev = gEditor.pSong->sync.GetMostRecentEvent(GHE_BPM, gEditor.offset);
			if(pEv->event != GHE_Anchor && pPrev && pPrev->parameter == pEv->parameter)
				gEditor.pSong->sync.RemoveEvent(pEv);
		}

		// recalculate the note times from this point on
		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], gEditor.offset);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], gEditor.offset);
	}

	// place anchor
	if(TestControl(dBCtrl_Edit_Anchor, GHCT_Once) && gEditor.offset > 0)
	{
		GHEvent *pEv = gEditor.pSong->sync.FindEvent(GHE_BPM, gEditor.offset, 0);

		if(pEv)
		{
			pEv->event = GHE_Anchor;
			pEv->time = gEditor.pSong->CalculateTimeOfTick(pEv->tick);
		}
		else
		{
			pEv = gEditor.pSong->sync.FindEvent(GHE_Anchor, gEditor.offset, 0);

			if(!pEv)
			{
				pEv = gEditor.pSong->sync.AddEvent(GHE_Anchor, gEditor.offset, 0, gEditor.currentBPM);
				pEv->time = gEditor.pSong->CalculateTimeOfTick(pEv->tick);
			}
			else
			{
				GHEvent *pLast = gEditor.pSong->sync.GetMostRecentSyncEvent(pEv->tick);

				if(pLast && pLast->parameter == pEv->parameter)
					gEditor.pSong->sync.RemoveEvent(pEv);
				else
					pEv->event = GHE_BPM;
			}
		}

		// recalculate the note times
		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], 0);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], 0);
	}

	// change time signature
	if(TestControl(dBCtrl_Edit_DecreaseTS, GHCT_Delay) && gEditor.currentTimeSignature > 1)
	{
		int tsTime = gEditor.offset - ((gEditor.offset - gEditor.lastTimeSignature) % (res*gEditor.currentTimeSignature));

		GHEvent *pEv = gEditor.pSong->sync.FindEvent(GHE_TimeSignature, tsTime, 0);

		if(!pEv)
			pEv = gEditor.pSong->sync.AddEvent(GHE_TimeSignature, tsTime, 0, gEditor.currentTimeSignature - 1);
		else
			--pEv->parameter;

		gEditor.currentTimeSignature = pEv->parameter;
		gEditor.lastTimeSignature = tsTime;

		// remove this BPM marker if its the same as the previous one..
		GHEvent *pPrev = gEditor.pSong->sync.GetMostRecentEvent(GHE_TimeSignature, tsTime);
		if(pPrev && pPrev->parameter == pEv->parameter)
		{
			gEditor.lastTimeSignature = pPrev->tick;
			gEditor.pSong->sync.RemoveEvent(pEv);
		}

		// recalculate the note times from this point on
		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], tsTime);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], tsTime);
	}
	else if(TestControl(dBCtrl_Edit_IncreaseTS, GHCT_Delay) && gEditor.currentTimeSignature < 99)
	{
		int tsTime = gEditor.offset - ((gEditor.offset - gEditor.lastTimeSignature) % (res*gEditor.currentTimeSignature));

		GHEvent *pEv = gEditor.pSong->sync.FindEvent(GHE_TimeSignature, tsTime, 0);

		if(!pEv)
			pEv = gEditor.pSong->sync.AddEvent(GHE_TimeSignature, tsTime, 0, gEditor.currentTimeSignature + 1);
		else
			++pEv->parameter;

		gEditor.currentTimeSignature = pEv->parameter;
		gEditor.lastTimeSignature = tsTime;

		// remove this BPM marker if its the same as the previous one..
		GHEvent *pPrev = gEditor.pSong->sync.GetMostRecentEvent(GHE_TimeSignature, tsTime);
		if(pPrev && pPrev->parameter == pEv->parameter)
		{
			gEditor.lastTimeSignature = pPrev->tick;
			gEditor.pSong->sync.RemoveEvent(pEv);
		}

		// recalculate the note times from this point on
		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], tsTime);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], tsTime);
	}

	// add/remove notes
	GHEventManager &noteStream = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]];
	dBControlType keys_righty[] = { dBCtrl_Edit_Note0, dBCtrl_Edit_Note1, dBCtrl_Edit_Note2, dBCtrl_Edit_Note3, dBCtrl_Edit_Note4, dBCtrl_Edit_PS1, dBCtrl_Edit_PS2, dBCtrl_Edit_Note5 };
	dBControlType keys_lefty[] = { dBCtrl_Edit_Note4, dBCtrl_Edit_Note3, dBCtrl_Edit_Note2, dBCtrl_Edit_Note1, dBCtrl_Edit_Note0, dBCtrl_Edit_PS1, dBCtrl_Edit_PS2, dBCtrl_Edit_Note5 };
	dBControlType *keys = gConfig.controls.leftyFlip[0] ? keys_lefty : keys_righty;

	for(int a=0; a<8; a++)
	{
		GHEventType ev = a < 5 ? GHE_Note : GHE_Special;
		int key = a < 5 ? GHEK_Green + a : GHS_Player1 + (a - 5);

		if(TestControl(keys[a], GHCT_Hold))
		{
			if(pHold[a])
			{
				if(pHold[a] != (GHEvent*)(size_t)0xFFFFFFFF)
				{
					pHold[a]->parameter = MFMax(gEditor.offset - pHold[a]->tick, 0);
				}
			}
			else
			{
				GHEvent *pEv = noteStream.FindEvent(ev, gEditor.offset, key);

				if(pEv)
				{
					noteStream.RemoveEvent(pEv);
					pHold[a] = (GHEvent*)(size_t)0xFFFFFFFF;
					for(int i=0; i<8; ++i)
					{
						if(pHold[i] && pHold[i] != (GHEvent*)(size_t)0xFFFFFFFF)
						{
							if(pHold[i] > pEv)
								--pHold[i];
						}
					}
				}
				else
				{
					// check if we are intersecting a hold note
					pEv = noteStream.GetMostRecentEvent(GHE_Note, gEditor.offset);

					if(pEv && pEv->parameter > gEditor.offset - pEv->tick)
					{
						// the last note was a hold note, we'll cut it short...
						do
						{
							pEv->parameter = gEditor.offset - pEv->tick;
							pEv = pEv->Prev();
						}
						while(pEv && pEv->tick == pEv->Next()->tick);
					}

					pEv = noteStream.AddEvent(ev, gEditor.offset, key);
					pEv->time = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);
					pHold[a] = pEv;
				}
			}
		}
		else
		{
			if(a<5)
			{
				// check if we have just released a hold note
				if(pHold[a] && pHold[a] != (GHEvent*)(size_t)0xFFFFFFFF && pHold[a]->parameter != 0)
				{
					// remove any other notes within the hold range
					GHEvent *pEv = gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].GetNextEvent(pHold[a]->tick);

					while(pEv && pEv->tick < pHold[a]->tick+pHold[a]->parameter)
					{
						GHEvent *pNext = pEv->Next();

						if(pEv->event == GHE_Note)
						{
							// and make sure we dont remove chords
							if(pHold[a]->tick != pEv->tick || pHold[a]->parameter != pEv->parameter)
							{
								pNext = noteStream.RemoveEvent(pEv);
								for(int i=0; i<8; ++i)
								{
									if(pHold[i] && pHold[i] != (GHEvent*)(size_t)0xFFFFFFFF)
									{
										if(pHold[i] > pEv)
											--pHold[i];
									}
								}
							}
						}

						pEv = pNext;
					}
				}
			}
			else
			{
				// remove zero length special events...
				if(pHold[a] && pHold[a] != (GHEvent*)(size_t)0xFFFFFFFF && pHold[a]->parameter == 0)
				{
					noteStream.RemoveEvent(pHold[a]);
				}
			}

			pHold[a] = NULL;
		}
	}
}
