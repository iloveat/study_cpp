
#ifndef _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW__H_   
#define _EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW__H_


#include "emCommon.h"
#include "Front_ProcHZRhythm_New_PW.h"
#include "Front_ProcHZRhythm_New_PPH.h"


#ifdef __cplusplus
extern "C" {
#endif


void emCall Rhythm_New();		//韵律新方案

void emCall Rhythm_ForcePPH_Basic();				//调整强制PPH

void Front_Rhythm_RT_To_Link( PRhythmRT  pRhythmRT);		//汉字链表  转  pRhythmRT
void Front_Rhythm_Link_To_RT( PRhythmRT pRhythmRT);			//pRhythmRT 转 汉字链表 


#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__FRONT_PROC_HANZI_RHYTHM_NEW__H_ */