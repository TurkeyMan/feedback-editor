#include "FeedBack.h"
#include "Fuji/MFFileSystem.h"
#include "Control.h"

FileSelectorScreen::FileSelectorScreen()
{
	frame.SetMaterial("Images/window");
}

FileSelectorScreen::~FileSelectorScreen()
{
	Destroy();
}

void FileSelectorScreen::Destroy()
{
	items.clear();

	selected = 0;
	listOffset = 0;
	directoryDepth = 0;

	message[0] = 0;
	path[0] = 0;
	extensions[0] = 0;
}

void FileSelectorScreen::Show(const char *pMessage, const char *pPath, const char *pFilters, FileCallback pCallback, uint32 _flags, void *_pUserData, float _width, float _height)
{
	Destroy();

	MFString_Copy(message, pMessage);
	MFString_Copy(path, pPath);
	if(pFilters)
		MFString_Copy(extensions, pFilters);
	else
		MFString_Copy(extensions, "*");
	pCompleteCallback = pCallback;

	// make sure path ends with a slash
	int len = MFString_Length(path);
	if(len && path[len-1] != ':' && path[len-1] != '/' && path[len-1] != '\\')
		path[len] = '/', path[len+1] = 0;

	// tokenise extensions
	extensions[MFString_Length(extensions)+1] = 0;
	char *pSlash = extensions;
	while((pSlash = MFString_Chr(pSlash, '|')))
	{
		*pSlash = 0;
		++pSlash;
	}

	selected = 0;
	listOffset = 0;

#if defined(MF_PSP)
	_width = 600.f;
	_height = 300.f;
#endif
	width = _width;
	height = _height;
	flags = _flags;
	pUserData = _pUserData;

	const float textHeight = TEXT_HEIGHT;

	pageMax = (int)(height/textHeight);

	typeBuffer[0] = 0;
	typeTimeout = 0.0f;

	ScanDirectory();

	Push();
}

int SortFiles(const void *elem1, const void *elem2)
{
	const FileListItem *pI1 = (const FileListItem*)elem1;
	const FileListItem *pI2 = (const FileListItem*)elem2;

	int pred = pI2->directory - pI1->directory;
	if(pred)
		return pred;
	return MFString_CaseCmp(pI1->filename, pI2->filename);
}

void FileSelectorScreen::ScanDirectory()
{
	items.clear();

	int sortStart = 0;

	if(flags & SF_ShowNone)
	{
		FileListItem &item = items.push();
		MFString_Copy(item.filename, MFStr("[%s]", MFTranslation_GetString(pStrings, MENU_NONE)));
		item.directory = false;
		++sortStart;
	}

	if(directoryDepth)
	{
		FileListItem &item = items.push();
		MFString_Copy(item.filename, ".. [Parent]");
		item.directory = true;
		++sortStart;
	}

	MFFindData fd;
	MFFind *pFind = MFFileSystem_FindFirst(MFStr("%s*", path), &fd);
	if(pFind)
	{
		do
		{
			if(fd.attributes & MFFA_Directory)
			{
				if(fd.pFilename[0] != '.' && !(flags & SF_NoFolders))
				{
					FileListItem &item = items.push();
					MFString_Copy(item.filename, fd.pFilename);
					item.directory = true;
				}
			}
			else
			{
				const char *pExt = extensions;
				while(*pExt)
				{
					if(MFString_PatternMatch(pExt, fd.pFilename))
					{
						FileListItem &item = items.push();
						MFString_Copy(item.filename, fd.pFilename);
						item.directory = false;
					}

					pExt += MFString_Length(pExt) + 1;
				}
			}
		}
		while(MFFileSystem_FindNext(pFind, &fd));

		MFFileSystem_FindClose(pFind);
	}

	// sort
	if(items.size() - sortStart > 1)
		qsort(&items[sortStart], items.size() - sortStart, sizeof(FileListItem), SortFiles);
}

void FileSelectorScreen::Update()
{
}

void FileSelectorScreen::UpdateInput()
{
	if(TestControl(dBCtrl_Menu_Cancel, GHCT_Once))
	{
		Pop();

		if(pCompleteCallback)
			pCompleteCallback(1, NULL, NULL, pUserData);
	}
	else if(TestControl(dBCtrl_Menu_Accept, GHCT_Once))
	{
		if(items[selected].directory)
		{
			// if it's '..', remove last directory, otherwise append the directory
			if(items[selected].filename[0] == '.' && items[selected].filename[1] == '.')
			{
				path[MFString_Length(path)-1] = 0;
				char *pSlash = MFString_RChr(path, '/');
				if(!pSlash)
					pSlash = MFString_RChr(path, ':');
				pSlash[1] = 0;
				--directoryDepth;
			}
			else
			{
				MFString_Cat(path, MFStr("%s/", items[selected].filename));
				++directoryDepth;
			}

			ScanDirectory();
			listOffset = selected = 0;
		}
		else
		{
			Pop();

			if(pCompleteCallback)
				pCompleteCallback(0, items[selected].filename, MFStr("%s%s", path, items[selected].filename), pUserData);
		}
	}
	else if(TestControl(dBCtrl_Menu_Up, GHCT_Delay))
	{
		selected = MFMax(selected-1, 0);
		listOffset = MFMin(selected, listOffset);
	}
	else if(TestControl(dBCtrl_Menu_Down, GHCT_Delay))
	{
		selected = MFMin(selected+1, (int)items.size()-1);
		listOffset = MFMax(listOffset, selected-(pageMax-1));
	}
	else if(TestControl(dBCtrl_Menu_Home, GHCT_Delay))
	{
		listOffset = selected = 0;
	}
	else if(TestControl(dBCtrl_Menu_End, GHCT_Delay))
	{
		selected = items.size()-1;
		listOffset = MFMax(listOffset, selected-(pageMax-1));
	}
	else if(TestControl(dBCtrl_Menu_PgUp, GHCT_Delay))
	{
		selected = MFMax(selected-(pageMax-1), 0);
		listOffset = MFMin(selected, listOffset);
	}
	else if(TestControl(dBCtrl_Menu_PgDn, GHCT_Delay))
	{
		selected = MFMin(selected+(pageMax-1), (int)items.size()-1);
		listOffset = MFMax(listOffset, selected-(pageMax-1));
	}

	// check for alpha-numeric keypress
	int pressed = 0;
	bool shift = MFInput_Read(Key_LShift, IDD_Keyboard) || MFInput_Read(Key_RShift, IDD_Keyboard);
	bool caps  = MFInput_GetKeyboardStatusState(KSS_CapsLock);

	for(int a=0; a<30; ++a)
	{
		if(MFInput_WasPressed(Key_A + a, IDD_Keyboard))
		{
			pressed = MFInput_KeyToAscii(Key_A + a, shift, caps);
			break;
		}
	}
	if(!pressed)
	{
		for(int a=0; a<14; ++a)
		{
			if(MFInput_WasPressed(Key_Comma + a, IDD_Keyboard))
			{
				pressed = MFInput_KeyToAscii(Key_Comma + a, shift, caps);
				break;
			}
		}
	}

	if(pressed)
	{
		// append keystroke
		int len = MFString_Length(typeBuffer);
		if(len < 255)
		{
			typeBuffer[len++] = pressed;
			typeBuffer[len] = 0;
			typeTimeout = 0.75f;

			// find first matching item in the list
			for(size_t a=0; a<items.size(); ++a)
			{
				if(!MFString_CaseCmpN(typeBuffer, items[a].filename, len))
				{
					selected = a;
					listOffset = MFClamp(selected-(pageMax-1), listOffset, selected);
					break;
				}
			}
		}
	}
	else if(typeTimeout > 0.0f)
	{
		typeTimeout -= MFSystem_TimeDelta();
		if(typeTimeout <= 0.0f)
			typeBuffer[0] = 0;
	}
}

void FileSelectorScreen::Draw()
{
	GHScreen::Draw();

	MFView_Push();

	MFRect rect;
	rect.x = MFDisplay_IsWidescreen() ? -106.0f : 0.0f;
	rect.y = 0.0f;
	rect.width = MFDisplay_IsWidescreen() ? 852.0f : 640.0f;
	rect.height = 480.0f;
	MFView_SetOrtho(&rect);

	char ext[256];
	const char *pExt = extensions;
	int a=0;
	for(a=0; pExt[a] || pExt[a+1]; ++a)
		ext[a] = pExt[a] ? pExt[a] : '|';
	ext[a] = 0;

	float textHeight = TEXT_HEIGHT;

	float w = width, h, x, y, lh = height - fmodf(height, textHeight);
	MFFont_GetStringWidth(pHeading, message, textHeight*2.f, w, -1, &h);
	float pathy = h;
	h += textHeight + 10;
	float listy = h;
	h += lh + 10;
	float patterny = h + 5;
	h += textHeight;

	x = 320.0f - w*0.5f;
	y = 240.0f - h*0.5f;

	MFRect rect2 = { x, y, w, h };
	frame.SetFrame(&rect2);
	frame.Draw();

	float pathWidth = MFFont_GetStringWidth(pText, "Path:", textHeight, w);
	float patternWidth = MFFont_GetStringWidth(pText, "Pattern:", textHeight, w);

	MFPrimitive_DrawUntexturedQuad(x+pathWidth+5, y-5 + pathy, w-pathWidth, textHeight + 5, MakeVector(0,0,0,1));
	MFPrimitive_DrawUntexturedQuad(x-5, y-5 + listy, w+10, lh + 10, MakeVector(0,0,0,1));
	MFPrimitive_DrawUntexturedQuad(x+patternWidth+5, y-5 + patterny, w-patternWidth, textHeight + 5, MakeVector(0,0,0,1));

	MFFont_DrawTextAnchored(pHeading, message, MakeVector(x, y-5, 0.0f), MFFontJustify_Top_Left, w, textHeight*2.f, MFVector::yellow);
	MFFont_DrawText2(pText, x, y-3 + pathy, textHeight, MFVector::white, "Path:");
	MFFont_DrawText2(pText, x+pathWidth+10, y-3 + pathy, textHeight, MFVector::white, path);
	MFFont_DrawText2(pText, x, y-3 + patterny, textHeight, MFVector::white, "Pattern:");
	MFFont_DrawText2(pText, x+patternWidth+10, y-3 + patterny, textHeight, MFVector::white, ext);

	y = y + listy;

	int maxItem = MFMin(listOffset+pageMax, (int)items.size());
	for(int a=listOffset; a<maxItem; ++a)
	{
		bool hilite = a == selected;

		if(hilite)
			MFPrimitive_DrawUntexturedQuad(x-5, y, w+10, textHeight, MakeVector(0,0,0.5f,1));

		MFFont_DrawText2(pText, x, y, textHeight, hilite ? MFVector::yellow : (items[a].directory ? MFVector::blue : MFVector::white), items[a].filename);
		y += textHeight;
	}

	MFView_Pop();
}
