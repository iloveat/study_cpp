/* G.723.1 编解码算法头文件 */

#ifndef _EMTTS__EM_G7231__H_ 
#define _EMTTS__EM_G7231__H_


/*
 *	包含相关头文件
 */

#include "emCommon.h"


#define EM_G7231_MAX_CODE_SIZE			(24)
#define EM_G7231_MAX_CODE_SIZE_RATE6	(24)
#define EM_G7231_MAX_CODE_SIZE_RATE5	(20)

#define EM_G7231_FRAME_SAMPLES	(240)
#define EM_G7231_DELAY_SAMPLES	(EM_G7231_FRAME_SAMPLES>>2)

/* G7231 编解码器实例的内存需求(须根据平台定制) */
#define EM_G7231_CODER_INSTANCE_SIZE	(1476)
#define EM_DECODER_G7231R_INSTANCE_SIZE	(420)


#ifdef __cplusplus
extern "C" {
#endif


void emCall emG7231_InitCoder( emPointer pInst, emBool bRate5 );
emUInt8 emCall emG7231_Encode( emPointer pInst, emPInt16 pPcmIn, emPUInt8 pCodeOut );

void emCall emG7231_InitDecoder( emPointer pInst, emBool bRate5 );
emUInt8 emCall emG7231_Decode( emPointer pInst, emPUInt8 pCodeIn, emPInt16 pPcmOut );


#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__EM_G7231__H_ */
