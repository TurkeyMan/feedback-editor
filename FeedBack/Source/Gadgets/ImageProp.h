#if !defined(_IMAGEPROP_H)
#define _IMAGEPROP_H

#include "Entity.h"
#include "Fuji/MFMaterial.h"
#include "Fuji/MFFont.h"

class dBImageProp : public dBEntity
{
public:
	static void RegisterEntity();
	static void *Create() { return new dBImageProp; }

	dBImageProp();
	virtual ~dBImageProp();

	virtual void Update();
	virtual void Draw();

	virtual const char *GetProperty(const char *pPropertyName);

protected:
	static void SetImage(dBEntity *pEntity, dBRuntimeArgs *pArguments);

	MFMaterial *pImage;
	MFRect imageRect;
};

#endif
