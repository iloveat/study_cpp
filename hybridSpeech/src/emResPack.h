
#ifndef	_EMTTS__EM_RES_PACK__H_
#define _EMTTS__EM_RES_PACK__H_


#include "emTTS_Common.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct tagResPack emTResPack, emPtr emPResPack;

struct tagResPack
{
	emPointer		m_pResParam;			//资源回调参数
	emSize			m_nSize;				//资源大小
	emCBReadRes		m_cbReadRes;			//读取资源回调

#if EM_RES_CACHE
	emPUInt8		m_pIndex;
	emPByte			m_pBuffer;
	emUInt8			m_nSizeShift, m_nCountShift;
	emResAddress	m_nSizeMask, m_nCountMask;
	emResSize		m_nSegSize;
	emSize			m_nTotalBlocks;
#endif
};


#if EM_RES_CACHE && DEBUG_LOG_SWITCH_CACHE

	void emCall emResCacheLog_Begin();
	void emCall emResCacheLog_End();

#else

	#define emResCacheLog_Begin()
	#define emResCacheLog_End()

#endif

void emCall emRes_Read(emPResPack pResPack, emPointer pBuffer, emResAddress iPos, emSize nSize);
void emCall emResPack_Initialize(emPResPack pResPack, emPResPackDesc pResPackDesc);
#define emResPack_Terminate		emAssert


#if EM_RES_CACHE
	emBool emCall emResPack_IsCacheValid(emPResPackDesc pResPackDesc);
	void emCall emResPack_Read(emPResPack pResPack, emPointer pBuffer, emResAddress iPos, emSize nSize);
#else

	#undef emResPack_Read
	#define emResPack_Read(pResPack,pBuffer,iPos,nSize)	(pResPack)->m_cbReadRes((pResPack)->m_pResParam,pBuffer,iPos,nSize)
#endif

emExtern  struct tagResPack g_ResPack;

#ifdef __cplusplus
}
#endif


#endif /* !_EMTTS__EM_RES_PACK__H_ */
