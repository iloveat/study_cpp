#ifndef	_EMTTS__EM_COMMON__H_
#define _EMTTS__EM_COMMON__H_


#include "emTTS_Common.h"
#include "emRes.h"


#ifdef __cplusplus
extern "C" {
#endif


#define PROMPT_DECODE_ADPCM				1
#define PROMPT_DECODE_G7231				2
#define PROMPT_DECODE_PCM				3

#define	VOICE_TYPE_SYNTH				1		//声音类型：合成音
#define	VOICE_TYPE_PROMPT				2		//声音类型：提示音



/* 对齐指针/大小 */
#define emGridPtr(p)	((emPointer)(((emAddress)(p)+(EM_PTR_GRID-1))/EM_PTR_GRID*EM_PTR_GRID))
#define emGridSize(n)	((emSize)(((emSize)(n)+(EM_PTR_GRID-1))/EM_PTR_GRID*EM_PTR_GRID))


void emCall	ToPinYin();

void emCall ClearTextItem();

void emCall GenPauseNoOutSil();	//制造句尾停顿效果		

emTTSErrID emCall PlayMuteDelay(emInt32 nMuteMs);

emInt16 emCall CheckIsTypeBaiFenHao( emPByte pCurStr, emInt16  nCurIndex );

void emCall IsValidMarkAndPlayPrev();

emInt16  emCall  PlayPromptPreRecordVoice(emInt8 nPromptType, emInt8 nMoreType, emInt16 nPromptNo, struct promptInfo emPtr nCurIndex,emInt32 nIndexTableOffset);


emTTSErrID  emCall CpyPartStr( emByte *strDest, emByte *strSource, emInt16 nStart,  emInt16 nLen ,  emInt16 nSizeOfDest);

emInt16 emCall  CheckDataType( emPByte pCurStr, emInt16  nCurIndex);

emBool emCall  IsShuangBiaoDian( emPByte pCurStr, emInt16  nCurIndex);


void ClearBuffer( emPByte pCurStr, emInt16 n );

emInt16 emCall  GetWord( emPByte pCurStr, emUInt32  nCurIndex);

void PutIntoWord( emPByte pCurStr, emInt16  nCurIndex,emInt16  nWord );

emInt16 emCall  GetStrSum( emPByte pCurStr, emInt16  nCurIndex, emInt16	nDataType );

emInt16 emCall SoundSZBuffer( emPByte pCurStr, emInt16 nCurIndexSZBuffer, emInt16 nPos );

emInt16  emCall  ChangeShuZiToHanZi( emInt16 nLuoMaShuZi );

emInt32 emCall  ChangeShuZiToInt( emPByte pCurStr, emInt16  nCurIndex );

emInt32 emCall  ChangeShuZiToLong( emPByte pCurStr, emInt16  nCurIndex );

emInt16 emCall  ReadDigitHaoMa( emPByte pCurStr, emInt16  nCurIndex );

emInt16 emCall  ReadDigitShuZhi( emPByte pCurStr, emInt16  nCurIndex );

emInt16 emCall  ReadDigitShuZhiAddDian( emPByte pCurStr, emInt16  nCurIndex );

emInt16 emCall  ReadDigitShuZhiOfFour( emPByte pCurStr, emInt16  nCurIndex, emInt16 nSumOfRead);

emInt16 emCall  GetHanZiSum( emPByte pCurStr, emInt16  nCurIndex);

emInt16 emCall  GetYingWenSum( emPByte pCurStr, emInt16  nCurIndex);

emInt16 emCall  GetFuHaoSum( emPByte pCurStr, emInt16  nCurIndex);

emInt16 emCall  GetShuZiSum( emPByte pCurStr, emInt16  nCurIndex);

emInt16  emCall  GetPinYinCode( emPByte  pCurStr,emInt16  nCurIndex,emBool   bHaveOnlyPinYin, emByte   *pnShengMuNo,emByte   *pnYunMuNo,	emByte   *pnShengDiaoNo );//新系统

emInt16  emCall  CheckDanWeiTable( emPByte pCurStr, emInt16  nCurIndex );

emInt16  emCall  CheckHanZiLiangCiTable( emPByte pCurStr, emInt16  nCurIndex );

emInt16  emCall  CheckHanZiLetDigitToPhoneTable( emPByte pCurStr, emInt16  nCurIndex ,emByte nTypeTable );


emInt16 emCall  WriteToHanZiLink(emPByte pCurStr, emInt16 nCurIndexSZBuffer,emInt16 nPos, emInt16 nPinYin);
void emCall JustHanZiLetDigitToPhone();



void emCall	ChangePcmVol(emInt8 nVoiceType, emPByte  pStart, emInt32 nLen, emInt32  nVol);

emUInt16 ErFenSearchOne_FromRAM( emPByte, emInt16, emPByte, emInt16, emUInt16, emByte);		    //add by songkai

emUInt16 ErFenSearchTwo_FromRAM( emPByte, emInt16, emPByte, emInt16, emUInt16, emByte);		    //add by songkai


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__EM_COMMON__H_ */