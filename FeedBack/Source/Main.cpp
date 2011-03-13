#include "FeedBack.h"
#include "MFSystem.h"
#include "MFDisplay.h"
#include "MFFileSystem.h"
#include "FileSystem/MFFileSystemNative.h"
#include "FileSystem/MFFileSystemCachedFile.h"
#include "FileSystem/MFFileSystemZipFile.h"
#include "MFView.h"
#include "MFPrimitive.h"
#include "MFTexture.h"
#include "MFIni.h"
#include "MFThread.h"
#include "DebugMenu.h"
#include "Control.h"

#include "Screens/Nationality.h"
#include "Screens/MainMenu.h"
#include "Theme.h"

// this is for the program icon
#if defined(_WINDOWS)
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include "resource.h"
#endif

MFSystemCallbackFunction pInitFujiFS = NULL;

dBTheme *gpTheme = NULL;

MFFont *pHeading = NULL;
MFFont *pText = NULL;

MFStringTable *pStrings = NULL;

extern MenuItemBool bRenderNoteTimes;
extern MenuItemBool bMetronome;
extern MenuItemBool bClaps;
extern MenuItemIntString bHalfSpeed;
extern MenuItemBool bHalfFrets;
MenuItemFloat gScrollSpeed;

void ScanForThemes(const char *pThemesPath)
{
	MFFindData fd;
	MFFind *pFind = MFFileSystem_FindFirst(MFStr("%s*", pThemesPath), &fd);
	if(pFind)
	{
		do
		{
			if((fd.attributes & MFFA_Directory) && fd.pFilename[0] != '.')
			{
				// validate theme, and add to theme list
				if(MFFileSystem_Exists(MFStr("%s%s/theme.ini", pThemesPath, fd.pFilename)))
				{
					dBThemeList &theme = gGameData.themes.push();
					MFString_Copy(theme.themeName, fd.pFilename);
					MFString_Copy(theme.themePath, MFStr("%s%s/", pThemesPath, fd.pFilename));

					// set default theme
					if(gGameData.themes.size() == 1 || !MFString_CaseCmp(theme.themeName, "Default"))
						gGameData.pCurrentTheme = &theme;
				}
			}
		}
		while(MFFileSystem_FindNext(pFind, &fd));

		MFFileSystem_FindClose(pFind);
	}
}

int ScanForSongFolders(const char *pGroupPath, dBSongGroup *pGroup)
{
	int numSongsFound = 0;

	MFFindData fd;
	MFFind *pFind = MFFileSystem_FindFirst(MFStr("%s*", pGroupPath), &fd);
	if(pFind)
	{
		do
		{
			if((fd.attributes & MFFA_Directory) && fd.pFilename[0] != '.')
			{
				MFFindData fd2;
				MFFind *pFindChart = MFFileSystem_FindFirst(MFStr("%s%s/*", pGroupPath, fd.pFilename), &fd2);
				if(pFindChart)
				{
					do
					{
						if(!(fd2.attributes & MFFA_Directory) && fd2.pFilename[0] != '.')
						{
							const char *pExt = MFString_GetFileExtension(fd2.pFilename);
							if(pExt && (!MFString_CaseCmp(pExt, ".chart") || !MFString_CaseCmp(pExt, ".midi") || !MFString_CaseCmp(pExt, ".mid")))
							{
								dBSongList &song = pGroup->songs.push();
								MFString_Copy(song.chartPath, MFStr("%s%s/%s", pGroupPath, fd.pFilename, fd2.pFilename));
								++numSongsFound;
							}
						}
					}
					while(MFFileSystem_FindNext(pFindChart, &fd2));

					MFFileSystem_FindClose(pFindChart);
				}
			}
		}
		while(MFFileSystem_FindNext(pFind, &fd));

		MFFileSystem_FindClose(pFind);
	}

	return numSongsFound;
}

void ScanForSongs(const char *pSongsPath)
{
	dBSongGroup &defGroup = gGameData.songGroups[0];

	ScanForSongFolders(pSongsPath, &defGroup);

	MFFindData fd;
	MFFind *pFind = MFFileSystem_FindFirst(MFStr("%s*", pSongsPath), &fd);
	if(pFind)
	{
		do
		{
			if((fd.attributes & MFFA_Directory) && fd.pFilename[0] != '.')
			{
				// check if this is a song, or a song group...
				bool isGroup = true;

				MFFindData fd2;
				MFFind *pFindChart = MFFileSystem_FindFirst(MFStr("%s%s/*", pSongsPath, fd.pFilename), &fd2);
				if(pFindChart)
				{
					do
					{
						if(!(fd2.attributes & MFFA_Directory) && fd2.pFilename[0] != '.')
						{
							const char *pExt = MFString_GetFileExtension(fd2.pFilename);
							if(pExt && (!MFString_CaseCmp(pExt, ".chart") || !MFString_CaseCmp(pExt, ".midi") || !MFString_CaseCmp(pExt, ".mid")))
							{
								isGroup = false;
								break;
							}
						}
					}
					while(MFFileSystem_FindNext(pFindChart, &fd2));

					MFFileSystem_FindClose(pFindChart);
				}

				if(isGroup)
				{
					dBSongGroup &group = gGameData.songGroups.push();
					MFString_Copy(group.groupName, fd.pFilename);
//					group.pGroupLogo = MFMaterial_Create(MFStr("%s%s/folder", pSongsPath, fd.pFilename));
					group.pGroupLogo = NULL;
					// if it's the missing material, destroy and load the theme default instead..

					// scan for songs in the group
					ScanForSongFolders(MFStr("%s%s/", pSongsPath, fd.pFilename), &group);
				}
			}
			else
			{
				const char *pExt = MFString_GetFileExtension(fd.pFilename);
				if(pExt && (!MFString_CaseCmp(pExt, ".chart") || !MFString_CaseCmp(pExt, ".midi") || !MFString_CaseCmp(pExt, ".mid")))
				{
					dBSongList &song = defGroup.songs.push();
					MFString_Copy(song.chartPath, MFStr("%s%s", pSongsPath, fd.pFilename));
					song.pChart = NULL;
				}
			}
		}
		while(MFFileSystem_FindNext(pFind, &fd));

		MFFileSystem_FindClose(pFind);
	}
}

void ScanForPackages(const char *pPackagesPath)
{
	MFFindData fd;
	MFFind *pFind = MFFileSystem_FindFirst(MFStr("%s*.zip", pPackagesPath), &fd);
	if(pFind)
	{
		do
		{
			MFFile *pFile = MFFileSystem_Open(MFStr("%s%s", pPackagesPath, fd.pFilename));
			if(pFile)
			{
				// mount zip and scan contents
				dBPackage &package = gGameData.packages.push();
				MFString_Copy(package.packageFilename, MFStr("%s%s", pPackagesPath, fd.pFilename));
				MFString_Copy(package.mountPoint, MFStr_TruncateExtension(fd.pFilename));

				// attempt to cache the zip archive
				MFOpenDataCachedFile cachedOpen;
				cachedOpen.cbSize = sizeof(MFOpenDataCachedFile);
				cachedOpen.openFlags = MFOF_Read | MFOF_Binary | MFOF_Cached_CleanupBaseFile;
				cachedOpen.maxCacheSize = 128*1024; // 128k cache for packages should be plenty
				cachedOpen.pBaseFile = pFile;

				MFFile *pCachedFile = MFFile_Open(MFFileSystem_GetInternalFileSystemHandle(MFFSH_CachedFileSystem), &cachedOpen);
				if(pCachedFile)
					pFile = pCachedFile;

				// mount the zip archive.
				MFMountDataZipFile zipMountData;
				zipMountData.cbSize = sizeof(MFMountDataZipFile);
				zipMountData.flags = 0;
				zipMountData.priority = MFMP_Normal;
				zipMountData.pMountpoint = package.mountPoint;
				zipMountData.pZipArchive = pFile;
				MFFileSystem_Mount(MFFileSystem_GetInternalFileSystemHandle(MFFSH_ZipFileSystem), &zipMountData);

				// scan the package for stuff..
				char temp[256];
				MFString_Copy(temp, MFStr("%s:", package.mountPoint));

				ScanForThemes(temp);
				ScanForSongs(temp);
				ScanForPackages(temp);
			}
		}
		while(MFFileSystem_FindNext(pFind, &fd));

		MFFileSystem_FindClose(pFind);
	}
}

void Game_InitFilesystem()
{
	// mount the game directory
	MFFileSystemHandle hNative = MFFileSystem_GetInternalFileSystemHandle(MFFSH_NativeFileSystem);
	MFMountDataNative mountData;
	mountData.cbSize = sizeof(MFMountDataNative);
	mountData.priority = MFMP_Normal;
	mountData.flags = MFMF_FlattenDirectoryStructure | MFMF_Recursive;
	mountData.pMountpoint = "game";
#if defined(MF_IPHONE)
	mountData.pPath = MFFile_SystemPath();
#else
	mountData.pPath = MFFile_SystemPath("../Data/");
#endif
	MFFileSystem_Mount(hNative, &mountData);

	if(pInitFujiFS)
		pInitFujiFS();
}

// initialise game stuff after fuji has finished initialising.
void Game_Init()
{
	MFCALLSTACK;

	// create debug menu items
	DebugMenu_AddMenu("Guitar Hero", DebugMenu_GetRootMenu());
	DebugMenu_AddItem("Show note times", "Guitar Hero", &bRenderNoteTimes);
	DebugMenu_AddItem("Show halfFrets", "Guitar Hero", &bHalfFrets);
	DebugMenu_AddItem("Metronome", "Guitar Hero", &bMetronome);
	DebugMenu_AddItem("Claps", "Guitar Hero", &bClaps);
	DebugMenu_AddItem("Half Speed", "Guitar Hero", &bHalfSpeed);
	DebugMenu_AddItem("Scroll Speed", "Guitar Hero", &gScrollSpeed);

	MFFileSystemHandle hNative = MFFileSystem_GetInternalFileSystemHandle(MFFSH_NativeFileSystem);

	// mount the game directory
	MFMountDataNative mountData;
	mountData.cbSize = sizeof(MFMountDataNative);
	mountData.priority = MFMP_Normal;
	mountData.flags = MFMF_DontCacheTOC;
	mountData.pMountpoint = "game";
	mountData.pPath = MFFile_SystemPath();
	MFFileSystem_Mount(hNative, &mountData);

	mountData.flags = MFMF_DontCacheTOC;
	mountData.pMountpoint = "songs";
	mountData.pPath = MFFile_SystemPath("Songs/");
	MFFileSystem_Mount(hNative, &mountData);

	mountData.flags = MFMF_OnlyAllowExclusiveAccess | MFMF_FlattenDirectoryStructure | MFMF_Recursive;
	mountData.pMountpoint = "packages";
	mountData.pPath = MFFile_SystemPath("Packages/");
	MFFileSystem_Mount(hNative, &mountData);

	// iterate themes directory to build a list of themes...
	ScanForThemes("game:Themes/");

	// iterate songs directory to build a list of available songs...
	dBSongGroup &defGroup = gGameData.songGroups.push();
	MFString_Copy(defGroup.groupName, "Songs");
	ScanForSongs("songs:");

	// iterate pagkages to build a list of available packages... (adding to the themes/songs/etc)
	ScanForPackages("packages:");

	// mount the default theme
//	MFFileSystem_MountFujiPath("default-theme", gGameData.pCurrentTheme->themePath, MFMP_Normal, MFMF_FlattenDirectoryStructure);

	// load the config file
	GHControl::InitControls();
	LoadConfigFile("Config");

	// mount selected theme (from config file)
//	MFFileSystem_MountFujiPath("theme", gGameData.pCurrentTheme->themePath, MFMP_AboveNormal, MFMF_FlattenDirectoryStructure);

	// HACK: Mount the default theme...
	mountData.flags = MFMF_DontCacheTOC;
	mountData.pMountpoint = "theme";
	mountData.pPath = MFFile_SystemPath("Themes/Default/");
	MFFileSystem_Mount(hNative, &mountData);

	// init runtime data
	dBActionManager::InitManager();
	dBEntityManager::InitManager();
	dBRuntimeArgs::Init();

	// load the theme
	gpTheme = new dBTheme;
	gConfig.general.language = MFLang_English;

	// load global data (TODO: just get these from the theme!)
	pHeading = MFFont_Create("CandombeOutline");
	pText = MFFont_Create("Arial");

	gGameData.songGroups[0].pGroupLogo = MFMaterial_Create("defaultSongGroup");

	// begin game
	if(gConfig.general.language == MFLang_Unknown)
	{
		dBScreen::SetNext(new NationalityScreen);
	}
	else
	{
		// begin theme's main screen
		pStrings = MFTranslation_LoadStringTable("Strings", gConfig.general.language, MFLang_English);
//		dBScreen::SetNext(new MainMenuScreen);
		dBScreen::SetNext(new dBThemeScreen(gpTheme->GetStartScreen(), gpTheme));
	}
}

// Per frame game update.
void Game_Update()
{
	MFCALLSTACKc;

	GHControl::UpdateControls();
	dBScreen::UpdateScreen();
}


// Per frame game draw.
void Game_Draw()
{
	MFCALLSTACKc;

	// Clear Screen.
	MFRenderer_SetClearColour(0, 0, 0, 0);
	MFRenderer_ClearScreen();

	dBScreen::DrawScreen();
}

void Game_Deinit()
{
	MFCALLSTACK;

	dBScreen::GetCurrent()->Deselect();

	SaveConfigFile("Config");

	dBRuntimeArgs::Deinit();
	dBActionManager::DeinitManager();
	dBEntityManager::DeinitManager();

	// clean up fonts
	MFFont_Destroy(pHeading);
	MFFont_Destroy(pText);
//	MFFont_Destroy(pFancy);

	DebugMenu_DestroyMenu("Guitar Hero");
}

void CenterText(float y, float height, const MFVector &colour, const char *pText, MFFont *pFont, bool outline, const MFVector &outlineColour)
{
	MFRect rect;
	MFView_GetOrthoRect(&rect);

	float halfWidth = MFFont_GetStringWidth(pFont, pText, height, rect.width) * 0.5f;

	if(outline)
	{
		MFFont_DrawText(pFont, (rect.x + rect.width*0.5f - 2) - halfWidth, y-2, height, outlineColour, pText);
		MFFont_DrawText(pFont, (rect.x + rect.width*0.5f + 2) - halfWidth, y-2, height, outlineColour, pText);
		MFFont_DrawText(pFont, (rect.x + rect.width*0.5f - 2) - halfWidth, y+2, height, outlineColour, pText);
		MFFont_DrawText(pFont, (rect.x + rect.width*0.5f + 2) - halfWidth, y+2, height, outlineColour, pText);
	}

	MFFont_DrawText(pFont, (rect.x + rect.width*0.5f) - halfWidth, y, height, colour, pText);
}

float TestControl(dBControlType ctrl, GHControlType type)
{
	return gConfig.controls.controls[ctrl].Test(type) ? 1.0f : 0.0f;
}

int GameMain(MFInitParams *pInitParams)
{
	MFRand_Seed((uint32)MFSystem_ReadRTC());

#if !defined(_PSP)
	gDefaults.display.displayWidth = 1024;
	gDefaults.display.displayHeight = 576;
#endif
//	gDefaults.input.useDirectInputKeyboard = false;
	gDefaults.input.useXInput = false;
//	gDefaults.system.threadPriority = MFPriority_AboveNormal;
	gDefaults.display.pWindowTitle = "FeedBack Chart Editor v0.96b";
#if defined(_WINDOWS)
	gDefaults.display.pIcon = MAKEINTRESOURCE(IDI_ICON1);
#endif

	MFSystem_RegisterSystemCallback(MFCB_InitDone, Game_Init);
	MFSystem_RegisterSystemCallback(MFCB_Update, Game_Update);
	MFSystem_RegisterSystemCallback(MFCB_Draw, Game_Draw);
	MFSystem_RegisterSystemCallback(MFCB_Deinit, Game_Deinit);

	pInitFujiFS = MFSystem_RegisterSystemCallback(MFCB_FileSystemInit, Game_InitFilesystem);

	return MFMain(pInitParams);
}

#if defined(MF_WINDOWS) || defined(_WINDOWS)
#include <windows.h>

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	MFInitParams initParams;
	MFZeroMemory(&initParams, sizeof(MFInitParams));
	initParams.hInstance = hInstance;
	initParams.pCommandLine = lpCmdLine;

	return GameMain(&initParams);
}

#elif defined(MF_PSP) || defined(_PSP)
#include <pspkernel.h>

int main(int argc, char *argv[])
{
	MFInitParams initParams;
	MFZeroMemory(&initParams, sizeof(MFInitParams));
	initParams.argc = argc;
	initParams.argv = argv;

	int r = GameMain(&initParams);

	sceKernelExitGame();
	return r;
}

#else

int main(int argc, char *argv[])
{
	MFInitParams initParams;
	MFZeroMemory(&initParams, sizeof(MFInitParams));
	initParams.argc = argc;
	initParams.argv = (const char**)argv;

	return GameMain(&initParams);
}

#endif
