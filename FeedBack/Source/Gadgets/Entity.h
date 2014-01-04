#if !defined(_ENTITY_H)
#define _ENTITY_H

#include "Fuji/MFIni.h"
#include "Fuji/MFPtrList.h"
#include "../Tools/Factory.h"
#include "../Tools/HashTable.h"
#include "Action.h"

class dBEntityManager;

struct dBEvent
{
	const char *pEvent;
	dBActionScript *pScript;
};

class dBEntity
{
	friend class dBEntityManager;
public:
	static void RegisterEntity();

	dBEntity();
	virtual ~dBEntity();

	virtual void Init(dBActionManager *pActionManager, MFIniLine *pEntityData);

	virtual void Update();
	virtual void Draw();

	dBFactoryType *GetType() { return pType; }
	bool IsType(dBFactoryType *pType);

	const char *GetName() { return name; }

	virtual const char *GetProperty(const char *pPropertyName);

	void SignalEvent(const char *pEvent, const char *pParams);
	MFMatrix GetMatrix();

	dBEntityManager *GetManager() { return pManager; }

protected:
	dBFactoryType *pType;

	char name[56]; // 64 - vtable - pType = 56
	MFVector startPos, startRot, startScale, startColour;
	MFVector pos, rot, scale, colour;

	bool bEnabled, bVisible;

	dBEntityManager *pManager;
	dBActionManager *pActionManager;

	MFArray<dBEvent> events;

	// script functions
	static void SetPos(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetRot(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetSize(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetColour(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetEnable(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetVisible(dBEntity *pEntity, dBRuntimeArgs *pArguments);
};

class dBEntityManager
{
public:
	static void InitManager();
	static void DeinitManager();
	static dBFactoryType *RegisterEntityType(const char *pEntityTypeName, dBFactory_CreateFunc *pCreateFunc, const char *pParentType = NULL);

	void Init(dBThemeScreen *pThemeScreen);
	void Deinit();

	dBEntity *Create(const char *pType, const char *pName);
	void Destroy(dBEntity *pEntity);

	dBEntity *Find(const char *pName);
	dBEntity *Iterate(dBEntity *pLast);

	dBThemeScreen *GetScreen() { return pThemeScreen; }

private:
	dBHashList<dBEntity> entityPool;

	dBThemeScreen *pThemeScreen;

	static dBFactory<dBEntity> entityFactory;
};

#endif
