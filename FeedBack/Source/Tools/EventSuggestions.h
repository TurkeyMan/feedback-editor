#if !defined(_EVENTSUGGESTIONS_H)
#define _EVENTSUGGESTIONS_H

#define MAX_SUGGESTIONS 64

class EventSuggestions
{
public:
	EventSuggestions();
	~EventSuggestions();

	void LoadEventSuggestions(const char *pEventsFile);
	void ReleaseEventSuggestions();

	int CollectAndSortEventSuggestions(const char *pString);
	const char *GetSuggestion(int suggestion);

protected:
	static int SortFunc(const void *p1, const void *p2);

	struct EventSuggestion
	{
		char string[256];
		EventSuggestion *pNext;
	};

	EventSuggestion *pEventSuggestions;
	EventSuggestion *pSuggestionList[MAX_SUGGESTIONS];
	int numEventSuggestions;
};

#endif
