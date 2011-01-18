#include "FeedBack.h"
#include "MFSockets.h"
#include "MFThread.h"

#include "Download.h"
#include "MainMenu.h"

DownloadScreen::DownloadScreen()
{
	bWaitingForNetwork = false;
	pGenres = NULL;
	numGenres = 0;
	currentGenre = -1;
}

DownloadScreen::~DownloadScreen()
{
	if(pGenres)
		MFHeap_Free(pGenres);
}

void DownloadScreen::Select()
{
	if(MFSockets_IsActive())
	{
		thread = MFThread_CreateThread("CustomHero Network Thread", NetworkJobThread, NULL);
	}
}

int DownloadScreen::Update()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		// go back..
		dBScreen::SetNext(new MainMenuScreen);
	}

	if(!MFSockets_IsActive())
		return 0;


	return 0;
}

void DownloadScreen::Draw()
{
	if(!MFSockets_IsActive())
	{
		// if the network is not active, we'll display some message and wait for the user to return..
	}
	else
	{

	}
}

void DownloadScreen::Deselect()
{
	// if the thread never responds, we'll have to kill it...
//	MFThread_DestroyThread(thread);

	delete this;
}

int DownloadScreen::NetworkJobThread(void *pUserData)
{
//	MFFileSystem_Load(

	return 0;
}
