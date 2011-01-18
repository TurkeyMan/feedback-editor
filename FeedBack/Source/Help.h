#if !defined(_HELPSCREEN_H)
#define _HELPSCREEN_H

#include "Gadgets/Frame.h"

class HelpScreen : public GHScreen
{
public:
	HelpScreen();
	virtual ~HelpScreen();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	dBFrame frame;
};

#endif
