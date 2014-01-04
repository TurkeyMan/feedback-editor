#if !defined(_NATIONALITYSCREEN_H)
#define _NATIONALITYSCREEN_H

#include "Fuji/MFMaterial.h"
#include "Screen.h"

class NationalityScreen : public dBScreen
{
public:
	NationalityScreen();
	virtual ~NationalityScreen();

	virtual void Select();
	virtual int Update();
	virtual void Draw();
	virtual void Deselect();

	int languages[MFLang_Max];
	int numLanguages;

	int language;
	MFFont *pFont;

	float yOffset;
	float fontHeight;

	MFMaterial *pFlags;
};

#endif
