#if !defined(_MIDIFILE_H)
#define _MIDIFILE_H

enum MIDIEventType
{
	MET_Note = 0,
	MET_SYSEX = 0xF0,
	MET_Custom = 0xFF
};

enum MIDINoteEvent
{
	MNE_NoteOff = 0x80,
	MNE_NoteOn = 0x90,
	MNE_KeyAfterTouch = 0xA0,
	MNE_ControlChange = 0xB0,
	MNE_ProgramChange = 0xC0,
	MNE_ChannelAfterTouch = 0xD0,
	MNE_PitchWheel = 0xE0
};

enum MIDIEvents
{
	MEV_SequenceNumber = 0x00, // Sequence Number
	MEV_Text = 0x01, // Text
	MEV_Copyright = 0x02, // Copyright
	MEV_TrackName = 0x03, // Sequence/Track Name
	MEV_Instrument = 0x04, // Instrument
	MEV_Lyric = 0x05, // Lyric
	MEV_Marker = 0x06, // Marker
	MEV_CuePoint = 0x07, // Cue Point
	MEV_PatchName = 0x08, // Program (Patch) Name
	MEV_PortName = 0x09, // Device (Port) Name
	MEV_EndOfTrack = 0x2F, // End of Track
	MEV_Tempo = 0x51, // Tempo
	MEV_SMPTE = 0x54, // SMPTE Offset
	MEV_TimeSignature = 0x58, // Time Signature
	MEV_KeySignature = 0x59, // Key Signature
	MEV_Custom = 0x7F, // Proprietary Event
};

struct MTHD_CHUNK
{
	uint32	id;   // 'M','T','h','d'
	uint32	length;

	uint16	format;
	uint16	numTracks;
	uint16	ticksPerBeat;
};

struct MTRK_CHUNK
{
	uint32	id;   // 'M','T','r','k' */
	uint32	length;
};

struct MIDIEvent
{
	uint32 tick;
	uint32 delta;
	uint32 type;
	uint32 subType;

	MIDIEvent *pNext;
};

struct MIDIFile
{
	char name[512];

	int format;
	int ticksPerBeat;

	MIDIEvent **ppTracks;
	int numTracks;
};

struct MIDIEvent_Note : public MIDIEvent
{
	int event;
	int channel;
	int note;
	int velocity;
};

struct MIDIEvent_SequenceNumber : public MIDIEvent
{
	int sequence;
};

struct MIDIEvent_Text : public MIDIEvent
{
	char buffer[512];
};

struct MIDIEvent_Tempo : public MIDIEvent
{
	float BPM;
	int microsecondsPerBeat;
};

struct MIDIEvent_SMPTE : public MIDIEvent
{
	uint8 hours, minutes, seconds, frames, subFrames;
};

struct MIDIEvent_TimeSignature : public MIDIEvent
{
	uint8 numerator, denominator;
	uint8 clocks;
	uint8 d;
};

struct MIDIEvent_KeySignature : public MIDIEvent
{
	uint8 sf;
	uint8 minor;
};

struct MIDIEvent_Custom : public MIDIEvent
{
	const char *pData;
	uint32 size;
};

MIDIFile* LoadMidiFromFile(const char *pFile, size_t size = 0);
void FreeMidi(MIDIFile *pMidi);

#endif
