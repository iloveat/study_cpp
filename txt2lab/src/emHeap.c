/* �ѣ�ջ���ļ�ֱ�Ӷ�ȡ -- ����Դ�ļ� */

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


//��ӡջ����ַ
void emCall LOG_StackAddr(emCharA *pText)
{
	emInt32  k; 
	emUInt32 nCurStackVal;
	emInt32  nDiff;

	if( g_nMaxStackSize >5950)			//������
		g_nMaxStackSize=g_nMaxStackSize;

	if( g_BaseStack == 0)
	{
			//ջ�ڴ�   sqb 2017-5-27
//		__asm__
//		(
//			mov g_BaseStack, esp
//		)
		g_nMaxStackSize = 0;
		g_nPrevStackSize = 0;
		fprintf(fp_StackLog, (emCStrA)(" ջ��%08x    ��׼��%6d						          %s%s\n"), g_BaseStack, 0, pText,"();");
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

		if( nCurStackVal> g_nMaxStackSize)		//��ֵ��ӡ
		{
			g_nMaxStackSize = nCurStackVal;
			fprintf(fp_StackLog, (emCStrA)(" ջ��%08x    peak��%6d      val��%6d		this��%6d		%s%s\n"), k, g_nMaxStackSize,nCurStackVal,nDiff, pText,"();");
		}
		else										//�Ƿ�ֵ��ӡ   -- �ɴ�ӡ �� ����ӡ
		{			
			if( nDiff > 200)   //���˺���: ջ>X�ֽ�   -- �ɵ��ڴ�ֵ������Ĵ�ӡ��Ϣ
				fprintf(fp_StackLog, (emCStrA)(" ջ��%08x                      val��%6d		this��%6d		%s%s\n"), k,nCurStackVal,nDiff, pText,"();");
		}

		g_nPrevStackSize = nCurStackVal;
	}
	
}


#else

#define	LOG_StackAddr(p)		((void)0)

#endif


/* ��ʼ�� */
void emCall emHeap_InitH( emPHeap pHeap, emSize nSize )
{

	/* ����������Ч�� */
	emAssert(pHeap && nSize);
	/* ��ֹ�Ѵ�С����   */
	nSize = emGridSize(nSize) - 4;

	g_nMaxSize = 0;



	/* ��ʼ���� */
	pHeap->m_pFreeStart = pHeap->m_pMemStart;
	pHeap->m_pFreeEnd = pHeap->m_pMemStart + (nSize - HEAP_HEADSIZE);
	pHeap->m_pObjHeap = emNull;

#if DEBUG_LOG_SWITCH_HEAP

	//ջ�ڴ�
	fp_StackLog = fopen("log/��־_�ڴ�_ջ.log", "w");
	g_BaseStack = 0;

	//���ڴ�
	fp_HeapLog =  fopen("log/��־_�ڴ�_��.log", "w");	
	fprintf(fp_HeapLog, (emCStrA)("Load memory success, memory left %u.\n\n\n"), nSize);
	fprintf(fp_HeapLog, (emCStrA)("         start(%p,%6u)��           =     0                          peak:     0\n"), pHeap->m_pFreeStart,0);
	fprintf(fp_HeapLog, (emCStrA)("         end  (%p,%6u)��           =     0                          peak:     0\n\n"), pHeap->m_pFreeEnd,  0);
#endif

	
}

/* ���ʼ�� */
void emCall emHeap_UninitH( emPHeap pHeap )
{


#if DEBUG_LOG_SWITCH_HEAP
	//���ڴ�
	fprintf(fp_HeapLog, (emCStrA)("\n\nUnload memory success, once peak value  alloc %u!!!\n"), g_nMaxSize);
	fclose(fp_HeapLog);

	//ջ�ڴ�
	fprintf(fp_StackLog, (emCStrA)("\n\n once peak value: %u!!!\n"), g_nMaxStackSize);
	fclose(fp_StackLog);
#endif
}

//�Ӷѿռ���϶˷���
#if DEBUG_LOG_SWITCH_HEAP
emPointer emCall emHeap_AllocZeroH( emPHeap pHeap, emSize nSize,emCharA *pText )
#else
emPointer emCall emHeap_AllocZeroH( emPHeap pHeap, emSize nSize )
#endif
{
	emPointer pBuffer;


	/* ����������Ч�� */
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

	/* �����ڴ� */
	pBuffer = pHeap->m_pFreeStart;
	pHeap->m_pFreeStart += nSize;

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("Start��  Alloc(%p,%6u)"), pBuffer, nSize);

	/* ��¼��ǰ�������ͷ����ֵ */
	g_nSize += nSize;
	if ( g_nMaxSize < g_nSize )
	{
		g_nMaxSize = g_nSize;
		fprintf(fp_HeapLog, (emCStrA)("��   +%6u =%6u   %40s   peak:%6u\n"), nSize, g_nSize,pText, g_nMaxSize);
	}
	else
	{
		fprintf(fp_HeapLog, (emCStrA)("��   +%6u =%6u   %40s\n"),            nSize, g_nSize, pText, g_nMaxSize);
	}

#endif

	

	emMemSet(pBuffer, 0, nSize);
	/* ���ص�ַ */
	return pBuffer;
}

/* �ͷ��ڴ� */   //�Ӷѿռ���϶��ͷ�
#if DEBUG_LOG_SWITCH_HEAP
void emCall emHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize ,emCharA *pText)
#else
void emCall emHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize)
#endif
{

	/* ����������Ч�� */
	emAssert(pHeap && pBuffer && nSize);
	nSize = emGridSize(nSize);

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("Start��  Free (%p,%6u)"), pBuffer, nSize);
	fprintf(fp_HeapLog, (emCStrA)("��   -%6u =%6u   %40s"),            nSize, g_nSize-nSize,pText, g_nMaxSize);	
	if( (pHeap->m_pFreeStart -nSize) != pBuffer)
	{
		fprintf(fp_HeapLog, (emCStrA)("                                       ʵ���ͷŵ�ַ��=%p�� �������飡����"),     (pHeap->m_pFreeStart -nSize));	
	}
	fprintf(fp_HeapLog, (emCStrA)("\n"));	
#endif

	/* �ͷ��ڴ� */
	pHeap->m_pFreeStart -= nSize;
	g_nSize -= nSize;

}

/*++
	�Ӷ�������
--*/

//�Ӷѿռ���¶˷���
#if DEBUG_LOG_SWITCH_HEAP
emPointer emCall emObjHeap_AllocZeroH( emPHeap pHeap, emSize nSize,emCharA *pText )
#else
emPointer emCall emObjHeap_AllocZeroH( emPHeap pHeap, emSize nSize)
#endif
{


	/* ����������Ч�� */
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

	/* �����ڴ� */
	pHeap->m_pFreeEnd -= nSize;

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("End  ��  Alloc(%p,%6u)"), pHeap->m_pFreeEnd, nSize);

	/* ��¼��ǰ�������ͷ����ֵ */
	g_nSize += nSize;
	if ( g_nMaxSize < g_nSize )
	{
		g_nMaxSize = g_nSize;

		/* �����־ */
		fprintf(fp_HeapLog, (emCStrA)("��   +%6u =%6u   %40s   peak:%6u\n"), nSize, g_nSize,pText, g_nMaxSize);
	}
	else
	{
		/* �����־ */
		fprintf(fp_HeapLog, (emCStrA)("��   +%6u =%6u   %40s \n"),            nSize, g_nSize, pText, g_nMaxSize);
	}
#endif	

	emMemSet(pHeap->m_pFreeEnd, 0, nSize);

	/* ���ص�ַ */
	return pHeap->m_pFreeEnd;

}

/* �ͷ��ڴ� */	//�Ӷѿռ���¶��ͷ�
#if DEBUG_LOG_SWITCH_HEAP
void emCall emObjHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize ,emCharA *pText)
#else
void emCall emObjHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize )
#endif
{



	/* ����������Ч�� */
	emAssert(pHeap && pBuffer && nSize);
	nSize = emGridSize(nSize);

#if DEBUG_LOG_SWITCH_HEAP
	fprintf(fp_HeapLog, (emCStrA)("End  ��  Free (%p,%6u)"), pBuffer, nSize);
	fprintf(fp_HeapLog, (emCStrA)("��   -%6u =%6u   %40s"),            nSize, g_nSize-nSize, pText,g_nMaxSize);	
	if( (pHeap->m_pFreeEnd ) != pBuffer)
	{
		fprintf(fp_HeapLog, (emCStrA)("                                       ʵ���ͷŵ�ַ��=%p�� �������飡����"),     (pHeap->m_pFreeEnd ));	
	}
	fprintf(fp_HeapLog, (emCStrA)("\n"));	
#endif

	/* �ͷ��ڴ� */
	pHeap->m_pFreeEnd += nSize;
	g_nSize -= nSize;
}





//******************************** �ڴ������ **********************************************************

#if !EM_ANSI_MEMORY


/* ��ʼ���ڴ�� */
void emCall emMemSet( emPointer pBuffer0, emByte nValue, emSize nSize )
{
	emPByte pBuffer;


	pBuffer = (emPByte)pBuffer0;
	emAssert(pBuffer);

	while ( nSize -- )
		*pBuffer++ = nValue;
}

/* �����ڴ�� */
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

/* �ƶ��ڴ�� */
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

/* �Ƚ��ڴ�� */
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
	TempBuf2[3] = 0;						//t1��ֵת���ַ���TempBuf2
	emMemCpy(TempBuf1+7, TempBuf2, 4);
	Ptr =  emHeap_AllocZero(size,TempBuf1);	
#else
	Ptr =  emHeap_AllocZero(size);	
#endif

	(g_PrivateMallocMark+g_PrivateMallocCurMarkCount)->serial = g_PrivateMallocCurMarkCount;
	(g_PrivateMallocMark+g_PrivateMallocCurMarkCount)->addr = Ptr;
	(g_PrivateMallocMark+g_PrivateMallocCurMarkCount)->size = size;
	
	g_PrivateMallocCurMarkCount++;
	if(g_PrivateMallocCurMarkCount > MaxMallocMarkCount)		//���������������
	{
		;	//		SYN_Malloc_Trace("Malloc list is full ,Ptr=:%08x,size=%d",Ptr,size);		
		return NULL;
	}
	return Ptr;
}


//�ͷŵ�ǰ��ַ���ڴ棬˳���ѡ�δ�ͷŶ����С����ͷŵ�Ҳ�ͷŵ�
void emHeap_PrivateFree(void *Ptr)
{
	emUInt32  i,j;
#if DEBUG_LOG_SWITCH_HEAP
	emUInt8   TempBuf2[4];
	emUInt8   TempBuf1[12]="free   ";
	emUInt32  t1;
#endif
	
	LOG_StackAddr(__FUNCTION__);
	if(g_PrivateMallocCurMarkCount <= 0 )		//�ͷŴ��� > ���ٴ���
	{
		;	//SYN_Malloc_Trace("%s, error ,Ptr=:%08x",__func__,Ptr);
		return ;
	}
	if((emUInt32)Ptr == (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr) // �ʺ��ͷ�
	{		
		//�����ͷ�һ��
#if DEBUG_LOG_SWITCH_HEAP
		t1 = (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->serial;	
		TempBuf2[0] = 0x30 + t1/100;
		TempBuf2[1] = 0x30 + (t1%100 - t1%10)/10;
		TempBuf2[2] = 0x30 + t1%10;
		TempBuf2[3] = 0;						//t1��ֵת���ַ���TempBuf2
		emMemCpy(TempBuf1+7, TempBuf2, 4);
		emHeap_Free(Ptr,(g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size,TempBuf1);		
#else
		emHeap_Free(Ptr,(g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size);
#endif
		g_PrivateMallocCurMarkCount--;
		
		if(g_PrivateMallocCurMarkCount == 0)//δ�ͷŶ�������0
		{
			;	//SYN_Malloc_Trace("%s, free success  and all Malloc space is freed,Ptr=:%08x",__func__,Ptr);
			return;
		}
		else			//δ�ͷŶ����л�����Ҫ�ͷŵ�
		{
			//ѭ���ͷš�δ�ͷŶ����С��е�
			while(1)
			{
				if( g_PrivateMallocCurMarkCount <= 0)
					break;

				for(i=0;i< MaxMallocMarkCount;i++)					//��δ�ͷŶ���������
				{				
					if(((g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr == (g_PrivateFreeMark+i)->addr)&&((g_PrivateFreeMark+i)->addr !=0))
					{
						(g_PrivateFreeMark+i)->addr  = 0;
						(g_PrivateFreeMark+i)->size = 0;
						(g_PrivateFreeMark+i)->serial = 0;
						
						g_PrivateNeedFreeMarkCount--;
						
						//�����ͷ�һ��
#if DEBUG_LOG_SWITCH_HEAP
						t1 =  (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->serial;	
						TempBuf2[0] = 0x30 + t1/100;
						TempBuf2[1] = 0x30 + (t1%100 - t1%10)/10;
						TempBuf2[2] = 0x30 + t1%10;
						TempBuf2[3] = 0;							//t1��ֵת���ַ���TempBuf2
						emMemCpy(TempBuf1+7, TempBuf2, 4);
						emHeap_Free((g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr, (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size,TempBuf1);		
#else
						emHeap_Free((g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->addr, (g_PrivateMallocMark+g_PrivateMallocCurMarkCount -1)->size);
#endif
						g_PrivateMallocCurMarkCount--;
						break;
					}					
				}
				if( i == MaxMallocMarkCount )						//��δ�ͷŶ����У�û������
						break;
			}
		}
	}
	else// ���ʺ��ͷ�
	{
		for(i=0;i< g_PrivateMallocCurMarkCount;i++)				//�����ͷŵ�ַ�ڲ��������б���
		{
			if((emUInt32)Ptr == (g_PrivateMallocMark+i)->addr)	//���б��У�����Ұֵ������ӵ�δ�ͷ��б�
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
	g_PrivateMallocMark = emHeap_AllocZero(MaxMallocMarkCount*sizeof(PrivateMallocMark),"g_PrivateMallocMark����Ӣ�ı�׼����");	//��������������������Ϣ	
	g_PrivateFreeMark = emHeap_AllocZero(MaxMallocMarkCount*sizeof(PrivateFreeMark),"g_PrivateFreeMark����Ӣ�ı�׼����");			//����������ͷţ�����δ���ͷŵ�����Ϣ       
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
	emHeap_Free(g_PrivateFreeMark,MaxMallocMarkCount*sizeof(PrivateFreeMark),"g_PrivateFreeMark����Ӣ�ı�׼����");
	emHeap_Free(g_PrivateMallocMark,MaxMallocMarkCount*sizeof(PrivateMallocMark),"g_PrivateMallocMark����Ӣ�ı�׼����");
#else
	emHeap_Free(g_PrivateFreeMark,MaxMallocMarkCount*sizeof(PrivateFreeMark));
	emHeap_Free(g_PrivateMallocMark,MaxMallocMarkCount*sizeof(PrivateMallocMark));
#endif

	if(g_PrivateMallocCurMarkCount!= 0 || g_PrivateNeedFreeMarkCount != 0 )
		;	//�������붼Ϊ0
}

