#include "emPCH.h"

#include "Front_ProcHZRhythm_New_PW.h"


emPRes	g_ResPWRule;
emPRes	g_ResPPHC45;
emPRes	g_ResPPHRatio;


/*
 *	���Ե�������ʱ����
 */
typedef struct tagPwRT CPwRT, emPtr PPwRT;

struct tagPwRT
{
	/* ��������(Ϊ����-1��һ����λ) */
	emUInt8	m_pIndex[CE_MAX_SYLLABLE + 4];
	emUInt8	m_nIndexCount;

	/* �ڵ����� */
	emInt16		m_pWeight[CE_MAX_SYLLABLE + 1];	/* ���ֵ�����ƴ��Ȩֵ */
	emUInt8		m_pFlag[CE_MAX_SYLLABLE + 1];
	emInt8		m_pRhythm[CE_MAX_SYLLABLE + 1];
	emUInt8		m_pPoS[CE_MAX_SYLLABLE + 1];	/* ���ֵ�����ƴ�Ӵ��� */

	/* ���������� */
	emInt16		m_nVerdictWeight;	/* ƴ��Ȩֵ */
	emUInt8		m_nVerdictFlag;		/* ��ʶ */
	emInt8		m_tVerdictRhythm;	/* ����Ľ��� */
	emInt8		m_tVerdictPoS;		/* ƴ�Ӵ��� */
	emInt8		m_tVerdictPoS2;		/* ƴ�Ӵ���2 */
};

#define SetIndex(pPwRT, j, tIndex)	((void)(((pPwRT)->m_pIndex[(j)+1])=(tIndex)))
#define GetIndex(pPwRT, j)			((pPwRT)->m_pIndex[(j)+1])
#define GetIndexPtr(pPwRT, j)			((pPwRT)->m_pIndex+(j)+1)

#define SetIndexCount(pPwRT, tCount)	((void)(((pPwRT)->m_nIndexCount)=(tCount)))
#define GetIndexCount(pPwRT)			((pPwRT)->m_nIndexCount)

#define SetWeight(pPwRT, tIndex, nWeight)	((void)(((pPwRT)->m_pWeight[tIndex])=(nWeight)))
#define GetWeight(pPwRT, tIndex)			((pPwRT)->m_pWeight[tIndex])


/*	���� */
#define F_A	0	/* �ڽڵ��ھ��е�λ�� h������ͷ; t������β */
#define F_b	1	/* �ֻ�ʱ��� */
#define F_h	2	/* ��ͷ */
#define F_t	3	/* ��β */
#define F_x	4	/* ���� */
#define F_S	5	/* ���Լ��ϼ������еĶ������ */
#define F_u	6	/* ��չ���� */
#define F_W	7	/* �ɴʺ��������������03��ʾ��0��ź��ֺ͵�3��ź��ֳɴ� ���������һ������ */
#define F_y	8	/* ��ǰ��Ͻ�ʵ����ڸ��� */
#define F_v	9	/* ��y���ƣ��������Ǵ��������Զ���������ʶ�� */

/* �ַ������ */
#define	O_In		0	/* { �������� */
#define	O_Include	1	/* } ���� */
#define	O_Well		4	/* # �ַ������� */

/* �ַ����ڵ���� */
#define	O_Node		8	/* $,@ �ַ�����(�ڵ�)(��)��ͬ */

/*	ƴ������  */
#define CbWord	0x01	/* ƴ�ӳɴ� */
#define CbPW	0x02	/* ����ƴ��PW,�ʽ������ */

/* ��Ԫ���� */
#define BothSingle	0x03


/***************	��־���   ��ʼ  ********************************************/

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
	sprintf(szPath, "log/��־_%s.log", szClass);
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
				emLog1(pLogClass, "%s", "��" );
			}
			if( nBoundary == BdBreath )
			{
				emLog1(pLogClass, "%s", "��" );
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
				emLog1(pLogClass, "%s", "��" );		
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
				emLog1(pLogClass, "%s", "��" );
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
				emLog1(pLogClass, "%s", "��" );		
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

/***************	��־���   ����  ********************************************/






/*	�ı�ƥ��  */

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
	HandleString_In,		/* { �������� */
	HandleString_Include,	/* { �������� */
	HandleString_Out,		/* ~ �������� */
	HandleString_Exclude,	/* _ ������ */
	HandleString_Well,		/* # �ַ������� */
	HandleString_Not,		/* ! �ַ��������� */
	emNull,
	HandleString_Power		/* ^ �ַ�������Ϊ�� */
};


/*	ֵƥ��  */

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

	HandleValue_Less,	/* < С�� */
	HandleValue_BitInc,	/* & ���ز��԰��� */
	HandleValue_More,	/* > ���� */
	HandleValue_BitDec,	/* - �����ų� */
	HandleValue_Equal	/* = ֵ���� */
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


/* ƥ������ */
emStatic emBool emCall HandlePWRule( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,			/* ����ʵʱ���� */
	PPwRT			pPwRT,				/* ����ʱ���� */
	emUInt8		j,						/* ��ʼλ�� */
	emPCUInt8		pRule,				/* ������ */
	emSize			nLen				/* ���������� */
)
{
	emPCUInt8 pRuleEnd;
	emBool isOperHighBitEquaOne;

	LOG_StackAddr(__FUNCTION__);

	emAssert(nLen >= 2);

	emLog_ClearRule();

	/* ������������λ�� */
	pRuleEnd = pRule + nLen;

	/* ����ÿ������ */
	do
	{
		emInt8 nPos;
		emUInt8 tIndex, t;

		emUInt8 nFunc, nOper;
		emUInt8 nVal, nVal0;
		emPCUInt8 pText;

		/* ��ȡ�������õĽڵ�Ͳ����� */
		nVal = *pRule++;
		nPos = (emInt8)(nVal & 0x07) - 1;
		tIndex = GetIndex(pPwRT, j + nPos);
		nVal >>= 3;

		/* ��ȡ�����ĺ����Ͳ����� */
		nOper = *pRule++;
		nFunc = nOper & 0x0F;
		nOper >>= 4;

		isOperHighBitEquaOne = FALSE;
		if( ( nOper & 0x08) == 0x08 )		//nOper�����λΪ1
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

		/* ����Խ�� */
		if ( SylSuffixNull == tIndex )
		{
			/* ����Щ������ֱ����Ϊ��ƥ�� */
			switch ( nOper & 0x07 )
			{
			case O_In:
			case O_Include:
			case O_Well:
				return emFalse;
			}

			/* ��ָ���ı������Ĳ����� */
			if ( nFunc > F_A && nFunc <= F_S && 0 == (nOper & O_Node) )
			{
				/* ��չһ���ֽڴ洢�ı�����? */
				if ( !nVal )
				{
					/* ��ȡ��չ���ı����� */
					nVal = *pRule++;
					emAssert(pRule <= pRuleEnd);
				}

				/* ��������ָ�����ı� */
				pRule += nVal;
				emAssert(pRule <= pRuleEnd);
			}
		}
		else if ( nFunc > F_A && nFunc <= F_S )
		{
			/* �Խڵ��ı��Ĳ��� */
			if ( nOper & O_Node )
			{
				/* �鲢������ */
				nOper &= 7;

				/* ��ȡ��ؽڵ���������� */
				t = GetIndex(pPwRT, j + nVal);

				/* ��ȡ��ؽڵ���ı� */
				pText = GetText(pRhythmRT, t);
				nLen = GetPWTextLen(pRhythmRT, t);
			}
			/* ��ָ���ı��Ĳ��� */
			else
			{
				/* �ı����ȴ洢�ڲ�������? */
				if ( nVal )
				{
					/* �ı����Ⱦ��ǲ����� */
					nLen = nVal;
				}
				/* ��չһ���ֽڴ洢�ı����� */
				else
				{
					/* ��ȡ��չ���ı����� */
					nLen = *pRule++;
					emAssert(pRule <= pRuleEnd);
				}

				/* ��ȡ����ָ�����ı� */
				pText = pRule;
				pRule += nLen;
				emAssert(pRule <= pRuleEnd);
			}

			/* ���ݺ���ƥ������ */
			switch ( nFunc & 0x07 )
			{
			case F_b: /* PW������ı� */
				if( isOperHighBitEquaOne == FALSE)			
				{
					TextLowByteAdd1((emPUInt8)pText,nLen);				//�ı����ֽ���1
				}
				if ( !g_pfnHandleString[nOper]((emPCUInt8)GetText(pRhythmRT, tIndex), GetPWTextLen(pRhythmRT, tIndex), pText, nLen) )
					return emFalse;
				break;

			case F_h: /* ��ͷ */
				if( isOperHighBitEquaOne == FALSE)			
				{
					TextLowByteAdd1((emPUInt8)pText,nLen);				//�ı����ֽ���1
				}
				if ( !g_pfnHandleString[nOper]((emPCUInt8)GetText(pRhythmRT, tIndex), 2, pText, nLen) )
					return emFalse;
				break;

			case F_t: /* ��β */
				if( isOperHighBitEquaOne == FALSE)			
				{
					TextLowByteAdd1((emPUInt8)pText,nLen);				//�ı����ֽ���1
				}
				if ( !g_pfnHandleString[nOper]((emPCUInt8)GetText(pRhythmRT, tIndex) + (GetPWTextLen(pRhythmRT, tIndex) - 2), 2, pText, nLen) )
					return emFalse;
				break;

			case F_x: /* ���� */
				/* ƴ�����ɴʵ�û�д��� */
				if ( TestFrontFlag(pRhythmRT, tIndex, FrontFlag_CombinedRhythm) )
					return emFalse;

				/* �Ǵ�ͷ��û�д��� */
				if ( !IsWordHead(pRhythmRT, tIndex) )
					return emFalse;

				if ( !g_pfnHandleString[nOper](&GetPoS(pRhythmRT, tIndex), 1, pText, nLen) )
					return emFalse;

				break;

			case F_S: /* ���Լ���(�������еĶ������) */
				{
					return emFalse;
				}
				break;

			default: /* �Ƿ� */
				emAssert(emFalse);
				return emFalse;
			}
		}
		else if ( F_W != nFunc )
		{
			switch ( nFunc )
			{
			case F_A: /* ����λ�� */
				if ( 0 == j + nPos )
					nVal0 = 0; /* ��ͷ */
				else if ( GetIndexCount(pPwRT) - 1 == j + nPos )
					nVal0 = 1; /* ��β */
				else
					nVal0 = 2; /* ���� */
				break;

			case F_u: /* ������չ */
				nVal0 = (pRhythmRT)->m_tFrontFlag[tIndex];
				break;

			case F_v: /* ���ڸ���(������β������) */
				nVal0 = GetPWLen(pRhythmRT, tIndex);

				/* ������β������ */
				while ( nVal0 && ChsTone_Light == GetTone(pRhythmRT, tIndex + nVal0 - 1) )
					-- nVal0;

				break;

			case F_y: /* ���ڸ��� */
				nVal0 = GetPWLen(pRhythmRT, tIndex);
				break;

			default: /* �Ƿ� */
				emAssert(emFalse);
				return emFalse;
			}

			/* ƥ��ֵ */
			if ( !g_pfnHandleValue[nOper](nVal0, nVal) )
				return emFalse;
		}
		/* �ɴ�?(�ʵ��) */
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

/* ƥ���Ԫ���� */
emStatic void emCall HandleRuleTwo( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,			/* ����ʵʱ���� */
	PPwRT			pPwRT,				/* ����ʱ���� */
	emUInt8			j,					/* ��ʼλ�� */
	emUInt8			bLightTail			/* �Ƿ�������(0-��, 1-��, 2-δ��) */
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

	/* ��ȡ��1���ڵ������ */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t )
		return;

	/* ���δ����־ */
	if ( -1 == GetWeight(pPwRT, t) )
		SetWeight(pPwRT, t, 0);

	/* ��ȡ��1���ڵ�ĳ��� */
	nLen0 = GetPWLen(pRhythmRT, t);

	/* ��ȡ��2���ڵ������ */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return;

	/* ����δ��������β���в����ж� */
	if ( 2 == bLightTail )
	{
		if ( 1 == GetPWLen(pRhythmRT, t) && ChsTone_Light == GetTone(pRhythmRT, t) )
			bLightTail = 1;
		else
			bLightTail = 0;
	}

	/* ��ȡ��2���ڵ�ĳ��� */
	nLen1 = GetPWLen(pRhythmRT, t);


	/* ���������ڵ�ĵ������� */
	nMask = 0;
	if ( nLen0 > 1 )	nMask |= 0x01;
	if ( nLen1 > 1 )	nMask |= 0x02;

	/* ��Ԫ���������һ������ */
	if ( 3 == nMask )
		return;

	/* û�г��ȳ���6�Ķ�Ԫ���� */
	emAssert(nLen0 + nLen1 <= 6);
	if ( nLen0 + nLen1 > 6 )
		return;

	/* ���Ǻ��������� */
	if ( !bLightTail )
	{
		/* û�г��ȳ���5�Ķ�Ԫ���򲻺������ */
		if ( nLen0 + nLen1 > 5 )
			return;

		/* ����5�Ķ�Ԫ�����2���ڵ�����ǵ��� */
		if ( nLen0 + nLen1 == 5 && nLen1 > 1 )
			return;

		/* û�е�2���ڵ㳤�ȳ���2�Ķ�Ԫ���� */
		if ( nLen1 > 2 )
			return;

		/* ��1���ڵ�����д��� */
		if ( !GetPoS(pRhythmRT, t) )
			return;
	}

	/* ��ȡ��ӵ�1���ڵ� */
	t = GetIndex(pPwRT, j + 2);

	/* ��ӵ�1���ڵ����? */
	if ( t != SylSuffixNull  )
	{
		/* �����ӵ�1���ڵ�ĵ������� */
		if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
			nMask |= 0x04;

		/* ��ȡ��ӵ�2���ڵ� */
		t = GetIndex(pPwRT, j + 3);

		/* ��ӵ�2���ڵ����? */
		if ( t != SylSuffixNull )
		{
			/* �����ӵ�2���ڵ�ĵ������� */
			if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
				nMask |= 0x08;
		}
	}

	/* ���ù������ */
	emRes_SetPos(pRhythmRes->m_subFootAdjust.m_pRes,
		pRhythmRes->m_subFootAdjust.m_pRuleTwoEntry);

	/* ��ȡ�������� */
	n = pRhythmRes->m_subFootAdjust.m_nRuleTwoCount;

	/* ѭ��ƥ��������� */
	for ( i = 0; i < n; ++ i )
	{
		/* ��ȡ��������ĳ��� */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);

		/* ӳ�䱾����������� */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* ƥ�����룬�����жϵ��ֲַ��Ƿ��������Ҫ�� */
		if ( nMask & p[2] )
		{
			/* �������������������� */

			continue;
		}

		/* ƥ����� */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 6, l - 6) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog3(&g_tLogClass, "ƥ���Ԫ���� %u, %lu: %s", j, i, g_szRule);
#endif

			/* ��ȡ�����Ȩֵ */
			nWeight = em16From8s(p);

			/* ��ȡ�Ƚ�Ȩֵ�����λ�� */
			nPos = p[3] >> 4;
			t = GetIndex(pPwRT, j + nPos);

			/* ����ȨֵҪ�����趨ֵ������ز�ƥ�� */
			if ( nWeight <= GetWeight(pPwRT, t) )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)Ȩֵ����!!!\n\t", nWeight, GetWeight(pPwRT, t) );
#endif

				/* Ȩֵ������������������ */

				return;
			}

			/* ����Ҫ��ƴ�ӳɴ�,��ƴ�����ɴʲ�������ƴ�ӳɴ�? */
			if ( (p[3] & 0x0F) == CbWord && TestFrontFlag(pRhythmRT, t, FrontFlag_CombinedRhythm) )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, " ������ƴ�ӳɴ�!!!\n\t");
#endif

				/* ������ƴ�ӳɴ��������������� */

				continue;
			}

#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t", nWeight, GetWeight(pPwRT, t) );
#endif

			/* ����2ƴ�� */
			if ( 0 == (BothSingle & (nMask >> nPos)) )
				nWeight += 5;

			/* ��ȡ�ڵ��λ�� */
			t = GetIndex(pPwRT, j);

			/* ���ö�Ԫƴ��Ȩֵ */
			SetWeight(pPwRT, t, nWeight);

			/* ��¼����Ĳ��� */
			pPwRT->m_pFlag[t] = p[3] & 0x0F;
			pPwRT->m_pRhythm[t] = p[4];
			pPwRT->m_pPoS[t] = p[5];

			/* ƥ��ɹ�,�˳� */

			return;
		}

		/* ׼��ƥ����һ������ */

	}

	/* ��������ı�����ǰƴ�� */
	if ( bLightTail )
	{
		/* ��ȡ��1���ڵ������ */
		t = GetIndex(pPwRT, j);

		/* ���ö�Ԫƴ��Ȩֵ */
		SetWeight(pPwRT, t, 90);

		/* ��¼����Ĳ��� */
		pPwRT->m_pFlag[t] = CbPW;
		pPwRT->m_pRhythm[t] = 0;
		pPwRT->m_pPoS[t] = 0;
	}
}

/*
 *	ƥ����Ԫ����
 */
emStatic emBool emCall HandleRuleThree( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT,		/* ����ʱ���� */
	emUInt8		j				/* ��ʼλ�� */
)
{
	emUInt8	t;
	emUInt8		n0, n1, n2;
	emUInt8		nMask;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ��2���ڵ������ */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��2���ڵ�ĳ��� */
	n1 = GetPWLen(pRhythmRT, t);

	/* ��ȡ��3���ڵ������ */
	t = GetIndex(pPwRT, j + 2);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��2���ڵ�ĳ��� */
	n2 = GetPWLen(pRhythmRT, t);

	/* ��ȡ��1���ڵ������(��1���ڵ������ں���,tҪ��������) */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��1���ڵ�ĳ��� */
	n0 = GetPWLen(pRhythmRT, t);


	/* ���㵥������(���������������������֣�Ҳ����˵���һ���ǵ����ֽڵ�) */
	if ( n0 > 1 )
	{
		if ( n1 > 1 || n2 > 1 )
			return emFalse;

		/* ��1���ڵ�ǵ��� */
		nMask = 0x01;
	}
	else if ( n1 > 1 )
	{
		if ( n2 > 1 )
			return emFalse;

		/* ��2���ڵ�ǵ��� */
		nMask = 0x02;
	}
	else if ( n2 > 1 )
	{
		/* ��3���ڵ�ǵ��� */
		nMask = 0x04;
	}
	else
	{
		/* �����ڵ��Ϊ���� */
		nMask = 0;
	}

	/* ��4����һ�ر�Ļ��� */
	if ( 4 == n0 )
	{
		/* ����ֻ���� 411 ƴ�� */
		if ( Rhythm22 != GetRhythm(pRhythmRT, t) )
			return emFalse;
	}
	else
	{
		/* �����κνڵ㲻�ó��������� */
		if ( n0 > 2 )	return emFalse;
		if ( n1 > 2 )	return emFalse;
		if ( n2 > 2 )	return emFalse;
	}

	/* ��ȡ��ӵ�1���ڵ� */
	t = GetIndex(pPwRT, j + 3);

	/* ��ӵ�1���ڵ����? */
	if ( t != SylSuffixNull  )
	{
		/* �����ӵ�1���ڵ�ĵ������� */
		if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
			nMask |= 0x08;
	}

	/* ���ù������ */
	emRes_SetPos(g_ResPWRule,
		pRhythmRes->m_subFootAdjust.m_pRuleThreeEntry);

	/* ��ȡ�������� */
	n = pRhythmRes->m_subFootAdjust.m_nRuleThreeCount;

	/* ѭ��ƥ��������� */
	for ( i = 0; i < n; ++ i )
	{
		/* ��ȡ��������ĳ��� */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);

		/* ӳ�䱾����������� */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* ƥ�����룬�����жϵ��ֲַ��Ƿ��������Ҫ�� */
		if ( nMask & p[2] )
		{
			/* �������������������� */

			continue;
		}

		/* ƥ����� */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 6, l - 6) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, "ƥ����Ԫ���� %lu: %s", i, g_szRule);
#endif

			/* ��ȡ�����Ȩֵ */
			nWeight = em16From8s(p);

			/* ����ȨֵҪ�����趨ֵ������ز�ƥ�� */
			if ( nWeight <= pPwRT->m_nVerdictWeight )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)Ȩֵ����!!!\n\t", nWeight, pPwRT->m_nVerdictWeight );
#endif

				/* Ȩֵ������������������ */

				return emFalse;
			}

			/* ����Ҫ��ƴ�ӳɴ�,��ƴ�����ɴʲ�������ƴ�ӳɴ�? */
			if ( (p[3] & 0x0F) == CbWord && TestFrontFlag(pRhythmRT, t, FrontFlag_CombinedRhythm) )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, " ������ƴ�ӳɴ�!!!\n\t");
#endif

				/* ������ƴ�ӳɴ��������������� */

				continue;
			}

#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t", nWeight, pPwRT->m_nVerdictWeight );
#endif

			/* ��¼�����Ȩֵ�Ȳ��� */
			pPwRT->m_nVerdictWeight = nWeight;
			pPwRT->m_nVerdictFlag = p[3] & 0x0F;
			pPwRT->m_tVerdictRhythm = p[4];
			pPwRT->m_tVerdictPoS = (emInt8)p[5];

			/* ƥ��ɹ�,�˳� */

			return emTrue;
		}

		/* ׼��ƥ����һ������ */

	}

	/* δ��ƥ���� */
	return emFalse;
}

/* ƥ����Ԫ���� */
emStatic emBool emCall HandleRuleFour( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT,		/* ����ʱ���� */
	emUInt8		j				/* ��ʼλ�� */
)
{
	/* ��Ԫ����Ҫ���ǵ��֣����Բ���nMask */
	emUInt8	t;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ��1���ڵ������ */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* �ڵ�ĳ��ȱ�����1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* ��ȡ��2���ڵ������ */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* �ڵ�ĳ��ȱ�����1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* ��ȡ��3���ڵ������ */
	t = GetIndex(pPwRT, j + 2);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* �ڵ�ĳ��ȱ�����1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* ��ȡ��4���ڵ������ */
	t = GetIndex(pPwRT, j + 3);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* �ڵ�ĳ��ȱ�����1 */
	if ( GetRhythm(pRhythmRT, t) != Rhythm1 )
		return emFalse;

	/* ���ù������ */
	emRes_SetPos(g_ResPWRule,
		pRhythmRes->m_subFootAdjust.m_pRuleFourEntry);

	/* ��ȡ�������� */
	n = pRhythmRes->m_subFootAdjust.m_nRuleFourCount;

	/* ѭ��ƥ��������� */
	for ( i = 0; i < n; ++ i )
	{
		/* ��ȡ��������ĳ��� */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);
		
		/* ӳ�䱾����������� */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* ƥ����� */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 6, l - 6) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, "ƥ����Ԫ���� %lu: %s", i, g_szRule);
#endif

			/* ��ȡ�����Ȩֵ */
			nWeight = em16From8s(p);

			/* ����ȨֵҪ�����趨ֵ������ز�ƥ�� */
			if ( nWeight <= pPwRT->m_nVerdictWeight )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)Ȩֵ����!!!\n\t",nWeight, pPwRT->m_nVerdictWeight );
#endif

				/* Ȩֵ������������������ */

				return emFalse;
			}
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t",	nWeight, pPwRT->m_nVerdictWeight );
#endif

			/* ��¼�����Ȩֵ�Ȳ��� */
			pPwRT->m_nVerdictWeight = nWeight;
			pPwRT->m_nVerdictFlag = p[3] & 0x0F;
			pPwRT->m_tVerdictRhythm = p[4];
			pPwRT->m_tVerdictPoS = (emInt8)p[5];

			/* ƥ��ɹ�,�˳� */

			return emTrue;
		}

		/* ׼��ƥ����һ������ */

	}

	/* δ��ƥ���� */
	return emFalse;
}

/* ƥ�临�Ϲ��� */
emStatic emBool emCall HandleRuleMore( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT,		/* ����ʱ���� */
	emUInt8		j				/* ��ʼλ�� */
)
{
	emUInt8		nMask;
	emUInt8	t;
	emUInt16	n, i;
	emUInt8		l, p[MAX_MAP_SIZE];
	emUInt16	nWeight;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ��1���ڵ������ */
	t = GetIndex(pPwRT, j);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��1���ڵ�ĳ��� */
	n = GetPWLen(pRhythmRT, t);

	nMask = 0;

	/* �ǵ��ֽڵ� */
	if ( n > 1 )
	{
		/* �ڵ㳤�Ȳ��ܳ���2 */
		if ( n > 2 )
			return emFalse;

		/* �޸ĵ������� */
		nMask |= 0x01;
	}

	/* ��ȡ��2���ڵ������ */
	t = GetIndex(pPwRT, j + 1);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��2���ڵ�ĳ��� */
	n = GetPWLen(pRhythmRT, t);

	/* �ǵ��ֽڵ� */
	if ( n > 1 )
	{
		/* �ڵ㳤�Ȳ��ܳ���2 */
		if ( n > 2 )
			return emFalse;

		/* �޸ĵ������� */
		nMask |= 0x02;
	}

	/* 1,2�����ڵ��б�����һ������ */
	if ( 0x03 == nMask )
		return emFalse;

	/* ��ȡ��3���ڵ������ */
	t = GetIndex(pPwRT, j + 2);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��3���ڵ�ĳ��� */
	n = GetPWLen(pRhythmRT, t);

	/* ��ʱ���ڴ洢3,4�����ڵ�ĵ������� */
	l = 0;

	/* �ǵ��ֽڵ� */
	if ( n > 1 )
	{
		/* �ڵ㳤�Ȳ��ܳ���2 */
		if ( n > 2 )
			return emFalse;

		/* �޸ĵ������� */
		l |= 0x04;
	}

	/* ��ȡ��4���ڵ������ */
	t = GetIndex(pPwRT, j + 3);
	emAssert(t != SylSuffixNull);
	if ( SylSuffixNull == t  )
		return emFalse;

	/* �ڵ��ϱ����д��� */
	if ( !GetPoS(pRhythmRT, t) )
		return emFalse;

	/* ��ȡ��4���ڵ�ĳ��� */
	n = GetPWLen(pRhythmRT, t);

	/* �ǵ��ֽڵ� */
	if ( n > 1 )
	{
		/* �ڵ㳤�Ȳ��ܳ���2 */
		if ( n > 2 )
			return emFalse;

		/* �޸ĵ������� */
		l |= 0x08;
	}

	/* 3,4�����ڵ��б�����һ������ */
	if ( 0x0C == l )
		return emFalse;

	/* �ϲ��������� */
	nMask |= l;

	/* ���ù������ */
	emRes_SetPos(g_ResPWRule,
		pRhythmRes->m_subFootAdjust.m_pRuleMoreEntry);

	/* ��ȡ�������� */
	n = pRhythmRes->m_subFootAdjust.m_nRuleMoreCount;

	/* ѭ��ƥ��������� */
	for ( i = 0; i < n; ++ i )
	{
		/* ��ȡ��������ĳ��� */
		l = emRes_Get8(pRhythmRes->m_subFootAdjust.m_pRes);

		/* ӳ�䱾����������� */
		emRes_Map8s(pRhythmRes->m_subFootAdjust.m_pRes, p, l);

		/* ƥ�����룬�����жϵ��ֲַ��Ƿ��������Ҫ�� */
		if ( nMask & p[2] )
		{
			/* �������������������� */

			continue;
		}

		/* ƥ����� */
		if ( HandlePWRule( pRhythmRes, pRhythmRT, pPwRT, j, p + 5, l - 5) )
		{
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, "ƥ�临�Ϲ��� %lu: %s", i, g_szRule);
#endif

			/* ��ȡ�����Ȩֵ */
			nWeight = em16From8s(p);

			/* ����ȨֵҪ�����趨ֵ������ز�ƥ�� */
			if ( nWeight <= pPwRT->m_nVerdictWeight )
			{
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog2(&g_tLogClass, " (%d<=%d)Ȩֵ����!!!\n\t",nWeight, pPwRT->m_nVerdictWeight );
#endif

				/* Ȩֵ������������������ */

				return emFalse;
			}
#if DEBUG_LOG_SWITCH_PW_COMBINE
			emLog2(&g_tLogClass, " (%d>%d)OK.\n\t",	nWeight, pPwRT->m_nVerdictWeight );
#endif
			/* ��¼�����Ȩֵ�Ȳ��� */
			pPwRT->m_nVerdictWeight = nWeight;
			pPwRT->m_nVerdictFlag = 0;
			pPwRT->m_tVerdictRhythm = 0;
			pPwRT->m_tVerdictPoS = (emInt8)p[3];
			pPwRT->m_tVerdictPoS2 = (emInt8)p[4];

			/* ƥ��ɹ�,�˳� */

			return emTrue;
		}

		/* ׼��ƥ����һ������ */

	}

	/* δ��ƥ���� */
	return emFalse;
}

/* ִ�ж�Ԫƴ�� */
emStatic emInt32  emCall CombineTwoPW(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT,		/* ����ʱ���� */
	emUInt8		j				/* ��ʼλ�� */
)
{
	emBool		bCbWord;
	emUInt8	iL, iR;
	emUInt8		nL, nR, nCombineSize;
	emInt8		rL, rR;
	emUInt8	t;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ�Ƿ�ƴ�ӳɴʱ�־ */
	bCbWord = (CbWord == pPwRT->m_nVerdictFlag);

	/* ��ȡ������ƴ�ӽڵ���������� */
	iL = GetIndex(pPwRT, j);
	iR = GetIndex(pPwRT, j + 1);

	/* ��1�����ɴʲ��Ǵ�ͷ,�����Ѿ���ƴ�����ɴ�,������ƴ�� */
	if ( !IsWordHead(pRhythmRT, iL) ||
		 TestFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm) ||
		 TestFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm) )
	{
		bCbWord = emFalse;
	}

	/* ��ȡ������ƴ�ӽڵ�ĳ��� */
	nL = GetPWLen(pRhythmRT, iL);
	nR = GetPWLen(pRhythmRT, iR);

	/* ������ƴ�ӽڵ���������һ������ */
	emAssert(1 == nL || 1 == nR);
	if ( nL > 1 && nR > 1 )
		return 0;

	/* ����ƴ�ӳ��� */
	nCombineSize = nL + nR;

	/* ƴ�ӳ��Ȳ�����5 */
	emAssert(nCombineSize <= 5);
	if ( nCombineSize > 5 )
		return 0;

	/* ��������ƴ�ӷ�����ǰ�������жϣ��������Ч�� */
	if ( 2 == nCombineSize )
	{
		/* ������ƴ�ӳ�2�������ɴ� */
		SetRhythm(pRhythmRT, iL, Rhythm2);
	}
	else if ( GetRhythmLen(pPwRT->m_tVerdictRhythm) == nCombineSize )
	{
		/* ���ý������ */
		SetRhythm(pRhythmRT, iL, pPwRT->m_tVerdictRhythm);
	}
	else
	{
		/* ��ȡ������ƴ�ӽڵ�Ľ��� */
		rL = GetRhythm(pRhythmRT, iL);
		rR = GetRhythm(pRhythmRT, iR);

		/* ��ȡ��ӵ�1���ڵ���������� */
		t = GetIndex(pPwRT, j + 2);

		/* ����ƴ�ӳ������ý��� */
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
			/* ֻ����31ƴ�� */
			emAssert(1 == nR);
			if ( nR > 1 )
				return 0;

			/* ������β? */
			if ( ChsTone_Light == GetTone(pRhythmRT, iR) )
			{
				if ( Rhythm21 == rL )
				{
					/* 21+1 -> 22: �ƶ� ��+�� -> �ƶ� �µ� */
					SetRhythm(pRhythmRT, iL, Rhythm22);
				}
				else
				{
					/* 12/3+1 -> 31: С����`�� */
					SetRhythm(pRhythmRT, iL, Rhythm31);
				}
			}
			/* ������2������? */
			else if ( ChsTone_Light == GetTone(pRhythmRT, iR - 1) )
			{
				/* ���ǵ�|�� */
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
			/* ֻ����41ƴ�� */
			emAssert(1 == nR);
			if ( nR > 1 )
				return 0;

			/*-----------------------------------------
			!!! ����5�������ϣ�������Ϊ�������ɴ�
			!!! ֱ�ӽ��������ԭ��������PW�ڵ�
			------------------------------------------*/
			/* ��1�����ɴ�Ϊ���ֽ�β(һ��������)��� */
			if ( Rhythm31 == rL || Rhythm121 == rL )
			{
				/* ������`�� �� */
				/* С����`�� �� */

				/* �����2�����ɴʵ��������� */
				iR = iL + 3;

				/* �ֳ�3,2�������ɴ� */
				SetRhythm(pRhythmRT, iL, Rhythm3);
				SetRhythm(pRhythmRT, iR, Rhythm2);
			}
			/* ��1�����ɴ�Ϊ22���� */
			else
			{
				/* ����`����|�� ֳ��`ͳ��|�� */

				/* �����2�����ɴʵ��������� */
				iR = iL + 2;

				/* �ֳ�2,21�������ɴ� */
				SetRhythm(pRhythmRT, iL, Rhythm2);
				SetRhythm(pRhythmRT, iR, Rhythm21);
			}

			/* ���õ�2�����ɴʵĴ��� */
			if ( !GetPoS(pRhythmRT, iR) )
			{
				SetPoS(pRhythmRT, iR, GetPoS(pRhythmRT, iL));
			}

			/* �޸ĵ�2�����ɴʵ����� */
			SetIndex(pPwRT, j + 1, iR);

			/* ����ƴ�����ɴʱ�־ */
			SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
			SetFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm);

			/* ��1�����ɴʲ��������ƴ���� */
			SetWeight(pPwRT, iL, 0);

			/* ��ӵ���ʱ��2�����ɴʻ����������ƴ�� */
			if ( t != SylSuffixNull &&
				 Rhythm1 == GetRhythm(pRhythmRT, t) )
			{
				/* ��ƴ��Ȩֵ����Ⱦ�������¼��� */
				SetWeight(pPwRT, iR, -1);
			}
			else
			{
				/* �����2�����ɴʲ��������ƴ���� */
				SetWeight(pPwRT, iR, 0);
			}

			/* ���Ǿ���? */
			if ( j )
			{
				/* ǰ1�����ɴʲ��������ƴ���� */
				SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
			}

			/* ƴ�ӳɹ�,���������ɴ����� */
			return 0;

		default: /* �����ܵ���� */
			emAssert(emFalse);

			/* δƴ��,���������ɴ����� */
			return 0;
		}
	}

	/* ��ȡ��ӵ�1���ڵ���������� */
	t = GetIndex(pPwRT, j + 2);

	/* ��ӵ���ʱ�����������ƴ�� */
	if ( t != SylSuffixNull &&
		Rhythm1 == GetRhythm(pRhythmRT, t) )
	{
		/* ��ƴ��Ȩֵ����Ⱦ�������¼��� */
		SetWeight(pPwRT, iL, -1);
	}
	else
	{
		/* �����������ƴ���� */
		SetWeight(pPwRT, iL, 0);
	}

	/* ���Ǿ���? */
	if ( j )
	{
		/* ��ȡǰ�ӵ�1���ڵ���������� */
		t = GetIndex(pPwRT, j - 1);

		/* ǰ�ӵ�1���ڵ��ǵ���ʱ�����������ƴ�� */
		if ( GetRhythm(pRhythmRT, t) == Rhythm1 )
		{
			/* ��ƴ��Ȩֵ����Ⱦ�������¼��� */
			SetWeight(pPwRT, t, -1);
		}
		else
		{
			/* �����������ƴ���� */
			SetWeight(pPwRT, t, 0);
		}
	}

	/*
	 *	ɾ����һ��PW�ڵ�
	 */

	/* �����µĽڵ����� */
	t = GetIndexCount(pPwRT) - 1;

	/* ���½ڵ���������� */
	emMemMove(GetIndexPtr(pPwRT, j + 1),
			  GetIndexPtr(pPwRT, j + 2),
			  sizeof(emUInt8) * (t - j + 2));

	/* �����µĽڵ����� */
	SetIndexCount(pPwRT, t);

	/* ƴ�ӳɴ�? */
	if ( bCbWord )
	{
		emUInt8 cL, cR;

		/* �޸Ĵʳ� */
		SetWordLen(pRhythmRT, iL, nCombineSize);
		while ( -- nCombineSize )
			SetWordLen(pRhythmRT, iL + nCombineSize, - (emInt8)nCombineSize);

		/* ��ȡ������ƴ�ӴʵĴ�Ƶ */
		cL = GetFrCost(pRhythmRT, iL);
		cR = GetFrCost(pRhythmRT, iR);

		/* ���¼����Ƶ */
		if ( cL > cR )
			cL = (2 * (emUInt16)cL + cR) / 3;
		else
			cL = (cL + 3 * (emUInt16)cR) / 3;

		/* ����ƴ�Ӵʴ�Ƶ */
		SetFrCost(pRhythmRT, iL, cL);

		/* ����ƴ�Ӵ��� */
		if ( pPwRT->m_tVerdictPoS <= 0 )
		{
			/* ʹ�õ�2��ƴ�ӴʵĴ��� */
			if ( -1 == pPwRT->m_tVerdictPoS )
				SetPoS(pRhythmRT, iL, GetPoS(pRhythmRT, iR));
		}
		else
		{
			/* ʹ�ù������õĴ��� */
			SetPoS(pRhythmRT, iL, (emUInt8)pPwRT->m_tVerdictPoS);
		}

		/* ɾ����2����ƴ�ӴʵĴ��� */
		SetPoS(pRhythmRT, iR, 0);

		/* ����ƴ����չ���� */
		if ( ChsPoS_v == GetPoS(pRhythmRT, iL) && ChsPoS_n == GetPoS(pRhythmRT, iR) )
			SetFrontFlag(pRhythmRT, iL, FrontFlag_VerbIntrans);
	}
	else
	{
		/* ƴ�ӳ����ɴ� */
		SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
	}

	/* ƴ�ӳɹ�,����1�����ɴ� */
	return 1;
}

/* ִ����Ԫƴ�� */
emStatic emInt32  emCall CombineThreePW(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT,		/* ����ʱ���� */
	emUInt8		j				/* ��ʼλ�� */
)
{
	emBool		bCbWord;
	emUInt8	iL, iM, iR;
	emUInt8		nL, nM, nR, nCombineSize;
	emUInt8	t;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ�Ƿ�ƴ�ӳɴʱ�־ */
	bCbWord = (CbWord == pPwRT->m_nVerdictFlag);

	/* ��ȡ������ƴ�ӽڵ���������� */
	iL = GetIndex(pPwRT, j);
	iM = GetIndex(pPwRT, j + 1);
	iR = GetIndex(pPwRT, j + 2);

	/* ��1�����ɴʲ��Ǵ�ͷ,�����Ѿ���ƴ�����ɴ�,������ƴ�� */
	if ( !IsWordHead(pRhythmRT, iL) ||
		 TestFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm) ||
		 TestFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm) )
	{
		bCbWord = emFalse;
	}

	/* ��ȡ������ƴ�ӽڵ�ĳ��� */
	nL = GetPWLen(pRhythmRT, iL);
	nM = GetPWLen(pRhythmRT, iM);
	nR = GetPWLen(pRhythmRT, iR);

	/* ����ƴ�ӳ��� */
	nCombineSize = nL + nM + nR;

	/* ƴ�ӳ���Ϊ6? */
	if ( 6 == nCombineSize )
	{
		/* �ض���411ƴ�� */
		emAssert(4 == nL);
		if ( nL != 4 )
		{
			/* δƴ��,���������ɴ����� */
			return 0;
		}

		/* �����2�����ɴʵ��������� */
		iR = iL + 2;

		/* �ֳ�2,31�������ɴ� */
		SetRhythm(pRhythmRT, iL, Rhythm2);
		SetRhythm(pRhythmRT, iR, Rhythm31);

		/* �޸ĵ�2�����ɴʵ����� */
		SetIndex(pPwRT, j + 1, iR);

		/* ����ƴ�����ɴʱ�־ */
		SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
		SetFrontFlag(pRhythmRT, iR, FrontFlag_CombinedRhythm);

		/* �������ɴʾ������ٲ���ƴ���� */
		SetWeight(pPwRT, iL, 0);
		SetWeight(pPwRT, iR, 0);

		/* ���Ǿ���? */
		if ( j )
		{
			/* ǰ1�����ɴ�Ҳ���������ƴ���� */
			SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
		}

		/*
		 *	ɾ����һ��PW�ڵ�
		 */

		/* �����µĽڵ����� */
		t = GetIndexCount(pPwRT) - 1;

		/* ���½ڵ���������� */
		emMemMove(GetIndexPtr(pPwRT, j + 2),
			GetIndexPtr(pPwRT, j + 3),
			sizeof(emUInt8) * (t - j + 1));

		/* �����µĽڵ����� */
		SetIndexCount(pPwRT, t);

		/* ƴ�ӳɹ�,����1�����ɴ� */
		return 1;
	}

	/* ��411ƴ����,ƴ�ӳ��Ȳ�����4 */
	emAssert(nCombineSize <= 4);
	if ( nCombineSize > 4 )
	{
		/* δƴ��,���������ɴ����� */
		return 0;
	}

	/* ���ý��� */
	if ( GetRhythmLen(pPwRT->m_tVerdictRhythm) == nCombineSize )
	{
		/* ���ý������ */
		SetRhythm(pRhythmRT, iL, pPwRT->m_tVerdictRhythm);
	}
	/* ������ƴ��? */
	else if ( 3 == nCombineSize )
	{
		/* �������? */
		if ( ChsTone_Light == GetTone(pRhythmRT, iR) )
		{
			/* 1+1+1(����) -> 21 */
			SetRhythm(pRhythmRT, iL, Rhythm21);
		}
		else
		{
			/* 1+1+1(!����) -> 3 */
			SetRhythm(pRhythmRT, iL, Rhythm3);
		}
	}
	else
	{
		/* ��1���ڵ�ǵ���? */
		if ( 2 == nL )
		{
			/* ��2���ڵ�������? */
			if ( ChsTone_Light == GetTone(pRhythmRT, iM) )
			{
				/* 2+1(����)+1 -> 31 */
				SetRhythm(pRhythmRT, iL, Rhythm31);
			}
			else
			{
				/* 2+1(!����)+1 -> 22 */
				SetRhythm(pRhythmRT, iL, Rhythm22);
			}
		}
		else
		{
			/* ��1���ڵ�ǵ���? */
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

	/* ��ȡ��ӵ�1���ڵ���������� */
	t = GetIndex(pPwRT, j + 3);

	/* ��ӵ���ʱ�����������ƴ�� */
	if ( t != SylSuffixNull &&
		Rhythm1 == GetRhythm(pRhythmRT, t) )
	{
		/* ��ƴ��Ȩֵ����Ⱦ�������¼��� */
		SetWeight(pPwRT, iL, -1);
	}
	else
	{
		/* �����������ƴ���� */
		SetWeight(pPwRT, iL, 0);
	}

	/* ������? */
	if ( j )
	{
		/* �������Ѿ�>3,��������ǰƴ�� */
		SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
	}

	/*
	 *	ɾ��������PW�ڵ�
	 */

	/* �����µĽڵ����� */
	t = GetIndexCount(pPwRT) - 2;

	/* ���½ڵ���������� */
	emMemMove(GetIndexPtr(pPwRT, j + 1),
		GetIndexPtr(pPwRT, j + 3),
		sizeof(emUInt8) * (t - j + 2));

	/* �����µĽڵ����� */
	SetIndexCount(pPwRT, t);

	/* ƴ�ӳɴ�? */
	if ( bCbWord )
	{
		/* �޸Ĵʳ� */
		SetWordLen(pRhythmRT, iL, nCombineSize);
		while ( -- nCombineSize )
			SetWordLen(pRhythmRT, iL + nCombineSize, - (emInt8)nCombineSize);

		/* ���¼����Ƶ */
		SetFrCost(pRhythmRT, iL,
			(GetFrCost(pRhythmRT, iL) +
			GetFrCost(pRhythmRT, iM) +
			GetFrCost(pRhythmRT, iR)) / 3);

		/* ����ƴ�Ӵ��� */
		switch ( pPwRT->m_tVerdictPoS )
		{
		case -1: /* ʹ�õ�2��ƴ�ӴεĴ��� */
			SetPoS(pRhythmRT, iL, GetPoS(pRhythmRT, iM));
		case 0: /* ʹ�õ�1��ƴ�ӴεĴ��� */
			break;

		case -2: /* ʹ�õ�3��ƴ�ӴεĴ��� */
			SetPoS(pRhythmRT, iL, GetPoS(pRhythmRT, iR));
			break;

		default: /* ʹ�ù������õĴ��� */
			SetPoS(pRhythmRT, iL, (emUInt8)pPwRT->m_tVerdictPoS);
		}

		/* ɾ����2,3������ƴ�ӴʵĴ��� */
		SetPoS(pRhythmRT, iM, PoS_Null);
		SetPoS(pRhythmRT, iR, PoS_Null);
	}
	else
	{
		/* ƴ�ӳ����ɴ� */
		SetFrontFlag(pRhythmRT, iL, FrontFlag_CombinedRhythm);
	}

	/* ƴ�ӳɹ�,����2�����ɴ� */
	return 2;
}

/* ִ����Ԫƴ�� */
emStatic emInt32  emCall CombineFourHz(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT,		/* ����ʱ���� */
	emUInt8		j				/* ��ʼλ�� */
)
{
	emUInt8 tIndex, t;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ��ƴ�ӽڵ����ʼ�������� */
	tIndex = GetIndex(pPwRT, j);

	/* ȷ����4�������ĵ��� */
	emAssert(GetIndex(pPwRT, j + 3) == tIndex + 3 &&
			 GetRhythm(pRhythmRT, tIndex + 3) == Rhythm1);
	if ( GetIndex(pPwRT, j + 3) != tIndex + 3 ||
		 GetRhythm(pRhythmRT, tIndex + 3) != Rhythm1 )
	{
		/* δƴ��,���������ɴ����� */
		return 0;
	}

	/* ���ý��� */
	if ( GetRhythmLen(pPwRT->m_tVerdictRhythm) == 4 )
	{
		/* ���ý������ */
		SetRhythm(pRhythmRT, tIndex, pPwRT->m_tVerdictRhythm);
	}
	else
	{
		/* ����Ĭ�ϵĽ��� */
		SetRhythm(pRhythmRT, tIndex, Rhythm22);
	}

	/* ��ȡ��ӵ�1���ڵ���������� */
	t = GetIndex(pPwRT, j + 4);

	/* ��ӵ���ʱ�����������ƴ�� */
	if ( t != SylSuffixNull &&
		Rhythm1 == GetRhythm(pRhythmRT, t) )
	{
		/* ��ƴ��Ȩֵ����Ⱦ�������¼��� */
		SetWeight(pPwRT, tIndex, -1);
	}
	else
	{
		/* �����������ƴ���� */
		SetWeight(pPwRT, tIndex, 0);
	}

	/* ������? */
	if ( j )
	{
		/* �������Ѿ�>3,��������ǰƴ�� */
		SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
	}

	/*
	 *	ɾ��������PW�ڵ�
	 */

	/* �����µĽڵ����� */
	t = GetIndexCount(pPwRT) - 3;

	/* ���½ڵ���������� */
	emMemMove(GetIndexPtr(pPwRT, j + 1),
		GetIndexPtr(pPwRT, j + 4),
		sizeof(emUInt8) * (t - j + 2));

	/* �����µĽڵ����� */
	SetIndexCount(pPwRT, t);

	/* ƴ�ӳɴ�? */
	if ( CbWord == pPwRT->m_nVerdictFlag )
	{
		/* �޸Ĵʳ� */
		SetWordLen(pRhythmRT, tIndex, 4);
		SetWordLen(pRhythmRT, tIndex + 1, -1);
		SetWordLen(pRhythmRT, tIndex + 2, -2);
		SetWordLen(pRhythmRT, tIndex + 3, -3);

		/* ���¼����Ƶ */
		SetFrCost(pRhythmRT, tIndex,
			(GetFrCost(pRhythmRT, tIndex) +
			GetFrCost(pRhythmRT, tIndex + 1) +
			GetFrCost(pRhythmRT, tIndex + 2) +
			GetFrCost(pRhythmRT, tIndex + 3)) >> 2);

		/* ����ƴ�Ӵ��� */
		if ( pPwRT->m_tVerdictPoS <= 0 )
		{
			/* ʹ�����ĴʵĴ��� */
			SetPoS(pRhythmRT, tIndex,
				GetPoS(pRhythmRT, tIndex - pPwRT->m_tVerdictPoS));
		}
		else
		{
			/* ʹ�ù������õĴ��� */
			SetPoS(pRhythmRT, tIndex, (emUInt8)pPwRT->m_tVerdictPoS);
		}

		/* ɾ����2,3,4������ƴ�ӴʵĴ��� */
		SetPoS(pRhythmRT, tIndex + 1, PoS_Null);
		SetPoS(pRhythmRT, tIndex + 2, PoS_Null);
		SetPoS(pRhythmRT, tIndex + 3, PoS_Null);
	}
	else
	{
		/* ƴ�ӳ����ɴ� */
		SetFrontFlag(pRhythmRT, tIndex, FrontFlag_CombinedRhythm);
	}

	/* ƴ�ӳɹ�,����3�����ɴ� */
	return 3;
}

/* ���������ĵ��ֽڵ� */
emStatic void emCall AdjustStub(
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT,		/* ����ʵʱ���� */
	PPwRT			pPwRT			/* ����ʱ���� */
)
{
	/* "�ֲ��ѻ�ȴ��Ҳ������" */
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

	/* ��ȡ�ڵ��� */
	n = GetIndexCount(pPwRT);

	/* ��ʼ��ƴ�ӱ�־�ʹ��� */
	pPwRT->m_nVerdictFlag = 0;
	pPwRT->m_tVerdictPoS = 0;

	/* ����ÿ���ڵ� */
	for ( j = 0; j < n; ++ j )
	{
		emUInt8 i;

		/* ��ȡ��ǰ�ڵ������ */
		i = GetIndex(pPwRT, j);

		/* ��ǰ�ڵ��ǵ�������? */
		if ( Rhythm1 == GetRhythm(pRhythmRT, i) )
		{
			emUInt8 tPoS;

			/* ��ȡ��ǰ�ڵ�Ĵ��� */
			tPoS = GetPoS(pRhythmRT, i);

			/* ��ǰ�ڵ�Ĵ��Կ�����Ϊ��ͷ */
			if ( tPoS != ChsPoS_u && tPoS != ChsPoS_p )
			{
				/* �Ե�ǰ�ڵ㿪ʼ�����������ֽڵ���� */
				for ( m = 1; j + m < n; ++ m )
				{
					/* ��ȡ��ǰ�ڵ������ */
					i = GetIndex(pPwRT, j + m);

					/* ��ǰ�ڵ㲻�ǵ��������������� */
					if ( GetRhythm(pRhythmRT, i) != Rhythm1 )
						break;

					/* ��ǰ�ڵ㲻�����ڷǴ�ͷ���������� */
					if ( ChsPoS_c == tPoS || ChsPoS_r == tPoS || ChsPoS_d == tPoS &&
							emStrStr((emPCUInt8)g_szSpecChar, emArrayCount(g_szSpecChar),
													GetText(pRhythmRT, i), 2) )
						break;
				}

				if ( m > 1 )
				{
					/* ���˺��!! */
					-- j;

					/* ����ƴ�ӽ���2 */
					pPwRT->m_tVerdictRhythm = Rhythm2;

					/* ƴ��������m���������е�ż������ */
					while ( m > 3 || 2 == m )
					{
						CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, ++ j);
						m -= 2;
					}

					/* ƴ��������m���������е���������(�������) */
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
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT		/* ����ʵʱ���� */
)
{
	emUInt8 n, j;

	LOG_StackAddr(__FUNCTION__);

	/* ��ȡ������ */
	n = GetSylCount(pRhythmRT);

	/* ����ÿ�����ɴ� */
	for ( j = 0; j < n; )
	{
		emUInt8 iNext, m;
		emInt8 tRhythm;

		/* ���4�ִʵ����ɴ� */
		switch ( GetRhythm(pRhythmRT, j) )
		{
		case Rhythm22:
			/* ������1�����ɴ� */
			iNext = j + 2;
			SetRhythm(pRhythmRT, j, Rhythm2);
			SetRhythm(pRhythmRT, iNext, Rhythm2);
			SetPoS(pRhythmRT, iNext, GetPoS(pRhythmRT, j));
			break;

		case Rhythm13:
			/* ������1�����ɴ� */
			iNext = j + 1;
			SetRhythm(pRhythmRT, j, Rhythm1);
			SetRhythm(pRhythmRT, iNext, Rhythm3);
			SetPoS(pRhythmRT, iNext, GetPoS(pRhythmRT, j));
			break;

		default:
			/* ��ȡ��1�����ɴ� */
			iNext = GetNextPW(pRhythmRT, j);
		}

		/* ���õ�ǰ���ɴʵ�ǰ��߽� */
		SetBoundaryBefore(pRhythmRT, j, BdPW);
		SetBoundaryBefore(pRhythmRT, iNext, BdPW);

		/* ��ȡ��ǰ���ɴʵĽ������� */
		m = ((emUInt8)((GetRhythm(pRhythmRT, j))>>4));

		/* �������ɴ��ڵ����� */
		for ( tRhythm = 0, ++ j; j < iNext; ++ j )
		{
			/* �������ɴ��ڱ߽�(12,121,13�����⴦���KT��Ч) */
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
					/* !!!��12 121��������⴦�� */
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

			/* �������ɴ��ڵĽ���ֵ */
			SetRhythm(pRhythmRT, j, -- tRhythm);

			/* �������� */
			m >>= 1;
		}
	}
}


/*
 *	�ӿں���
 */

/* ��ʼ�� */
void emCall Front_Rhythm_PW_Init(  PRhythmRes pRhythmRes)
{

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRes);


	if ( g_Res.offset_PWRule_New)
	{

		g_ResPWRule->m_iStart = g_Res.offset_PWRule_New;
		g_ResPWRule->m_iCurrent = g_Res.offset_PWRule_New ;
		g_ResPWRule->m_pResPack->m_pResParam = g_hTTS->fResFrontMain;
		g_ResPWRule->m_pResPack->m_nSize = 0;

		
		emRes_Get16(g_ResPWRule);

		/* ��ȡ��Ԫ������ں����� */
		pRhythmRes->m_subFootAdjust.m_pRuleTwoEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleTwoCount = emRes_Get16(g_ResPWRule);

		/* ��ȡ��Ԫ������ں����� */
		pRhythmRes->m_subFootAdjust.m_pRuleThreeEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleThreeCount = emRes_Get16(g_ResPWRule);

		/* ��ȡ��Ԫ������ں����� */
		pRhythmRes->m_subFootAdjust.m_pRuleFourEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleFourCount = emRes_Get16(g_ResPWRule);

		/* ��ȡ���Ϲ������ */
		pRhythmRes->m_subFootAdjust.m_pRuleMoreEntry = emRes_Get16(g_ResPWRule);
		pRhythmRes->m_subFootAdjust.m_nRuleMoreCount = emRes_Get16(g_ResPWRule);

		pRhythmRes->m_subFootAdjust.m_pRes = g_ResPWRule;

	}
#if DEBUG_LOG_SWITCH_PW_COMBINE
	emLogCreate(&g_tLogClass, "���ɴ�ƴ��");
#endif
}


/******************************************************************************
* ������        : Front_Rhythm_PW
* ����          : ���ɴʵ���
* ����          : [in]     pRhythmRes   - ����ģ����Դ����
*               : [in/out] pRhythmRT - ����ʵʱ����

******************************************************************************/
void emCall Front_Rhythm_PW( 
	PRhythmRes		pRhythmRes,			/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT		/* ����ʵʱ���� */
)
{
	PPwRT pPwRT;
	emUInt8 n, j, tIndex2, tIndex;
	emUInt8 bLightTail;
	emUInt8 nCombinePos = 0;
	emInt16 nWeight;
	emInt8 tRhythm;

	LOG_StackAddr(__FUNCTION__);

	/* ������ */
	emAssert(pRhythmRes && pRhythmRT);

	/* �����ڲŴ��� */
	if ( GetSylCount(pRhythmRT) )
	{
		if ( pRhythmRes->m_subFootAdjust.m_pRes )
		{
			/* ��ȡ������ */
			n = GetSylCount(pRhythmRT);

			/* ֻ��һ������ֻ��� */
			if ( GetWordLen(pRhythmRT, 0) < GetSylCount(pRhythmRT) )
			{
				/* ��������ʱ�ռ� */
#if DEBUG_LOG_SWITCH_HEAP
				pPwRT = (PPwRT)emHeap_AllocZero(sizeof(struct tagPwRT), "PWʱ�⣺������ģ��  ��");
#else
				pPwRT = (PPwRT)emHeap_AllocZero(sizeof(struct tagPwRT));
#endif

				emAssert(pPwRT);

				emLog1(&g_tLogClass, "\n��%s��\n", GetText(pRhythmRT, 0));



				/*	��1������ʼ��ƴ��Ȩֵ	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n��1������ʼ��ƴ��Ȩֵ\t");
#endif

				/* ����ÿ�����ɴ� */
				for ( j = tIndex = 0;
					  tIndex < n;
					  ++ j, tIndex = GetNextPW(pRhythmRT, tIndex) )
				{
					/* ��Ӹ����ɴʽڵ� */
					SetIndex(pPwRT, j, tIndex);

					/* �Ǵ�ͷ���ɴ�? */
					if ( !IsWordHead(pRhythmRT, tIndex) )
					{
						/* ���ø����ɴ�Ϊ�������ƴ�� */
						SetWeight(pPwRT, tIndex, 0);
					}
					/* ���ɴ�С�ڴ�? */
					else if ( GetPWLen(pRhythmRT, tIndex) != GetWordLen(pRhythmRT, tIndex) )
					{
						emAssert(GetPWLen(pRhythmRT, tIndex) < GetWordLen(pRhythmRT, tIndex));

						/* ���Ǿ���? */
						if ( j )
						{
							/* ����ǰ1���ɴ�Ϊ�������ƴ�� */
							SetWeight(pPwRT, GetIndex(pPwRT, j - 1), 0);
						}

						/* ���ø����ɴ�Ϊ�������ƴ�� */
						SetWeight(pPwRT, tIndex, 0);
					}
					/* ���ɴʼ��Ǵ�? */
					else
					{
						/* ���ø����ɴ�Ϊ�������ƴ�� */
						SetWeight(pPwRT, tIndex, -1);
					}
				}

				/* ��¼�ڵ���� */
				n = j;
				SetIndexCount(pPwRT, n);

				/* ������β������ڵ� */
				SetIndex(pPwRT, -1, SylSuffixNull);
				SetIndex(pPwRT, n, SylSuffixNull);
				SetIndex(pPwRT, n + 1, SylSuffixNull);
				SetIndex(pPwRT, n + 2, SylSuffixNull);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\tȨ�أ�\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t�߽磺\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t���ࣺ\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n��");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "��");	
#endif

				/*	��2��������ƴ��Ȩֵ	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\n��2��������ƴ��Ȩֵ\n\t");
#endif

				/* ����ÿ���ڵ�(��ȥ���1��,��Ϊ��Ҫ����������ƴ��Ȩֵ) */
				for ( j = 0; j + 1 < n; ++ j )
				{
					/* ��ȡ��ǰ�ڵ���������� */
					tIndex = GetIndex(pPwRT, j);

					/* ����ƴ����? */
					if ( GetWeight(pPwRT, tIndex) < 0 )
					{
						/* ��ȡ��1���ڵ���������� */
						tIndex2 = GetIndex(pPwRT, j + 1);

						/* ���������? */
						if ( 1 == GetWordLen(pRhythmRT, tIndex2) &&
								ChsTone_Light == GetTone(pRhythmRT, tIndex2) )
							bLightTail = 1;
						else
							bLightTail = 0;

						/* �����Ԫƴ�Ӵ��� */
						HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, j, bLightTail);
					}
				}

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\tȨ�أ�\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t�߽磺\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t���ࣺ\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n��");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "��\n");	
#endif


				/*	��3�������Ϻ���Ԫ	 */

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n��3�������Ϻ���Ԫ\t");
#endif

				/* ����ÿ���ڵ�(��ȥ���3��,��Ϊ��Ҫ���������Ԫƴ��Ȩֵ) */
				for ( j = 0; j + 3 < n; ++ j )
				{
					/*
					 *	��ǰ�������ѡ�ϴ��Ȩֵ���ó�ʼȨֵ����
					 */

					/* ��ʼ����ʼȨֵ���� */
					pPwRT->m_nVerdictWeight = -1;

					/* ���Ǿ���? */
					if ( j )
					{
						/* ��ǰ1�ڵ��Ȩֵ���³�ʼȨֵ���� */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, GetIndex(pPwRT, j - 1));
					}

					/* ��ȡ��ǰ��3���ڵ���������� */
					tIndex = GetIndex(pPwRT, j + 3);

					/* ��ǰ��3���ڵ�����ڴ���,����Ȩֵ�ϴ�? */
					if ( GetWeight(pPwRT, tIndex) > pPwRT->m_nVerdictWeight )
					{
						/* ���³�ʼȨֵ���� */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, tIndex);
					}

					/* ���Ϲ��� */
					if ( HandleRuleMore( pRhythmRes, pRhythmRT, pPwRT, j) )
					{
						/* ִ�и���ƴ�� */
						n -= CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, j);
						++ j;
						pPwRT->m_tVerdictPoS = pPwRT->m_tVerdictPoS2;
						n -= CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, j);
					}
					/* ��Ԫ���� */
					else if ( HandleRuleFour( pRhythmRes, pRhythmRT, pPwRT, j) )
					{
						/* ִ����Ԫƴ�� */
						n -= CombineFourHz(pRhythmRes, pRhythmRT, pPwRT, j);
					}
				}

				/* ����ǰ��ƴ�Ӻ����ƴ��Ȩֵ */
				for ( j = 0; j + 1 < n; ++ j )
					if ( GetWeight(pPwRT, GetIndex(pPwRT, j)) < 0 )
						HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, j, 2);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\tȨ�أ�\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t�߽磺\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t���ࣺ\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n��");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "��\n");	
#endif


				/*	��4������Ԫ	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n��4������Ԫ\t");
#endif

				/* ����ÿ���ڵ�(��ȥ���2��,��Ϊ��Ҫ���������Ԫƴ��Ȩֵ) */
				for ( j = 0; j + 2 < n; ++ j )
				{
					/*
					 *	��ǰ�������ѡ�ϴ��Ȩֵ���ó�ʼȨֵ����
					 */

					/* ��ʼ����ʼȨֵ���� */
					pPwRT->m_nVerdictWeight = -1;

					/* ���Ǿ���? */
					if ( j )
					{
						/* ��ǰ1�ڵ��Ȩֵ���³�ʼȨֵ���� */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, GetIndex(pPwRT, j - 1));
					}

					/* ��ȡ��ǰ��3���ڵ���������� */
					tIndex = GetIndex(pPwRT, j + 2);

					/* ��ǰ��3���ڵ�����ڴ���,����Ȩֵ�ϴ�? */
					if ( GetWeight(pPwRT, tIndex) > pPwRT->m_nVerdictWeight )
					{
						/* ���³�ʼȨֵ���� */
						pPwRT->m_nVerdictWeight = GetWeight(pPwRT, tIndex);
					}

					/* ��Ԫ���� */
					if ( HandleRuleThree( pRhythmRes, pRhythmRT, pPwRT, j) )
					{
						/* ִ����Ԫƴ�� */
						n -= CombineThreePW(pRhythmRes, pRhythmRT, pPwRT, j);
					}
				}

				/* ����ǰ��ƴ�Ӻ����ƴ��Ȩֵ */
				for ( j = 0; j + 1 < n; ++ j )
					if ( GetWeight(pPwRT, GetIndex(pPwRT, j)) < 0 )
						HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, j, 2);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\tȨ�أ�\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t�߽磺\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t���ࣺ\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n��");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "��\n");	
#endif


				/*	��5������Ԫ	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n��5������Ԫ");
#endif

				/* ������Ԫ���� */
				for ( tIndex2 = 0; ; ++ tIndex2 )
				{
					emInt16 nMaxWeight;

					/*
					 *	��������ƴ��λ��
					 */

					/* ��ʼ�����Ȩֵ */
					nMaxWeight = 0;

					/* ����ÿ���ڵ�(��ȥ���1��,��Ϊ��Ҫ����������ƴ��Ȩֵ) */
					for ( j = 0; j + 1 < n; ++ j )
					{
						/* ��ȡ��ǰ�ڵ���������� */
						tIndex = GetIndex(pPwRT, j);

						/* ��ȡ��ǰ�ڵ�����ƴ��Ȩֵ */
						nWeight = GetWeight(pPwRT, tIndex);

						/* Ȩֵ�ϴ�? */
						if ( nWeight > 0 && nWeight >= nMaxWeight )
						{
							/* �������Ȩֵ */
							nMaxWeight = nWeight;

							/* �������Ȩֵλ�� */
							nCombinePos = j;
						}
					}

					/* û���ҵ�����ƴ�ӵ�λ�����˳�ѭ�� */
					if ( nMaxWeight <= 0 )
						break;
#if DEBUG_LOG_SWITCH_PW_COMBINE
					emLog1(&g_tLogClass, "\n\t�ҵ�����λ�� %d - ", nCombinePos);
#endif

					/* ��ȡƴ��λ�õ��������� */
					tIndex = GetIndex(pPwRT, nCombinePos);

					/* ����ƴ�Ӳ��� */
					pPwRT->m_nVerdictFlag = pPwRT->m_pFlag[tIndex];
					pPwRT->m_tVerdictPoS = (emInt8)pPwRT->m_pPoS[tIndex];
					pPwRT->m_tVerdictRhythm = pPwRT->m_pRhythm[tIndex];

					/* ִ��ƴ�� */
					if ( !CombineTwoPW(pRhythmRes, pRhythmRT, pPwRT, nCombinePos) )
					{
						/* ƴ��û�м������ɴ��� */
						emAssert(nCombinePos + 1 < n);

						/* ָ����1���ڵ� */
						++ nCombinePos;

						/* ��1���ڵ㲻�����ƴ�� */
						if ( GetWeight(pPwRT, GetIndex(pPwRT, nCombinePos)) < 0 )
							HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, nCombinePos, 2);
					}
					else
					{
						/* ƴ�Ӽ���1�����ɴ� */
						-- n;

						/* ��ȡƴ�ӽڵ�Ľ��� */
						tRhythm = GetRhythm(pRhythmRT, tIndex);

						/* �ٸ���Ԫ����һ�λ��� */
						if ( tIndex2 < 3 && 2 == GetRhythmLen(tRhythm) && nCombinePos + 1 < n )
						{
							/* ���ó�ʼȨֵ���� */
							pPwRT->m_nVerdictWeight = -1;

							/* ���Ǿ���? */
							if ( nCombinePos )
							{
								/* ��ǰ1�ڵ��Ȩֵ���³�ʼȨֵ���� */
								pPwRT->m_nVerdictWeight = GetWeight(pPwRT,
											GetIndex(pPwRT, nCombinePos - 1));
							}

							/* ��ȡ��ǰ��3���ڵ���������� */
							tIndex = GetIndex(pPwRT, nCombinePos + 2);

							/* ��ǰ��3���ڵ�����ڴ���,����Ȩֵ�ϴ�? */
							if ( tIndex != SylSuffixNull )
							{
								/* ��ǰ��3���ڵ�Ȩֵ�ϴ�? */
								if ( GetWeight(pPwRT, tIndex) > pPwRT->m_nVerdictWeight )
								{
									/* ���³�ʼȨֵ���� */
									pPwRT->m_nVerdictWeight = GetWeight(pPwRT, tIndex);
								}

								/* ��Ԫ���� */
								if ( HandleRuleThree( pRhythmRes, pRhythmRT, pPwRT, nCombinePos) )
								{
									/* ִ����Ԫƴ�� */
									n -= CombineThreePW(pRhythmRes, pRhythmRT, pPwRT, nCombinePos);
								}
							}
						}

						/* ���滹�нڵ�,������Ҫ����ƴ��Ȩֵ? */
						if ( nCombinePos + 1 < n &&
							 GetWeight(pPwRT, GetIndex(pPwRT, nCombinePos)) < 0 )
						{
							/* ����ƴ��Ȩֵ */
							HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, nCombinePos, 2);
						}

						/* ǰ�滹�нڵ�? */
						if ( nCombinePos )
						{
							/* ָ��ǰ��Ľڵ� */
							-- nCombinePos;

							/* ����ƴ��Ȩֵ */
							if ( GetWeight(pPwRT, GetIndex(pPwRT, nCombinePos)) < 0 )
								HandleRuleTwo( pRhythmRes, pRhythmRT, pPwRT, nCombinePos, 2);
						}
					}
#if DEBUG_LOG_SWITCH_PW_COMBINE
					emLogWeight(pPwRT);
					emLog0(&g_tLogClass, "\n��");	
					emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
					emLog0(&g_tLogClass, "��\n");	
#endif
				}

				/*	��6����ƴ��ʣ�µĵ���	 */
#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\n��6����ƴ��ʣ�µĵ���\n\t");
#endif

				AdjustStub(pRhythmRes, pRhythmRT, pPwRT);

#if DEBUG_LOG_SWITCH_PW_COMBINE
				emLog0(&g_tLogClass, "\n\tȨ�أ�\t");	
				emLogWeight(pPwRT);
				emLog0(&g_tLogClass, "\n\t�߽磺\t");	
				emLogBoundary(pRhythmRT);
				emLog0(&g_tLogClass, "\n\t���ࣺ\t");	
				emLogRhythm(pRhythmRT);
				emLog0(&g_tLogClass, "\n��");	
				emLogTextAddRhythm(&g_tLogClass,pRhythmRT);
				emLog0(&g_tLogClass, "��\n");
#endif


				/* �ͷ�����ʱ�ռ� */
#if DEBUG_LOG_SWITCH_HEAP
				emHeap_Free(pPwRT, sizeof(struct tagPwRT), "PWʱ�⣺������ģ��  ��");
#else
				emHeap_Free(pPwRT, sizeof(struct tagPwRT));
#endif



			}
		}

		/*	��7�������߽�	 */

		/* ���߽� */
		FillBoundary(pRhythmRes, pRhythmRT);

#if DEBUG_LOG_SWITCH_PW_COMBINE
		emLog0(&g_tLogClass, "\n��7�������߽磺\n	��");	
		emLogTextAddBoundary(&g_tLogClass,pRhythmRT);
		emLog0(&g_tLogClass, "��\n");	
#endif

	}
}


