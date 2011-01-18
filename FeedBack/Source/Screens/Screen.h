#if !defined(_SCREEN_H)
#define _SCREEN_H

class dBScreen
{
public:
	dBScreen();
	virtual ~dBScreen();

	virtual void Select() = 0;
	virtual int Update() = 0;
	virtual void Draw() = 0;
	virtual void Deselect() = 0;

	static int UpdateScreen();
	static void DrawScreen();
	static void SetNext(dBScreen *pNext);
	static dBScreen *GetCurrent() { return pCurrent; }

	static dBScreen *pCurrent;
	static dBScreen *pNext;
};

#endif
