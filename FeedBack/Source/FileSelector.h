#if !defined(_FILESELECTORSCREEN_H)
#define _FILESELECTORSCREEN_H

#include "Gadgets/Frame.h"

typedef void (*FileCallback)(int, const char *, const char *, void *);

struct FileListItem
{
	char filename[256];
	bool directory;
};

class FileSelectorScreen : public GHScreen
{
public:
	enum ShowFlags
	{
		SF_ShowNone = MFBIT(0),
		SF_NoFolders = MFBIT(1)
	};

	FileSelectorScreen();
	virtual ~FileSelectorScreen();

	void Show(const char *pMessage, const char *pPath, const char *pFileTypes, FileCallback pCallback, uint32 flags, void *pUserData = NULL, float width = 400.0f, float height = 150.0f);
	void Destroy();

	virtual void Update();
	virtual void UpdateInput();
	virtual void Draw();

protected:
	void ScanDirectory();

	char message[1024];
	char path[256];
	char extensions[256];

	dBFrame frame;

	MFArray<FileListItem> items;

	FileCallback pCompleteCallback;
	int directoryDepth;

	int selected;
	int listOffset;
	int pageMax;
	char typeBuffer[256];
	float typeTimeout;

	void *pUserData;
	uint32 flags;

	float width, height;
};

#endif
