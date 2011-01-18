#include "FeedBack.h"
#include "MFFileSystem.h"
#include "MFHeap.h"

#include "MidiFile.h"

void WriteVarLen(char *&pBuffer, uint32 value)
{
	uint32 buffer;
	buffer = value & 0x7F;

	while((value >>= 7))
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
	}

	while(1)
	{
		*pBuffer++ = buffer&0xFF;
		if(buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
}

uint32 ReadVarLen(const char *&pBuffer)
{
	uint32 value;
	uint8 c;

	if((value = *pBuffer++) & 0x80)
	{
		value &= 0x7F;
		do
		{
			value = (value << 7) + ((c = *pBuffer++) & 0x7F);
		}
		while(c & 0x80);
	}

	return value;
}

MIDIFile* LoadMidiFromFile(const char *pFile, uint32 size)
{
	if(!size)
		pFile = MFFileSystem_Load(pFile, &size);

	if(!pFile)
		return NULL;

	const char *pOffset = pFile;

	if(*(uint32*)pFile == MFMAKEFOURCC('R','I','F','F'))
	{
		pOffset += 8;
		MFDebug_Assert(*(uint32*)pFile == MFMAKEFOURCC('R','M','I','D'), "Not a midi file...");
		pOffset += 4;
	}

	MTHD_CHUNK *pHd = (MTHD_CHUNK*)pOffset;
	MFDebug_Assert(pHd->id == MFMAKEFOURCC('M','T','h','d'), "Not a midi file...");

	MFEndian_BigToHost(&pHd->length);
	MFEndian_BigToHost(&pHd->format);
	MFEndian_BigToHost(&pHd->numTracks);
	MFEndian_BigToHost(&pHd->ticksPerBeat);

	MIDIFile *pMidiFile = (MIDIFile*)MFHeap_Alloc(sizeof(MIDIFile) + sizeof(MIDIEvent*) * pHd->numTracks);

	pMidiFile->name[0] = 0;
	pMidiFile->format = pHd->format;
	pMidiFile->ticksPerBeat = pHd->ticksPerBeat;
	pMidiFile->numTracks = pHd->numTracks;
	pMidiFile->ppTracks = (MIDIEvent**)&pMidiFile[1];
	MFZeroMemory(pMidiFile->ppTracks, sizeof(MIDIEvent*)*pMidiFile->numTracks);

	// we will only deal with type 1 midi files here..
	MFDebug_Assert(pHd->format == 1, "Invalid midi type.");

	pOffset += 8 + pHd->length;

	for(int t = 0; t < pMidiFile->numTracks && pOffset < pFile + size; ++t)
	{
		MTRK_CHUNK track;
		MFCopyMemory(&track.id, pOffset, sizeof(track.id)); pOffset += sizeof(track.id);
		MFCopyMemory(&track.length, pOffset, sizeof(track.length)); pOffset += sizeof(track.length);
		MFEndian_BigToHost(&track.length);

		if(track.id == MFMAKEFOURCC('M','T','r','k'))
		{
			const char *pTrk = pOffset;
			uint32 tick = 0;

			while(pTrk < pOffset + track.length)
			{
				uint32 delta = ReadVarLen(pTrk);
				tick += delta;
				uint8 status = *(uint8*)pTrk++;

				MIDIEvent *pEvent = NULL;

				if(status == 0xFF)
				{
					// non-midi event
					uint8 type = *(uint8*)pTrk++;
					uint32 bytes = ReadVarLen(pTrk);

					// read event
					switch(type)
					{
						case MEV_SequenceNumber:
						{
							static int sequence = 0;

							MIDIEvent_SequenceNumber *pSeq = (MIDIEvent_SequenceNumber*)MFHeap_Alloc(sizeof(MIDIEvent_SequenceNumber));
							pEvent = pSeq;

							if(!bytes)
								pSeq->sequence = sequence++;
							else
							{
								uint16 seq;
								MFCopyMemory(&seq, pTrk, 2);
								MFEndian_BigToHost(&seq);
								pSeq->sequence = (int)seq;
							}
							break;
						}
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
							MIDIEvent_Text *pText = (MIDIEvent_Text*)MFHeap_Alloc(sizeof(MIDIEvent_Text));
							pEvent = pText;
							MFCopyMemory(pText->buffer, pTrk, bytes);
							pText->buffer[bytes] = 0;
							break;
						}
						case MEV_EndOfTrack:
						{
							MIDIEvent *pE = (MIDIEvent*)MFHeap_Alloc(sizeof(MIDIEvent));
							pEvent = pE;
//							MFDebug_Assert(pTrk == pOffset + track.length, "Track seems to end prematurely...");
							pTrk = pOffset + track.length;
							break;
						}
						case MEV_Tempo:
						{
							MIDIEvent_Tempo *pTempo = (MIDIEvent_Tempo*)MFHeap_Alloc(sizeof(MIDIEvent_Tempo));
							pEvent = pTempo;
							pTempo->microsecondsPerBeat = ((uint8*)pTrk)[0] << 16;
							pTempo->microsecondsPerBeat |= ((uint8*)pTrk)[1] << 8;
							pTempo->microsecondsPerBeat |= ((uint8*)pTrk)[2];
							pTempo->BPM = 60000000.0f/(float)pTempo->microsecondsPerBeat;
							break;
						}
						case MEV_SMPTE:
						{
							MIDIEvent_SMPTE *pSMPTE = (MIDIEvent_SMPTE*)MFHeap_Alloc(sizeof(MIDIEvent_SMPTE));
							pEvent = pSMPTE;
							pSMPTE->hours = ((uint8*)pTrk)[0];
							pSMPTE->minutes = ((uint8*)pTrk)[1];
							pSMPTE->seconds = ((uint8*)pTrk)[2];
							pSMPTE->frames = ((uint8*)pTrk)[3];
							pSMPTE->subFrames = ((uint8*)pTrk)[4];
							break;
						}
						case MEV_TimeSignature:
						{
							MIDIEvent_TimeSignature *pTS = (MIDIEvent_TimeSignature*)MFHeap_Alloc(sizeof(MIDIEvent_TimeSignature));
							pEvent = pTS;
							pTS->numerator = ((uint8*)pTrk)[0];
							pTS->denominator = ((uint8*)pTrk)[1];
							pTS->clocks = ((uint8*)pTrk)[2];
							pTS->d = ((uint8*)pTrk)[3];
							break;
						}
						case MEV_KeySignature:
						{
							MIDIEvent_KeySignature *pKS = (MIDIEvent_KeySignature*)MFHeap_Alloc(sizeof(MIDIEvent_KeySignature));
							pEvent = pKS;
							pKS->sf = ((uint8*)pTrk)[0];
							pKS->minor = ((uint8*)pTrk)[1];
							break;
						}
						case MEV_Custom:
						{
							MIDIEvent_Custom *pCustom = (MIDIEvent_Custom*)MFHeap_Alloc(sizeof(MIDIEvent_Custom));
							pEvent = pCustom;
							pCustom->pData = pTrk;
							pCustom->size = bytes;
							break;
						}
					}

					if(pEvent)
						pEvent->subType = type;

					pTrk += bytes;
				}
				else if(status == 0xF0)
				{
					uint32 bytes = ReadVarLen(pTrk);

					// SYSEX event...
					MFDebug_Log(2, "Encountered SYSEX event...");

					pTrk += bytes;
				}
				else
				{
					static int lastStatus = 0;

					if(status < 0x80)
					{
						--pTrk;
						status = lastStatus;
					}

					lastStatus = status;

					int eventType = status&0xF0;

					int param1 = ReadVarLen(pTrk);
					int param2 = 0;
					if(eventType != MNE_ProgramChange && eventType != MNE_ChannelAfterTouch)
						param2 = ReadVarLen(pTrk);

					switch(eventType)
					{
						case MNE_NoteOn:
						case MNE_NoteOff:
						{
							MIDIEvent_Note *pNote = (MIDIEvent_Note*)MFHeap_Alloc(sizeof(MIDIEvent_Note));
							pEvent = pNote;
							pNote->event = status&0xF0;
							pNote->channel = status&0x0F;
							pNote->note = param1;
							pNote->velocity = param2;
							break;
						}
					}

					if(pEvent)
					{
						pEvent->subType = status;
						status = MET_Note;
					}
				}

				// append event to track
				if(pEvent)
				{
					pEvent->tick = tick;
					pEvent->delta = delta;
					pEvent->type = status;
					pEvent->pNext = NULL;

					MIDIEvent *pEv = pMidiFile->ppTracks[t];

					if(!pEv)
					{
						pMidiFile->ppTracks[t] = pEvent;
					}
					else
					{
						while(pEv->pNext)
							pEv = pEv->pNext;
						pEv->pNext = pEvent;
					}
				}
			}
		}

		pOffset += track.length;
	}

	return pMidiFile;
}

void FreeMidi(MIDIFile *pMidi)
{

}
