#if !defined(_MIXER_H)
#define _MIXER_H

class MixerControl
{
public:
	void Update();
	void Draw(float x, float y, float w, float *pHeight);
	float GetHeight();

	void Set(const char *pName, int type, float *pLevel);

	char name[64];
	int type;
	float height;
	float *pLevel;
};

class Mixer : public GHScreen
{
public:
	Mixer();
	~Mixer();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	int mixerTrack;
	int numControls;

	MixerControl *pControls;
};

#endif
