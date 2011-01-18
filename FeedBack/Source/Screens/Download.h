#if !defined(_DOWNLOADSCREEN_H)
#define _DOWNLOADSCREEN_H

class DownloadScreen : public dBScreen
{
public:
	DownloadScreen();
	virtual ~DownloadScreen();

	virtual void Select();
	virtual int Update();
	virtual void Draw();
	virtual void Deselect();

	static int NetworkJobThread(void *pData);

private:
	struct CHSong
	{
		const char *pSong;
		const char *pSongTitle;
		MFMaterial *pImage;
		// music stream?
	};

	struct CHGenre
	{
		const char *pGenre;

		CHSong *pSongs;
		int numSongs;
	};

	MFThread thread;

	int currentGenre;

	bool bWaitingForNetwork;

	CHGenre *pGenres;
	int numGenres;
};

#endif
