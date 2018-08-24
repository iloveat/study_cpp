//实现频率范围内任意采样率的相互转换
#ifndef	_EMTTS__EM_RESAMPLE__H_
#define _EMTTS__EM_RESAMPLE__H_

#include "emCommon.h"

#if defined(__cplusplus)
extern "C" {
#endif

emUInt32 GetMaxOutSamples(PReSampleData hReSample, emUInt32 nuSamples);		//给定输入的采样点数 返回最大可能输出采样点数 

emExtern emUInt32 ReSample(PReSampleData hReSample,emPInt16 PcmIn,emPInt16 PcmOut,emUInt32 nuSamples	);		//进行采样转换 返回采样转换后采样点数 

emExtern void ReSample_Init(PReSampleData hReSample,emUInt16 PcmInRate,emUInt16 PcmOutRate);	

#if defined(__cplusplus)
}
#endif

#endif	/* #define _EMTTS__EM_RESAMPLE__H_  */
