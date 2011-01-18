#if !defined(_TRACK_H)
#define _TRACK_H

#include "MFParticleSystem.h"

class dBTrack
{
public:
	dBTrack();
	virtual ~dBTrack();

//	virtual void Update();
	virtual void Draw(float time, dBChart *pSong, int track) = 0;

	void SetScrollSpeed(float speed) { scrollSpeed = speed; }

	void SetViewPoint(int view) { viewPoint = view; }
	int GetViewPoint() { return viewPoint; }

protected:
	float scrollSpeed;
	int viewPoint;
};

class Fretboard : public dBTrack
{
public:
	Fretboard();
	virtual ~Fretboard();

	virtual void Draw(float time, dBChart *pSong, int track);

	void LoadFretboard(const char *pImage);

	void HitNote(int note);

protected:
	MFMaterial *pFretboard;

	MFMaterial *pEdge;
	MFMaterial *pBar;

	MFMaterial *pFrets;
	MFMaterial *pRing;
	MFMaterial *pColourRing[5];

	MFMaterial *pButtonMat[5];
	MFMaterial *pButtonRing[5];

	MFModel *pButton;

	MFParticleSystem *pParticles;
	MFParticleEmitter *pEmitter;

	static MFVector gColours[5];
};

#endif
