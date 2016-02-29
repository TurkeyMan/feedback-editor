#include "FeedBack.h"
#include "Fuji/MFDisplay.h"
#include "Fuji/MFRenderer.h"
#include "Fuji/DebugMenu.h"

#include "Control.h"
#include "Screens/Editor.h"

const char * gSpeedStrings[] = { "1x", "0.85x", "0.75x", "0.5x" };
const char * gViewPoints[3];

MenuItemBool bMetronome(false);
MenuItemBool bClaps(false);
MenuItemIntString bHalfSpeed(gSpeedStrings, 0);

float gHyperSpeedTable[] = { 1.0f, 1.25f, 1.5f, 2.0f };
int gHyperSpeed = 0;

GameScreen::GameScreen()
{
}

GameScreen::~GameScreen()
{
}

void GameScreen::Update()
{

}

void GameScreen::UpdateInput()
{
	MFCALLSTACKc;

	//show help window
	if(TestControl(dBCtrl_Edit_Help, GHCT_Once))
		gpHelp->Push();

	// enable/disable metronome
	if(TestControl(dBCtrl_Edit_Metronome, GHCT_Once))
		bMetronome = !bMetronome;

	// enable/disable note clap
	if(TestControl(dBCtrl_Edit_Clap, GHCT_Once))
		bClaps = !bClaps;

	// hyperspeed
	if(TestControl(dBCtrl_Edit_Hyperspeed, GHCT_Once))
	{
		gHyperSpeed = (gHyperSpeed+1) % (sizeof(gHyperSpeedTable)/sizeof(gHyperSpeedTable[0]));
		pTrack->SetScrollSpeed(12.0f * gHyperSpeedTable[gHyperSpeed]);
		pTrack2->SetScrollSpeed(12.0f * gHyperSpeedTable[gHyperSpeed]);
	}

	// swap point of view
	if(TestControl(dBCtrl_Edit_View, GHCT_Once))
	{
		if(gEditor.selectedStream == 0)
			gGame.pTrack->SetViewPoint((gGame.pTrack->GetViewPoint() + 1) % (sizeof(gViewPoints)/sizeof(gViewPoints[0])));
		else
			gGame.pTrack2->SetViewPoint((gGame.pTrack2->GetViewPoint() + 1) % (sizeof(gViewPoints)/sizeof(gViewPoints[0])));
	}

	bool ctrlState = MFInput_Read(Key_LControl, IDD_Keyboard) || MFInput_Read(Key_RControl, IDD_Keyboard);
	bool shiftState = MFInput_Read(Key_LShift, IDD_Keyboard) || MFInput_Read(Key_LShift, IDD_Keyboard);
	bool altState = MFInput_Read(Key_LAlt, IDD_Keyboard) || MFInput_Read(Key_RAlt, IDD_Keyboard);

	if(TestControl(dBCtrl_Edit_DecreaseOffset, GHCT_Delay))
	{
		// reduce BPM
		int64 inc = 1000000;

		if(shiftState)
			inc /= 10;
		if(ctrlState)
			inc /= 100;
		if(altState)
			inc *= 10;

		int64 oldOffset = gEditor.pSong->startOffset;

		GHEvent *pAnchor = NULL;

		GHEvent *pEv = gEditor.pSong->sync.First();
		while(pEv && pEv->tick <= gEditor.offset)
		{
			if(pEv->event == GHE_Anchor)
				pAnchor = pEv;
			pEv = pEv->Next();
		}

		if(!pAnchor)
		{
			gEditor.pSong->startOffset = MFMax(gEditor.pSong->startOffset - inc, 0LL);

			// update anchors
			GHEvent *pEv = gEditor.pSong->sync.First();
			while(pEv)
			{
				if(pEv->event == GHE_Anchor)
					pEv->time = pEv->time - oldOffset + gEditor.pSong->startOffset;
				pEv = pEv->Next();
			}
		}
		else
		{
			// needs to be limites past the previous anchor
			pEv = gEditor.pSong->sync.GetMostRecentEvent(GHE_Anchor, gEditor.offset);
			int64 shift = pEv && pAnchor->time - inc > pEv->time+50000 ? -inc : (pAnchor->time - inc >= gEditor.pSong->startOffset+50000 ? -inc : 0);
			pAnchor->time += shift;
		}

		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], 0);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], 0);

		gEditor.playingTime = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);
	}
	if(TestControl(dBCtrl_Edit_IncreaseOffset, GHCT_Delay))
	{
		// increase BPM
		int64 inc = 1000000;

		if(shiftState)
			inc /= 10;
		if(ctrlState)
			inc /= 100;
		if(altState)
			inc *= 10;

		int64 oldOffset = gEditor.pSong->startOffset;

		GHEvent *pAnchor = NULL;

		GHEvent *pEv = gEditor.pSong->sync.First();
		while(pEv && pEv->tick <= gEditor.offset)
		{
			if(pEv->event == GHE_Anchor)
				pAnchor = pEv;
			pEv = pEv->Next();
		}

		if(!pAnchor)
		{
			gEditor.pSong->startOffset += inc;

			// update anchors
			GHEvent *pEv = gEditor.pSong->sync.First();
			while(pEv)
			{
				if(pEv->event == GHE_Anchor)
					pEv->time = pEv->time - oldOffset + gEditor.pSong->startOffset;
				pEv = pEv->Next();
			}
		}
		else
		{
			// needs to be limited before the next anchor
			pEv = gEditor.pSong->sync.GetNextEvent(gEditor.offset);
			while(pEv && (pEv->tick == gEditor.offset || pEv->event != GHE_Anchor))
				pEv = pEv->Next();
			if(pEv)
			{
				GHEvent *pPE = gEditor.pSong->sync.GetMostRecentSyncEvent(pEv->tick);
				if(pPE->time + inc >= pEv->time - 50000)
					inc = 0;
			}

			pAnchor->time += inc;
		}

		gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[0], 0);
		if(gEditor.currentStream[1] != -1)
			gEditor.pSong->CalculateNoteTimes(gEditor.currentStream[1], 0);

		gEditor.playingTime = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);
	}

	if(gEditor.currentStream[1] != -1 && TestControl(dBCtrl_Edit_SwapTracks, GHCT_Once))
	{
		gEditor.selectedStream = 1 - gEditor.selectedStream;
	}

	for(int a=0; a<GHS_NumDifficulties; ++a)
	{
		if(TestControl((dBControlType)(dBCtrl_Edit_Easy + a), GHCT_Once))
		{
			gEditor.currentStream[gEditor.selectedStream] = (a * GHS_NumTracks) + (gEditor.currentStream[gEditor.selectedStream] % GHS_NumTracks);
		}
	}

	for(int a=0; a<GHS_NumTracks; ++a)
	{
		if(TestControl((dBControlType)(dBCtrl_Edit_Track1 + a), GHCT_Once))
		{
			gEditor.currentStream[gEditor.selectedStream] = (gEditor.currentStream[gEditor.selectedStream] - (gEditor.currentStream[gEditor.selectedStream] % GHS_NumTracks)) + a;
		}
	}
}


void GameScreen::Draw()
{
	MFCALLSTACKc;

	// Setup view into 3D mode.
	MFView_Push();

	MFRect rect;
	MFDisplay_GetDisplayRect(&rect);

	MFView_ConfigureProjection(MFDEGREES(45.0f), 1.0f, 100.0f);

	float currentTime;
	if(gEditor.state != GHPS_Stopped)
		currentTime = GETSECONDS(gEditor.playingTime);
	else
		currentTime = GETSECONDS(gEditor.pSong->CalculateTimeOfTick(gEditor.offset));

	MFView_SetOrtho(&rect);

	if(gEditor.currentStream[1] == -1)
	{
		pTrack->Draw(currentTime, gEditor.pSong, gEditor.currentStream[0]);

		int difficulty = gEditor.currentStream[0] / GHS_NumTracks;
		int track = gEditor.currentStream[0] % GHS_NumTracks;
		CenterText(10.0f, 60.0f, MFVector::red, MFStr("%s - %s", gEditor.pSong->GetDifficultyName(difficulty), gEditor.pSong->GetTrackName(track)), pHeading, false);
	}
	else
	{
		// set viewport for 2 halves..
		rect.width *= 0.5f;

		MFView_Push();
		MFView_SetOrtho(&rect);

		MFView_SetViewport(&rect);
		pTrack->Draw(currentTime, gEditor.pSong, gEditor.currentStream[0]);

		int difficulty = gEditor.currentStream[0] / GHS_NumTracks;
		int track = gEditor.currentStream[0] % GHS_NumTracks;
		CenterText(10.0f, gEditor.selectedStream == 0 ? 40.0f : 30.0f, gEditor.selectedStream == 0 ? MFVector::red : MFVector::white, MFStr("%s - %s", gEditor.pSong->GetDifficultyName(difficulty), gEditor.pSong->GetTrackName(track)), pHeading, false);

		rect.x += rect.width;
		MFView_SetViewport(&rect);
		pTrack2->Draw(currentTime, gEditor.pSong, gEditor.currentStream[1]);

		difficulty = gEditor.currentStream[1] / GHS_NumTracks;
		track = gEditor.currentStream[1] % GHS_NumTracks;
		CenterText(10.0f, gEditor.selectedStream == 1 ? 40.0f : 30.0f, gEditor.selectedStream == 1 ? MFVector::red : MFVector::white, MFStr("%s - %s", gEditor.pSong->GetDifficultyName(difficulty), gEditor.pSong->GetTrackName(track)), pHeading, false);

		MFView_SetViewport(nullptr);

		MFView_Pop();
	}

	MFFont_DrawTextAnchored(pText, gEditor.pSong->songName, MakeVector(rect.width - 16, 16), MFFontJustify_Top_Right, 1280.0f, 24.0f, MFVector::white);
	MFFont_DrawTextAnchored(pText, gEditor.pSong->artistName, MakeVector(rect.width - 16, 40), MFFontJustify_Top_Right, 1280.0f, 24.0f, MFVector::white);

	float trackOffset;
	if(gEditor.state == GHPS_Stopped)
		trackOffset = ((float)gEditor.measure + (float)gEditor.beat / (float)gEditor.quantisation) * 4.f;
	else
		trackOffset = (float)gEditor.offset/gEditor.pSong->GetRes();
	MFFont_DrawText2f(pText, 10.0f, 10.0f, 20.0f, MFVector::yellow, "%s: %g", MFTranslation_GetString(pStrings, TRACK_POSITION), trackOffset);

	int minutes = (int)(gEditor.playingTime / 60000000);
	int seconds = (int)((gEditor.playingTime%60000000)/1000000);
	int milliseconds = (int)((gEditor.playingTime%1000000)/1000);
	MFFont_DrawText2f(pText, 10.0f, 30.0f, 20.0f, MFVector::yellow, "%s: %02d:%02d.%03d", MFTranslation_GetString(pStrings, TRACK_TIME), minutes, seconds, milliseconds);
	MFFont_DrawText2f(pText, 10.0f, 50.0f, 20.0f, MFVector::yellow, "%s: 1/%d", MFTranslation_GetString(pStrings, TRACK_STEP), gEditor.quantisation);

	MFFont_DrawText2f(pText, 10.0f, 80.0f, 20.0f, MFVector::yellow, "%s: %g", MFTranslation_GetString(pStrings, TRACK_BPM), (float)gEditor.currentBPM * 0.001f);
	MFFont_DrawText2f(pText, 10.0f, 100.0f, 20.0f, MFVector::yellow, "%s: %g", MFTranslation_GetString(pStrings, TRACK_START_OFFSET), GETSECONDS(gEditor.pSong->startOffset));

	if(bMetronome || bClaps)
	{
		MFFont_DrawText2f(pHeading, 10.0f, 120.0f, 30.0f, MFVector::yellow, "%s%s", bMetronome ? "M" : "", bClaps ? "C" : "");

//		MFMaterial_SetMaterial(gEditor.pMetronome);
//		MFPrimitive_DrawQuad(10, 90, 64, 64);
	}

	if(bHalfSpeed)
		MFFont_DrawText2f(pText, 10.0f, 160.0f, 20.0f, MFVector::yellow, "%s", gSpeedStrings[bHalfSpeed]);

	if(gHyperSpeed)
		MFFont_DrawText2f(pText, 10.0f, 180.0f, 20.0f, MFVector::yellow, "%s: %gx", MFTranslation_GetString(pStrings, TRACK_HYPERSPEED), gHyperSpeedTable[gHyperSpeed]);

	MFView_Pop();
}
