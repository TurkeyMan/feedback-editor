#if !defined(_FEEDBACK_H)
#define _FEEDBACK_H

#include "Fuji.h"

#include "MFInput.h"
#include "MFMaterial.h"
#include "MFModel.h"
#include "MFSound.h"
#include "MFFont.h"
#include "MFView.h"
#include "MFDisplay.h"
#include "MFPrimitive.h"
#include "MFRenderer.h"
#include "MFSystem.h"
#include "MFTranslation.h"
#include "MFArray.h"

#include "Settings.h"
#include "Song.h"
#include "Screens/Screen.h"
#include "GHScreen.h"
#include "Help.h"
#include "Menu.h"
#include "MsgBox.h"
#include "StringBox.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "FileSelector.h"
#include "Mixer.h"
#include "FingerEditor.h"
#include "Game.h"
#include "OldEditor.h"
#include "Player.h"
#include "Song.h"

#include "../Data/Strings.h"

#define GETSECONDS(time) ((float)(time)*(1.0f/1000000.0f))

#define KEY_PAUSE 0.3f
#define KEY_REPEAT 0.1f

#define NOTE_WINDOW 100000

#if defined(_PSP)
	#define TEXT_HEIGHT 32
	#define HELP_TEXT_HEIGHT 32
#else
	#define TEXT_HEIGHT 16
	#define HELP_TEXT_HEIGHT 20
#endif

struct dBThemeList
{
	char themeName[256];
	char themePath[256];
};

struct dBSongList
{
	char chartPath[256];
	dBChart *pChart;
};

struct dBSongGroup
{
	MFArray<dBSongList> songs;
	MFMaterial *pGroupLogo;
	char groupName[256];
};

struct dBPackage
{
	char packageFilename[256];
	char mountPoint[256];
};

struct dBGameData
{
	MFArray<dBThemeList> themes;
	MFArray<dBSongGroup> songGroups;
	MFArray<dBPackage> packages;

	dBThemeList *pCurrentTheme;
};

enum GHPlayState
{
	GHPS_Stopped,
	GHPS_Playing,
	GHPS_Recording
};

class GameState
{
public:
	GameState() {}
	virtual ~GameState() {}

	static void Update()
	{
		if(pCurrentState)
			pCurrentState->UpdateState();

		if(pNextState)
		{
			if(pCurrentState)
				pCurrentState->Deinit();

			pCurrentState = pNextState;
			pNextState = NULL;

			pCurrentState->Init();

			Update();
		}
	}

	static void Draw()
	{
		if(pCurrentState)
			pCurrentState->DrawState();
		else
			MFDebug_Warn(2, "No game state selected.");
	}

	GameState* Select()
	{
		pNextState = this;
		return pCurrentState;
	}

protected:
	virtual void Init() = 0;
	virtual void Deinit() = 0;

	virtual void UpdateState() = 0;
	virtual void DrawState() = 0;

	static GameState *pCurrentState;
	static GameState *pNextState;
};

struct GHEditor
{
	GHEditor();

	dBChart *pSong;

	GHPlayState state;		// play state
	int offset;				// current offset into the song
	int measure;			// current measure
	int beat;				// current beat in the measure (relative to the quantisation)
	int currentBPM;
	int currentTimeSignature;
	int lastTimeSignature;
	float speedMultiplier;

	int selectedStream;
	int currentStream[2];	// stream being edited

	int quantisation;		// 1, 2, 3, 4, 6, 8, 16, 24, 32 (fraction of a bar ie, quantise time = 1/quantisation)
	int quantiseStep;

	int selectStart;		// start of select range
	int selectEnd;			// end of select range
	int copyLen;			// length of selection

	bool metronome;
	bool clap;

	int playbackStartOffset;
	int64 playingTime;		// physical time at current offset in microseconds
	int64 startTime;		// chart time when playback began in microseconds
	uint64 playStart;		// moment that playback began
	uint64 lastTimestamp;	// timestamp at last frame

	float musicPan;

//private:
	// internal stuff
	MFMaterial *pMetronome;

	MFSound *pStepSound;
	MFSound *pChangeSound;
	MFSound *pSaveSound;
	MFSound *pClapSound;
	MFSound *pHighTickSound;
	MFSound *pLowTickSound;

	GHEventManager selectEvents;
};

extern dBSettings gConfig;
extern dBGameData gGameData;

extern MFFont *pHeading;
extern MFFont *pText;

extern MFStringTable *pStrings;

extern int gQuantiseSteps[];
extern int gButtons[];

void CenterText(float y, float height, const MFVector &colour, const char *pText, MFFont *pFont, bool outline = false, const MFVector &outlineColour = MFVector::black);

float TestControl(dBControlType ctrl, GHControlType type);

#endif
