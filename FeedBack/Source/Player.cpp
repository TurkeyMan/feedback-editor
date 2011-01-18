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
#include "Screens/Editor.h"

extern MenuItemBool bMetronome;
extern MenuItemIntString bHalfSpeed;

static bool gChain = true;

extern float gSpeeds[];
static const uint64 gSpeedsMultiplier[] = { 1000000, 850000, 750000, 500000 };

void OffsetToMeasureAndBeat(int offset, int *pMeasure, int *pBeat);
int MeasureAndBeatToOffset(int measure, int beat);

void PlayerScreen::Update()
{
	// update timers
	int startOffset = gEditor.offset;
	int res = gEditor.pSong->GetRes();

	int64 oldTime = gEditor.playingTime;

	// if we're playing music, we need to sync against the play cursor for proper sync accuracy
	MFAudioStream *pStream = gEditor.pSong->pStream ? gEditor.pSong->pStream : (gEditor.pSong->pGuitar ? gEditor.pSong->pGuitar : gEditor.pSong->pBass);
	if(pStream)
	{
		// sync against the play cursor
		MFVoice *pVoice = MFSound_GetStreamVoice(pStream);
		uint32 playCursor = MFSound_GetPlayCursor(pVoice);

		MFSoundInfo info;
		MFSound_GetSoundInfo(MFSound_GetSoundFromVoice(pVoice), &info);

		if(playCursor < gEditor.lastTimestamp)
			gEditor.playStart += info.numSamples - gEditor.lastTimestamp + playCursor;
		else
			gEditor.playStart += playCursor - gEditor.lastTimestamp;
		gEditor.lastTimestamp = playCursor;

		gEditor.playingTime = gEditor.startTime + gEditor.playStart*1000000 / (int64)info.sampleRate;
	}
	else
	{
		// sync against the cpu timer
		uint64 tick = MFSystem_ReadRTC();
		gEditor.playingTime = gEditor.startTime + ((tick-gEditor.playStart)*gSpeedsMultiplier[bHalfSpeed] / MFSystem_GetRTCFrequency());
	}

	int64 delta = gEditor.playingTime - oldTime;

	// play metronome and note claps
	if(gEditor.playingTime < gEditor.pSong->startOffset)
	{
		gEditor.offset = 0;
	}
	else
	{
		// update editor offset
		gEditor.offset = MFMax(gEditor.pSong->CalculateTickAtTime(gEditor.playingTime, &gEditor.currentBPM), startOffset+1);

		if(bMetronome)
		{
			// check if we crossed a beat this frame..
			int beatOffset = startOffset % res;
			if(beatOffset == 0 || beatOffset + (gEditor.offset - startOffset) > res)
			{
				int beat = gEditor.offset - (gEditor.offset % res);
				bool bar = false;

				// this is a metronome tick
				GHEvent *pLastTS = gEditor.pSong->sync.GetMostRecentEvent(GHE_TimeSignature, beat);
				if(pLastTS)
					bar = ((beat - pLastTS->tick) % (pLastTS->parameter*res)) == 0;

				MFVoice *pVoice;

				if(bar)
					pVoice = MFSound_Play(gEditor.pHighTickSound, MFPF_BeginPaused);
				else
					pVoice = MFSound_Play(gEditor.pLowTickSound, MFPF_BeginPaused);

				MFSound_SetVolume(pVoice, gConfig.sound.tickLevel);
				MFSound_Pause(pVoice, false);
			}
		}
	}

	scoreKeeper[0].Update(delta);
	if(gEditor.currentStream[1] != -1)
		scoreKeeper[1].Update(delta);
}

void PlayerScreen::UpdateInput()
{
	GHScreen::UpdateInput();

	// swap modes
	if(TestControl(dBCtrl_Edit_StopPlayback, GHCT_Once) || TestControl(dBCtrl_Edit_StopReturn, GHCT_Once))
	{
		gEditor.pSong->Stop();

		gEditor.state = GHPS_Stopped;

		if(TestControl(dBCtrl_Edit_StopReturn, GHCT_Once))
			gEditor.offset = gEditor.playbackStartOffset;

		// snap the time to the last beat
		OffsetToMeasureAndBeat(gEditor.offset, &gEditor.measure, &gEditor.beat);
		gEditor.offset = MeasureAndBeatToOffset(gEditor.measure, gEditor.beat);

		gEditor.playingTime = gEditor.pSong->CalculateTimeOfTick(gEditor.offset);

		gEditor.pSong->notes[gEditor.currentStream[gEditor.selectedStream]].ResetNotes();

		Pop();
		gEdit.Push();
	}

	if(TestControl(dBCtrl_Edit_Mixer, GHCT_Once))
	{
		((EditorScreen*)dBScreen::GetCurrent())->gMixer.Push();
	}
}
