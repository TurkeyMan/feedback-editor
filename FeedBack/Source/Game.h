#if !defined(_GAMESCREEN_H)
#define _GAMESCREEN_H

#include "Gadgets/Track.h"

class GameScreen : public GHScreen
{
public:
	GameScreen();
	~GameScreen();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	Fretboard *pTrack;
	Fretboard *pTrack2;
};

#endif
