#if !defined(_ACTION_H)
#define _ACTION_H

#include "Fuji/MFIni.h"
#include "Fuji/MFPtrList.h"
#include "Fuji/MFObjectPool.h"
#include "../Tools/Factory.h"
#include "../Tools/HashTable.h"

class dBThemeScreen;

class dBEntity;
struct dBActionScript_Token;
struct dBActionScript_Action;
class dBRuntimeArgs;

class dBRuntimeArgs
{
private:
	enum ArgType
	{
		AT_Allocated = -1,
		AT_Unallocated = 0,

		AT_String,
		AT_Int,
		AT_Float,
		AT_Array
	};

	struct Argument
	{
		ArgType type;
		union
		{
			int iValue;
			float fValue;
			dBRuntimeArgs *pArray;
		};
		MFString string; // this should go in the union... :/
	};

public:
	static void Init();
	static void Deinit();

	static dBRuntimeArgs *Allocate(int numArgs);
	int Extend(dBRuntimeArgs *pValue);
	void Release(int startArg = 0);

	inline int GetNumArgs() { return numArgs; }

	bool IsNumeric(int index = -1);

	Argument *Get(int index);
	MFString GetString(int index);
	bool GetBool(int index);
	int GetInt(int index);
	float GetFloat(int index);
	MFVector GetVector(int index);
	dBRuntimeArgs *GetArray(int index);

	void Set(int index, Argument *pArg);
	void SetValue(int index, MFString str, bool bString);
	void SetArray(int index, dBRuntimeArgs *pArray);
	void NegateValue(int index = -1);

protected:
	Argument *pArgs;
	int numArgs;

	inline static Argument *FindStart(Argument *pStart);
	inline static Argument *AllocArgs(int numArgs);
	static MFObjectPool runtimeArgPool;
	static Argument *pArgPool;
	static Argument *pCurrent;
	static int poolSize;
};

class dBAction
{
	friend class dBActionManager;
public:
	virtual void Init(dBRuntimeArgs *pParameters) = 0;
	virtual bool Update() = 0;

protected:
	dBEntity *pEntity;
};


struct dBActionScript
{
	dBActionScript_Action *pAction;
	dBActionScript_Token *pTokens;
};

struct  dBActionMetric
{
	dBActionScript_Token *pTokens;
	int numTokens;
};

struct dBActionScript_Token
{
	enum TokenType
	{
		TT_Unknown = 0,

		TT_Identifier,
		TT_Number,
		TT_String,
		TT_Arithmetic,
		TT_Syntax
	};

	inline bool IsString(const char *pString) { return !MFString_CaseCmp(pToken, pString); }
	inline bool IsSyntax(const char *pString = NULL) { return type == TT_Syntax && (!pString || !MFString_CaseCmp(pToken, pString)); }
	inline bool IsOperator(const char *pString = NULL) { return type == TT_Arithmetic && (!pString || !MFString_CaseCmp(pToken, pString)); }
	inline bool IsIdentifier() { return type == TT_Identifier; }
	inline bool IsNumeric() { return type == TT_Number || (type == TT_String && MFString_IsNumber(pToken)); }
	inline bool IsValue() { return type == TT_Identifier || type == TT_Number || type == TT_String; }

	const char *pToken;
	TokenType type;
};

struct dBActionScript_Action
{
	int ParseAction(dBActionScript_Token *pToken, int numTokens);

	const char *pEntity;
	const char *pAction;
	dBActionScript_Token *pArgs;
	dBActionScript_Action *pNext;
	int numArgs;
	bool bWaitComplete;
};

class dBActionManager
{
public:
	typedef void (InstantActionHandler)(dBEntity *pEntity, dBRuntimeArgs *pArguments);

	static void RegisterInstantAction(const char *pActionName, InstantActionHandler *pActionHandler, dBFactoryType *pEntityType);
	static void RegisterDeferredAction(const char *pActionName, dBFactory_CreateFunc *pCreateFunc, dBFactoryType *pEntityType);

	static void InitManager();
	static void DeinitManager();

	// instance methods
	void Init(dBThemeScreen *pThemeScreen);
	void Deinit();

	void Update();

	dBActionScript* ParseScript(const char *pScript);
	dBActionMetric* ParseMetric(const char *pMetric);

	void RunScript(dBActionScript *pScript, dBEntity *pEntity);

	bool RunAction(const char *pAction, dBRuntimeArgs *pArgs, dBEntity *pActionEntity, dBActionScript_Action *pNextAction, dBEntity *pNextEntity);

	dBThemeScreen *GetScreen() { return pThemeScreen; }

protected:
	// internal structures
	struct ActiveAction
	{
		dBAction *pAction;
		dBEntity *pEntity;
		dBActionScript_Action *pNext;
	};

	// private methods
	void Continue(dBActionScript_Action *pNextData, dBEntity *pEntity);

	void *Lex(const char *pAction, int *pNumTokens, int preBytes = 0);
	dBActionScript_Action *ParseActions(dBActionScript_Token *pTokens, int numTokens);

	dBRuntimeArgs *ResolveArguments(dBActionScript_Token *pTokens, int numTokens, int *pNumUsed = NULL);

	dBRuntimeArgs *ResolveIdentifier(const char *pIdentifier);
	dBRuntimeArgs *GetNextValue(dBActionScript_Token *&pT, int &remaining);
	dBRuntimeArgs *CalculateProducts(dBActionScript_Token *&pT, int &remaining);

	// private members
	MFPtrListDL<ActiveAction> activeDeferredActions;

	dBThemeScreen *pThemeScreen;

	////////////////////////////////////////////////////////////////////////////////////
	// static stuff
	struct ActionType
	{
		const char *pName;
		InstantActionHandler *pInstantActionHandler;
		dBFactoryType *pEntityType;
	};

	// static members
	static dBFactory<dBAction> actionFactory;

	static MFObjectPool actionTypePool;
	static dBHashList<ActionType> actionRegistry;
};

#endif
