#if !defined(_TEXTPROP_H)
#define _TEXTPROP_H

#include "Entity.h"
#include "MFFont.h"

class dBTextProp : public dBEntity
{
public:
	static void RegisterEntity();
	static void *Create() { return new dBTextProp; }

	dBTextProp();
	virtual ~dBTextProp();

	virtual void Update();
	virtual void Draw();

	virtual const char *GetProperty(const char *pPropertyName);

protected:
	static void SetFont(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetText(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetTextHeight(dBEntity *pEntity, dBRuntimeArgs *pArguments);
	static void SetJustification(dBEntity *pEntity, dBRuntimeArgs *pArguments);

	MFFont *pFont;
	MFString text;
	float textHeight;
	MFFontJustify justification;
	MFRect boundingRect;
};

#endif
