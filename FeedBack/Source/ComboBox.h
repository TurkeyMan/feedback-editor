#if !defined(_COMBOBOXSCREEN_H)
#define _COMBOBOXSCREEN_H

typedef void (*ComboCallback)(int, int, const char *, void *);

#include "Gadgets/Frame.h"

struct ComboItem
{
	char *pString;
	void *pData;
	ComboItem *pNext;
};

class ComboBoxScreen : public GHScreen
{
public:
	ComboBoxScreen();
	virtual ~ComboBoxScreen();

	void Show(const char *pMessage, const char *pDefaultString, ComboCallback pCallback, ComboCallback pChangeCallback = NULL, float width = 400.0f, float height = 150.0f);
	void AddItem(const char *pItem, void *pData = NULL);
	void Clear();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	char message[1024];

	dBFrame frame;

	StringEntryLogic stringLogic;

	ComboCallback pCompleteCallback;
	ComboCallback pChangeCallback;

	// list box
	int numItems;
	ComboItem *pFirst;

	int selected;
	int listOffset;
	int pageMax;

	float width, height;

protected:
	static void StringChangeCallback(const char *pString, void *pUserData);
};

#endif
