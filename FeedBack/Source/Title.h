#if !defined(_TITLESCREEN_H)
#define _TITLESCREEN_H

class TitleScreen : public GHScreen
{
public:
	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	virtual void Activate();
};

#endif
