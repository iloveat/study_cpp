
#ifndef	_EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW_PPH__H_
#define _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW_PPH__H_


#include "emCommon.h"
#include "Front_ProcHZRhythm_New_PW.h"

#ifdef __cplusplus
extern "C" {
#endif


void  Front_Rhythm_PPH_Init(  PRhythmRes pRhythmRes );


void  Front_Rhythm_PPH( 
	PRhythmRes		pRhythmRes,		/* ����ģ����Դ���� */
	PRhythmRT		pRhythmRT		/* ����ʵʱ���� */
);




#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW_PPH__H_ */
