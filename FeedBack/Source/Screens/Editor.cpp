#include "FeedBack.h"
#include "MFFileSystem.h"

#include "Screens/Editor.h"
#include "GHScreen.h"
#include "OldEditor.h"

#include "Gadgets/Scene.h"

EventSuggestions EditorScreen::eventSuggestions;

////////////////////////////////////////////
GameScreen gGame;
extern const char * gViewPoints[3];
Editor gEdit;
PlayerScreen gPlay;
MenuScreen gMenu;
HelpScreen *gpHelp = NULL;
MessageBoxScreen *gpMsgBox = NULL;
StringBoxScreen *gpStringBox = NULL;
ListBoxScreen *gpListBox = NULL;
ComboBoxScreen *gpComboBox = NULL;
FileSelectorScreen *gpFileSelector = NULL;

GHEditor gEditor;

//dBScene *pScene;
////////////////////////////////////////////

EditorScreen::EditorScreen()
{
	gViewPoints[0] = MFTranslation_GetString(pStrings, POV_INCOMING);
	gViewPoints[1] = MFTranslation_GetString(pStrings, POV_OVERHEAD);
	gViewPoints[2] = MFTranslation_GetString(pStrings, POV_SIDE_SCROLL);
}

EditorScreen::~EditorScreen()
{
}

void EditorScreen::Select()
{
	gpHelp = new HelpScreen;
	gpMsgBox = new MessageBoxScreen;
	gpStringBox = new StringBoxScreen;
	gpListBox = new ListBoxScreen;
	gpComboBox = new ComboBoxScreen;
	gpFileSelector = new FileSelectorScreen;

//	pScene = dBScene::Create("s_testscene");

	gEditor.pMetronome = MFMaterial_Create("metronome");

	// load sounds
	gEditor.pStepSound = MFSound_Create("Sounds/row");
	gEditor.pChangeSound = MFSound_Create("Sounds/prompt");
	gEditor.pSaveSound = MFSound_Create("Sounds/save");
	gEditor.pClapSound = MFSound_Create("Sounds/claps");
	gEditor.pHighTickSound = MFSound_Create("Sounds/hightick");
	gEditor.pLowTickSound = MFSound_Create("Sounds/lowtick");

	if(gConfig.editor.lastOpenedChart[0])
	{
		gEditor.pSong = dBChart::LoadChart(gConfig.editor.lastOpenedChart);
		gEditor.offset = 0;
		gEditor.currentBPM = gEditor.pSong->GetStartBPM();
	}
	else
		gEditor.pSong = dBChart::Create();

	gGame.pTrack = new Fretboard;
	gGame.pTrack2 = new Fretboard;

	if(gConfig.editor.editorFretboard[0])
	{
		gGame.pTrack->LoadFretboard(gConfig.editor.editorFretboard);
		gGame.pTrack2->LoadFretboard(gConfig.editor.editorFretboard);
	}
	else
	{
		int image = MFRand()%10;
		MFFindData fd;
		MFFind *pFind = MFFileSystem_FindFirst("theme:Fretboards/*", &fd);
		if(pFind)
		{
			bool more = true;
			while(fd.pFilename[0] == '.' && more)
				more = MFFileSystem_FindNext(pFind, &fd);
			if(!more)
			{
				MFFileSystem_FindClose(pFind);
				pFind = NULL;
			}
		}
		if(pFind)
		{
			while(image--)
			{
				if(!MFFileSystem_FindNext(pFind, &fd))
				{
					MFFileSystem_FindClose(pFind);
					pFind = MFFileSystem_FindFirst("theme:Fretboards/*", &fd);
					while(fd.pFilename[0] == '.')
						MFFileSystem_FindNext(pFind, &fd);
				}
			}

			const char *pFilename = MFStr_GetFileNameWithoutExtension(fd.pFilename);
			gGame.pTrack->LoadFretboard(pFilename);
			gGame.pTrack2->LoadFretboard(pFilename);
		}
	}

	eventSuggestions.LoadEventSuggestions("game:events.txt");

	gGame.Push();
	gEdit.Push();
	gpHelp->Push();
}

int EditorScreen::Update()
{
	GHScreen::UpdateScreens();
	return 0;
}

void EditorScreen::Draw()
{
	GHScreen::DrawScreens();	

	MFRenderer_ClearScreen(CS_ZBuffer);

	MFMatrix mat;
	mat.LookAt(MakeVector(2.0,1.5,-1.0), MakeVector(1.0,0.3f,1.0f));
	MFView_SetCameraMatrix(mat);
//	pScene->Draw();
}

void EditorScreen::Deselect()
{
	eventSuggestions.ReleaseEventSuggestions();

	delete gGame.pTrack;
	delete gGame.pTrack2;

	gEditor.pSong->Destroy();

	// clean up sounds
	MFSound_Destroy(gEditor.pStepSound);
	MFSound_Destroy(gEditor.pChangeSound);
	MFSound_Destroy(gEditor.pSaveSound);
	MFSound_Destroy(gEditor.pClapSound);
	MFSound_Destroy(gEditor.pHighTickSound);
	MFSound_Destroy(gEditor.pLowTickSound);

	MFMaterial_Destroy(gEditor.pMetronome);
}
