#if !defined(_STRING_ENTRY_LOGIC)
#define _STRING_ENTRY_LOGIC

typedef void (*StringChangeCallback)(const char *pString, void *pUserData);

class OnScreenKeyboard
{
	OnScreenKeyboard();
	virtual ~OnScreenKeyboard() = 0;

	virtual void Show() = 0;
	virtual void Hide() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual const char *GetString() = 0;
};

class StringEntryLogic
{
public:
	StringEntryLogic();
	StringEntryLogic(int bufferSize);
	~StringEntryLogic() { Destroy(); }

	void Create(int bufferSize);
	void Destroy();

	void Update();
	void Draw() {}

	const char *GetString() { return pBuffer; }
	void SetString(const char *pString);

	int StringLength() { return stringLen; }

	void SetChangeCallback(StringChangeCallback _pChangeCallback, void *_pUserData) { pChangeCallback = _pChangeCallback; pUserData = _pUserData; }

	int GetCursorPos() { return cursorPos; }
	void GetSelection(int *pSelStart, int *pSelEnd) { if(pSelStart) *pSelStart = selectionStart; if(pSelEnd) *pSelEnd = selectionEnd; }

	static void SetRepeatParams(float repeatDelay, float repeatRate) { gRepeatDelay = repeatDelay; gRepeatRate = repeatRate; }
	static void RegisterOnScreenKeyboard(OnScreenKeyboard *pOnScreenKeyboard) { pOSK = pOnScreenKeyboard; }

private:
	void StringCopyOverlap(char *pDest, const char *pSrc);
	void ClearSelection();

	char *pBuffer;
	int bufferLen;
	int stringLen;
	int cursorPos;
	int selectionStart, selectionEnd;
	int holdKey;
	float repeatDelay;

	StringChangeCallback pChangeCallback;
	void *pUserData;

	static OnScreenKeyboard *pOSK;

	static float gRepeatDelay;
	static float gRepeatRate;
};

#endif
