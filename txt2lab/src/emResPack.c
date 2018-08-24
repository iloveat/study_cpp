
#include "emPCH.h"

#include "emResPack.h"

struct tagResPack g_ResPack;


#if EM_RES_CACHE && DEBUG_LOG_SWITCH_CACHE
emStatic FILE			*g_fResCacheLog;		//资源包缓存日志
emStatic emSize			g_nResCacheByteRead;	//资源包缓存实际读取字节数
emStatic emSize			g_nResCacheByteUse;		//资源包缓存内部读取字节数
emStatic emSize			g_nResCachePageRead;	//资源包缓存实际读取页数
emStatic emSize			g_nResCachePageUse;		//资源包缓存内部读取页数

void emCall emResCacheLog_Begin()
{
	g_fResCacheLog = fopen("log/日志_资源包.log", "a");
}

void emCall emResCacheLog_End()
{
	fprintf(g_fResCacheLog, "资源包数据：  需读取%u字节，       实际读取%u字节（合%u块,每块%u字节）。\n", g_nResCacheByteUse, g_nResCacheByteRead,g_nResCachePageRead,g_nResCacheByteRead/g_nResCachePageRead);
	fprintf(g_fResCacheLog, "资源包数据：  需读取%u次（块），   实际读取%u块。\n\n\n", g_nResCachePageUse, g_nResCachePageRead);
	fclose(g_fResCacheLog);
}
#endif 


void emCall emResPack_Initialize(emPResPack pResPack, emPResPackDesc pResPackDesc)
{

	//LOG_StackAddr(__FUNCTION__);

	pResPack->m_pResParam = pResPackDesc->pCBParam;
	pResPack->m_nSize = pResPackDesc->nSize;
	pResPack->m_cbReadRes = pResPackDesc->pfnRead;

#if EM_RES_CACHE
	pResPack->m_pIndex = emNull;
	if (pResPackDesc->pCacheBlockIndex)
	{
		emUInt32 i;

		for (i = 0; i < pResPackDesc->nCacheBlockCount + pResPackDesc->nCacheBlockExt; ++ i)
			pResPackDesc->pCacheBlockIndex[i] = (emUInt8)~0;

		pResPack->m_pIndex = pResPackDesc->pCacheBlockIndex;
		pResPack->m_pBuffer = pResPackDesc->pCacheBuffer;
		pResPack->m_nSegSize = pResPackDesc->nCacheBlockSize;

		pResPack->m_nSizeShift = 0;
		pResPack->m_nSizeMask = (emSize)~0;

		for (i = pResPackDesc->nCacheBlockSize; i != 1; i >>= 1)
		{
			++ pResPack->m_nSizeShift;
			pResPack->m_nSizeMask <<= 1;
		}

		pResPack->m_nSizeMask = ~pResPack->m_nSizeMask;

		pResPack->m_nCountShift = pResPack->m_nSizeShift;
		pResPack->m_nCountMask = (emSize)~0;
		
		for (i = pResPackDesc->nCacheBlockCount; i != 1; i >>= 1)
		{
			++ pResPack->m_nCountShift;
			pResPack->m_nCountMask <<= 1;
		}
		
		pResPack->m_nCountMask = ~pResPack->m_nCountMask;

		pResPack->m_nTotalBlocks = (emSize)(pResPackDesc->nCacheBlockCount + pResPackDesc->nCacheBlockExt);
		//pResPack->m_bMapped = emFalse;
	}
#endif
}


#if EM_RES_CACHE
emBool emCall emResPack_IsCacheValid(emPResPackDesc pResPackDesc)
{
	LOG_StackAddr(__FUNCTION__);

	if (pResPackDesc->pCacheBlockIndex || pResPackDesc->pCacheBuffer)
	{
		/* 注意保证大小和个数是2的幂 */
		if (emNull == pResPackDesc->pCacheBlockIndex ||
			emNull == pResPackDesc->pCacheBuffer ||
			0 == pResPackDesc->nCacheBlockSize ||
			0 == pResPackDesc->nCacheBlockCount ||
			(pResPackDesc->nCacheBlockSize & -(emInt32)pResPackDesc->nCacheBlockSize) != pResPackDesc->nCacheBlockSize ||
			(pResPackDesc->nCacheBlockCount & -(emInt32)pResPackDesc->nCacheBlockCount) != pResPackDesc->nCacheBlockCount)
		{
			return emFalse;
		}
	}
	return emTrue;
}


emStatic emPCByte emCall emResPack_Cache(emPResPack pResPack, emUInt8 iPage, emUInt16 iSegment, emUInt16 nSegment)
{
	emResAddress iPos;
	emPByte pRes, pRet;

	LOG_StackAddr(__FUNCTION__);

	iPos = ((emResAddress)iSegment << pResPack->m_nSizeShift);
	pRes = pResPack->m_pBuffer + iPos;
	pRet = pRes;
	iPos += ((emResAddress)iPage << pResPack->m_nCountShift);

	for ( ; ; )
	{
		if (pResPack->m_pIndex[iSegment] != iPage)
		{

			pResPack->m_cbReadRes(pResPack->m_pResParam, pRes, iPos, pResPack->m_nSegSize);
			pResPack->m_pIndex[iSegment] = iPage;


#if DEBUG_LOG_SWITCH_CACHE
			g_nResCacheByteRead += pResPack->m_nSegSize;
			++ g_nResCachePageRead;
#endif
		}

		if (0 == --nSegment)
			break;

		++iSegment;

		iPos += pResPack->m_nSegSize;
		pRes += pResPack->m_nSegSize;
	}
	return pRet;
}


//映射方式：直接相联映射
//变量描述：假定：Cache块共16块，块大小512字节；    资源：120页，每页16块，每块512字节
//
//			iPos				：资源内总偏移
//			iOffset				：块内偏移
//			iSegment			：Cache块号
//			nSegment			：Cache中跨度块数目
//			iPage				：资源页号
//			nSize				：需读取的字节数
//			pResPack->m_pIndex[]：当前Cache块中存储的资源页号
void emCall emResPack_Read(emPResPack pResPack, emPointer pBuffer, emResAddress iPos, emSize nSize)
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pResPack && pResPack->m_cbReadRes);

#if EM_RES_CACHE
	if (pResPack->m_pIndex )
	{
		emResAddress iOffset;
		emUInt16 iSegment;
		emUInt16 nSegment;

#if DEBUG_LOG_SWITCH_CACHE
		g_nResCacheByteUse += nSize;
		++ g_nResCachePageUse;
#endif
		iOffset = (emResAddress)(iPos & pResPack->m_nSizeMask);
		iSegment = (emUInt16)((iPos >> pResPack->m_nSizeShift) & pResPack->m_nCountMask);
		nSegment = (emUInt16)((iOffset + nSize + pResPack->m_nSizeMask) >> pResPack->m_nSizeShift);

		if ((emSize)(iSegment + nSegment) <= pResPack->m_nTotalBlocks)
		{
			emUInt8 iPage;
			iPage = (emUInt8)(iPos >> pResPack->m_nCountShift);
			emMemCpy(pBuffer, emResPack_Cache(pResPack, iPage, iSegment, nSegment) + iOffset, nSize);
			return;
		}

#if DEBUG_LOG_SWITCH_CACHE
		fprintf(g_fResCacheLog, "未读入Cache块的数据字节数 = %u\n", nSize);
#endif


	}
#endif
	pResPack->m_cbReadRes(pResPack->m_pResParam, pBuffer, iPos, nSize);
}

#endif 


void emCall emRes_Read(emPResPack pResPack, emPointer pBuffer, emResAddress iPos, emSize nSize)
{
	FILE* pFile = (FILE*)pResPack->m_pResParam;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pResPack && pResPack->m_cbReadRes);

	fFrontSeek(pFile, iPos, 0);
	fFrontRead(pBuffer, 1, nSize, pFile);
}

