#include "FeedBack.h"
#include "Fuji/MFIni.h"
#include "Fuji/MFSystem.h"
#include "Fuji/MFFileSystem.h"

#include "Settings.h"

dBSettings gConfig =
{
	// General
	{
		MFLang_Unknown,
	},

	// Screen
	{
		1024, 576,
		false,
		false
	},

	// Sound
	{
		0.8f,
		1.0f, 1.0f, 1.0f,
		0.8f,
		0.8f,
		0.8f
	},
/*
	// Gameplay
	{

	},
*/
	// Editor
	{
		"",
		"",
		""
	},
	// Controls
	{
		{ false, false }
	}
};

void LoadConfigFile(const char *pConfig)
{
	// load config file
	MFIni *pIni = MFIni::Create(pConfig);

	if(pIni)
	{
		MFIniLine *pLine = pIni->GetFirstLine();

		while(pLine)
		{
			if(pLine->IsSection("Settings"))
			{
				MFIniLine *pSub = pLine->Sub();

				while(pSub)
				{
					if(pSub->IsSection("General"))
					{
						MFIniLine *pOption = pSub->Sub();

						while(pOption)
						{
							if(pOption->IsString(0, "language"))
							{
								gConfig.general.language = MFLang_Unknown;

								const char *pString = pOption->GetString(1);
								if(pString)
								{
									for(int a=0; a<MFLang_Max; ++a)
									{
										if(!MFString_CaseCmp(pString, MFTranslation_GetLanguageName((MFLanguage)a, false)))
										{
											gConfig.general.language = (MFLanguage)a;
											break;
										}
									}
								}
							}

							pOption = pOption->Next();
						}
					}
					else if(pSub->IsSection("Screen"))
					{
						MFIniLine *pOption = pSub->Sub();

						while(pOption)
						{
							if(pOption->IsString(0, "x_res"))
							{
								gConfig.screen.xRes = pOption->GetInt(1);
							}
							else if(pOption->IsString(0, "y_res"))
							{
								gConfig.screen.yRes = pOption->GetInt(1);
							}
							else if(pOption->IsString(0, "fullscreen"))
							{
								gConfig.screen.fullscreen = !!pOption->GetInt(1);
							}
							else if(pOption->IsString(0, "low_detail"))
							{
								gConfig.screen.lowDetail = !!pOption->GetInt(1);
							}

							pOption = pOption->Next();
						}
					}
					else if(pSub->IsSection("Sound"))
					{
						MFIniLine *pOption = pSub->Sub();

						while(pOption)
						{
							if(pOption->IsString(0, "music_level"))
							{
								gConfig.sound.masterLevel = pOption->GetFloat(1);
							}
							else if(pOption->IsString(0, "fx_level"))
							{
								gConfig.sound.fxLevel = pOption->GetFloat(1);
							}
							else if(pOption->IsString(0, "tick_level"))
							{
								gConfig.sound.tickLevel = pOption->GetFloat(1);
							}
							else if(pOption->IsString(0, "clap_level"))
							{
								gConfig.sound.clapLevel = pOption->GetFloat(1);
							}

							pOption = pOption->Next();
						}
					}
					else if(pSub->IsSection("Gameplay"))
					{
						MFIniLine *pOption = pSub->Sub();

						while(pOption)
						{

							pOption = pOption->Next();
						}
					}
					else if(pSub->IsSection("Editor"))
					{
						MFIniLine *pOption = pSub->Sub();

						while(pOption)
						{
							if(pOption->IsString(0, "last_opened"))
							{
								const char *pString = pOption->GetString(1);
								if(pString)
									MFString_Copy(gConfig.editor.lastOpenedChart, pString);
							}
							else if(pOption->IsString(0, "save_action"))
							{
								const char *pString = pOption->GetString(1);
								if(pString)
									MFString_Copy(gConfig.editor.saveAction, pString);
							}
							else if(pOption->IsString(0, "fretboard"))
							{
								const char *pString = pOption->GetString(1);
								if(pString)
									MFString_Copy(gConfig.editor.editorFretboard, pString);
							}

							pOption = pOption->Next();
						}
					}
					else if(pSub->IsSection("Controls"))
					{
						MFIniLine *pOption = pSub->Sub();

						while(pOption)
						{
							if(pOption->IsString(0, "lefty_flip_1"))
							{
								int flip = pOption->GetInt(1);
								gConfig.controls.leftyFlip[0] = !!flip;
							}
							else if(pOption->IsString(0, "lefty_flip_2"))
							{
								int flip = pOption->GetInt(1);
								gConfig.controls.leftyFlip[1] = !!flip;
							}
							else
							{
								if(pOption->GetStringCount() >= 2)
								{
									const char *pControl = pOption->GetString(0);

									for(int a=0; a<dBCtrl_Max; ++a)
									{
										if(!MFString_CaseCmp(pControl, GHControl::GetControlString(a)))
										{
											gConfig.controls.controls[a].Set(pOption);
											break;
										}
									}
								}
							}

							pOption = pOption->Next();
						}
					}

					pSub = pSub->Next();
				}
			}
			else if(pLine->IsSection("System"))
			{
				MFSystem_InitFromSettings(pLine);
			}

			pLine = pLine->Next();
		}

		MFIni::Destroy(pIni);
	}
}

void SaveConfigFile(const char *pConfig)
{
	MFFile *pFile = MFFileSystem_Open(MFStr("game:%s.ini", pConfig), MFOF_Binary | MFOF_Write);

	if(pFile)
	{
		// write program settings
		const char * pString = "[Settings]\r\n{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// General settings
		pString = "\t[General]\r\n\t{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = MFStr("\t\tlanguage = %s\r\n", MFTranslation_GetLanguageName(gConfig.general.language, false));
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "\t}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// Screen settings
		pString = "\t[Screen]\r\n\t{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = MFStr("\t\tx_res = %d\r\n", gConfig.screen.xRes);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\ty_res = %d\r\n", gConfig.screen.yRes);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\tfullscreen = %d\r\n", gConfig.screen.fullscreen);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\tlow_detail = %d\r\n", gConfig.screen.lowDetail);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "\t}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// Sound settings
		pString = "\t[Sound]\r\n\t{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = MFStr("\t\tmusic_level = %.2g\r\n", gConfig.sound.masterLevel);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\tfx_level = %.2g\r\n", gConfig.sound.fxLevel);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\ttick_level = %.2g\r\n", gConfig.sound.tickLevel);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\tclap_level = %.2g\r\n", gConfig.sound.clapLevel);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "\t}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "\t[Gameplay]\r\n\t{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "\t}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// Editor settings
		pString = "\t[Editor]\r\n\t{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = MFStr("\t\tlast_opened = \"%s\"\r\n", gConfig.editor.lastOpenedChart);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\tsave_action = \"%s\"\r\n", gConfig.editor.saveAction);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		if(gConfig.editor.editorFretboard[0])
		{
			pString = MFStr("\t\tfretboard = \"%s\"\r\n", gConfig.editor.editorFretboard);
			MFFile_Write(pFile, pString, MFString_Length(pString), false);
		}

		pString = "\t}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// Control settings
		pString = "\t[Controls]\r\n\t{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = MFStr("\t\tlefty_flip_1 = %d\r\n", gConfig.controls.leftyFlip[0] ? 1 : 0);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);
		pString = MFStr("\t\tlefty_flip_2 = %d\r\n", gConfig.controls.leftyFlip[1] ? 1 : 0);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		for(int a=0; a<dBCtrl_Max; ++a)
		{
			pString = gConfig.controls.controls[a].GetSettingsString((dBControlType)a);
			MFFile_Write(pFile, pString, MFString_Length(pString), false);
		}

		pString = "\t}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// write system settings
		pString = "\r\n[System]\r\n{\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		// get a settings string from fuji and write it out here...
		pString = MFSystem_GetSettingString(1);
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		pString = "}\r\n";
		MFFile_Write(pFile, pString, MFString_Length(pString), false);

		MFFile_Close(pFile);
	}
}
