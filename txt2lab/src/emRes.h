#ifndef	_EMTTS__EM_RES__H_
#define _EMTTS__EM_RES__H_


#include "emTTS_Common.h"
#include "emResPack.h"


#ifdef __cplusplus
extern "C" {
#endif






#if ANKAI_TOTAL_SWITCH
	emInt32 GetTickCount();	//计时
#endif

#define emResSize_Int8		1
#define emResSize_Int16		2
#define emResSize_Int32		4

#define MAX_MAP_SIZE		1000

struct tagGUID
{
	emUInt32	Data1;
	emUInt16	Data2;
	emUInt16	Data3;
	emUInt8		Data4[8];
};



struct tagRes
{
	emInt32	cur_offset;

	emInt32	offset_CompanyInfo;
	emInt32	offset_InsertPlayInfo;

	emInt32	offset_CodeBig5ToGbk;
	emInt32	offset_CodeHanziUnicodeToGbk;
	emInt32	offset_CodeFuhaoUnicodeToGbk;

	emInt32	offset_HanZiLiangCi;
	emInt32	offset_EnglishLiangCiToHanZi;
	emInt32	offset_HanZiLetDigitToPhone ;
	emInt32	offset_GbkFuhaoToHanzi ;
	emInt32	offset_GBKDefaultPinyin ;
	emInt32	offset_PolyRuleIndex ;
	emInt32	offset_PolyRule ;
	emInt32	offset_PinYinInquireIndex ;
	emInt32	offset_PinYinInquire ;
	emInt32	offset_ConvertRatioOfGram ;
	emInt32	offset_SurnameRule;

	emInt32	offset_DictCnPtGB2312Index ;
	emInt32	offset_DictCnPt ;
	emInt32	offset_DictCnUser;

	emInt32	offset_RearVoiceLib_Cur ;
	emInt32	offset_RearVoiceLib01 ;		//中文音库：阿涛
	emInt32	offset_RearVoiceLib03 ;		//中文音库：小林 （默认音库）
	emInt32	offset_RearVoiceLib_Eng;	//英文音库：Koh

	emInt32	offset_PromptIndex;


#if EM_SYS_SWITCH_RHYTHM_NEW
	emInt32	offset_PWRule_New ;
	emInt32	offset_PPHC45_New;
	emInt32	offset_PPHRatio_New;
#else
	emInt32	offset_ConvertRatioOfPW;
	emInt32	offset_ConvertRatioOfPPH ;
#endif

	emInt32	offset_FanToJian;

	//英文前端
	emInt32	offset_EnF_GbksymbolToEn;

	//纯英文前端（不含数字等处理）
	emInt32	offset_EnPF_PosTernar;
	emInt32	offset_EnPF_DictNormal;
	emInt32	offset_EnPF_DictUser;
	emInt32	offset_EnPF_L3C45;
	emInt32	offset_EnPF_L3DistribProb;
	emInt32	offset_EnPF_LTSStress;
	emInt32	offset_EnPF_LTSaz;
	emInt32	offset_EnPF_ToBIAccentC45;
	emInt32	offset_EnPF_ToBIToneC45;
};

emInt32 GetOneResOffset(emPByte pResTablePara, emInt32 resTotalCountPara, struct tagGUID  sResID);
emBool	IsSameGUID(struct tagGUID  * pGuid1, struct tagGUID  * pGuid2);
void	GetTotalResOffset();


typedef struct tagResPack emTResPack, emPtr emPResPack2;

struct tagRuleRes
{
	emPResPack2		m_pResPack;		/* 资源对象 */
	emResAddress	m_iStart;		/* 资源起始位置 */
	emResAddress	m_iEnd;			/* 资源结束位置 */

	emResAddress	m_iCurrent;		/* 游标位置 */
};
typedef struct tagRuleRes emCRes, emPtr emPRes;


#define em16From8s(pBytes)			(((pBytes)[0])|((emUInt16)((pBytes)[1])<<8))
#define em32From8s(pBytes)			(((pBytes)[0])|((emUInt32)((pBytes)[1])<<8)|((emUInt32)((pBytes)[2])<<16)|((emUInt32)((pBytes)[3])<<24))


emPRes emCall emRes_Create( emResAddress iStart);
void   emCall emRes_Release( emPRes pThis);

emUInt8		emCall emRes_Get8(emPRes pThis);
emUInt16	emCall emRes_Get16(emPRes pThis);
emUInt32	emCall emRes_Get32(emPRes pThis);
void		emCall emRes_Read8s(emPRes pThis, emPUInt8 pBuffer, emResSize nCount);
void        emCall emRes_Read16s(emPRes pThis, emPUInt16 pBuffer, emResSize nCount);
void        emCall emRes_Read32s(emPRes pThis, emPUInt32 pBuffer, emResSize nCount);
void		emCall emRes_Map8s( emPRes pThis, emPUInt8	buffer,  emResSize nCount);
void        emCall emRes_Map16s( emPRes pThis, emPUInt16 buffer,emResSize nCount);


#define	emRes_SetPos(pThis,nPos)	((pThis)->m_iCurrent=(emResAddress)((pThis)->m_iStart+(nPos)))
#define	emRes_GetPos(pThis)			(emResAddress)((pThis)->m_iCurrent-(pThis)->m_iStart)


emExtern  struct tagRes g_Res;

#ifdef __cplusplus
}
#endif



#endif	/* #define _EMTTS__EM_RES__H_ */