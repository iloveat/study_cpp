/*
   ITU-T G.723.1, G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
   All rights reserved.
*/

/*
   Types definitions
*/

#ifndef	_EMTTS__EM_G7231_TYPEDEF__H_
#define _EMTTS__EM_G7231_TYPEDEF__H_


#include "emCommon.h"
#include "emMath.h"


#define EM_MT

#define WMOPS	0	/* 去除时间统计  */


#include "emG7231_CST_LBC.h"


#ifdef EM_MT


typedef struct tagCodec CCodec, emPtr PCodec;

struct tagCodec
{
	enum Crate m_WrkRate;
};


#define WrkRate		(pThis->m_WrkRate)


typedef struct tagCoder CCoder, emPtr PCoder;

struct tagCoder
{
	CCodec m_Codec;
	emBool m_UseHp, m_UseVx;
	CODSTATDEF m_CodStat;
	CODCNGDEF m_CodCng;
	VADSTATDEF m_VadStat;
};


#define UseHp		(((PCoder)pThis)->m_UseHp)
#define	UseVx		(((PCoder)pThis)->m_UseVx)

#define CodStat		(((PCoder)pThis)->m_CodStat)
#define	CodCng		(((PCoder)pThis)->m_CodCng)
#define VadStat		(((PCoder)pThis)->m_VadStat)


typedef struct tagDecoder CDecoder, emPtr PDecoder;

struct tagDecoder
{
	CCodec m_Codec;
	emBool m_UsePf;
	DECSTATDEF m_DecStat;
	DECCNGDEF m_DecCng;
};


#define UsePf		(((PDecoder)pThis)->m_UsePf)

#define DecStat		(((PDecoder)pThis)->m_DecStat)
#define	DecCng		(((PDecoder)pThis)->m_DecCng)


#define NEED_THIS0	PCodec pThis
#define USE_THIS0	pThis

#define NEED_THIS	PCodec pThis,
#define USE_THIS	pThis,


#else /* EM_MT */


emExtern	enum  Crate    WrkRate;

emExtern	emBool  UseHp;
emExtern	emBool  UseVx;

emExtern	CODSTATDEF	CodStat;
emExtern	CODCNGDEF	CodCng;
emExtern	VADSTATDEF	VadStat;

emExtern	emBool  UsePf;

emExtern	DECSTATDEF	DecStat;
emExtern	DECCNGDEF	DecCng;


#define NEED_THIS0	void
#define USE_THIS0

#define NEED_THIS
#define USE_THIS


#endif /* EM_MT */


#endif /* !_EMTTS__EM_G7231_TYPEDEF__H_ */
