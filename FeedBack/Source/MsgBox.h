#if !defined(_MSGBOXSCREEN_H)
#define _MSGBOXSCREEN_H

#include "Gadgets/Frame.h"

typedef void (*MessageCallback)();

class MessageBoxScreen : public GHScreen
{
public:
	MessageBoxScreen();
	virtual ~MessageBoxScreen();

	void Show(const char *pMessage, MessageCallback pCallback = NULL);

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	char gMessage[1024];

	dBFrame frame;

	MessageCallback gpCompleteCallback;
};

#endif
