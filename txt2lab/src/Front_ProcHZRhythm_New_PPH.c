
#include "emPCH.h"

#include "Front_ProcHZRhythm_New_PPH.h"




#define PPH_COST_GATE			20000	
#define PPH_COST_1TO6GATE		16000	
#define PPH_COST_TRACE_GATE		1541	

#define TWO_PPH_COST_BASE	2000
#define THREE_PPH_COST_BASE	5000

#define MAX_PPH_LEN			11
#define LEN_TO_CONSIDER		6



#define C45NodeType_Leaf		0
#define C45NodeType_Branch		1
#define C45NodeType_CutTwoStub  2



#define C45Node_NodeType	0	
#define C45Node_AttrNo		2	
#define C45Node_Borderline	4	
#define C45Node_PphCost		2	
#define C45Node_PwCost		4	
#define C45Node_ChildPos	6	



#define C45Node_Size	6


typedef emUInt8 TC45Node[C45Node_Size];


/* PW�������� */
#define PwAttr_CurPoS		0	/* ��ǰ���� */
#define PwAttr_CurLen		1	/* ��ǰ�ʳ��� */
#define PwAttr_CurPwLen		2	/* ��ǰPW���� */
#define PwAttr_CurCost		3	/* ��ǰ�ʴ��� */

#define PwAttr_PrevPoS		4	/* ǰ���� */
#define PwAttr_PrevLen		5	/* ǰ�ʳ��� */
#define PwAttr_PrevPwLen	6	/* ǰPW���� */
#define PwAttr_PrevCost		7	/* ǰ�ʴ��� */

#define PwAttr_NextPoS		8	/* ����� */
#define PwAttr_NextLen		9	/* ��ʳ��� */
#define PwAttr_NextPwLen	10	/* ��PW���� */

#define PwAttr_PrevPrevPoS	11	/* ǰǰ���� */
#define PwAttr_PrevPrevLen	12	/* ǰǰ�ʳ��� */

#define PwAttr_NextNextPoS	13	/* ������ */
#define PwAttr_NextNextLen	14	/* ���ʳ��� */


/* PW������ */
#define PwAttr_Count	15

/* PW�������� */
typedef emUInt8 TPwSerial[PwAttr_Count];



typedef struct tagCostInfo TCostInfo, emPtr PCostInfo;

struct tagCostInfo
{
	emUInt16 wPphCost;
	emUInt16 wPwCost;
};

typedef struct tagWordPosLen TWordPosLen, emPtr PWordPosLen;

struct tagWordPosLen
{
	
	emUInt8   prePreFeaPos;		//ǰǰ��    ������
	emUInt8   preFeaPos;		//ǰ��      ������
	emUInt8   curFeaPos;		//��ǰ�����ʣ�����
	emUInt8   nextFeaPos;		//���      ������
	emUInt8   nextNextFeaPos;	//����    ������

	emUInt8   prePreFeaLen;		//ǰǰ��    ���ʳ�
	emUInt8   preFeaLen;		//ǰ��      ���ʳ�
	emUInt8   curFeaLen;		//��ǰ�����ʣ��ʳ�	
	emUInt8   nextFeaLen;		//���      ���ʳ�
	emUInt8   nextNextFeaLen;	//����    ���ʳ�
};



/******************************************************************************
* ������        : GetPoSVal
* ����          : ������ת�ɾ�������Ҫ����ɢֵ
* ����          : [in] tPoS - ����
* ����          : <emUInt8> - ��ɢֵ

******************************************************************************/
emStatic emUInt8 emCall GetPoSVal(emUInt8 tPoS)
{

	LOG_StackAddr(__FUNCTION__);

	if ( PoS_Null == tPoS )
		return 0;

	if ( tPoS >= ChsPoS_a && tPoS <= ChsPoS_ngp )
		return tPoS - ChsPoS_a + 1;

	return 6;
}

/******************************************************************************
* ������        : AskPphPwCost
* ����          : ���PPH��PW�ڵ����
* ����          : [in]  pRhythmRes   - ����ģ����Դ����
*               : [in]  tPwAttrs - PW����
*               : [out] pCost    - PW��PPH�Ĵ���
* ����          : <��>

******************************************************************************/
emStatic void emCall AskPphPwCost(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	TPwSerial		tPwAttrs,			/* PW���� */
	PCostInfo		pCost				/* PW��PPH�Ĵ��� */
)
{
	emAddress nPos;

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRes && tPwAttrs && pCost);

	for ( nPos = 0; ; )
	{
		TC45Node tNode;

		emRes_SetPos(pRhythmRes->m_subSyntax.m_pC45Res, nPos);
		emRes_Read8s(pRhythmRes->m_subSyntax.m_pC45Res, tNode, C45Node_Size);
		nPos += C45Node_Size;

		if ( C45NodeType_Leaf == tNode[C45Node_NodeType] )
		{
			pCost->wPphCost = em16From8s(tNode + C45Node_PphCost);
			pCost->wPwCost = em16From8s(tNode + C45Node_PwCost);
			return;
		}

		switch ( tNode[C45Node_NodeType] )
		{
		case C45NodeType_Branch:
			nPos += emResSize_Int16 * tPwAttrs[tNode[C45Node_AttrNo]];
			break;

		case C45NodeType_CutTwoStub:
			if ( tPwAttrs[tNode[C45Node_AttrNo]] > tNode[C45Node_Borderline] )
				nPos += emResSize_Int16;
			break;

		default:
			emAssert(emFalse);
			return;
		}

		emRes_SetPos(pRhythmRes->m_subSyntax.m_pC45Res, nPos);
		nPos = emRes_Get16(pRhythmRes->m_subSyntax.m_pC45Res);
		emAssert(nPos);
		if ( !nPos )
			return;
	}

	emAssert(emFalse);
}

/******************************************************************************
* ������        : CalcSplitCost
* ����          : ����PPH���ִ���
* ����          : [in]  pRhythmRes   - ����ģ����Դ����
*               : [in]  pRhythmRT - ����ʵʱ����
*               : [out] pCost    - PW��PPH��������
* ����          : <emBool>       - �Ƿ�ɹ�

******************************************************************************/
emInline emBool emCall CalcSplitCost(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PCostInfo		pCost			/* PW��PPH�������� */
)
{
	TPwSerial tPwAttrs;
	emUInt8 n, i, iNext;

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRes && pRhythmRT && pCost);

	/* ��ȡ������ */
	n = GetSylCount(pRhythmRT);
	if ( 0 == n )
		return emFalse;	

	/* ��ȡ��2���ʵ��������� */
	iNext = GetNextWord(pRhythmRT, 0);
	if ( iNext >= n )
		return emFalse;	

	/* ��ʼ������ */
	emMemSet(pCost, 0,sizeof(TCostInfo) * n);

	/* ���ص�1���ʵ����Ե���ǰ������ */ 
	tPwAttrs[PwAttr_CurPoS] = GetPoSVal(GetPoS(pRhythmRT, 0));
	tPwAttrs[PwAttr_CurLen] = GetWordLen(pRhythmRT, 0);
	if (tPwAttrs[PwAttr_CurLen] > 7)
		tPwAttrs[PwAttr_CurLen] = 7;
	tPwAttrs[PwAttr_CurPwLen] = GetPWLen(pRhythmRT, 0);
	tPwAttrs[PwAttr_CurCost] = GetFrCost(pRhythmRT, 0);

	/* ���غ�ʵ����� */ 
	tPwAttrs[PwAttr_NextPoS] = GetPoSVal(GetPoS(pRhythmRT, iNext));
	tPwAttrs[PwAttr_NextLen] = GetWordLen(pRhythmRT, iNext);
	if (tPwAttrs[PwAttr_NextLen] > 7)
		tPwAttrs[PwAttr_NextLen] = 7;

	/* ǰ�ʵ�������Ϊ�� */
	tPwAttrs[PwAttr_PrevPoS] = 0;
	tPwAttrs[PwAttr_PrevLen] = 0;

	/* ��ȡ��3���ʵ��������� */
	iNext = GetNextWord(pRhythmRT, iNext);

	/* ���ص�3���ʵ����Ե��������� */
	if ( iNext >= n )
	{
		tPwAttrs[PwAttr_NextNextPoS] = 0;
		tPwAttrs[PwAttr_NextNextLen] = 0;
	}
	else
	{
		tPwAttrs[PwAttr_NextNextPoS] = GetPoSVal(GetPoS(pRhythmRT, iNext));
		tPwAttrs[PwAttr_NextNextLen] = GetWordLen(pRhythmRT, iNext);
		if (tPwAttrs[PwAttr_NextNextLen] > 7)
			tPwAttrs[PwAttr_NextNextLen] = 7;
	}

	for ( i = GetNextWord(pRhythmRT, 0); i < n; i = GetNextWord(pRhythmRT, i) )
	{
		/* �������� */
		tPwAttrs[PwAttr_PrevPrevPoS] = tPwAttrs[PwAttr_PrevPoS];
		tPwAttrs[PwAttr_PrevPrevLen] = tPwAttrs[PwAttr_PrevLen];

		tPwAttrs[PwAttr_PrevPoS] = tPwAttrs[PwAttr_CurPoS];
		tPwAttrs[PwAttr_PrevLen] = tPwAttrs[PwAttr_CurLen];
		tPwAttrs[PwAttr_PrevPwLen] = tPwAttrs[PwAttr_CurPwLen];
		tPwAttrs[PwAttr_PrevCost] = tPwAttrs[PwAttr_CurCost];

		tPwAttrs[PwAttr_CurPoS] = tPwAttrs[PwAttr_NextPoS];
		tPwAttrs[PwAttr_CurLen] = tPwAttrs[PwAttr_NextLen];
		tPwAttrs[PwAttr_CurCost] = GetFrCost(pRhythmRT, i);

		tPwAttrs[PwAttr_NextPoS] = tPwAttrs[PwAttr_NextNextPoS];
		tPwAttrs[PwAttr_NextLen] = tPwAttrs[PwAttr_NextNextLen];

		/* ��ȡ���ʵ��������� */
		if (iNext < n)
			iNext = GetNextWord(pRhythmRT, iNext);

		if ( iNext >= n )
		{
			tPwAttrs[PwAttr_NextNextPoS] = 0;
			tPwAttrs[PwAttr_NextNextLen] = 0;
		}
		else
		{
			tPwAttrs[PwAttr_NextNextPoS] = GetPoSVal(GetPoS(pRhythmRT, iNext));
			tPwAttrs[PwAttr_NextNextLen] = GetWordLen(pRhythmRT, iNext);
			if (tPwAttrs[PwAttr_NextNextLen] > 7)
				tPwAttrs[PwAttr_NextNextLen] = 7;
		}

		/* ������Ǳ߽磬���ܻ���PPH */
		if ( GetBoundaryBefore(pRhythmRT, i) < BdPW || TestFrontFlag(pRhythmRT, i, FrontFlag_NotPPH) )
		{
			/* ���ô���Ϊ��� */
			pCost[i].wPwCost = 1;
			pCost[i].wPphCost = EM_USHORT_MAX;
		}
		else
		{
			emUInt8 iNextPW;
			emPCUInt8 pText;

			/* ����ǰPW���� */ 
			tPwAttrs[PwAttr_CurPwLen] = GetPWLen(pRhythmRT, GetPrevPW(pRhythmRT, i));

			/* ���غ����ɴʳ� */ 
			iNextPW = GetNextPW(pRhythmRT, i);
			tPwAttrs[PwAttr_NextPwLen] = iNextPW >= n ? 0 : GetPWLen(pRhythmRT, iNextPW);

			/* ��ȡ��ǰ�ʵ��ı� */
			pText = (emPCUInt8)GetText(pRhythmRT, i);

			/* ʹ�þ������õ�PW��PPH�Ĵ��� */
			AskPphPwCost(pRhythmRes, tPwAttrs, pCost + i);
			if ( 0 == pCost[i].wPwCost )
			pCost[i].wPwCost = 1;
		}
	}

	/* ���һ�����ڿ϶���PPH������������PW����Ϊ��� */
	pCost[i].wPwCost = EM_USHORT_MAX;

	return emTrue;
}

/******************************************************************************
* ������        : FindCurSnap
* ����          : Ѱ�ҵ�ǰ�ڵ��Ժ�6������,16�����ڵ�һ�����Ŷϵ� 
* ����          : [in] pRhythmRT - ����ʵʱ����
*               : [in] pCost    - PW��PPH��������
*               : [in] iPos     - ��ǰλ��
* ����          : <emUInt8>  - ���ŵ�λ��

******************************************************************************/
emInline emUInt8 emCall FindCurSnap(
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PCostInfo		pCost,			/* PW��PPH�������� */
	emUInt8		iPos				/* ��ǰλ�� */
)
{
	emStatic emConst emUInt16 PPHProbWeight[] =
	{
		65535,	65535,	65535,	65535,	14692, 8576,	/* <6 */  //hyl 2012-02-23 ԭ��Ϊ 65535,	65535,	65535,	65535,	65535,	65535,
		2116,	1000,	0,		0,		1000,			/* 7-11 */
		2116,	5846,	8576,	14692,	22844			/* 12-16 */
	};

	emUInt8 j, i, iBest;
	emUInt32 nMinCost;

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRT && pCost);
	emAssert(0 == iPos || GetBoundaryBefore(pRhythmRT, iPos) >= BdPW);
	emAssert(0 == iPos || pCost[iPos].wPwCost);

	/* ����16����������,�������һ���ڵ� */
	j = GetSylCount(pRhythmRT) - iPos;
	if ( j <= 16 )
		return GetSylCount(pRhythmRT);

	
	/* �����16������ */
	if ( j > 16 )
		j = 16;

	/* �������λ�� */
	j += iPos;

	/* ��������λ�� */
	iBest = iPos;
	nMinCost = EM_ULONG_MAX;

	for ( i = iPos; i < j; i = GetNextWord(pRhythmRT, i) )
	{
		emUInt32 nCost;

		if( i>= (iPos+4) && pCost[i].wPphCost>0 )		//hyl 2012-02-23 ԭ������Ϊ6
		{
			nCost = pCost[i].wPphCost + PPHProbWeight[i - iPos];

			if ( nCost < nMinCost )
			{
				iBest = i;
				nMinCost = nCost;
			}
		}
	}

	return iBest;
}


/******************************************************************************
* ������        : GetOtherGandidate
* ����          : ��PPH��������ѡ�����ŵ������㣬������������
* ����          : [in]  pCost  - PW��PPH��������
*               : [in]  iBegin - ��ʼ����
*               : [in]  iEnd   - ��������
*               : [out] pBest  - ���ŵ�λ������
* ����          : <��>

******************************************************************************/
emInline void emCall GetOtherGandidate(
	PCostInfo		pCost,			/* PW��PPH�������� */
	emUInt8			iBegin,			/* ��ʼ���� */
	emUInt8			iEnd,			/* �������� */
	emUInt8 *		pBest			/* ���ŵ�λ������ */
)
{
	emUInt8 i;

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pCost);

	/* ��1~4�������һ�� */
	for ( i = iBegin + 1; i <= emMin(iBegin + 4, iEnd); ++ i )
	{
		if ( 0 == pCost[i].wPwCost )
			continue;

		if ( pCost[i].wPphCost > PPH_COST_1TO6GATE )
			continue;

		if ( 0 == pBest[0] )
		{
			pBest[0] = i;
		}
		else if ( pCost[i].wPphCost < pCost[pBest[0]].wPphCost )
		{
			pBest[0] = i;
		}
	}

	/* ��3��,��2��,��1�����ŵ� */
	for ( ; i < iEnd; ++ i )
	{
		if ( 0 == pCost[i].wPwCost )
			continue;

		if ( 0 == pBest[0] )
		{
			pBest[0] = i;
		}
		else if ( pCost[i].wPphCost < pCost[pBest[0]].wPphCost )
		{
			pBest[2] = pBest[1];
			pBest[1] = pBest[0];
			pBest[0] = i;
		}
		else if ( 0 == pBest[1] )
		{
			pBest[1] = i;
		}
		else if ( pCost[i].wPphCost < pCost[pBest[1]].wPphCost )
		{
			pBest[2] = pBest[1];
			pBest[1] = i;
		}
		else if ( 0 == pBest[2] )
		{
			pBest[2] = i;
		}
		else if ( pCost[i].wPphCost < pCost[pBest[2]].wPphCost )
		{
			pBest[2] = i;
		}
	}

	if ( iEnd - pBest[0] > 6 )
	{
		if ( iEnd - iBegin < 12 && pCost[pBest[1]].wPphCost > PPH_COST_GATE )
		{
			pBest[1] = pBest[2] = 0;
		}
		else if( iEnd - iBegin < 15 && pCost[pBest[2]].wPphCost > PPH_COST_GATE )
		{
			pBest[2] = 0;
		}
	}
	else if ( iEnd - pBest[1] > 6 )
	{
		if ( iEnd - iBegin < 15 && pCost[pBest[2]].wPphCost > PPH_COST_GATE )
		{
			pBest[2] = 0;
		}
	}
}

emStatic emUInt32 emCall GetSinglePPHCost(PRhythmRes pRhythmRes, emUInt8 nLen)
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pRhythmRes);

	if ( 0 == nLen || nLen > MAX_PPH_LEN + 3 )
		return 65535000UL;

	emRes_SetPos(pRhythmRes->m_subSyntax.m_pPPHRes,
		pRhythmRes->m_subSyntax.m_dwSinglePPHCost + emResSize_Int16 * (nLen - 1));
	return emRes_Get16(pRhythmRes->m_subSyntax.m_pPPHRes);
}

emStatic emUInt32 emCall GetTwoPPHCost(PRhythmRes pRhythmRes, emUInt8 nLen1, emUInt8 nLen2)
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pRhythmRes);

	if ( 0 == nLen1 || 0 == nLen2 || nLen1 > MAX_PPH_LEN || nLen2 > MAX_PPH_LEN )
		return 65535000UL;

	emRes_SetPos(pRhythmRes->m_subSyntax.m_pPPHRes, pRhythmRes->m_subSyntax.m_dwTwoPPHCost +
								emResSize_Int16 * ((nLen1 - 1) * 11 + (nLen2 - 1)));
	return TWO_PPH_COST_BASE + emRes_Get16(pRhythmRes->m_subSyntax.m_pPPHRes);
}

emInline emUInt32 emCall GetThreePPHCost(PRhythmRes pRhythmRes, emUInt8 nLen1, emUInt8 nLen2, emUInt8 nLen3)
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pRhythmRes);
	
	if ( 0 == nLen1 || 0 == nLen2 || 0 == nLen3 ||
		 nLen1 > MAX_PPH_LEN || nLen2 > MAX_PPH_LEN || nLen3 > MAX_PPH_LEN)
		return 65535000UL;

	emRes_SetPos(pRhythmRes->m_subSyntax.m_pPPHRes, pRhythmRes->m_subSyntax.m_dwThreePPHCost +
		emResSize_Int16 * ((nLen1 - 1) * (11 * 11) + (nLen2 - 1) * 11 + (nLen3 - 1)));
	return THREE_PPH_COST_BASE + emRes_Get16(pRhythmRes->m_subSyntax.m_pPPHRes);
}

/******************************************************************************
* ������        : GetPWTotalCost
* ����          : �õ�����PW�Ĵ��ۺ�
* ����          : [in]  pCost  - PW��PPH��������
*               : [in]  iBegin - ��ʼ����
*               : [in]  iEnd   - ��������
* ����          : <emUInt32>    - ���ۺ�

******************************************************************************/
emInline emUInt32 emCall GetPWTotalCost(
	PCostInfo		pCost,			/* PW��PPH�������� */
	emUInt8			iBegin,			/* ��ʼ���� */
	emUInt8			iEnd			/* �������� */
)
{
	emUInt32 nTotalCost;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pCost);

	for ( nTotalCost = 0; iBegin < iEnd; ++ iBegin )
		nTotalCost += pCost[iBegin].wPwCost;

	return nTotalCost + pCost[iEnd].wPphCost;
}

/******************************************************************************
* ������        : Decision1Or2PPH
* ����          : ���߳�1������2�����ŵ�
* ����           : [in]     pRhythmRes - ����ģ����Դ����
*                      : [in]     pCost  - PW��PPH��������
*                      : [in]     iBegin - ��ʼ����
*                      : [in]     iEnd   - ��������
*                      : [in/out] pBest  - ���ŵ�λ������

******************************************************************************/
emStatic void emCall Decision1Or2PPH(
	PRhythmRes		pRhythmRes,		/* ����ģ����Դ���� */
	PCostInfo		pCost,			/* PW��PPH�������� */
	emUInt8			iBegin,			/* ��ʼ���� */
	emUInt8			iEnd,			/* �������� */
	emUInt8 *		pBest			/* ���ŵ�λ������ */
)
{
	emUInt8 m, n;
	emUInt32 dwMinCost, dwCost;
	emUInt8 nPosA, nPosB;

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRes && pCost);

	nPosA = pBest[0];
	nPosB = pBest[1];

	/* ��û�����ŵ�ʱ��ֱ�ӷ��� */
	if ( 0 == nPosA )
	{
		pBest[0] = iEnd;
		pBest[1] = 0;
		pBest[2] = 0;
		return;
	} 

	/* ����һ��PPH����� */
	/********************************************************************
	*  0            A                                   1
	*  |____________|___________________________________|
	*
	*	�õ���1��PPH���Ϊ��01
	*********************************************************************/
	m = iEnd - iBegin;
	dwMinCost = 6553500 + GetSinglePPHCost(pRhythmRes, m);
	pBest[0] = iEnd;
	pBest[1] = 0;
	pBest[2] = 0;

	if (0 != iBegin)
		dwMinCost -= TWO_PPH_COST_BASE;

	/* ����PPH��� */
	/********************************************************************
	*  0            A                    B               1
	*  |____________|____________________|_______________|
	*
	*	�õ�������PPH���Ϊ��0A A1
	*********************************************************************/
	m = nPosA- iBegin;
	n = iEnd - nPosA;
	dwCost = 6553500 + GetTwoPPHCost(pRhythmRes, m, n);		//���󱾾�ĳ���ֵ���ɽ�11�־䣨�����£���PPH�зֽ���
	dwCost += pCost[nPosA].wPphCost;
	dwCost -= pCost[nPosA].wPwCost;
	if ( dwMinCost > dwCost ) 
	{
		dwMinCost = dwCost;
		pBest[0] = nPosA;
		pBest[1] = iEnd;
	}

	if ( 0 == pBest[1] )/* ��ֻ��һ�����ŵ�ʱ����������� */
		return;

	/* �ڶ�������PPH����� */
	/********************************************************************
	*  0            A                    B               1
	*  |____________|____________________|_______________|
	*
	*	�õ�������PPH���Ϊ��0B B1
	*********************************************************************/
	m = nPosB - iBegin;
	n = iEnd - nPosB;
	dwCost = 6553500 + GetTwoPPHCost(pRhythmRes, m, n);
	dwCost += pCost[nPosB].wPphCost;
	dwCost -= pCost[nPosB].wPwCost;
	if ( dwMinCost > dwCost )
	{
		dwMinCost = dwCost;
		pBest[0] = nPosB;
		pBest[1] = iEnd;
	}	
}

/******************************************************************************
* ������        : Decision2Or3PPH
* ����          : ���߳�2����3��PPH�ڵ�
* ����           : [in]     pRhythmRes - ����ģ����Դ����
*                      : [in]     pCost  - PW��PPH��������
*                      : [in]     iBegin - ��ʼ����
*                      : [in]     iEnd   - ��������
*                      : [in/out] pBest  - ���ŵ�λ������
* ����               : <��>

******************************************************************************/
emInline void emCall Decision2Or3PPH(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PCostInfo		pCost,			/* PW��PPH�������� */
	emUInt8		iBegin,			/* ��ʼ���� */
	emUInt8		iEnd,			/* �������� */
	emUInt8 *		pBest			/* ���ŵ�λ������ */
)
{ 
	/********************************************************************
	*  0            A                    B               C               1
	*  |____________|____________________|_______________|_______________|
	*
	*********************************************************************/
	emUInt32 dwMinCost, dwCost, dwPWTotalCost;
	emUInt8 nPPHPos[3], nPosTmp;
	emUInt8 m, n, l, i;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pRhythmRes && pCost);

	if ( 0 == pBest[1] )
	{
		Decision1Or2PPH(pRhythmRes, pCost, iBegin, iEnd, pBest);
		return;
	}

	nPPHPos[2] = pBest[2];

	/* ���ĸ����ŵ��λ������ */
	if ( pBest[0] > pBest[1] )
	{
		nPPHPos[0] = pBest[1];
		nPPHPos[1] = pBest[0];
	}
	else
	{
		nPPHPos[0] = pBest[0];
		nPPHPos[1] = pBest[1];
	}

	if ( 0 != nPPHPos[2] )
	{
		if (nPPHPos[1] > nPPHPos[2])
		{
			nPosTmp = nPPHPos[1];
			nPPHPos[1] = nPPHPos[2];
			nPPHPos[2] = nPosTmp;
		}

		if (nPPHPos[0] > nPPHPos[1])
		{
			nPosTmp = nPPHPos[0];
			nPPHPos[0] = nPPHPos[1];
			nPPHPos[1] = nPosTmp;
		}
	}
	else
	{
		nPPHPos[2] = nPPHPos[1];
	}
	/* ������� */
	
	/* ��PW���ʺ� */ 
	dwPWTotalCost = GetPWTotalCost(pCost, iBegin, iEnd);
	pBest[1] = 0;
	pBest[2] = 0;
	
	if (iEnd - iBegin <= 14)
	{
		m = iEnd - iBegin;
		dwMinCost = dwPWTotalCost + GetSinglePPHCost(pRhythmRes, m);
		pBest[0] = iEnd;
	}
	else
		dwMinCost = 6553500;
	
	/********************************************************************
	* ����ǰ ����2��PPH�����
	*  0            A                    B               C               1
	*  |____________|____________________|_______________|_______________|
	*
	*	����У� 0A A1
	*			 0B B1
	*			 0C C1
	*********************************************************************/
	/* 0? ?1 */
	for ( i = 0; i < 3; ++ i )
	{
		m = nPPHPos[i] - iBegin;
		n = iEnd - nPPHPos[i];
		dwCost = dwPWTotalCost + GetTwoPPHCost(pRhythmRes, m, n);
		dwCost += pCost[nPPHPos[i]].wPphCost;
		dwCost -= pCost[nPPHPos[i]].wPwCost;
		if(dwCost < dwMinCost) 
		{
			dwMinCost = dwCost;
			pBest[0] = nPPHPos[i];
			pBest[1] = iEnd;
		}
	}

	/********************************************************************
	*	����3��PPH�����
	*  0            A                    B               C               1
	*  |____________|____________________|_______________|_______________|
	*
	*	����У� 0A AB B1
	*			 0B	BC C1
	*			 0A AC C1
	*********************************************************************/
	/* 0A AB B1 */
	for ( i = 0; i < 3; ++ i )
	{
		emUInt8  jL, jR;
		if ( i >= 2 )
		{
			jL = nPPHPos[0];
			jR = nPPHPos[2];
		}
		else
		{
			jL = nPPHPos[i];
			jR = nPPHPos[i+1];
		}
		m = jL - iBegin;
		n = jR - jL;
		l = iEnd - jR;
		dwCost = dwPWTotalCost + GetThreePPHCost(pRhythmRes, m, n, l);
		dwCost +=  pCost[jL].wPphCost;
		dwCost -= pCost[jL].wPwCost;
		dwCost += pCost[jR].wPphCost;
		dwCost -= pCost[jR].wPwCost;
		if ( dwCost < dwMinCost )
		{
			dwMinCost = dwCost;
			pBest[0] = jL;
			pBest[1] = jR;
			pBest[2] = iEnd;
		}
	}

	if ( pCost[iEnd].wPphCost < PPH_COST_TRACE_GATE )
		return;

	if ( pCost[pBest[2]].wPphCost > pCost[pBest[1]].wPphCost )
		pBest[2] = 0;
}


/*
 *	�ӿں���
 */

void  Front_Rhythm_PPH_Init(  PRhythmRes pRhythmRes )
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pRhythmRes);

	if ( g_Res.offset_PPHC45_New  && g_Res.offset_PPHRatio_New )
	{

		g_ResPPHC45->m_iStart = g_Res.offset_PPHC45_New;
		g_ResPPHC45->m_iCurrent = g_Res.offset_PPHC45_New ;
		g_ResPPHC45->m_pResPack->m_pResParam = g_hTTS->fResFrontMain;
		g_ResPPHC45->m_pResPack->m_nSize = 0;

		g_ResPPHRatio->m_iStart = g_Res.offset_PPHRatio_New;
		g_ResPPHRatio->m_iCurrent = g_Res.offset_PPHRatio_New ;
		g_ResPPHRatio->m_pResPack->m_pResParam = g_hTTS->fResFrontMain;
		g_ResPPHRatio->m_pResPack->m_nSize = 0;


		/* ��ȡ��Դͷ */
		emRes_SetPos(g_ResPPHRatio, 32);	/* �����ļ����� */
		pRhythmRes->m_subSyntax.m_dwSinglePPHCost = emRes_Get32(g_ResPPHRatio);
		pRhythmRes->m_subSyntax.m_dwTwoPPHCost	= emRes_Get32(g_ResPPHRatio);
		pRhythmRes->m_subSyntax.m_dwThreePPHCost	= emRes_Get32(g_ResPPHRatio);

		pRhythmRes->m_subSyntax.m_pC45Res = g_ResPPHC45;
		pRhythmRes->m_subSyntax.m_pPPHRes = g_ResPPHRatio;
	}
}


/**************************************************************************************************************************/
/*************************************** PPH�߽绮�ֺ���������ʼ ******************************************************/
/**************************************************************************************************************************/

#if PPH_AFTER_DEAL					//PPH�߽绮�ֺ���


/******************************************************************************
* ����          : �������������
******************************************************************************/
emInline void emCall MoreShortPPHClear(
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	emUInt8			allNodes		/* �ı��ܳ��� */
)
{
	//���磺���ⷴӳ�˵ؼۡ����������г�Ҫ�ء��ڼ۸�ˮƽ����Э���ġ�  ->  ���ⷴӳ�˵ؼۡ��������г�Ҫ�ء��ڼ۸�ˮƽ����Э���ġ�

	emUInt8  PPHIndex[20];
	emUInt8		j = 0, nPPHLen = 0, nPPHCount,nCurIndex;
	emUInt8		curPos;				//����ʱ��λ��

	LOG_StackAddr(__FUNCTION__);

	emMemSet( PPHIndex , 0 ,20);
	for ( curPos = 1; curPos < allNodes; curPos++)
	{
		if( GetBoundaryBefore(pRhythmRT,curPos) != BdBreath )
			nPPHLen++;
		else
		{
			nPPHLen++;
			PPHIndex[j] = nPPHLen;
			nPPHLen = 0;
			j++;
		}
	}
	PPHIndex[j] = nPPHLen+1;

	nPPHCount = j+1;

	nCurIndex = 0;
	for( j = 0; j< (nPPHCount-1); j++)
	{
		nCurIndex += PPHIndex[j];
		if(    PPHIndex[j]+PPHIndex[j+1] <= 8 )
		{
			if(    PPHIndex[j+1] > 2
				&& (   (PPHIndex[j]+PPHIndex[j+1]) <= (PPHIndex[j+1]+PPHIndex[j+2])
				     || PPHIndex[j+2] == 0 )  )
			{
				SetBoundaryBefore(pRhythmRT, nCurIndex, BdPW);		//�������������: �������ϵ��ܳ���<=8
				j++;
			}
			else
				;		//����<=2�֣�������Ժ���

		}
	}
}


/******************************************************************************
* ����          : �ı��ض����͵������з�
******************************************************************************/
emInline void emCall SpecialSenChangeCost(
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	emUInt8			allNodes,		/* �ı��ܳ��� */
	emUInt8 *		PPHPos			/* PPH�߽����� */
)
{

	LOG_StackAddr(__FUNCTION__);

	//����****���򡷣����������  �������ɻ���Ч��
	if(    allNodes <= 12 
		&& pRhythmRT->m_tPoSBeforePW[0] == ChsPoS_p
		&& pRhythmRT->m_tPoSBeforePW[1] > 0
		&& pRhythmRT->m_tPoSBeforePW[allNodes-2] == ChsPoS_n
		&& GetWord(pRhythmRT->m_cText,0) == (emInt16)0xcdf9	)	//������
	{
		//�����ӣ����ϡ���****���򡷽ṹ���Ҿ䳤<=12���򱾾��PPH�߽�ȫ�����
		PPHPos[0] = allNodes;
		PPHPos[1] = 0;
		PPHPos[2] = 0;
	}

	//������λ��****�����������ɸı䣺  �������ɻ���Ч��
	if(    allNodes <= 17 &&  allNodes >= 9 
		&& pRhythmRT->m_tPoSBeforePW[0] == ChsPoS_n
		&& pRhythmRT->m_tPoSBeforePW[1] == 0 
		&& pRhythmRT->m_tPoSBeforePW[2] == ChsPoS_v 
		&& pRhythmRT->m_tPoSBeforePW[allNodes-2] == ChsPoS_f
		&& GetWord(pRhythmRT->m_cText,4) == (emInt16)0xcebb		//��λ��
		&& GetWord(pRhythmRT->m_cText,6) == (emInt16)0xd3da	)	//���ڡ�
	{
		//�����ӣ����ϡ�����λ��****�������ṹ����9=<�䳤<=17���򱾾��PPH�߽�ı�ɣ�4+��
		PPHPos[0] = 4;
		PPHPos[1] = allNodes;
		PPHPos[2] = 0;
	}

	//���ӽ�Ŀ�ĵ�****�������ɸı䣺  �������ɻ���Ч��
	if(    allNodes <= 19 &&  allNodes >= 9 
		&& pRhythmRT->m_tPoSBeforePW[0] == ChsPoS_v
		&& pRhythmRT->m_tPoSBeforePW[1] == 0 
		&& pRhythmRT->m_tPoSBeforePW[2] == ChsPoS_n 
		&& GetWord(pRhythmRT->m_cText,4) == (emInt16)0xc4bf		//��Ŀ��
		&& GetWord(pRhythmRT->m_cText,6) == (emInt16)0xb5c4		//���ġ�
		&& GetWord(pRhythmRT->m_cText,8) == (emInt16)0xb5d8	)	//���ء�
	{
		//�����ӣ����ϡ��ӽ�Ŀ�ĵ�****���ṹ����9=<�䳤<=17���򱾾��PPH�߽�ı�ɣ�5+��
		PPHPos[0] = 5;
		PPHPos[1] = allNodes;
		PPHPos[2] = 0;
	}
}			


/******************************************************************************
* ����          : �ı䱾���������ض����Լ����ش���
******************************************************************************/
emInline void emCall SpecialWordChangeCost(
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PCostInfo		pCost,			/* PW��PPH�������� */
	emUInt8			allNodes		/* �ı��ܳ��� */
)
{
	emBool      nChangeCostType;
	emInt8		tBefore,tAfter;
	emUInt8		i, nPrePos, nNextPos;

	#define	CHANGE_TYPE_m_mq		1
	#define	CHANGE_TYPE_f_v			2
	#define	CHANGE_TYPE_m_v			4
	#define	CHANGE_TYPE_f_n			5
	#define	CHANGE_TYPE_ngp_null	6

	LOG_StackAddr(__FUNCTION__);

	for(i=1;i<allNodes;i++)
	{
		nChangeCostType = 0;
		if( pCost[i].wPphCost != 0)
		{					
			for(tBefore=i-1; tBefore>=0; tBefore--)
			{
				nPrePos = pRhythmRT->m_tSylInfo[tBefore].m_tPoS;
				nNextPos = 0;
				if( nPrePos != 0)
				{
					for(tAfter=i; tAfter<=allNodes; tAfter++)
					{
						nNextPos = pRhythmRT->m_tSylInfo[tAfter].m_tPoS;					
						if( nNextPos != 0 )
							break;
					}	

					if( nPrePos == ChsPoS_m && ( nNextPos == ChsPoS_m || nNextPos == ChsPoS_q ))
					{
						nChangeCostType = CHANGE_TYPE_m_mq;						
					}
					else if( nPrePos == ChsPoS_f && nNextPos == ChsPoS_v )
					{
						nChangeCostType = CHANGE_TYPE_f_v;		
					}
					else if( nPrePos == ChsPoS_ngp )
					{						
						nChangeCostType = CHANGE_TYPE_ngp_null;	

					}
					else if( nPrePos == ChsPoS_m && nNextPos == ChsPoS_v )
					{
						nChangeCostType = CHANGE_TYPE_m_v;		
					}
					else if( pRhythmRT->m_tSylInfo[i-1].m_tPoS == ChsPoS_f && nNextPos == ChsPoS_n )
					{
						nChangeCostType = CHANGE_TYPE_f_n;		
					}

					if( nChangeCostType > 0 || nNextPos != 0)
						break;
				}				
			}
		}
		switch(nChangeCostType)
		{
			case  CHANGE_TYPE_m_mq:
				//<����>��<����>֮�䣬<����>��<����>֮�䣻���˱߽��wPphCost��󣻽��ͽ��˱߽绮��ΪPPH�ķ���
				pCost[i].wPphCost = 65535;		
				break;
			case  CHANGE_TYPE_f_v:
				//<��λ��>��<����>֮�䣬���˱߽��wPphCost��󣻽��ͽ��˱߽绮��ΪPPH�ķ���
				pCost[i].wPphCost = emMin(pCost[i].wPphCost*3,65000);	
				break;
			case  CHANGE_TYPE_ngp_null:
				//<����>��<�����>֮�䣬���˱߽��wPphCost��󣻽��ͽ��˱߽绮��ΪPPH�ķ���
				pCost[i].wPphCost = emMin(pCost[i].wPphCost*4,65000);	
				break;
			case  CHANGE_TYPE_m_v:
				//<����>��<����>֮�䣬���˱߽��wPphCost��󣻽��ͽ��˱߽绮��ΪPPH�ķ���
				pCost[i].wPphCost = emMin(pCost[i].wPphCost*6,65000);				
				break;
			case  CHANGE_TYPE_f_n:
				//<1�ַ�λ��>��<����>֮�䣬���˱߽��wPphCost��󣻽��ͽ��˱߽绮��ΪPPH�ķ���
				pCost[i].wPphCost = 65535;				
				break;
			default:
				break;
		}
	}
}




/******************************************************************************
* ������        : GetPrevAndNextWordPosLen
* ����          : ��ȡ��iReturnBest�ֽ��ǰ����صĴ��Ժʹʳ�
*
******************************************************************************/
emInline void emCall GetPrevAndNextWordPosLen(
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PWordPosLen		pWordPosLen,	/* ǰ����صĴ��Ժʹʳ���Ϣ */
	emUInt8			curPos,			/* ��ǰλ�� */
	emUInt8			allNodes		/* �ı��ܳ��� */
)
{
	emUInt8 temp1;

	LOG_StackAddr(__FUNCTION__);

	pWordPosLen->curFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos];
	pWordPosLen->curFeaLen    = pRhythmRT->m_tRhythmBeforePW[curPos]&0x0F ;
				
	if( (curPos - 1)>0 && pRhythmRT->m_tRhythmBeforePW[curPos - 1] > 0 )		//������ǰ��(1�ִ�)		
	{
		pWordPosLen->preFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-1];
		pWordPosLen->preFeaLen    = 1 ;					
		if( (curPos - 2)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 2] > 0 )			//������ǰǰ��(1�ִ�)			
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-2];
			pWordPosLen->prePreFeaLen    = 1 ;
		}
		else if( (curPos - 3)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 3] > 0 )		//������ǰǰ��(2�ִ�)	
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-3];
			pWordPosLen->prePreFeaLen    = 2 ;
		}
	}
	else if( (curPos - 2)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 2] > 0 )	//������ǰ��(2�ִ�)		
	{
		pWordPosLen->preFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-2];
		pWordPosLen->preFeaLen    = 2;
		if( (curPos - 3)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 3] > 0 )			//������ǰǰ��(1�ִ�)			
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-3];
			pWordPosLen->prePreFeaLen    = 1 ;
		}
		else if( (curPos - 4)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 4] > 0 )		//������ǰǰ��(2�ִ�)	
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-4];
			pWordPosLen->prePreFeaLen    = 2 ;
		}
	}
	else if( (curPos - 3)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 3] > 0 )	//������ǰ��(3�ִ�)		
	{
		pWordPosLen->preFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-3];
		pWordPosLen->preFeaLen    = 3;
	}

	
	temp1 = curPos + pWordPosLen->curFeaLen;
	if(    (temp1) <  allNodes
		&& pRhythmRT->m_tRhythmBeforePW[temp1] > 0 )	//���������	
	{
		pWordPosLen->nextFeaPos    = pRhythmRT->m_tPoSBeforePW[temp1];
		pWordPosLen->nextFeaLen    = pRhythmRT->m_tRhythmBeforePW[temp1]&0x0F ;
		
		temp1 = curPos + pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen;
		if(    temp1 <  allNodes
		    && pRhythmRT->m_tRhythmBeforePW[temp1] > 0 )						//����������	
		{
			pWordPosLen->nextNextFeaPos    = pRhythmRT->m_tPoSBeforePW[temp1];
			pWordPosLen->nextNextFeaLen    = pRhythmRT->m_tRhythmBeforePW[temp1]&0x0F ;
		}
	}
}




/******************************************************************************
* ����          : PPH�߽��Ƿ���Ҫǰ�ƣ����ǰ��1����
* ����          : ���º�ĵ�ǰ��PPH�ֽ��
******************************************************************************/
emInline emUInt8 PPHNeedLitterMove(  
								PRhythmRT			pRhythmRT,		/* ����ʵʱ���� */
								emUInt8				nPPHIndex,		/* ��ǰPPH�ֽ�� */
								emUInt8				nPrevPPHIndex,	/* ǰһPPH�ֽ�� */
								emUInt8				allNodes)		/* �ı��ܳ��� */
{
	emUInt8	 nNewPPHIndex=0,k;	

	TWordPosLen tWordPosLen;
	PWordPosLen pWordPosLen = &tWordPosLen;

	LOG_StackAddr(__FUNCTION__);


	emMemSet(&tWordPosLen, 0 , sizeof(tWordPosLen));

	//��ȡ��iReturnBest�ֽ��ǰ����صĴ��Ժʹʳ�
	GetPrevAndNextWordPosLen( pRhythmRT, pWordPosLen, nPPHIndex, allNodes );	

	if( (nPPHIndex-nPrevPPHIndex) >= 4)
	{
		//��ʼǰ���ж�
		if(    pWordPosLen->preFeaPos    == ChsPoS_d				//ǰ��(1�ִ�)	  ������  ���磺��d
			&& pWordPosLen->preFeaLen    == 1)				
		{	
			if(    (nPPHIndex-nPrevPPHIndex) >= 5  
				&& pWordPosLen->prePreFeaLen == 1
				&& (   pWordPosLen->prePreFeaPos == ChsPoS_v				//ǰǰ��(1�ִ�)   ������  ���磺˵v+��d
					|| pWordPosLen->prePreFeaPos == ChsPoS_d				//ǰǰ��(1�ִ�)   ������  ���磺Ҳd��d 
					|| pWordPosLen->prePreFeaPos == ChsPoS_c))				//ǰǰ��(1�ִ�)   ������  ���磺��c��d 
			{
				//ǰ�Ƶ���ǰǰ��֮ǰ
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen - pWordPosLen->prePreFeaLen;	
			}
			else
			{
				//ǰ�Ƶ���ǰ��֮ǰ
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	
			}
		}
		else if(  pWordPosLen->preFeaPos  ==  ChsPoS_v &&  pWordPosLen->preFeaLen    == 1                                  //by zcc 02-27
			   && pWordPosLen->prePreFeaPos != ChsPoS_c
			   && pWordPosLen->prePreFeaPos != ChsPoS_p
			   &&(  GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xbfc9       //��
			   ||GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xd3a6          //Ӧ
			   ||GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xd2aa ) )      //Ҫ
		{
			//ǰ���Ƕ��ʡ��ɡ���Ӧ����Ҫ�����Ҵʳ�Ϊ1

			if( (  pWordPosLen->prePreFeaPos == ChsPoS_d 
				 ||pWordPosLen->prePreFeaPos == ChsPoS_r
				 ||pWordPosLen->prePreFeaPos == ChsPoS_R )  
			  &&  pWordPosLen->prePreFeaLen    == 1 )      //ǰǰ��(1�ִ�)   �����ʻ����
			{
				//ǰ�Ƶ���ǰǰ��֮ǰ
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen - pWordPosLen->prePreFeaLen;	
			}
			else
			{
				//ǰ�Ƶ���ǰ��֮ǰ
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	
			}

			// ���磺��˵�Ŀɡ������滰��    �����鸱�����Ӧ���ڿ�ͥ������ǰ���ʹﱻ����
			// ���磺�Ҵ���û����������Ϊ�㿪��ȺҪ���ӳ���������Ĵ����˳Թ���     
			
		}
		else if(   pWordPosLen->preFeaPos    == ChsPoS_v				
			&& pWordPosLen->preFeaLen    == 1
			&& pWordPosLen->curFeaPos    == ChsPoS_v				
			&& pWordPosLen->curFeaLen    == 1)

		{
			if(    pWordPosLen->prePreFeaPos == ChsPoS_d && pWordPosLen->prePreFeaLen == 1)
			{
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen  - pWordPosLen->prePreFeaLen;	
				//���磺  �������  -��  ������� 
			}
			else
			{
				//ǰ�ʺͱ��ʶ���1�ֶ��ʣ�ǰ�Ƶ�ǰ��֮ǰ
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	
				//���磺  �͡���  -��  ���ͳ�
			}
		}
		else if(   ( pWordPosLen->preFeaPos  ==  ChsPoS_r        //by zcc 02-22
			    || pWordPosLen->preFeaPos  ==  ChsPoS_R )
				&& pWordPosLen->prePreFeaPos != ChsPoS_R
				&& pWordPosLen->prePreFeaPos != ChsPoS_r
				&& pWordPosLen->prePreFeaPos != ChsPoS_p
				&& pWordPosLen->prePreFeaPos != ChsPoS_u
				&& pWordPosLen->prePreFeaPos != ChsPoS_v)

		{
			//ǰ���Ǵ�����ǰǰ�ʲ��Ƕ��ʡ���ʡ����ʣ�ǰ�Ƶ�ǰ��֮ǰ
			nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	

			// ���磺 ��������������һֱ��ѧ��ô������˵�Բ���   �����Ϊʲô�ҡ����ǰ������ֱ�����

		}
		else if(   pWordPosLen->preFeaPos  ==  ChsPoS_c 
			    || ( pWordPosLen->preFeaPos ==  ChsPoS_p && pWordPosLen->preFeaLen > 1)
				|| (    pWordPosLen->preFeaPos    ==  ChsPoS_p 
				     && pWordPosLen->preFeaLen    == 1 
					 && pWordPosLen->prePreFeaPos != ChsPoS_p 
					 && pWordPosLen->prePreFeaPos != ChsPoS_d 
					 && pWordPosLen->prePreFeaPos != ChsPoS_v )) 
		{
			//ǰ���ǽ�ʡ����ʣ�ǰ�Ƶ�ǰ��֮ǰ

			if(   pWordPosLen->preFeaPos  ==  ChsPoS_p 
			   && pWordPosLen->preFeaLen    == 1
			   && pWordPosLen->prePreFeaPos == ChsPoS_v
			   &&GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xceaa )          //��Ϊ��				  
			{
				//���ǡ�Ϊ������ǰ��
				;
			}
			else
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	

			// ���磺��ЩС�鲢�����������ڷ�����Ƥͼ�µ�����   ���Ҳ�����Ϊ�����ǲ���������   ���������ѡ��ҷ����׷������� 

		}
		else if(  pWordPosLen->preFeaPos  ==  ChsPoS_u  &&  pWordPosLen->preFeaLen    == 1        //by zcc 02-28
		        && GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xcbf9 )        //��
	    {
			//ǰ�������ʡ�������ǰ�Ƶ�ǰ��֮ǰ
			nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	

			// ���磺�Ի����ǽ������֮������������δ�������Ψһ;�� 

		}
	}
	
	//��ʼ�����ж�
	if(  pWordPosLen->preFeaPos  ==  ChsPoS_u  &&  pWordPosLen->preFeaLen    == 1       //by zcc 02-27
	   && GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xd6ae )        //֮
	{
		//ǰ����1�����ʡ�֮��

		if( pWordPosLen->nextFeaPos  == ChsPoS_u && pWordPosLen->nextFeaLen  == 1)      //���	��1������
		{
			//���Ƶ������֮��
			nNewPPHIndex = nPPHIndex + pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen;
		}
		else if( pWordPosLen->curFeaPos  == ChsPoS_m && pWordPosLen->nextFeaPos  == ChsPoS_m )
		{
			//����
			;
		}
		else
			//���Ƶ�������֮��
			nNewPPHIndex = nPPHIndex + pWordPosLen->curFeaLen;

		// ���磺ǧ������Ц���������³�������ʦ�����񹲺͹�����ͷ֮��һ���ܶ������� 

	}

	if(  nNewPPHIndex != 0)
	{
		//������ΪPPH�ı߽�ԭ������PW�߽磬�����޸ĳ�PW�߽�
		if(  GetBoundaryBefore(pRhythmRT, nNewPPHIndex) < BdPW)			//�ֱ߽粻��PW�߽�		
		{
			if( pRhythmRT->m_tRhythmBeforePW[nNewPPHIndex]>0 )				//���ֱ߽����﷨�ʱ߽�
			{
				for( k = (nNewPPHIndex-1);k>0; k--)
				{
					if(  pRhythmRT->m_tRhythm[k]  > 0 )
					{
						pRhythmRT->m_tRhythm[nNewPPHIndex] = (pRhythmRT->m_tRhythm[k]&0x0F) - (nNewPPHIndex - k);		// (pRhythmRT->m_tRhythm[j]&0x0F)Ϊ���ɴʳ�
						pRhythmRT->m_tRhythm[k]           = nNewPPHIndex - k;									
						break;
					}
				}	
				SetBoundaryBefore(pRhythmRT, nNewPPHIndex,BdPW);
			}
			else							
				nNewPPHIndex = 0;								//�������﷨�ʱ߽磬��ǰ��Ĭ����Ч
		}	
	}

	if(  nNewPPHIndex == 0)
		nNewPPHIndex = nPPHIndex;		//ά��ԭ�ֽ��

	return nNewPPHIndex;
}




/******************************************************************************
* ����          : �ı䱾������������صĴ���
******************************************************************************/
emInline void emCall PPHFeatureWordsHandle(
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PCostInfo		pCost,			/* PW��PPH�������� */	
	emUInt8			allNodes		/* �ı��ܳ��� */
)
{
	#define	FEATURE_WORD_MAX		150			//�����ʵ�������
	

	#define PROB_MAX				3			//���ȼ������
	#define PROB_MID				2			//���ȼ����е�
	#define PROB_MIN				1			//���ȼ������
	
	#define FINAL_NO_FIND_COUNT		3			//��β�ļ����ַ�Χ��

	#define COST_NEED_ADD_RANGE		4			//�����ʱ߽�ǰ�󼸸��ֵķ�Χ�ڴ��ۼӴ�

	emUInt8	 curPos = 0;			//��ǰλ�� 

	emUInt8 i,j,k, curProb,  curSelWordIndex, nFinalIndex;

	emInt8 iReturnBest;

	emUInt8 nStartIndex=0, nEndIndex=0;
	
	emBool  bBestNeedMove;

	emInt8  nMoveZiPos;	//�ƶ����ֵ�λ��  �� < 0��ǰ��  > 0 ������

	struct FeatureWordStruct
	{
		emCharA		Word[4];					//�����ʣ�					���2�֣�ֻ��1��ʱ�ÿո���
		emUInt8		WordCount;					//�����ʵ�������			1�� �� 2��
		emUInt8		WordPos;					//�����ʵĴ��ԣ�
		emUInt8		WordStartIndex;				//�����ʿ�ʼ��Ѱ����ʼλ�ã��ڼ�����
		emUInt8		WordBestInjust;				//���ŵ�λ���������������ʼλ���ϵĵ���ֵ  ���磺1.���ȡ� ��ΪPPHβ����=1��2.�������� ��ΪPPH�ף���=0��3.��λ�ڡ� ��ΪPPHβ����=2��
		emUInt8		WordProb;					//���������ȼ�����Ϊ1����2����3���� Խ�����ȼ�Խ��
	};

	//******************************************************************************************************
	//*   ������˵����
	//*					1. ���ȼ���һ��2�ִʵ����ȼ���ΪPROB_MAX������ߣ���1�ִʵ����ȼ����ø��ݲ��������
	//* 				3. һ������£� WordStartIndex = 6 - WordBestInjust������������ǰ5���ֳ�PPH����1��PPH�ӵ�6���ֿ�ʼ
	//*   
	//*                 ���������飺  ��СΪ��12*100�ֽ�=1.2K�ֽ�
	//******************************************************************************************************
	struct FeatureWordStruct     FeatureWords[FEATURE_WORD_MAX]={		
		/*  ������  ����    ����     ��Ѱ��ʼ��  �ŵ����   ���ȼ�     */

		// 2�������ʣ�PPH�߽���ǰ
		{ "λ��",  2 ,  ChsPoS_v ,     3 ,        0 ,   PROB_MAX },		  //hyl
		{ "���",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //hyl
		{ "�ȷ�",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc  
		{ "����",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "����",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "����",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "�ο�",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "�ƺ�",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "��Ȼ",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc		
		{ "һ��",  2 ,  ChsPoS_u ,     5 ,        0 ,   PROB_MAX },		  //zcc		
		{ "�Ա�",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "�Լ�",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "���",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "���",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "��Ϊ",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc		
		{ "��Ȼ",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "�ñ�",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "��ʹ",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },       //zcc
		{ "��Ȼ",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "ԭ��",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_d ,     4 ,        0 ,   PROB_MAX },       //zcc
		{ "ֻ��",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "ֻ��",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "һ��",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "��Ȼ",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "��ʹ",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "Ʃ��",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "ƫƫ",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "Ȼ��",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "���",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },		  //zcc
		{ "��ͬ",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc	
		{ "����",  2 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "ֻ��",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc				
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "��Ȼ",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //by zcc 02-22	
		{ "����",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //by zcc 02-28

		{ "����",  2 ,  ChsPoS_n ,     5 ,        0 ,   PROB_MIN },		  //zcc		//�������ȼ���ͣ�һ����������ʲ��������ʣ��������л��кܶཫ ������  v��������� ������  n��
		{ "һ��",  2 ,  ChsPoS_a ,     5 ,        0 ,   PROB_MIN },		  //zcc
		{ "һ��",  2 ,  ChsPoS_d ,     4 ,        0 ,   PROB_MIN },		  //zcc

		


		// 1�������ʣ�PPH�߽���ǰ
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MAX },		  //hyl
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //hyl
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc 
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "ȴ  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "ͬ  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc		
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "Ҫ  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_f ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "��  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "Ϊ  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MIN },		  //by zcc 02-28
		{ "Ҳ  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //by zcc 02-28
		{ "��  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //by zcc 02-28


		// 1�������ʣ�PPH�߽��ں�		
		{ "˵  ",  1 ,  ChsPoS_v ,     4 ,        1 ,   PROB_MID },		  //hyl				
		{ "��  ",  1 ,  ChsPoS_u ,     4 ,        1 ,   PROB_MID },		  //hyl		
		{ "����",  2 ,  ChsPoS_f ,     5 ,        2 ,   PROB_MAX },		  //by zcc 02-28


		//������־
		{ "    ",  1 ,  ChsPoS_v ,     5 ,        1 ,   PROB_MID }};	
		


		/*******   ����Ϊ���ܼӵ�  *******************
		��		ChsPoS_v	 //hyl		������� �������
		��		ChsPoS_f	 //hyl		������ӣ��޸��ƣ�����Щ��仵

		����	ChsPoS_		 //zcc			�롰�ǡ���أ����м䱻�Ͽ���û�д˴�
		����	ChsPoS_ 	 //zcc			�롰�ǡ���أ����м䱻�Ͽ���û�д˴�
		����	ChsPoS_ 	 //zcc			�롰�ǡ���أ����м䱻�Ͽ���û�д˴�
		����	ChsPoS_v	 //zcc			�롰�ǡ���أ����м䱻�Ͽ���û�д˴� 
		����	ChsPoS_v	 //zcc			�롰�ǡ���أ����м䱻�Ͽ���û�д˴�
		Ҫ��	ChsPoS_c	 //zcc			�롰�ǡ���أ����м䱻�Ͽ���û�д˴�
		����	ChsPoS_v	 //zcc			��Ӻ�Ч������
		����	ChsPoS_c	 //zcc			��Ӻ�Ч������
		����	ChsPoS_p	 //zcc			��Ӻ�Ч������
		����	ChsPoS_v 	 //zcc			��Ӻ�Ч������
		����	ChsPoS_ 	 //zcc			���ϲ��㣬���ǶԵ�
		��֪	ChsPoS_ 	 //zcc			���ϲ��㣬���ǶԵ�
		����	ChsPoS_ 	 //zcc			�����л������ǡ������ڡ����������ڡ���������+�ڡ���				ռ30%
		�Ǿ�	ChsPoS_ 	 //zcc			�����л������ǡ���+��+�ǡ�������+�͡�										ռ4%
		�ǹ�	ChsPoS_ 	 //zcc			�����л������ǡ���+���¡�������+���ϡ�									ռ15%
		����	ChsPoS_ 	 //zcc			�����л������ǡ���+�������ȣ������Ǵʡ����䡱
		��ͬ	ChsPoS_ 	 //zcc			�����л������ǡ���+ͬѧ���ȣ������Ǵʡ���ͬ��			
		��		ChsPoS_ 	 //zcc			�����л������ǡ�һ�¡��������¡�
		��		ChsPoS_ 	 //zcc			�����л������ǡ���Ϊ�������Ժ�
		��		ChsPoS_ 	 //zcc			���ϲ��㣬�������ǡ����ԡ�
		��		ChsPoS_ 	 //zcc			���ϲ��㣬�������ǡ�������
		˵��	ChsPoS_ 	 //zcc			���м䱻�Ͽ���û�д˴�
		����	ChsPoS_ 	 //zcc			��Ӻ�Ч������
		�ղ�	ChsPoS_t 	 //hyl			���ʺ���������
		֮��	ChsPoS_t 	 //hyl			���ʺ���������
		Ŀǰ	ChsPoS_t 	 //hyl			���ʺ���������
		�Ժ�	ChsPoS_f 	 //hyl			���ʺ���������
		��ǰ	ChsPoS_f 	 //hyl			���ʺ���������
		��			 	 	 //hyl			���Ժ��ӣ��Ҳ��ʺ��������ʣ�
		��		ChsPoS_c 	 //hyl			�ʺ��������ʣ��������д���ǡ���Ϊ p�� ����Ϊ c��  
		**********************************************/



	LOG_StackAddr(__FUNCTION__);

	//����PPH�����ʣ������ı�pCost[i].wPphCost��ֵ����PPH�����ʵı߽��wPphCost��С������1000������߽��˱߽绮��ΪPPH�ĸ��ʣ�	
	iReturnBest = 0;
	curProb = 0;
	curSelWordIndex = 0;
	

	//���ı���β�����FINAL_NO_FIND_COUNT�������ڲ�����������
	nFinalIndex = allNodes;
	if( (allNodes - nFinalIndex ) <= FINAL_NO_FIND_COUNT )				
	{
		if(  (allNodes - FINAL_NO_FIND_COUNT) > 0 )
			nFinalIndex = allNodes - FINAL_NO_FIND_COUNT;	
		else
			nFinalIndex = 0;
	}

	for(i=curPos;i<nFinalIndex;i++)						//ѭ����Ѱ��ǰ�ı�
	{
		for(j = 0; j < FEATURE_WORD_MAX; j++ )		//ѭ����Ѱ������
		{
			if( FeatureWords[j].Word[0] ==  0 )		
				break;								//���1�������ʣ�����

			//������Ѱ��ʼλ��( ��ǰλ�ô��ڱ������ʵ���ʼλ���Ҵ������������С��ʼλ��)
			if( FeatureWords[j].WordStartIndex <= (i-curPos+1)  )	
			{
				//��������ƥ���ϣ��Ҵ���ƥ����,�Ҵʵ���������
				if(  emMemCmp( pRhythmRT->m_cText+i*2 , FeatureWords[j].Word, FeatureWords[j].WordCount*2)==0	//ƥ�䣺������
				  && pRhythmRT->m_tPoSBeforePW[i] == FeatureWords[j].WordPos 									//ƥ�䣺���ԣ���PWƴ��ǰ��ԭʼ���ԱȽϣ�
				  && (pRhythmRT->m_tRhythmBeforePW[i]&0x0F) == FeatureWords[j].WordCount)						//ƥ�䣺�ʵ���������PWƴ��ǰ��ԭʼRhythm�Ƚϣ�
				{				

					iReturnBest = i;
					curProb = FeatureWords[j].WordProb;
					curSelWordIndex = j;

					if( iReturnBest != 0 )
					{

				//*********************************************************************************************************
				//*********************************** �жϣ����ŵ��Ƿ���Ҫ�ڱ������ʵĻ������ƶ�����ʼ ********************
				//*********************************************************************************************************

						TWordPosLen tWordPosLen;
						PWordPosLen pWordPosLen = &tWordPosLen;
						emMemSet(&tWordPosLen, 0 , sizeof(tWordPosLen));

						//��ȡ��iReturnBest�ֽ��ǰ����صĴ��Ժʹʳ�
						GetPrevAndNextWordPosLen( pRhythmRT, pWordPosLen, iReturnBest, allNodes );		

						bBestNeedMove = emFalse;		//��ʼ�����������ŵ㲻��Ҫ�ڱ������ʵĻ������ƶ�
						nMoveZiPos = 0;

						//Ŀǰ���ŵ����λ������Ӧ����
						if(  (iReturnBest + FeatureWords[curSelWordIndex].WordBestInjust - curPos) >= 4	)	
						{	
							
							//************************* �жϣ���������:����	  ǰ��:����  �ŵ����ֵ=0  ***************************
							//****************************************************************************************************
							if(    FeatureWords[curSelWordIndex].WordBestInjust == 0				//�ŵ����ֵ=0      ������
								&& pWordPosLen->curFeaPos == ChsPoS_v											//�������ʣ�		����
								&& (    pWordPosLen->preFeaPos == ChsPoS_d										//ǰ�ʣ�			����
									 || pWordPosLen->preFeaPos == ChsPoS_v 										//					����	��������
									 || pWordPosLen->preFeaPos == ChsPoS_r 										//					����	
									 || pWordPosLen->preFeaPos == ChsPoS_R 										//					�˳ƴ���
									 || pWordPosLen->preFeaPos == ChsPoS_c 										//					����
									 || pWordPosLen->preFeaPos == ChsPoS_t ))                                 //by zcc 02-22
									 //|| pWordPosLen->preFeaPos == ChsPoS_a ))									//ǰ�ʣ�			���ݴ�														
							{
								if( pWordPosLen->preFeaPos == ChsPoS_v )	//��������
								{
									
									if(    GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xcac7	 //���ǡ�
										|| GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd2aa	 //��Ҫ��
										|| GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xc8c3) //���á�
									{
										if(    pWordPosLen->prePreFeaPos ==  ChsPoS_R 
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_r
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_c    //by zcc 02-22
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_p    //by zcc 02-22  ���磺��������������Ϊ��һ�����صĵ���
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_d)
										{
											//�Ƶ���ǰǰ��֮ǰ
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen;	
										}
										else
										{
											//�Ƶ���ǰ��֮ǰ
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen;	
										}

										//���磺 ��ת�����������������   ���񿴵��ӻ�����Ҹо���ʹ��һ��   ����˵�������Ҫһ���������
									}	

								}
								else if(    pWordPosLen->preFeaPos ==  ChsPoS_t )          //by zcc 02-22					
								{
									//ǰ����ʱ��ʣ���������
									if(	GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xcac7    //��
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_p
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_t
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_v  )
									{										
										//�Ƶ���ǰ��֮ǰ
										bBestNeedMove = emTrue;
										nMoveZiPos -= pWordPosLen->preFeaLen;	

										//���磺�����˾��ڵľ�������������˱�֮�����ĳ���

									}
									else
										bBestNeedMove = emFalse;	//��ǰ��
								}
								else if(    pWordPosLen->prePreFeaPos == ChsPoS_u 
										 && pWordPosLen->prePreFeaLen == 1 
										 && pWordPosLen->preFeaPos != ChsPoS_d)
								{					
									//ǰǰ��:1�����ʣ� ���ŵ㲻�ƶ�
									bBestNeedMove = emFalse;	

									//���䣺�����˵�Ŭ���ñ����˶����ϵĽ�����
								}
								else if(   pWordPosLen->prePreFeaPos == ChsPoS_d 
										|| pWordPosLen->prePreFeaPos == ChsPoS_c 
										|| pWordPosLen->prePreFeaPos == ChsPoS_p)	          						
								{
									//ǰǰ��:���ʣ����ʣ����
									if(  (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen) > 2
									   && pWordPosLen->preFeaLen == 1)
									{
										//ǰ��+ǰǰ�ʣ��ʳ�֮�� >2 : �����ŵ���Ƶ���������֮��
										bBestNeedMove = emTrue;	
										nMoveZiPos += pWordPosLen->curFeaLen;								
									}
									else if(  (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen) > 2    //by zcc 02-22
										    && pWordPosLen->preFeaLen > 1)
									{
										//ǰ��+ǰǰ�ʣ��ʳ�֮�� >2 : �����ŵ㲻�Ƶ�
										bBestNeedMove = emFalse;								
									   
										//���磺Ϊʲôƫƫ�ڴ�ʱҪ�������Ͷ�ʱ��

									}
									else
									{
										//ǰ��+ǰǰ�ʣ��ʳ�֮��<=2 : �����ŵ���Ƶ���ǰǰ��֮ǰ
										bBestNeedMove = emTrue;	
										nMoveZiPos -= (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen);				
									}
								}
								else										
								{
									//ǰǰ��:���Ǹ������ʽ�ʣ����ŵ���Ƶ���ǰ��֮ǰ
									bBestNeedMove = emTrue;	
									nMoveZiPos -= pWordPosLen->preFeaLen;		
								}				
							}

							//*********** �жϣ���������:����|����|���	  ǰ��(1�ִ�):����|����|����|����|�˳ƴ���|��� **********
							//****************************************** �ŵ����ֵ=0 ********************************************
							else if( FeatureWords[curSelWordIndex].WordBestInjust == 0				//�ŵ����ֵ=0      ������   
								&&  (pWordPosLen->curFeaPos ==  ChsPoS_c										//�������ʣ�		����
									  || pWordPosLen->curFeaPos ==  ChsPoS_d									//					����				
									  || pWordPosLen->curFeaPos ==  ChsPoS_p )									//					���				
								&&  (    (pWordPosLen->preFeaPos == ChsPoS_c && pWordPosLen->preFeaLen==1)		//ǰ�ʣ�			����
									  || (pWordPosLen->preFeaPos == ChsPoS_d )									//					����
									  || (pWordPosLen->preFeaPos == ChsPoS_v )									//					����	����������
									  || (pWordPosLen->preFeaPos == ChsPoS_r )									//					����	
									  || (pWordPosLen->preFeaPos == ChsPoS_R )									//					�˳ƴ���
									  || (pWordPosLen->preFeaPos == ChsPoS_m )									//					����
									  || (pWordPosLen->preFeaPos == ChsPoS_t )                                  //by zcc 02-22
									  || (pWordPosLen->preFeaPos == ChsPoS_p && pWordPosLen->preFeaLen==1)))	//					���		
							{	
								if(    pWordPosLen->preFeaPos ==  ChsPoS_m )					
								{
									//ǰ�������ʣ���������
									if(	   pWordPosLen->curFeaPos ==  ChsPoS_p 
										&& pWordPosLen->curFeaLen <= 2
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_m
										&& GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd6c1 )	 //������											
									{										
										//�Ƶ���ǰ��֮ǰ
										bBestNeedMove = emTrue;
										nMoveZiPos -= pWordPosLen->preFeaLen;	

										//���磺 ������ֲ�������ʮ�����������ܳɻ�
									}
									else
										bBestNeedMove = emFalse;	//��ǰ��
								}
								else if(    pWordPosLen->preFeaPos ==  ChsPoS_t )          //by zcc 02-22					
								{
									//ǰ����ʱ��ʣ���������
									if(	   GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xbbb9 	 //������											
									    && pWordPosLen->prePreFeaPos !=  ChsPoS_p
									    && pWordPosLen->prePreFeaPos !=  ChsPoS_t
									    && pWordPosLen->prePreFeaPos !=  ChsPoS_v  )
									{										
										//�Ƶ���ǰ��֮ǰ
										bBestNeedMove = emTrue;
										nMoveZiPos -= pWordPosLen->preFeaLen;	

										//���磺 ���ô����Ŀǰ����û���ܹ��������˹�����Ʊ���ֻ

									}
									else
										bBestNeedMove = emFalse;	//��ǰ��
								}
								else if(    pWordPosLen->preFeaPos ==  ChsPoS_v )					
								{
									//ǰ���Ƕ��ʣ���������
									if(	   pWordPosLen->curFeaPos ==  ChsPoS_p 
										&& (    GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd6c1	 //������
											 || GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xb0d1	 //���ѡ�
											 || GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xcdf9	 //������
											 || GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd4da))	 //���ڡ�
									{
										if(    pWordPosLen->prePreFeaPos ==  ChsPoS_R 
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_r
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_d)
										{
											//�Ƶ���ǰǰ��֮ǰ
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen;	
										}
										else if( pWordPosLen->prePreFeaPos !=  ChsPoS_v)
										{
											//�Ƶ���ǰ��֮ǰ
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen;	
										}
										else
										{
											bBestNeedMove = emFalse;	//��ǰ��
										}

										//���磺 ����ҹ�ֿɽ���������һ�ٰ�ʮ�����϶�  ��������ҷ����ڴ��ϴ��   �����Ǿ���Ҫ������������Ϊһ�����е��µ�
									}
									else if(	pWordPosLen->curFeaPos ==  ChsPoS_p && pWordPosLen->curFeaLen == 1          //by zcc 02-28
											    &&  GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xceaa )	 //��Ϊ��
									{
										//�Ƶ�������֮��
										bBestNeedMove = emTrue;
										nMoveZiPos += pWordPosLen->curFeaLen;	
									}
									else
										bBestNeedMove = emFalse;	//��ǰ��
								}
								else if( pWordPosLen->prePreFeaPos == ChsPoS_v &&
									( pWordPosLen->preFeaPos == ChsPoS_r ||
									  pWordPosLen->preFeaPos == ChsPoS_R ))
								{
									//ǰǰ�ʣ����ʣ�ǰ�ʣ�1�ִ��ʣ� ���ŵ㲻�ƶ�
									bBestNeedMove = emFalse;	

									//���磺 �ҵĲ��֡���λʮ�����͡��ҵ������������ĳ�Ƭ��˾����
								}
								else if(   pWordPosLen->prePreFeaPos == ChsPoS_u 
										&& pWordPosLen->prePreFeaLen==1 
										&& pWordPosLen->preFeaPos != ChsPoS_d
										&& GetWord(pRhythmRT->m_cText,iReturnBest*2-4) != (emInt16)0xc1cb)	//���ˡ�
									 	
								{					
									//ǰǰ��:1�����ʣ� ���ŵ㲻�ƶ�
									bBestNeedMove = emFalse;	

									//���䣺������ի־����д��Ȼ������ڤ����֮��
									//���䣺�ģ�֮��
								}
								else if(   (   pWordPosLen->prePreFeaPos == ChsPoS_d 
											|| pWordPosLen->prePreFeaPos == ChsPoS_c 
											|| pWordPosLen->prePreFeaPos == ChsPoS_p) 
										 && pWordPosLen->prePreFeaLen==1 )
										 //&& (pWordPosLen->prePreFeaLen+pWordPosLen->preFeaLen)<= 2) //by zcc 02-22	
								{
									//ǰǰ��(1�ִ�):�������ʽ�ʣ����ŵ��ƶ���ǰǰ��֮ǰ
									bBestNeedMove = emTrue;
									nMoveZiPos -= (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen);			
								}
								else
								{
									//������������ŵ��ƶ���ǰ��֮ǰ
									bBestNeedMove = emTrue;
									nMoveZiPos -= pWordPosLen->preFeaLen;
								}				
							}
							//************************* �жϣ���������:��λ��	  ǰ��:����  �ŵ����ֵ=0  ***************************
							//****************************************************************************************************
							else if(    FeatureWords[curSelWordIndex].WordBestInjust == 0				//�ŵ����ֵ=0      ������
								&& pWordPosLen->curFeaPos == ChsPoS_f)											//�������ʣ�		��λ��
							{
								if(  pWordPosLen->nextFeaPos == ChsPoS_v)					//��ʣ�	����
								{
									//���ŵ㲻�ƶ�
									bBestNeedMove = emFalse;
								}
								else
								{
									bBestNeedMove = emFalse;
									iReturnBest = 0;					//���ı����ŵ㣬����ԭ�������ŵ�
								}
							}
							//************************* �жϣ���������:����	  ���:���� ����  �ŵ����ֵ>0 ***********************
							//****************************************************************************************************
							else if(    FeatureWords[curSelWordIndex].WordBestInjust > 0			//�ŵ����ֵ>0      ����
									 && pWordPosLen->curFeaPos == ChsPoS_v)										//�������ʣ�		����														
							{
								if(     pWordPosLen->nextFeaPos == ChsPoS_d					//��ʣ�	����
									 || pWordPosLen->nextFeaPos == ChsPoS_u	 				//			����		
									 || pWordPosLen->nextFeaPos == ChsPoS_v					//			����
									 || pWordPosLen->preFeaPos  == ChsPoS_v					//ǰ�ʣ�	����
									 || pWordPosLen->preFeaPos  == ChsPoS_d	 )				//			����					
								{
									bBestNeedMove = emFalse;
									iReturnBest = 0;					//���ı����ŵ㣬����ԭ�������ŵ�
								}
								else
								{
									//���ŵ㲻�ƶ�
									bBestNeedMove = emFalse;
								}
							}
							else if(    FeatureWords[curSelWordIndex].WordBestInjust > 0			//�ŵ����ֵ>0      ����
									 && pWordPosLen->curFeaPos == ChsPoS_u)							//�������ʣ�		����														
							{
								if(    GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xb5c8	 //���ȡ�		
									&& pWordPosLen->nextFeaPos     == ChsPoS_n
									&& pWordPosLen->nextNextFeaPos != ChsPoS_n)
								{
									if( pWordPosLen->nextNextFeaPos ==  ChsPoS_u)
									{
										//�Ƶ�������֮��
										bBestNeedMove = emTrue;
										nMoveZiPos += pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen + pWordPosLen->nextNextFeaLen;	
										
										//���磺 �����ܶ�������������ȡ��˵Ļ��������¡��ٿ�����
										
									}
									else
									{
										//�Ƶ������֮��
										bBestNeedMove = emTrue;
										nMoveZiPos += pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen;	
										
										//���磺 ���������ȡ����Ʋ������ĸ���֮�� 
									}
								}
								else
								{
									//���ŵ㲻�ƶ�
									bBestNeedMove = emFalse;
								}
							}


							//�����ƶ�
							if( bBestNeedMove == emTrue)
								iReturnBest += nMoveZiPos;
						}
				//*********************************************************************************************************
				//*********************************** �жϣ����ŵ��Ƿ���Ҫ�ڱ������ʵĻ������ƶ������� ********************
				//*********************************************************************************************************


						if( bBestNeedMove == emFalse && iReturnBest>0)		//�����ŵ㲻��Ҫ�ڱ������ʵĻ������ƶ��������ñ���������ص����ŵ�
						{			
							if(    (iReturnBest + FeatureWords[curSelWordIndex].WordBestInjust) <= 3 		//ר��Ϊ�����ʣ���λ�ڡ�
								&& GetWord(pRhythmRT->m_cText,iReturnBest*2)   == (emInt16)0xcebb		//��λ��
								&& GetWord(pRhythmRT->m_cText,iReturnBest*2+2) == (emInt16)0xd3da )		//���ڡ�
							{
								iReturnBest += FeatureWords[curSelWordIndex].WordCount;			//���ŵ�λ�õ�����ǿ�е����������ʺ��棨�����ŵ��ھ��׵ļ������ڣ�
							}
							else
								iReturnBest += FeatureWords[curSelWordIndex].WordBestInjust;	//���ŵ�λ�õ������������������ٽ���PPH�߽�	
						}

						//�����ı�����ŵ㲻���ѻ��ֺõ�PPH�߽磬�������ı����ŵ㣬���򲻸ı�
						if( iReturnBest >0 )
						{
							//������ΪPPH�ı߽�(���ı������ȵ�)ԭ������PW�߽磬�����޸ĳ�PW�߽�
							if( GetBoundaryBefore(pRhythmRT, iReturnBest) < BdPW)				//�ֱ߽粻��PW�߽�		
							{
								if( pRhythmRT->m_tRhythmBeforePW[iReturnBest]>0 )				//���ֱ߽����﷨�ʱ߽�
								{
									for( k = (iReturnBest-1);k>0; k--)
									{
										if(  pRhythmRT->m_tRhythm[k]  > 0 )
										{
											pRhythmRT->m_tRhythm[iReturnBest] = (pRhythmRT->m_tRhythm[k]&0x0F) - (iReturnBest - k);		// (pRhythmRT->m_tRhythm[j]&0x0F)Ϊ���ɴʳ�
											pRhythmRT->m_tRhythm[k]           = iReturnBest - k;									
											break;
										}
									}	
									SetBoundaryBefore(pRhythmRT, iReturnBest,BdPW);
								}
								else							
									iReturnBest = 0;								//�������﷨�ʱ߽磬���ҵ���������Ĭ����Ч
							}	
							
							//�ı�߽���ۣ��������ֵ����
							if( iReturnBest != 0 )
							{								
								nStartIndex=0;
								nEndIndex=0;

								//�ı����ŵ���ۣ����ݲ�ͬ�����ȼ������в�ͬ�ĸı� ��ԭ�������ǲ������ȼ���ͳһ��4����
								if( curProb== PROB_MAX )
									pCost[iReturnBest].wPphCost /= 6;
								else if( curProb== PROB_MID )
									pCost[iReturnBest].wPphCost /= 4;
								else if( curProb== PROB_MIN )
									pCost[iReturnBest].wPphCost /= 2;


								//ǰ��ʴ��ۣ�����3��(ǰ��2���ּ�����)
								if( (iReturnBest-1)>0 && pCost[iReturnBest-1].wPphCost > 0 )
									pCost[iReturnBest-1].wPphCost = emMin(pCost[iReturnBest-1].wPphCost*3,65000);
								if( (iReturnBest-2)>0 && pCost[iReturnBest-2].wPphCost > 0 )
									pCost[iReturnBest-2].wPphCost = emMin(pCost[iReturnBest-2].wPphCost*3,65000);	
								if((iReturnBest+1)<allNodes &&  pCost[iReturnBest+1].wPphCost > 0 )
									pCost[iReturnBest+1].wPphCost = emMin(pCost[iReturnBest+1].wPphCost*3,65000);
								if( (iReturnBest+2)<allNodes && pCost[iReturnBest+2].wPphCost > 0 )
									pCost[iReturnBest+2].wPphCost = emMin(pCost[iReturnBest+2].wPphCost*3,65000);
								
							}
						}
					}

					iReturnBest = 0;
					i += 2;
					break;
				}
			}
		}
	}
}


#endif			//end: PPH_AFTER_DEAL

/**************************************************************************************************************************/
/*************************************** PPH�߽绮�ֺ����������� ******************************************************/
/**************************************************************************************************************************/


/******************************************************************************
* ������        : Front_Rhythm_PPH  ����������
* ����          : ����PPH
* ����           : [in]     pRhythmRes   - ����ģ����Դ����
*                      : [in/out] pRhythmRT - ����ʵʱ����
* ����               : <��>

******************************************************************************/
void  Front_Rhythm_PPH( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT		/* ����ʵʱ���� */
)
{
	PCostInfo		pCost;			/* ��������ָ�룬��̬�����ڴ� */	

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRes && pRhythmRT);

	if ( pRhythmRes->m_subSyntax.m_pC45Res && pRhythmRes->m_subSyntax.m_pPPHRes )
	{
		/* �����ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
		pCost = (PCostInfo)emHeap_AllocZero(sizeof(TCostInfo) * CE_MAX_SYLLABLE, "PPH���ۣ�������ģ��  ��");
#else
		pCost = (PCostInfo)emHeap_AllocZero(sizeof(TCostInfo) * CE_MAX_SYLLABLE);
#endif

		emAssert(pCost);

		/* �õ��������ڵ�PW��PPH���� */
		if ( CalcSplitCost(pRhythmRes, pRhythmRT, pCost) )
		{
			emUInt8		allNodes;			/* �������ڸ��� */
			emUInt8		curPos;				/* ����ʱ��λ�� */

			allNodes = GetSylCount(pRhythmRT);

#if PPH_AFTER_DEAL							//PPH�߽绮�ֺ���

			//�ı䱾���������ض����Լ����ش���
			SpecialWordChangeCost(pRhythmRT, pCost,allNodes);			

			//�ı䱾������������صĴ���
			PPHFeatureWordsHandle(pRhythmRT, pCost,allNodes);
#endif					

			for ( curPos = 0; curPos < allNodes; )
			{
				emUInt8	PPHPos[3];			/* PPH�зֵ�λ�� */
				emUInt8	tempPos;
				emUInt8		j;								

				/* ȡ6������16�����ڵ�һ�����ŵ� */
				tempPos = FindCurSnap(pRhythmRT, pCost, curPos);

				//��ʼ����PPHPos�߽�����
				PPHPos[0] = PPHPos[1] = PPHPos[2] = 0;

				/* ȡ1~4���ڵ����ŵ� */
				GetOtherGandidate(pCost, curPos, tempPos, PPHPos);

				if ((tempPos - curPos) > 11)
				{/* �������11�ֵ����,�е���Ҫ����3�� */
					Decision2Or3PPH(pRhythmRes, pCost, curPos, tempPos, PPHPos);
				}
				else
				{/* �������Ǵ���С��11�ֵ����, �����е��ŵ���Ϊ1��0����� */
					Decision1Or2PPH(pRhythmRes, pCost, curPos, tempPos, PPHPos);
				}

#if PPH_AFTER_DEAL			//PPH�߽绮�ֺ���

				//�ı��ض����͵������з�		���磺������λ��****��������
				SpecialSenChangeCost(pRhythmRT, allNodes, PPHPos);	
#endif

				/* ����PPHPos�߽���������ս��������PPH�߽� */
				for ( j = 0; j < 3; ++ j )
				{
					if ( 0 == PPHPos[j] )
						break;

					if ( PPHPos[j] < allNodes )
					{	

#if PPH_AFTER_DEAL								//PPH�߽绮�ֺ���
						emUInt8 nPrevPPHIndex;
						if( j > 0 )
							nPrevPPHIndex = PPHPos[j-1];
						else
							nPrevPPHIndex = curPos;

						// �������PPH�߽��Ƿ���Ҫ�����ƶ�������ƶ�2����
						//��PPH�߽�ǰ����Щ���ԣ�d,c,p,r,u�������ܷ����ƶ�
						PPHPos[j] = PPHNeedLitterMove(pRhythmRT, PPHPos[j], nPrevPPHIndex,allNodes);						
#endif
						//����PPH�߽�
						SetBoundaryBefore(pRhythmRT, PPHPos[j], BdBreath);
					}
				}
				/* �����һ��PPHλ���ٴ������� */
				curPos = PPHPos[-- j];
			}


#if PPH_AFTER_DEAL					//PPH�߽绮�ֺ���

			//�����������		���磺���ⷴӳ�˵ؼۡ����������г�Ҫ�ء��ڼ۸�ˮƽ����Э���ġ�  ->  ���ⷴӳ�˵ؼۡ��������г�Ҫ�ء��ڼ۸�ˮƽ����Э���ġ�
			MoreShortPPHClear(pRhythmRT, allNodes);			
#endif

		}				
		

		/* �ͷ��ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pCost, sizeof(TCostInfo) * CE_MAX_SYLLABLE, "PPH���ۣ�������ģ��  ��");
#else
		emHeap_Free(pCost, sizeof(TCostInfo) * CE_MAX_SYLLABLE);
#endif
	}
}

