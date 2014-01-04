#include "FeedBack.h"
#include "Fuji/MFIni.h"

#include "Control.h"

#include <string.h>

const char *GHControl::dBControlStrings[dBCtrl_Max] =
{
	"Menu_Up",
	"Menu_Down",
	"Menu_Home",
	"Menu_End",
	"Menu_PageUp",
	"Menu_PageDown",
	"Menu_Accept",
	"Menu_Cancel",
	"Game_G1_Green",
	"Game_G1_Red",
	"Game_G1_Yellow",
	"Game_G1_Blue",
	"Game_G1_Orange",
	"Game_G1_StrumUp",
	"Game_G1_StrumDown",
	"Game_G1_Start",
	"Game_G1_Select",
	"Game_G1_Tilt",
	"Game_G1_Whammy",
	"Game_G2_Green",
	"Game_G2_Red",
	"Game_G2_Yellow",
	"Game_G2_Blue",
	"Game_G2_Orange",
	"Game_G2_StrumUp",
	"Game_G2_StrumDown",
	"Game_G2_Start",
	"Game_G2_Select",
	"Game_G2_Tilt",
	"Game_G2_Whammy",
	"Game_Drum_Hat",
	"Game_Drum_Snare",
	"Game_Drum_Kick",
	"Game_Drum_Tom1",
	"Game_Drum_Tom2",
	"Game_Drum_Ride",
	"Game_Drum2_Hat",
	"Game_Drum2_Snare",
	"Game_Drum2_Kick",
	"Game_Drum2_Tom1",
	"Game_Drum2_Tom2",
	"Game_Drum2_Ride",
	"Edit_Forward",
	"Edit_Back",
	"Edit_UpMeasure",
	"Edit_DownMeasure",
	"Edit_NextSection",
	"Edit_PrevSection",
	"Edit_End",
	"Edit_Start",
	"Edit_RangeSelect",
	"Edit_SetQuantise",
	"Edit_QuantiseUp",
	"Edit_QuantiseDown",
	"Edit_IncreaseBPM",
	"Edit_DecreaseBPM",
	"Edit_IncreaseOffset",
	"Edit_DecreaseOffset",
	"Edit_IncreaseTS",
	"Edit_DecreaseTS",
	"Edit_Anchor",
	"Edit_Event",
	"Edit_TrackEvent",
	"Edit_Section",
	"Edit_Note0",
	"Edit_Note1",
	"Edit_Note2",
	"Edit_Note3",
	"Edit_Note4",
	"Edit_Note5",
	"Edit_PS1",
	"Edit_PS2",
	"Edit_ShiftForwards",
	"Edit_ShiftBackwards",
	"Edit_ShiftLeft",
	"Edit_ShiftRight",
	"Edit_Cut",
	"Edit_Copy",
	"Edit_Paste",
	"Edit_Delete",
	"Edit_Goto",
	"Edit_Save",
	"Edit_StartPlayback",
	"Edit_RestartPlayback",
	"Edit_StopPlayback",
	"Edit_StopReturn",
	"Edit_PlaybackRate",
	"Edit_Metronome",
	"Edit_Clap",
	"Edit_Hyperspeed",
	"Edit_View",
	"Edit_Help",
	"Edit_Menu",
	"Edit_PopupMenu",
	"Edit_Mixer",
	"Edit_VolumeUp",
	"Edit_VolumeDown",
	"Edit_SwapTracks",
	"Edit_Easy",
	"Edit_Medium",
	"Edit_Hard",
	"Edit_Expert",
	"Edit_Track1",
	"Edit_Track2",
	"Edit_Track3",
	"Edit_Track4",
	"Edit_Track5",
	"Edit_Track6",
	"Edit_Track7",
	"Edit_Track8",
	"Edit_Track9",
	"Edit_Track10",
	"Edit_Track11"
};

static const char *gpGamepadStrings[GamepadType_Max] =
{
	"Pad_Cross",
	"Pad_Circle",
	"Pad_Box",
	"Pad_Triangle",
	"Pad_L1",
	"Pad_R1",
	"Pad_L2",
	"Pad_R2",
	"Pad_Start",
	"Pad_Select",
	"Pad_L3",
	"Pad_R3",
	"Pad_Up",
	"Pad_Down",
	"Pad_Left",
	"Pad_Right",
	"Pad_LX",
	"Pad_LY",
	"Pad_RX",
	"Pad_RY",
	"Pad_Home"
};

void GHControl::UpdateControls()
{
	bool ctrlState = MFInput_Read(Key_LControl, IDD_Keyboard) || MFInput_Read(Key_RControl, IDD_Keyboard);
	bool shiftState = MFInput_Read(Key_LShift, IDD_Keyboard) || MFInput_Read(Key_LShift, IDD_Keyboard);
	bool altState = MFInput_Read(Key_LAlt, IDD_Keyboard) || MFInput_Read(Key_RAlt, IDD_Keyboard);

#if defined(_PSP)
	shiftState = shiftState || MFInput_Read(Button_PP_R, IDD_Gamepad);
#endif

	uint32 mods = (shiftState ? 1 : 0) | (ctrlState ? 2 : 0) | (altState ? 4 : 0);

	for(int a=0; a<dBCtrl_Max; ++a)
		gConfig.controls.controls[a].Update(mods);
}

void GHControl::Set(int key, uint32 mod, int type, int holdKey, int secondTrigger, int secondType, int holdKey2)
{
	GHControlData data, data2;
	GHControlData *pD2 = NULL;

	data.Reset();
	data.mod = mod;
	data.trigger.Set(key, type);
	data.holdKeys[0].Set(holdKey, type);

	if(secondTrigger >= 0)
	{
		data2.Reset();
		data2.mod = mod;
		data2.trigger.Set(secondTrigger, secondType);
		data2.holdKeys[0].Set(holdKey2, secondType);
		pD2 = &data2;
	}

	Set(&data, pD2);
}

void GHControl::Set(GHControlData *pControlData, GHControlData *pControlData2)
{
	data[0].data = *pControlData;
	if(pControlData2)
		data[1].data = *pControlData2;
	else
		data[1].data.trigger.button = -1;
	pressed = false;
}

void GHControl::Set(MFIniLine *pLine)
{
	GHControlData data1, data2;
	GHControlData *pD2 = NULL;

	data1.FromString(pLine->GetString(1));

	if(pLine->GetStringCount() >= 3)
	{
		data2.FromString(pLine->GetString(2));
		pD2 = &data2;
	}

	Set(&data1, pD2);
}

void GHControl::InitControls()
{
	GHControlData data;

	gConfig.controls.controls[dBCtrl_Menu_Up].Set(Key_Up, GHCM_IgnoreAll, IDD_Keyboard, -1, Button_GH_StrumUp);
	gConfig.controls.controls[dBCtrl_Menu_Down].Set(Key_Down, GHCM_IgnoreAll, IDD_Keyboard, -1, Button_GH_StrumDown);
	gConfig.controls.controls[dBCtrl_Menu_Home].Set(Key_Home, GHCM_IgnoreAll, IDD_Keyboard, -1);
	gConfig.controls.controls[dBCtrl_Menu_End].Set(Key_End, GHCM_IgnoreAll, IDD_Keyboard, -1);
	gConfig.controls.controls[dBCtrl_Menu_PgUp].Set(Key_PageUp, GHCM_IgnoreAll, IDD_Keyboard, -1);
	gConfig.controls.controls[dBCtrl_Menu_PgDn].Set(Key_PageDown, GHCM_IgnoreAll, IDD_Keyboard, -1);
	gConfig.controls.controls[dBCtrl_Menu_Accept].Set(Key_Return, GHCM_IgnoreAll, IDD_Keyboard, -1, Button_GH_Green);
	gConfig.controls.controls[dBCtrl_Menu_Cancel].Set(Key_Escape, GHCM_IgnoreAll, IDD_Keyboard, -1, Button_GH_Red);

	// we need to do some tricky shit here...
	gConfig.controls.controls[dBCtrl_Game_G1_Green].Set(Button_GH_Green, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Red].Set(Button_GH_Red, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Yellow].Set(Button_GH_Yellow, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Blue].Set(Button_GH_Blue, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Orange].Set(Button_GH_Orange, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_StrumUp].Set(Button_GH_StrumUp, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_StrumDown].Set(Button_GH_StrumDown, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Start].Set(Button_GH_Start, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Select].Set(Button_GH_Select, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Tilt].Set(Button_GH_TiltTrigger, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Game_G1_Whammy].Set(Button_GH_Whammy, GHCM_IgnoreAll, IDD_Gamepad);
/*
	gConfig.controls.controls[dBCtrl_Game_G1_Green].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Red].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Yellow].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Blue].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Orange].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_StrumUp].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_StrumDown].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Start].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Select].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Tilt].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Whammy].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Green].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Red].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Yellow].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Blue].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Orange].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Start].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Select].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Tilt].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Whammy].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Hat].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Snare].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Kick].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Tom1].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Tom2].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Ride].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Hat].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Snare].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Kick].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Tom1].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Tom2].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Ride].Set(
*/
	gConfig.controls.controls[dBCtrl_Edit_Forward].Set(Key_Up, GHCM_IgnoreShift, IDD_Keyboard, -1, Button_GH_StrumDown);
	gConfig.controls.controls[dBCtrl_Edit_Back].Set(Key_Down, GHCM_IgnoreShift, IDD_Keyboard, -1, Button_GH_StrumUp);
	gConfig.controls.controls[dBCtrl_Edit_UpMeasure].Set(Key_PageUp, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_DownMeasure].Set(Key_PageDown, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_NextSection].Set(Key_Up, GHCM_IgnoreShift|GHCM_Alt);
	gConfig.controls.controls[dBCtrl_Edit_PrevSection].Set(Key_Down, GHCM_IgnoreShift|GHCM_Alt);
	gConfig.controls.controls[dBCtrl_Edit_End].Set(Key_Home, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_Start].Set(Key_End, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_RangeSelect].Set(Key_LShift, GHCM_IgnoreAll, IDD_Keyboard, -1, Key_RShift, IDD_Keyboard);
	gConfig.controls.controls[dBCtrl_Edit_QuantiseUp].Set(Key_Right, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_QuantiseDown].Set(Key_Left, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_IncreaseBPM].Set(Key_Equals, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_DecreaseBPM].Set(Key_Hyphen, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_IncreaseOffset].Set(Key_RBracket, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_DecreaseOffset].Set(Key_LBracket, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_IncreaseTS].Set(Key_Period, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_DecreaseTS].Set(Key_Comma, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_Quantise].Set(Key_Q);
	gConfig.controls.controls[dBCtrl_Edit_Anchor].Set(Key_A);
	gConfig.controls.controls[dBCtrl_Edit_Event].Set(Key_E);
	gConfig.controls.controls[dBCtrl_Edit_TrackEvent].Set(Key_W);
	gConfig.controls.controls[dBCtrl_Edit_Section].Set(Key_R);
	gConfig.controls.controls[dBCtrl_Edit_Note0].Set(Key_1, 0, IDD_Keyboard, -1, Button_GH_Green);
	gConfig.controls.controls[dBCtrl_Edit_Note1].Set(Key_2, 0, IDD_Keyboard, -1, Button_GH_Red);
	gConfig.controls.controls[dBCtrl_Edit_Note2].Set(Key_3, 0, IDD_Keyboard, -1, Button_GH_Yellow);
	gConfig.controls.controls[dBCtrl_Edit_Note3].Set(Key_4, 0, IDD_Keyboard, -1, Button_GH_Blue);
	gConfig.controls.controls[dBCtrl_Edit_Note4].Set(Key_5, 0, IDD_Keyboard, -1, Button_GH_Orange);
	gConfig.controls.controls[dBCtrl_Edit_Note5].Set(Key_8);
	gConfig.controls.controls[dBCtrl_Edit_PS1].Set(Key_6);
	gConfig.controls.controls[dBCtrl_Edit_PS2].Set(Key_7);
	gConfig.controls.controls[dBCtrl_Edit_ShiftForwards].Set(Key_Up, GHCM_Ctrl|GHCM_IgnoreAlt);
	gConfig.controls.controls[dBCtrl_Edit_ShiftBackwards].Set(Key_Down, GHCM_Ctrl|GHCM_IgnoreAlt);
	gConfig.controls.controls[dBCtrl_Edit_ShiftLeft].Set(Key_Left, GHCM_Ctrl);
	gConfig.controls.controls[dBCtrl_Edit_ShiftRight].Set(Key_Right, GHCM_Ctrl);
	gConfig.controls.controls[dBCtrl_Edit_Cut].Set(Key_X, GHCM_Ctrl);
	gConfig.controls.controls[dBCtrl_Edit_Copy].Set(Key_C, GHCM_Ctrl);
	gConfig.controls.controls[dBCtrl_Edit_Paste].Set(Key_V, GHCM_Ctrl);
	gConfig.controls.controls[dBCtrl_Edit_Delete].Set(Key_Delete);
	gConfig.controls.controls[dBCtrl_Edit_Goto].Set(Key_G, GHCM_Ctrl);
	gConfig.controls.controls[dBCtrl_Edit_Save].Set(Key_S, GHCM_IgnoreCtrl);
	gConfig.controls.controls[dBCtrl_Edit_StartPlayback].Set(Key_Space, 0, IDD_Keyboard, -1, Button_GH_Start);
	gConfig.controls.controls[dBCtrl_Edit_RestartPlayback].Set(Key_Space, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_StopPlayback].Set(Key_Space, GHCM_IgnoreShift, IDD_Keyboard, -1, Button_GH_Start);
	gConfig.controls.controls[dBCtrl_Edit_StopReturn].Set(Key_Space, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_PlaybackRate].Set(Key_Slash);
	gConfig.controls.controls[dBCtrl_Edit_Metronome].Set(Key_M, 0, IDD_Keyboard);
	gConfig.controls.controls[dBCtrl_Edit_Clap].Set(Key_N);
	gConfig.controls.controls[dBCtrl_Edit_Hyperspeed].Set(Key_Apostrophe);
	gConfig.controls.controls[dBCtrl_Edit_View].Set(Key_P);
	gConfig.controls.controls[dBCtrl_Edit_Help].Set(Key_H);
	gConfig.controls.controls[dBCtrl_Edit_Menu].Set(Key_Escape, 0, IDD_Keyboard, -1, Button_GH_Select);
	gConfig.controls.controls[dBCtrl_Edit_PopupMenu].Set(Key_Tab, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_Mixer].Set(Key_V);
	gConfig.controls.controls[dBCtrl_Edit_VolumeUp].Set(Key_Right, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_VolumeDown].Set(Key_Left, GHCM_IgnoreAll);

	gConfig.controls.controls[dBCtrl_Edit_SwapTracks].Set(Key_Grave, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_Easy].Set(Key_F1, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_Medium].Set(Key_F2, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_Hard].Set(Key_F3, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_Expert].Set(Key_F4, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_Track1].Set(Key_F1, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track2].Set(Key_F2, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track3].Set(Key_F3, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track4].Set(Key_F4, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track5].Set(Key_F5, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track6].Set(Key_F6, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track7].Set(Key_F7, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track8].Set(Key_F8, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track9].Set(Key_F9, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track10].Set(Key_F10, 0);
	gConfig.controls.controls[dBCtrl_Edit_Track11].Set(Key_F11, 0);

#if defined(MF_PSP)
/*
	gConfig.controls.controls[dBCtrl_Game_G1_Green].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Red].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Yellow].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Blue].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Orange].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Start].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Select].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Tilt].Set(
	gConfig.controls.controls[dBCtrl_Game_G1_Whammy].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Green].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Red].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Yellow].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Blue].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Orange].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Start].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Select].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Tilt].Set(
	gConfig.controls.controls[dBCtrl_Game_G2_Whammy].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Hat].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Snare].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Kick].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Tom1].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Tom2].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum_Ride].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Hat].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Snare].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Kick].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Tom1].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Tom2].Set(
	gConfig.controls.controls[dBCtrl_Game_Drum2_Ride].Set(
	gConfig.controls.controls[dBCtrl_Edit_Anchor].Set(
	gConfig.controls.controls[dBCtrl_Edit_Event].Set(
	gConfig.controls.controls[dBCtrl_Edit_TrackEvent].Set(
	gConfig.controls.controls[dBCtrl_Edit_Section].Set(
	gConfig.controls.controls[dBCtrl_Edit_Goto].Set(
	gConfig.controls.controls[dBCtrl_Edit_Save].Set(
	gConfig.controls.controls[dBCtrl_Edit_StopReturn].Set(
	gConfig.controls.controls[dBCtrl_Edit_PlaybackRate].Set(
	gConfig.controls.controls[dBCtrl_Edit_Metronome].Set(
	gConfig.controls.controls[dBCtrl_Edit_Clap].Set(
	gConfig.controls.controls[dBCtrl_Edit_Hyperspeed].Set(
	gConfig.controls.controls[dBCtrl_Edit_View].Set(
	gConfig.controls.controls[dBCtrl_Edit_Help].Set(
	gConfig.controls.controls[dBCtrl_Edit_Mixer].Set(
*/
	gConfig.controls.controls[dBCtrl_Menu_Up].Set(Button_DUp, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Menu_Down].Set(Button_DDown, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Menu_Accept].Set(Button_PP_Cross, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Menu_Cancel].Set(Button_PP_Circle, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Forward].Set(Button_DUp, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Back].Set(Button_DDown, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_UpMeasure].Set(Button_DUp, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_DownMeasure].Set(Button_DDown, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_NextSection].Set(Key_Up, GHCM_IgnoreShift|GHCM_Alt);
	gConfig.controls.controls[dBCtrl_Edit_PrevSection].Set(Key_Down, GHCM_IgnoreShift|GHCM_Alt);
	gConfig.controls.controls[dBCtrl_Edit_End].Set(Key_Home, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_Start].Set(Key_End, GHCM_IgnoreShift);
	gConfig.controls.controls[dBCtrl_Edit_RangeSelect].Set(Button_PP_L, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_QuantiseUp].Set(Button_DRight, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_QuantiseDown].Set(Button_DLeft, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_IncreaseBPM].Set(Axis_LX);
	gConfig.controls.controls[dBCtrl_Edit_DecreaseBPM].Set(Key_Hyphen, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_IncreaseOffset].Set(Axis_LX, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_DecreaseOffset].Set(Key_LBracket, GHCM_IgnoreAll);
	gConfig.controls.controls[dBCtrl_Edit_IncreaseTS].Set(Button_DRight, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_DecreaseTS].Set(Button_DLeft, GHCM_Shift);
	gConfig.controls.controls[dBCtrl_Edit_Note0].Set(Button_PP_Cross, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Note1].Set(Button_PP_Circle, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Note2].Set(Button_PP_Box, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Note3].Set(Button_PP_Triangle, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Note4].Set(Button_PP_Cross, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Note5].Set(Button_PP_Circle, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_PS1].Set(Button_PP_Box, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_PS2].Set(Button_PP_Triangle, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_ShiftForwards].Set(Key_Up, GHCM_Ctrl|GHCM_IgnoreAlt);
	gConfig.controls.controls[dBCtrl_Edit_ShiftBackwards].Set(Key_Down, GHCM_Ctrl|GHCM_IgnoreAlt);
	gConfig.controls.controls[dBCtrl_Edit_ShiftLeft].Set(Button_DLeft, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_ShiftRight].Set(Button_DRight, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Cut].Set(Button_PP_Box, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Copy].Set(Button_PP_Cross, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Paste].Set(Button_PP_Circle, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_StartPlayback].Set(Button_PP_Start, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_RestartPlayback].Set(Button_PP_Start, GHCM_Shift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_StopPlayback].Set(Button_PP_Start, GHCM_IgnoreShift, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_Menu].Set(Button_PP_Select, 0, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_PopupMenu].Set(Button_PP_L);
	gConfig.controls.controls[dBCtrl_Edit_VolumeUp].Set(Button_DRight, GHCM_IgnoreAll, IDD_Gamepad);
	gConfig.controls.controls[dBCtrl_Edit_VolumeDown].Set(Button_DLeft, GHCM_IgnoreAll, IDD_Gamepad);
#endif
}

bool GHControlUnit::Test(uint32 mods, GHControlType type)
{
	if(type == GHCT_None || data.trigger.button == -1)
		return false;

	uint32 mask = ~(data.mod >> 4) & 0xF;
	if((data.mod & mask) != (mods & mask))
		return false;

	if(type == GHCT_Once)
	{
		for(int a=0; a<2; ++a)
		{
			if(data.holdKeys[a].button >= 0)
			{
				if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
					return false;
			}
		}

		if(MFInput_WasPressed(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			return true;
	}
	else if(type == GHCT_Hold)
	{
		for(int a=0; a<2; ++a)
		{
			if(data.holdKeys[a].button >= 0)
			{
				if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
					return false;
			}
		}

		if(MFInput_Read(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			return true;
	}
	else if(type == GHCT_Release)
	{
		if(MFInput_Read(data.trigger.button, data.trigger.device, data.trigger.deviceID))
		{
			for(int a=0; a<2; ++a)
			{
				if(data.holdKeys[a].button >= 0)
				{
					if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
						return true;
				}
			}
		}
		else if(MFInput_WasReleased(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			return true;
	}
	else if(type == GHCT_Repeat)
	{
		if(held2)
		{
			bool holding = true;

			for(int a=0; a<2; ++a)
			{
				if(data.holdKeys[a].button >= 0)
				{
					if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
					{
						holding = false;
						break;
					}
				}
			}

			if(holding && MFInput_Read(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			{
				int fc = MFSystem_GetFrameCounter();
				if(frameCount != fc)
				{
					frameCount = fc;
					timeout2 -= MFSystem_TimeDelta();
				}

				if(timeout2 <= 0.0f)
				{
					timeout2 += KEY_REPEAT;
					return true;
				}
			}
			else
			{
				held2 = false;
			}
		}
		else
		{
			for(int a=0; a<2; ++a)
			{
				if(data.holdKeys[a].button >= 0)
				{
					if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
						return false;
				}
			}

			if(MFInput_WasPressed(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			{
				held2 = true;
				timeout2 = KEY_REPEAT;
				return true;
			}
		}
	}
	else if(type == GHCT_Delay)
	{
		if(held)
		{
			bool holding = true;

			for(int a=0; a<2; ++a)
			{
				if(data.holdKeys[a].button >= 0)
				{
					if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
					{
						holding = false;
						break;
					}
				}
			}

			if(holding && MFInput_Read(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			{
				int fc = MFSystem_GetFrameCounter();
				if(frameCount != fc)
				{
					frameCount = fc;
					timeout -= MFSystem_TimeDelta();
				}

				if(timeout <= 0.0f)
				{
					timeout += KEY_REPEAT;
					return true;
				}
			}
			else
			{
				held = false;
			}
		}
		else
		{
			for(int a=0; a<2; ++a)
			{
				if(data.holdKeys[a].button >= 0)
				{
					if(!MFInput_Read(data.holdKeys[a].button, data.holdKeys[a].device, data.holdKeys[a].deviceID))
						return false;
				}
			}

			if(MFInput_WasPressed(data.trigger.button, data.trigger.device, data.trigger.deviceID))
			{
				held = true;
				timeout = KEY_PAUSE;
				return true;
			}
		}
	}

	return false;
}

void GHControl::Update(uint32 mods)
{
	for(int a=0; a<GHCT_Max; ++a)
		pressedType[a] = data[0].Test(mods, (GHControlType)a) || data[1].Test(mods, (GHControlType)a);
}

bool GHControl::Test(GHControlType type)
{
	if(!this)
		return false;
	if(type == GHCT_None)
		return false;
	return pressedType[type];
}

const char *GHControl::GetSettingsString(dBControlType control)
{
	GHControl &ctrl = gConfig.controls.controls[control];

	const char *pSecond = "";
	if(ctrl.data[1].data.trigger.button != -1)
		pSecond = MFStr(", \"%s\"", ctrl.data[1].data.GetString());

	return MFStr("\t\t%s = \"%s\"%s\r\n", dBControlStrings[control], ctrl.data[0].data.GetString(), pSecond);
}

const char* GHControlData::GetString(bool withExclusions)
{
	const char *pString = "";

	if(mod & GHCM_Shift)
		pString = MFStr("%sShift+", pString);
	if(mod & GHCM_Ctrl)
		pString = MFStr("%sCtrl+", pString);
	if(mod & GHCM_Alt)
		pString = MFStr("%sAlt+", pString);
	if(withExclusions)
	{
		if((mod & GHCM_IgnoreAll) == GHCM_IgnoreAll)
			pString = MFStr("%s~All+", pString);
		else
		{
			if(mod & GHCM_IgnoreShift)
				pString = MFStr("%s~Shift+", pString);
			if(mod & GHCM_IgnoreCtrl)
				pString = MFStr("%s~Ctrl+", pString);
			if(mod & GHCM_IgnoreAlt)
				pString = MFStr("%s~Alt+", pString);
		}
	}

	const char *pButton;

	for(int a=0; a<2; a++)
	{
		if(holdKeys[a].button > -1)
		{
			const char *pButton;

			if(holdKeys[a].device == IDD_Gamepad)
				pButton = gpGamepadStrings[holdKeys[a].button];
			else
				pButton = MFInput_EnumerateString(holdKeys[a].button, holdKeys[a].device, holdKeys[a].deviceID);

			pString = MFStr("%s%s+", pString, pButton);
		}
	}

	if(trigger.device == IDD_Gamepad)
		pButton = gpGamepadStrings[trigger.button];
	else
		pButton = MFInput_EnumerateString(trigger.button, trigger.device, trigger.deviceID);

	pString = MFStr("%s%s", pString, pButton);

	return pString;
}

void GetControl(GHButton *pButton, const char *pToken)
{
	for(int a=0; a<GamepadType_Max; ++a)
	{
		if(!MFString_CaseCmp(pToken, gpGamepadStrings[a]))
		{
			pButton->button = a;
			pButton->device = IDD_Gamepad;
			pButton->deviceID = -1;
			return;
		}
	}

	for(int a=0; a<Key_Max; ++a)
	{
		if(!MFString_CaseCmp(pToken, MFInput_EnumerateString(a, IDD_Keyboard)))
		{
			pButton->button = a;
			pButton->device = IDD_Keyboard;
			pButton->deviceID = 0;
			return;
		}
	}

	pButton->button = -1;
	pButton->device = pButton->deviceID = 0;
}

void GHControlData::FromString(const char *pString)
{
	mod = 0;

	char *pToken = strtok((char*)MFStr(pString), "+");

	while(pToken)
	{
		if(!MFString_CaseCmp(pToken, "Shift"))
			mod |= GHCM_Shift;
		else if(!MFString_CaseCmp(pToken, "Ctrl"))
			mod |= GHCM_Ctrl;
		else if(!MFString_CaseCmp(pToken, "Alt"))
			mod |= GHCM_Alt;
		else if(!MFString_CaseCmp(pToken, "~All"))
			mod |= GHCM_IgnoreAll;
		else if(!MFString_CaseCmp(pToken, "~Shift"))
			mod |= GHCM_IgnoreShift;
		else if(!MFString_CaseCmp(pToken, "~Ctrl"))
			mod |= GHCM_IgnoreCtrl;
		else if(!MFString_CaseCmp(pToken, "~Alt"))
			mod |= GHCM_IgnoreAlt;
		else
		{
			if(trigger.button != -1)
			{
				if(holdKeys[0].button != -1)
				{
					holdKeys[1] = trigger;
				}
				else
				{
					holdKeys[0] = trigger;
				}
			}

			GetControl(&trigger, pToken);
		}

		pToken = strtok(NULL, "+");
	}
}

