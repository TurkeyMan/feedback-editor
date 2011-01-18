#if !defined(_DBSONG_H)
#define _DBSONG_H

enum GHStreams
{
	GHS_Unknown = -1,
	GHS_EasySingle = 0,
	GHS_EasyDoubleGuitar,
	GHS_EasyDoubleBass,
	GHS_EasyEnhancedGuitar,
	GHS_EasyEnhancedCoopLead,
	GHS_EasyEnhancedCoopBass,
	GHS_Easy10KeyGuitar,
	GHS_EasyDrums,
	GHS_EasyDoubleDrums,
	GHS_EasyVocals,
	GHS_EasyKeyboard,
	GHS_MediumSingle,
	GHS_MediumDoubleGuitar,
	GHS_MediumDoubleBass,
	GHS_MediumEnhancedGuitar,
	GHS_MediumEnhancedCoopLead,
	GHS_MediumEnhancedCoopBass,
	GHS_Medium10KeyGuitar,
	GHS_MediumDrums,
	GHS_MediumDoubleDrums,
	GHS_MediumVocals,
	GHS_MediumKeyboard,
	GHS_HardSingle,
	GHS_HardDoubleGuitar,
	GHS_HardDoubleBass,
	GHS_HardEnhancedGuitar,
	GHS_HardEnhancedCoopLead,
	GHS_HardEnhancedCoopBass,
	GHS_Hard10KeyGuitar,
	GHS_HardDrums,
	GHS_HardDoubleDrums,
	GHS_HardVocals,
	GHS_HardKeyboard,
	GHS_ExpertSingle,
	GHS_ExpertDoubleGuitar,
	GHS_ExpertDoubleBass,
	GHS_ExpertEnhancedGuitar,
	GHS_ExpertEnhancedCoopLead,
	GHS_ExpertEnhancedCoopBass,
	GHS_Expert10KeyGuitar,
	GHS_ExpertDrums,
	GHS_ExpertDoubleDrums,
	GHS_ExpertVocals,
	GHS_ExpertKeyboard,

	GHS_Max,
	GHS_NumTracks = GHS_MediumSingle,
	GHS_NumDifficulties = GHS_Max / GHS_NumTracks
};

enum GHEventType
{
	GHE_Unknown = 0,

	GHE_Note,
	GHE_BPM,
	GHE_Anchor,
	GHE_TimeSignature,
	GHE_Special,
	GHE_Event,
	GHE_HandPos,

	GHE_Max
};

enum Player2Type
{
	GHP2T_Bass,
	GHP2T_Rhythm
};

enum GHEventKey
{
	GHEK_Green,
	GHEK_Red,
	GHEK_Yellow,
	GHEK_Blue,
	GHEK_Orange,

	GHEK_NumKeys
};

enum GHSpecial
{
	GHS_Player1,
	GHS_Player2,
	GHS_Super
};

enum GHEventFlags
{
	GHEK_Tap = 0x1
};

struct GHSyncEvent
{
	int64 time;		// the physical time of the note (in microseconds)
	int tick;		// in ticks
	int bpm;		// in thousandths of a beat per minute
	GHEventType event;
};

struct GHEvent
{
	GHEventType event;

	int64 time;		// the physical time of the note (in microseconds)
	int tick;		// in ticks

	int key;
	int parameter;	// for a note, this is the sustain, for a BPM, this is the BPM in thousandths of a beat per minute..
	char *pString;
	uint32 flags;

	// temp runtime data
	int played;

	GHEvent *Next() { return this[1].event != GHE_Unknown ? this + 1 : NULL; }
	GHEvent *Prev() { return this[-1].event != GHE_Unknown ? this - 1 : NULL; }
	GHEvent *NextSibling() { return this[1].event != GHE_Unknown && this[1].tick == tick ?  this + 1 : NULL; }
	GHEvent *NextBefore(int tick) { return this[1].event != GHE_Unknown && this[1].tick < tick ? this + 1 : NULL; }
	GHEvent *NextBeforeTime(int64 time) { return this[1].event != GHE_Unknown && this[1].time < time ? this + 1 : NULL; }

	const char *GetString() { return pString; }
};

class GHEventManager
{
public:
	void Init(int numNotes = 256);
	void Deinit();

	void Clear();

	GHEvent* AddEvent(GHEventType type, int tick, int button = 0, int parameter = 0);
	GHEvent* AddStringEvent(GHEventType type, int tick, const char *pString);
	GHEvent* RemoveEvent(GHEvent *pEvent);
	GHEvent* FindEvent(int tick);
	GHEvent* FindEvent(GHEventType type, int tick, int button = 0);
	GHEvent* GetMostRecentEvent(GHEventType type, int tick);
	GHEvent* GetMostRecentEventTime(GHEventType type, int64 time);
	GHEvent* GetMostRecentSyncEvent(int tick);
	GHEvent* GetMostRecentSyncEventTime(int64 time);

	GHEvent* GetNextEvent(int tick);
	GHEvent* GetNextEventTime(int64 time);

	GHEvent* First() { return numNotes ? pNotes + 1 : NULL; }
	GHEvent* Last() { return numNotes ? pNotes + numNotes : NULL; }

	void ResetNotes();

	GHEvent *pNotes;
	int numAllocated;
	int numNotes;
};

class dBChart
{
public:
	dBChart();
	~dBChart();

	static dBChart *Create(const char *pMusic = NULL);
	static dBChart *LoadChart(const char *pChart);
	static dBChart *LoadMidi(const char *pMidi);

	void Destroy();

	void SaveChart();

	void Play(int64 time);
	void Stop();

	void SetVolume(float volume);
	void SetPan(float pan);

	int GetLastNoteTick();
	int GetStartBPM();

	void CalculateNoteTimes(int stream, int startTick);
	int64 CalculateTimeOfTick(int tick);
	int CalculateTickAtTime(int64 time, int *pBPM = NULL);

	const char *GetDifficultyName(int difficulty);
	const char *GetTrackName(int track);

	int GetRes() { return resolution; }
	int GetTick(float beat) { return (int)(beat * (float)resolution); }

	MFAudioStream *PlayStream(const char *pFilename);

	char songName[256];
	char artistName[256];
	char charterName[256];
	char songPath[256];

	char musicFilename[256];
	char guitarFilename[256];
	char bassFilename[256];

	char fretboard[256];

	char musicURL[256];
	char previewURL[256];

	char genre[256];
	char mediaType[256];

	float previewStart;
	float previewEnd;

	int player2Type;

	int difficulty;

	int64 startOffset;		// starting offset, in microseconds

	int resolution;

	GHEventManager sync;	// song sync stuff
	GHEventManager events;	// general song events, specials, etc
	GHEventManager notes[GHS_Max];

	MFAudioStream *pStream;
	MFAudioStream *pGuitar;
	MFAudioStream *pBass;

	MFMaterial *pFretboard;
};

#endif
