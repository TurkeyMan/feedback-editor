#include "FeedBack.h"
#include "Fuji/MFFileSystem.h"

#include "EventSuggestions.h"

EventSuggestions::EventSuggestions()
{
	pEventSuggestions = NULL;
	numEventSuggestions = 0;
}

EventSuggestions::~EventSuggestions()
{
}

void EventSuggestions::LoadEventSuggestions(const char *pEventsFile)
{
	ReleaseEventSuggestions();

	size_t size;
	char *pEvents = MFFileSystem_Load(pEventsFile, &size);
	if(pEvents)
	{
		char *pE = pEvents;
		while(pE - pEvents < (int)size)
		{
			char *pEE = pE;

			while(pEE - pEvents < (int)size && *pEE && !MFIsNewline(*pEE))
			{
				++pEE;
			}

			*pEE = 0;

			int stringLen = (int)(pEE - pE);

			if(stringLen)
			{
				EventSuggestion *pSuggestion = (EventSuggestion*)MFHeap_Alloc(sizeof(EventSuggestion));
				MFString_CopyN(pSuggestion->string, pE, stringLen);
				pSuggestion->string[stringLen] = 0;
				pSuggestion->pNext = pEventSuggestions;
				pEventSuggestions = pSuggestion;
			}

			pE = pEE;

			while(pE - pEvents < (int)size && (MFIsNewline(*pE) || !*pE))
				++pE;
		}
	}
}

void EventSuggestions::ReleaseEventSuggestions()
{
	if(pEventSuggestions)
	{
		// free them first
		while(pEventSuggestions)
		{
			EventSuggestion *pNext = pEventSuggestions->pNext;
			MFHeap_Free(pEventSuggestions);
			pEventSuggestions = pNext;
		}

		numEventSuggestions = 0;
	}
}

int EventSuggestions::SortFunc(const void *p1, const void *p2)
{
	const EventSuggestion **ppE1 = (const EventSuggestion **)p1;
	const EventSuggestion **ppE2 = (const EventSuggestion **)p2;
	return MFString_Compare((*ppE1)->string, (*ppE2)->string);
}

int EventSuggestions::CollectAndSortEventSuggestions(const char *pString)
{
	numEventSuggestions = 0;

	size_t len = MFString_Length(pString);

	EventSuggestion *pES = pEventSuggestions;
	while(pES && numEventSuggestions < MAX_SUGGESTIONS)
	{
		if(!MFString_CompareN(pES->string, pString, len))
		{
			pSuggestionList[numEventSuggestions] = pES;
			++numEventSuggestions;
		}

		pES = pES->pNext;
	}

	if(numEventSuggestions)
		qsort(pSuggestionList, numEventSuggestions, sizeof(EventSuggestion*), SortFunc);

	return numEventSuggestions;
}

const char *EventSuggestions::GetSuggestion(int suggestion)
{
	return suggestion < numEventSuggestions ? pSuggestionList[suggestion]->string : NULL;
}
