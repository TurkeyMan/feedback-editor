#if !defined(_FINGEREDITSCREEN_H)
#define _FINGEREDITSCREEN_H

class FingerEditScreen : public GHScreen
{
public:
	FingerEditScreen();
	virtual ~FingerEditScreen();

	void Show();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();
};

#endif
