#if !defined(_MENUSCREEN_H)
#define _MENUSCREEN_H

class MenuScreen : public GHScreen
{
public:
	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

	virtual void Activate();
	static void ListCallback(int cancel, int listID, const char *pString, void *pData);
	static void NewCallback(int cancel, const char *pFilename, const char *pPath, void *pData);
	static void LoadCallback(int cancel, const char *pFilename, const char *pPath, void *pData);
	static void ShowChartSettings(bool returnToMenu);
	static void SettingsCallback(int cancel, int listID, const char *pString, void *pData);
	static void NameCallback(int cancel, const char *pString);
	static void ArtistCallback(int cancel, const char *pString);
	static void CharterCallback(int cancel, const char *pString);
	static void Player2Callback(int cancel, int listID, const char *pString, void *pData);
	static void SetMusicCallback(int cancel, const char *pFilename, const char *pPath, void *pData);
	static void SetFretboardCallback(int cancel, const char *pFilename, const char *pPath, void *pData);
	static void ConfigCallback(int cancel, int listID, const char *pString, void *pData);
	static void DifficultyCallback(int cancel, int listID, const char *pString, void *pData);
	static void GenreCallback(int cancel, int listID, const char *pString, void *pData);
	static void MediaCallback(int cancel, int listID, const char *pString, void *pData);
	static void URLCallback(int cancel, const char *pString);
	static void PreviewCallback(int cancel, const char *pString);
	static void SetFretboard(int cancel, const char *pFilename, const char *pPath, void *pData);
};

#endif
