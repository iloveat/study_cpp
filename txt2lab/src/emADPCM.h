/* ADPCM 编解码算法头文件 */

#ifndef _EMTTS__EM_ADPCM__H_
#define _EMTTS__EM_ADPCM__H_


/*
 *	包含相关头文件
 */

#include "emCommon.h"


#ifdef __cplusplus
extern "C" {
#endif


/* Define ADPCM encode data structure and function */

typedef struct tagADPCMEncoder emTADPCMEncoder, emPtr emPADPCMEncoder;

struct tagADPCMEncoder
{
	emInt16		m_nPrevVal;		/* Previous output value */
	emInt8		m_nIndex;		/* Index into stepsize table */
};

#define emADPCM_InitCoder(pEncoder)	{(pEncoder)->m_nPrevVal=0;(pEncoder)->m_nIndex=0;}
emSize emCall emADPCM_Encode(emPADPCMEncoder pEncoder, emPInt16 pPcmIn, emSize nSamples, emPUInt8 pCodeOut);


/* Define ADPCM decode data structure and function */

typedef struct tagADPCMDecoder emTADPCMDecoder, emPtr emPADPCMDecoder;

struct tagADPCMDecoder
{
	emInt16		m_nPrevVal;		/* Previous output value */
	emUInt16	m_nIndex;		/* Index into stepsize table */
};

#define emADPCM_InitDecoder(pDecoder)	{(pDecoder)->m_nPrevVal=0;(pDecoder)->m_nIndex=0;}
emSize emCall emADPCM_Decode(emPADPCMDecoder pDecoder, emPUInt8 pCodeIn, emSize nCodeSize, emPInt16 pPcmOut);


#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__EM_ADPCM__H_*/
