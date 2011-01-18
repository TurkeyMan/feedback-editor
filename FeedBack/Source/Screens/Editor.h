#if !defined(_EDITORSCREEN_H)
#define _EDITORSCREEN_H

#include "Tools/EventSuggestions.h"

#include "MFMaterial.h"
#include "Screen.h"

class EditorScreen : public dBScreen
{
public:
	EditorScreen();
	virtual ~EditorScreen();

	virtual void Select();
	virtual int Update();
	virtual void Draw();
	virtual void Deselect();

	static EventSuggestions eventSuggestions;

	Mixer gMixer;
};


extern GHEditor gEditor;

extern GameScreen gGame;
extern Editor gEdit;
extern PlayerScreen gPlay;
extern HelpScreen *gpHelp;
extern MenuScreen gMenu;
extern MessageBoxScreen *gpMsgBox;
extern StringBoxScreen *gpStringBox;
extern ListBoxScreen *gpListBox;
extern ComboBoxScreen *gpComboBox;
extern FileSelectorScreen *gpFileSelector;

#endif
