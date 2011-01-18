#if !defined(_THEME_H)
#define _THEME_H

#include "Tools/ResourceCache.h"
#include "Gadgets/Action.h"
#include "Gadgets/Entity.h"

struct dBThemeAction
{
	const char *pName;
	dBActionScript *pScript;
};

struct dBThemeMetric
{
	const char *pName;
	dBActionMetric *pMetric;
};

class dBTheme
{
public:
	dBTheme();
	~dBTheme();

	const char *GetStartScreen() { return pStartScreen; }

protected:
	MFIni *pThemeConfig;
	const char *pThemeName;
	const char *pStartScreen;

	dBResourceCache resourceCache;
};

class dBThemeScreen : public dBScreen
{
public:
	dBThemeScreen(const char *pScreenName, dBTheme *pTheme);
	virtual ~dBThemeScreen();

	virtual void Select();
	virtual int Update();
	virtual void Draw();
	virtual void Deselect();

	dBActionManager *GetActionManager() { return &actionManager; }
	dBEntityManager *GetEntityPool() { return &entityPool; }
	dBResourceCache *GetResourceCache() { return &resourceCache; }

	dBActionScript *GetAction(const char *pName);
	dBActionMetric *GetMetric(const char *pName);

protected:
	dBTheme *pTheme;

	MFIni *pThemeScreen;
	const char *pScreenName;

	MFVector clearColour;

	dBResourceCache resourceCache;
	dBEntityManager entityPool;
	dBActionManager actionManager;

	MFArray<dBThemeAction> actions;
	MFArray<dBThemeMetric> metrics;
};

#endif
