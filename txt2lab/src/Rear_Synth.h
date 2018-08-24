#ifndef	_EMTTS__REAR_SYNTH__H_
#define _EMTTS__REAR_SYNTH__H_

#include "Rear_Common.h"
#include "Rear_Pre.h"



#if EM_SYS_SWITCH_FIX
	#include "Rear_model_fix.h"
#else
	#include "vocoder/world.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif



emTTSErrID emCall RearSynth(emInt8 nSynLangType);

void emCall LoadRearAllDecision();			//���غ�˵ľ�����
void emCall FreeRearAllDecision();			//�ͷź�˵ľ�����


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__REAR_SYNTH__H_ */