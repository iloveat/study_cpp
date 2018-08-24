/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.0
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:    tame.c
**
** √Ë ˆ: Functions used to avoid possible explosion of the decoder
**              excitation in case of series of long term unstable filters
**              and when the encoder and the decoder are de-synchronized
**
** Functions:
**
**  Computing excitation error estimation :
**      Update_Err()
**  Test excitation error :
**      Test_Err()
*/

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_BASOP.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_CODER.h"
#include "emG7231_EXC_LBC.h"
#include "emG7231_TAME.h"
#include "emG7231_TAB_LBC.h"


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Update_Err()
**
** √Ë ˆ:     Estimation of the excitation error associated
**                  to the excitation signal when it is disturbed at
**                  the decoder, the disturbing signal being filtered
**                  by the long term synthesis filters
**                  one value for (SubFrLen/2) samples
**                  Updates the table CodStat.Err
**
** Links to text:   Section
**
** Arguments:
**
**  Q15 emInt16 Olp      Center value for pitch delay
**  Q15 emInt16 AcLg     Offset value for pitch delay
**  Q15 emInt16 AcGn     Index of Gain LT filter
**
** Outputs: None
**
** ∑µªÿ value:    None
**
*/

void Update_Err( NEED_THIS Q15 emInt16 Olp, Q15 emInt16 AcLg, Q15 emInt16 AcGn )
{
    Q15 emInt16 *ptr_tab;
    Q15 emInt16 i, iz, temp1, temp2;
    Q15 emInt16 Lag;
    Q31 emInt32 Worst1, Worst0, L_temp;
    Q15 emInt16 beta;

	LOG_StackAddr(__FUNCTION__);

    Lag = Olp - (Q15 emInt16)Pstep + AcLg;

    /* Select Quantization tables */
    i = 0 ;
    ptr_tab = (Q15 emInt16*)tabgain85;
    if ( WrkRate == Rate63 ) {
        if ( Olp >= (Q15 emInt16) (SubFrLen-2) ) ptr_tab = (Q15 emInt16*)tabgain170;
    }
    else {
        ptr_tab = (Q15 emInt16*)tabgain170;
    }
    beta = ptr_tab[(emInt32 )AcGn]; /* beta = gain * 8192 */


    if(Lag <= (SubFrLen/2)) {
        Worst0 = L_mls(CodStat.Err[0], beta);
        Worst0 = L_shl(Worst0, 2);
        Worst0 = L_add(Err0, Worst0);
        Worst1 = Worst0;
    }

    else {
        iz = mult(Lag, 1092);   /* Lag / 30 */
        temp1 = add(iz, 1);
        temp2 = sub(shl(temp1, 5), shl(temp1, 1));      /* 30 (iz+1) */
        if(temp2 != Lag) {

            if(iz == 1) {
                Worst0 = L_mls(CodStat.Err[0], beta);
                Worst0 = L_shl(Worst0, 2);
                Worst0 = L_add(Err0, Worst0);
                Worst1 = L_mls(CodStat.Err[1], beta);
                Worst1 = L_shl(Worst1, 2);
                Worst1 = L_add(Err0, Worst1);
                if(Worst0 > Worst1) Worst1 = Worst0;
                else Worst0 = Worst1;
            }

            else {
                Worst0 = L_mls(CodStat.Err[iz-2], beta);
                Worst0 = L_shl(Worst0, 2);
                Worst0 = L_add(Err0, Worst0);
                L_temp = L_mls(CodStat.Err[iz-1], beta);
                L_temp = L_shl(L_temp, 2);
                L_temp = L_add(Err0, L_temp);
                if(L_temp > Worst0) Worst0 = L_temp;
                Worst1 = L_mls(CodStat.Err[iz], beta);
                Worst1 = L_shl(Worst1, 2);
                Worst1 = L_add(Err0, Worst1);
                if(L_temp > Worst1) Worst1 = L_temp;
            }
        }
        else {  /* Lag % SubFrLen = 0 */
            Worst0 = L_mls(CodStat.Err[iz-1], beta);
            Worst0 = L_shl(Worst0, 2);
            Worst0 = L_add(Err0, Worst0);
            Worst1 = L_mls(CodStat.Err[iz], beta);
            Worst1 = L_shl(Worst1, 2);
            Worst1 = L_add(Err0, Worst1);
        }
    }

    for(i=4; i>=2; i--) {
        CodStat.Err[i] = CodStat.Err[i-2];
    }
    CodStat.Err[0] = Worst0;
    CodStat.Err[1] = Worst1;

    return;
}

/*
**
** Function:        Test_Err()
**
** √Ë ˆ:     Check the error excitation maximum for
**                  the subframe and computes an index iTest used to
**                  calculate the maximum nb of filters (in Find_Acbk) :
**                  Bound = Min(Nmin + iTest x pas, Nmax) , with
**                  AcbkGainTable085 : pas = 2, Nmin = 51, Nmax = 85
**                  AcbkGainTable170 : pas = 4, Nmin = 93, Nmax = 170
**                  iTest depends on the relative difference between
**                  errmax and a fixed threshold
**
** Links to text:   Section
**
** Arguments:
**
**  Q15 emInt16 Lag1      1st long term Lag of the tested zone
**  Q15 emInt16 Lag2      2nd long term Lag of the tested zone
**
** Outputs: None
**
** ∑µªÿ value:
**  Q15 emInt16          index iTest used to compute Acbk number of filters
*/

Q15 emInt16 Test_Err( NEED_THIS Q15 emInt16 Lag1, Q15 emInt16 Lag2 )
{

    emInt32  i, i1, i2;
    Q15 emInt16 zone1, zone2;
    Q31 emInt32 Acc, Err_max;
    Q15 emInt16 iTest;

	LOG_StackAddr(__FUNCTION__);

    i2 = Lag2 + ClPitchOrd/2;
    zone2 = mult( (Q15 emInt16) i2, (Q15 emInt16) 1092);

    i1 = - SubFrLen + 1 + Lag1 - ClPitchOrd/2;
    if(i1 <= 0) i1 = 1;
    zone1 = mult( (Q15 emInt16) i1, (Q15 emInt16) 1092);

    Err_max = -1L;
    for(i=zone2; i>=zone1; i--) {
        Acc = L_sub(CodStat.Err[i], Err_max);
        if(Acc > 0L) {
                Err_max = CodStat.Err[i];
        }
    }
    Acc = L_sub(Err_max, ThreshErr);
    if((Acc > 0L) || (CodStat.SinDet < 0 ) ) {
        iTest = 0;
    }
    else {
        Acc = L_negate(Acc);
        Acc = L_shr(Acc, DEC);
        iTest = extract_l(Acc);
    }

    return(iTest);
}


#endif /* EM_USER_ENCODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
