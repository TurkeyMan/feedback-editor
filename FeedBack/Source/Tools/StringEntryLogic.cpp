#include "Fuji.h"
#include "MFInput.h"
#include "MFString.h"
#include "MFHeap.h"
#include "MFSystem.h"
#include "StringEntryLogic.h"

#if defined(MF_WINDOWS)
	#include <Windows.h>
	extern HWND apphWnd;
#endif

float StringEntryLogic::gRepeatDelay = 0.3f;
float StringEntryLogic::gRepeatRate = 0.06f;

OnScreenKeyboard *StringEntryLogic::pOSK = NULL;

StringEntryLogic::StringEntryLogic()
{
	pBuffer = NULL;;
	bufferLen = 0;
	stringLen = 0;
	cursorPos = 0;
	selectionStart = selectionEnd = 0;
	holdKey = 0;
	repeatDelay = 0.f;
	pChangeCallback = NULL;
	pUserData = NULL;
}

StringEntryLogic::StringEntryLogic(int bufferSize)
{
	stringLen = 0;
	cursorPos = 0;
	selectionStart = selectionEnd = 0;
	holdKey = 0;
	repeatDelay = 0.f;
	pChangeCallback = NULL;
	pUserData = NULL;

	Create(bufferSize);
}

void StringEntryLogic::StringCopyOverlap(char *pDest, const char *pSrc)
{
	if(pDest < pSrc)
	{
		while(*pSrc)
			*pDest++ = *pSrc++;
		*pDest = 0;
	}
	else
	{
		int len = MFString_Length(pSrc);

		while(len >= 0)
		{
			pDest[len] = pSrc[len];
			--len;
		}
	}
}

void StringEntryLogic::ClearSelection()
{
	if(selectionStart == selectionEnd)
		return;

	int selMin = MFMin(selectionStart, selectionEnd);
	int selMax = MFMax(selectionStart, selectionEnd);

	StringCopyOverlap(&pBuffer[selMin], &pBuffer[selMax]);

	cursorPos = selMin;
	stringLen -= selMax - selMin;

	selectionStart = selectionEnd = cursorPos;

	if(pChangeCallback)
		pChangeCallback(pBuffer, pUserData);
}

void StringEntryLogic::Create(int bufferSize)
{
	pBuffer = (char*)MFHeap_Alloc(bufferSize);
	pBuffer[0] = 0;
	bufferLen = bufferSize;
}

void StringEntryLogic::Destroy()
{
	if(pBuffer)
		MFHeap_Free(pBuffer);
}

void StringEntryLogic::Update()
{
	bool shiftL = !!MFInput_Read(Key_LShift, IDD_Keyboard);
	bool shiftR = !!MFInput_Read(Key_RShift, IDD_Keyboard);
	bool ctrlL = !!MFInput_Read(Key_LControl, IDD_Keyboard);
	bool ctrlR = !!MFInput_Read(Key_RControl, IDD_Keyboard);

	int keyPressed = 0;

	bool shift = shiftL || shiftR;
	bool ctrl = ctrlL || ctrlR;

#if defined(MF_WINDOWS)
	if(ctrl && MFInput_WasPressed(Key_C, IDD_Keyboard) && selectionStart != selectionEnd)
	{
		BOOL opened = OpenClipboard(apphWnd);

		if(opened)
		{
			int selMin = MFMin(selectionStart, selectionEnd);
			int selMax = MFMax(selectionStart, selectionEnd);

			int numChars = selMax-selMin;

			HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, numChars + 1);
			char *pString = (char*)GlobalLock(hData);

			MFString_CopyN(pString, pBuffer + selMin, numChars);
			pString[numChars] = 0;

			GlobalUnlock(hData);

			EmptyClipboard();
			SetClipboardData(CF_TEXT, hData);

			CloseClipboard();
		}
	}
	else if(ctrl && MFInput_WasPressed(Key_X, IDD_Keyboard) && selectionStart != selectionEnd)
	{
		BOOL opened = OpenClipboard(apphWnd);

		if(opened)
		{
			int selMin = MFMin(selectionStart, selectionEnd);
			int selMax = MFMax(selectionStart, selectionEnd);

			int numChars = selMax-selMin;

			HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, numChars + 1);
			char *pString = (char*)GlobalLock(hData);

			MFString_CopyN(pString, pBuffer + selMin, numChars);
			pString[numChars] = 0;

			GlobalUnlock(hData);

			EmptyClipboard();
			SetClipboardData(CF_TEXT, hData);

			CloseClipboard();

			ClearSelection();
		}
	}
	else if(ctrl && MFInput_WasPressed(Key_V, IDD_Keyboard))
	{
		BOOL opened = OpenClipboard(apphWnd);

		if(opened)
		{
			ClearSelection();

			HANDLE hData = GetClipboardData(CF_TEXT);
			const char *pString = (const char*)GlobalLock(hData);

			int pasteLen = MFString_Length(pString);
			MFString_Copy(pBuffer + cursorPos, MFStr("%s%s", pString, pBuffer + cursorPos));

			GlobalUnlock(hData);

			cursorPos += pasteLen;
			selectionStart = selectionEnd = cursorPos;
			stringLen += pasteLen;

			GlobalUnlock(hData);

			CloseClipboard();

			if(pasteLen && pChangeCallback)
				pChangeCallback(pBuffer, pUserData);
		}
	}
	else
#endif
	{
		// check for new keypresses
		for(int a=0; a<255; a++)
		{
			if(MFInput_WasPressed(a, IDD_Keyboard))
			{
				keyPressed = a;
				holdKey = a;
				repeatDelay = gRepeatDelay;
			}
		}

		// handle repeat keys
		if(holdKey && MFInput_Read(holdKey, IDD_Keyboard))
		{
			repeatDelay -= MFSystem_TimeDelta();
			if(repeatDelay <= 0.f)
			{
				keyPressed = holdKey;
				repeatDelay += gRepeatRate;
			}
		}
		else
			holdKey = 0;

		// if there was a new key press
		if(keyPressed)
		{
			switch(keyPressed)
			{
				case Key_Backspace:
				case Key_Delete:
				{
					if(selectionStart != selectionEnd)
					{
						ClearSelection();
					}
					else
					{
						if(keyPressed == Key_Backspace && cursorPos > 0)
						{
							StringCopyOverlap(&pBuffer[cursorPos-1], &pBuffer[cursorPos]);
							--cursorPos;
							--stringLen;

							if(pChangeCallback)
								pChangeCallback(pBuffer, pUserData);
						}
						else if(keyPressed == Key_Delete && cursorPos < stringLen)
						{
							StringCopyOverlap(&pBuffer[cursorPos], &pBuffer[cursorPos+1]);
							--stringLen;

							if(pChangeCallback)
								pChangeCallback(pBuffer, pUserData);
						}
					}
					break;
				}

				case Key_Left:
				case Key_Right:
				case Key_Home:
				case Key_End:
				{
					if(ctrl)
					{
						if(keyPressed == Key_Left)
						{
							while(cursorPos && MFIsWhite(pBuffer[cursorPos-1]))
								--cursorPos;
							if(MFIsAlphaNumeric(pBuffer[cursorPos-1]))
							{
								while(cursorPos && MFIsAlphaNumeric(pBuffer[cursorPos-1]))
									--cursorPos;
							}
							else if(cursorPos)
							{
								--cursorPos;
								while(cursorPos && pBuffer[cursorPos-1] == pBuffer[cursorPos])
									--cursorPos;
							}
						}
						else if(keyPressed == Key_Right)
						{
							while(cursorPos < stringLen && MFIsWhite(pBuffer[cursorPos]))
								++cursorPos;
							if(MFIsAlphaNumeric(pBuffer[cursorPos]))
							{
								while(cursorPos < stringLen && MFIsAlphaNumeric(pBuffer[cursorPos]))
									++cursorPos;
							}
							else if(cursorPos < stringLen)
							{
								++cursorPos;
								while(cursorPos < stringLen && pBuffer[cursorPos] == pBuffer[cursorPos-1])
									++cursorPos;
							}
						}
						else if(keyPressed == Key_Home)
							cursorPos = 0;
						else if(keyPressed == Key_End)
							cursorPos = stringLen;
					}
					else
					{
						if(keyPressed == Key_Left)
							cursorPos = (!shift && selectionStart != selectionEnd ? MFMin(selectionStart, selectionEnd) : MFMax(cursorPos-1, 0));
						else if(keyPressed == Key_Right)
							cursorPos = (!shift && selectionStart != selectionEnd ? MFMax(selectionStart, selectionEnd) : MFMin(cursorPos+1, stringLen));
						else if(keyPressed == Key_Home)
							cursorPos = 0;	// TODO: if multiline, go to start of line..
						else if(keyPressed == Key_End)
							cursorPos = stringLen;	// TODO: if multiline, go to end of line...
					}

					if(shift)
						selectionEnd = cursorPos;
					else
						selectionStart = selectionEnd = cursorPos;

					break;
				}

				default:
				{
					bool caps = MFInput_GetKeyboardStatusState(KSS_CapsLock);
					int ascii = MFInput_KeyToAscii(keyPressed, shift, caps);

					if(ascii && stringLen < bufferLen-1)
					{
						// if selection range, delete selection
						ClearSelection();

						StringCopyOverlap(&pBuffer[cursorPos+1], &pBuffer[cursorPos]);
						pBuffer[cursorPos] = ascii;
						++cursorPos;
						++stringLen;

						selectionStart = selectionEnd = cursorPos;

						if(pChangeCallback)
							pChangeCallback(pBuffer, pUserData);
					}
					break;
				}
			}
		}
	}
}

void StringEntryLogic::SetString(const char *pString)
{
	int len = MFString_Length(pString);
	if(len < bufferLen-1)
	{
		MFString_Copy(pBuffer, pString);
		selectionStart = selectionEnd = cursorPos = stringLen = len;

		if(pChangeCallback)
			pChangeCallback(pBuffer, pUserData);
	}
}
