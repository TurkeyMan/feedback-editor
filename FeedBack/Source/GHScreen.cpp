#include "FeedBack.h"
#include "GHScreen.h"

GHScreen *GHScreen::pStack = NULL;

void GHScreen::Push()
{
	pNext = pStack;
	pStack = this;

	Activate();
}

void GHScreen::Pop()
{
	pStack = pStack->pNext;
}

void GHScreen::UpdateScreens()
{
	if(pStack)
		pStack->UpdateInput();

	if(pStack)
		pStack->Update();
}

void GHScreen::DrawScreens()
{
	if(pStack)
		pStack->Draw();
}

void GHScreen::Update()
{
	if(pNext)
		pNext->Update();
}

void GHScreen::UpdateInput()
{
	if(pNext)
		pNext->UpdateInput();
}

void GHScreen::Draw()
{
	if(pNext)
		pNext->Draw();	
}
