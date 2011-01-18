#if !defined(_PLAYERSCREEN_H)
#define _PLAYERSCREEN_H

#include "Tools/NoteScoreKeeper.h"

class PlayerScreen : public GHScreen
{
public:
	virtual void Update();
	virtual void UpdateInput();

	dBNoteScoreKeeper scoreKeeper[2];
};

#endif
