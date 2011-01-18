#if !defined(_NOTE_SCORE_KEEPER_H)
#define _NOTE_SCORE_KEEPER_H

class dBNoteScoreKeeper
{
public:
	dBNoteScoreKeeper();
	~dBNoteScoreKeeper();

	void Begin(dBChart *pChart, GHStreams stream, int player, int64 startTime = 0);
	void Update(int64 delta);

	int GetCombo();
	int GetScore();
	int GetMultiplier() { return (MFMin(combo / 10, 3) + 1) * (starBegin >= 0 ? 2 : 1); }

protected:
	int64 playTime;

	dBChart *pChart;
	GHStreams stream;
	GHEventManager *pStream;

	int combo;
	int score;
	float starCharge;
	int starBegin;

	int player;
	int noteHold[10];
	GHEvent *pNextEvent;
};

#endif
