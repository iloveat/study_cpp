
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


/* PW属性索引 */
#define PwAttr_CurPoS		0	/* 当前词性 */
#define PwAttr_CurLen		1	/* 当前词长度 */
#define PwAttr_CurPwLen		2	/* 当前PW长度 */
#define PwAttr_CurCost		3	/* 当前词代价 */

#define PwAttr_PrevPoS		4	/* 前词性 */
#define PwAttr_PrevLen		5	/* 前词长度 */
#define PwAttr_PrevPwLen	6	/* 前PW长度 */
#define PwAttr_PrevCost		7	/* 前词代价 */

#define PwAttr_NextPoS		8	/* 后词性 */
#define PwAttr_NextLen		9	/* 后词长度 */
#define PwAttr_NextPwLen	10	/* 后PW长度 */

#define PwAttr_PrevPrevPoS	11	/* 前前词性 */
#define PwAttr_PrevPrevLen	12	/* 前前词长度 */

#define PwAttr_NextNextPoS	13	/* 后后词性 */
#define PwAttr_NextNextLen	14	/* 后后词长度 */


/* PW属性数 */
#define PwAttr_Count	15

/* PW属性类型 */
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
	
	emUInt8   prePreFeaPos;		//前前词    ：词性
	emUInt8   preFeaPos;		//前词      ：词性
	emUInt8   curFeaPos;		//当前特征词：词性
	emUInt8   nextFeaPos;		//后词      ：词性
	emUInt8   nextNextFeaPos;	//后后词    ：词性

	emUInt8   prePreFeaLen;		//前前词    ：词长
	emUInt8   preFeaLen;		//前词      ：词长
	emUInt8   curFeaLen;		//当前特征词：词长	
	emUInt8   nextFeaLen;		//后词      ：词长
	emUInt8   nextNextFeaLen;	//后后词    ：词长
};



/******************************************************************************
* 函数名        : GetPoSVal
* 描述          : 将词性转成决策树需要的离散值
* 参数          : [in] tPoS - 词性
* 返回          : <emUInt8> - 离散值

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
* 函数名        : AskPphPwCost
* 描述          : 获得PPH和PW节点代价
* 参数          : [in]  pRhythmRes   - 韵律模块资源对象
*               : [in]  tPwAttrs - PW属性
*               : [out] pCost    - PW和PPH的代价
* 返回          : <无>

******************************************************************************/
emStatic void emCall AskPphPwCost(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	TPwSerial		tPwAttrs,			/* PW属性 */
	PCostInfo		pCost				/* PW和PPH的代价 */
)
{
	emAddress nPos;

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
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
* 函数名        : CalcSplitCost
* 描述          : 计算PPH划分代价
* 参数          : [in]  pRhythmRes   - 韵律模块资源对象
*               : [in]  pRhythmRT - 韵律实时对象
*               : [out] pCost    - PW和PPH代价数组
* 返回          : <emBool>       - 是否成功

******************************************************************************/
emInline emBool emCall CalcSplitCost(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PCostInfo		pCost			/* PW和PPH代价数组 */
)
{
	TPwSerial tPwAttrs;
	emUInt8 n, i, iNext;

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pRhythmRes && pRhythmRT && pCost);

	/* 获取音节数 */
	n = GetSylCount(pRhythmRT);
	if ( 0 == n )
		return emFalse;	

	/* 获取第2个词的音节索引 */
	iNext = GetNextWord(pRhythmRT, 0);
	if ( iNext >= n )
		return emFalse;	

	/* 初始化数据 */
	emMemSet(pCost, 0,sizeof(TCostInfo) * n);

	/* 加载第1个词的属性到当前词属性 */ 
	tPwAttrs[PwAttr_CurPoS] = GetPoSVal(GetPoS(pRhythmRT, 0));
	tPwAttrs[PwAttr_CurLen] = GetWordLen(pRhythmRT, 0);
	if (tPwAttrs[PwAttr_CurLen] > 7)
		tPwAttrs[PwAttr_CurLen] = 7;
	tPwAttrs[PwAttr_CurPwLen] = GetPWLen(pRhythmRT, 0);
	tPwAttrs[PwAttr_CurCost] = GetFrCost(pRhythmRT, 0);

	/* 加载后词的属性 */ 
	tPwAttrs[PwAttr_NextPoS] = GetPoSVal(GetPoS(pRhythmRT, iNext));
	tPwAttrs[PwAttr_NextLen] = GetWordLen(pRhythmRT, iNext);
	if (tPwAttrs[PwAttr_NextLen] > 7)
		tPwAttrs[PwAttr_NextLen] = 7;

	/* 前词的属性置为空 */
	tPwAttrs[PwAttr_PrevPoS] = 0;
	tPwAttrs[PwAttr_PrevLen] = 0;

	/* 获取第3个词的音节索引 */
	iNext = GetNextWord(pRhythmRT, iNext);

	/* 加载第3个词的属性到后后词属性 */
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
		/* 交换属性 */
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

		/* 获取后后词的音节索引 */
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

		/* 如果不是边界，不能划分PPH */
		if ( GetBoundaryBefore(pRhythmRT, i) < BdPW || TestFrontFlag(pRhythmRT, i, FrontFlag_NotPPH) )
		{
			/* 设置代价为最大 */
			pCost[i].wPwCost = 1;
			pCost[i].wPphCost = EM_USHORT_MAX;
		}
		else
		{
			emUInt8 iNextPW;
			emPCUInt8 pText;

			/* 加载前PW长度 */ 
			tPwAttrs[PwAttr_CurPwLen] = GetPWLen(pRhythmRT, GetPrevPW(pRhythmRT, i));

			/* 加载后韵律词长 */ 
			iNextPW = GetNextPW(pRhythmRT, i);
			tPwAttrs[PwAttr_NextPwLen] = iNextPW >= n ? 0 : GetPWLen(pRhythmRT, iNextPW);

			/* 获取当前词的文本 */
			pText = (emPCUInt8)GetText(pRhythmRT, i);

			/* 使用决策树得到PW和PPH的代价 */
			AskPphPwCost(pRhythmRes, tPwAttrs, pCost + i);
			if ( 0 == pCost[i].wPwCost )
			pCost[i].wPwCost = 1;
		}
	}

	/* 最后一个音节肯定是PPH，所以设置其PW代价为最大 */
	pCost[i].wPwCost = EM_USHORT_MAX;

	return emTrue;
}

/******************************************************************************
* 函数名        : FindCurSnap
* 描述          : 寻找当前节点以后6字以上,16字以内的一个最优断点 
* 参数          : [in] pRhythmRT - 韵律实时对象
*               : [in] pCost    - PW和PPH代价数组
*               : [in] iPos     - 当前位置
* 返回          : <emUInt8>  - 最优点位置

******************************************************************************/
emInline emUInt8 emCall FindCurSnap(
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PCostInfo		pCost,			/* PW和PPH代价数组 */
	emUInt8		iPos				/* 当前位置 */
)
{
	emStatic emConst emUInt16 PPHProbWeight[] =
	{
		65535,	65535,	65535,	65535,	14692, 8576,	/* <6 */  //hyl 2012-02-23 原来为 65535,	65535,	65535,	65535,	65535,	65535,
		2116,	1000,	0,		0,		1000,			/* 7-11 */
		2116,	5846,	8576,	14692,	22844			/* 12-16 */
	};

	emUInt8 j, i, iBest;
	emUInt32 nMinCost;

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pRhythmRT && pCost);
	emAssert(0 == iPos || GetBoundaryBefore(pRhythmRT, iPos) >= BdPW);
	emAssert(0 == iPos || pCost[iPos].wPwCost);

	/* 后面16个音节以内,返回最后一个节点 */
	j = GetSylCount(pRhythmRT) - iPos;
	if ( j <= 16 )
		return GetSylCount(pRhythmRT);

	
	/* 最大处理到16个音节 */
	if ( j > 16 )
		j = 16;

	/* 计算结束位置 */
	j += iPos;

	/* 搜索最有位置 */
	iBest = iPos;
	nMinCost = EM_ULONG_MAX;

	for ( i = iPos; i < j; i = GetNextWord(pRhythmRT, i) )
	{
		emUInt32 nCost;

		if( i>= (iPos+4) && pCost[i].wPphCost>0 )		//hyl 2012-02-23 原来常量为6
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
* 函数名        : GetOtherGandidate
* 描述          : 对PPH进行排序，选出最优的三个点，共有两次排序
* 参数          : [in]  pCost  - PW和PPH代价数组
*               : [in]  iBegin - 起始音节
*               : [in]  iEnd   - 结束音节
*               : [out] pBest  - 最优点位置数组
* 返回          : <无>

******************************************************************************/
emInline void emCall GetOtherGandidate(
	PCostInfo		pCost,			/* PW和PPH代价数组 */
	emUInt8			iBegin,			/* 起始音节 */
	emUInt8			iEnd,			/* 结束音节 */
	emUInt8 *		pBest			/* 最优点位置数组 */
)
{
	emUInt8 i;

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pCost);

	/* 求1~4字中最大一个 */
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

	/* 求3个,或2个,或1个最优点 */
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
* 函数名        : GetPWTotalCost
* 描述          : 得到整个PW的代价和
* 参数          : [in]  pCost  - PW和PPH代价数组
*               : [in]  iBegin - 起始音节
*               : [in]  iEnd   - 结束音节
* 返回          : <emUInt32>    - 代价和

******************************************************************************/
emInline emUInt32 emCall GetPWTotalCost(
	PCostInfo		pCost,			/* PW和PPH代价数组 */
	emUInt8			iBegin,			/* 起始音节 */
	emUInt8			iEnd			/* 结束音节 */
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
* 函数名        : Decision1Or2PPH
* 描述          : 决策出1个或者2个最优点
* 参数           : [in]     pRhythmRes - 韵律模块资源对象
*                      : [in]     pCost  - PW和PPH代价数组
*                      : [in]     iBegin - 起始音节
*                      : [in]     iEnd   - 结束音节
*                      : [in/out] pBest  - 最优点位置数组

******************************************************************************/
emStatic void emCall Decision1Or2PPH(
	PRhythmRes		pRhythmRes,		/* 韵律模块资源对象 */
	PCostInfo		pCost,			/* PW和PPH代价数组 */
	emUInt8			iBegin,			/* 起始音节 */
	emUInt8			iEnd,			/* 结束音节 */
	emUInt8 *		pBest			/* 最优点位置数组 */
)
{
	emUInt8 m, n;
	emUInt32 dwMinCost, dwCost;
	emUInt8 nPosA, nPosB;

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pRhythmRes && pCost);

	nPosA = pBest[0];
	nPosB = pBest[1];

	/* 当没有最优点时，直接返回 */
	if ( 0 == nPosA )
	{
		pBest[0] = iEnd;
		pBest[1] = 0;
		pBest[2] = 0;
		return;
	} 

	/* 考虑一个PPH的情况 */
	/********************************************************************
	*  0            A                                   1
	*  |____________|___________________________________|
	*
	*	得到的1个PPH情况为：01
	*********************************************************************/
	m = iEnd - iBegin;
	dwMinCost = 6553500 + GetSinglePPHCost(pRhythmRes, m);
	pBest[0] = iEnd;
	pBest[1] = 0;
	pBest[2] = 0;

	if (0 != iBegin)
		dwMinCost -= TWO_PPH_COST_BASE;

	/* 两个PPH情况 */
	/********************************************************************
	*  0            A                    B               1
	*  |____________|____________________|_______________|
	*
	*	得到的两个PPH情况为：0A A1
	*********************************************************************/
	m = nPosA- iBegin;
	n = iEnd - nPosA;
	dwCost = 6553500 + GetTwoPPHCost(pRhythmRes, m, n);		//增大本句的常量值，可将11字句（及以下）的PPH切分降低
	dwCost += pCost[nPosA].wPphCost;
	dwCost -= pCost[nPosA].wPwCost;
	if ( dwMinCost > dwCost ) 
	{
		dwMinCost = dwCost;
		pBest[0] = nPosA;
		pBest[1] = iEnd;
	}

	if ( 0 == pBest[1] )/* 当只有一个最优点时，有两种情况 */
		return;

	/* 第二种两个PPH的情况 */
	/********************************************************************
	*  0            A                    B               1
	*  |____________|____________________|_______________|
	*
	*	得到的两个PPH情况为：0B B1
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
* 函数名        : Decision2Or3PPH
* 描述          : 决策出2个或3个PPH节点
* 参数           : [in]     pRhythmRes - 韵律模块资源对象
*                      : [in]     pCost  - PW和PPH代价数组
*                      : [in]     iBegin - 起始音节
*                      : [in]     iEnd   - 结束音节
*                      : [in/out] pBest  - 最优点位置数组
* 返回               : <无>

******************************************************************************/
emInline void emCall Decision2Or3PPH(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PCostInfo		pCost,			/* PW和PPH代价数组 */
	emUInt8		iBegin,			/* 起始音节 */
	emUInt8		iEnd,			/* 结束音节 */
	emUInt8 *		pBest			/* 最优点位置数组 */
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

	/* 对四个最优点的位置排序 */
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
	/* 排序结束 */
	
	/* 求PW概率和 */ 
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
	* 回溯前 考虑2个PPH的情况
	*  0            A                    B               C               1
	*  |____________|____________________|_______________|_______________|
	*
	*	情况有： 0A A1
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
	*	考虑3个PPH的情况
	*  0            A                    B               C               1
	*  |____________|____________________|_______________|_______________|
	*
	*	情况有： 0A AB B1
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
 *	接口函数
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


		/* 读取资源头 */
		emRes_SetPos(g_ResPPHRatio, 32);	/* 跳过文件描述 */
		pRhythmRes->m_subSyntax.m_dwSinglePPHCost = emRes_Get32(g_ResPPHRatio);
		pRhythmRes->m_subSyntax.m_dwTwoPPHCost	= emRes_Get32(g_ResPPHRatio);
		pRhythmRes->m_subSyntax.m_dwThreePPHCost	= emRes_Get32(g_ResPPHRatio);

		pRhythmRes->m_subSyntax.m_pC45Res = g_ResPPHC45;
		pRhythmRes->m_subSyntax.m_pPPHRes = g_ResPPHRatio;
	}
}


/**************************************************************************************************************************/
/*************************************** PPH边界划分后处理函数：开始 ******************************************************/
/**************************************************************************************************************************/

#if PPH_AFTER_DEAL					//PPH边界划分后处理


/******************************************************************************
* 描述          : 连续短韵律清除
******************************************************************************/
emInline void emCall MoreShortPPHClear(
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	emUInt8			allNodes		/* 文本总长度 */
)
{
	//例如：【这反映了地价◆与其它◆市场要素◆在价格水平上是协调的】  ->  【这反映了地价◆与其它市场要素◆在价格水平上是协调的】

	emUInt8  PPHIndex[20];
	emUInt8		j = 0, nPPHLen = 0, nPPHCount,nCurIndex;
	emUInt8		curPos;				//遍历时的位置

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
				SetBoundaryBefore(pRhythmRT, nCurIndex, BdPW);		//连续短韵律清除: 两个短韵的总长度<=8
				j++;
			}
			else
				;		//后韵<=2字，则等着以后清

		}
	}
}


/******************************************************************************
* 描述          : 改变特定句型的韵律切分
******************************************************************************/
emInline void emCall SpecialSenChangeCost(
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	emUInt8			allNodes,		/* 文本总长度 */
	emUInt8 *		PPHPos			/* PPH边界数组 */
)
{

	LOG_StackAddr(__FUNCTION__);

	//《往****方向》，韵律清除：  改善韵律划分效果
	if(    allNodes <= 12 
		&& pRhythmRT->m_tPoSBeforePW[0] == ChsPoS_p
		&& pRhythmRT->m_tPoSBeforePW[1] > 0
		&& pRhythmRT->m_tPoSBeforePW[allNodes-2] == ChsPoS_n
		&& GetWord(pRhythmRT->m_cText,0) == (emInt16)0xcdf9	)	//“往”
	{
		//若句子：符合《往****方向》结构，且句长<=12，则本句的PPH边界全部清除
		PPHPos[0] = allNodes;
		PPHPos[1] = 0;
		PPHPos[2] = 0;
	}

	//《车辆位于****附近》，韵律改变：  改善韵律划分效果
	if(    allNodes <= 17 &&  allNodes >= 9 
		&& pRhythmRT->m_tPoSBeforePW[0] == ChsPoS_n
		&& pRhythmRT->m_tPoSBeforePW[1] == 0 
		&& pRhythmRT->m_tPoSBeforePW[2] == ChsPoS_v 
		&& pRhythmRT->m_tPoSBeforePW[allNodes-2] == ChsPoS_f
		&& GetWord(pRhythmRT->m_cText,4) == (emInt16)0xcebb		//“位”
		&& GetWord(pRhythmRT->m_cText,6) == (emInt16)0xd3da	)	//“于”
	{
		//若句子：符合《车辆位于****附近》结构，且9=<句长<=17，则本句的PPH边界改变成：4+？
		PPHPos[0] = 4;
		PPHPos[1] = allNodes;
		PPHPos[2] = 0;
	}

	//《接近目的地****》，韵律改变：  改善韵律划分效果
	if(    allNodes <= 19 &&  allNodes >= 9 
		&& pRhythmRT->m_tPoSBeforePW[0] == ChsPoS_v
		&& pRhythmRT->m_tPoSBeforePW[1] == 0 
		&& pRhythmRT->m_tPoSBeforePW[2] == ChsPoS_n 
		&& GetWord(pRhythmRT->m_cText,4) == (emInt16)0xc4bf		//“目”
		&& GetWord(pRhythmRT->m_cText,6) == (emInt16)0xb5c4		//“的”
		&& GetWord(pRhythmRT->m_cText,8) == (emInt16)0xb5d8	)	//“地”
	{
		//若句子：符合《接近目的地****》结构，且9=<句长<=17，则本句的PPH边界改变成：5+？
		PPHPos[0] = 5;
		PPHPos[1] = allNodes;
		PPHPos[2] = 0;
	}
}			


/******************************************************************************
* 描述          : 改变本句中两个特定词性间的相关代价
******************************************************************************/
emInline void emCall SpecialWordChangeCost(
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PCostInfo		pCost,			/* PW和PPH代价数组 */
	emUInt8			allNodes		/* 文本总长度 */
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
				//<数词>与<数词>之间，<数词>与<量词>之间；将此边界的wPphCost变大；降低将此边界划分为PPH的风险
				pCost[i].wPphCost = 65535;		
				break;
			case  CHANGE_TYPE_f_v:
				//<方位词>与<动词>之间，将此边界的wPphCost变大；降低将此边界划分为PPH的风险
				pCost[i].wPphCost = emMin(pCost[i].wPphCost*3,65000);	
				break;
			case  CHANGE_TYPE_ngp_null:
				//<姓名>与<任意词>之间，将此边界的wPphCost变大；降低将此边界划分为PPH的风险
				pCost[i].wPphCost = emMin(pCost[i].wPphCost*4,65000);	
				break;
			case  CHANGE_TYPE_m_v:
				//<数词>与<动词>之间，将此边界的wPphCost变大；降低将此边界划分为PPH的风险
				pCost[i].wPphCost = emMin(pCost[i].wPphCost*6,65000);				
				break;
			case  CHANGE_TYPE_f_n:
				//<1字方位词>与<名词>之间，将此边界的wPphCost变大；降低将此边界划分为PPH的风险
				pCost[i].wPphCost = 65535;				
				break;
			default:
				break;
		}
	}
}




/******************************************************************************
* 函数名        : GetPrevAndNextWordPosLen
* 描述          : 获取与iReturnBest分界点前后相关的词性和词长
*
******************************************************************************/
emInline void emCall GetPrevAndNextWordPosLen(
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PWordPosLen		pWordPosLen,	/* 前后相关的词性和词长信息 */
	emUInt8			curPos,			/* 当前位置 */
	emUInt8			allNodes		/* 文本总长度 */
)
{
	emUInt8 temp1;

	LOG_StackAddr(__FUNCTION__);

	pWordPosLen->curFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos];
	pWordPosLen->curFeaLen    = pRhythmRT->m_tRhythmBeforePW[curPos]&0x0F ;
				
	if( (curPos - 1)>0 && pRhythmRT->m_tRhythmBeforePW[curPos - 1] > 0 )		//搜索：前词(1字词)		
	{
		pWordPosLen->preFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-1];
		pWordPosLen->preFeaLen    = 1 ;					
		if( (curPos - 2)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 2] > 0 )			//搜索：前前词(1字词)			
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-2];
			pWordPosLen->prePreFeaLen    = 1 ;
		}
		else if( (curPos - 3)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 3] > 0 )		//搜索：前前词(2字词)	
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-3];
			pWordPosLen->prePreFeaLen    = 2 ;
		}
	}
	else if( (curPos - 2)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 2] > 0 )	//搜索：前词(2字词)		
	{
		pWordPosLen->preFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-2];
		pWordPosLen->preFeaLen    = 2;
		if( (curPos - 3)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 3] > 0 )			//搜索：前前词(1字词)			
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-3];
			pWordPosLen->prePreFeaLen    = 1 ;
		}
		else if( (curPos - 4)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 4] > 0 )		//搜索：前前词(2字词)	
		{
			pWordPosLen->prePreFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-4];
			pWordPosLen->prePreFeaLen    = 2 ;
		}
	}
	else if( (curPos - 3)>0 &&  pRhythmRT->m_tRhythmBeforePW[curPos - 3] > 0 )	//搜索：前词(3字词)		
	{
		pWordPosLen->preFeaPos    = pRhythmRT->m_tPoSBeforePW[curPos-3];
		pWordPosLen->preFeaLen    = 3;
	}

	
	temp1 = curPos + pWordPosLen->curFeaLen;
	if(    (temp1) <  allNodes
		&& pRhythmRT->m_tRhythmBeforePW[temp1] > 0 )	//搜索：后词	
	{
		pWordPosLen->nextFeaPos    = pRhythmRT->m_tPoSBeforePW[temp1];
		pWordPosLen->nextFeaLen    = pRhythmRT->m_tRhythmBeforePW[temp1]&0x0F ;
		
		temp1 = curPos + pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen;
		if(    temp1 <  allNodes
		    && pRhythmRT->m_tRhythmBeforePW[temp1] > 0 )						//搜索：后后词	
		{
			pWordPosLen->nextNextFeaPos    = pRhythmRT->m_tPoSBeforePW[temp1];
			pWordPosLen->nextNextFeaLen    = pRhythmRT->m_tRhythmBeforePW[temp1]&0x0F ;
		}
	}
}




/******************************************************************************
* 描述          : PPH边界是否需要前移：最多前移1个字
* 返回          : 更新后的当前的PPH分界点
******************************************************************************/
emInline emUInt8 PPHNeedLitterMove(  
								PRhythmRT			pRhythmRT,		/* 韵律实时对象 */
								emUInt8				nPPHIndex,		/* 当前PPH分界点 */
								emUInt8				nPrevPPHIndex,	/* 前一PPH分界点 */
								emUInt8				allNodes)		/* 文本总长度 */
{
	emUInt8	 nNewPPHIndex=0,k;	

	TWordPosLen tWordPosLen;
	PWordPosLen pWordPosLen = &tWordPosLen;

	LOG_StackAddr(__FUNCTION__);


	emMemSet(&tWordPosLen, 0 , sizeof(tWordPosLen));

	//获取与iReturnBest分界点前后相关的词性和词长
	GetPrevAndNextWordPosLen( pRhythmRT, pWordPosLen, nPPHIndex, allNodes );	

	if( (nPPHIndex-nPrevPPHIndex) >= 4)
	{
		//开始前移判断
		if(    pWordPosLen->preFeaPos    == ChsPoS_d				//前词(1字词)	  ：副词  例如：不d
			&& pWordPosLen->preFeaLen    == 1)				
		{	
			if(    (nPPHIndex-nPrevPPHIndex) >= 5  
				&& pWordPosLen->prePreFeaLen == 1
				&& (   pWordPosLen->prePreFeaPos == ChsPoS_v				//前前词(1字词)   ：动词  例如：说v+过d
					|| pWordPosLen->prePreFeaPos == ChsPoS_d				//前前词(1字词)   ：副词  例如：也d不d 
					|| pWordPosLen->prePreFeaPos == ChsPoS_c))				//前前词(1字词)   ：连词  例如：而c不d 
			{
				//前移到：前前词之前
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen - pWordPosLen->prePreFeaLen;	
			}
			else
			{
				//前移到：前词之前
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	
			}
		}
		else if(  pWordPosLen->preFeaPos  ==  ChsPoS_v &&  pWordPosLen->preFeaLen    == 1                                  //by zcc 02-27
			   && pWordPosLen->prePreFeaPos != ChsPoS_c
			   && pWordPosLen->prePreFeaPos != ChsPoS_p
			   &&(  GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xbfc9       //可
			   ||GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xd3a6          //应
			   ||GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xd2aa ) )      //要
		{
			//前词是动词“可”“应”“要”，且词长为1

			if( (  pWordPosLen->prePreFeaPos == ChsPoS_d 
				 ||pWordPosLen->prePreFeaPos == ChsPoS_r
				 ||pWordPosLen->prePreFeaPos == ChsPoS_R )  
			  &&  pWordPosLen->prePreFeaLen    == 1 )      //前前词(1字词)   ：副词或代词
			{
				//前移到：前前词之前
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen - pWordPosLen->prePreFeaLen;	
			}
			else
			{
				//前移到：前词之前
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	
			}

			// 例如：我说的可◆都是真话啊    起诉书副本最迟应◆在开庭七日以前◆送达被告人
			// 例如：我从来没跟他这样◆为躲开人群要◆从厨房溜进来的大名人吃过饭     
			
		}
		else if(   pWordPosLen->preFeaPos    == ChsPoS_v				
			&& pWordPosLen->preFeaLen    == 1
			&& pWordPosLen->curFeaPos    == ChsPoS_v				
			&& pWordPosLen->curFeaLen    == 1)

		{
			if(    pWordPosLen->prePreFeaPos == ChsPoS_d && pWordPosLen->prePreFeaLen == 1)
			{
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen  - pWordPosLen->prePreFeaLen;	
				//例如：  不念◆出  -》  ◆不念出 
			}
			else
			{
				//前词和本词都是1字动词，前移到前词之前
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	
				//例如：  送◆出  -》  ◆送出
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
			//前词是代词且前前词不是动词、介词、助词，前移到前词之前
			nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	

			// 例如： 他告诉人们他◆一直在学怎么用中文说对不起   这就是为什么我◆总是把它往手臂上拉

		}
		else if(   pWordPosLen->preFeaPos  ==  ChsPoS_c 
			    || ( pWordPosLen->preFeaPos ==  ChsPoS_p && pWordPosLen->preFeaLen > 1)
				|| (    pWordPosLen->preFeaPos    ==  ChsPoS_p 
				     && pWordPosLen->preFeaLen    == 1 
					 && pWordPosLen->prePreFeaPos != ChsPoS_p 
					 && pWordPosLen->prePreFeaPos != ChsPoS_d 
					 && pWordPosLen->prePreFeaPos != ChsPoS_v )) 
		{
			//前词是介词、连词，前移到前词之前

			if(   pWordPosLen->preFeaPos  ==  ChsPoS_p 
			   && pWordPosLen->preFeaLen    == 1
			   && pWordPosLen->prePreFeaPos == ChsPoS_v
			   &&GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xceaa )          //“为”				  
			{
				//若是“为”，则不前移
				;
			}
			else
				nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	

			// 例如：这些小组并◆不仅仅是在发挥橡皮图章的作用   但我不是因为◆他们才想打篮球的   教练甚至把◆我放在首发阵容中 

		}
		else if(  pWordPosLen->preFeaPos  ==  ChsPoS_u  &&  pWordPosLen->preFeaLen    == 1        //by zcc 02-28
		        && GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xcbf9 )        //所
	    {
			//前词是助词“所”，前移到前词之前
			nNewPPHIndex = nPPHIndex - pWordPosLen->preFeaLen;	

			// 例如：对话才是解决巴以之间所◆有悬而未决问题的唯一途径 

		}
	}
	
	//开始后移判断
	if(  pWordPosLen->preFeaPos  ==  ChsPoS_u  &&  pWordPosLen->preFeaLen    == 1       //by zcc 02-27
	   && GetWord(pRhythmRT->m_cText,(nPPHIndex-1)*2) == (emInt16)0xd6ae )        //之
	{
		//前词是1字助词“之”

		if( pWordPosLen->nextFeaPos  == ChsPoS_u && pWordPosLen->nextFeaLen  == 1)      //后词	：1字助词
		{
			//后移到：后词之后
			nNewPPHIndex = nPPHIndex + pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen;
		}
		else if( pWordPosLen->curFeaPos  == ChsPoS_m && pWordPosLen->nextFeaPos  == ChsPoS_m )
		{
			//不移
			;
		}
		else
			//后移到：本词之后
			nNewPPHIndex = nPPHIndex + pWordPosLen->curFeaLen;

		// 例如：千百万张笑脸◆朝着新朝代建筑师◆人民共和国三巨头之◆一的周恩来欢呼 

	}

	if(  nNewPPHIndex != 0)
	{
		//需设置为PPH的边界原来不是PW边界，需先修改成PW边界
		if(  GetBoundaryBefore(pRhythmRT, nNewPPHIndex) < BdPW)			//现边界不是PW边界		
		{
			if( pRhythmRT->m_tRhythmBeforePW[nNewPPHIndex]>0 )				//但现边界是语法词边界
			{
				for( k = (nNewPPHIndex-1);k>0; k--)
				{
					if(  pRhythmRT->m_tRhythm[k]  > 0 )
					{
						pRhythmRT->m_tRhythm[nNewPPHIndex] = (pRhythmRT->m_tRhythm[k]&0x0F) - (nNewPPHIndex - k);		// (pRhythmRT->m_tRhythm[j]&0x0F)为韵律词长
						pRhythmRT->m_tRhythm[k]           = nNewPPHIndex - k;									
						break;
					}
				}	
				SetBoundaryBefore(pRhythmRT, nNewPPHIndex,BdPW);
			}
			else							
				nNewPPHIndex = 0;								//若不是语法词边界，则前移默认无效
		}	
	}

	if(  nNewPPHIndex == 0)
		nNewPPHIndex = nPPHIndex;		//维持原分界点

	return nNewPPHIndex;
}




/******************************************************************************
* 描述          : 改变本句中特征词相关的代价
******************************************************************************/
emInline void emCall PPHFeatureWordsHandle(
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PCostInfo		pCost,			/* PW和PPH代价数组 */	
	emUInt8			allNodes		/* 文本总长度 */
)
{
	#define	FEATURE_WORD_MAX		150			//特征词的最多个数
	

	#define PROB_MAX				3			//优先级：最高
	#define PROB_MID				2			//优先级：中等
	#define PROB_MIN				1			//优先级：最低
	
	#define FINAL_NO_FIND_COUNT		3			//句尾的几个字范围内

	#define COST_NEED_ADD_RANGE		4			//特征词边界前后几个字的范围内代价加大

	emUInt8	 curPos = 0;			//当前位置 

	emUInt8 i,j,k, curProb,  curSelWordIndex, nFinalIndex;

	emInt8 iReturnBest;

	emUInt8 nStartIndex=0, nEndIndex=0;
	
	emBool  bBestNeedMove;

	emInt8  nMoveZiPos;	//移动的字的位置  ： < 0：前移  > 0 ：后移

	struct FeatureWordStruct
	{
		emCharA		Word[4];					//特征词：					最多2字，只有1字时用空格补齐
		emUInt8		WordCount;					//特征词的字数：			1个 或 2个
		emUInt8		WordPos;					//特征词的词性：
		emUInt8		WordStartIndex;				//特征词开始搜寻的起始位置：第几个字
		emUInt8		WordBestInjust;				//最优点位置相对于特征词起始位置上的调整值  例如：1.“等” 作为PPH尾，则=1；2.“不但” 作为PPH首，则=0；3.“位于” 作为PPH尾，则=2；
		emUInt8		WordProb;					//特征词优先级，分为1级，2级，3级； 越大优先级越高
	};

	//******************************************************************************************************
	//*   特征词说明：
	//*					1. 优先级：一般2字词的优先级设为PROB_MAX（即最高），1字词的优先级设置根据测试情况定
	//* 				3. 一般情况下： WordStartIndex = 6 - WordBestInjust，即期望至少前5个字成PPH，下1个PPH从第6个字开始
	//*   
	//*                 特征词数组：  大小为：12*100字节=1.2K字节
	//******************************************************************************************************
	struct FeatureWordStruct     FeatureWords[FEATURE_WORD_MAX]={		
		/*  特征词  字数    词性     搜寻起始字  优点调整   优先级     */

		// 2字特征词，PPH边界在前
		{ "位于",  2 ,  ChsPoS_v ,     3 ,        0 ,   PROB_MAX },		  //hyl
		{ "随后",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //hyl
		{ "比方",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc  
		{ "不料",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "不论",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },		  //zcc
		{ "不如",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "不如",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "但是",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "而且",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },		  //zcc
		{ "而是",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "何况",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "似乎",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "虽然",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "所以",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "倘若",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc		
		{ "一般",  2 ,  ChsPoS_u ,     5 ,        0 ,   PROB_MAX },		  //zcc		
		{ "以便",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc 
		{ "以及",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "因此",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "因而",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "因为",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc		
		{ "纵然",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "不过",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "不过",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "还是",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "还是",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "好比",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "即使",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },       //zcc
		{ "固然",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },       //zcc
		{ "等于",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "不但",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "原来",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "正在",  2 ,  ChsPoS_d ,     4 ,        0 ,   PROB_MAX },       //zcc
		{ "只是",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "只是",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "一面",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "既然",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "假如",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "假使",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "仅仅",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "尽管",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "例如",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "譬如",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "偏偏",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },       //zcc
		{ "然而",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "如果",  2 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MAX },		  //zcc
		{ "如同",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "若是",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc	
		{ "关于",  2 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "或者",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "乃至",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "只有",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "才能",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc				
		{ "并且",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "仍然",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "比如",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "况且",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "甚至",  2 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "甚至",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "曾经",  2 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MAX },		  //zcc
		{ "可能",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //by zcc 02-22	
		{ "还有",  2 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MAX },		  //by zcc 02-28

		{ "才能",  2 ,  ChsPoS_n ,     5 ,        0 ,   PROB_MIN },		  //zcc		//设置优先级最低（一般情况下名词不做特征词），语料中还有很多将 “才能  v”错分析成 “才能  n”
		{ "一般",  2 ,  ChsPoS_a ,     5 ,        0 ,   PROB_MIN },		  //zcc
		{ "一边",  2 ,  ChsPoS_d ,     4 ,        0 ,   PROB_MIN },		  //zcc

		


		// 1字特征词，PPH边界在前
		{ "往  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MAX },		  //hyl
		{ "是  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //hyl
		{ "但  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "到  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "都  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "而  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "跟  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "共  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "还  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc 
		{ "或  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "及  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "就  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "且  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "请  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "却  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "仍  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "如  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "和  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "若  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "同  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "像  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "像  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "像  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "又  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "又  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "与  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "与  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "则  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "则  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "曾  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "至  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "至  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "并  ",  1 ,  ChsPoS_c ,     4 ,        0 ,   PROB_MID },		  //zcc
		{ "并  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "从  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "把  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "对  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "在  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc		
		{ "让  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "要  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "才  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "才  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "来  ",  1 ,  ChsPoS_f ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "既  ",  1 ,  ChsPoS_c ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "被  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "比  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "有  ",  1 ,  ChsPoS_v ,     5 ,        0 ,   PROB_MID },		  //zcc
		{ "为  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MIN },		  //by zcc 02-28
		{ "也  ",  1 ,  ChsPoS_d ,     5 ,        0 ,   PROB_MID },		  //by zcc 02-28
		{ "由  ",  1 ,  ChsPoS_p ,     5 ,        0 ,   PROB_MID },		  //by zcc 02-28


		// 1字特征词，PPH边界在后		
		{ "说  ",  1 ,  ChsPoS_v ,     4 ,        1 ,   PROB_MID },		  //hyl				
		{ "等  ",  1 ,  ChsPoS_u ,     4 ,        1 ,   PROB_MID },		  //hyl		
		{ "以来",  2 ,  ChsPoS_f ,     5 ,        2 ,   PROB_MAX },		  //by zcc 02-28


		//结束标志
		{ "    ",  1 ,  ChsPoS_v ,     5 ,        1 ,   PROB_MID }};	
		


		/*******   以下为不能加的  *******************
		讲		ChsPoS_v	 //hyl		不能添加 ，会更坏
		后		ChsPoS_f	 //hyl		不用添加，无改善，且有些会变坏

		于是	ChsPoS_		 //zcc			与“是”相关，词中间被断开，没有此词
		才是	ChsPoS_ 	 //zcc			与“是”相关，词中间被断开，没有此词
		就是	ChsPoS_ 	 //zcc			与“是”相关，词中间被断开，没有此词
		不是	ChsPoS_v	 //zcc			与“是”相关，词中间被断开，没有此词 
		都是	ChsPoS_v	 //zcc			与“是”相关，词中间被断开，没有此词
		要是	ChsPoS_c	 //zcc			与“是”相关，词中间被断开，没有此词
		到达	ChsPoS_v	 //zcc			添加后效果更差
		由于	ChsPoS_c	 //zcc			添加后效果更差
		由于	ChsPoS_p	 //zcc			添加后效果更差
		可能	ChsPoS_v 	 //zcc			添加后效果更差
		可是	ChsPoS_ 	 //zcc			语料不足，都是对的
		岂知	ChsPoS_ 	 //zcc			语料不足，都是对的
		至于	ChsPoS_ 	 //zcc			语料中基本都是“以至于”，“不至于”，“乃至+于”，				占30%
		那就	ChsPoS_ 	 //zcc			语料中基本都是“那+就+是”，“那+就”										占4%
		是故	ChsPoS_ 	 //zcc			语料中基本都是“是+故事”，“是+故障”									占15%
		与其	ChsPoS_ 	 //zcc			语料中基本都是“与+其他”等，很少是词“与其”
		与同	ChsPoS_ 	 //zcc			语料中基本都是“与+同学”等，很少是词“与同”			
		致		ChsPoS_ 	 //zcc			语料中基本都是“一致”，“导致”
		以		ChsPoS_ 	 //zcc			语料中基本都是“以为”，“以后”
		可		ChsPoS_ 	 //zcc			语料不足，基本都是“可以”
		乃		ChsPoS_ 	 //zcc			语料不足，基本都是“乃至”
		说到	ChsPoS_ 	 //zcc			词中间被断开，没有此词
		不及	ChsPoS_ 	 //zcc			添加后效果更差
		刚才	ChsPoS_t 	 //hyl			不适合做特征词
		之后	ChsPoS_t 	 //hyl			不适合做特征词
		目前	ChsPoS_t 	 //hyl			不适合做特征词
		以后	ChsPoS_f 	 //hyl			不适合做特征词
		以前	ChsPoS_f 	 //hyl			不适合做特征词
		来			 	 	 //hyl			词性很杂，且不适合做特征词，
		因		ChsPoS_c 	 //hyl			适合做特征词，但语料中大多是“因为 p” “因为 c”  
		**********************************************/



	LOG_StackAddr(__FUNCTION__);

	//依据PPH特征词，调整改变pCost[i].wPphCost的值，将PPH特征词的边界的wPphCost变小（暂设1000）（提高将此边界划分为PPH的概率）	
	iReturnBest = 0;
	curProb = 0;
	curSelWordIndex = 0;
	

	//在文本结尾的最后FINAL_NO_FIND_COUNT个字以内不查找特征词
	nFinalIndex = allNodes;
	if( (allNodes - nFinalIndex ) <= FINAL_NO_FIND_COUNT )				
	{
		if(  (allNodes - FINAL_NO_FIND_COUNT) > 0 )
			nFinalIndex = allNodes - FINAL_NO_FIND_COUNT;	
		else
			nFinalIndex = 0;
	}

	for(i=curPos;i<nFinalIndex;i++)						//循环搜寻当前文本
	{
		for(j = 0; j < FEATURE_WORD_MAX; j++ )		//循环搜寻特征词
		{
			if( FeatureWords[j].Word[0] ==  0 )		
				break;								//最后1个特征词，跳出

			//大于搜寻起始位置( 当前位置大于本特征词的起始位置且大于输入参数最小起始位置)
			if( FeatureWords[j].WordStartIndex <= (i-curPos+1)  )	
			{
				//与特征词匹配上，且词性匹配上,且词的字数对上
				if(  emMemCmp( pRhythmRT->m_cText+i*2 , FeatureWords[j].Word, FeatureWords[j].WordCount*2)==0	//匹配：特征词
				  && pRhythmRT->m_tPoSBeforePW[i] == FeatureWords[j].WordPos 									//匹配：词性（与PW拼接前的原始词性比较）
				  && (pRhythmRT->m_tRhythmBeforePW[i]&0x0F) == FeatureWords[j].WordCount)						//匹配：词的字数（与PW拼接前的原始Rhythm比较）
				{				

					iReturnBest = i;
					curProb = FeatureWords[j].WordProb;
					curSelWordIndex = j;

					if( iReturnBest != 0 )
					{

				//*********************************************************************************************************
				//*********************************** 判断：最优点是否需要在本特征词的基础上移动：开始 ********************
				//*********************************************************************************************************

						TWordPosLen tWordPosLen;
						PWordPosLen pWordPosLen = &tWordPosLen;
						emMemSet(&tWordPosLen, 0 , sizeof(tWordPosLen));

						//获取与iReturnBest分界点前后相关的词性和词长
						GetPrevAndNextWordPosLen( pRhythmRT, pWordPosLen, iReturnBest, allNodes );		

						bBestNeedMove = emFalse;		//初始化：设置最优点不需要在本特征词的基础上移动
						nMoveZiPos = 0;

						//目前最优点相对位置至少应大于
						if(  (iReturnBest + FeatureWords[curSelWordIndex].WordBestInjust - curPos) >= 4	)	
						{	
							
							//************************* 判断：本特征词:动词	  前词:副词  优点调整值=0  ***************************
							//****************************************************************************************************
							if(    FeatureWords[curSelWordIndex].WordBestInjust == 0				//优点调整值=0      不调整
								&& pWordPosLen->curFeaPos == ChsPoS_v											//本特征词：		动词
								&& (    pWordPosLen->preFeaPos == ChsPoS_d										//前词：			副词
									 || pWordPosLen->preFeaPos == ChsPoS_v 										//					动词	单独处理
									 || pWordPosLen->preFeaPos == ChsPoS_r 										//					代词	
									 || pWordPosLen->preFeaPos == ChsPoS_R 										//					人称代词
									 || pWordPosLen->preFeaPos == ChsPoS_c 										//					连词
									 || pWordPosLen->preFeaPos == ChsPoS_t ))                                 //by zcc 02-22
									 //|| pWordPosLen->preFeaPos == ChsPoS_a ))									//前词：			形容词														
							{
								if( pWordPosLen->preFeaPos == ChsPoS_v )	//单独处理
								{
									
									if(    GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xcac7	 //“是”
										|| GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd2aa	 //“要”
										|| GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xc8c3) //“让”
									{
										if(    pWordPosLen->prePreFeaPos ==  ChsPoS_R 
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_r
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_c    //by zcc 02-22
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_p    //by zcc 02-22  例如：西藏曾经被◆认为是一个神秘的地区
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_d)
										{
											//移到：前前词之前
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen;	
										}
										else
										{
											//移到：前词之前
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen;	
										}

										//例如： 她转过身见◆是她的朋友   就像看电视会◆让我感觉到痛苦一样   还听说他们想◆要一个控球后卫
									}	

								}
								else if(    pWordPosLen->preFeaPos ==  ChsPoS_t )          //by zcc 02-22					
								{
									//前词是时间词：单独处理
									if(	GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xcac7    //是
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_p
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_t
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_v  )
									{										
										//移到：前词之前
										bBestNeedMove = emTrue;
										nMoveZiPos -= pWordPosLen->preFeaLen;	

										//例如：伊拉克境内的警察局如今◆是众人避之不及的场所

									}
									else
										bBestNeedMove = emFalse;	//不前移
								}
								else if(    pWordPosLen->prePreFeaPos == ChsPoS_u 
										 && pWordPosLen->prePreFeaLen == 1 
										 && pWordPosLen->preFeaPos != ChsPoS_d)
								{					
									//前前词:1字助词， 最优点不移动
									bBestNeedMove = emFalse;	

									//例句：几代人的努力好比是运动场上的接力赛
								}
								else if(   pWordPosLen->prePreFeaPos == ChsPoS_d 
										|| pWordPosLen->prePreFeaPos == ChsPoS_c 
										|| pWordPosLen->prePreFeaPos == ChsPoS_p)	          						
								{
									//前前词:副词，连词，介词
									if(  (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen) > 2
									   && pWordPosLen->preFeaLen == 1)
									{
										//前词+前前词：词长之和 >2 : 则最优点后移到：特征词之后
										bBestNeedMove = emTrue;	
										nMoveZiPos += pWordPosLen->curFeaLen;								
									}
									else if(  (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen) > 2    //by zcc 02-22
										    && pWordPosLen->preFeaLen > 1)
									{
										//前词+前前词：词长之和 >2 : 则最优点不移到
										bBestNeedMove = emFalse;								
									   
										//例如：为什么偏偏在此时要◆缩短劳动时间

									}
									else
									{
										//前词+前前词：词长之和<=2 : 则最优点后移到：前前词之前
										bBestNeedMove = emTrue;	
										nMoveZiPos -= (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen);				
									}
								}
								else										
								{
									//前前词:不是副词连词介词，最优点后移到：前词之前
									bBestNeedMove = emTrue;	
									nMoveZiPos -= pWordPosLen->preFeaLen;		
								}				
							}

							//*********** 判断：本特征词:连词|副词|介词	  前词(1字词):动词|副词|连词|代词|人称代词|介词 **********
							//****************************************** 优点调整值=0 ********************************************
							else if( FeatureWords[curSelWordIndex].WordBestInjust == 0				//优点调整值=0      不调整   
								&&  (pWordPosLen->curFeaPos ==  ChsPoS_c										//本特征词：		连词
									  || pWordPosLen->curFeaPos ==  ChsPoS_d									//					副词				
									  || pWordPosLen->curFeaPos ==  ChsPoS_p )									//					介词				
								&&  (    (pWordPosLen->preFeaPos == ChsPoS_c && pWordPosLen->preFeaLen==1)		//前词：			连词
									  || (pWordPosLen->preFeaPos == ChsPoS_d )									//					副词
									  || (pWordPosLen->preFeaPos == ChsPoS_v )									//					动词	（单独处理）
									  || (pWordPosLen->preFeaPos == ChsPoS_r )									//					代词	
									  || (pWordPosLen->preFeaPos == ChsPoS_R )									//					人称代词
									  || (pWordPosLen->preFeaPos == ChsPoS_m )									//					数词
									  || (pWordPosLen->preFeaPos == ChsPoS_t )                                  //by zcc 02-22
									  || (pWordPosLen->preFeaPos == ChsPoS_p && pWordPosLen->preFeaLen==1)))	//					介词		
							{	
								if(    pWordPosLen->preFeaPos ==  ChsPoS_m )					
								{
									//前词是数词：单独处理
									if(	   pWordPosLen->curFeaPos ==  ChsPoS_p 
										&& pWordPosLen->curFeaLen <= 2
										&& pWordPosLen->prePreFeaPos !=  ChsPoS_m
										&& GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd6c1 )	 //“至”											
									{										
										//移到：前词之前
										bBestNeedMove = emTrue;
										nMoveZiPos -= pWordPosLen->preFeaLen;	

										//例如： 而谷类植物◆五至十年后◆播种仍能成活
									}
									else
										bBestNeedMove = emFalse;	//不前移
								}
								else if(    pWordPosLen->preFeaPos ==  ChsPoS_t )          //by zcc 02-22					
								{
									//前词是时间词：单独处理
									if(	   GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xbbb9 	 //“还”											
									    && pWordPosLen->prePreFeaPos !=  ChsPoS_p
									    && pWordPosLen->prePreFeaPos !=  ChsPoS_t
									    && pWordPosLen->prePreFeaPos !=  ChsPoS_v  )
									{										
										//移到：前词之前
										bBestNeedMove = emTrue;
										nMoveZiPos -= pWordPosLen->preFeaLen;	

										//例如： 加拿大军方目前◆还没有能够到达◆汉斯岛的破冰舰只

									}
									else
										bBestNeedMove = emFalse;	//不前移
								}
								else if(    pWordPosLen->preFeaPos ==  ChsPoS_v )					
								{
									//前词是动词：单独处理
									if(	   pWordPosLen->curFeaPos ==  ChsPoS_p 
										&& (    GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd6c1	 //“至”
											 || GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xb0d1	 //“把”
											 || GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xcdf9	 //“往”
											 || GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xd4da))	 //“在”
									{
										if(    pWordPosLen->prePreFeaPos ==  ChsPoS_R 
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_r
											|| pWordPosLen->prePreFeaPos ==  ChsPoS_d)
										{
											//移到：前前词之前
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen;	
										}
										else if( pWordPosLen->prePreFeaPos !=  ChsPoS_v)
										{
											//移到：前词之前
											bBestNeedMove = emTrue;
											nMoveZiPos -= pWordPosLen->preFeaLen;	
										}
										else
										{
											bBestNeedMove = emFalse;	//不前移
										}

										//例如： 而黑夜又可降◆至零下一百八十三摄氏度  吵到最后我伏◆在床上大哭   但我们觉得要◆把他培养成为一个◆有道德的
									}
									else if(	pWordPosLen->curFeaPos ==  ChsPoS_p && pWordPosLen->curFeaLen == 1          //by zcc 02-28
											    &&  GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xceaa )	 //“为”
									{
										//移到：本词之后
										bBestNeedMove = emTrue;
										nMoveZiPos += pWordPosLen->curFeaLen;	
									}
									else
										bBestNeedMove = emFalse;	//不前移
								}
								else if( pWordPosLen->prePreFeaPos == ChsPoS_v &&
									( pWordPosLen->preFeaPos == ChsPoS_r ||
									  pWordPosLen->preFeaPos == ChsPoS_R ))
								{
									//前前词：动词，前词：1字代词， 最优点不移动
									bBestNeedMove = emFalse;	

									//例如： 我的伯乐◆那位十分欣赏◆我但又无能无力的唱片公司老总
								}
								else if(   pWordPosLen->prePreFeaPos == ChsPoS_u 
										&& pWordPosLen->prePreFeaLen==1 
										&& pWordPosLen->preFeaPos != ChsPoS_d
										&& GetWord(pRhythmRT->m_cText,iReturnBest*2-4) != (emInt16)0xc1cb)	//“了”
									 	
								{					
									//前前词:1字助词， 最优点不移动
									bBestNeedMove = emFalse;	

									//例句：所：聊斋志异所写虽然多是幽冥幻域之境
									//例句：的：之：
								}
								else if(   (   pWordPosLen->prePreFeaPos == ChsPoS_d 
											|| pWordPosLen->prePreFeaPos == ChsPoS_c 
											|| pWordPosLen->prePreFeaPos == ChsPoS_p) 
										 && pWordPosLen->prePreFeaLen==1 )
										 //&& (pWordPosLen->prePreFeaLen+pWordPosLen->preFeaLen)<= 2) //by zcc 02-22	
								{
									//前前词(1字词):副词连词介词，最优点移动：前前词之前
									bBestNeedMove = emTrue;
									nMoveZiPos -= (pWordPosLen->preFeaLen + pWordPosLen->prePreFeaLen);			
								}
								else
								{
									//其它情况，最优点移动：前词之前
									bBestNeedMove = emTrue;
									nMoveZiPos -= pWordPosLen->preFeaLen;
								}				
							}
							//************************* 判断：本特征词:方位词	  前词:副词  优点调整值=0  ***************************
							//****************************************************************************************************
							else if(    FeatureWords[curSelWordIndex].WordBestInjust == 0				//优点调整值=0      不调整
								&& pWordPosLen->curFeaPos == ChsPoS_f)											//本特征词：		方位词
							{
								if(  pWordPosLen->nextFeaPos == ChsPoS_v)					//后词：	动词
								{
									//最优点不移动
									bBestNeedMove = emFalse;
								}
								else
								{
									bBestNeedMove = emFalse;
									iReturnBest = 0;					//不改变最优点，还用原来的最优点
								}
							}
							//************************* 判断：本特征词:动词	  后词:副词 助词  优点调整值>0 ***********************
							//****************************************************************************************************
							else if(    FeatureWords[curSelWordIndex].WordBestInjust > 0			//优点调整值>0      调整
									 && pWordPosLen->curFeaPos == ChsPoS_v)										//本特征词：		动词														
							{
								if(     pWordPosLen->nextFeaPos == ChsPoS_d					//后词：	副词
									 || pWordPosLen->nextFeaPos == ChsPoS_u	 				//			助词		
									 || pWordPosLen->nextFeaPos == ChsPoS_v					//			动词
									 || pWordPosLen->preFeaPos  == ChsPoS_v					//前词：	动词
									 || pWordPosLen->preFeaPos  == ChsPoS_d	 )				//			副词					
								{
									bBestNeedMove = emFalse;
									iReturnBest = 0;					//不改变最优点，还用原来的最优点
								}
								else
								{
									//最优点不移动
									bBestNeedMove = emFalse;
								}
							}
							else if(    FeatureWords[curSelWordIndex].WordBestInjust > 0			//优点调整值>0      调整
									 && pWordPosLen->curFeaPos == ChsPoS_u)							//本特征词：		助词														
							{
								if(    GetWord(pRhythmRT->m_cText,iReturnBest*2) == (emInt16)0xb5c8	 //“等”		
									&& pWordPosLen->nextFeaPos     == ChsPoS_n
									&& pWordPosLen->nextNextFeaPos != ChsPoS_n)
								{
									if( pWordPosLen->nextNextFeaPos ==  ChsPoS_u)
									{
										//移到：后后词之后
										bBestNeedMove = emTrue;
										nMoveZiPos += pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen + pWordPosLen->nextNextFeaLen;	
										
										//例如： 才在周恩来◆和王稼祥等◆人的积极主张下◆召开会议
										
									}
									else
									{
										//移到：后词之后
										bBestNeedMove = emTrue;
										nMoveZiPos += pWordPosLen->curFeaLen + pWordPosLen->nextFeaLen;	
										
										//例如： 在王钦若等◆人推波助澜的辅佐之下 
									}
								}
								else
								{
									//最优点不移动
									bBestNeedMove = emFalse;
								}
							}


							//真正移动
							if( bBestNeedMove == emTrue)
								iReturnBest += nMoveZiPos;
						}
				//*********************************************************************************************************
				//*********************************** 判断：最优点是否需要在本特征词的基础上移动：结束 ********************
				//*********************************************************************************************************


						if( bBestNeedMove == emFalse && iReturnBest>0)		//若最优点不需要在本特征词的基础上移动，则设置本特征词相关的最优点
						{			
							if(    (iReturnBest + FeatureWords[curSelWordIndex].WordBestInjust) <= 3 		//专门为特征词：“位于”
								&& GetWord(pRhythmRT->m_cText,iReturnBest*2)   == (emInt16)0xcebb		//“位”
								&& GetWord(pRhythmRT->m_cText,iReturnBest*2+2) == (emInt16)0xd3da )		//“于”
							{
								iReturnBest += FeatureWords[curSelWordIndex].WordCount;			//最优点位置调整，强行调整至特征词后面（当最优点在句首的几个字内）
							}
							else
								iReturnBest += FeatureWords[curSelWordIndex].WordBestInjust;	//最优点位置调整，调整至特征词临近的PPH边界	
						}

						//如果需改变的最优点不是已划分好的PPH边界，则真正改变最优点，否则不改变
						if( iReturnBest >0 )
						{
							//需设置为PPH的边界(即改变后的优先点)原来不是PW边界，需先修改成PW边界
							if( GetBoundaryBefore(pRhythmRT, iReturnBest) < BdPW)				//现边界不是PW边界		
							{
								if( pRhythmRT->m_tRhythmBeforePW[iReturnBest]>0 )				//但现边界是语法词边界
								{
									for( k = (iReturnBest-1);k>0; k--)
									{
										if(  pRhythmRT->m_tRhythm[k]  > 0 )
										{
											pRhythmRT->m_tRhythm[iReturnBest] = (pRhythmRT->m_tRhythm[k]&0x0F) - (iReturnBest - k);		// (pRhythmRT->m_tRhythm[j]&0x0F)为韵律词长
											pRhythmRT->m_tRhythm[k]           = iReturnBest - k;									
											break;
										}
									}	
									SetBoundaryBefore(pRhythmRT, iReturnBest,BdPW);
								}
								else							
									iReturnBest = 0;								//若不是语法词边界，则找到的特征词默认无效
							}	
							
							//改变边界代价，进行相对值调整
							if( iReturnBest != 0 )
							{								
								nStartIndex=0;
								nEndIndex=0;

								//改变最优点代价：根据不同的优先级，进行不同的改变 （原来方案是不论优先级，统一除4倍）
								if( curProb== PROB_MAX )
									pCost[iReturnBest].wPphCost /= 6;
								else if( curProb== PROB_MID )
									pCost[iReturnBest].wPphCost /= 4;
								else if( curProb== PROB_MIN )
									pCost[iReturnBest].wPphCost /= 2;


								//前后词代价：升高3倍(前后2个字及以内)
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
/*************************************** PPH边界划分后处理函数：结束 ******************************************************/
/**************************************************************************************************************************/


/******************************************************************************
* 函数名        : Front_Rhythm_PPH  （主函数）
* 描述          : 划分PPH
* 参数           : [in]     pRhythmRes   - 韵律模块资源对象
*                      : [in/out] pRhythmRT - 韵律实时对象
* 返回               : <无>

******************************************************************************/
void  Front_Rhythm_PPH( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT		/* 韵律实时对象 */
)
{
	PCostInfo		pCost;			/* 代价数组指针，动态分配内存 */	

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pRhythmRes && pRhythmRT);

	if ( pRhythmRes->m_subSyntax.m_pC45Res && pRhythmRes->m_subSyntax.m_pPPHRes )
	{
		/* 分配内存 */
#if DEBUG_LOG_SWITCH_HEAP
		pCost = (PCostInfo)emHeap_AllocZero(sizeof(TCostInfo) * CE_MAX_SYLLABLE, "PPH代价：《韵律模块  》");
#else
		pCost = (PCostInfo)emHeap_AllocZero(sizeof(TCostInfo) * CE_MAX_SYLLABLE);
#endif

		emAssert(pCost);

		/* 得到所有音节的PW和PPH代价 */
		if ( CalcSplitCost(pRhythmRes, pRhythmRT, pCost) )
		{
			emUInt8		allNodes;			/* 所有音节个数 */
			emUInt8		curPos;				/* 遍历时的位置 */

			allNodes = GetSylCount(pRhythmRT);

#if PPH_AFTER_DEAL							//PPH边界划分后处理

			//改变本句中两个特定词性间的相关代价
			SpecialWordChangeCost(pRhythmRT, pCost,allNodes);			

			//改变本句中特征词相关的代价
			PPHFeatureWordsHandle(pRhythmRT, pCost,allNodes);
#endif					

			for ( curPos = 0; curPos < allNodes; )
			{
				emUInt8	PPHPos[3];			/* PPH切分点位置 */
				emUInt8	tempPos;
				emUInt8		j;								

				/* 取6字以上16字以内的一个最优点 */
				tempPos = FindCurSnap(pRhythmRT, pCost, curPos);

				//开始决策PPHPos边界数组
				PPHPos[0] = PPHPos[1] = PPHPos[2] = 0;

				/* 取1~4字内的最优点 */
				GetOtherGandidate(pCost, curPos, tempPos, PPHPos);

				if ((tempPos - curPos) > 11)
				{/* 处理大于11字的情况,有点数要等于3个 */
					Decision2Or3PPH(pRhythmRes, pCost, curPos, tempPos, PPHPos);
				}
				else
				{/* 其他的是处理小于11字的情况, 包含有点优点数为1个0的情况 */
					Decision1Or2PPH(pRhythmRes, pCost, curPos, tempPos, PPHPos);
				}

#if PPH_AFTER_DEAL			//PPH边界划分后处理

				//改变特定句型的韵律切分		例如：《车辆位于****附近》等
				SpecialSenChangeCost(pRhythmRT, allNodes, PPHPos);	
#endif

				/* 根据PPHPos边界数组的最终结果，设置PPH边界 */
				for ( j = 0; j < 3; ++ j )
				{
					if ( 0 == PPHPos[j] )
						break;

					if ( PPHPos[j] < allNodes )
					{	

#if PPH_AFTER_DEAL								//PPH边界划分后处理
						emUInt8 nPrevPPHIndex;
						if( j > 0 )
							nPrevPPHIndex = PPHPos[j-1];
						else
							nPrevPPHIndex = curPos;

						// 总体规则：PPH边界是否需要少量移动：最多移动2个字
						//若PPH边界前是这些词性（d,c,p,r,u），可能发生移动
						PPHPos[j] = PPHNeedLitterMove(pRhythmRT, PPHPos[j], nPrevPPHIndex,allNodes);						
#endif
						//设置PPH边界
						SetBoundaryBefore(pRhythmRT, PPHPos[j], BdBreath);
					}
				}
				/* 从最后一个PPH位置再次向后遍历 */
				curPos = PPHPos[-- j];
			}


#if PPH_AFTER_DEAL					//PPH边界划分后处理

			//连续短韵清除		例如：【这反映了地价◆与其它◆市场要素◆在价格水平上是协调的】  ->  【这反映了地价◆与其它市场要素◆在价格水平上是协调的】
			MoreShortPPHClear(pRhythmRT, allNodes);			
#endif

		}				
		

		/* 释放内存 */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pCost, sizeof(TCostInfo) * CE_MAX_SYLLABLE, "PPH代价：《韵律模块  》");
#else
		emHeap_Free(pCost, sizeof(TCostInfo) * CE_MAX_SYLLABLE);
#endif
	}
}

