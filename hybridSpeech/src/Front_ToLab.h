#ifndef	_EMTTS__FRONT_TO_LAB__H_
#define _EMTTS__FRONT_TO_LAB__H_

#include "emCommon.h"
#include "List_Rear_Lab.hxx"

#ifdef __cplusplus
extern "C" {
#endif


#define  SilPau_PlaceCode				0xff		//句首sil和句尾sil的位置信息

void emCall	ToLab();
void emCall MergeWordsOfPW();
emUInt8  emCall	GetSenInfo(emUInt8 *pMaxOfHanZiInSEN, emUInt8 *pMaxOfPWInSEN, emUInt8 *pMaxOfPPHInSEN);

void emCall	GetPinYinInfoToLab();
void emCall GetPlaceAndCountInfoToLab();
void HandleLabQst();

emUInt8 emCall GetMaxHanZiInNextPW(emUInt8,emBool isSil);
emUInt8 emCall GetMaxHanZiInNextPPH(emUInt8,emBool isSil);
emUInt8 emCall GetMaxPWInNextPPH(emUInt8,emBool isSil);


void emCall ChangeTone();


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_TO_LAB__H_ */