/*
 *	TYPEDEF.C
 */

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"


#ifndef EM_MT


enum  Crate    WrkRate;


#if EM_USER_ENCODER_G7231


emBool  UseHp;
emBool  UseVx;


CODSTATDEF CodStat;
CODCNGDEF CodCng;
VADSTATDEF VadStat;


#endif /* EM_USER_ENCODER_G7231 */


#if EM_USER_DECODER_G7231


emBool  UsePf;


DECSTATDEF DecStat;
DECCNGDEF DecCng;


#endif /* EM_USER_DECODER_G7231 */


#endif /* EM_MT */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
