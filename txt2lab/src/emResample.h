//ʵ��Ƶ�ʷ�Χ����������ʵ��໥ת��
#ifndef	_EMTTS__EM_RESAMPLE__H_
#define _EMTTS__EM_RESAMPLE__H_

#include "emCommon.h"

#if defined(__cplusplus)
extern "C" {
#endif

emUInt32 GetMaxOutSamples(PReSampleData hReSample, emUInt32 nuSamples);		//��������Ĳ������� ��������������������� 

emExtern emUInt32 ReSample(PReSampleData hReSample,emPInt16 PcmIn,emPInt16 PcmOut,emUInt32 nuSamples	);		//���в���ת�� ���ز���ת����������� 

emExtern void ReSample_Init(PReSampleData hReSample,emUInt16 PcmInRate,emUInt16 PcmOutRate);	

#if defined(__cplusplus)
}
#endif

#endif	/* #define _EMTTS__EM_RESAMPLE__H_  */
