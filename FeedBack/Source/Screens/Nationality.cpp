#include "FeedBack.h"
#include "Fuji/MFFileSystem.h"
#include "Screen.h"
#include "Nationality.h"

#include "Screens/MainMenu.h"

static int gFlags[] =
{
	0, // MFLang_English
	1, // MFLang_French
	3, // MFLang_German
	-1, // MFLang_Dutch
	4, // MFLang_Italian
	2, // MFLang_Spanish
	12, // MFLang_Portuguese
	5, // MFLang_Swedish
	6, // MFLang_Norwegian
	-1, // MFLang_Finnish
	7, // MFLang_Danish
	11, // MFLang_Russian
	-1, // MFLang_Greek
	8, // MFLang_Japanese
	9, // MFLang_Korean
	10 // MFLang_Chinese
};

NationalityScreen::NationalityScreen()
{
	language = MFTranslation_GetDefaultLanguage();

	pFlags = MFMaterial_Create("flags");
	pFont = MFFont_Create("Blzee");

	fontHeight = 70.0f;
}

NationalityScreen::~NationalityScreen()
{
	MFMaterial_Release(pFlags);
	MFFont_Destroy(pFont);
}

void NationalityScreen::Select()
{
	language = gConfig.general.language == MFLang_Unknown ? MFLang_English : gConfig.general.language;
	yOffset = (float)language * fontHeight;
	numLanguages = 0;

	for(int a=0; a<MFLang_Max; ++a)
	{
		if(MFFileSystem_Exists(MFStr("Strings.%s", MFTranslation_GetLanguageName((MFLanguage)a))))
		{
			languages[numLanguages] = a;
			++numLanguages;
		}
	}
}

int NationalityScreen::Update()
{
	if(TestControl(dBCtrl_Menu_Accept, GHCT_Once))
	{
		gConfig.general.language = (MFLanguage)languages[language];
		pStrings = MFTranslation_LoadStringTable("Strings", gConfig.general.language, MFLang_English);
		dBScreen::SetNext(new MainMenuScreen);
		return 0;
	}

	if(TestControl(dBCtrl_Menu_Up, GHCT_Delay))
	{
		if(language > 0)
			language--;
	}
	else if(TestControl(dBCtrl_Menu_Down, GHCT_Delay))
	{
		if(language < numLanguages-1)
			language++;
	}

	float targetY = (float)language * fontHeight;

	float direction = (targetY - yOffset) * 4.0f * MFSystem_TimeDelta();
	yOffset += direction;

	if(direction < 0.0f && yOffset < targetY)
	{
		yOffset = targetY;
	}
	else if(direction > 0.0f && yOffset>targetY)
	{
		yOffset = targetY;
	}

	return 0;
}

void NationalityScreen::Draw()
{
	MFView_Push();

	MFRect rect;
	float width = 480.0f * MFDisplay_GetNativeAspectRatio();
	float overflow = width - 640.0f;
	rect.x = -overflow * 0.5f;
	rect.y = 0.0f;
	rect.height = 480.0f;
	rect.width = width;
	MFView_SetOrtho(&rect);

	float y = (480 - fontHeight)*0.5f - yOffset;

	MFMaterial_SetMaterial(pFlags);

	for(int a=0; a<numLanguages; a++)
	{
		int lang = gFlags[languages[a]];
		const float rx = 1.0f / 3.0f;
		const float ry = 1.0f / 5.0f;
		float tx = (float)(lang / 5);
		float ty = (float)(lang % 5);

		MFPrimitive_DrawQuad(100, y, 110, 60, MFVector::one, tx*rx, ty*ry, tx*rx+rx, ty*ry+ry);
		y += fontHeight;
	}

	y = (480 - fontHeight)*0.5f + 20.0f - yOffset;

	for(int a=0; a<numLanguages; a++)
	{
		MFFont_DrawText(pFont, MakeVector(250.0f, y, 0.0f), fontHeight - 40.0f, a == language ? MakeVector(1,1,0,1) : MFVector::one, MFTranslation_GetLanguageName((MFLanguage)languages[a], false));
		y += fontHeight;
	}

	MFView_Pop();
}

void NationalityScreen::Deselect()
{
	delete this;
}
