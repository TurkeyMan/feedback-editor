#if !defined(_VAGFILE_H)
#define _VAGFILE_H

#include "MFSound.h"

struct TrackInfo
{
	int sampleRate;
	int numBlocks;
};

struct VGSFile
{
	union
	{
		char padd[128];

		struct
		{
			uint32 VgS;
			char version;
			TrackInfo tracks[6];
		};
	};
};

void LoadVGSFile(GHSong *pSong, const char *pFilename);

#endif
