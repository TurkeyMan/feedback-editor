#if !defined(_LISTBOXSCREEN_H)
#define _LISTBOXSCREEN_H

typedef void (*ListCallback)(int, int, const char *, void *);

#include "Gadgets/Frame.h"

struct ListItem
{
	char *pString;
	void *pData;
	ListItem *pNext;
};

class ListBoxScreen : public GHScreen
{
public:
	ListBoxScreen();
	virtual ~ListBoxScreen();

	void Show(const char *pMessage, ListCallback pCallback, float width = 400.0f, float height = 150.0f);
	void AddItem(const char *pItem, void *pData = NULL);
	void Destroy();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	char gMessage[1024];

	dBFrame frame;

	ListCallback gpCompleteCallback;

	int numItems;
	ListItem *pFirst;

	int selected;
	int listOffset;
	int pageMax;
	char typeBuffer[256];
	float typeTimeout;

	float width, height;
};

#endif
