#include "emPCH.h"

#include "Front_ProcHZRhythm_New_PW.h"


emPRes	g_ResPWRule;
emPRes	g_ResPPHC45;
emPRes	g_ResPPHRatio;


/*
 *	词性调整运行时数据
 */
typedef struct tagPwRT CPwRT, emPtr PPwRT;

struct tagPwRT
{
	/* 二级索引(为访问-1有一个错位) */
	emUInt8	m_pIndex[CE_MAX_SYLLABLE + 4];
	emUInt8	m_nIndexCount;

	/* 节点数据 */
	emInt16		m_pWeight[CE_MAX_SYLLABLE + 1];	/* 单字的右向拼接权值 */
	emUInt8		m_pFlag[CE_MAX_SYLLABLE + 1];
	emInt8		m_pRhythm[CE_MAX_SYLLABLE + 1];
	emUInt8		m_pPoS[CE_MAX_SYLLABLE + 1];	/* 单字的右向拼接词性 */

	/* 规则处理数据 */
	emInt16		m_nVerdictWeight;	/* 拼接权值 */
	emUInt8		m_nVerdictFlag;		/* 标识 */
	emInt8		m_tVerdictRhythm;	/* 建议的节奏 */
	emInt8		m_tVerdictPoS;		/* 拼接词性 */
	emInt8		m_tVerdictPoS2;		/* 拼接词性2 */
};

#define SetIndex(pPwRT, j, tIndex)	((void)(((pPwRT)->m_pIndex[(j)+1])=(tIndex)))
#define GetIndex(pPwRT, j)			((pPwRT)->m_pIndex[(j)+1])
#define GetIndexPtr(pPwRT, j)			((pPwRT)->m_pIndex+(j)+1)

#define SetIndexCount(pPwRT, tCount)	((void)(((pPwRT)->m_nIndexCount)=(tCount)))
#define GetIndexCount(pPwRT)			((pPwRT)->m_nIndexCount)

#define SetWeight(pPwRT, tIndex, nWeight)	((void)(((pPwRT)->m_pWeight[tIndex])=(nWeight)))
#define GetWeight(pPwRT, tIndex)			((pPwRT)->m_pWeight[tIndex])


/*	函数 */
#define F_A	0	/* 在节点在句中的位置 h：代表头; t：代表尾 */
#define F_b	1	/* 字或词本身 */
#define F_h	2	/* 词头 */
#define F_t	3	/* 词尾 */
#define F_x	4	/* 词性 */
#define F_S	5	/* 词性集合即从所有的多词类中 */
#define F_u	6	/* 扩展词性 */
#define F_W	7	/* 成词后接索引串，例如03表示第0编号汉字和第3编号汉字成词 必须是最后一个条件 */
#define F_y	8	/* 当前所辖词的音节个数 */
#define F_v	9	/* 与y相似，但区别是此条件可以对轻声进行识别 */

/* 字符串相关 */
#define	O_In		0	/* { 被包含于 */
#define	O_Include	1	/* } 包含 */
#define	O_Well		4	/* # 字符串等于 */

/* 字符串节点相关 */
#define	O_Node		8	/* $,@ 字符串与(节点)(不)相同 */

/*	拼接类型  */
#define CbWord	0x01	/* 拼接成词 */
#define CbPW	0x02	/* 仅仅拼接PW,词结果不变 */

/* 二元规则 */
#define BothSingle	0x03


/***************	日志相关   开始  ********************************************/

#if DEBUG_LOG_SWITCH_PW_COMBINE


typedef emPointer emTLogClass, emPtr emPLogClass;

emStatic emTLogClass g_tLogClass;
emStatic emBool g_bPoSRule;
emStatic emCharA g_szRule[1024];

emStatic emConst emCStrA g_szPoSName[] =
{
	"XX",	
	"a",	"c",	"d",	"f",	"m",	"n",	"ng",	"p",	"q",	"s",	
	"t",	"u",	"v",	"h",	"k",	"o",	"r",	"R",	"y",	"ngp"
};


void emCall emLogCreate( emPLogClass pLogClass, emCStrA szClass )
{
	emCharA szPath[1000];
	sprintf(szPath, "log/日志_%s.log", szClass);
	*pLogClass = (emTLogClass)fopen(szPath, emTextA("wt"));
	emAssert(*pLogClass);
}


void emCall emLogRelease( emPLogClass pLogClass )
{
	if ( *pLogClass )
	{
		fclose((FILE emPtr)*pLogClass);
		*pLogClass = (emTLogClass)emNull;
	}
}

void emCall emLog0(emPLogClass pLogClass, emCStrA szFormat)	
{
	if ( *pLogClass )
	{
		fprintf((FILE emPtr)*pLogClass, szFormat);
		fflush((FILE emPtr)*pLogClass);
	}
}

void emCall emLog1(emPLogClass pLogClass, emCStrA szFormat,emCStrA szArg1)
{
	if ( *pLogClass )
	{
		fprintf((FILE emPtr)*pLogClass, szFormat,szArg1);
		fflush((FILE emPtr)*pLogClass);
	}
}

void emCall emLog2(emPLogClass pLogClass, emCStrA szFormat,emCStrA szArg1,emCStrA szArg2)
{
	if ( *pLogClass )
	{
		fprintf((FILE emPtr)*pLogClass, szFormat,szArg1,szArg2);
		fflush((FILE emPtr)*pLogClass);
	}
}

void emCall emLog3(emPLogClass pLogClass, emCStrA szFormat,emCStrA szArg1,emCStrA szArg2,emCStrA szArg3)
{
	if ( *pLogClass )
	{
		fprintf((FILE emPtr)*pLogClass, szFormat,szArg2,szArg1,szArg3);
		fflush((FILE emPtr)*pLogClass);
	}
}


#define emLog_ClearRule()	(*g_szRule = '\0')
#define emLog_SetPoSRule(bPoSRule)	(g_bPoSRule=(emBool)(bPoSRule))
#define emLog_AppendRule0(c)			sprintf(g_szRule+emStrLenA(g_szRule),(c))
#define emLog_AppendRule1(c,p1)		sprintf(g_szRule+emStrLenA(g_szRule),(c),(p1))
#define emLog_AppendRule2(c,p1,p2)	sprintf(g_szRule+emStrLenA(g_szRule),(c),(p1),(p2))
#define emLog_AppendRule3(c,p1,p2,p3)	sprintf(g_szRule+emStrLenA(g_szRule),(c),(p1),(p2),(p3))

emCStrA emCall emLogGetPoSName(emUInt8 tPoS)
{
	emAssert(tPoS < emArrayCount(g_szPoSName));
	if ( tPoS < emArrayCount(g_szPoSName) )
		return g_szPoSName[tPoS];
	else
		return "(error)";
}

emStatic void emCall emLogCopyRuleString( emPUInt8 ptr, emPCUInt8 pText, emSize nLen )
{
	if ( g_bPoSRule )
	{
		emSize i;
		for ( i = 0; i < nLen; ++ i )
		{
			emPCUInt8 szPoS;
			szPoS = emLogGetPoSName((emUInt8)pText[i]);
			if ( 'g' == szPoS[1] && 'n' == szPoS[0] )
			{
				if ( 'p' == szPoS[2] )
					ptr[i] = (emUInt8)'}';
				else
					ptr[i] = (emUInt8)'|';
			}
			else
				ptr[i] = (emUInt8)*szPoS;
		}
	}
	else
	{
		emMemCpy(ptr, pText, nLen);
	}
}

emStatic void emCall emLogWeight( PPwRT pPwRT )
{
	emUInt8 n, j, tIndex;
	emInt16 nWeight;

	n = GetIndexCount(pPwRT) - 1;

	for ( j = 0; j < n; ++ j )
	{
		tIndex = GetIndex(pPwRT, j);
		nWeight = GetWeight(pPwRT, tIndex);
		emLog2(&g_tLogClass, "(%2u)%2ld ", tIndex, nWeight);
	}
	
}

emStatic void emCall emLogBoundary(PRhythmRT pRhythmRT)
{
	emUInt8 n, j;
	emInt16 nBoundary;

	n = pRhythmRT->m_nSylCount;

	for ( j = 0; j < n; ++ j )
	{
		nBoundary = pRhythmRT->m_tSylInfo[j].m_tBoundary;
		emLog2(&g_tLogClass, "(%2u)%2ld ", j, nBoundary);
	}		
}

emStatic void emCall emLogRhythm(PRhythmRT pRhythmRT)
{
	emUInt8 n, j;
	emInt16 nRhythm;

	n = pRhythmRT->m_nSylCount;

	for ( j = 0; j < n; ++ j )
	{
		nRhythm = pRhythmRT->m_tRhythm[j];
		emLog2(&g_tLogClass, "(%2u)%2ld ", j, nRhythm);
	}
}



emStatic void emCall emLogTextAddBoundary(emPLogClass pLogClass, PRhythmRT pRhythmRT)
{
	emUInt8 n, j,t1;
	emInt16 nBoundary;
	emInt16 nRhythm, nRhythm_L4,nRhythm_H4,nMaxRhythm;

#if   DEBUG_LOG_SWITCH_IS_OUTPUT_POS
	emInt16 nPos=0;
#endif
	
	emCharA szWord[EM_CE_TEXT_SIZE + 1] = { 0 };

	n = pRhythmRT->m_nSylCount;
	t1 = 1;

	for ( j = 0; j < n; ++ j )
	{
		t1--;
		nBoundary = pRhythmRT->m_tSylInfo[j].m_tBoundary;
		nRhythm = pRhythmRT->m_tRhythm[j];
		if( j>0 )
		{
			if( nBoundary == BdPW )
			{
				emLog1(pLogClass, "%s", "｜" );
			}
			if( nBoundary == BdBreath )
			{
				emLog1(pLogClass, "%s", "●" );
			}
		}	

		emMemCpy(szWord, pRhythmRT->m_cText + j * 2, 2);
		emLog1(pLogClass, "%s", szWord );	

#if   DEBUG_LOG_SWITCH_IS_OUTPUT_POS
		if(pRhythmRT->m_tSylInfo[j].m_tPoS>0)
		{
			nPos  = pRhythmRT->m_tSylInfo[j].m_tPoS;
		}
		if(pRhythmRT->m_tSylInfo[j+1].m_tPoS>0)
		{
			emLog1(pLogClass, "%s", emLogGetPoSName(nPos) );
		}
#endif

		nRhythm_L4 = nRhythm & 0x000F;
		
		if( nRhythm >0 && nRhythm!=0xff && t1 == 0 )
		{
			t1 = nRhythm_L4;
			nMaxRhythm = nRhythm_L4;
			nRhythm_H4 = (nRhythm & 0x00F0)>>4;
		}			

		if( nRhythm_H4 > 0 && nRhythm_H4 != 0x0F )
		{
			if(  (( nRhythm_H4 >> (  nMaxRhythm - t1  ) ) & 1) == 1 )
			{
				emLog1(pLogClass, "%s", "·" );		
			}
		}
	}
#if   DEBUG_LOG_SWITCH_IS_OUTPUT_POS
	emLog1(pLogClass, "%s", emLogGetPoSName(nPos) );
#endif

}



emStatic void emCall emLogTextAddRhythm(emPLogClass pLogClass,PRhythmRT pRhythmRT)
{
	emUInt8 n, j, t1;
	emInt16 nRhythm, nRhythm_L4,nRhythm_H4,nMaxRhythm;

#if   DEBUG_LOG_SWITCH_IS_OUTPUT_POS
	emInt16 nPos=0;
#endif

	emCharA szWord[EM_CE_TEXT_SIZE + 1] = { 0 };

	n = pRhythmRT->m_nSylCount;
	t1 = 1;

	for ( j = 0; j < n; ++ j )
	{
		t1--;
		nRhythm = pRhythmRT->m_tRhythm[j];
		if( j>0 )
		{
			if( t1 == 0 )
			{
				emLog1(pLogClass, "%s", "｜" );
			}
		}			
		emMemCpy(szWord, pRhythmRT->m_cText + j * 2, 2);
		emLog1(pLogClass, "%s", szWord );	

#if   DEBUG_LOG_SWITCH_IS_OUTPUT_POS
		if(pRhythmRT->m_tSylInfo[j].m_tPoS>0)
		{
			nPos  = pRhythmRT->m_tSylInfo[j].m_tPoS;
		}
		if(pRhythmRT->m_tSylInfo[j+1].m_tPoS>0)
		{
			emLog1(pLogClass, "%s", emLogGetPoSName(nPos) );
		}
#endif

		nRhythm_L4 = nRhythm & 0x000F;
		
		if( nRhythm >0 && nRhythm!=0xff && t1 == 0 )
		{
			t1 = nRhythm_L4;
			nMaxRhythm = nRhythm_L4;
			nRhythm_H4 = (nRhythm & 0x00F0)>>4;
		}			

		if( nRhythm_H4 > 0 && nRhythm_H4 != 0x0F )
		{
			if(  (( nRhythm_H4 >> (  nMaxRhythm - t1  ) ) & 1) == 1 )
			{
				emLog1(pLogClass, "%s", "·" );		
			}
		}
	}
#if   DEBUG_LOG_SWITCH_IS_OUTPUT_POS
	emLog1(pLogClass, "%s", emLogGetPoSName(nPos) );
#endif

}

	
#else

	#define emLog_ClearRule()							((void)0)
	#define emLog_SetPoSRule(bPoSRule)					((void)0)

	#define emLog_AppendRule0(c)						((void)0)
	#define emLog_AppendRule1(c,p1)						((void)0)
	#define emLog_AppendRule2(c,p1,p2)					((void)0)
	#define emLog_AppendRule3(c,p1,p2,p3)				((void)0)

	#define emLogCopyRuleString( p1,p2 ,n)				((void)0)



	#define emLogWeight(pPwRT)							((void)0)

	#define emLogBoundary(pRhythmRT)					((void)0)
	#define emLogRhythm(pLogClass,pRhythmRT)			((void)0)
	#define emLogTextAddBoundary(pLogClass, pRhythmRT)	((void)0)
	#define emLogTextAddRhythm(pRhythmRT);				((void)0)

	#define emLogCreate( p,c )
	#define emLogRelease( p )
	#define emLog0(p, c)								((void)0)
	#define emLog1(p, c, c1)							((void)0)
	#define emLog2(p, c, c1, c2)						((void)0)
	#define emLog3(p, c, c1, c2, c3)					((void)0)

#endif			// ! DEBUG_LOG_SWITCH_PW_COMBINE

/***************	日志相关   结束  ********************************************/






/*	文本匹配  */

typedef emBool (emCall emPtr THandleStringProc)( emPCUInt8 pText1, emSize nText1Len,
												emPCUInt8 pText2, emSize nText2Len );

emStatic emBool emCall HandleString_In(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{

#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	return emNull != emStrStr(pText2, nText2Len, pText1, nText1Len);
}

emStatic emBool emCall HandleString_Include(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{
#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	return emNull != emStrStr(pText1, nText1Len, pText2, nText2Len);
}

emStatic emBool emCall HandleString_Out(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{
#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	return emNull == emStrStr(pText2, nText2Len, pText1, nText1Len);
}

emStatic emBool emCall HandleString_Exclude(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{
#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	return emNull == emStrStr(pText1, nText1Len, pText2, nText2Len);
}

emStatic emBool emCall HandleString_Power(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{
	emPCUInt8 pNext, pEnd;

#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	pEnd = pText2 + nText2Len;

	for ( ; ; )
	{
		pNext = emStrChar(pText2, nText2Len, 0x60 /*'`'*/);

		if ( !pNext )
			return emNull != emStrStr(pText1, nText1Len, pText2, nText2Len);

		if ( emNull != emStrStr(pText1, nText1Len, pText2, pNext - pText2) )
			return emTrue;

		pText2 = pNext + 1;
		nText2Len = pEnd - pText2;
	}

	return emFalse;
}

emStatic emBool emCall HandleString_Well(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{
#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	return emIsEqual(emStrComp(pText1, nText1Len, pText2, nText2Len));
}

emStatic emBool emCall HandleString_Not(
	emPCUInt8 pText1, emSize nText1Len, emPCUInt8 pText2, emSize nText2Len )
{
#if DEBUG_LOG_SWITCH_PW_COMBINE

	emUInt8 ptr[1024];
	emLogCopyRuleString(ptr, pText2, nText2Len);
	ptr[nText2Len] = '\0';
	emLog_AppendRule0(ptr);

#endif

	LOG_StackAddr(__FUNCTION__);

	return !emIsEqual(emStrComp(pText1, nText1Len, pText2, nText2Len));
}

emStatic emConst THandleStringProc g_pfnHandleString[] =
{
	HandleString_In,		/* { 被包含于 */
	HandleString_Include,	/* { 被包含于 */
	HandleString_Out,		/* ~ 不包含于 */
	HandleString_Exclude,	/* _ 不包含 */
	HandleString_Well,		/* # 字符串等于 */
	HandleString_Not,		/* ! 字符串不等于 */
	emNull,
	HandleString_Power		/* ^ 字符交集不为空 */
};


/*	值匹配  */

typedef emBool (emCall emPtr THandleValueProc)( emUInt16 nValue1, emUInt16 nValue2 );

emStatic emBool emCall HandleValue_Less( emUInt16 nValue1, emUInt16 nValue2 )
{

	LOG_StackAddr(__FUNCTION__);

	return nValue1 < nValue2;
}

emStatic emBool emCall HandleValue_More( emUInt16 nValue1, emUInt16 nValue2 )
{

	LOG_StackAddr(__FUNCTION__);

	return nValue1 > nValue2;
}

emStatic emBool emCall HandleValue_Equal( emUInt16 nValue1, emUInt16 nValue2 )
{

	LOG_StackAddr(__FUNCTION__);

	return nValue1 == nValue2;
}

emStatic emBool emCall HandleValue_BitInc( emUInt16 nValue1, emUInt16 nValue2 )
{

	LOG_StackAddr(__FUNCTION__);

	nValue1 = ~nValue1;
	nValue1 &= nValue2;
	return 0 == nValue1;
}

emStatic emBool emCall HandleValue_BitDec( emUInt16 nValue1, emUInt16 nValue2 )
{

	LOG_StackAddr(__FUNCTION__);

	nValue1 &= nValue2;
	return 0 == nValue1;
}

emStatic emConst THandleValueProc g_pfnHandleValue[] =
{

	HandleValue_Less,	/* < 小于 */
	HandleValue_BitInc,	/* & 比特测试包含 */
	HandleValue_More,	/* > 大于 */
	HandleValue_BitDec,	/* - 比特排除 */
	HandleValue_Equal	/* = 值等于 */
};


#define FISRT_BG	0xB0
#define FISRT_ED	0xF7
#define SECOND_BG	0xA1
#define SECOND_ED	0xFE


void emCall TextLowByteAdd1(emPUInt8 pText,emSize nLen)
{
	emInt16	i;

	LOG_StackAddr(__FUNCTION__);

	for(i = 1;i < nLen; i=i+3)
	{
		*( pText + i ) = *( pText + i )  + 1;
	}
}


/* 匹配条件 */
emStatic emBool emCall HandlePWRule( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,			/* 韵律实时对象 */
	PPwRT			pPwRT,				/* 运行时数据 */
	emUInt8		j,						/* 起始位置 */
	emPCUInt8		pRule,				/* 条件串 */
	emSize			nLen				/* 条件串长度 */
)
{
	emPCUInt8 pRuleEnd;
	emBool isOperHighBitEquaOne;

	LOG_StackAddr(__FUNCTION__);

	emAssert(nLen >= 2);

	emLog_ClearRule();

	/* 计算条件结束位置 */
	pRuleEnd = pRule + nLen;

	/* 遍历每个条件 */
	do
	{
		emInt8 nPos;
		emUInt8 tIndex, t;

		emUInt8 nFunc, nOper;
		emUInt8 nVal, nVal0;
		emPCUInt8 pText;

		/* 提取条件作用的节点和操作数 */
		nVal = *pRule++;
		nPos = (emInt8)(nVal & 0x07) - 1;
		tIndex = GetIndex(pPwRT, j + nPos);
		nVal >>= 3;

		/* 提取条件的函数和操作符 */
		nOper = *pRule++;
		nFunc = nOper & 0x0F;
		nOper >>= 4;

		isOperHighBitEquaOne = FALSE;
		if( ( nOper & 0x08) == 0x08 )		//nOper的最高位为1
		{
			isOperHighBitEquaOne = TRUE;
		}

#if DEBUG_LOG_SWITCH_PW_COMBINE

		emLog_AppendRule1("%d", nPos);
		emLog_SetPoSRule(F_x == nFunc);

		if ( F_x == nFunc && O_Well == nOper )
		{
			emLog_AppendRule0("X");
		}
		else if ( F_h == nFunc && O_Well == nOper )
		{
			emLog_AppendRule0("H");
		}
		else
		{
			emStatic emConst emCharA g_szFunc[] = "AbhtxSuWyv";
			emLog_AppendRule1("%c", g_szFunc[nFunc]);

			if ( F_A < nFunc && F_S >= nFunc )
			{
				emStatic emConst emCharA g_szOper1[] = "{}~_#!\0^\0\0\0\0$@";
				emLog_AppendRule1("%c", g_szOper1[nOper]);
			}
			else if ( F_W != nFunc )
			{
				emStatic emConst emCharA g_szOper2[] = "<&>-=";
				emLog_AppendRule2("%c%d", g_szOper2[nOper], nVal);
			}
			else
			{
				emLog_AppendRule2("%d%d", nVal & 7, (nVal & 7) + (nVal >> 3));
			}
		}

#endif

		/* 索引越标 */
		if ( SylSuffixNull == tIndex )
		{
			/* 对这些操作符直接判为不匹配 */
			switch ( nOper & 0x07 )
			{
			case O_In:
			case O_Include:
			case O_Well:
				return emFalse;
			}

			/* 对指定文本操作的操作符 */
			if ( nFunc > F_A && nFunc <= F_S && 0 == (nOper & O_Node) )
			{
				/* 扩展一个字节存储文本长度? */
				if ( !nVal )
				{
					/* 获取扩展的文本长度 */
					nVal = *pRule++;
					emAssert(pRule <= pRuleEnd);
				}

				/* 跳过条件指定的文本 */
				pRule += nVal;
				emAssert(pRule <= pRuleEnd);
			}
		}
		else if ( nFunc > F_A && nFunc <= F_S )
		{
			/* 对节点文本的操作 */
			if ( nOper & O_Node )
			{
				/* 归并操作符 */
				nOper &= 7;

				/* 获取相关节点的音节索引 */
				t = GetIndex(pPwRT, j + nVal);

				/* 获取相关节点的文本 */
				pText = GetText(pRhythmRT, t);
				nLen = GetPWTextLen(pRhythmRT, t);
			}
			/* 对指定文本的操作 */
			else
			{
				/* 文本长度存储在操作数中? */
				if ( nVal )
				{
					/* 文本长度就是操作数 */
					nLen = nVal;
				}
				/* 扩展一个字节存储文本长度 */
				else
				{
					/* 获取扩展的文本长度 */
					nLen = *pRule++;
					emAssert(pRule <= pRuleEnd);
				}

				/* 获取条件指定的文本 */
				pText = pRule;
				pRule += nLen;
				emAssert(pRule <= pRuleEnd);
			}

			/* 根据函数匹配条件 */
			switch ( nFunc & 0x07 )
			{
			case F_b: /* PW本身的文本 */
				if( isOperHighBitEquaOne == FALSE)			
				{
					TextLowByteAdd1((emPUInt8)pText,nLen);				//文本低字节增1
				}
				if ( !g_pfnHandleString[nOper]((emPCUInt8)GetText(pRhythmRT, tIndex), GetPWTextLen(pRhythmRT, tIndex), pText, nLen) )
					return emFalse;
				break;

			case F_h: /* 词头 */
				if( isOperHighBitEquaOne == FALSE)			
				{
					TextLowByteAdd1((emPUInt8)pText,nLen);				//文本低字节增1
				}
				if ( !g_pfnHandleString[nOper]((emPCUInt8)GetText(pRhythmRT, tIndex), 2, pText, nLen) )
					return emFalse;
				break;

			case F_t: /* 词尾 */
				if( isOperHighBitEquaOne == FALSE)			
				{
					TextLowByteAdd1((emPUInt8)pText,nLen);				//文本低字节增1
				}
				if ( !g_pfnHandleString[nOper]((emPCUInt8)GetText(pRhythmRT, tIndex) + (GetPWTextLen(pRhythmRT, tIndex) - 2), 2, pText, nLen) )
					return emFalse;
				break;

			case F_x: /* 词性 */
				/* 拼接韵律词的没有词性 */
				if ( TestFrontFlag(pRhythmRT, tIndex, FrontFlag_CombinedRhythm) )
					return emFalse;

				/* 非词头的没有词性 */
				if ( !IsWordHead(pRhythmRT, tIndex) )
					return emFalse;

				if ( !g_pfnHandleString[nOper](&GetPoS(pRhythmRT, tIndex), 1, pText, nLen) )
					return emFalse;

				break;

			case F_S: /* 词性集合(即从所有的多词类中) */
				{
					return emFalse;
				}
				break;

			default: /* 非法 */
				emAssert(emFalse);
				return emFalse;
			}
		}
		else if ( F_W != nFunc )
		{
			switch ( nFunc )
			{
			case F_A: /* 句中位置 */
				if ( 0 == j + nPos )
					nVal0 = 0; /* 句头 */
				else if ( GetIndexCount(pPwRT) - 1 == j + nPos )
					nVal0 = 1; /* 句尾 */
				else
					nVal0 = 2; /* 句中 */
				break;

			case F_u: /* 词性扩展 */
				nVal0 = (pRhythmRT)->m_tFrontFlag[tIndex];
				break;

			case F_v: /* 音节个数(不含结尾的轻声) */
				nVal0 = GetPWLen(pRhythmRT, tIndex);

				/* 跳过结尾的轻声 */
				while ( nVal0 && ChsTone_Light == GetTone(pRhythmRT, tIndex + nVal0 - 1) )
					-- nVal0;

				break;

			case F_y: /* 音节个数 */
				nVal0 = GetPWLen(pRhythmRT, tIndex);
				break;

			default: /* 非法 */
				emAssert(emFalse);
				return emFalse;
			}

			/* 匹配值 */
			if ( !g_pfnHandleValue[nOper](nVal0, nVal) )
				return emFalse;
		}
		/* 成词?(词典词) */
		else
		{
			return emFalse;
		}

		if ( pRule < pRuleEnd )
			emLog_AppendRule0(" ");
	}
	while ( pRule < pRuleEnd );

	return emTrue;
}

/* 匹配二元规则 */
emStatic void emCall HandleRuleTwo( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,			/* 韵律实时对象 */
	PPwRT			pPwRT,				/* 运行时数据 */
	emUInt8			j,					/* 起始位置 */
	emUInt8			bLightTail			/* 是否后接轻声(0-否, 1-是, 2-未决) */
)
{
	emUInt8	t;
	emUInt8		nLen0, nLen1;
	emUInt8		nMask;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;
	emUInt8		nPos;

	LOG_StackAddr(__FUNCTION__);

	/* 获取第1个节点的索引 */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t )
		return;

	/* 清除未求解标志 */
	if ( -1 == GetWeight(pPwRT, t) )
		SetWeight(pPwRT, t, 0);

	/* 获取第1个节点的长度 */
	nLen0 = GetPWLen(pRhythmRT, t);

	/* 获取第2个节点的索引 */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return;

	/* 对于未决的轻声尾进行补充判断 */
	if ( 2 == bLightTail )
	{
		if ( 1 == GetPWLen(pRhythmRT, t) && ChsTone_Light == GetTone(pRhythmRT, t) )
			bLightTail = 1;
		else
			bLightTail = 0;
	}

	/* 获取第2个节点的长度 */
	nLen1 = GetPWLen(pRhythmRT, t);


	/* 计算两个节点的单字掩码 */
	nMask = 0;
	if ( nLen0 > 1 )	nMask |= 0x01;
	if ( nLen1 > 1 )	nMask |= 0x02;

	/* 二元规则必须有一个单字 */
	if ( 3 == nMask )
		return;

	/* 没有长度超过6的二元规则 */
	emAssert(nLen0 + nLen1 <= 6);
	if ( nLen0 + nLen1 > 6 )
		return;

	/* 不是后接轻声情况 */
	if ( !bLightTail )
	{
		/* 没有长度超过5的二元规则不后接轻声 */
		if ( nLen0 + nLen1 > 5 )
			return;

		/* 长度5的二元规则第2个节点必须是单字 */
		if ( nLen0 + nLen1 == 5 && nLen1 > 1 )
			return;

		/* 没有第2个节点长度超过2的二元规则 */
		if ( nLen1 > 2 )
			return;

		/* 第1个节点必须有词性 */
		if ( !GetPoS(pRhythmRT, t) )
			return;
	}

	/* 获取后接第1个节点 */
	t = GetIndex(pPwRT, j + 2);

	/* 后接第1个节点存在? */
	if ( t != SylSuffixNull  )
	{
		/* 计算后接第1个节点的单字掩码 */
		if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
			nMask |= 0x04;

		/* 获取后接第2个节点 */
		t = GetIndex(pPwRT, j + 3);

		/* 后接第2个节点存在? */
		if ( t != SylSuffixNull )
		{
			/* 计算后接第2个节点的单字掩码 */
			if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
				nMask |= 0x08;
		}
	}

	/* 设置规则入口 */
	emRes_SetPos(pRhythmRes->m_subFootAdjust.m_pRes,
		pRhythmRes->m_subFootAdjust.m_pRuleTwoEntry);

	/* 获取规则条数 */
	n = pRhythmRes->m_subFootAdjust.m_nRuleTwoCount;

	/* 循环匹配各条规则 */
	for ( i = 0; i < n; ++ i )
	{
		/* 读取本条规则的长度 */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);

		/* 映射本条规则的数据 */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* 匹配掩码，用于判断单字分布是否满足规则要求 */
		if ( nMask & p[2] )
		{
			/* 不满足则跳过本条规则 */

			continue;
		}

		/* 匹配规则 */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 6, l - 6) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog3(&g_tLogClass, "匹配二元规则 %u, %lu: %s", j, i, g_szRule);
#endif

			/* 提取规则的权值 */
			nWeight = em16From8s(p);

			/* 获取比较权值的相对位置 */
			nPos = p[3] >> 4;
			t = GetIndex(pPwRT, j + nPos);

			/* 规则权值要大于设定值，否则必不匹配 */
			if ( nWeight <= GetWeight(pPwRT, t) )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)权值不够!!!\n\t", nWeight, GetWeight(pPwRT, t) );
#endif

				/* 权值不够则跳过本条规则 */

				return;
			}

			/* 规则要求拼接成词,而拼接韵律词不允许再拼接成词? */
			if ( (p[3] & 0x0F) == CbWord && TestFrontFlag(pRhythmRT, t, FrontFlag_CombinedRhythm) )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, " 不允许拼接成词!!!\n\t");
#endif

				/* 不允许拼接成词则跳过本条规则 */

				continue;
			}

#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t", nWeight, GetWeight(pPwRT, t) );
#endif

			/* 奖励2拼接 */
			if ( 0 == (BothSingle & (nMask >> nPos)) )
				nWeight += 5;

			/* 获取节点的位置 */
			t = GetIndex(pPwRT, j);

			/* 设置二元拼接权值 */
			SetWeight(pPwRT, t, nWeight);

			/* 记录规则的参数 */
			pPwRT->m_pFlag[t] = p[3] & 0x0F;
			pPwRT->m_pRhythm[t] = p[4];
			pPwRT->m_pPoS[t] = p[5];

			/* 匹配成功,退出 */

			return;
		}

		/* 准备匹配下一条规则 */

	}

	/* 后接轻声的必须向前拼接 */
	if ( bLightTail )
	{
		/* 获取第1个节点的索引 */
		t = GetIndex(pPwRT, j);

		/* 设置二元拼接权值 */
		SetWeight(pPwRT, t, 90);

		/* 记录规则的参数 */
		pPwRT->m_pFlag[t] = CbPW;
		pPwRT->m_pRhythm[t] = 0;
		pPwRT->m_pPoS[t] = 0;
	}
}

/*
 *	匹配三元规则
 */
emStatic emBool emCall HandleRuleThree( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT,		/* 运行时数据 */
	emUInt8		j				/* 起始位置 */
)
{
	emUInt8	t;
	emUInt8		n0, n1, n2;
	emUInt8		nMask;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;

	LOG_StackAddr(__FUNCTION__);

	/* 获取第2个节点的索引 */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第2个节点的长度 */
	n1 = GetPWLen(pRhythmRT, t);

	/* 获取第3个节点的索引 */
	t = GetIndex(pPwRT, j + 2);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第2个节点的长度 */
	n2 = GetPWLen(pRhythmRT, t);

	/* 获取第1个节点的索引(第1个节点必须放在后面,t要带下面用) */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第1个节点的长度 */
	n0 = GetPWLen(pRhythmRT, t);


	/* 计算单字掩码(三个中至少有两个单汉字，也就是说最多一个非单汉字节点) */
	if ( n0 > 1 )
	{
		if ( n1 > 1 || n2 > 1 )
			return emFalse;

		/* 第1个节点非单字 */
		nMask = 0x01;
	}
	else if ( n1 > 1 )
	{
		if ( n2 > 1 )
			return emFalse;

		/* 第2个节点非单字 */
		nMask = 0x02;
	}
	else if ( n2 > 1 )
	{
		/* 第3个节点非单字 */
		nMask = 0x04;
	}
	else
	{
		/* 三个节点均为单字 */
		nMask = 0;
	}

	/* 给4节奏一特别的机会 */
	if ( 4 == n0 )
	{
		/* 并且只允许 411 拼接 */
		if ( Rhythm22 != GetRhythm(pRhythmRT, t) )
			return emFalse;
	}
	else
	{
		/* 否则任何节点不得超过两个字 */
		if ( n0 > 2 )	return emFalse;
		if ( n1 > 2 )	return emFalse;
		if ( n2 > 2 )	return emFalse;
	}

	/* 获取后接第1个节点 */
	t = GetIndex(pPwRT, j + 3);

	/* 后接第1个节点存在? */
	if ( t != SylSuffixNull  )
	{
		/* 计算后接第1个节点的单字掩码 */
		if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
			nMask |= 0x08;
	}

	/* 设置规则入口 */
	emRes_SetPos(g_ResPWRule,
		pRhythmRes->m_subFootAdjust.m_pRuleThreeEntry);

	/* 获取规则条数 */
	n = pRhythmRes->m_subFootAdjust.m_nRuleThreeCount;

	/* 循环匹配各条规则 */
	for ( i = 0; i < n; ++ i )
	{
		/* 读取本条规则的长度 */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);

		/* 映射本条规则的数据 */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* 匹配掩码，用于判断单字分布是否满足规则要求 */
		if ( nMask & p[2] )
		{
			/* 不满足则跳过本条规则 */

			continue;
		}

		/* 匹配规则 */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 6, l - 6) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, "匹配三元规则 %lu: %s", i, g_szRule);
#endif

			/* 提取规则的权值 */
			nWeight = em16From8s(p);

			/* 规则权值要大于设定值，否则必不匹配 */
			if ( nWeight <= pPwRT->m_nVerdictWeight )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)权值不够!!!\n\t", nWeight, pPwRT->m_nVerdictWeight );
#endif

				/* 权值不够则跳过本条规则 */

				return emFalse;
			}

			/* 规则要求拼接成词,而拼接韵律词不允许再拼接成词? */
			if ( (p[3] & 0x0F) == CbWord && TestFrontFlag(pRhythmRT, t, FrontFlag_CombinedRhythm) )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, " 不允许拼接成词!!!\n\t");
#endif

				/* 不允许拼接成词则跳过本条规则 */

				continue;
			}

#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t", nWeight, pPwRT->m_nVerdictWeight );
#endif

			/* 记录规则的权值等参数 */
			pPwRT->m_nVerdictWeight = nWeight;
			pPwRT->m_nVerdictFlag = p[3] & 0x0F;
			pPwRT->m_tVerdictRhythm = p[4];
			pPwRT->m_tVerdictPoS = (emInt8)p[5];

			/* 匹配成功,退出 */

			return emTrue;
		}

		/* 准备匹配下一条规则 */

	}

	/* 未能匹配上 */
	return emFalse;
}

/* 匹配四元规则 */
emStatic emBool emCall HandleRuleFour( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT,		/* 运行时数据 */
	emUInt8		j				/* 起始位置 */
)
{
	/* 四元规则要求都是单字，所以不用nMask */
	emUInt8	t;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;

	LOG_StackAddr(__FUNCTION__);

	/* 获取第1个节点的索引 */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 节点的长度必须是1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* 获取第2个节点的索引 */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 节点的长度必须是1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* 获取第3个节点的索引 */
	t = GetIndex(pPwRT, j + 2);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 节点的长度必须是1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* 获取第4个节点的索引 */
	t = GetIndex(pPwRT, j + 3);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 节点的长度必须是1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* 设置规则入口 */
	emRes_SetPos(g_ResPWRule,
		pRhythmRes->m_subFootAdjust.m_pRuleFourEntry);

	/* 获取规则条数 */
	n = pRhythmRes->m_subFootAdjust.m_nRuleFourCount;

	/* 循环匹配各条规则 */
	for ( i = 0; i < n; ++ i )
	{
		/* 读取本条规则的长度 */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);
		
		/* 映射本条规则的数据 */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* 匹配规则 */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 6, l - 6) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, "匹配四元规则 %lu: %s", i, g_szRule);
#endif

			/* 提取规则的权值 */
			nWeight = em16From8s(p);

			/* 规则权值要大于设定值，否则必不匹配 */
			if ( nWeight <= pPwRT->m_nVerdictWeight )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)权值不够!!!\n\t",nWeight, pPwRT->m_nVerdictWeight );
#endif

				/* 权值不够则跳过本条规则 */

				return emFalse;
			}
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t",	nWeight, pPwRT->m_nVerdictWeight );
#endif

			/* 记录规则的权值等参数 */
			pPwRT->m_nVerdictWeight = nWeight;
			pPwRT->m_nVerdictFlag = p[3] & 0x0F;
			pPwRT->m_tVerdictRhythm = p[4];
			pPwRT->m_tVerdictPoS = (emInt8)p[5];

			/* 匹配成功,退出 */

			return emTrue;
		}

		/* 准备匹配下一条规则 */

	}

	/* 未能匹配上 */
	return emFalse;
}

/* 匹配复合规则 */
emStatic emBool emCall HandleRuleMore( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT,		/* 运行时数据 */
	emUInt8		j				/* 起始位置 */
)
{
	emUInt8		nMask;
	emUInt8	t;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;

	LOG_StackAddr(__FUNCTION__);

	/* 获取第1个节点的索引 */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第1个节点的长度 */
	n = GetPWLen(pRhythmRT, t);

	nMask = 0;

	/* 非单字节点 */
	if ( n > 1 )
	{
		/* 节点长度不能超过2 */
		if ( n > 2 )
			return emFalse;

		/* 修改单字掩码 */
		nMask |= 0x01;
	}

	/* 获取第2个节点的索引 */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第2个节点的长度 */
	n = GetPWLen(pRhythmRT, t);

	/* 非单字节点 */
	if ( n > 1 )
	{
		/* 节点长度不能超过2 */
		if ( n > 2 )
			return emFalse;

		/* 修改单字掩码 */
		nMask |= 0x02;
	}

	/* 1,2两个节点中必须有一个单字 */
	if ( 0x03 == nMask )
		return emFalse;

	/* 获取第3个节点的索引 */
	t = GetIndex(pPwRT, j + 2);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第3个节点的长度 */
	n = GetPWLen(pRhythmRT, t);

	/* 临时用于存储3,4两个节点的单字掩码 */
	l = 0;

	/* 非单字节点 */
	if ( n > 1 )
	{
		/* 节点长度不能超过2 */
		if ( n > 2 )
			return emFalse;

		/* 修改单字掩码 */
		l |= 0x04;
	}

	/* 获取第4个节点的索引 */
	t = GetIndex(pPwRT, j + 3);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* 节点上必须有词性 */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* 获取第4个节点的长度 */
	n = GetPWLen(pRhythmRT, t);

	/* 非单字节点 */
	if ( n > 1 )
	{
		/* 节点长度不能超过2 */
		if ( n > 2 )
			return emFalse;

		/* 修改单字掩码 */
		l |= 0x08;
	}

	/* 3,4两个节点中必须有一个单字 */
	if ( 0x0C == l )
		return emFalse;

	/* 合并单字掩码 */
	nMask |= l;

	/* 设置规则入口 */
	emRes_SetPos(g_ResPWRule,
		pRhythmRes->m_subFootAdjust.m_pRuleMoreEntry);

	/* 获取规则条数 */
	n = pRhythmRes->m_subFootAdjust.m_nRuleMoreCount;

	/* 循环匹配各条规则 */
	for ( i = 0; i < n; ++ i )
	{
		/* 读取本条规则的长度 */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);

		/* 映射本条规则的数据 */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* 匹配掩码，用于判断单字分布是否满足规则要求 */
		if ( nMask & p[2] )
		{
			/* 不满足则跳过本条规则 */

			continue;
		}

		/* 匹配规则 */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 5, l - 5) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, "匹配复合规则 %lu: %s", i, g_szRule);
#endif

			/* 提取规则的权值 */
			nWeight = em16From8s(p);

			/* 规则权值要大于设定值，否则必不匹配 */
			if ( nWeight <= pPwRT->m_nVerdictWeight )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)权值不够!!!\n\t",nWeight, pPwRT->m_nVerdictWeight );
#endif

				/* 权值不够则跳过本条规则 */

				return emFalse;
			}
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t",	nWeight, pPwRT->m_nVerdictWeight );
#endif
			/* 记录规则的权值等参数 */
			pPwRT->m_nVerdictWeight = nWeight;
			pPwRT->m_nVerdictFlag = 0;
			pPwRT->m_tVerdictRhythm = 0;
			pPwRT->m_tVerdictPoS = (emInt8)p[3];
			pPwRT->m_tVerdictPoS2 = (emInt8)p[4];

			/* 匹配成功,退出 */

			return emTrue;
		}

		/* 准备匹配下一条规则 */

	}

	/* 未能匹配上 */
	return emFalse;
}

/* 执行二元拼接 */
emStatic emInt32  emCall CombineTwoPW(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT,		/* 运行时数据 */
	emUInt8		j				/* 起始位置 */
)
{
	emBool		bCbWord;
	emUInt8	iL, iR;
	emUInt8		nL, nR, nCombineSize;
	emInt8		rL, rR;
	emUInt8	t;

	LOG_StackAddr(__FUNCTION__);

	/* 提取是否拼接成词标志 */
	bCbWord = (CbWord == pPwRT->m_nVerdictFlag);

	/* 获取两个待拼接节点的音节索引 */
	iL = GetIndex(pPwRT, j);
	iR = GetIndex(pPwRT, j + 1);

	/* 第1个韵律词不是词头,或者已经是拼接韵律词,则不允许拼词 */
	if ( !IsWordHead(pRhythmRT, iL) ||
		 TestFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm) ||
		 TestFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm) )
	{
		bCbWord = emFalse;
	}

	/* 获取两个待拼接节点的长度 */
	nL = GetPWLen(pRhythmRT, iL);
	nR = GetPWLen(pRhythmRT, iR);

	/* 两个待拼接节点中至少有一个单字 */
	emAssert(1 == nL || 1 == nR);
	if ( nL > 1 && nR > 1 )
		return 0;

	/* 计算拼接长度 */
	nCombineSize = nL + nR;

	/* 拼接长度不超过5 */
	emAssert(nCombineSize <= 5);
	if ( nCombineSize > 5 )
		return 0;

	/* 把两单字拼接放在最前面优先判断，可以提高效率 */
	if ( 2 == nCombineSize )
	{
		/* 两单字拼接成2节奏韵律词 */
		SetRhythm(pRhythmRT, iL, Rhythm2);
	}
	else if ( GetRhythmLen(pPwRT->m_tVerdictRhythm) == nCombineSize )
	{
		/* 采用建议节奏 */
		SetRhythm(pRhythmRT, iL, pPwRT->m_tVerdictRhythm);
	}
	else
	{
		/* 获取两个待拼接节点的节奏 */
		rL = GetRhythm(pRhythmRT, iL);
		rR = GetRhythm(pRhythmRT, iR);

		/* 获取后接第1个节点的音节索引 */
		t = GetIndex(pPwRT, j + 2);

		/* 根据拼接长度设置节奏 */
		switch ( nCombineSize )
		{
		case 3:
			if ( 1 == nL )
			{
				/* 1+2 -> 3 */
				SetRhythm(pRhythmRT, iL, Rhythm3);
			}
			else
			{
				/* 2+1 -> 21 */
				SetRhythm(pRhythmRT, iL, Rhythm21);
			}
			break;

		case 4:
			/* 只能是31拼接 */
			emAssert(1 == nR);
			if ( nR > 1 )
				return 0;

			/* 轻声结尾? */
			if ( ChsTone_Light == GetTone(pRhythmRT, iR) )
			{
				if ( Rhythm21 == rL )
				{
					/* 21+1 -> 22: 推动 下+的 -> 推动 下的 */
					SetRhythm(pRhythmRT, iL, Rhythm22);
				}
				else
				{
					/* 12/3+1 -> 31: 小朋友`的 */
					SetRhythm(pRhythmRT, iL, Rhythm31);
				}
			}
			/* 倒数第2个轻声? */
			else if ( ChsTone_Light == GetTone(pRhythmRT, iR - 1) )
			{
				/* 我们的|心 */
				SetRhythm(pRhythmRT, iL, Rhythm31);
			}
			else
			{
				if ( Rhythm21 == rL )
				{
					/* 21+1 -> 22 */
					SetRhythm(pRhythmRT, iL, Rhythm22);
				}
				else if ( Rhythm12 == rL )
				{
					/* 12+1 -> 121 */
					SetRhythm(pRhythmRT, iL, Rhythm121);
				}
				else
				{
					/* 3+1 -> 31 */
					SetRhythm(pRhythmRT, iL, Rhythm31);
				}
			}

			break;

		case 5:
			/* 只能是41拼接 */
			emAssert(1 == nR);
			if ( nR > 1 )
				return 0;

			/*-----------------------------------------
			!!! 对于5音节以上，必须拆分为两个韵律词
			!!! 直接将结果存入原来的两个PW节点
			------------------------------------------*/
			/* 第1个韵律词为单字结尾(一般是轻声)情况 */
			if ( Rhythm31 == rL || Rhythm121 == rL )
			{
				/* 操碎了`心 的 */
				/* 小孩子`们 的 */

				/* 计算第2个韵律词的音节索引 */
				iR = iL + 3;

				/* 分成3,2两个韵律词 */
				SetRhythm(pRhythmRT, iL, Rhythm3);
				SetRhythm(pRhythmRT, iR, Rhythm2);
			}
			/* 第1个韵律词为22节奏 */
			else
			{
				/* 方针`政策|的 殖民`统治|下 */

				/* 计算第2个韵律词的音节索引 */
				iR = iL + 2;

				/* 分成2,21两个韵律词 */
				SetRhythm(pRhythmRT, iL, Rhythm2);
				SetRhythm(pRhythmRT, iR, Rhythm21);
			}

			/* 设置第2个韵律词的词性 */
			if ( !GetPoS(pRhythmRT, iR) )
			{
				SetPoS(pRhythmRT, iR, GetPoS(pRhythmRT, iL));
			}

			/* 修改第2个韵律词的索引 */
			SetIndex(pPwRT, j + 1, iR);

			/* 设置拼接韵律词标志 */
			SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
			SetFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm);

			/* 第1个韵律词不能再向后拼接了 */
			SetWeight(pPwRT, iL, 0);

			/* 后接单字时第2个韵律词还允许再向后拼接 */
			if ( t != SylSuffixNull &&
				 Rhythm1 == GetRhythm(pRhythmRT, t) )
			{
				/* 但拼接权值被污染，须重新计算 */
				SetWeight(pPwRT, iR, -1);
			}
			else
			{
				/* 否则第2个韵律词不能再向后拼接了 */
				SetWeight(pPwRT, iR, 0);
			}

			/* 不是句首? */
			if ( j )
			{
				/* 前1个韵律词不能再向后拼接了 */
				SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
			}

			/* 拼接成功,不减少韵律词总数 */
			return 0;

		default: /* 不可能到这儿 */
			emAssert(emFalse);

			/* 未拼接,不减少韵律词总数 */
			return 0;
		}
	}

	/* 获取后接第1个节点的音节索引 */
	t = GetIndex(pPwRT, j + 2);

	/* 后接单字时还允许再向后拼接 */
	if ( t != SylSuffixNull &&
		Rhythm1 == GetRhythm(pRhythmRT, t) )
	{
		/* 但拼接权值被污染，须重新计算 */
		SetWeight(pPwRT, iL, -1);
	}
	else
	{
		/* 否则不能再向后拼接了 */
		SetWeight(pPwRT, iL, 0);
	}

	/* 不是句首? */
	if ( j )
	{
		/* 获取前接第1个节点的音节索引 */
		t = GetIndex(pPwRT, j - 1);

		/* 前接第1个节点是单字时还允许再向后拼接 */
		if ( GetRhythm(pRhythmRT, t) == Rhythm1 )
		{
			/* 但拼接权值被污染，须重新计算 */
			SetWeight(pPwRT, t, -1);
		}
		else
		{
			/* 否则不能再向后拼接了 */
			SetWeight(pPwRT, t, 0);
		}
	}

	/*
	 *	删除下一个PW节点
	 */

	/* 计算新的节点总数 */
	t = GetIndexCount(pPwRT) - 1;

	/* 更新节点入口索引表 */
	emMemMove(GetIndexPtr(pPwRT, j + 1),
			  GetIndexPtr(pPwRT, j + 2),
			  sizeof(emUInt8) * (t - j + 2));

	/* 设置新的节点总数 */
	SetIndexCount(pPwRT, t);

	/* 拼接成词? */
	if ( bCbWord )
	{
		emUInt8 cL, cR;

		/* 修改词长 */
		SetWordLen(pRhythmRT, iL, nCombineSize);
		while ( -- nCombineSize )
			SetWordLen(pRhythmRT, iL + nCombineSize, - (emInt8)nCombineSize);

		/* 获取两个待拼接词的词频 */
		cL = GetFrCost(pRhythmRT, iL);
		cR = GetFrCost(pRhythmRT, iR);

		/* 重新计算词频 */
		if ( cL > cR )
			cL = (2 * (emUInt16)cL + cR) / 3;
		else
			cL = (cL + 3 * (emUInt16)cR) / 3;

		/* 更新拼接词词频 */
		SetFrCost(pRhythmRT, iL, cL);

		/* 更新拼接词性 */
		if ( pPwRT->m_tVerdictPoS <= 0 )
		{
			/* 使用第2个拼接词的词性 */
			if ( -1 == pPwRT->m_tVerdictPoS )
				SetPoS(pRhythmRT, iL, GetPoS(pRhythmRT, iR));
		}
		else
		{
			/* 使用规则设置的词性 */
			SetPoS(pRhythmRT, iL, (emUInt8)pPwRT->m_tVerdictPoS);
		}

		/* 删除第2个待拼接词的词性 */
		SetPoS(pRhythmRT, iR, 0);

		/* 更新拼接扩展词性 */
		if ( ChsPoS_v == GetPoS(pRhythmRT, iL) && ChsPoS_n == GetPoS(pRhythmRT, iR) )
			SetFrontFlag(pRhythmRT, iL, FrontFlag_VerbIntrans);
	}
	else
	{
		/* 拼接成韵律词 */
		SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
	}

	/* 拼接成功,减少1个韵律词 */
	return 1;
}

/* 执行三元拼接 */
emStatic emInt32  emCall CombineThreePW(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT,		/* 运行时数据 */
	emUInt8		j				/* 起始位置 */
)
{
	emBool		bCbWord;
	emUInt8	iL, iM, iR;
	emUInt8		nL, nM, nR, nCombineSize;
	emUInt8	t;

	LOG_StackAddr(__FUNCTION__);

	/* 提取是否拼接成词标志 */
	bCbWord = (CbWord == pPwRT->m_nVerdictFlag);

	/* 获取三个待拼接节点的音节索引 */
	iL = GetIndex(pPwRT, j);
	iM = GetIndex(pPwRT, j + 1);
	iR = GetIndex(pPwRT, j + 2);

	/* 第1个韵律词不是词头,或者已经是拼接韵律词,则不允许拼词 */
	if ( !IsWordHead(pRhythmRT, iL) ||
		 TestFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm) ||
		 TestFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm) )
	{
		bCbWord = emFalse;
	}

	/* 获取三个待拼接节点的长度 */
	nL = GetPWLen(pRhythmRT, iL);
	nM = GetPWLen(pRhythmRT, iM);
	nR = GetPWLen(pRhythmRT, iR);

	/* 计算拼接长度 */
	nCombineSize = nL + nM + nR;

	/* 拼接长度为6? */
	if ( 6 == nCombineSize )
	{
		/* 必定是411拼接 */
		emAssert(4 == nL);
		if ( nL != 4 )
		{
			/* 未拼接,不减少韵律词总数 */
			return 0;
		}

		/* 计算第2个韵律词的音节索引 */
		iR = iL + 2;

		/* 分成2,31两个韵律词 */
		SetRhythm(pRhythmRT, iL, Rhythm2);
		SetRhythm(pRhythmRT, iR, Rhythm31);

		/* 修改第2个韵律词的索引 */
		SetIndex(pPwRT, j + 1, iR);

		/* 设置拼接韵律词标志 */
		SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
		SetFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm);

		/* 两个韵律词均不能再参与拼接了 */
		SetWeight(pPwRT, iL, 0);
		SetWeight(pPwRT, iR, 0);

		/* 不是句首? */
		if ( j )
		{
			/* 前1个韵律词也不能再向后拼接了 */
			SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
		}

		/*
		 *	删除下一个PW节点
		 */

		/* 计算新的节点总数 */
		t = GetIndexCount(pPwRT) - 1;

		/* 更新节点入口索引表 */
		emMemMove(GetIndexPtr(pPwRT, j + 2),
			GetIndexPtr(pPwRT, j + 3),
			sizeof(emUInt8) * (t - j + 1));

		/* 设置新的节点总数 */
		SetIndexCount(pPwRT, t);

		/* 拼接成功,减少1个韵律词 */
		return 1;
	}

	/* 除411拼接外,拼接长度不超过4 */
	emAssert(nCombineSize <= 4);
	if ( nCombineSize > 4 )
	{
		/* 未拼接,不减少韵律词总数 */
		return 0;
	}

	/* 设置节奏 */
	if ( GetRhythmLen(pPwRT->m_tVerdictRhythm) == nCombineSize )
	{
		/* 采用建议节奏 */
		SetRhythm(pRhythmRT, iL, pPwRT->m_tVerdictRhythm);
	}
	/* 三单字拼接? */
	else if ( 3 == nCombineSize )
	{
		/* 后接轻声? */
		if ( ChsTone_Light == GetTone(pRhythmRT, iR) )
		{
			/* 1+1+1(轻声) -> 21 */
			SetRhythm(pRhythmRT, iL, Rhythm21);
		}
		else
		{
			/* 1+1+1(!轻声) -> 3 */
			SetRhythm(pRhythmRT, iL, Rhythm3);
		}
	}
	else
	{
		/* 第1个节点非单字? */
		if ( 2 == nL )
		{
			/* 第2个节点是轻声? */
			if ( ChsTone_Light == GetTone(pRhythmRT, iM) )
			{
				/* 2+1(轻声)+1 -> 31 */
				SetRhythm(pRhythmRT, iL, Rhythm31);
			}
			else
			{
				/* 2+1(!轻声)+1 -> 22 */
				SetRhythm(pRhythmRT, iL, Rhythm22);
			}
		}
		else
		{
			/* 第1个节点非单字? */
			if ( 2 == nM )
			{
				/* 1+2+1 -> 121 */
				SetRhythm(pRhythmRT, iL, Rhythm121);
			}
			else
			{
				/* 1+1+2 -> 22 */
				SetRhythm(pRhythmRT, iL, Rhythm22);
			}
		}
	}

	/* 获取后接第1个节点的音节索引 */
	t = GetIndex(pPwRT, j + 3);

	/* 后接单字时还允许再向后拼接 */
	if ( t != SylSuffixNull &&
		Rhythm1 == GetRhythm(pRhythmRT, t) )
	{
		/* 但拼接权值被污染，须重新计算 */
		SetWeight(pPwRT, iL, -1);
	}
	else
	{
		/* 否则不能再向后拼接了 */
		SetWeight(pPwRT, iL, 0);
	}

	/* 句首吗? */
	if ( j )
	{
		/* 本节奏已经>3,不能再向前拼接 */
		SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
	}

	/*
	 *	删除下两个PW节点
	 */

	/* 计算新的节点总数 */
	t = GetIndexCount(pPwRT) - 2;

	/* 更新节点入口索引表 */
	emMemMove(GetIndexPtr(pPwRT, j + 1),
		GetIndexPtr(pPwRT, j + 3),
		sizeof(emUInt8) * (t - j + 2));

	/* 设置新的节点总数 */
	SetIndexCount(pPwRT, t);

	/* 拼接成词? */
	if ( bCbWord )
	{
		/* 修改词长 */
		SetWordLen(pRhythmRT, iL, nCombineSize);
		while ( -- nCombineSize )
			SetWordLen(pRhythmRT, iL + nCombineSize, - (emInt8)nCombineSize);

		/* 重新计算词频 */
		SetFrCost(pRhythmRT, iL,
			(GetFrCost(pRhythmRT, iL) +
			GetFrCost(pRhythmRT, iM) +
			GetFrCost(pRhythmRT, iR)) / 3);

		/* 更新拼接词性 */
		switch ( pPwRT->m_tVerdictPoS )
		{
		case -1: /* 使用第2个拼接次的词性 */
			SetPoS(pRhythmRT, iL, GetPoS(pRhythmRT, iM));
		case 0: /* 使用第1个拼接次的词性 */
			break;

		case -2: /* 使用第3个拼接次的词性 */
			SetPoS(pRhythmRT, iL, GetPoS(pRhythmRT, iR));
			break;

		default: /* 使用规则设置的词性 */
			SetPoS(pRhythmRT, iL, (emUInt8)pPwRT->m_tVerdictPoS);
		}

		/* 删除第2,3两个待拼接词的词性 */
		SetPoS(pRhythmRT, iM, PoS_Null);
		SetPoS(pRhythmRT, iR, PoS_Null);
	}
	else
	{
		/* 拼接成韵律词 */
		SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
	}

	/* 拼接成功,减少2个韵律词 */
	return 2;
}

/* 执行四元拼接 */
emStatic emInt32  emCall CombineFourHz(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT,		/* 运行时数据 */
	emUInt8		j				/* 起始位置 */
)
{
	emUInt8 tIndex, t;

	LOG_StackAddr(__FUNCTION__);

	/* 获取待拼接节点的起始音节索引 */
	tIndex = GetIndex(pPwRT, j);

	/* 确认是4个连续的单字 */
	emAssert(GetIndex(pPwRT, j + 3) == tIndex + 3 &&
			 GetRhythm(pRhythmRT, tIndex + 3) == Rhythm1);
	if ( GetIndex(pPwRT, j + 3) != tIndex + 3 ||
		 GetRhythm(pRhythmRT, tIndex + 3) != Rhythm1 )
	{
		/* 未拼接,不减少韵律词总数 */
		return 0;
	}

	/* 设置节奏 */
	if ( GetRhythmLen(pPwRT->m_tVerdictRhythm) == 4 )
	{
		/* 采用建议节奏 */
		SetRhythm(pRhythmRT, tIndex, pPwRT->m_tVerdictRhythm);
	}
	else
	{
		/* 设置默认的节奏 */
		SetRhythm(pRhythmRT, tIndex, Rhythm22);
	}

	/* 获取后接第1个节点的音节索引 */
	t = GetIndex(pPwRT, j + 4);

	/* 后接单字时还允许再向后拼接 */
	if ( t != SylSuffixNull &&
		Rhythm1 == GetRhythm(pRhythmRT, t) )
	{
		/* 但拼接权值被污染，须重新计算 */
		SetWeight(pPwRT, tIndex, -1);
	}
	else
	{
		/* 否则不能再向后拼接了 */
		SetWeight(pPwRT, tIndex, 0);
	}

	/* 句首吗? */
	if ( j )
	{
		/* 本节奏已经>3,不能再向前拼接 */
		SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
	}

	/*
	 *	删除下三个PW节点
	 */

	/* 计算新的节点总数 */
	t = GetIndexCount(pPwRT) - 3;

	/* 更新节点入口索引表 */
	emMemMove(GetIndexPtr(pPwRT, j + 1),
		GetIndexPtr(pPwRT, j + 4),
		sizeof(emUInt8) * (t - j + 2));

	/* 设置新的节点总数 */
	SetIndexCount(pPwRT, t);

	/* 拼接成词? */
	if ( CbWord == pPwRT->m_nVerdictFlag )
	{
		/* 修改词长 */
		SetWordLen(pRhythmRT, tIndex, 4);
		SetWordLen(pRhythmRT, tIndex + 1, -1);
		SetWordLen(pRhythmRT, tIndex + 2, -2);
		SetWordLen(pRhythmRT, tIndex + 3, -3);

		/* 重新计算词频 */
		SetFrCost(pRhythmRT, tIndex,
			(GetFrCost(pRhythmRT, tIndex) +
			GetFrCost(pRhythmRT, tIndex + 1) +
			GetFrCost(pRhythmRT, tIndex + 2) +
			GetFrCost(pRhythmRT, tIndex + 3)) >> 2);

		/* 更新拼接词性 */
		if ( pPwRT->m_tVerdictPoS <= 0 )
		{
			/* 使用中心词的词性 */
			SetPoS(pRhythmRT, tIndex,
				GetPoS(pRhythmRT, tIndex - pPwRT->m_tVerdictPoS));
		}
		else
		{
			/* 使用规则设置的词性 */
			SetPoS(pRhythmRT, tIndex, (emUInt8)pPwRT->m_tVerdictPoS);
		}

		/* 删除第2,3,4三个待拼接词的词性 */
		SetPoS(pRhythmRT, tIndex + 1, PoS_Null);
		SetPoS(pRhythmRT, tIndex + 2, PoS_Null);
		SetPoS(pRhythmRT, tIndex + 3, PoS_Null);
	}
	else
	{
		/* 拼接成韵律词 */
		SetFrontFlag(pRhythmRT, tIndex, FrontFlag_CombinedRhythm);
	}

	/* 拼接成功,减少3个韵律词 */
	return 3;
}

/* 调整连续的单字节点 */
emStatic void emCall AdjustStub(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT,		/* 韵律实时对象 */
	PPwRT			pPwRT			/* 运行时数据 */
)
{
	/* "又才已还却就也曾将先" */
	emStatic emConst emCharA g_szSpecChar[] = {
		(emCharA)'\xD3', (emCharA)'\xD6', 0,
		(emCharA)'\xB2', (emCharA)'\xC5', 0,
		(emCharA)'\xD2', (emCharA)'\xD1', 0,
		(emCharA)'\xBB', (emCharA)'\xB9', 0,
		(emCharA)'\xC8', (emCharA)'\xB4', 0,
		(emCharA)'\xBE', (emCharA)'\xCD', 0,
		(emCharA)'\xD2', (emCharA)'\xB2', 0,
		(emCharA)'\xD4', (emCharA)'\xF8', 0,
		(emCharA)'\xBD', (emCharA)'\xAB', 0,
		(emCharA)'\xCF', (emCharA)'\xC8'
	};

	emUInt8 n, m, j;

	LOG_StackAddr(__FUNCTION__);

	/* 获取节点数 */
	n = GetIndexCount(pPwRT);

	/* 初始化拼接标志和词性 */
	pPwRT->m_nVerdictFlag = 0;
	pPwRT->m_tVerdictPoS = 0;

	/* 遍历每个节点 */
	for ( j = 0; j < n; ++ j )
	{
		emUInt8 i;

		/* 获取当前节点的索引 */
		i = GetIndex(pPwRT, j);

		/* 当前节点是单汉字吗? */
		if ( Rhythm1 == GetRhythm(pRhythmRT, i) )
		{
			emUInt8 tPoS;

			/* 获取当前节点的词性 */
			tPoS = GetPoS(pRhythmRT, i);

			/* 当前节点的词性可以作为词头 */
			if ( tPoS != ChsPoS_u && tPoS != ChsPoS_p )
			{
				/* 对当前节点开始的连续单汉字节点计数 */
				for ( m = 1; j + m < n; ++ m )
				{
					/* 获取当前节点的索引 */
					i = GetIndex(pPwRT, j + m);

					/* 当前节点不是单汉字则跳出计数 */
					if ( GetRhythm(pRhythmRT, i) != Rhythm1 )
						break;

					/* 当前节点不能用在非词头则跳出计数 */
					if ( ChsPoS_c == tPoS || ChsPoS_r == tPoS || ChsPoS_d == tPoS &&
							emStrStr((emPCUInt8)g_szSpecChar, emArrayCount(g_szSpecChar),
													GetText(pRhythmRT, i), 2) )
						break;
				}

				if ( m > 1 )
				{
					/* 先退后进!! */
					-- j;

					/* 设置拼接节奏2 */
					pPwRT->m_tVerdictRhythm = Rhythm2;

					/* 拼接连续的m个孤立字中的偶数部分 */
					while ( m > 3 || 2 == m )
					{
						CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, ++ j);
						m -= 2;
					}

					/* 拼接连续的m个孤立字中的奇数部分(如果存在) */
					if ( 3 == m )
					{
						pPwRT->m_tVerdictRhythm = Rhythm3;
						CombineThreePW(pRhythmRes, pRhythmRT, pPwRT, ++ j);
					}
				}
			}
		}
	}
}

emStatic void emCall FillBoundary(
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT		/* 韵律实时对象 */
)
{
	emUInt8 n, j;

	LOG_StackAddr(__FUNCTION__);

	/* 获取音节数 */
	n = GetSylCount(pRhythmRT);

	/* 遍历每个韵律词 */
	for ( j = 0; j < n; )
	{
		emUInt8 iNext, m;
		emInt8 tRhythm;

		/* 拆分4字词的韵律词 */
		switch ( GetRhythm(pRhythmRT, j) )
		{
		case Rhythm22:
			/* 设置下1个韵律词 */
			iNext = j + 2;
			SetRhythm(pRhythmRT, j, Rhythm2);
			SetRhythm(pRhythmRT, iNext, Rhythm2);
			SetPoS(pRhythmRT, iNext, GetPoS(pRhythmRT, j));
			break;

		case Rhythm13:
			/* 设置下1个韵律词 */
			iNext = j + 1;
			SetRhythm(pRhythmRT, j, Rhythm1);
			SetRhythm(pRhythmRT, iNext, Rhythm3);
			SetPoS(pRhythmRT, iNext, GetPoS(pRhythmRT, j));
			break;

		default:
			/* 获取下1个韵律词 */
			iNext = GetNextPW(pRhythmRT, j);
		}

		/* 设置当前韵律词的前后边界 */
		SetBoundaryBefore(pRhythmRT, j, BdPW);
		SetBoundaryBefore(pRhythmRT, iNext, BdPW);

		/* 获取当前韵律词的节奏掩码 */
		m = ((emUInt8)((GetRhythm(pRhythmRT, j))>>4));

		/* 遍历韵律词内的音节 */
		for ( tRhythm = 0, ++ j; j < iNext; ++ j )
		{
			/* 设置韵律词内边界(12,121,13的特殊处理对KT无效) */
			if ( 1 & m )
			{
				SetBoundaryBefore(pRhythmRT, j, BdPuny);

				if ( ChsTone_Light == GetTone(pRhythmRT, j) )
				{
					SetBoundaryBefore(pRhythmRT, j, BdNull);
				}
				else if ( 0 == tRhythm && (
					Rhythm12  == GetRhythm(pRhythmRT, j - 1) || 
					Rhythm121 == GetRhythm(pRhythmRT, j - 1) || 
					Rhythm13  == GetRhythm(pRhythmRT, j - 1)) )
				{
					/* !!!对12 121节奏的特殊处理 */
					SetBoundaryBefore(pRhythmRT, j, BdPW);
				}
				else
				{
					SetBoundaryBefore(pRhythmRT, j, BdPuny);
				}
			}
			else
			{
				SetBoundaryBefore(pRhythmRT, j, BdNull);
			}

			/* 设置韵律词内的节奏值 */
			SetRhythm(pRhythmRT, j, -- tRhythm);

			/* 滑动掩码 */
			m >>= 1;
		}
	}
}


/*
 *	接口函数
 */

/* 初始化 */
void emCall Front_Rhythm_PW_Init(  PRhythmRes pRhythmRes)
{

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pRhythmRes);


	if ( g_Res.offset_PWRule_New)
	{

		g_ResPWRule->m_iStart = g_Res.offset_PWRule_New;
		g_ResPWRule->m_iCurrent = g_Res.offset_PWRule_New ;
		g_ResPWRule->m_pResPack->m_pResParam = g_hTTS->fResFrontMain;
		g_ResPWRule->m_pResPack->m_nSize = 0;

		
		emRes_Get16(g_ResPWRule);

		/* 读取二元规则入口和条数 */
		pRhythmRes->m_subFootAdjust.m_pRuleTwoEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleTwoCount = emRes_Get16(g_ResPWRule);

		/* 读取三元规则入口和条数 */
		pRhythmRes->m_subFootAdjust.m_pRuleThreeEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleThreeCount = emRes_Get16(g_ResPWRule);

		/* 读取四元规则入口和条数 */
		pRhythmRes->m_subFootAdjust.m_pRuleFourEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleFourCount = emRes_Get16(g_ResPWRule);

		/* 读取复合规则入口 */
		pRhythmRes->m_subFootAdjust.m_pRuleMoreEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleMoreCount = emRes_Get16(g_ResPWRule);

		pRhythmRes->m_subFootAdjust.m_pRes = g_ResPWRule;

	}
#if DEBUG_LOG_SWITCH_PW_COMBINE
	emLogCreate(&g_tLogClass, "韵律词拼接");
#endif
}


/******************************************************************************
* 函数名        : Front_Rhythm_PW
* 描述          : 韵律词调整
* 参数          : [in]     pRhythmRes   - 韵律模块资源对象
*               : [in/out] pRhythmRT - 韵律实时对象

******************************************************************************/
void emCall Front_Rhythm_PW( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT		/* 韵律实时对象 */
)
{
	PPwRT pPwRT;
	emUInt8 n, j, tIndex2, tIndex;
	emUInt8 bLightTail;
	emUInt8 nCombinePos = 0;
	emInt16 nWeight;
	emInt8 tRhythm;

	LOG_StackAddr(__FUNCTION__);

	/* 检查参数 */
	emAssert(pRhythmRes && pRhythmRT);

	/* 有音节才处理 */
	if ( GetSylCount(pRhythmRT) )
	{
		if ( pRhythmRes->m_subFootAdjust.m_pRes )
		{
			/* 获取音节数 */
			n = GetSylCount(pRhythmRT);

			/* 只有一个词则只填充 */
			if ( GetWordLen(pRhythmRT, 0) < GetSylCount(pRhythmRT) )
			{
				/* 分配运行时空间 */
#if DEBUG_LOG_SWITCH_HEAP
				pPwRT = (PPwRT)emHeap_AllocZero(sizeof(struct tagPwRT), "PW时库：《韵律模块  》");
#else
				pPwRT = (PPwRT)emHeap_AllocZero(sizeof(struct tagPwRT));
#endif

				emAssert(pPwRT);

				emLog1(&g_tLogClass, "\n【%s】\n", GetText(pRhythmRT, 0));



				/*	第1步：初始化拼接权值	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n第1步：初始化拼接权值\t");
#endif

				/* 遍历每个韵律词 */
				for ( j = tIndex = 0;
					  tIndex < n;
					  ++ j, tIndex = GetNextPW(pRhythmRT, tIndex) )
				{
					/* 添加该韵律词节点 */
					SetIndex(pPwRT, j, tIndex);

					/* 非词头韵律词? */
					if ( !IsWordHead(pRhythmRT, tIndex) )
					{
						/* 设置该韵律词为不可向后拼接 */
						SetWeight(pPwRT, tIndex, 0);
					}
					/* 韵律词小于词? */
					else if ( GetPWLen(pRhythmRT, tIndex) != GetWordLen(pRhythmRT, tIndex) )
					{
						emAssert(GetPWLen(pRhythmRT, tIndex) < GetWordLen(pRhythmRT, tIndex));

						/* 不是句首? */
						if ( j )
						{
							/* 设置前1韵律词为不可向后拼接 */
							SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
						}

						/* 设置该韵律词为不可向后拼接 */
						SetWeight(pPwRT, tIndex, 0);
					}
					/* 韵律词即是词? */
					else
					{
						/* 设置该韵律词为可以向后拼接 */
						SetWeight(pPwRT, tIndex, -1);
					}
				}

				/* 记录节点个数 */
				n = j;
				SetIndexCount(pPwRT, n);

				/* 设置首尾的虚拟节点 */
				SetIndex(pPwRT, -1, SylSuffixNull);
				SetIndex(pPwRT, n, SylSuffixNull);
				SetIndex(pPwRT, n + 1, SylSuffixNull);
				SetIndex(pPwRT, n + 2, SylSuffixNull);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\t权重：\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t边界：\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t节奏：\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n【");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "】");	
#endif

				/*	第2步：计算拼接权值	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\n第2步：计算拼接权值\n\t");
#endif

				/* 遍历每个节点(除去最后1个,因为将要计算的是向后拼接权值) */
				for ( j = 0; j + 1 < n; ++ j )
				{
					/* 获取当前节点的音节索引 */
					tIndex = GetIndex(pPwRT, j);

					/* 允许拼接吗? */
					if ( GetWeight(pPwRT, tIndex) < 0 )
					{
						/* 获取下1个节点的音节索引 */
						tIndex2 = GetIndex(pPwRT, j + 1);

						/* 后接轻声吗? */
						if ( 1 == GetWordLen(pRhythmRT, tIndex2) &&
								ChsTone_Light == GetTone(pRhythmRT, tIndex2) )
							bLightTail = 1;
						else
							bLightTail = 0;

						/* 计算二元拼接代价 */
						HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, j, bLightTail);
					}
				}

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\t权重：\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t边界：\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t节奏：\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n【");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "】\n");	
#endif


				/*	第3步：复合和四元	 */

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n第3步：复合和四元\t");
#endif

				/* 遍历每个节点(除去最后3个,因为将要计算的是四元拼接权值) */
				for ( j = 0; j + 3 < n; ++ j )
				{
					/*
					 *	从前后词中挑选较大的权值设置初始权值门限
					 */

					/* 初始化初始权值门限 */
					pPwRT->m_nVerdictWeight = -1;

					/* 不是句首? */
					if ( j )
					{
						/* 用前1节点的权值更新初始权值门限 */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, GetIndex(pPwRT, j - 1));
					}

					/* 获取当前第3个节点的音节索引 */
					tIndex = GetIndex(pPwRT, j + 3);

					/* 当前第3个节点的音节存在,并且权值较大? */
					if ( GetWeight(pPwRT, tIndex) > pPwRT->m_nVerdictWeight )
					{
						/* 更新初始权值门限 */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, tIndex);
					}

					/* 复合规则 */
					if ( HandleRuleMore( pRhythmRes, pRhythmRT, pPwRT, j) )
					{
						/* 执行复合拼接 */
						n -= CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, j);
						++ j;
						pPwRT->m_tVerdictPoS = pPwRT->m_tVerdictPoS2;
						n -= CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, j);
					}
					/* 四元规则 */
					else if ( HandleRuleFour( pRhythmRes, pRhythmRT, pPwRT, j) )
					{
						/* 执行四元拼接 */
						n -= CombineFourHz(pRhythmRes, pRhythmRT, pPwRT, j);
					}
				}

				/* 补充前面拼接后的重拼接权值 */
				for ( j = 0; j + 1 < n; ++ j )
					if ( GetWeight(pPwRT, GetIndex(pPwRT, j)) < 0 )
						HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, j, 2);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\t权重：\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t边界：\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t节奏：\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n【");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "】\n");	
#endif


				/*	第4步：三元	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n第4步：三元\t");
#endif

				/* 遍历每个节点(除去最后2个,因为将要计算的是三元拼接权值) */
				for ( j = 0; j + 2 < n; ++ j )
				{
					/*
					 *	从前后词中挑选较大的权值设置初始权值门限
					 */

					/* 初始化初始权值门限 */
					pPwRT->m_nVerdictWeight = -1;

					/* 不是句首? */
					if ( j )
					{
						/* 用前1节点的权值更新初始权值门限 */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, GetIndex(pPwRT, j - 1));
					}

					/* 获取当前第3个节点的音节索引 */
					tIndex = GetIndex(pPwRT, j + 2);

					/* 当前第3个节点的音节存在,并且权值较大? */
					if ( GetWeight(pPwRT, tIndex) > pPwRT->m_nVerdictWeight )
					{
						/* 更新初始权值门限 */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, tIndex);
					}

					/* 三元规则 */
					if ( HandleRuleThree( pRhythmRes, pRhythmRT, pPwRT, j) )
					{
						/* 执行三元拼接 */
						n -= CombineThreePW(pRhythmRes, pRhythmRT, pPwRT, j);
					}
				}

				/* 补充前面拼接后的重拼接权值 */
				for ( j = 0; j + 1 < n; ++ j )
					if ( GetWeight(pPwRT, GetIndex(pPwRT, j)) < 0 )
						HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, j, 2);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\t权重：\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t边界：\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t节奏：\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n【");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "】\n");	
#endif


				/*	第5步：二元	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n第5步：二元");
#endif

				/* 反复二元规则 */
				for ( tIndex2 = 0; ; ++ tIndex2 )
				{
					emInt16 nMaxWeight;

					/*
					 *	搜索最优拼接位置
					 */

					/* 初始化最大权值 */
					nMaxWeight = 0;

					/* 遍历每个节点(除去最后1个,因为将要计算的是向后拼接权值) */
					for ( j = 0; j + 1 < n; ++ j )
					{
						/* 获取当前节点的音节索引 */
						tIndex = GetIndex(pPwRT, j);

						/* 获取当前节点的向后拼接权值 */
						nWeight = GetWeight(pPwRT, tIndex);

						/* 权值较大? */
						if ( nWeight > 0 && nWeight >= nMaxWeight )
						{
							/* 更新最大权值 */
							nMaxWeight = nWeight;

							/* 更新最大权值位置 */
							nCombinePos = j;
						}
					}

					/* 没有找到可以拼接的位置则退出循环 */
					if ( nMaxWeight <= 0 )
						break;
#if DEBUG_LOG_SWITCH_PW_COMBINE
					emLog1(&g_tLogClass, "\n\t找到最优位置 %d - ", nCombinePos);
#endif

					/* 获取拼接位置的音节索引 */
					tIndex = GetIndex(pPwRT, nCombinePos);

					/* 加载拼接参数 */
					pPwRT->m_nVerdictFlag = pPwRT->m_pFlag[tIndex];
					pPwRT->m_tVerdictPoS = (emInt8)pPwRT->m_pPoS[tIndex];
					pPwRT->m_tVerdictRhythm = pPwRT->m_pRhythm[tIndex];

					/* 执行拼接 */
					if ( !CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, nCombinePos) )
					{
						/* 拼接没有减少韵律词数 */
						emAssert(nCombinePos + 1 < n);

						/* 指向下1个节点 */
						++ nCombinePos;

						/* 下1个节点不能向后拼接 */
						if ( GetWeight(pPwRT, GetIndex(pPwRT, nCombinePos)) < 0 )
							HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, nCombinePos, 2);
					}
					else
					{
						/* 拼接减少1个韵律词 */
						-- n;

						/* 获取拼接节点的节奏 */
						tRhythm = GetRhythm(pRhythmRT, tIndex);

						/* 再给三元规则一次机会 */
						if ( tIndex2 < 3 && 2 == GetRhythmLen(tRhythm) && nCombinePos + 1 < n )
						{
							/* 设置初始权值门限 */
							pPwRT->m_nVerdictWeight = -1;

							/* 不是句首? */
							if ( nCombinePos )
							{
								/* 用前1节点的权值更新初始权值门限 */
								pPwRT->m_nVerdictWeight = GetWeight(pPwRT,
											GetIndex(pPwRT, nCombinePos - 1));
							}

							/* 获取当前第3个节点的音节索引 */
							tIndex = GetIndex(pPwRT, nCombinePos + 2);

							/* 当前第3个节点的音节存在,并且权值较大? */
							if ( tIndex != SylSuffixNull )
							{
								/* 当前第3个节点权值较大? */
								if ( GetWeight(pPwRT, tIndex) > pPwRT->m_nVerdictWeight )
								{
									/* 更新初始权值门限 */
									pPwRT->m_nVerdictWeight = GetWeight(pPwRT, tIndex);
								}

								/* 三元规则 */
								if ( HandleRuleThree( pRhythmRes, pRhythmRT, pPwRT, nCombinePos) )
								{
									/* 执行三元拼接 */
									n -= CombineThreePW(pRhythmRes, pRhythmRT, pPwRT, nCombinePos);
								}
							}
						}

						/* 后面还有节点,并且需要更新拼接权值? */
						if ( nCombinePos + 1 < n &&
							 GetWeight(pPwRT, GetIndex(pPwRT, nCombinePos)) < 0 )
						{
							/* 更新拼接权值 */
							HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, nCombinePos, 2);
						}

						/* 前面还有节点? */
						if ( nCombinePos )
						{
							/* 指向前面的节点 */
							-- nCombinePos;

							/* 更新拼接权值 */
							if ( GetWeight(pPwRT, GetIndex(pPwRT, nCombinePos)) < 0 )
								HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, nCombinePos, 2);
						}
					}
#if DEBUG_LOG_SWITCH_PW_COMBINE
					emLogWeight(pPwRT);
					emLog0(&g_tLogClass, "\n【");	
					emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
					emLog0(&g_tLogClass, "】\n");	
#endif
				}

				/*	第6步：拼接剩下的单字	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\n第6步：拼接剩下的单字\n\t");
#endif

				AdjustStub(pRhythmRes, pRhythmRT, pPwRT);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\t权重：\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t边界：\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t节奏：\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n【");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "】\n");
#endif


				/* 释放运行时空间 */
#if DEBUG_LOG_SWITCH_HEAP
				emHeap_Free(pPwRT, sizeof(struct tagPwRT), "PW时库：《韵律模块  》");
#else
				emHeap_Free(pPwRT, sizeof(struct tagPwRT));
#endif



			}
		}

		/*	第7步：填充边界	 */

		/* 填充边界 */
		FillBoundary(pRhythmRes, pRhythmRT);

#if DEBUG_LOG_SWITCH_PW_COMBINE
		emLog0(&g_tLogClass, "\n第7步：填充边界：\n	【");	
		emLogTextAddBoundary(&g_tLogClass,pRhythmRT);
		emLog0(&g_tLogClass, "】\n");	
#endif

	}
}


