#ifndef SWF_LIST_H
#define SWF_LIST_H

#ifdef MIPS_VERSION
#define INLINE static inline
#else
#define INLINE static
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

/************************************************************************/
/* Bidirection Loop link                                                */
/************************************************************************/

typedef struct _BLIST {
	struct _BLIST *pNext; 
	struct _BLIST *pPrev;
}BLIST, *PBLIST;

/*
* SDLIST_INIT , circular list
*/
#define BLIST_INIT(pList)\
	{(pList)->pPrev = pList; (pList)->pNext = pList;}
#define BLIST_INIT_DECLARE(List)\
	BLIST List =   {&List, &List}

#define BLIST_IS_EMPTY(pList) (((pList)->pPrev == (pList)) && ((pList)->pNext == (pList)))
#define BLIST_GET_ITEM_AT_HEAD(pList) (pList)->pNext
#define BLIST_GET_ITEM_AT_TAIL(pList) (pList)->pPrev

/*
* SDITERATE_OVER_LIST pStart is the list, pTemp is a temp list member
*/
#define ITERATE_OVER_LIST(pStart, pTemp) \
	for((pTemp) =(pStart)->pNext; pTemp != (pStart); (pTemp) = (pTemp)->pNext)

/* get structure from contained field */
#define CONTAINING_STRUCT(address, struct_type, field_name)\
	((struct_type *)((unsigned char*)(address) - (unsigned char*)(&((struct_type *)0)->field_name)))

INLINE PBLIST BListInsertTail(PBLIST pList, PBLIST pAdd) {
	/* insert at tail */
    pAdd->pPrev = pList->pPrev;
    pAdd->pNext = pList;
    pList->pPrev->pNext = pAdd;
    pList->pPrev = pAdd;
    return pAdd;
}

INLINE PBLIST BListInsertHead(PBLIST pList, PBLIST pAdd) {
	/* insert at head */
    pAdd->pPrev = pList;
    pAdd->pNext = pList->pNext;
    pList->pNext->pPrev = pAdd;
    pList->pNext = pAdd;
    return pAdd;
}

INLINE void BListJoinHead(PBLIST dst, PBLIST src) {
	PBLIST pItem;
	if(!BLIST_IS_EMPTY(src))
	{
		pItem = dst->pNext;
		dst->pNext = src->pNext;
		src->pNext->pPrev = dst;
		src->pPrev->pNext = pItem;
		pItem->pPrev = src->pPrev;
		src->pPrev = src->pNext = src;
	}
}

#define BListAdd(pList,pItem) BListInsertTail((pList),(pItem))

INLINE PBLIST BListRemove(PBLIST pDel) {
    pDel->pNext->pPrev = pDel->pPrev;
    pDel->pPrev->pNext = pDel->pNext;
	/* point back to itself just to be safe, incase remove is called again */
    pDel->pNext = pDel;
    pDel->pPrev = pDel;
    return pDel;
}

INLINE PBLIST BListRemoveItemFromHead(PBLIST pList) {
    PBLIST pItem = NULL;
    if (pList->pNext != pList) {
        pItem = pList->pNext;
		/* remove the first item from head */
        BListRemove(pItem);
    }
    return pItem;
}

INLINE void BListSplite(PBLIST pList, PBLIST pSplite) {
	if(!BLIST_IS_EMPTY(pList))
	{
		*pSplite = *pList;
		pList->pPrev->pNext = pSplite;
		pList->pNext->pPrev = pSplite;
		BLIST_INIT(pList);
	}
	else
	{
		BLIST_INIT(pSplite);
	}
}

/************************************************************************/
/* Single link                                                          */
/************************************************************************/

typedef struct _SLIST
{
	struct _SLIST * pNext;
} SLIST, *PSLIST;

typedef struct _SHEAD
{
	PSLIST pNext;
	PSLIST pTail;
} SHEAD, *PSHEAD;

#define SHEAD_INIT(pHead)\
	{(pHead)->pNext = NULL;(pHead)->pTail = NULL;}

#define ITERATE_OVER_SLIST(pStart, pTemp) \
	for((pTemp) =(pStart)->pNext; pTemp != NULL; (pTemp) = (pTemp)->pNext)

INLINE PSLIST SListInsertHead(PSHEAD pHead, PSLIST pAdd) {
	pAdd->pNext = pHead->pNext;
	pHead->pNext = pAdd;
	if(pHead->pTail == NULL)
	{
		pHead->pTail = pAdd;
	}
    return pAdd;
}

INLINE PSLIST SListInsertTail(PSHEAD pHead, PSLIST pAdd) {
	if(pHead->pTail == NULL)
	{
		pHead->pNext = pAdd;
	}
	else
	{
		pHead->pTail->pNext = pAdd;
	}
	pHead->pTail = pAdd;
	pAdd->pNext = NULL;
    return pAdd;
}

#define SListAdd(pHead,pItem) SListInsertTail((pHead),(pItem))

INLINE PSLIST SListRemove(PSHEAD pHead, PSLIST pDel) {
	PSLIST pItem = NULL;
	if(pHead->pNext == pDel)
	{
		//SHEAD_INIT(pHead);
		pHead->pNext = pDel->pNext;
		if(pHead->pTail == pDel)
		{
			pHead->pTail = NULL;
		}
	}
	else
	{
		ITERATE_OVER_SLIST(pHead, pItem)
		{
			if(pItem->pNext == pDel)
			{
				pItem->pNext = pDel->pNext;
				if(pHead->pTail == pDel)
				{
					pHead->pTail = pItem;
				}
				break;
			}
		}
	}
    return pDel;
}

INLINE PSLIST SListRemoveItemFromHead(PSHEAD pHead) {
    PSLIST pItem = NULL;
	if(pHead->pNext != NULL)
	{
		pItem = pHead->pNext;
		pHead->pNext = pItem->pNext;
		if(pItem == pHead->pTail)
		{
			pHead->pTail = NULL;
		}
	}
	return pItem;
}

#endif
