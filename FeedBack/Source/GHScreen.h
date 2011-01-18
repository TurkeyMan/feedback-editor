#if !defined(_GHSCREEN_H)
#define _GHSCREEN_H

class GHScreen
{
public:
	GHScreen() {}
	virtual ~GHScreen() {}

	void Push();
	static void Pop();

	static void UpdateScreens();
	static void DrawScreens();

	virtual void Activate() {};

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	static GHScreen *pStack;
	GHScreen *pNext;
};

#endif
