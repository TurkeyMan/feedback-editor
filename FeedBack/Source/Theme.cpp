#include "FeedBack.h"
#include "MFIni.h"
#include "Theme.h"

void LoadResources(MFIniLine *pLine, dBResourceCache *pResourceCache)
{
	while(pLine)
	{
		if(pLine->IsString(1, "font"))
		{
			pResourceCache->LoadFont(pLine->GetString(0), MFStr_TruncateExtension(pLine->GetString(2)));
		}
		else if(pLine->IsString(1, "material") || pLine->IsString(1, "image"))
		{
			pResourceCache->LoadMaterial(pLine->GetString(0), MFStr_TruncateExtension(pLine->GetString(2)));
		}
		else if(pLine->IsString(1, "model"))
		{
			pResourceCache->LoadModel(pLine->GetString(0), MFStr_TruncateExtension(pLine->GetString(2)));
		}
		else if(pLine->IsString(1, "sound"))
		{
			pResourceCache->LoadSound(pLine->GetString(0), MFStr_TruncateExtension(pLine->GetString(2)));
		}
		pLine = pLine->Next();
	}
}

dBTheme::dBTheme()
{
	resourceCache.Init();

	pThemeConfig = MFIni::Create("theme:theme");
	MFDebug_Assert(pThemeConfig, "Couldn't load theme theme.ini");

	MFIniLine *pLine = pThemeConfig->GetFirstLine();
	while(pLine)
	{
		if(pLine->IsSection("Theme"))
		{
			MFIniLine *pSub = pLine->Sub();
			while(pSub)
			{
				if(pSub->IsString(0, "name"))
				{
					pThemeName = pSub->GetString(1);
				}
				else if(pSub->IsString(0, "startscreen"))
				{
					pStartScreen = pSub->GetString(1);
				}
				pSub = pSub->Next();
			}
		}
		else if(pLine->IsSection("Resources"))
		{
			LoadResources(pLine->Sub(), &resourceCache);
		}
		else if(pLine->IsSection("Metrics"))
		{

		}
		pLine = pLine->Next();
	}
}

dBTheme::~dBTheme()
{
	MFIni::Destroy(pThemeConfig);

	resourceCache.Deinit();
}

dBThemeScreen::dBThemeScreen(const char *pScreenName, dBTheme *_pTheme)
{
	pTheme = _pTheme;

	resourceCache.Init();
	entityPool.Init(this);
	actionManager.Init(this);

	pThemeScreen = MFIni::Create(pScreenName);
	MFDebug_Assert(pThemeScreen, MFStr("Couldn't load screen '%s'", pScreenName));

	MFIniLine *pLine = pThemeScreen->GetFirstLine();
	while(pLine)
	{
		if(pLine->IsSection("Screen"))
		{
			MFIniLine *pSLine = pLine->Sub();
			while(pSLine)
			{
				if(pSLine->IsString(0, "name"))
				{
					pScreenName = pSLine->GetString(1);
				}
				else if(pSLine->IsString(0, "selection"))
				{
					// default selected entity
				}
				else if(pSLine->IsString(0, "clearcolour"))
				{
					clearColour = pSLine->GetVector4(1);
				}
				else if(pSLine->IsSection("Resources"))
				{
					LoadResources(pSLine->Sub(), &resourceCache);
				}
				else if(pSLine->IsSection("Metrics"))
				{
					MFIniLine *pSub = pSLine->Sub();
					while(pSub)
					{
						dBThemeMetric &metric = metrics.push();
						metric.pName = pSub->GetString(0);
						metric.pMetric = actionManager.ParseMetric(pSub->GetString(1));

						pSub = pSub->Next();
					}
				}
				else if(pSLine->IsSection("Actions"))
				{
					MFIniLine *pSub = pSLine->Sub();
					while(pSub)
					{
						dBThemeAction &action = actions.push();
						action.pName = pSub->GetString(0);
						action.pScript = actionManager.ParseScript(pSub->GetString(1));

						pSub = pSub->Next();
					}
				}
				else if(pSLine->IsSection("Entities"))
				{
					MFIniLine *pSub = pSLine->Sub();
					while(pSub)
					{
						if(pSub->IsString(0, "section"))
						{
							MFIniLine *pEntity = pSub->Sub();
							while(pEntity)
							{
								if(pEntity->IsString(0, "name"))
								{
									dBEntity *pNewEntity = entityPool.Create(pSub->GetString(1), pEntity->GetString(1));
									pNewEntity->Init(&actionManager, pSub->Sub());
									break;
								}

								pEntity = pEntity->Next();
							}
						}

						pSub = pSub->Next();
					}
				}

				pSLine = pSLine->Next();
			}
		}

		pLine = pLine->Next();
	}

	// init all the entities
	dBEntity *pE = NULL;
	while(pE = entityPool.Iterate(pE))
	{
		pE->SignalEvent("init", NULL);
	}
}

dBThemeScreen::~dBThemeScreen()
{
	MFIni::Destroy(pThemeScreen);

	entityPool.Deinit();
	resourceCache.Deinit();
}

void dBThemeScreen::Select()
{

}

int dBThemeScreen::Update()
{
	dBEntity *pE = NULL;
	while(pE = entityPool.Iterate(pE))
		pE->Update();

	actionManager.Update();

	return 0;
}

void dBThemeScreen::Draw()
{
	MFRenderer_SetClearColour(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
	MFRenderer_ClearScreen();

	MFView_Push();
	MFRect rect = { 0.f, 0.f, 1280, 720 };
	MFView_SetOrtho(&rect);

	dBEntity *pE = NULL;
	while(pE = entityPool.Iterate(pE))
		pE->Draw();

	MFView_Pop();
}

void dBThemeScreen::Deselect()
{

}

dBActionScript *dBThemeScreen::GetAction(const char *pName)
{
	for(int a=0; a<actions.size(); ++a)
	{
		if(!MFString_Compare(actions[a].pName, pName))
			return actions[a].pScript;
	}
	return NULL;
}

dBActionMetric *dBThemeScreen::GetMetric(const char *pName)
{
	for(int a=0; a<metrics.size(); ++a)
	{
		if(!MFString_Compare(metrics[a].pName, pName))
			return metrics[a].pMetric;
	}

	// search the theme for global metrics
	//...

	return NULL;
}
