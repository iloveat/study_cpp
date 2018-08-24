/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "cst_lbc.h"
**
** √Ë ˆ:  
**    LBC Coder for 6.3/5.3 kbps.
**
*/

#ifndef _EMTTS__EM_G7231_CST_LBC__H_
#define _EMTTS__EM_G7231_CST_LBC__H_

#include "emG7231_TYPEDEF.h"

/* Definition of the working mode */
enum  Wmode { Both, Cod, Dec } ;

/* Coder rate */
enum  Crate    { Rate63, Rate53 } ;


#define  Frame       240
#define  LpcFrame    180
#define  SubFrames   4
#define  SubFrLen    (Frame/SubFrames)

/* LPC constants */
#define  LpcOrder          10
#define  RidgeFact         10
#define  CosineTableSize   512
#define  PreCoef           ((Q15 emInt16)0xc000)            /* -0.25*2 */

#define  LspPrd0           12288
#define  LspPrd1           23552

#define  LspQntBands       3
#define  LspCbSize         256
#define  LspCbBits         8

/* LTP constants */
#define  PitchMin          18
#define  PitchMax          (PitchMin+127)
#define  PwConst           ((Q15 emInt16)0x2800)
#define  PwRange           3
#define  ClPitchOrd        5
#define  Pstep             1
#define NbFilt085          85
#define NbFilt170          170

/* MP-MLQ constants */
#define  Sgrid             2
#define  MaxPulseNum       6
#define  MlqSteps          2

/* acelp constants */
#define SubFrLen2          (SubFrLen +4)
#define DIM_RR             416
#define NB_POS             8
#define STEP               8
#define MSIZE              64
#define threshold          16384  /* 0.5 = 16384 in Q15 */
#define max_time           120

/* Gain constant */
#define  NumOfGainLev      24

/* FER constant */
#define  ErrMaxNum         3

/* CNG constants  */
#define NbAvAcf            3  /* Nb of frames for Acf average               */
#define NbAvGain           3  /* Nb of frames for gain average              */
#define ThreshGain         3  /* Theshold for quantized gains               */
#define FracThresh         7000   /* Itakura dist threshold: frac. part     */
#define NbPulsBlk          11 /* Nb of pulses in 2-subframes blocks         */

#define InvNbPulsBlk       2979 /* 32768/NbPulsBlk                          */
#define NbFilt             50 /* number of filters for CNG exc generation   */
#define LpcOrderP1         (LpcOrder+1)
#define SizAcf             ((NbAvAcf+1)*LpcOrderP1) /* size of array Acf    */
#define SubFrLenD          (2*SubFrLen)
#define Gexc_Max           5000  /* Maximum gain for fixed CNG excitation   */

/* Taming constants */
#define NbFilt085_min      51
#define NbFilt170_min      93
#define SizErr             5
#define Err0               (Q31 emInt32)4  /* scaling factor */
#define ThreshErr          0x40000000L
#define DEC                (30 - 7)

/*
   Used structures
*/
typedef  struct   {
   /* High pass variables */
   Q15 emInt16   HpfZdl   ;
   Q31 emInt32   HpfPdl   ;

   /* Sine wave detector */
   Q15 emInt16   SinDet   ;

   /* Lsp previous vector */
   Q15 emInt16   PrevLsp[LpcOrder] ;

   /* All pitch operation buffers */
   Q15 emInt16   PrevWgt[PitchMax] ;
   Q15 emInt16   PrevErr[PitchMax] ;
   Q15 emInt16   PrevExc[PitchMax] ;

   /* Required memory for the delay */
   Q15 emInt16   PrevDat[LpcFrame-SubFrLen] ;

   /* Used delay lines */
   Q15 emInt16   WghtFirDl[LpcOrder] ;
   Q15 emInt16   WghtIirDl[LpcOrder] ;
   Q15 emInt16   RingFirDl[LpcOrder] ;
   Q15 emInt16   RingIirDl[LpcOrder] ;

   /* Taming procedure errors */
   Q31 emInt32 Err[SizErr];

   } CODSTATDEF  ;

typedef  struct   {
   Q15 emInt16   Ecount ;
   Q15 emInt16   InterGain ;
   Q15 emInt16   InterIndx ;
   Q15 emInt16   Rseed ;
   Q15 emInt16   Park  ;
   Q15 emInt16   Gain  ;
   /* Lsp previous vector */
   Q15 emInt16   PrevLsp[LpcOrder] ;

   /* All pitch operation buffers */
   Q15 emInt16   PrevExc[PitchMax] ;

   /* Used delay lines */
   Q15 emInt16   SyntIirDl[LpcOrder] ;
   Q15 emInt16   PostFirDl[LpcOrder] ;
   Q15 emInt16   PostIirDl[LpcOrder] ;

   } DECSTATDEF  ;

   /* subframe coded parameters */
typedef  struct   {
   Q15 emInt16   AcLg  ;
   Q15 emInt16   AcGn  ;
   Q15 emInt16   Mamp  ;
   Q15 emInt16   Grid  ;
   Q15 emInt16   Tran  ;
   Q15 emInt16   Pamp  ;
   Q31 emInt32   Ppos  ;
   } SFSDEF ;

   /* frame coded parameters */
typedef  struct   {
   Q15 emInt16   Crc   ;
   Q31 emInt32   LspId ;
   Q15 emInt16   Olp[SubFrames/2] ;
   SFSDEF   Sfs[SubFrames] ;
   } LINEDEF ;

   /* harmonic noise shaping filter parameters */
typedef  struct   {
   Q15 emInt16   Indx  ;
   Q15 emInt16   Gain  ;
   } PWDEF  ;

    /* pitch postfilter parameters */
typedef  struct   {
   Q15 emInt16   Indx  ;
   Q15 emInt16   Gain  ;
   Q15 emInt16   ScGn  ;
   } PFDEF  ;

    /* best excitation vector parameters for the high rate */
typedef  struct {
   Q31 emInt32   MaxErr   ;
   Q15 emInt16   GridId   ;
   Q15 emInt16   MampId   ;
   Q15 emInt16   UseTrn   ;
   Q15 emInt16   Ploc[MaxPulseNum] ;
   Q15 emInt16   Pamp[MaxPulseNum] ;
   } BESTDEF ;

    /* VAD emStatic variables */
typedef struct {
    Q15 emInt16  Hcnt ;
    Q15 emInt16  Vcnt ;
    Q31 emInt32  Penr ;
    Q31 emInt32  Nlev ;
    Q15 emInt16  Aen ;
    Q15 emInt16  Polp[4] ;
    Q15 emInt16  NLpc[LpcOrder] ;
} VADSTATDEF ;


/* CNG features */

/* Coder part */
typedef struct {
    Q15 emInt16 CurGain;
    Q15 emInt16 PastFtyp;
    Q15 emInt16 Acf[SizAcf];
    Q15 emInt16 ShAcf[NbAvAcf+1];
    Q15 emInt16 LspSid[LpcOrder] ;
    Q15 emInt16 SidLpc[LpcOrder] ;
    Q15 emInt16 RC[LpcOrderP1];
    Q15 emInt16 ShRC;
    Q15 emInt16 Ener[NbAvGain];
    Q15 emInt16 NbEner;
    Q15 emInt16 IRef;
    Q15 emInt16 SidGain;
    Q15 emInt16 RandSeed;
} CODCNGDEF;

/* Decoder part */
typedef struct {
    Q15 emInt16 CurGain;
    Q15 emInt16 PastFtyp;
    Q15 emInt16 LspSid[LpcOrder] ;
    Q15 emInt16 SidGain;
    Q15 emInt16 RandSeed;
} DECCNGDEF;

/* G.723.1 Encoder Handle Define*/
typedef struct  {
	VADSTATDEF  VadStat;
	CODSTATDEF  CodStat;
	CODCNGDEF   CodCng;
	enum Crate WrkRate;
	emBool    UseHp;
	emBool    UseVx;
}g7231_encode_flag ;

typedef void* g7231_encode_handle;

/* G.723.1 Decoder Handle Define*/
typedef struct {
	DECSTATDEF  DecStat;
	DECCNGDEF DecCng;
	enum Crate    WrkRate;
	emBool    UsePf;
}g7231_decode_flag ;

typedef void* g7231_decode_handle;

#endif/*_EMTTS__EM_G7231_CST_LBC__H_*/
