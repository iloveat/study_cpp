#ifndef	_EMTTS__EM_DECODE__H_
#define _EMTTS__EM_DECODE__H_


#include "emCommon.h"
#include "emG7231.h"
#include "emADPCM.h"

#ifdef __cplusplus
extern "C" {
#endif


emTTSErrID emCall DecodePromptPreRecordPcmAndPlay(emInt16 nParaSample,  emInt32 nTableOffset);
emTTSErrID emCall DecodePromptPreRecordG7231AndPlay(emInt16 nParaSample,emInt32 nTableOffset);

#if EM_USER_DECODER_ADPCM
	emTTSErrID emCall DecodePromptPreRecordAdpcmAndPlay(emInt16 nParaSample,emInt32 nTableOffset);
#endif

#ifdef __cplusplus
}
#endif

#endif	/* #define _EMTTS__EM_DECODE__H_ */