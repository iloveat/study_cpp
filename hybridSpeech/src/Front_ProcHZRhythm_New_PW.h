/* 拼接韵律词 */

#ifndef _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW_PW__H_   
#define _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW_PW__H_

#include "emCommon.h"


#ifdef __cplusplus
extern "C" {
#endif




#define EM_CE_TEXT_SIZE 126
#define CE_MAX_SYLLABLE	(EM_CE_TEXT_SIZE * 2 + 4)

#define FrontFlag_VerbIntrans		0x01	
#define FrontFlag_CombinedRhythm	0x04	
#define FrontFlag_NotPPH			0x08	

#define ChsTone_Light	5

#define SylSuffixNull	((TSylSuffix)~0)
typedef	emUInt8 TSylSuffix, emPtr PSylSuffix;


#define BdNull		0		
#define BdPuny		1		
#define BdPW		2		
#define BdBreath	4		


#define	Rhythm1		0x01
#define	Rhythm2		0x02
#define	Rhythm3		0x03
#define	Rhythm11	0x12
#define	Rhythm12	0x13
#define	Rhythm21	0x23
#define	Rhythm13	0x14
#define	Rhythm22	0x24
#define	Rhythm31	0x44
#define	Rhythm121	0x54


enum enPoS
{
	PoS_Null = 0,
	ChsPoS_a = 1,
	ChsPoS_c = 2,
	ChsPoS_d = 3,
	ChsPoS_f = 4,
	ChsPoS_m = 5,
	ChsPoS_n = 6,
	ChsPoS_ng = 7,
	ChsPoS_p = 8,
	ChsPoS_q = 9,
	ChsPoS_s = 10,
	ChsPoS_t = 11,
	ChsPoS_u = 12,
	ChsPoS_v = 13,
	ChsPoS_h = 14,
	ChsPoS_k = 15,
	ChsPoS_o = 16,
	ChsPoS_r = 17,
	ChsPoS_R = 18,
	ChsPoS_y = 19,
	ChsPoS_ngp = 20
};


#define emArrayCount(ar)							(sizeof(ar)/sizeof(*(ar)))
#define GetText(pProsody,iSyl)						((emPCUInt8)((pProsody)->m_cText+(pProsody)->m_tSylInfo[iSyl].m_iSylText))
#define GetSylCount(pProsody)						((pProsody)->m_nSylCount)
#define GetTone(pProsody,iSyl)						((pProsody)->m_tSylInfo[iSyl].m_tTone)
#define GetPoS(pProsody,iSyl)						((pProsody)->m_tSylInfo[iSyl].m_tPoS)
#define SetPoS(pProsody,iSyl,tPoS)					((pProsody)->m_tSylInfo[iSyl].m_tPoS=(tPoS))
#define GetBoundaryBefore(pProsody,iSyl)			((pProsody)->m_tSylInfo[iSyl].m_tBoundary)
#define SetBoundaryBefore(pProsody,iSyl,tBoundary)	((pProsody)->m_tSylInfo[iSyl].m_tBoundary=(tBoundary))
#define GetRhythm(pRT,iSyl)			(pRT)->m_tRhythm[iSyl]
#define SetRhythm(pRT,iSyl,tRhythm)	((pRT)->m_tRhythm[iSyl]=(tRhythm))
#define GetRhythmLen(tRhythm)	((emUInt8)((tRhythm)&0x0F))



#define IsWordHead(pRT,iSyl)		((pRT)->m_tWordLen[iSyl]>0)
#define GetWordHead(pRT,iSyl)		((pRT)->m_tWordLen[iSyl]>0?iSyl:iSyl+(pRT)->m_tWordLen[iSyl])
#define GetWordLen(pRT,iSyl)		((pRT)->m_tWordLen[iSyl])
#define SetWordLen(pRT,iSyl,nLen)	((pRT)->m_tWordLen[iSyl]=(nLen))
#define GetNextWord(pRT,iSyl)		((iSyl)+GetWordLen(pRT,iSyl))


#define IsPWHead(pRT,iSyl)	((emBool)(((GetRhythm(pRT,iSyl))&0xF0)!=0xF0))
#define GetPWHead(pRT,iSyl)	(IsPWHead(pRT,iSyl)?(iSyl):(iSyl)+GetRhythm(pRT,iSyl))
#define GetPWLen(pRT,iSyl)		GetRhythmLen(GetRhythm(pRT,iSyl))
#define GetPWTextLen(pRT,iSyl)	((pRT)->m_tSylInfo[GetNextPW(pRT,iSyl)].m_iSylText-(pRT)->m_tSylInfo[iSyl].m_iSylText)
#define GetPrevPW(pRT,iSyl)	GetPWHead(pRT,(iSyl)-1)
#define GetNextPW(pRT,iSyl)	((iSyl)+GetPWLen(pRT,iSyl))


#define GetFrCost(pRT,iSyl)			(pRT)->m_nFrCost[iSyl]
#define SetFrCost(pRT,iSyl,nFrCost)	((pRT)->m_nFrCost[iSyl]=(nFrCost))


#define SetFrontFlag(pRT,iSyl,tFlag)	((pRT)->m_tFrontFlag[iSyl]|=(tFlag))
#define TestFrontFlag(pRT,iSyl,tFlag)	((pRT)->m_tFrontFlag[iSyl]&(tFlag))



//《结构体》相关********************************************************************************************** 




struct tagRhythmRes
{
	/* 韵律词调整相关变量 */
	struct {
		emPRes		m_pRes;				/* 韵律词调整资源对象 */
		emUInt16	m_pRuleTwoEntry;	/* 二元规则入口 */
		emUInt16	m_nRuleTwoCount;	/* 二元规则条数 */
		emUInt16	m_pRuleThreeEntry;	/* 三元规则入口 */
		emUInt16	m_nRuleThreeCount;	/* 三元规则条数 */
		emUInt16	m_pRuleFourEntry;	/* 四元规则入口 */
		emUInt16	m_nRuleFourCount;	/* 四元规则条数 */
		emUInt16	m_pRuleMoreEntry;	/* 复合规则入口 */
		emUInt16	m_nRuleMoreCount;	/* 复合规则条数 */
	} m_subFootAdjust;

	/* PPH相关变量 */
	struct {
		emPRes		m_pC45Res;				
		emPRes		m_pPPHRes;				
		emUInt32	m_dwSinglePPHCost;		
		emUInt32	m_dwTwoPPHCost;			
		emUInt32	m_dwThreePPHCost;		
	} m_subSyntax;
};

typedef struct tagRhythmRes CRhythmRes, emPtr PRhythmRes;

struct tagCESylInfo
{
	emUInt8		m_iSylText;							/* 文本位置 */
	emUInt8		m_tBoundary;						/* 前边界 */
	emUInt8			m_tTone;						/* 调型 */
	emUInt8			m_tPoS;							/* 词性 */

};

typedef struct tagCESylInfo TCESylInfo, emPtr PCESylInfo;

struct tagRhythmRT
{

	emUInt8			m_nTextLen;								
	emPByte			m_cText;								


	emUInt8			m_nSylCount;							
	emUInt8			m_tFrontFlag[CE_MAX_SYLLABLE];			
	PCESylInfo		m_tSylInfo;								//音节信息		

	emUInt8			m_tPoSBeforePW[CE_MAX_SYLLABLE];		//PW拼接前的原始词性

	emInt8			m_tRhythmBeforePW[CE_MAX_SYLLABLE];		//PW拼接前的原始Rhythm

	emInt8			m_tWordLen[CE_MAX_SYLLABLE];			
	emInt8			m_tRhythm[CE_MAX_SYLLABLE];			
	emUInt8			m_nFrCost[CE_MAX_SYLLABLE];			
};

typedef struct tagRhythmRT TRhythmRT, emPtr PRhythmRT;










void emCall Front_Rhythm_PW_Init(  PRhythmRes pRhythmRes);

void emCall Front_Rhythm_PW( 
	PRhythmRes		pRhythmRes,			/* 韵律模块资源对象 */
	PRhythmRT		pRhythmRT			/* 韵律实时对象 */
);




emExtern emPRes	g_ResPWRule;
emExtern emPRes	g_ResPPHC45;
emExtern emPRes	g_ResPPHRatio;



#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW_PW__H_ */
