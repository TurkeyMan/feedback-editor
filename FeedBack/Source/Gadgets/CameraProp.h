#if !defined(_CAMERAPROP_H)
#define _CAMERAPROP_H

class CameraProp : public dBProp
{
public:
	static CameraProp *Create(const char *pName);
	virtual void Destroy();

	virtual void Update();
	virtual void Draw();

};

#endif
