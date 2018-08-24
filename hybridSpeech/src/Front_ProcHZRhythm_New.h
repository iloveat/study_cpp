
#ifndef _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW__H_   
#define _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW__H_


#include "emCommon.h"
#include "Front_ProcHZRhythm_New_PW.h"
#include "Front_ProcHZRhythm_New_PPH.h"


#ifdef __cplusplus
extern "C" {
#endif


void emCall Rhythm_New();		//�����·���

void emCall Rhythm_ForcePPH_Basic();				//����ǿ��PPH

void Front_Rhythm_RT_To_Link( PRhythmRT  pRhythmRT);		//��������  ת  pRhythmRT
void Front_Rhythm_Link_To_RT( PRhythmRT pRhythmRT);			//pRhythmRT ת �������� 


#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW__H_ */