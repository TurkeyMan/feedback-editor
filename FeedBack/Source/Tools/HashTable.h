#if !defined(_DB_HASHTABLE_H)
#define _DB_HASHTABLE_H

#include "MFHeap.h"
#include "MFObjectPool.h"

template <typename T>
class dBHashList
{
private:
	struct dBHashItem
	{
		const char *pName;
		T *pItem;
		dBHashItem *pNext;
	};

public:
	void Create(int _tableSize, int maxItems, int growItems = 0)
	{
		itemPool.Init(sizeof(dBHashItem), maxItems, growItems);
		ppItems = (dBHashItem**)MFHeap_AllocAndZero(sizeof(dBHashItem*)*_tableSize);
		tableSize = _tableSize;
		itemCount = 0;
	}

	void Destroy()
	{
		itemPool.Deinit();
		MFHeap_Free(ppItems);
		tableSize = 0;
		itemCount = 0;
	}

	void Add(T* pItem, const char *pItemName)
	{
		dBHashItem *pNew = (dBHashItem*)itemPool.Alloc();
		pNew->pItem = pItem;
		pNew->pName = pItemName;

		uint32 hash = MFUtil_HashString(pItemName) % tableSize;

		pNew->pNext = ppItems[hash];
		ppItems[hash] = pNew;

		++itemCount;
	}

	void Remove(const char *pName)
	{
		uint32 hash = MFUtil_HashString(pName) % tableSize;

		dBHashItem *pTemp = ppItems[hash];
		dBHashItem *pDel = NULL;
		if(pTemp->pItem == pItem)
		{
			pDel = pTemp;
			ppItems[hash] = pTemp->pNext;
		}
		else
		{
			while(pTemp->pNext && pTemp->pNext->pItem != pItem)
				pTemp = pTemp->pNext;
			if(pTemp->pNext)
			{
				pDel = pTemp->pNext;
				pTemp->pNext = pTemp->pNext->pNext;
			}
		}

		if(pDel)
		{
			itemPool.Free(pDel);
			--itemCount;
		}
	}

	void Remove(T* pItem)
	{
		dBHashItem *pDel = NULL;
		for(int a=0; a<tableSize; ++a)
		{
			dBHashItem *pTemp = ppItems[a];
			if(pTemp->pItem == pItem)
			{
				pDel = pTemp;
				ppItems[a] = pTemp->pNext;
				break;
			}
			else
			{
				while(pTemp->pNext && pTemp->pNext->pItem != pItem)
					pTemp = pTemp->pNext;
				if(pTemp->pNext)
				{
					pDel = pTemp->pNext;
					pTemp->pNext = pTemp->pNext->pNext;
					break;
				}
			}
		}

		if(pDel)
		{
			itemPool.Free(pDel);
			--itemCount;
		}
	}

	T* Find(const char *pItemName) const
	{
		uint32 hash = MFUtil_HashString(pItemName) % tableSize;
		dBHashItem *pItem = ppItems[hash];

		while(pItem && MFString_CaseCmp(pItem->pName, pItemName))
			pItem = pItem->pNext;

		return pItem ? pItem->pItem : NULL;
	}

	T* First() const
	{
		if(!itemCount)
			return NULL;

		for(int a=0; a<tableSize; ++a)
		{
			if(ppItems[a])
				return ppItems[a]->pItem;
		}
		return NULL;
	}

	T* Next(T *pItem) const
	{
		if(!pItem)
			return First();

		uint32 hash = MFUtil_HashString(pItem->GetName()) % tableSize;
		dBHashItem *pTemp = ppItems[hash];

		while(pTemp && pTemp->pItem != pItem)
			pTemp = pTemp->pNext;

		if(!pTemp)
			return NULL;
		else if(pTemp->pNext)
			return pTemp->pNext->pItem;
		else
		{
			for(int a=hash+1; a<tableSize; ++a)
			{
				if(ppItems[a])
					return ppItems[a]->pItem;
			}
		}
		return NULL;
	}

protected:
	MFObjectPool itemPool;
	dBHashItem **ppItems;
	int tableSize;
	int itemCount;
};

#endif
