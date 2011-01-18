#if !defined(_STRINGBOXSCREEN_H)
#define _STRINGBOXSCREEN_H

#include "Tools/StringEntryLogic.h"
#include "Gadgets/Frame.h"

typedef void (*StringCallback)(int, const char *);

class StringBoxScreen : public GHScreen
{
public:
	StringBoxScreen();
	virtual ~StringBoxScreen();

	void Show(const char *pMessage, const char *pDefaultString, StringCallback pCallback);

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	char message[1024];

	dBFrame frame;

	StringEntryLogic stringLogic;
	StringCallback pCompleteCallback;

protected:
	static void StringChangeCallback(const char *pString, void *pUserData);
};

#endif
