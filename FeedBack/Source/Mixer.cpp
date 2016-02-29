#include "FeedBack.h"
#include "Control.h"

#include "Screens/Editor.h"

Mixer::Mixer()
{
	mixerTrack = 0;
	numControls = 8;

	pControls = new MixerControl[numControls];

	pControls[0].Set(MFTranslation_GetString(pStrings, VOLUME_MASTER), 0, &gConfig.sound.masterLevel);
	pControls[1].Set(MFTranslation_GetString(pStrings, VOLUME_PAN), 1, &gEditor.musicPan);
	pControls[2].Set(MFTranslation_GetString(pStrings, VOLUME_MUSIC), 0, &gConfig.sound.musicLevel);
	pControls[3].Set(MFTranslation_GetString(pStrings, VOLUME_GUITAR), 0, &gConfig.sound.guitarLevel);
	pControls[4].Set(MFTranslation_GetString(pStrings, VOLUME_BASS), 0, &gConfig.sound.bassLevel);
	pControls[5].Set(MFTranslation_GetString(pStrings, VOLUME_FX), 0, &gConfig.sound.fxLevel);
	pControls[6].Set(MFTranslation_GetString(pStrings, VOLUME_CLAP), 0, &gConfig.sound.clapLevel);
	pControls[7].Set(MFTranslation_GetString(pStrings, VOLUME_TICK), 0, &gConfig.sound.tickLevel);
}

Mixer::~Mixer()
{
	delete[] pControls;
}

void Mixer::Update()
{
	GHScreen::Update();
}

void Mixer::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		Pop();
	}
	else if(TestControl(dBCtrl_Menu_Accept, GHCT_Once))
	{
		Pop();
	}

	if(TestControl(dBCtrl_Menu_Up, GHCT_Delay))
	{
		--mixerTrack;
		if(mixerTrack < 0)
			mixerTrack = numControls-1;
	}
	if(TestControl(dBCtrl_Menu_Down, GHCT_Delay))
	{
		mixerTrack = (mixerTrack + 1) % numControls;
	}

	pControls[mixerTrack].Update();

	// HACK: set the stream volume/pan
	if(mixerTrack == 1)
	{
		gEditor.pSong->SetPan(gEditor.musicPan);
	}
	else if(mixerTrack >= 0 && mixerTrack <= 4)
	{
		gEditor.pSong->SetVolume(gConfig.sound.masterLevel);
	}
}

void Mixer::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = (MFDisplay_GetAspectRatio() >= 1.5) ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = (MFDisplay_GetAspectRatio() >= 1.5) ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	float textHeight = 16;
	float w = 250.0f, h, x, y;
	MFFont_GetStringWidth(pText, MFTranslation_GetString(pStrings, MENU_VOLUME), textHeight*1.5f, w, -1, &h);

	float lh = 0;
	for(int a=0; a<numControls; ++a)
		lh += pControls->GetHeight();

	h += lh + 10;

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	MFPrimitive_DrawUntexturedQuad(x-10, y-10, w+20, h + 20.0f, MakeVector(0,0,0.8f,0.8f));
	MFPrimitive_DrawUntexturedQuad(x-5, y-5 + (h-lh), w+10, lh + 10, MakeVector(0,0,0,1));

	MFFont_DrawTextAnchored(pHeading, MFTranslation_GetString(pStrings, MENU_VOLUME), MakeVector(x + w*0.5f, y-5, 0.0f), MFFontJustify_Top_Center, w, textHeight*2.f, MFVector::yellow);

	y += (h-lh);
	for(int a=0; a<numControls; ++a)
	{
		if(a == mixerTrack)
		{
			MFPrimitive_DrawUntexturedQuad(x-5, y, w+10, pControls[a].GetHeight(), MakeVector(0,0,0.5f,1));
		}

		float h;
		pControls[a].Draw(x, y, w, &h);

		y += h;
	}

	MFView_Pop();
}

void MixerControl::Set(const char *pName, int _type, float *_pLevel)
{
	MFString_Copy(name, pName);
	type = _type;
	pLevel = _pLevel;
	height = 32.0f;
}

void MixerControl::Update()
{
	if(TestControl(dBCtrl_Edit_VolumeDown, GHCT_Hold))
	{
		if(pLevel)
		{
			*pLevel -= (type == 0 ? 1.0f : 2.0f) * MFSystem_GetTimeDelta();
			*pLevel = MFMax(*pLevel, type == 0 ? 0.0f : -1.0f);
		}
	}
	else if(TestControl(dBCtrl_Edit_VolumeUp, GHCT_Hold))
	{
		if(pLevel)
		{
			*pLevel += (type == 0 ? 1.0f : 2.0f) * MFSystem_GetTimeDelta();
			*pLevel = MFMin(*pLevel, 1.0f);
		}
	}
	else
	{
		if(type == 1 && pLevel)
		{
			if(MFAbs(*pLevel) < 0.1f)
				*pLevel = 0.0f;
		}
	}
}

void MixerControl::Draw(float x, float y, float w, float *pHeight)
{
	// draw volume slider
	MFMaterial_SetMaterial(MFMaterial_GetStockMaterial(MFMat_White));
	MFPrimitive(PT_TriList, 0);

	float lx = x+5;
	float lw = w-10;

	if(type == 0)
	{
		MFBegin(3);
		MFSetColour(0.2f, 0.4f, 1.0f, 1.0f);
		MFSetPosition(lx, y+height-5, 0);
		MFSetPosition(lx+lw, y+5, 0);
		MFSetPosition(lx+lw, y+height-5, 0);
		MFEnd();
	}
	else if(type == 1)
	{
		MFBegin(6);
		MFSetColour(0.2f, 0.4f, 1.0f, 1.0f);
		MFSetPosition(lx+lw*0.5f, y+height-5, 0);
		MFSetPosition(lx+lw, y+5, 0);
		MFSetPosition(lx+lw, y+height-5, 0);
		MFSetPosition(lx, y+5, 0);
		MFSetPosition(lx+lw*0.5f, y+height-5, 0);
		MFSetPosition(lx, y+height-5, 0);
		MFEnd();
	}

	if(pLevel)
	{
		float offset = type == 0 ? lw**pLevel : lw*((*pLevel + 1.0f) * 0.5f);

		MFBegin(6);
		MFSetColour(1.0f, 1.0f, 0.0f, 1.0f);
		MFSetPosition(lx+offset-5, y+2, 0);
		MFSetPosition(lx+offset+5, y+height-2, 0);
		MFSetPosition(lx+offset-5, y+height-2, 0);

		MFSetPosition(lx+offset-5, y+2, 0);
		MFSetPosition(lx+offset+5, y+2, 0);
		MFSetPosition(lx+offset+5, y+height-2, 0);
		MFEnd();
	}

	MFFont_DrawTextJustified(pHeading, name, MakeVector(x+2, y), w, height, MFFontJustify_Center_Left, 22.0f, MFVector::white);

	if(pHeight)
		*pHeight = 32.0f;
}

float MixerControl::GetHeight()
{
	return 32.0f;
}
