#ifndef	_EMTTS__REAR_PRE__H_
#define _EMTTS__REAR_PRE__H_


#include "Rear_Common.h"

#ifdef __cplusplus
extern "C" {
#endif


emInt16	LocateSegment_Cn(emInt16 nStartLine, emInt16 nMaxLine);		// ���ķֶ�
//emInt16	LocateSegment_Eng(emInt16 nStartLine, emInt16 nMaxLine);	// Ӣ�ķֶ�
emInt16	GetSegmentLength(emInt16 nStartLineOfPau ,emInt16 nLineCountOfPau);				//��ȡÿ��С�ֶε���֡�������ֶκϳɣ�
emInt16	GetSegmentMsdLength( emInt16 PauLength);				//��ȡÿ��С�ֶε�����֡�������ֶκϳɣ� 


//emInt16		GetLabelFileToLabRam( );		// ��lab�������뵽�ڴ�

#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__REAR_PRE__H_ */