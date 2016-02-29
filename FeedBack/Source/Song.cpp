#include "FeedBack.h"
#include "Fuji/MFFileSystem.h"
#include "Fuji/MFView.h"
#include "Fuji/MFInput.h"
#include "Fuji/MFPrimitive.h"
#include "Fuji/MFHeap.h"
#include "Fuji/MFSound.h"
#include "Fuji/FileSystem/MFFileSystemNative.h"
#include "Fuji/MFIni.h"
#include "Fuji/DebugMenu.h"

#include "MidiFile.h"

extern MessageBoxScreen *gpMsgBox;

const char * const noteSheet[GHS_Max] =
{
	"EasySingle",
	"EasyDoubleGuitar",
	"EasyDoubleBass",
	"EasyEnhancedGuitar",
	"EasyCoopLead",
	"EasyCoopBass",
	"Easy10KeyGuitar",
	"EasyDrums",
	"EasyDoubleDrums",
	"EasyVocals",
	"EasyKeyboard",
	"MediumSingle",
	"MediumDoubleGuitar",
	"MediumDoubleBass",
	"MediumEnhancedGuitar",
	"MediumCoopLead",
	"MediumCoopBass",
	"Medium10KeyGuitar",
	"MediumDrums",
	"MediumDoubleDrums",
	"MediumVocals",
	"MediumKeyboard",
	"HardSingle",
	"HardDoubleGuitar",
	"HardDoubleBass",
	"HardEnhancedGuitar",
	"HardCoopLead",
	"HardCoopBass",
	"Hard10KeyGuitar",
	"HardDrums",
	"HardDoubleDrums",
	"HardVocals",
	"HardKeyboard",
	"ExpertSingle",
	"ExpertDoubleGuitar",
	"ExpertDoubleBass",
	"ExpertEnhancedGuitar",
	"ExpertCoopLead",
	"ExpertCoopBass",
	"Expert10KeyGuitar",
	"ExpertDrums",
	"ExpertDoubleDrums",
	"ExpertVocals",
	"ExpertKeyboard"
};

const char * gDifficultyStrings[GHS_NumDifficulties];
const char * gTrackStrings[GHS_NumTracks];
const char * gTrackCoopStrings[4];
const char * player2Strings[2];

const char * const player2TypeStrings[] =
{
	"bass",
	"rhythm"
};

const char * const eventType[] =
{
	"?",
	"N",
	"B",
	"A",
	"TS",
	"S",
	"E",
	"H"
};

extern float gSpeeds[];
extern MenuItemIntString bHalfSpeed;

// helpers

void PutString(MFFile *f, const char *pString)
{
	MFFile_Write(f, pString, MFString_Length(pString), false);
}


// functions

dBChart::dBChart()
{
	pStream = NULL;
	pGuitar = NULL;
	pBass = NULL;
	pFretboard = NULL;

//	pVoice = NULL;
//	pGuitarVoice = NULL;
//	pBassVoice = NULL;

	startOffset = 0;
	MFString_Copy(songPath, "");
	MFString_Copy(songName, "Untitled Song");
	MFString_Copy(artistName, "Unknown Artist");
	MFString_Copy(charterName, "Unknown Charter");

	MFString_Copy(musicURL, "");
	MFString_Copy(previewURL, "");
	MFString_Copy(genre, "rock");
	MFString_Copy(mediaType, "cd");

	previewStart = 0.0f;
	previewEnd = 0.0f;

	difficulty = 0;

	resolution = 192;

	player2Type = GHP2T_Bass;

	sync.Init();
	events.Init();

	for(int a=0; a<GHS_Max; a++)
		notes[a].Init();

	musicFilename[0] = 0;
	guitarFilename[0] = 0;
	bassFilename[0] = 0;
	fretboard[0] = 0;

	sync.AddEvent(GHE_BPM, 0, 0, 120000);
	sync.AddEvent(GHE_TimeSignature, 0, 0, 4);
}

dBChart::~dBChart()
{
	sync.Deinit();
	events.Deinit();

	for(int a=0; a<GHS_Max; a++)
		notes[a].Deinit();

	if(pStream)
		MFSound_DestroyStream(pStream);
	if(pGuitar)
		MFSound_DestroyStream(pGuitar);
	if(pBass)
		MFSound_DestroyStream(pBass);
	if(pFretboard)
		MFMaterial_Release(pFretboard);
}

const char *dBChart::GetDifficultyName(int difficulty)
{
	return gDifficultyStrings[difficulty];
}

const char *dBChart::GetTrackName(int track)
{
	if(track == GHS_EasyDoubleBass)
		return gTrackCoopStrings[player2Type];
	else if(track == GHS_EasyEnhancedCoopBass)
		return gTrackCoopStrings[player2Type+2];
	else
		return gTrackStrings[track];
}

MFAudioStream *dBChart::PlayStream(const char *pFilename)
{
	MFAudioStream *pStream = MFSound_CreateStream(MFStr("%s%s", MFStr_GetFilePath(songPath), pFilename), MFASF_QueryLength | MFASF_AllowSeeking | MFASF_AllowBuffering);
	if(pStream)
	{
		MFSound_PlayStream(pStream, MFPF_BeginPaused);
		MFSound_SetPlaybackRate(MFSound_GetStreamVoice(pStream), gSpeeds[bHalfSpeed]);
	}
	return pStream;
}

dBChart *dBChart::Create(const char *pMusic)
{
	dBChart *pChart = new dBChart;

	if(pMusic)
	{
		pChart->pStream = MFSound_CreateStream(pMusic, MFASF_QueryLength | MFASF_AllowSeeking | MFASF_AllowBuffering);
		MFSound_PlayStream(pChart->pStream, MFPF_BeginPaused);

		if(!pChart->pStream)
		{
			gpMsgBox->Show(MFTranslation_GetString(pStrings, ERROR_INVALID_STREAM));
		}
		else
		{
			MFString_Copy(pChart->songPath, MFStr_TruncateExtension(pMusic));
			MFString_Copy(pChart->musicFilename, MFStr_GetFileName(pMusic));

			const char *pTrackName = MFSound_GetStreamInfo(pChart->pStream, MFSIT_TrackName);
			const char *pArtistName = MFSound_GetStreamInfo(pChart->pStream, MFSIT_ArtistName);

			if(pTrackName)
				MFString_Copy(pChart->songName, pTrackName);
			else
				MFString_Copy(pChart->songName, MFStr_GetFileName(pChart->songPath));

			if(pArtistName)
				MFString_Copy(pChart->artistName, pArtistName);
		}
	}

	return pChart;
}

dBChart *dBChart::LoadChart(const char *pSong)
{
	dBChart *pNew = new dBChart;

	MFString_Copy(pNew->songPath, pSong);
	MFString_Copy(gConfig.editor.lastOpenedChart, pSong);
	pNew->resolution = 192;

	// check for a matching song file
	size_t size;
	char *pChart = MFFileSystem_Load(MFStr("%s.chart", pSong), &size);
	if(pChart)
	{
		char x = pChart[size];
		pChart[size] = 0;
		MFIni *pIni = MFIni::CreateFromMemory(pChart);
		pChart[size] = x;

		pNew->sync.Clear();

		MFIniLine *pLine = pIni->GetFirstLine();

		while(pLine)
		{
			if(pLine->IsSection("Song"))
			{
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					if(pSub->IsString(0, "Name"))
					{
						MFString_Copy(pNew->songName, pSub->GetString(1));
					}
					else if(pSub->IsString(0, "Artist"))
					{
						MFString_Copy(pNew->artistName, pSub->GetString(1));
					}
					else if(pSub->IsString(0, "Charter"))
					{
						MFString_Copy(pNew->charterName, pSub->GetString(1));
					}
					else if(pSub->IsString(0, "Offset"))
					{
						pNew->startOffset = (int)(pSub->GetFloat(1)*1000000.0f);
					}
					else if(pSub->IsString(0, "Resolution"))
					{
						pNew->resolution = pSub->GetInt(1);
					}
					else if(pSub->IsString(0, "Player2"))
					{
						const char *pPlayer2 = pSub->GetString(1);

						const int numStrings = sizeof(player2TypeStrings)/sizeof(player2TypeStrings[0]);
						for(int a=0; a<numStrings; a++)
						{
							if(!MFString_CaseCmp(pPlayer2, player2TypeStrings[a]))
								pNew->player2Type = a;
						}
					}
					else if(pSub->IsString(0, "MusicStream"))
					{
						MFString_Copy(pNew->musicFilename, pSub->GetString(1));

						if(!MFString_Compare(pNew->musicFilename, "None"))
							pNew->musicFilename[0] = 0;
					}
					else if(pSub->IsString(0, "GuitarStream"))
					{
						MFString_Copy(pNew->guitarFilename, pSub->GetString(1));

						if(!MFString_Compare(pNew->guitarFilename, "None"))
							pNew->guitarFilename[0] = 0;
					}
					else if(pSub->IsString(0, "BassStream"))
					{
						MFString_Copy(pNew->bassFilename, pSub->GetString(1));

						if(!MFString_Compare(pNew->bassFilename, "None"))
							pNew->bassFilename[0] = 0;
					}
					else if(pSub->IsString(0, "Fretboard"))
					{
						MFString_Copy(pNew->fretboard, pSub->GetString(1));

						if(!MFString_Compare(pNew->fretboard, "None"))
							pNew->fretboard[0] = 0;
					}
					else if(pSub->IsString(0, "MusicURL"))
					{
						MFString_Copy(pNew->musicURL, pSub->GetString(1));

						if(!MFString_Compare(pNew->musicURL, "None"))
							pNew->musicURL[0] = 0;
					}
					else if(pSub->IsString(0, "PreviewURL"))
					{
						MFString_Copy(pNew->previewURL, pSub->GetString(1));

						if(!MFString_Compare(pNew->previewURL, "None"))
							pNew->previewURL[0] = 0;
					}
					else if(pSub->IsString(0, "Genre"))
					{
						MFString_Copy(pNew->genre, pSub->GetString(1));
					}
					else if(pSub->IsString(0, "MediaType"))
					{
						MFString_Copy(pNew->mediaType, pSub->GetString(1));
					}
					else if(pSub->IsString(0, "PreviewStart"))
					{
						pNew->previewStart = pSub->GetFloat(1);
					}
					else if(pSub->IsString(0, "PreviewEnd"))
					{
						pNew->previewEnd = pSub->GetFloat(1);
					}
					else if(pSub->IsString(0, "Difficulty"))
					{
						pNew->difficulty = pSub->GetInt(1);
					}

					pSub = pSub->Next();
				}
			}
			else if(pLine->IsSection("SyncTrack"))
			{
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					const char *pType = pSub->GetString(1);

					int type = -1;

					for(int a=0; a<GHE_Max; a++)
					{
						if(!MFString_CaseCmp(pType, eventType[a]))
						{
							type = a;
							break;
						}
					}

					GHEvent *pEv = NULL;
					if(type > 0)
						pEv = pNew->sync.AddEvent((GHEventType)type, pSub->GetInt(0), 0, pSub->GetInt(2));

					if(pEv && type == GHE_Anchor)
					{
						pSub = pSub->Next();
						pEv->time = pEv->parameter + pNew->startOffset;
						pEv->parameter = pSub->GetInt(2);
					}

					pSub = pSub->Next();
				}
			}
			else if(pLine->IsSection("Events"))
			{
				// read string events
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					const char *pType = pSub->GetString(1);

					int type = -1;

					for(int a=0; a<GHE_Max; a++)
					{
						if(!MFString_CaseCmp(pType, eventType[a]))
						{
							type = a;
							break;
						}
					}

					if(type > 0)
						pNew->events.AddStringEvent((GHEventType)type, pSub->GetInt(0), pSub->GetString(2));

					pSub = pSub->Next();
				}
			}
			else if(pLine->IsString(0, "section"))
			{
				// find which note pattern this is...
				const char *pPattern = pLine->GetString(1);

				int pattern = -1;

				for(int a=0; a<GHS_Max; a++)
				{
					if(!MFString_CaseCmp(pPattern, noteSheet[a]))
					{
						pattern = a;
						break;
					}
				}

				if(pattern > -1)
				{
					MFIniLine *pSub = pLine->Sub();

					while(pSub)
					{
						const char *pType = pSub->GetString(1);

						int type = -1;

						for(int a=0; a<GHE_Max; a++)
						{
							if(!MFString_CaseCmp(pType, eventType[a]))
							{
								type = a;
								break;
							}
						}

						int param = pSub->GetInt(3);

						if(type == GHE_Event)
						{
							pNew->notes[pattern].AddStringEvent((GHEventType)type, pSub->GetInt(0), pSub->GetString(2));
						}
						else
						{
							if(type > 0 && (type != GHE_Special || (type == GHE_Special && param > 0)))
								pNew->notes[pattern].AddEvent((GHEventType)type, pSub->GetInt(0), pSub->GetInt(2), param);
						}

						pSub = pSub->Next();
					}
				}
			}

			pLine = pLine->Next();
		}

		for(int a=0; a<GHS_Max; a++)
			pNew->CalculateNoteTimes(a, 0);

		MFIni::Destroy(pIni);

		pSong = MFStr_GetFileName(pSong);

		if(!*pNew->musicFilename)
			MFString_Copy(pNew->musicFilename, MFStr("%s.mp3", pSong));

		bool showOnce = false;

		if(!(pNew->pStream = pNew->PlayStream(pNew->musicFilename)))
		{
			gpMsgBox->Show(MFStr(MFTranslation_GetString(pStrings, ERROR_NO_AUDIO), pNew->musicFilename));
			showOnce = true;
		}

		if(*pNew->guitarFilename)
		{
			if(!(pNew->pGuitar = pNew->PlayStream(pNew->guitarFilename)) && !showOnce)
			{
				gpMsgBox->Show(MFStr(MFTranslation_GetString(pStrings, ERROR_NO_GUITAR), pNew->guitarFilename));
				showOnce = true;
			}
		}
		if(*pNew->bassFilename)
		{
			if(!(pNew->pBass = pNew->PlayStream(pNew->bassFilename)) && !showOnce)
			{
				gpMsgBox->Show(MFStr(MFTranslation_GetString(pStrings, ERROR_NO_BASS), pNew->bassFilename));
				showOnce = true;
			}
		}

		if(*pNew->fretboard)
		{
			pNew->pFretboard = MFMaterial_Create(pNew->fretboard);
		}

		MFHeap_Free(pChart);
	}

	return pNew;
}

#include <stdio.h>
void WriteString(const char *pString, FILE *pFile)
{
	size_t len = MFString_Length(pString);
	fwrite(pString, len, 1, pFile);
}

void WriteMidiToTxt(MIDIFile *pMidi)
{
	FILE *pFile = fopen("C:\\midi.txt", "wb");

	WriteString(MFStr("%s\r\n", pMidi->name), pFile);

	WriteString(MFStr("Format: %d\r\n", pMidi->format), pFile);
	WriteString(MFStr("Res: %d\r\n", pMidi->ticksPerBeat), pFile);
	WriteString(MFStr("Num Tracks: %d\r\n", pMidi->numTracks), pFile);

	for(int a=0; a<pMidi->numTracks; ++a)
	{
		WriteString(MFStr("Track %d:\r\n", a), pFile);

		MIDIEvent *pEv = pMidi->ppTracks[a];

		while(pEv)
		{
			WriteString(MFStr("  %d:\t%d:%d (%X:%X) - ", pEv->tick, pEv->type, pEv->subType, pEv->type, pEv->subType), pFile);
			switch(pEv->type)
			{
				case MET_Custom:
					WriteString("Non-MIDI event: ", pFile);
					switch(pEv->subType)
					{
						case MEV_SequenceNumber:
							break;
						case MEV_Text:
						case MEV_Copyright:
						case MEV_TrackName:
						case MEV_Instrument:
						case MEV_Lyric:
						case MEV_Marker:
						case MEV_CuePoint:
						case MEV_PatchName:
						case MEV_PortName:
						{
							MIDIEvent_Text *pSE = (MIDIEvent_Text*)pEv;
							WriteString(MFStr("    String: \"%s\"\r\n", pSE->buffer), pFile);
							break;
						}
						case MEV_EndOfTrack:
							WriteString("    End Of Track\r\n", pFile);
							break;
						case MEV_Tempo:
						{
							MIDIEvent_Tempo *pTE = (MIDIEvent_Tempo*)pEv;
							WriteString(MFStr("    Tempo: %gbpm\r\n", pTE->BPM), pFile);
							break;
						}
						case MEV_SMPTE:
							break;
						case MEV_TimeSignature:
						{
							MIDIEvent_TimeSignature *pTSE = (MIDIEvent_TimeSignature*)pEv;
							WriteString(MFStr("    TS: %d/4\r\n", pTSE->numerator), pFile);
							break;
						}
						case MEV_KeySignature:
							break;
						case MEV_Custom:
							break;
					}
					break;
				case MET_SYSEX:
					WriteString("SysEx event\r\n", pFile);
					break;
				case MET_Note:
				{
					MIDIEvent_Note *pNE = (MIDIEvent_Note*)pEv;
					WriteString(MFStr("MIDI -\te: %d, ch: %d, n: %d, v: %d\r\n", pNE->event, pNE->channel, pNE->note, pNE->velocity), pFile);
					break;
				}
				default:
					break;
			}

			pEv = pEv->pNext;
		}
	}

	fclose(pFile);
}

dBChart *dBChart::LoadMidi(const char *pFile)
{
	MIDIFile *pMidi = LoadMidiFromFile(pFile);

	if(!pMidi)
		return NULL;

//	WriteMidiToTxt(pMidi);

	dBChart *pNew = new dBChart;
	pNew->resolution = pMidi->ticksPerBeat;

	pFile = MFStr_TruncateExtension(pFile);
	MFString_Copy(pNew->songPath, pFile);
	MFString_Copy(gConfig.editor.lastOpenedChart, pFile);

	const char *pFilename = MFStr_GetFileName(pFile);

	// try to load music files...
	const char *pStreamName[] =
	{
		MFStr("%s.mp3", pFilename),
		MFStr("%s.ogg", pFilename),
		MFStr("%s.wav", pFilename),
		"song.mp3",
		"song.ogg",
		"song.wav"
	};

	for(uint32 a=0; a<(sizeof(pStreamName)/sizeof(pStreamName[0])); ++a)
	{
		pNew->pStream = pNew->PlayStream(pStreamName[a]);
		if(pNew->pStream)
		{
			MFString_Copy(pNew->musicFilename, MFStr_GetFileName(pStreamName[a]));
			break;
		}
	}

	const char *pGuitarName[] =
	{
		"guitar.mp3",
		"guitar.ogg",
		"guitar.wav"
	};

	for(uint32 a=0; a<(sizeof(pGuitarName)/sizeof(pGuitarName[0])); ++a)
	{
		pNew->pGuitar = pNew->PlayStream(pGuitarName[a]);
		if(pNew->pGuitar)
		{
			MFString_Copy(pNew->guitarFilename, MFStr_GetFileName(pGuitarName[a]));
			break;
		}
	}

	const char *pBassName[] =
	{
		"rhythm.mp3",
		"rhythm.ogg",
		"rhythm.wav",
		"bass.mp3",
		"bass.ogg",
		"bass.wav"
	};

	for(uint32 a=0; a<(sizeof(pBassName)/sizeof(pBassName[0])); ++a)
	{
		pNew->pBass = pNew->PlayStream(pBassName[a]);
		if(pNew->pBass)
		{
			MFString_Copy(pNew->bassFilename, MFStr_GetFileName(pBassName[a]));
			break;
		}
	}

	// attempt to parse the midi file..

	// parse track 0 (timing and stuff)
	MIDIEvent *pEv = pMidi->ppTracks[0];

	while(pEv)
	{
		// read BPM and shit...
		if(pEv->subType == MEV_TrackName)
		{
			MFString_Copy(pNew->songName, ((MIDIEvent_Text*)pEv)->buffer);
		}
		else if(pEv->subType == MEV_Tempo)
		{
			MIDIEvent_Tempo *pTempo = (MIDIEvent_Tempo*)pEv;

			GHEvent *pT = pNew->sync.FindEvent(GHE_BPM, pTempo->tick);
			if(pT)
				pT->parameter = (int)(pTempo->BPM * 1000);
			else
				pNew->sync.AddEvent(GHE_BPM, pTempo->tick, 0, (int)(pTempo->BPM * 1000));
		}
		else if(pEv->subType == MEV_TimeSignature)
		{
			MIDIEvent_TimeSignature *pTimeSig = (MIDIEvent_TimeSignature*)pEv;

			GHEvent *pT = pNew->sync.FindEvent(GHE_TimeSignature, pTimeSig->tick);
			if(pT)
				pT->parameter = pTimeSig->numerator;
			else
				pNew->sync.AddEvent(GHE_TimeSignature, pTimeSig->tick, 0, pTimeSig->numerator);			
		}

		pEv = pEv->pNext;
	}

	// are we parsing a GH1 or GH2 file?
	bool bIsGH2 = true;

	pEv = pMidi->ppTracks[1];
	while(pEv)
	{
		if(pEv->subType == MEV_TrackName)
		{
			MIDIEvent_Text *pText = (MIDIEvent_Text*)pEv;

			if(!MFString_CaseCmp(pText->buffer, "T1 GEMS"))
			{
				bIsGH2 = false;
				break;
			}
		}

		pEv = pEv->pNext;
	}

	// parse additional tracks...
	int maxGuitarTrack = bIsGH2 ? pMidi->numTracks : 2;
	for(int a=1; a<maxGuitarTrack; a++)
	{
		MIDIEvent *pEv = pMidi->ppTracks[a];

		GHStreams stream = GHS_Unknown;

		// get track name
		while(pEv)
		{
			if(pEv->subType == MEV_TrackName)
			{
				MIDIEvent_Text *pText = (MIDIEvent_Text*)pEv;

				if(!MFString_CaseCmp(pText->buffer, "T1 GEMS") ||
					!MFString_CaseCmp(pText->buffer, "PART_GUITAR") ||
					!MFString_CaseCmp(pText->buffer, "PART GUITAR"))
				{
					stream = GHS_EasySingle;
				}
				else if(!MFString_CaseCmp(pText->buffer, "PART_GUITAR COOP") ||
						!MFString_CaseCmp(pText->buffer, "PART GUITAR COOP"))
				{
					stream = GHS_EasyDoubleGuitar;
				}
				else if(!MFString_CaseCmp(pText->buffer, "PART_BASS") ||
						!MFString_CaseCmp(pText->buffer, "PART BASS"))
				{
					stream = GHS_EasyDoubleBass;
				}
				else if(!MFString_CaseCmp(pText->buffer, "PART_RHYTHM") ||
						!MFString_CaseCmp(pText->buffer, "PART RHYTHM"))
				{
					stream = GHS_EasyDoubleBass;
					pNew->player2Type = GHP2T_Rhythm;
				}
				else if(!MFString_CaseCmp(pText->buffer, "BAND_DRUMS") ||
						!MFString_CaseCmp(pText->buffer, "BAND DRUMS") ||
						!MFString_CaseCmp(pText->buffer, "PART DRUMS"))
				{
					stream = GHS_EasyDrums;
				}
				else if(!MFString_CaseCmp(pText->buffer, "BAND_SINGER") ||
						!MFString_CaseCmp(pText->buffer, "BAND SINGER"))
				{
					stream = GHS_EasyVocals;
				}
				else if(!MFString_CaseCmp(pText->buffer, "BAND_KEYS") ||
						!MFString_CaseCmp(pText->buffer, "BAND KEYS"))
				{
					stream = GHS_EasyKeyboard;
				}
				else if(!MFString_CaseCmp(pText->buffer, "EVENTS"))
				{
					stream = GHS_Max;
				}
				break;
			}
			pEv = pEv->pNext;
		}

		if(stream == GHS_Unknown)
			continue;

		int terminate[4][12];
//		int drumTerminate[2];
		int fingerTerminate[19];
		MFMemSet(terminate, -1, sizeof(terminate));
//		MFMemSet(drumTerminate, -1, sizeof(drumTerminate));
		MFMemSet(fingerTerminate, -1, sizeof(fingerTerminate));
		const int noteBase[4] = { 60, 72, 84, 96 };
		const int noteRange[4] = { 70, 82, 94, 106 };

		const int minHold = pNew->GetRes()/2;

		for(pEv = pMidi->ppTracks[a]; pEv; pEv = pEv->pNext)
		{
			switch(pEv->type)
			{
				case MET_Custom:
				{
					switch(pEv->subType)
					{
						case MEV_Text:
						{
							MIDIEvent_Text *pT = (MIDIEvent_Text*)pEv;
							if(pT->buffer[0] == '[')
							{
								const char *pEventString = MFStrN(&pT->buffer[1], MFString_Length(pT->buffer)-2);
								if(stream == GHS_Max)
								{
									pNew->events.AddStringEvent(GHE_Event, pEv->tick, pEventString);
								}
								else
								{
									pNew->notes[stream].AddStringEvent(GHE_Event, pEv->tick, pEventString);
									pNew->notes[stream + GHS_NumTracks].AddStringEvent(GHE_Event, pEv->tick, pEventString);
									pNew->notes[stream + GHS_NumTracks*2].AddStringEvent(GHE_Event, pEv->tick, pEventString);
									pNew->notes[stream + GHS_NumTracks*3].AddStringEvent(GHE_Event, pEv->tick, pEventString);
								}
							}
							break;
						}
						default:
							MFDebug_Log(2, MFStr("Unhandled event: %d", pEv->subType));
							break;
					}
					break;
				}
				case MET_Note:
				{
					MIDIEvent_Note *pNote = (MIDIEvent_Note*)pEv;

					int *pStart = NULL;
					GHEventManager *pEM = NULL;
					int note = -1;
					GHEventType et = GHE_Note;

					for(int a=0; a<4; ++a)
					{
						if(pNote->note >= noteBase[a] && pNote->note <= noteRange[a])
						{
							note = pNote->note-noteBase[a];
							pStart = &terminate[a][note];
							if(note > 4)
							{
								et = GHE_Special;
								if(note == 7)
									note = GHS_Super;
								else
									note -= 9;
							}
							pEM = &pNew->notes[stream + GHS_NumTracks*a];
							break;
						}
					}
/*
					// sadly this information doesn't seem to be correct...
					if(pNote->note >= 36 && pNote->note <= 37)
					{
						// GH2 drum notes
						note = pNote->note == 36 ? 1 : 4;
						pStart = &drumTerminate[pNote->note-36];
						pEM = &pNew->notes[GHS_ExpertDrums];
					}
*/
					if(pNote->note >= 40 && pNote->note <= 58)
					{
						// hand positions
						et = GHE_HandPos;
						note = pNote->note-40;
						pStart = &fingerTerminate[note];
						pEM = &pNew->events;
					}

					if(note > -1)
					{
						if(pNote->event == MNE_NoteOn && pNote->velocity > 0)
						{
							if(*pStart != -1)
							{
								// this is a degenerate case.. we have triggered another note while a note is already active.
								// MIDI probably does this to modulate the velocity...
								MFDebug_Warn(4, "Note already activated.");
/*
								// perhaps we want to terminate the current note and start a new one?
								int holdLen = pEv->tick - (*ppStart)->tick;
								if(holdLen < minHold)
									(*ppStart)->parameter = 0;
								else
									(*ppStart)->parameter = holdLen;

								*ppStart = NULL;

								*ppStart = (int)(pEM->AddEvent(et, pEv->tick, note) - pEM->First());
*/
							}
							else
							{
								GHEvent *pNewEvent = pEM->AddEvent(et, pEv->tick, note);
								*pStart = (int)(pNewEvent - pEM->First());
							}
						}
						else if(pNote->event == MNE_NoteOff || (pNote->event == MNE_NoteOn && pNote->velocity == 0))
						{
							if(*pStart != -1)
							{
								// end the note...
								GHEvent *pGEv = pEM->First() + *pStart;
								int holdLen = pEv->tick - pGEv->tick;
								if(holdLen < minHold)
									pGEv->parameter = 0;
								else
									pGEv->parameter = holdLen;

								*pStart = -1;
							}
							else
							{
								// no prior note-on event..
								MFDebug_Warn(4, "MIDI terminate event with no prior 'note on'.");
							}
						}
					}

					break;
				}
			}
		}
	}

	for(int a=0; a<GHS_Max; a++)
		pNew->CalculateNoteTimes(a, 0);

	FreeMidi(pMidi);

	// see if we have an accompanying ini file to get the artist and track name from
	const char *pPath = MFStr_GetFilePath(pFile);
	MFIni *pIni = MFIni::Create(MFStr("%ssong", pPath));

	if(pIni)
	{
		MFIniLine *pLine = pIni->GetFirstLine();
		while(pLine)
		{
			if(!MFString_CaseCmp(pLine->GetString(0), "name"))
			{
				pNew->songName[0] = 0;
				for(int a=1; a<pLine->GetStringCount(); ++a)
				{
					if(a > 1)
						MFString_Cat(pNew->songName, " ");
					MFString_Cat(pNew->songName, pLine->GetString(a));
				}
			}
			else if(!MFString_CaseCmp(pLine->GetString(0), "artist"))
			{
				pNew->artistName[0] = 0;
				for(int a=1; a<pLine->GetStringCount(); ++a)
				{
					if(a > 1)
						MFString_Cat(pNew->artistName, " ");
					MFString_Cat(pNew->artistName, pLine->GetString(a));
				}
			}
			else if(!MFString_CaseCmp(pLine->GetString(0), "cassettecolor"))
			{
				MFString_Copy(pNew->mediaType, "cassette");
			}

			pLine = pLine->Next();
		}

		MFIni::Destroy(pIni);
	}

	return pNew;
}

void dBChart::Destroy()
{
	delete this;
}

void dBChart::SaveChart()
{
	// write out the song file
	MFFile *file = MFFileSystem_Open(MFStr("songs:%s.chart", songPath), MFOF_Write | MFOF_Binary);
	MFString_Copy(gConfig.editor.lastOpenedChart, songPath);

	if(file)
	{
		// write file
		PutString(file, "[Song]\r\n{\r\n");
		PutString(file, MFStr("\tName = \"%s\"\r\n", songName));
		PutString(file, MFStr("\tArtist = \"%s\"\r\n", artistName));
		PutString(file, MFStr("\tCharter = \"%s\"\r\n", charterName));
		PutString(file, MFStr("\tOffset = %g\r\n", GETSECONDS(startOffset)));
		PutString(file, MFStr("\tResolution = %d\r\n", GetRes()));
		PutString(file, MFStr("\tPlayer2 = %s\r\n", player2TypeStrings[player2Type]));
		PutString(file, MFStr("\tDifficulty = %d\r\n", difficulty));
		PutString(file, MFStr("\tPreviewStart = %.2f\r\n", previewStart));
		PutString(file, MFStr("\tPreviewEnd = %.2f\r\n", previewEnd));
		PutString(file, MFStr("\tGenre = \"%s\"\r\n", genre));
		PutString(file, MFStr("\tMediaType = \"%s\"\r\n", mediaType));
		PutString(file, MFStr("\tMusicStream = \"%s\"\r\n", musicFilename));
		if(*guitarFilename)
			PutString(file, MFStr("\tGuitarStream = \"%s\"\r\n", guitarFilename));
		if(*bassFilename)
			PutString(file, MFStr("\tBassStream = \"%s\"\r\n", bassFilename));
		if(*fretboard)
			PutString(file, MFStr("\tFretboard = \"%s\"\r\n", fretboard));
		if(*musicURL)
			PutString(file, MFStr("\tMusicURL = \"%s\"\r\n", musicURL));
		if(*previewURL)
			PutString(file, MFStr("\tPreviewURL = \"%s\"\r\n", previewURL));
		PutString(file, "}\r\n");
		PutString(file, "[SyncTrack]\r\n{\r\n");
		for(GHEvent *pEv = sync.First(); pEv; pEv = pEv->Next())
		{
			// list all the sync events
			if(pEv->event == GHE_Anchor)
			{
				PutString(file, MFStr("\t%d = %s %d\r\n", pEv->tick, eventType[GHE_Anchor], pEv->time - startOffset));
				PutString(file, MFStr("\t%d = %s %d\r\n", pEv->tick, eventType[GHE_BPM], pEv->parameter));
			}
			else
			{
				PutString(file, MFStr("\t%d = %s %d\r\n", pEv->tick, eventType[pEv->event], pEv->parameter));
			}
		}
		PutString(file, "}\r\n");

		PutString(file, "[Events]\r\n{\r\n");
		for(GHEvent *pEv = events.First(); pEv; pEv = pEv->Next())
		{
			// list all the sync events
			if(pEv->event == GHE_Event)
			{
				PutString(file, MFStr("\t%d = %s \"%s\"\r\n", pEv->tick, eventType[pEv->event], pEv->GetString()));
			}
			else if(pEv->event == GHE_HandPos)
			{
				PutString(file, MFStr("\t%d = %s %d %d\r\n", pEv->tick, eventType[pEv->event], pEv->key, pEv->parameter));
			}
		}
		PutString(file, "}\r\n");

		// write each of the difficulties
		for(int a=0; a<GHS_Max; a++)
		{
			if(notes[a].First())
			{
				PutString(file, MFStr("[%s]\r\n{\r\n", noteSheet[a]));
				for(GHEvent *pEv = notes[a].First(); pEv; pEv = pEv->Next())
				{
					// list all the sync events
					if(pEv->event == GHE_Event)
					{
						PutString(file, MFStr("\t%d = %s %s\r\n", pEv->tick, eventType[pEv->event], pEv->GetString()));
					}
					else
						PutString(file, MFStr("\t%d = %s %d %d\r\n", pEv->tick, eventType[pEv->event], pEv->key, pEv->parameter));
				}
				PutString(file, "}\r\n");
			}
		}

		MFFile_Close(file);
	}
}

void dBChart::Play(int64 time)
{
	if(pStream)
	{
		MFSound_SeekStream(pStream, GETSECONDS(time));
		MFSound_SetVolume(MFSound_GetStreamVoice(pStream), gConfig.sound.musicLevel * gConfig.sound.masterLevel);
	}
	if(pGuitar)
	{
		MFSound_SeekStream(pGuitar, GETSECONDS(time));
		MFSound_SetVolume(MFSound_GetStreamVoice(pGuitar), gConfig.sound.guitarLevel * gConfig.sound.masterLevel);
	}
	if(pBass)
	{
		MFSound_SeekStream(pBass, GETSECONDS(time));
		MFSound_SetVolume(MFSound_GetStreamVoice(pBass), gConfig.sound.bassLevel * gConfig.sound.masterLevel);
	}
	if(pStream)
		MFSound_PauseStream(pStream, false);
	if(pGuitar)
		MFSound_PauseStream(pGuitar, false);
	if(pBass)
		MFSound_PauseStream(pBass, false);
}

void dBChart::Stop()
{
	if(pStream)
		MFSound_PauseStream(pStream, true);
	if(pGuitar)
		MFSound_PauseStream(pGuitar, true);
	if(pBass)
		MFSound_PauseStream(pBass, true);
/*
	if(pVoice)
	{
		MFSound_Stop(pVoice);
		pVoice = NULL;
	}
	if(pGuitarVoice)
	{
		MFSound_Stop(pGuitarVoice);
		pGuitarVoice = NULL;
	}
	if(pBassVoice)
	{
		MFSound_Stop(pBassVoice);
		pBassVoice = NULL;
	}
*/
}

void dBChart::SetVolume(float volume)
{
	if(pStream)
		MFSound_SetVolume(MFSound_GetStreamVoice(pStream), gConfig.sound.musicLevel * volume);
	if(pGuitar)
		MFSound_SetVolume(MFSound_GetStreamVoice(pGuitar), gConfig.sound.guitarLevel * volume);
	if(pBass)
		MFSound_SetVolume(MFSound_GetStreamVoice(pBass), gConfig.sound.bassLevel * volume);
}

void dBChart::SetPan(float pan)
{
	if(pStream)
		MFSound_SetPan(MFSound_GetStreamVoice(pStream), pan);
	if(pGuitar)
		MFSound_SetPan(MFSound_GetStreamVoice(pGuitar), pan);
	if(pBass)
		MFSound_SetPan(MFSound_GetStreamVoice(pBass), pan);
}

int dBChart::GetLastNoteTick()
{
	MFCALLSTACKc;

	GHEvent *pLast = sync.Last();
	int lastTick = pLast ? pLast->tick : 0;

	for(int a=0; a<GHS_Max; a++)
	{
		pLast = notes[a].Last();
		if(pLast)
			lastTick = MFMax(pLast->tick, lastTick);
	}

	return lastTick;
}

int dBChart::GetStartBPM()
{
	MFCALLSTACKc;

	int startBPM = 120000;

	GHEvent *pEv = sync.First();

	while(pEv && pEv->tick == 0)
	{
		if(pEv->event == GHE_BPM || pEv->event == GHE_Anchor)
		{
			startBPM = pEv->parameter;
			break;
		}

		pEv = pEv->Next();
	}

	return startBPM;
}

#define CALCTIME(x, y) (((int64)(x)*y)/resolution)

void dBChart::CalculateNoteTimes(int stream, int startTick)
{
	MFCALLSTACKc;

	int offset = 0;
	int currentBPM = GetStartBPM();
	int64 microsecondsPerBeat = 60000000000LL / currentBPM;
	int64 playTime = startOffset;
	int64 tempoTime = 0;

	GHEvent *pEv = sync.First();

	while(pEv)
	{
		if(pEv->event == GHE_BPM || pEv->event == GHE_Anchor)
		{
			tempoTime = CALCTIME(pEv->tick - offset, microsecondsPerBeat);

			// calculate event time (if event is not an anchor)
			if(pEv->event != GHE_Anchor)
				pEv->time = playTime + tempoTime;

			// calculate note times
			GHEvent *pNote = notes[stream].GetNextEvent(offset);
			while(pNote)
			{
				pNote->time = playTime + CALCTIME(pNote->tick - offset, microsecondsPerBeat);
				pNote = pNote->NextBefore(pEv->tick);
			}

			// calculate event times
			pNote = events.GetNextEvent(offset);
			while(pNote)
			{
				pNote->time = playTime + CALCTIME(pNote->tick - offset, microsecondsPerBeat);
				pNote = pNote->NextBefore(pEv->tick);
			}

			// increment play time to BPM location
			if(pEv->event == GHE_Anchor)
				playTime = pEv->time;
			else
				playTime += tempoTime;

			// find if next event is an anchor or not
			bool bNextIsAnchor = false;

			GHEvent *pNE = pEv->Next();
			while(pNE && pNE->event != GHE_BPM)
			{
				if(pNE->event == GHE_Anchor)
				{
					bNextIsAnchor = true;
					break;
				}

				pNE = pNE->Next();
			}

			// if it is, we need to calculate the BPM for this interval
			if(bNextIsAnchor)
			{
				int64 timeDifference = pNE->time - pEv->time;
				int tickDifference = pNE->tick - pEv->tick;
				pEv->parameter = (int)(60000000000LL / ((timeDifference*(int64)GetRes()) / (int64)tickDifference));
			}

			// update BPM and microsecondsPerBeat
			currentBPM = pEv->parameter;
			microsecondsPerBeat = 60000000000LL / currentBPM;

			offset = pEv->tick;
		}
		else
		{
			pEv->time = playTime + CALCTIME(pEv->tick - offset, microsecondsPerBeat);
		}

		pEv = pEv->Next();
	}

	GHEvent *pNote = notes[stream].GetNextEvent(offset);
	while(pNote)
	{
		pNote->time = playTime + CALCTIME(pNote->tick - offset, microsecondsPerBeat);
		pNote = pNote->Next();
	}

	pNote = events.GetNextEvent(offset);
	while(pNote)
	{
		pNote->time = playTime + CALCTIME(pNote->tick - offset, microsecondsPerBeat);
		pNote = pNote->Next();
	}
}

int64 dBChart::CalculateTimeOfTick(int tick)
{
	MFCALLSTACKc;

	int64 time;
	int offset;
	int currentBPM;
	int64 microsecondsPerBeat;

	GHEvent *pEv = sync.GetMostRecentSyncEvent(tick);
	if(pEv)
	{
		time = pEv->time;
		offset = pEv->tick;
		currentBPM = pEv->parameter;
	}
	else
	{
		time = startOffset;
		offset = 0;
		currentBPM = GetStartBPM();
	}

	microsecondsPerBeat = 60000000000LL / currentBPM;

	if(offset < tick)
		time += CALCTIME(tick - offset, microsecondsPerBeat);

	return time;
}

int dBChart::CalculateTickAtTime(int64 time, int *pBPM)
{
	MFCALLSTACKc;

	int currentBPM;
	int64 lastEventTime;
	int lastEventOffset;

	GHEvent *pEv = sync.GetMostRecentSyncEventTime(time);

	if(pEv)
	{
		lastEventTime = pEv->time;
		lastEventOffset = pEv->tick;
		currentBPM = pEv->parameter;
	}
	else
	{
		lastEventTime = startOffset;
		lastEventOffset = 0;
		currentBPM = GetStartBPM();
	}

	if(pBPM)
		*pBPM = currentBPM;

	return lastEventOffset + (int)((time-lastEventTime)*(int64)(currentBPM*resolution) / 60000000000LL);
}

/*** Event ***/

void GHEventManager::Init()
{
	pNotes = NULL;
	numAllocated = 0;
	numNotes = 0;
}

void GHEventManager::Deinit()
{
	Clear();

	MFHeap_Free(pNotes);
	numAllocated = 0;
}

void GHEventManager::Clear()
{
	for(int a = 1; a <= numNotes; ++a)
	{
		if(pNotes[a].event == GHE_Event)
			MFHeap_Free(pNotes[a].pString);
	}

	MFZeroMemory(pNotes + 1, sizeof(GHEvent));
	numNotes = 0;
}

GHEvent* GHEventManager::AddEvent(GHEventType type, int tick, int key, int parameter)
{
	if(numNotes >= numAllocated - 2)
	{
		// resize the array
		int newCount = numAllocated != 0 ? numAllocated * 2 : 256;
		pNotes = (GHEvent*)MFHeap_Realloc(pNotes, sizeof(GHEvent) * newCount);
		MFZeroMemory(pNotes + numAllocated, sizeof(GHEvent) * numAllocated);
		numAllocated = newCount;
	}

	// copy the sentinel
	pNotes[numNotes+2] = pNotes[numNotes+1];

	// shift all notes forward by one
	int newNote = numNotes+1;
	for(; newNote>1; --newNote)
	{
		// stop when we reach the point that this note should be inserted
		if(pNotes[newNote-1].tick <= tick)
			break;
		pNotes[newNote] = pNotes[newNote-1];
	}

	// write note details..
	pNotes[newNote].event = type;
	pNotes[newNote].tick = tick;
	pNotes[newNote].time = 0;
	pNotes[newNote].parameter = parameter;
	pNotes[newNote].pString = NULL;
	pNotes[newNote].key = key;
	pNotes[newNote].flags = 0;
	++numNotes;

	return pNotes + newNote;
}

GHEvent* GHEventManager::AddStringEvent(GHEventType type, int tick, const char *pString)
{
	GHEvent *pEv = AddEvent(type, tick);

	// we'll stuff the string pointer in the 'key' field..
	pEv->pString = (char*)MFHeap_Alloc(MFString_Length(pString)+1);
	MFString_Copy(pEv->pString, pString);

	return pEv;
}

GHEvent* GHEventManager::RemoveEvent(GHEvent *pEvent)
{
	int offset = (int)(pEvent - pNotes);

	// if note was a string event, free the string...
	if(pNotes[offset].pString)
		MFHeap_Free(pNotes[offset].pString);

	// shuffle notes back by one..
	for(int a=offset; a<=numNotes; ++a)
	{
		pNotes[a] = pNotes[a+1];
	}

	--numNotes;

	return pEvent;
}

GHEvent* GHEventManager::GetNextEvent(int tick)
{
	MFCALLSTACKc;

	if(!numNotes)
		return NULL;

	// get the top bit
	int i = numNotes;
	int topBit = 0;
	while((i >>= 1))
		++topBit;
	i = topBit = 1 << topBit;

	// binary search bitchez!!
	int target = 0;
	while(topBit)
	{
		if(i > numNotes || pNotes[i].tick >= tick)
		{
			if(i <= numNotes)
				target = i;
			i = (i & ~topBit) | topBit>>1;
		}
		else
			i |= topBit>>1;

		topBit >>= 1;
	}

	return target ? pNotes + target : NULL;
}

GHEvent* GHEventManager::GetNextEventTime(int64 time)
{
	MFCALLSTACKc;

	if(!numNotes)
		return NULL;

	// get the top bit
	int i = numNotes;
	int topBit = 0;
	while((i >>= 1))
		++topBit;
	i = topBit = 1 << topBit;

	// binary search bitchez!!
	int target = 0;
	while(topBit)
	{
		if(i > numNotes || pNotes[i].time >= time)
		{
			if(i <= numNotes)
				target = i;
			i = (i & ~topBit) | topBit>>1;
		}
		else
			i |= topBit>>1;

		topBit >>= 1;
	}

	return target ? pNotes + target : NULL;
}

GHEvent* GHEventManager::FindEvent(int tick)
{
	MFCALLSTACKc;

	if(!numNotes)
		return NULL;

	// get the top bit
	int i = numNotes;
	int topBit = 0;
	while((i >>= 1))
		++topBit;
	i = topBit = 1 << topBit;

	// binary search bitchez!!
	int target = 0;
	while(topBit)
	{
		if(i <= numNotes && pNotes[i].tick == tick)
		{
			while(i > 1 && pNotes[i-1].tick == pNotes[i].tick)
				--i;
			target = i;
			return pNotes + target;
		}

		if(i > numNotes || pNotes[i].tick >= tick)
			i = (i & ~topBit) | topBit>>1;
		else
			i |= topBit>>1;

		topBit >>= 1;
	}

	return NULL;
}

GHEvent* GHEventManager::FindEvent(GHEventType type, int tick, int key)
{
	MFCALLSTACKc;

	// find the first event at the requested time
	GHEvent *pEv = FindEvent(tick);
	if(!pEv)
		return NULL;

	int i = (int)(pEv - pNotes);

	// match the other conditions
	for(; i <= numNotes && pNotes[i].tick == tick; ++i)
	{
		if((!type || pNotes[i].event == type) && pNotes[i].key == key)
			return pNotes + i;
	}

	return NULL;
}

GHEvent* GHEventManager::GetMostRecentEvent(GHEventType type, int offset)
{
	MFCALLSTACKc;

	GHEvent *pEv = GetNextEvent(offset);
	if(!pEv)
		pEv = Last();
	else
		--pEv;

	for(; pEv > pNotes; --pEv)
	{
		if(!type || pEv->event == type)
			return pEv;
	}

	return NULL;
}

GHEvent* GHEventManager::GetMostRecentEventTime(GHEventType type, int64 time)
{
	MFCALLSTACKc;

	GHEvent *pEv = GetNextEventTime(time);
	if(!pEv)
		pEv = Last();
	else
		--pEv;

	for(; pEv > pNotes; --pEv)
	{
		if(!type || pEv->event == type)
			return pEv;
	}

	return NULL;
}

GHEvent* GHEventManager::GetMostRecentSyncEvent(int tick)
{
	GHEvent *pEv1 = GetMostRecentEvent(GHE_BPM, tick);
	GHEvent *pEv2 = GetMostRecentEvent(GHE_Anchor, tick);

	if(pEv1 && pEv2)
		pEv1 = pEv1->tick > pEv2->tick ? pEv1 : pEv2;

	return pEv1;
}

GHEvent* GHEventManager::GetMostRecentSyncEventTime(int64 time)
{
	GHEvent *pEv1 = GetMostRecentEventTime(GHE_BPM, time);
	GHEvent *pEv2 = GetMostRecentEventTime(GHE_Anchor, time);

	if(pEv1 && pEv2)
		pEv1 = pEv1->time > pEv2->time ? pEv1 : pEv2;

	return pEv1;
}

void GHEventManager::ResetNotes()
{
	for(int a = 1; a <= numNotes; ++a)
		pNotes[a].played = false;
}

