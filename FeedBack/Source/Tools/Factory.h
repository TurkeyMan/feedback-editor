#if !defined(_FACTORY_H)
#define _FACTORY_H

typedef void *(dBFactory_CreateFunc)();

struct dBFactoryType
{
	char typeName[64];
	dBFactory_CreateFunc *pCreateFunc;
	dBFactoryType *pParent;
	dBFactoryType *pNext;
};

template <typename T>
class dBFactory
{
public:
	dBFactory() { pTypes = NULL; }

	dBFactoryType *RegisterType(const char *pTypeName, dBFactory_CreateFunc *pCreateCallback, dBFactoryType *pParent = NULL)
	{
		dBFactoryType *pType = new dBFactoryType;
		MFString_Copy(pType->typeName, pTypeName);
		pType->pCreateFunc = pCreateCallback;
		pType->pParent = pParent;
		pType->pNext = pTypes;
		pTypes = pType;
		return pType;
	}

	T *Create(const char *pTypeName, dBFactoryType **ppType = NULL)
	{
		for(dBFactoryType *pType = pTypes; pType; pType = pType->pNext)
		{
			if(!MFString_CaseCmp(pTypeName, pType->typeName))
			{
				if(ppType)
					*ppType = pType;
				return (T*)pType->pCreateFunc();
			}
		}

		MFDebug_Warn(2, MFStr("Unknown factory type: '%s'", pTypeName));

		return NULL;
	}

	dBFactoryType *FindType(const char *pTypeName)
	{
		for(dBFactoryType *pType = pTypes; pType; pType = pType->pNext)
		{
			if(!MFString_CaseCmp(pTypeName, pType->typeName))
				return pType;
		}
		return NULL;
	}

protected:
	dBFactoryType *pTypes;
};

#endif
