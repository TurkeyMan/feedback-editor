#include "FeedBack.h"
#include "Screen.h"

dBScreen *dBScreen::pCurrent = NULL;
dBScreen *dBScreen::pNext = NULL;

dBScreen::dBScreen()
{
}

dBScreen::~dBScreen()
{
}

int dBScreen::UpdateScreen()
{
	if(pNext)
	{
		if(pCurrent)
			pCurrent->Deselect();
		pNext->Select();
		pCurrent = pNext;
		pNext = NULL;
	}

	if(pCurrent)
		return pCurrent->Update();
	return 0;
}

void dBScreen::DrawScreen()
{
	if(pCurrent)
		return pCurrent->Draw();
}

void dBScreen::SetNext(dBScreen *_pNext)
{
	pNext = _pNext;
}
