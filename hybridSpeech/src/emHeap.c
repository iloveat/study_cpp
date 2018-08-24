/* 堆，栈，文件直接读取 -- 操作源文件 */

#include "emPCH.h"

#include "emHeap.h"

emStatic emSize g_nSize;
emStatic emSize g_nMaxSize;
emStatic emUInt32	g_nMaxStackSize;
emStatic emUInt32	g_nPrevStackSize;
emUInt32	g_BaseStack ;


#if DEBUG_LOG_SWITCH_HEAP
	FILE *fp_HeapLog;
	FILE *fp_StackLog;
#endif

#define HEAP_HEADSIZE		emMemberOffset(emCHeap, m_pMemStart)
#define OBJHEAP_HEADSIZE	emMemberOffset(emCObjHeap, m_pFreeStart)



#if DEBUG_LOG_SWITCH_HEAP


//打印栈顶地址
void emCall LOG_StackAddr(emCharA *pText)
{
	emInt32  k; 
	emUInt32 nCurStackVal;
	emInt32  nDiff;

	if( g_nMaxStackSize >5950)			//调试用
		g_nMaxStackSize=g_nMaxStackSize;

	if( g_BaseStack == 0)
	{
			//栈内存   sqb 2017-5-27
//		__asm__
//		(
//			mov g_BaseStack, esp
//		)
		g_nMaxStackSize = 0;
		g_nPrevStackSize = 0;
		fprintf(fp_StackLog, (emCStrA)(" 栈：%08x    基准：%6d						          %s%s\n"), g_BaseStack, 0, pText,"();");
	}
	else
	{
//		__asm__
//		(
//			mov k, esp
//		)

		nCurStackVal = g_BaseStack-k;
		nDiff = nCurStackVal-g_nPrevStackSize;

		if( nDiff > 4000)
			nDiff = nDiff;

		if( nCurStackVal> g_nMaxStackSize)		//峰值打印
		{
			g_nMaxStackSize = nCurStackVal;
			fprintf(fp_StackLog, (emCStrA)(" 栈：%08x    peak：%6d      val：%6d		this：%6d		%s%s\n"), k, g_nMaxStackSize,nCurStackVal,nDiff, pText,"();");
		}
		else										//非峰值打印   -- 可打印 或 不打印
		{			
			if( nDiff > 200)   //当此函数: 栈>X字节   -- 可调节此值看更多的打印信息
				fprintf(fp_StackLog, (emCStrA)(" 栈：%08x                      val：%6d		this：%6d		%s%s\n"), k,nCurStackVal,nDiff, pText,"();");
		}

		g_nPrevStackSize = nCurStackVal;
	}
	
}


#else

#define	LOG_StackAddr(p)		((void)0)

#endif


/* 初始化 */
void emCall emHeap_InitH( emPHeap pHeap, emSize nSize )
{

	/* 检查参数的有效性 */
	emAssert(pHeap && nSize);
	/* 防止堆大小超出   */
	nSize = emGridSize(nSize) - 4;

	g_nMaxSize = 0;



	/* 初始化堆 */
	pHeap->m_pFreeStart = pHeap->m_pMemStart;
	pHeap->m_pFreeEnd = pHeap->m_pMemStart + (nSize - HEAP_HEADSIZE);
	pHeap->m_pObjHeap = emNull;

#if DEBUG_LOG_SWITCH_HEAP

	//栈内存
	fp_StackLog = fopen("log/日志_内存_栈.log", "w");
	g_BaseStack = 0;

	//堆内存
	fp_HeapLog =  fopen("log/日志_内存_堆.log", "w");	
	fprintf(fp_HeapLog, (emCStrA)("Load memory success, memory left %u.\n\n\n"), nSize);
	fprintf(fp_HeapLog, (emCStrA)("         start(%p,%6u)：           =     0                          peak:     0\n"), pHeap->m_pFreeStart,0);
	fprintf(fp_HeapLog, (emCStrA)("         end  (%p,%6u)：           =     0                          peak:     0\n\n"), pHeap->m_pFreeEnd,  0);
#endif

	
}

/* 逆初始化 */
void emCall emHeap_UninitH( emPHeap pHeap )
{


#if DEBUG_LOG_SWITCH_HEAP
	//堆内存
	fprintf(fp_HeapLog, (emCStrA)("\n\nUnload memory success, once peak value  alloc %u!!!\n"), g_nMaxSize);
	fclose(fp_HeapLog);

	//栈内存
	fprintf(fp_StackLog, (emCStrA)("\n\n once peak value: %u!!!\n"), g_nMaxStackSize);
	fclose(fp_StackLog);
#endif
}

//从堆空间的上端分配
#if DEBUG_LOG_SWITCH_HEAP
emPointer emCall emHeap_AllocZeroH( emPHeap pHeap, emSize nSize,emCharA *pText )
#else
emPointer emCall emHeap_AllocZeroH( emPHeap pHeap, emSize nSize )
#endif
{
	emPointer pBuffer;


	/* 检查参数的有效性 */
	emAssert(pHeap && nSize);
	nSize = emGridSize(nSize);

	emAssert(pHeap->m_pFreeStart + nSize <= pHeap->m_pFreeEnd);
	if ( pHeap->m_pFreeStart + nSize > pHeap->m_pFreeEnd )
	{
#if DEBUG_LOG_SWITCH_HEAP
		fprintf(fp_HeapLog, (emCStrA)("* Out of memory, alloc in heap Failed!   %20s\n"), pText);
#endif
		return emNull;
	}

	/* 分配内存 */
	pBuffer = pHeap->m_pFreeStart;
	pHeap->m_pFreeStart += nSize;

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("Start：  Alloc(%p,%6u)"), pBuffer, nSize);

	/* 记录当前分配量和分配峰值 */
	g_nSize += nSize;
	if ( g_nMaxSize < g_nSize )
	{
		g_nMaxSize = g_nSize;
		fprintf(fp_HeapLog, (emCStrA)("：   +%6u =%6u   %40s   peak:%6u\n"), nSize, g_nSize,pText, g_nMaxSize);
	}
	else
	{
		fprintf(fp_HeapLog, (emCStrA)("：   +%6u =%6u   %40s\n"),            nSize, g_nSize, pText, g_nMaxSize);
	}

#endif

	

	emMemSet(pBuffer, 0, nSize);
	/* 返回地址 */
	return pBuffer;
}

/* 释放内存 */   //从堆空间的上端释放
#if DEBUG_LOG_SWITCH_HEAP
void emCall emHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize ,emCharA *pText)
#else
void emCall emHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize)
#endif
{

	/* 检查参数的有效性 */
	emAssert(pHeap && pBuffer && nSize);
	nSize = emGridSize(nSize);

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("Start：  Free (%p,%6u)"), pBuffer, nSize);
	fprintf(fp_HeapLog, (emCStrA)("：   -%6u =%6u   %40s"),            nSize, g_nSize-nSize,pText, g_nMaxSize);	
	if( (pHeap->m_pFreeStart -nSize) != pBuffer)
	{
		fprintf(fp_HeapLog, (emCStrA)("                                       实际释放地址：=%p！ 出错，请检查！！！"),     (pHeap->m_pFreeStart -nSize));	
	}
	fprintf(fp_HeapLog, (emCStrA)("\n"));	
#endif

	/* 释放内存 */
	pHeap->m_pFreeStart -= nSize;
	g_nSize -= nSize;

}

/*++
	子对象堆相关
--*/

//从堆空间的下端分配
#if DEBUG_LOG_SWITCH_HEAP
emPointer emCall emObjHeap_AllocZeroH( emPHeap pHeap, emSize nSize,emCharA *pText )
#else
emPointer emCall emObjHeap_AllocZeroH( emPHeap pHeap, emSize nSize)
#endif
{


	/* 检查参数的有效性 */
	emAssert(pHeap && nSize);
	nSize = emGridSize(nSize);

	emAssert(pHeap->m_pFreeEnd - nSize >= pHeap->m_pFreeStart);
	if ( pHeap->m_pFreeEnd - nSize < pHeap->m_pFreeStart )
	{
#if DEBUG_LOG_SWITCH_HEAP
		fprintf(fp_HeapLog, (emCStrA)("* Out of memory, alloc in heap Failed!   %20s\n"),  pText);
#endif
		return emNull;
	}

	/* 分配内存 */
	pHeap->m_pFreeEnd -= nSize;

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("End  ：  Alloc(%p,%6u)"), pHeap->m_pFreeEnd, nSize);

	/* 记录当前分配量和分配峰值 */
	g_nSize += nSize;
	if ( g_nMaxSize < g_nSize )
	{
		g_nMaxSize = g_nSize;

		/* 输出日志 */
		fprintf(fp_HeapLog, (emCStrA)("：   +%6u =%6u   %40s   peak:%6u\n"), nSize, g_nSize,pText, g_nMaxSize);
	}
	else
	{
		/* 输出日志 */
		fprintf(fp_HeapLog, (emCStrA)("：   +%6u =%6u   %40s \n"),            nSize, g_nSize, pText, g_nMaxSize);
	}
#endif	

	emMemSet(pHeap->m_pFreeEnd, 0, nSize);

	/* 返回地址 */
	return pHeap->m_pFreeEnd;

}

/* 释放内存 */	//从堆空间的下端释放
#if DEBUG_LOG_SWITCH_HEAP
void emCall emObjHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize ,emCharA *pText)
#else
void emCall emObjHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize )
#endif
{



	/* 检查参数的有效性 */
	emAssert(pHeap && pBuffer && nSize);
	nSize = emGridSize(nSize);

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("End  ：  Free (%p,%6u)"), pBuffer, nSize);
	fprintf(fp_HeapLog, (emCStrA)("：   -%6u =%6u   %40s"),            nSize, g_nSize-nSize, pText,g_nMaxSize);	
	if( (pHeap->m_pFreeEnd ) != pBuffer)
	{
		fprintf(fp_HeapLog, (emCStrA)("                                       实际释放地址：=%p！ 出错，请检查！！！"),     (pHeap->m_pFreeEnd ));	
	}
	fprintf(fp_HeapLog, (emCStrA)("\n"));	
#endif

	/* 释放内存 */
	pHeap->m_pFreeEnd += nSize;
	g_nSize -= nSize;
}





//******************************** 内存操作库 **********************************************************

#if !EM_ANSI_MEMORY


/* 初始化内存块 */
void emCall emMemSet( emPointer pBuffer0, emByte nValue, emSize nSize )
{
	emPByte pBuffer;


	pBuffer = (emPByte)pBuffer0;
	emAssert(pBuffer);

	while ( nSize -- )
		*pBuffer++ = nValue;
}

/* 复制内存块 */
void emCall emMemCpy( emPointer pDesc0, emCPointer pSrc0, emSize nSize )
{
	emPByte pDesc;
	emPCByte pSrc;


	pDesc = (emPByte)pDesc0;
	pSrc = (emPCByte)pSrc0;
	emAssert(pDesc && pSrc);

	while ( nSize -- )
		*pDesc++ = *pSrc++;
}

/* 移动内存块 */
void emCall emMemMove( emPointer pDesc0, emCPointer pSrc0, emSize nSize )
{
	emPByte pDesc;
	emPCByte pSrc;


	pDesc = (emPByte)pDesc0;
	pSrc = (emPCByte)pSrc0;
	emAssert(pDesc && pSrc);

	if ( pDesc <= pSrc )
	{
		while ( nSize -- )
			*pDesc++ = *pSrc++;
	}
	else
	{
		pDesc += nSize;
		pSrc += nSize;

		while ( nSize -- )
			*--pDesc = *--pSrc;
	}
}

/* 比较内存块 */
emComp emCall emMemCmp( emCPointer pDesc0, emCPointer pSrc0, emSize nSize )
{
	emPCByte pDesc;
	emPCByte pSrc;


	pDesc = (emPCByte)pDesc0;
	pSrc = (emPCByte)pSrc0;
	emAssert(pDesc && pSrc);

	while ( nSize -- )
	{
		if ( *pDesc > *pSrc )
			return emGreater;

		if ( *pDesc < *pSrc )
			return emLesser;

		++ pDesc;
		++ pSrc;
	}

	return emEqual;
}
#endif



emUInt8 *emHeap_PrivateAllocZero(emUInt32 size)
{
	emUInt32  *Ptr;
#if DEBUG_LOG_SWITCH_HEAP
	emUInt8   TempBuf2[4];
	emUInt8   TempBuf1[12]="malloc ";
	emUInt32  t1;
#endif

	LOG_StackAddr(__FUNCTION__);
	size = emGridSize(size);

#if DEBUG_LOG_SWITCH_HEAP
	t1 = g_PrivateMallocCurMarkCount;	
	TempBuf2[0] = 0x30 + t1/100;
	TempBuf2[1] = 0x30 + (t1%100 - t1%10)/10;
	TempBuf2[2] = 0x30 + t1%10;
	TempBuf2[3] = 0;						//t1的值转成字符串TempBuf2
	emMemCpy(TempBuf1+7, TempBuf2, 4);
	Ptr =  emHeap_AllocZero(size,TempBuf1);	
#else
	Ptr =  emHeap_AllocZero(size);	
#endif

	(g_PrivateMallocMark+g_PrivateMallocCurMarkCount)->serial = g_PrivateMallocCurMarkCount;
	(g_PrivateMallocMark+g_PrivateMallocCurMarkCount)->addr = Ptr;
	(g_PrivateMallocMark+g_PrivateMallocCurMarkCount)->size = size;
	
	g_PrivateMallocCurMarkCount++;
	if(g_PrivateMallocCurMarkCount > MaxMallocMarkCount)		//超过数组的最大个数
	{
		;	//		SYN_Malloc_Trace("Malloc list is full ,Ptr=:%08x,size=%d",Ptr,size);		
		return NULL;
	}
	return Ptr;
}


//释放当前地址的内存，顺便会把“未释放队列中”能释放的也释放掉
void emHeap_PrivateFree(void *Ptr)
{
	emUInt32  i,j;
#if DEBUG_LOG_SWITCH_HEAP
	emUInt8   TempBuf2[4];
	emUInt8   TempBuf1[12]="free   ";
	emUInt32  t1;
#endif
	
	LOG_StackAddr(__FUNCTION__);
	if(g_PrivateMallocCurMarkCount <= 0 )		//释放次数 > 开辟次数
	{
		;	//SYN_Malloc_Trace("%s, error ,Ptr=:%08x",__func__,Ptr);
		return ;
	}
	if((emUInt32)Ptr == (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr) // 适合释放
	{		
		//真正释放一个
#if DEBUG_LOG_SWITCH_HEAP
		t1 = (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->serial;	
		TempBuf2[0] = 0x30 + t1/100;
		TempBuf2[1] = 0x30 + (t1%100 - t1%10)/10;
		TempBuf2[2] = 0x30 + t1%10;
		TempBuf2[3] = 0;						//t1的值转成字符串TempBuf2
		emMemCpy(TempBuf1+7, TempBuf2, 4);
		emHeap_Free(Ptr,(g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size,TempBuf1);		
#else
		emHeap_Free(Ptr,(g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size);
#endif
		g_PrivateMallocCurMarkCount--;
		
		if(g_PrivateMallocCurMarkCount == 0)//未释放队列中是0
		{
			;	//SYN_Malloc_Trace("%s, free success  and all Malloc space is freed,Ptr=:%08x",__func__,Ptr);
			return;
		}
		else			//未释放队列中还有需要释放的
		{
			//循环释放“未释放队列中”中的
			while(1)
			{
				if( g_PrivateMallocCurMarkCount <= 0)
					break;

				for(i=0;i< MaxMallocMarkCount;i++)					//在未释放队列中搜索
				{				
					if(((g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr == (g_PrivateFreeMark+i)->addr)&&((g_PrivateFreeMark+i)->addr !=0))
					{
						(g_PrivateFreeMark+i)->addr  = 0;
						(g_PrivateFreeMark+i)->size = 0;
						(g_PrivateFreeMark+i)->serial = 0;
						
						g_PrivateNeedFreeMarkCount--;
						
						//真正释放一个
#if DEBUG_LOG_SWITCH_HEAP
						t1 =  (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->serial;	
						TempBuf2[0] = 0x30 + t1/100;
						TempBuf2[1] = 0x30 + (t1%100 - t1%10)/10;
						TempBuf2[2] = 0x30 + t1%10;
						TempBuf2[3] = 0;							//t1的值转成字符串TempBuf2
						emMemCpy(TempBuf1+7, TempBuf2, 4);
						emHeap_Free((g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr, (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size,TempBuf1);		
#else
						emHeap_Free((g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr, (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size);
#endif
						g_PrivateMallocCurMarkCount--;
						break;
					}					
				}
				if( i == MaxMallocMarkCount )						//在未释放队列中：没搜索到
						break;
			}
		}
	}
	else// 不适合释放
	{
		for(i=0;i< g_PrivateMallocCurMarkCount;i++)				//搜索释放地址在不在申请列表中
		{
			if((emUInt32)Ptr == (g_PrivateMallocMark+i)->addr)	//在列表中（不是野值），添加到未释放列表
			{
				for(j=0;j<MaxMallocMarkCount;j++)
				{
					if((g_PrivateFreeMark+j)->addr == 0)
					{
						(g_PrivateFreeMark+j)->serial  = (g_PrivateMallocMark+i)->serial;
						(g_PrivateFreeMark+j)->addr   = (g_PrivateMallocMark+i)->addr;
						(g_PrivateFreeMark+j)->size    = (g_PrivateMallocMark+i)->size;
						g_PrivateNeedFreeMarkCount++;
						return;
					}
				 }
			}
		}
		return;
	}
}

void emHeap_PrivateInit(void)
{
	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	g_PrivateMallocMark = emHeap_AllocZero(MaxMallocMarkCount*sizeof(PrivateMallocMark),"g_PrivateMallocMark：《英文标准化》");	//用来存放所有申请过的信息	
	g_PrivateFreeMark = emHeap_AllocZero(MaxMallocMarkCount*sizeof(PrivateFreeMark),"g_PrivateFreeMark：《英文标准化》");			//用来存放想释放，但是未能释放掉的信息       
#else
	g_PrivateMallocMark = emHeap_AllocZero(MaxMallocMarkCount*sizeof(PrivateMallocMark));
	g_PrivateFreeMark = emHeap_AllocZero(MaxMallocMarkCount*sizeof(PrivateFreeMark));
#endif
	g_PrivateMallocCurMarkCount = 0;	
	g_PrivateNeedFreeMarkCount = 0;
}

void emHeap_PrivateUnInit(void)
{
	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_PrivateFreeMark,MaxMallocMarkCount*sizeof(PrivateFreeMark),"g_PrivateFreeMark：《英文标准化》");
	emHeap_Free(g_PrivateMallocMark,MaxMallocMarkCount*sizeof(PrivateMallocMark),"g_PrivateMallocMark：《英文标准化》");
#else
	emHeap_Free(g_PrivateFreeMark,MaxMallocMarkCount*sizeof(PrivateFreeMark));
	emHeap_Free(g_PrivateMallocMark,MaxMallocMarkCount*sizeof(PrivateMallocMark));
#endif

	if(g_PrivateMallocCurMarkCount!= 0 || g_PrivateNeedFreeMarkCount != 0 )
		;	//出错，必须都为0
}

