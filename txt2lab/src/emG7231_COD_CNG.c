/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:            "cod_cng.c"
**
** √Ë ˆ:     Comfort noise generation
**                  performed at the encoder part
**
** Functions:       Init_Cod_Cng()
**                  Cod_Cng()
**                  Update_Cng()
**
** Local functions:
**                  ComputePastAvFilter()
**                  CalcRC()
**                  LpcDiff()
**
**
*/

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_UTIL_LBC.h"
#include "emG7231_BASOP.h"
#include "emG7231_LSP.h"
#include "emG7231_LPC.h"
#include "emG7231_UTIL_CNG.h"
#include "emG7231_COD_CNG.h"
#include "emG7231_VAD.h"
#include "emG7231_CODER.h"


#if EM_USER_ENCODER_G7231


/* Declaration of local functions */
emStatic void ComputePastAvFilter(NEED_THIS Q15 emInt16 *Coeff);
emStatic void CalcRC(Q15 emInt16 *Coeff, Q15 emInt16 *RC, Q15 emInt16 *shRC);
emStatic emBool LpcDiff(Q15 emInt16 *RC, Q15 emInt16 shRC, Q15 emInt16 *Acf, Q15 emInt16 alpha);

/* »´≤ø±‰¡ø */

/*
**
** Function:        Init_Cod_Cng()
**
** √Ë ˆ:     Initialize Cod_Cng emStatic variables
**
** Links to text:
**
** Arguments:       None
**
** Outputs:         None
**
** ∑µªÿ value:    None
**
*/

void	Init_Cod_Cng(NEED_THIS0)
{
	emInt32 		i;

	LOG_StackAddr(__FUNCTION__);

	CodCng.CurGain = 0;

	for(i=0; i< SizAcf; i++) CodCng.Acf[i] = 0;
	for(i=0; i <= NbAvAcf; i++) CodCng.ShAcf[i] = 40;
	for(i=0; i < LpcOrder; i++) CodCng.SidLpc[i] = 0;

	CodCng.PastFtyp = 1;
	CodCng.RandSeed = 12345;
}


/*
**
** Function:           Cod_Cng()
**
** √Ë ˆ:        Computes Ftyp for inactive frames
**                              0  :  for untransmitted frames
**                              2  :  for SID frames
**                     Computes current frame excitation
**                     Computes current frame LSPs
**                     Computes the coded parameters of SID frames
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 *DataExc    Current frame synthetic excitation
**  Q15 emInt16 *Ftyp     Characterizes the frame type for CNG
**  LINEDEF *Line      Quantized parameters (used for SID frames)
**  Q15 emInt16 *QntLpc     Interpolated frame LPC coefficients
**
** Outputs:
**
**  Q15 emInt16 *DataExc
**  Q15 emInt16 *Ftyp
**  LINEDEF *Line
**  Q15 emInt16 *QntLpc
**
** ∑µªÿ value:       None
**
*/
void Cod_Cng(NEED_THIS Q15 emInt16 *DataExc, Q15 emInt16 *Ftyp, LINEDEF *Line, Q15 emInt16 *QntLpc)
{
    Q15 emInt16 curCoeff[LpcOrder];
    Q15 emInt16 curQGain;
    Q15 emInt16 temp;
    emInt32  i;

	LOG_StackAddr(__FUNCTION__);

 /*
  * Update Ener
  */
    for(i=NbAvGain-1; i>=1; i--) {
        CodCng.Ener[i] = CodCng.Ener[i-1];
    }

 /*
  * Compute LPC filter of present frame
  */
    CodCng.Ener[0] = Durbin(curCoeff, &CodCng.Acf[1], CodCng.Acf[0], &temp);

 /*
  * if first frame of silence => SID frame
  */
    if(CodCng.PastFtyp == 1) {
        *Ftyp = 2;
        CodCng.NbEner = 1;
        curQGain = Qua_SidGain(CodCng.Ener, CodCng.ShAcf, CodCng.NbEner);
    }

    else {
        CodCng.NbEner++;
        if(CodCng.NbEner > NbAvGain) CodCng.NbEner = NbAvGain;
        curQGain = Qua_SidGain(CodCng.Ener, CodCng.ShAcf, CodCng.NbEner);

 /*
  * Compute stationarity of current filter
  * versus reference filter
  */
        if(LpcDiff(CodCng.RC, CodCng.ShRC, CodCng.Acf, *(CodCng.Ener)) == 0) {
            /* transmit SID frame */
            *Ftyp = 2;
        }
        else {
#if 0
            temp = abs_s(sub(curQGain, CodCng.IRef));
#else
			{
				Q15 emInt16 tmp;
				tmp = sub(curQGain, CodCng.IRef);
			    temp = abs_s(tmp);
			}
#endif
            if(temp > ThreshGain) {
                *Ftyp = 2;
            }
            else {
                /* no transmission */
                *Ftyp = 0;
            }
        }
    }

 /*
  * If SID frame : Compute SID filter
  */
    if(*Ftyp == 2) {

 /*
  * Evaluates local stationnarity :
  * Computes difference between current filter and past average filter
  * if signal not locally stationary SID filter = current filter
  * else SID filter = past average filter
  */
        /* Compute past average filter */
        ComputePastAvFilter(USE_THIS CodCng.SidLpc) ;

        /* If adaptation enabled, fill noise filter */
        if ( !VadStat.Aen ) {
            for(i=0; i<LpcOrder; i++) VadStat.NLpc[i] = CodCng.SidLpc[i];
        }

        /* Compute autocorr. of past average filter coefficients */
        CalcRC(CodCng.SidLpc , CodCng.RC, &CodCng.ShRC);

        if(LpcDiff(CodCng.RC, CodCng.ShRC, CodCng.Acf, *(CodCng.Ener)) == 0){
            for(i=0; i<LpcOrder; i++) {
                CodCng.SidLpc[i] = curCoeff[i];
            }
            CalcRC(curCoeff, CodCng.RC, &CodCng.ShRC);
        }

 /*
  * Compute SID frame codes
  */
        /* Compute LspSid */
        AtoLsp(CodCng.LspSid, CodCng.SidLpc, CodStat.PrevLsp);
        Line->LspId = Lsp_Qnt(CodCng.LspSid, CodStat.PrevLsp);
        Lsp_Inq(CodCng.LspSid, CodStat.PrevLsp, Line->LspId, 0);

        Line->Sfs[0].Mamp = curQGain;
        CodCng.IRef = curQGain;
        CodCng.SidGain = Dec_SidGain(CodCng.IRef);

    } /* end of Ftyp=2 case (SID frame) */

 /*
  * Compute new excitation
  */
    if(CodCng.PastFtyp == 1) {
        CodCng.CurGain = CodCng.SidGain;
    }
    else {
          CodCng.CurGain = extract_h(L_add( L_mult(CodCng.CurGain,0x7000),
                    L_mult(CodCng.SidGain,0x1000) ) ) ;
    }
    Calc_Exc_Rand(USE_THIS CodCng.CurGain, CodStat.PrevExc, DataExc,
                                                &CodCng.RandSeed, Line);

 /*
  * Interpolate LSPs and update PrevLsp
  */
    Lsp_Int(QntLpc, CodCng.LspSid, CodStat.PrevLsp);
    for (i=0; i < LpcOrder ; i++) {
        CodStat.PrevLsp[i] = CodCng.LspSid[i];
    }

 /*
  * Output & save frame type info
  */
    CodCng.PastFtyp = *Ftyp;
    return;
}

/*
**
** Function:           Update_Acf()
**
** √Ë ˆ:        Computes & Stores sums of subframe-acfs
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 *Acf_sf     sets of subframes Acfs of current frame
**  Q15 emInt16 *ShAcf_sf   corresponding scaling factors
**
** Output :            None
**
** ∑µªÿ value:       None
**
*/
void Update_Acf(NEED_THIS Q15 emInt16 *Acf_sf, Q15 emInt16 *ShAcf_sf)
{

    emInt32  i, i_subfr;
    Q15 emInt16 *ptr1, *ptr2;
    Q31 emInt32 L_temp[LpcOrderP1];
    Q15 emInt16 sh1, temp;
    Q31 emInt32 L_acc0;

	LOG_StackAddr(__FUNCTION__);

    /* Update Acf and ShAcf */
    ptr2 = CodCng.Acf + SizAcf;
    ptr1 = ptr2 - LpcOrderP1;
    for(i=LpcOrderP1; i<SizAcf; i++) *(--ptr2) = *(--ptr1);
    for(i=NbAvAcf; i>=1; i--) CodCng.ShAcf[i] = CodCng.ShAcf[i-1];

    /* Search ShAcf_sf min for current frame */
    sh1 = ShAcf_sf[0];
    for(i_subfr=1; i_subfr<SubFrames; i_subfr++) {
        if(ShAcf_sf[i_subfr] < sh1) sh1 = ShAcf_sf[i_subfr];
    }
    sh1 = add(sh1, 14);  /* 2 bits of margin */

    /* Compute current sum of acfs */
    for(i=0; i<= LpcOrder; i++) L_temp[i] = 0;

    ptr2 = Acf_sf;
    for(i_subfr=0; i_subfr<SubFrames; i_subfr++) {
        temp = sub(sh1, ShAcf_sf[i_subfr]);
        for(i=0; i <= LpcOrder; i++) {
            L_acc0 = L_deposit_l(*ptr2++);
            L_acc0 = L_shl_1(L_acc0, temp);  /* shift right if temp<0 */
            L_temp[i] = L_add(L_temp[i], L_acc0);
        }
    }
    /* Normalize */
    temp = norm_l(L_temp[0]);
    temp = sub(16, temp);
    if(temp < 0) temp = 0;
    for(i=0; i <= LpcOrder; i++) {
        CodCng.Acf[i] = extract_l(L_shr_1(L_temp[i],temp));
    }

    CodCng.ShAcf[0] = sub(sh1, temp);

    return;
}

/*
**
** Function:           ComputePastAvFilter()
**
** √Ë ˆ:        Computes past average filter
**
** Links to text:
**
** Argument:
**
**  Q15 emInt16 *Coeff      set of LPC coefficients
**
** Output:
**
**  Q15 emInt16 *Coeff
**
** ∑µªÿ value:       None
**
*/
void ComputePastAvFilter(NEED_THIS Q15 emInt16 *Coeff)
{
    emInt32  i, j;
    Q15 emInt16 *ptr_Acf;
    Q31 emInt32 L_sumAcf[LpcOrderP1];
    Q15 emInt16 Corr[LpcOrder], Err;
    Q15 emInt16 sh1, temp;
    Q31 emInt32 L_acc0;

	LOG_StackAddr(__FUNCTION__);

    /* Search ShAcf min */
    sh1 = CodCng.ShAcf[1];
    for(i=2; i <= NbAvAcf; i ++) {
        temp = CodCng.ShAcf[i];
        if(temp < sh1) sh1 = temp;
    }
    sh1 = add(sh1, 14);     /* 2 bits of margin : NbAvAcf <= 4 */

    /* Compute sum of NbAvAcf frame-Acfs  */
    for(j=0; j <= LpcOrder; j++) L_sumAcf[j] = 0;

    ptr_Acf = CodCng.Acf + LpcOrderP1;
    for(i=1; i <= NbAvAcf; i ++) {
        temp = sub(sh1, CodCng.ShAcf[i]);
        for(j=0; j <= LpcOrder; j++) {
            L_acc0 = L_deposit_l(*ptr_Acf++);
            L_acc0 = L_shl_1(L_acc0, temp); /* shift right if temp<0 */
            L_sumAcf[j] = L_add(L_sumAcf[j], L_acc0);
        }
    }

    /* Normalize */
    temp = norm_l(L_sumAcf[0]);
    temp = sub(16, temp);
    if(temp < 0) temp = 0;
    Err = extract_l(L_shr(L_sumAcf[0],temp));
    for(i=1; i<LpcOrderP1; i++) {
        Corr[i-1] = extract_l(L_shr_1(L_sumAcf[i],temp));
    }

    Durbin(Coeff, Corr, Err, &temp);

    return;
}

/*
**
** Function:           CalcRC()
**
** √Ë ˆ:        Computes function derived from
**                     the autocorrelation of LPC coefficients
**                     used for Itakura distance
**
** Links to text:
**
** Arguments :
**
**  Q15 emInt16 *Coeff      set of LPC coefficients
**  Q15 emInt16 *RC         derived from LPC coefficients autocorrelation
**  Q15 emInt16 *ShRC       corresponding scaling factor
**
** Outputs :
**
**  Q15 emInt16 *RC
**  Q15 emInt16 *ShRC
**
** ∑µªÿ value:       None
**
*/
void CalcRC(Q15 emInt16 *Coeff, Q15 emInt16 *RC, Q15 emInt16 *ShRC)
{
    emInt32  i, j;
    Q15 emInt16 sh1;
    Q31 emInt32 L_acc;

	LOG_StackAddr(__FUNCTION__);

    L_acc = 0L;
    for(j=0; j<LpcOrder; j++) {
        L_acc = L_mac(L_acc, Coeff[j], Coeff[j]);
    }
    L_acc = L_shr(L_acc, 1);
    L_acc = L_add(L_acc, 0x04000000L);  /* 1 << 2 * Lpc_justif. */
    sh1 = norm_l(L_acc) - (Q15 emInt16)2;    /* 1 bit because of x2 in RC[i], i> 0*/
                                /* & 1 bit margin for Itakura distance */
    L_acc = L_shl(L_acc, sh1); /* shift right if < 0 */
    RC[0] = round(L_acc);

    for(i=1; i<=LpcOrder; i++) {
        L_acc = L_mult( (Q15 emInt16) 0xE000, Coeff[i-1]);   /* - (1 << Lpc_justif.) */
        for(j=0; j<LpcOrder-i; j++) {
            L_acc = L_mac(L_acc, Coeff[j], Coeff[j+i]);
        }
        L_acc = L_shl(L_acc, sh1);
        RC[i] = round(L_acc);
    }
    *ShRC = sh1;
    return;
}

/*
**
** Function:           LpcDiff()
**
** √Ë ˆ:        Comparison of two filters
**                     using Itakura distance
**                     1st filter : defined by *ptrAcf
**                     2nd filter : defined by *RC
**                     the autocorrelation of LPC coefficients
**                     used for Itakura distance
**
** Links to text:
**
** Arguments :
**
**  Q15 emInt16 *RC         derived from LPC coefficients autocorrelation
**  Q15 emInt16 ShRC        corresponding scaling factor
**  Q15 emInt16 *ptrAcf     pointer on signal autocorrelation function
**  Q15 emInt16 alpha       residual energy in LPC analysis using *ptrAcf
**
** Output:             None
**
** ∑µªÿ value:       flag = 1 if similar filters
**                     flag = 0 if different filters
**
*/
emBool LpcDiff(Q15 emInt16 *RC, Q15 emInt16 ShRC, Q15 emInt16 *ptrAcf, Q15 emInt16 alpha)
{
    Q31 emInt32 L_temp0, L_temp1;
    Q15 emInt16 temp;
    emInt32  i;
    emBool diff;

	LOG_StackAddr(__FUNCTION__);

    L_temp0 = 0L;
    for(i=0; i<=LpcOrder; i++) {
        temp = shr_1(ptrAcf[i], 2);  /* + 2 margin bits */
        L_temp0 = L_mac(L_temp0, RC[i], temp);
    }

    temp = mult_r(alpha, FracThresh);
    L_temp1 = L_add((Q31 emInt32)temp, (Q31 emInt32)alpha);
    temp = add(ShRC, 9);  /* 9 = Lpc_justif. * 2 - 15 - 2 */
    L_temp1 = L_shl(L_temp1, temp);

    if(L_temp0 < L_temp1) diff = 1;
    else diff = 0;
    return(diff);
}


#endif /* EM_USER_ENCODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
