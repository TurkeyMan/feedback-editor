#include "FeedBack.h"
#include "Fuji/DebugMenu.h"
#include "NoteScoreKeeper.h"
#include "Song.h"

#include "Control.h"
#include "Screens/Editor.h"

extern MenuItemBool bClaps;

dBNoteScoreKeeper::dBNoteScoreKeeper()
{
	pChart = NULL;
	stream = GHS_Unknown;
}

dBNoteScoreKeeper::~dBNoteScoreKeeper()
{

}

void dBNoteScoreKeeper::Begin(dBChart *_pChart, GHStreams _stream, int _player, int64 startTime)
{
	playTime = startTime;

	pChart = _pChart;
	stream = _stream;
	player = _player;

	pStream = &pChart->notes[stream];

	combo = 0;
	score = 0;
	starCharge = 0;
	starBegin = -1;

	MFZeroMemory(noteHold, sizeof(noteHold));
	pNextEvent = pStream->GetNextEventTime(startTime - NOTE_WINDOW);
}

int dBNoteScoreKeeper::GetCombo()
{
	return combo;
}

int dBNoteScoreKeeper::GetScore()
{
	return score;
}

void dBNoteScoreKeeper::Update(int64 delta)
{
	int64 startTime = gEditor.playingTime - delta;
	int res = pChart->GetRes();

	if(bClaps)
	{
		GHEvent *pEv = pNextEvent;

		while(pEv && pEv->time <= gEditor.playingTime)
		{
			if(pEv->event == GHE_Note && pEv->time >= startTime && pEv->time < gEditor.playingTime)
			{
				MFVoice *pVoice = MFSound_Play(gEditor.pClapSound, MFPF_BeginPaused);
				MFSound_SetVolume(pVoice, gConfig.sound.clapLevel);
				if(gEditor.currentStream[1] != -1)
					MFSound_SetPan(pVoice, player == 0 ? -1.0f : 1.0f);
				MFSound_Pause(pVoice, false);
				break;
			}

			pEv = pEv->Next();
		}
	}

	// first, check our note hasn't exceeded the timing window
	while(pNextEvent && pNextEvent->time < gEditor.playingTime-NOTE_WINDOW)
	{
		// time out, and the player didn't play it..
		pNextEvent->played = -1;
		combo = 0;	// broken the combo!
		pNextEvent = pNextEvent->Next();
	}

	if(!pNextEvent)
		return;

	// read input state
	int playerOffset = player == 0 ? 0 : dBCtrl_Game_G2_Green - dBCtrl_Game_G1_Green;
	bool strum = TestControl((dBControlType)(dBCtrl_Game_G1_StrumUp + playerOffset), GHCT_Once) || TestControl((dBControlType)(dBCtrl_Game_G1_StrumDown + playerOffset), GHCT_Once) ||
					MFInput_WasPressed(Key_Up, IDD_Keyboard) || MFInput_WasPressed(Key_Down, IDD_Keyboard);	// or the keyboard too?

	if(pNextEvent->time > gEditor.playingTime+NOTE_WINDOW)
	{
		if(strum)
			combo = 0;
		return;
	}

	int gCurrentButtonState = 0;
	int tap = -1;

	// get button state
	for(int a=0; a<5; a++)
		gCurrentButtonState |= (TestControl(dBControlType(dBCtrl_Game_G1_Green + a + playerOffset), GHCT_Hold) || MFInput_Read(Key_1 + a, IDD_Keyboard)) ? 1 << a : 0;

	// check for hammers
	for(int a=1; a<5; a++)
	{
		if((TestControl(dBControlType(dBCtrl_Game_G1_Green + a + playerOffset), GHCT_Once) || MFInput_WasPressed(Key_1 + a, IDD_Keyboard)) && (gCurrentButtonState & ((1<<a)-1)))
			tap = a;
	}

	// check for pulls
	for(int a=4; a>0; a--)
	{
		int compliment = (~((1<<(a+1))-1)) & 0x1F;

		if((TestControl(dBControlType(dBCtrl_Game_G1_Green + a + playerOffset), GHCT_Release) || MFInput_WasReleased(Key_1 + a, IDD_Keyboard)) && (gCurrentButtonState & ((1<<a)-1)) && !(gCurrentButtonState & compliment))
		{
			--a;

			while(a>0)
			{
				if(gCurrentButtonState & 1 << a)
					break;
				--a;
			}

			tap = a;
			break;
		}
	}

	// do note press logic...
	if(pNextEvent->Next() && pNextEvent->Next()->tick == pNextEvent->tick)
	{
		// this is a chord
		if(strum)
		{
			GHEvent *pE = pNextEvent;

			int chord = 1 << pNextEvent->key;

			while(pNextEvent->Next() && pNextEvent->Next()->tick == pNextEvent->tick)
			{
				pNextEvent = pNextEvent->Next();
				chord |= 1 << pNextEvent->key;
			}

			// mark the notes played
			while(pE != pNextEvent->Next())
			{
				// compare the chord against the button state
				if(chord == gCurrentButtonState)
				{
					pE->played = 1;
					++combo;

					for(int a=0; a<5; ++a)
					{
						if(chord & (1 << a))
							gGame.pTrack->HitNote(a);
					}
				}
				else
					pE->played = -1;

				pE = pE->Next();
			}

			if(chord != gCurrentButtonState)
				combo = 0;

			// skip to next event
			pNextEvent = pNextEvent->Next();
		}
	}
	else
	{
		// a regular note
		if(strum)
		{
			int compliment = (~((1 << (pNextEvent->key+1)) - 1)) & 0x1F;

			if(gCurrentButtonState & (1 << pNextEvent->key) && !(gCurrentButtonState & compliment))
			{
				pNextEvent->played = 1;
				gGame.pTrack->HitNote(pNextEvent->key);
				++combo;
			}
			else
				combo = 0;

			pNextEvent = pNextEvent->Next();
		}
		else if(tap && pNextEvent->key == tap && combo)
		{
			GHEvent *pEv = pNextEvent;

			if(pEv->Prev() && pEv->Prev()->tick < pEv->tick && pEv->Prev()->tick >= pEv->tick - (res/4) && pEv->Prev()->key != pEv->key
				&& (!pEv->Next() || !(pEv->Next()->tick == pEv->tick))
				&& !pEv->Prev()->parameter && !(pEv->Prev()->Prev() && pEv->Prev()->Prev()->tick == pEv->Prev()->tick))
			{
				pEv->played = 1;
				gGame.pTrack->HitNote(pNextEvent->key);
			}

			pNextEvent = pNextEvent->Next();
		}
	}
}
