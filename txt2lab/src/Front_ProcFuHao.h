#ifndef	_EMTTS__FRONT_PROC_FUHAO__H_
#define _EMTTS__FRONT_PROC_FUHAO__H_


#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


emInt16 emCall ProcessFuHao( emPByte strDataBuf, emInt16 nCurIndexOfBuf);

emInt16  emCall  CheckGBKFuhaoToHanziDic( emPByte pCurStr, emInt16  nCurIndex );

#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_PROC_FUHAO__H_ */





