#ifndef	_EMTTS__FRONT_PROC_YINGWEN__H_
#define _EMTTS__FRONT_PROC_YINGWEN__H_

#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

emInt16 emCall ProcessYingWen( emPByte strDataBuf, emInt16 nCurIndexOfBuf);

emInt16 emCall PlayYingWenVoice( emPByte pCurStr, emInt16  nCurIndex , emInt16 nLen );



#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_PROC_YINGWEN__H_ */