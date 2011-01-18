#if !defined(_MAINMENUSCREEN_H)
#define _MAINMENUSCREEN_H

#include "MFMaterial.h"
#include "Screen.h"

class MainMenuScreen : public dBScreen
{
public:
	MainMenuScreen();
	virtual ~MainMenuScreen();

	virtual void Select();
	virtual int Update();
	virtual void Draw();
	virtual void Deselect();

	int item;
};

#endif
