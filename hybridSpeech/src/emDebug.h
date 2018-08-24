#ifndef	_EMTTS__EM_DEBUG__H_
#define _EMTTS__EM_DEBUG__H_


#include "emTTS_Common.h"


#ifdef __cplusplus
extern "C" {
#endif


#if ( DEBUG_LOG_SWITCH_RHYTHM_MIDDLE || DEBUG_LOG_SWITCH_RHYTHM_RESULT )
	void emCall Print_To_Rhythm(emCharA*, emCharA*,emInt32 ,emCharA* );	//add by songkai
#endif

#if DEBUG_LOG_SWITCH_HANZI_LINK
	void emCall Print_To_Txt(emInt16 nPara, emByte *Text, emBool bIsOutputBorder);
#endif

#if DEBUG_LOG_POLY_TEST
	void Print_Poly_Test(emInt16 nPara, emByte *Text);
#endif

#if DEBUG_LOG_SWITCH_TIME
	void myTimeTrace_For_Debug(emInt16 nPara,emByte *Text, emInt16 nCount);
#endif


#if DEBUG_LOG_SWITCH_HANZI_LINK|DEBUG_LOG_POLY_TEST

	emUInt16 ErFenSearchPinPinLog( emInt32  nPinYin);
#endif


#ifdef __cplusplus
}
#endif

#endif	/* #define _EMTTS__EM_DEBUG__H_ */
