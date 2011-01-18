#if !defined(_MODELPROP_H)
#define _MODELPROP_H

#include "Entity.h"
#include "MFModel.h"

class dBModelProp : public dBEntity
{
public:
	static void RegisterEntity();
	static void *Create() { return new dBModelProp; }

	dBModelProp();
	virtual ~dBModelProp();

	virtual void Update();
	virtual void Draw();

	virtual const char *GetProperty(const char *pPropertyName);

protected:
	static void SetModel(dBEntity *pEntity, dBRuntimeArgs *pArguments);

	MFModel *pModel;
};

#endif
