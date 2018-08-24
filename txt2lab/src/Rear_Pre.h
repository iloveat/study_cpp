#ifndef	_EMTTS__REAR_PRE__H_
#define _EMTTS__REAR_PRE__H_


#include "Rear_Common.h"

#ifdef __cplusplus
extern "C" {
#endif


emInt16	LocateSegment_Cn(emInt16 nStartLine, emInt16 nMaxLine);		// 中文分段
//emInt16	LocateSegment_Eng(emInt16 nStartLine, emInt16 nMaxLine);	// 英文分段
emInt16	GetSegmentLength(emInt16 nStartLineOfPau ,emInt16 nLineCountOfPau);				//获取每个小分段的总帧数（按分段合成）
emInt16	GetSegmentMsdLength( emInt16 PauLength);				//获取每个小分段的浊音帧数（按分段合成） 


//emInt16		GetLabelFileToLabRam( );		// 将lab内容载入到内存

#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__REAR_PRE__H_ */