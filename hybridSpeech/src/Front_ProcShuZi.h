#ifndef	_EMTTS__FRONT_PROC_SHUZI__H_
#define _EMTTS__FRONT_PROC_SHUZI__H_

#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

emInt16 emCall ProcessShuZi( emPByte strDataBuf, emInt16 nCurIndexOfBuf);

emInt16  emCall  CheckDateFormat( emPByte pCurStr, emInt16  nCurIndex );

emInt16  emCall  CheckTimeFormat( emPByte pCurStr, emInt16  nCurIndex );

emInt16  emCall  CheckDanWeiTable( emPByte pCurStr, emInt16  nCurIndex );

#ifdef __cplusplus
}
#endif

#endif	/* #define _EMTTS__FRONT_PROC_SHUZI__H_ */





