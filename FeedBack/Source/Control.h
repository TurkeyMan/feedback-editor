#if !defined(_CONTROL_H)
#define _CONTROL_H

#include "MFInput.h"
#include "MFPtrList.h"

class MFIniLine;

enum dBControlType
{
	dBCtrl_MenuStart = 0,
		dBCtrl_Menu_Up = dBCtrl_MenuStart,
		dBCtrl_Menu_Down,
		dBCtrl_Menu_Home,
		dBCtrl_Menu_End,
		dBCtrl_Menu_PgUp,
		dBCtrl_Menu_PgDn,
		dBCtrl_Menu_Accept,
		dBCtrl_Menu_Cancel,

	dBCtrl_GameStart,
		dBCtrl_Game_G1_Green = dBCtrl_GameStart,
		dBCtrl_Game_G1_Red,
		dBCtrl_Game_G1_Yellow,
		dBCtrl_Game_G1_Blue,
		dBCtrl_Game_G1_Orange,
		dBCtrl_Game_G1_StrumUp,
		dBCtrl_Game_G1_StrumDown,
		dBCtrl_Game_G1_Start,
		dBCtrl_Game_G1_Select,
		dBCtrl_Game_G1_Tilt,
		dBCtrl_Game_G1_Whammy,

		dBCtrl_Game_G2_Green,
		dBCtrl_Game_G2_Red,
		dBCtrl_Game_G2_Yellow,
		dBCtrl_Game_G2_Blue,
		dBCtrl_Game_G2_Orange,
		dBCtrl_Game_G2_StrumUp,
		dBCtrl_Game_G2_StrumDown,
		dBCtrl_Game_G2_Start,
		dBCtrl_Game_G2_Select,
		dBCtrl_Game_G2_Tilt,
		dBCtrl_Game_G2_Whammy,

		dBCtrl_Game_Drum_Hat,
		dBCtrl_Game_Drum_Snare,
		dBCtrl_Game_Drum_Kick,
		dBCtrl_Game_Drum_Tom1,
		dBCtrl_Game_Drum_Tom2,
		dBCtrl_Game_Drum_Ride,
		dBCtrl_Game_Drum2_Hat,
		dBCtrl_Game_Drum2_Snare,
		dBCtrl_Game_Drum2_Kick,
		dBCtrl_Game_Drum2_Tom1,
		dBCtrl_Game_Drum2_Tom2,
		dBCtrl_Game_Drum2_Ride,

	dBCtrl_EditStart,
		dBCtrl_Edit_Forward = dBCtrl_EditStart,
		dBCtrl_Edit_Back,
		dBCtrl_Edit_UpMeasure,
		dBCtrl_Edit_DownMeasure,
		dBCtrl_Edit_NextSection,
		dBCtrl_Edit_PrevSection,
		dBCtrl_Edit_End,
		dBCtrl_Edit_Start,
		dBCtrl_Edit_RangeSelect,
		dBCtrl_Edit_Quantise,
		dBCtrl_Edit_QuantiseUp,
		dBCtrl_Edit_QuantiseDown,
		dBCtrl_Edit_IncreaseBPM,
		dBCtrl_Edit_DecreaseBPM,
		dBCtrl_Edit_IncreaseOffset,
		dBCtrl_Edit_DecreaseOffset,
		dBCtrl_Edit_IncreaseTS,
		dBCtrl_Edit_DecreaseTS,
		dBCtrl_Edit_Anchor,
		dBCtrl_Edit_Event,
		dBCtrl_Edit_TrackEvent,
		dBCtrl_Edit_Section,
		dBCtrl_Edit_Note0,	// green/hat
		dBCtrl_Edit_Note1,	// red/snare
		dBCtrl_Edit_Note2,	// yellow/kick
		dBCtrl_Edit_Note3,	// blue/tom1
		dBCtrl_Edit_Note4,	// orange/tom2
		dBCtrl_Edit_Note5,	// special/ride
		dBCtrl_Edit_PS1,	// player1 section
		dBCtrl_Edit_PS2,	// player2 section
		dBCtrl_Edit_ShiftForwards,
		dBCtrl_Edit_ShiftBackwards,
		dBCtrl_Edit_ShiftLeft,
		dBCtrl_Edit_ShiftRight,
		dBCtrl_Edit_Cut,
		dBCtrl_Edit_Copy,
		dBCtrl_Edit_Paste,
		dBCtrl_Edit_Delete,
		dBCtrl_Edit_Goto,
		dBCtrl_Edit_Save,

		dBCtrl_Edit_StartPlayback,
		dBCtrl_Edit_RestartPlayback,
		dBCtrl_Edit_StopPlayback,
		dBCtrl_Edit_StopReturn,
		dBCtrl_Edit_PlaybackRate,
		dBCtrl_Edit_Metronome,
		dBCtrl_Edit_Clap,
		dBCtrl_Edit_Hyperspeed,
		dBCtrl_Edit_View,

		dBCtrl_Edit_Help,
		dBCtrl_Edit_Menu,
		dBCtrl_Edit_PopupMenu,

		dBCtrl_Edit_Mixer,
		dBCtrl_Edit_VolumeUp,
		dBCtrl_Edit_VolumeDown,

		dBCtrl_Edit_SwapTracks,
		dBCtrl_Edit_Easy,
		dBCtrl_Edit_Medium,
		dBCtrl_Edit_Hard,
		dBCtrl_Edit_Expert,
		dBCtrl_Edit_Track1,
		dBCtrl_Edit_Track2,
		dBCtrl_Edit_Track3,
		dBCtrl_Edit_Track4,
		dBCtrl_Edit_Track5,
		dBCtrl_Edit_Track6,
		dBCtrl_Edit_Track7,
		dBCtrl_Edit_Track8,
		dBCtrl_Edit_Track9,
		dBCtrl_Edit_Track10,
		dBCtrl_Edit_Track11,

	dBCtrl_Max,

	dBCtrl_MenuCount = dBCtrl_GameStart - dBCtrl_MenuStart,
	dBCtrl_GameCount = dBCtrl_EditStart - dBCtrl_GameStart,
	dBCtrl_EditCount = dBCtrl_Max - dBCtrl_EditStart,

	dBCtrl_ForceInt = 0x7FFFFFFF
};

enum GHControlType
{
	GHCT_None = -1,
	GHCT_Once = 0,
	GHCT_Delay,
	GHCT_Repeat,
	GHCT_Hold,
	GHCT_Release,

	GHCT_Max,
	GHCT_ForceInt = 0x7FFFFFFF
};

enum GHControlMod
{
	GHCM_Shift = 0x1,
	GHCM_Ctrl = 0x2,
	GHCM_Alt = 0x4,

	GHCM_IgnoreShift = 0x10,
	GHCM_IgnoreCtrl = 0x20,
	GHCM_IgnoreAlt = 0x40,

	GHCM_IgnoreAll = GHCM_IgnoreShift | GHCM_IgnoreCtrl | GHCM_IgnoreAlt,

	GHCM_ForceInt = 0x7FFFFFFF
};

struct GHButton
{
	GHButton() { device = deviceID = 0; button = -1; }
	void Set(int _button, int _device = IDD_Keyboard, int _deviceID = 0) { device = _device; deviceID = _deviceID; button = _button; }

	int device;
	int deviceID;
	int button;
};

struct GHControlData
{
	void Reset() { holdKeys[0].button = holdKeys[1].button = -1; }
	const char* GetString(bool withExclusions = true);
	void FromString(const char *pString);

	uint32 mod;
	GHButton holdKeys[2];
	GHButton trigger;
};

class GHControlUnit
{
public:
	bool Test(uint32 mods, GHControlType type);

	GHControlData data;
	float timeout;
	bool held;
	float timeout2;
	bool held2;
	int frameCount;
};

class GHControl
{
	friend class HelpScreen;
public:
	void Set(GHControlData *pControlData, GHControlData *pControlData2 = NULL);
	void Set(MFIniLine *pLine);
	void Set(int key, uint32 mod = 0, int type = IDD_Keyboard, int holdKey = -1, int secondTrigger = -1, int secondType = IDD_Gamepad, int holdKey2 = -1);
	bool Test(GHControlType type);

	static void InitControls();
	static void UpdateControls();
	static const char *GetControlString(int id) { return dBControlStrings[id]; }
	static const char *GetSettingsString(dBControlType control);

protected:
	void Update(uint32 mods);

	GHControlUnit data[2];
	bool pressedType[GHCT_Max];
	bool pressed;

	static const char *dBControlStrings[];
	static const char *gpControlModes[GHCT_Max];
};

#endif
