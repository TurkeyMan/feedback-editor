#if !defined(_FRAME_H)
#define _FRAME_H

class dBFrame
{
public:
	dBFrame();
	~dBFrame();

	void Draw();

	void SetFrame(MFRect *pRect);
	void SetPosition(const MFVector &pos);
	void SetSize(float width, float height);
	void SetBorderWidth(float borderWidth);

	void SetMaterial(const char *pMaterial);
	void SetCornerColours(const MFVector &topLeft, const MFVector &topRight, const MFVector &bottomLeft, const MFVector &bottomRight);

protected:
	MFRect rect;
	MFVector colours[4];
	MFMaterial *pMat;

	float borderWidth;
};

#endif
