/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:    exc_lbc.c
**
** √Ë ˆ: Functions that implement adaptive and fixed codebook
**       operations.
**
** Functions:
**
**  Computing Open loop Pitch lag:
**
**      Estim_Pitch()
**
**  Harmonic noise weighting:
**
**      Comp_Pw()
**      Filt_Pw()
**
**  Fixed Cobebook computation:
**
**      Find_Fcbk()
**      Gen_Trn()
**      Find_Best()
**      Find_Pack()
**      Find_Unpk()
**      ACELP_LBC_code()
**      Cor_h()
**      Cor_h_X()
**      reset_max_time()
**      D4i64_LBC()
**      G_code()
**      search_T0()
**
**  Adaptive Cobebook computation:
**
**      Find_Acbk()
**      Get_Rez()
**      Decod_Acbk()
**
**  Pitch postfilter:
**      Comp_Lpf()
**      Find_B()
**      Find_F()
**      Filt_Lpf()
**
**  Residual interpolation:
**
**      Comp_Info()
**      Regen()
**
*/

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_BASOP.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_UTIL_LBC.h"
#include "emG7231_EXC_LBC.h"
#include "emG7231_TAME.h"
#include "emG7231_UTIL_CNG.h"


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Estim_Pitch()
**
** √Ë ˆ: Open loop pitch estimation made twice per frame (one for
**              the first two subframes and one for the last two).
**              The method is based on the maximization of the
**              crosscorrelation of the speech.
**
** Links to text:   Section 2.9
**
** Arguments:
**
**  Q15 emInt16 *Dpnt    Perceptually weighted speech
**  Q15 emInt16 Start    Starting index defining the subframes under study
**
** Outputs:
**
** ∑µªÿ value:
**
**  Q15 emInt16      Open loop pitch period
**
*/
Q15 emInt16   Estim_Pitch( Q15 emInt16 *Dpnt, Q15 emInt16 Start )
{
    emInt32    i,j   ;

    Q31 emInt32   Acc0,Acc1   ;

    Q15 emInt16   Exp,Tmp  ;
    Q15 emInt16   Ccr,Enr  ;

    Q15 emInt16   Indx = (Q15 emInt16) PitchMin ;

    Q15 emInt16   Mxp = (Q15 emInt16) 30 ;
    Q15 emInt16   Mcr = (Q15 emInt16) 0x4000 ;
    Q15 emInt16   Mnr = (Q15 emInt16) 0x7fff ;

    Q15 emInt16   Pr ;

	LOG_StackAddr(__FUNCTION__);

    /* Init the energy estimate */
    Pr = Start - (Q15 emInt16)PitchMin + (Q15 emInt16)1 ;
    Acc1 = (Q31 emInt32) 0 ;
    for ( j = 0 ; j < 2*SubFrLen ; j ++ )
        Acc1 = L_mac( Acc1, Dpnt[Pr+j], Dpnt[Pr+j] ) ;

    /* Main Olp search loop */
    for ( i = PitchMin ; i <= PitchMax-3 ; i ++ ) {

        Pr = sub( Pr, (Q15 emInt16) 1 ) ;

        /* Energy update */
        Acc1 = L_msu( Acc1, Dpnt[Pr+2*SubFrLen], Dpnt[Pr+2*SubFrLen] ) ;
        Acc1 = L_mac( Acc1, Dpnt[Pr], Dpnt[Pr] ) ;

        /*  Compute the cross */
        Acc0 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < 2*SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Dpnt[Start+j], Dpnt[Pr+j] ) ;

        if ( Acc0 > (Q31 emInt32) 0 ) {

            /* Compute Exp and mant of the cross */
            Exp = norm_l( Acc0 ) ;
            Acc0 = L_shl_1( Acc0, Exp ) ;
            Exp = shl( Exp, (Q15 emInt16) 1 ) ;
            Ccr = round_1( Acc0 ) ;
            Acc0 = L_mult( Ccr, Ccr ) ;
            Ccr = norm_l( Acc0 ) ;
            Acc0 = L_shl_1( Acc0, Ccr ) ;
            Exp = add( Exp, Ccr ) ;
            Ccr = extract_h( Acc0 ) ;

            /* Do the same with energy */
            Acc0 = Acc1 ;
            Enr = norm_l( Acc0 ) ;
            Acc0 = L_shl_1( Acc0, Enr ) ;
            Exp = sub( Exp, Enr ) ;
            Enr = round_1( Acc0 ) ;

            if ( Ccr >= Enr ) {
                Exp = sub( Exp, (Q15 emInt16) 1 ) ;
                Ccr = shr( Ccr, (Q15 emInt16) 1 ) ;
            }

            if ( Exp <= Mxp ) {

                if ( (Exp+1) < Mxp ) {
                    Indx = (Q15 emInt16) i ;
                    Mxp = Exp ;
                    Mcr = Ccr ;
                    Mnr = Enr ;
                    continue ;
                }

                if ( (Exp+1) == Mxp )
                    Tmp = shr( Mcr, (Q15 emInt16) 1 ) ;
                else
                    Tmp = Mcr ;

                /* Compare with equal exponents */
                Acc0 = L_mult( Ccr, Mnr ) ;
                Acc0 = L_msu( Acc0, Enr, Tmp ) ;
                if ( Acc0 > (Q31 emInt32) 0 ) {

                    if ( ((Q15 emInt16)i - Indx) < (Q15 emInt16) PitchMin ) {
                        Indx = (Q15 emInt16) i ;
                        Mxp = Exp ;
                        Mcr = Ccr ;
                        Mnr = Enr ;
                    }

                    else {
                        Acc0 = L_mult( Ccr, Mnr ) ;
#if 0
                        Acc0 = L_negate(L_shr( Acc0, (Q15 emInt16) 2 ) ) ;
#else
						{
							Q31 emInt32 L_temp;
							L_temp = L_shr( Acc0, (Q15 emInt16) 2 );
							Acc0 = L_negate(L_temp);
						}
#endif
                        Acc0 = L_mac( Acc0, Ccr, Mnr ) ;
                        Acc0 = L_msu( Acc0, Enr, Tmp ) ;
                        if ( Acc0 > (Q31 emInt32) 0 ) {
                            Indx = (Q15 emInt16) i ;
                            Mxp = Exp ;
                            Mcr = Ccr ;
                            Mnr = Enr ;
                        }
                    }
                }
            }
        }
    }

    return Indx ;
}

/*
**
** Function:        Comp_Pw()
**
** √Ë ˆ:     Computes harmonic noise filter coefficients.
**                  For each subframe, the optimal lag is searched around the
**                  open loop pitch lag based on only positive correlation
**                  maximization.
**
** Links to text:   Section 2.11
**
** Arguments:
**
**  Q15 emInt16 *Dpnt    Formant perceptually weighted speech
**  Q15 emInt16 Start
**  Q15 emInt16 Olp      Open loop pitch lag
**
** Outputs:         None
**
** ∑µªÿ value:
**
**  PWDEF   Q15 emInt16  Indx  lag of the harmonic noise shaping filter
**          Q15 emInt16  Gain  gain of the harmonic noise shaping filter
**
*/
PWDEF Comp_Pw( Q15 emInt16 *Dpnt, Q15 emInt16 Start, Q15 emInt16 Olp )
{

    emInt32    i,j   ;

    Q31 emInt32   Lcr[15] ;
    Q15 emInt16   Scr[15] ;
    PWDEF    Pw ;

    Q31 emInt32   Acc0,Acc1   ;
    Q15 emInt16   Exp   ;

    Q15 emInt16   Ccr,Enr  ;
    Q15 emInt16   Mcr,Mnr  ;

	LOG_StackAddr(__FUNCTION__);

    /* Compute and save target energy */
    Lcr[0] = (Q31 emInt32) 0 ;
    for ( i = 0 ; i < SubFrLen ; i ++ )
        Lcr[0] = L_mac( Lcr[0], Dpnt[Start+i], Dpnt[Start+i] ) ;

    /* Compute all Crosses and energys */
    for ( i = 0 ; i <= 2*PwRange ; i ++ ) {

        Acc1 = Acc0 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ ) {
            Acc0 = L_mac( Acc0, Dpnt[Start+j],
                                            Dpnt[Start-(Olp-PwRange+i)+j]) ;
            Acc1 = L_mac( Acc1, Dpnt[Start-(Olp-PwRange+i)+j],
                                            Dpnt[Start-(Olp-PwRange+i)+j] ) ;
        }

        /* Save both */
        Lcr[2*i+1] = Acc1 ;
        Lcr[2*i+2] = Acc0 ;
    }

    /* Normalize to maximum */
    Acc1 = (Q31 emInt32) 0 ;
    for ( i = 0 ; i < 15 ; i ++ ) {
        Acc0 = Lcr[i] ;
        Acc0 = L_abs( Acc0 ) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    Exp = norm_l( Acc1 ) ;
    for ( i = 0 ; i < 15 ; i ++ ) {
        Acc0 = L_shl_1( Lcr[i], Exp ) ;
        Scr[i] = round_1( Acc0 ) ;

    }

    /* Find the best pair */
    Pw.Indx = (Q15 emInt16) -1 ;
    Pw.Gain = (Q15 emInt16) 0  ;

    Mcr = (Q15 emInt16) 1 ;
    Mnr = (Q15 emInt16) 0x7fff ;

    for ( i = 0 ; i <= 2*PwRange ; i ++ ) {

        Enr = Scr[2*i+1] ;
        Ccr = Scr[2*i+2] ;

        if ( Ccr <= (Q15 emInt16) 0 )
            continue ;

        Exp = mult_r( Ccr, Ccr ) ;

        /* Compute the cross */
        Acc0 = L_mult( Exp, Mnr ) ;
        Acc0 = L_msu ( Acc0, Enr, Mcr ) ;

        if ( Acc0 > (Q31 emInt32) 0 ) {
            Mcr = Exp ;
            Mnr = Enr ;
            Pw.Indx = (Q15 emInt16)i ;
        }
    }

    if ( Pw.Indx == -1 ) {
        Pw.Indx = Olp ;
        return Pw ;
    }

    /* Check the db limit */
    Acc0 = L_mult( Scr[0], Mnr ) ;
    Acc1 = Acc0 ;
    Acc0 = L_shr( Acc0, (Q15 emInt16) 2 ) ;
    Acc1 = L_shr( Acc1, (Q15 emInt16) 3 ) ;
    Acc0 = L_add( Acc0, Acc1 ) ;
    Acc1 = L_mult( Scr[2*Pw.Indx+2], Scr[2*Pw.Indx+2] ) ;
    Acc0 = L_sub( Acc0, Acc1 ) ;

    if ( Acc0 < (Q31 emInt32) 0 ) {

        Exp = Scr[2*Pw.Indx + 2] ;

        if ( Exp >= Mnr )
            Pw.Gain = PwConst ;
        else {
            Pw.Gain = div_s( Exp, Mnr ) ;
            Pw.Gain = mult_r( Pw.Gain, PwConst ) ;
        }
    }

    Pw.Indx = Olp - PwRange + Pw.Indx ;

    return Pw ;

}

/*
**
** Function:        Filt_Pw()
**
** √Ë ˆ:     Applies harmonic noise shaping filter.
**                  Lth order FIR filter on each subframe (L: lag of the filter).
**
** Links to text:   Section 2.11
**
** Arguments:
**
**  Q15 emInt16 *DataBuff    Target vector
**  Q15 emInt16 *Dpnt        Formant perceptually weighted speech
**  Q15 emInt16 Start
**  PWDEF   Pw          ≤Œ ˝ of the harmonic noise shaping filter
**
** Outputs:
**
**  Q15 emInt16 *DataBuff    Target vector
**
** ∑µªÿ value:        None
**
*/
void  Filt_Pw( Q15 emInt16 *DataBuff, Q15 emInt16 *Dpnt, Q15 emInt16 Start, PWDEF Pw )
{
    emInt32    i  ;

    Q31 emInt32   Acc0 ;

	LOG_StackAddr(__FUNCTION__);

    /* Perform the harmonic weighting */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_deposit_h( Dpnt[PitchMax+Start+i] ) ;
        Acc0 = L_msu( Acc0, Pw.Gain, Dpnt[PitchMax+Start-Pw.Indx+i] ) ;
        DataBuff[Start+(Q15 emInt16)i] = round( Acc0 ) ;
    }

    return;
}

/*
**
** Function:        Find_Fcbk()
**
** √Ë ˆ:     Fixed codebook excitation computation.
**
**
** Links to text:   Sections 2.15 & 2.16
**
** Arguments:
**
**  Q15 emInt16 *Dpnt    Target vector
**  Q15 emInt16 *ImpResp Impulse response of the synthesis filter
**  LineDef *Line   Excitation parameters for one subframe
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:
**
**  Q15 emInt16 *Dpnt    Excitation vector
**  LINEDEF *Line   Fixed codebook parameters for one subframe
**
** ∑µªÿ value:        None
**
*/
void  Find_Fcbk( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *ImpResp, LINEDEF *Line, Q15 emInt16 Sfc )
{
    emInt32    i  ;
    Q15 emInt16 T0_acelp, gain_T0;
    Q15 emInt16   Srate ;


    BESTDEF  Best ;

	LOG_StackAddr(__FUNCTION__);

    switch(WrkRate)  {

        case Rate63: {

            Srate = Nb_puls[(emInt32 )Sfc] ;
            Best.MaxErr = (Q31 emInt32) 0xc0000000L ;
            Find_Best( &Best, Dpnt, ImpResp, Srate, (Q15 emInt16) SubFrLen ) ;
            if ( (*Line).Olp[Sfc>>1] < (Q15 emInt16) (SubFrLen-2) ) {
                Find_Best( &Best, Dpnt, ImpResp, Srate, (*Line).Olp[Sfc>>1]);
            }

            /* Reconstruct the excitation */
            for ( i = 0 ; i <  SubFrLen ; i ++ )
                Dpnt[i] = (Q15 emInt16) 0 ;
            for ( i = 0 ; i < Srate ; i ++ )
                Dpnt[Best.Ploc[i]] = Best.Pamp[i] ;

            /* Code the excitation */
            Fcbk_Pack( Dpnt, &((*Line).Sfs[Sfc]), &Best, Srate ) ;

            if ( Best.UseTrn == (Q15 emInt16) 1 )
                Gen_Trn( Dpnt, Dpnt, (*Line).Olp[Sfc>>1] ) ;

            break;
        }

        case Rate53: {

            T0_acelp = search_T0(
                    (Q15 emInt16) ((*Line).Olp[Sfc>>1]-1+(*Line).Sfs[Sfc].AcLg),
                    (*Line).Sfs[Sfc].AcGn, &gain_T0 );

            (*Line).Sfs[Sfc].Ppos = ACELP_LBC_code(
                    Dpnt, ImpResp, T0_acelp, Dpnt, &(*Line).Sfs[Sfc].Mamp,
                    &(*Line).Sfs[Sfc].Grid, &(*Line).Sfs[Sfc].Pamp, gain_T0 );

            (*Line).Sfs[Sfc].Tran = 0;

            break;
        }
    }

    return;
}


#endif /* EM_USER_ENCODER_G7231 */


/*
**
** Function:        Gen_Trn()
**
** √Ë ˆ:     Generation of a train of Dirac functions with the period
**                  Olp.
**
** Links to text:   Section 2.15
**
** Arguments:
**
**  Q15 emInt16 *Dst     Fixed codebook excitation vector with  train of Dirac
**  Q15 emInt16 *Src     Fixed codebook excitation vector without train of Dirac
**  Q15 emInt16 Olp      Closed-loop pitch lag of subframe 0 (for subframes 0 & 1)
**                  Closed-loop pitch lag of subframe 2 (for subframes 2 & 3)
**
** Outputs:
**
**  Q15 emInt16 *Dst     excitation vector
**
** ∑µªÿ value:    None
**
*/
void  Gen_Trn( Q15 emInt16 *Dst, Q15 emInt16 *Src, Q15 emInt16 Olp )
{
    emInt32    i  ;

    Q15 emInt16   Tmp0,Tmp1   ;
    Q15 emInt16   Tmp[SubFrLen] ;

	LOG_StackAddr(__FUNCTION__);

    Tmp0 = Olp ;

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Tmp[i] = Src[i] ;
        Dst[i] = Src[i] ;
    }

    while ( Tmp0 < SubFrLen ) {
        for ( i = (emInt32 ) Tmp0 ; i < SubFrLen ; i ++ ) {
            Tmp1 = add( Dst[i], Tmp[i-(emInt32 )Tmp0] ) ;
            Dst[i] = Tmp1 ;
        }
        Tmp0 = add( Tmp0, Olp ) ;
    }

    return;
}


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Find_Best()
**
** √Ë ˆ:     Fixed codebook search for the high rate encoder.
**                  It performs the quantization of the residual signal.
**                  The excitation made of Np positive or negative pulses
**                  multiplied by a gain and whose positions on the grid are
**                  either all odd or all even, should approximate as best as
**                  possible the residual signal (perceptual criterion).
**
** Links to text:   Section 2.15
**
** Arguments:
**
**  BESTDEF *Best   ≤Œ ˝ of the best excitation model
**  Q15 emInt16 *Tv      Target vector
**  Q15 emInt16 *ImpResp Impulse response of the combined filter
**  Q15 emInt16 Np       Number of pulses (6 for even subframes; 5 for odd subframes)
**  Q15 emInt16 Olp      Closed-loop pitch lag of subframe 0 (for subframes 0 & 1)
**                  Closed-loop pitch lag of subframe 2 (for subframes 2 & 3)
**
** Outputs:
**
**  BESTDEF *Best
**
** ∑µªÿ value:    None
**
*/
void  Find_Best( BESTDEF *Best, Q15 emInt16 *Tv, Q15 emInt16 *ImpResp, Q15 emInt16 Np,
Q15 emInt16 Olp )
{

    emInt32    i,j,k,l  ;
    BESTDEF  Temp  ;

    Q15 emInt16   Exp   ;
    Q15 emInt16   MaxAmpId ;
    Q15 emInt16   MaxAmp   ;
    Q31 emInt32   Acc0,Acc1,Acc2 ;

    Q15 emInt16   Imr[SubFrLen]  ;
    Q15 emInt16   OccPos[SubFrLen] ;
    Q15 emInt16   ImrCorr[SubFrLen] ;
    Q31 emInt32   ErrBlk[SubFrLen] ;
    Q31 emInt32   WrkBlk[SubFrLen] ;

	LOG_StackAddr(__FUNCTION__);


    /* Update Impulse response */
    if ( Olp < (Q15 emInt16) (SubFrLen-2) ) {
        Temp.UseTrn = (Q15 emInt16) 1 ;
        Gen_Trn( Imr, ImpResp, Olp ) ;
    }
    else {
        Temp.UseTrn = (Q15 emInt16) 0 ;
        for ( i = 0 ; i < SubFrLen ; i ++ )
            Imr[i] = ImpResp[i] ;
    }

    /* Scale Imr to avoid overflow */
    for ( i = 0 ; i < SubFrLen ; i ++ )
        OccPos[i] = shr_1( Imr[i], (Q15 emInt16) 1 ) ;

    /* Compute Imr AutoCorr function */
    Acc0 = (Q31 emInt32) 0 ;
    for ( i = 0 ; i < SubFrLen ; i ++ )
        Acc0 = L_mac_1( Acc0, OccPos[i], OccPos[i] ) ;

    Exp = norm_l( Acc0 ) ;
    Acc0 = L_shl_1( Acc0, Exp ) ;
    ImrCorr[0] = round_1( Acc0 ) ;

    /* Compute all the other */
    for ( i = 1 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Q31 emInt32) 0 ;
        for ( j = i ; j < SubFrLen ; j ++ )
            Acc0 = L_mac_1( Acc0, OccPos[j], OccPos[j-i] ) ;
        Acc0 = L_shl_1( Acc0, Exp ) ;
        ImrCorr[i] = round_1( Acc0 ) ;
    }

    /* Cross correlation with the signal */
    Exp = sub( Exp, 4 ) ;
    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Q31 emInt32) 0 ;
        for ( j = i ; j < SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Tv[j], Imr[j-i] ) ;
        ErrBlk[i] = L_shl_1( Acc0, Exp ) ;
    }

    /* Search for the best sequence */
    for ( k = 0 ; k < Sgrid ; k ++ ) {

        Temp.GridId = (Q15 emInt16) k ;

        /* Find maximum amplitude */
        Acc1 = (Q31 emInt32) 0 ;
        for ( i = k ; i < SubFrLen ; i += Sgrid ) {
            Acc0 = L_abs( ErrBlk[i] ) ;
            if ( Acc0 >= Acc1 ) {
                Acc1 = Acc0 ;
                Temp.Ploc[0] = (Q15 emInt16) i ;
            }
        }

        /* Quantize the maximum amplitude */
        Acc2 = Acc1 ;
        Acc1 = (Q31 emInt32) 0x40000000L ;
        MaxAmpId = (Q15 emInt16) (NumOfGainLev - MlqSteps) ;

        for ( i = MaxAmpId ; i >= MlqSteps ; i -- ) {
            Acc0 = L_mult( FcbkGainTable[i], ImrCorr[0] ) ;
            Acc0 = L_sub( Acc0, Acc2 ) ;
            Acc0 = L_abs( Acc0 ) ;
            if ( Acc0 < Acc1 ) {
                Acc1 = Acc0 ;
                MaxAmpId = (Q15 emInt16) i ;
            }
        }
        MaxAmpId -- ;

        for ( i = 1 ; i <=2*MlqSteps ; i ++ ) {

            for ( j = k ; j < SubFrLen ; j += Sgrid ) {
                WrkBlk[j] = ErrBlk[j] ;
                OccPos[j] = (Q15 emInt16) 0 ;
            }
            Temp.MampId = MaxAmpId - (Q15 emInt16) MlqSteps + (Q15 emInt16) i ;

            MaxAmp = FcbkGainTable[Temp.MampId] ;

            if ( WrkBlk[Temp.Ploc[0]] >= (Q31 emInt32) 0 )
                Temp.Pamp[0] = MaxAmp ;
            else
                Temp.Pamp[0] = negate(MaxAmp) ;

            OccPos[Temp.Ploc[0]] = (Q15 emInt16) 1 ;

            for ( j = 1 ; j < Np ; j ++ ) {

                Acc1 = (Q31 emInt32) 0xc0000000L ;

                for ( l = k ; l < SubFrLen ; l += Sgrid ) {

                    if ( OccPos[l] != (Q15 emInt16) 0 )
                        continue ;

                    Acc0 = WrkBlk[l] ;
                    Acc0 = L_msu( Acc0, Temp.Pamp[j-1],
                            ImrCorr[abs_s((Q15 emInt16)(l-Temp.Ploc[j-1]))] ) ;
                    WrkBlk[l] = Acc0 ;
                    Acc0 = L_abs( Acc0 ) ;
                    if ( Acc0 > Acc1 ) {
                        Acc1 = Acc0 ;
                        Temp.Ploc[j] = (Q15 emInt16) l ;
                    }
                }

                if ( WrkBlk[Temp.Ploc[j]] >= (Q31 emInt32) 0 )
                    Temp.Pamp[j] = MaxAmp ;
                else
                    Temp.Pamp[j] = negate(MaxAmp) ;

                OccPos[Temp.Ploc[j]] = (Q15 emInt16) 1 ;
            }

            /* Compute error vector */
            for ( j = 0 ; j < SubFrLen ; j ++ )
                OccPos[j] = (Q15 emInt16) 0 ;

            for ( j = 0 ; j < Np ; j ++ )
                OccPos[Temp.Ploc[j]] = Temp.Pamp[j] ;

            for ( l = SubFrLen-1 ; l >= 0 ; l -- ) {
                Acc0 = (Q31 emInt32) 0 ;
                for ( j = 0 ; j <= l ; j ++ )
                    Acc0 = L_mac( Acc0, OccPos[j], Imr[l-j] ) ;
                Acc0 = L_shl( Acc0, (Q15 emInt16) 2 ) ;
                OccPos[l] = extract_h( Acc0 ) ;
            }

            /* Evaluate error */
            Acc1 = (Q31 emInt32) 0 ;
            for ( j = 0 ; j < SubFrLen ; j ++ ) {
                Acc1 = L_mac( Acc1, Tv[j], OccPos[j] ) ;
                Acc0 = L_mult( OccPos[j], OccPos[j] ) ;
                Acc1 = L_sub( Acc1, L_shr( Acc0, (Q15 emInt16) 1 ) ) ;
            }

            if ( Acc1 > (*Best).MaxErr ) {
                (*Best).MaxErr = Acc1 ;
                (*Best).GridId = Temp.GridId ;
                (*Best).MampId = Temp.MampId ;
                (*Best).UseTrn = Temp.UseTrn ;
                for ( j = 0 ; j < Np ; j ++ ) {
                    (*Best).Pamp[j] = Temp.Pamp[j] ;
                    (*Best).Ploc[j] = Temp.Ploc[j] ;
                }
            }
        }
    }
    return;
}
/*
**
** Function:        Fcbk_Pack()
**
** √Ë ˆ:     Encoding of the pulse positions and gains for the high
**                  rate case.
**                  Combinatorial encoding is used to transmit the optimal
**                  combination of pulse locations.
**
** Links to text:   Section 2.15
**
** Arguments:
**
**  Q15 emInt16 *Dpnt    Excitation vector
**  SFSDEF *Sfs     Encoded parameters of the excitation model
**  BESTDEF *Best   ≤Œ ˝ of the best excitation model
**  Q15 emInt16 Np       Number of pulses (6 for even subframes; 5 for odd subframes)
**
** Outputs:
**
**  SFSDEF *Sfs     Encoded parameters of the excitation model
**
** ∑µªÿ value:    None
**
*/
void  Fcbk_Pack( Q15 emInt16 *Dpnt, SFSDEF *Sfs, BESTDEF *Best, Q15 emInt16 Np )
{
    emInt32    i,j   ;


	LOG_StackAddr(__FUNCTION__);

    /* Code the amplitudes and positions */
    j = MaxPulseNum - (emInt32 ) Np ;

    (*Sfs).Pamp = (Q15 emInt16) 0 ;
    (*Sfs).Ppos = (Q31 emInt32) 0 ;

    for ( i = 0 ; i < SubFrLen/Sgrid ; i ++ ) {

        if ( Dpnt[(emInt32 )(*Best).GridId + Sgrid*i] == (Q15 emInt16) 0 )
            (*Sfs).Ppos = L_add( (*Sfs).Ppos, CombinatorialTable[j][i] ) ;
        else {
            (*Sfs).Pamp = shl( (*Sfs).Pamp, (Q15 emInt16) 1 ) ;
            if ( Dpnt[(emInt32 )(*Best).GridId + Sgrid*i] < (Q15 emInt16) 0 )
                (*Sfs).Pamp = add( (*Sfs).Pamp, (Q15 emInt16) 1 ) ;

            j ++ ;
            /* Check for end */
            if ( j == MaxPulseNum )
                break ;
        }
    }

    (*Sfs).Mamp = (*Best).MampId ;
    (*Sfs).Grid = (*Best).GridId ;
    (*Sfs).Tran = (*Best).UseTrn ;

    return;
}


#endif /* EM_USER_ENCODER_G7231 */


#if EM_USER_DECODER_G7231


/*
**
** Function:        Fcbk_Unpk()
**
** √Ë ˆ:     Decoding of the fixed codebook excitation for both rates.
**                  Gains, pulse positions, grid position (odd or even), signs
**                  are decoded and used to reconstruct the excitation.
**
** Links to text:   Section 2.17 & 3.5
**
** Arguments:
**
**  Q15 emInt16 *Tv      Decoded excitation vector
**  SFSDEF Sfs      Encoded parameters of the excitation (for one subframe)
**  Q15 emInt16 Olp      Closed loop adaptive pitch lag
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:
**
**  Q15 emInt16 *Tv      Decoded excitation vector
**
** ∑µªÿ value:    None
**
*/
void  Fcbk_Unpk( NEED_THIS Q15 emInt16 *Tv, SFSDEF Sfs, Q15 emInt16 Olp, Q15 emInt16 Sfc )
{
    emInt32    i,j   ;

    Q31 emInt32   Acc0  ;
    Q15 emInt16   Np ;
    /*Q15 emInt16 Tv_tmp[SubFrLen+4];*/
    Q15 emInt16 acelp_gain, acelp_sign, acelp_shift, acelp_pos;
    Q15 emInt16 offset, ipos, T0_acelp, gain_T0;


	LOG_StackAddr(__FUNCTION__);


    switch(WrkRate)  {
        case Rate63: {

            Np = Nb_puls[(emInt32 )Sfc] ;

            for ( i = 0 ; i < SubFrLen ; i ++ )
                Tv[i] = (Q15 emInt16) 0 ;

            if ( Sfs.Ppos >= MaxPosTable[Sfc] )
                return ;

            /* Decode the amplitudes and positions */
            j = MaxPulseNum - (emInt32 ) Np ;

            Acc0 = Sfs.Ppos ;

            for ( i = 0 ; i < SubFrLen/Sgrid ; i ++ )  {

                Acc0 = L_sub( Acc0, CombinatorialTable[j][i] ) ;

                if ( Acc0 < (Q31 emInt32) 0 ) {
                    Acc0 = L_add( Acc0, CombinatorialTable[j][i] ) ;
                    j ++ ;
                    if ( (Sfs.Pamp & (1 << (MaxPulseNum-j) )) != (Q15 emInt16) 0 )
                        Tv[(emInt32 )Sfs.Grid + Sgrid*i] = -FcbkGainTable[Sfs.Mamp] ;
                    else
                        Tv[(emInt32 )Sfs.Grid + Sgrid*i] =  FcbkGainTable[Sfs.Mamp] ;

                    if ( j == MaxPulseNum )
                        break ;
                }
            }

            if ( Sfs.Tran == (Q15 emInt16) 1 )
                Gen_Trn( Tv, Tv, Olp ) ;
            break;
        }

        case Rate53: {
#if 0
            for ( i = 0 ; i < SubFrLen+4 ; i ++ )
                Tv_tmp[i] = (Q15 emInt16) 0 ;

            /* decoding gain */
            acelp_gain = FcbkGainTable[Sfs.Mamp];
            /* decoding grid */
            acelp_shift = Sfs.Grid;
            /* decoding Sign */
            acelp_sign = Sfs.Pamp;
            /* decoding Pos */
            acelp_pos = (emInt16) Sfs.Ppos;

            offset  = 0;
            for(i=0; i<4; i++) {
                ipos = (acelp_pos & (Q15 emInt16)0x0007) ;
                ipos = shl(ipos,3) + acelp_shift + offset;
                if( (acelp_sign & 1 )== 1) {
                    Tv_tmp[ipos] = acelp_gain;
                }
                else {
                    Tv_tmp[ipos] = -acelp_gain;
                }
                offset = add(offset,2);
                acelp_pos = shr(acelp_pos, 3);
                acelp_sign = shr(acelp_sign,1);
            }
            for (i = 0; i < SubFrLen; i++) Tv[i] = Tv_tmp[i];
            T0_acelp = search_T0( (Q15 emInt16) (Olp-1+Sfs.AcLg), Sfs.AcGn,
                                                            &gain_T0);
            if(T0_acelp <SubFrLen-2) {
                /* code[i] += 0.8 * code[i-Olp] */
                for (i = T0_acelp ; i < SubFrLen; i++)
                    Tv[i] = add(Tv[i], mult_1(Tv[i-T0_acelp ], gain_T0));
            }

            break;
#else		
            for ( i = 0 ; i < SubFrLen ; i ++ )
                Tv[i] = (Q15 emInt16) 0 ;

            /* decoding gain */
            acelp_gain = FcbkGainTable[Sfs.Mamp];
            /* decoding grid */
            acelp_shift = Sfs.Grid;
            /* decoding Sign */
            acelp_sign = Sfs.Pamp;
            /* decoding Pos */
            acelp_pos = (emInt16) Sfs.Ppos;
			
            offset  = 0;
            for(i=0; i<4; i++) 
			{
                ipos = (acelp_pos & (Q15 emInt16)0x0007) ;
                ipos = shl(ipos,(Q15 emInt16)3) + acelp_shift + offset;
				if(ipos < 60)
				{
					Tv[ipos] = ((acelp_sign & 1 )== 1)?acelp_gain:-acelp_gain;
				}
                offset = offset+2;
                acelp_pos = shr(acelp_pos, 3);
                acelp_sign = shr(acelp_sign,1);
            }
			/*search_T0*/
			T0_acelp = (Q15 emInt16) (Olp-1+Sfs.AcLg) + epsi170[Sfs.AcGn] ;
            if(T0_acelp <SubFrLen-2) 
			{
				gain_T0 = gain170[Sfs.AcGn];
                /* code[i] += 0.8 * code[i-*POlp] */
                for (i = T0_acelp ; i < SubFrLen; i++)
                    Tv[i] = add(Tv[i], mult(Tv[i-T0_acelp ], gain_T0));
            }
#endif
        }
    }
    return;
}


#endif /* EM_USER_DECODER_G7231 */


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Find_Acbk()
**
** √Ë ˆ:     Computation of adaptive codebook contribution in
**                  closed-loop around the open-loop pitch lag (subframes 0 & 2)
**                  around the previous subframe closed-loop pitch lag
**                  (subframes 1 & 3).  For subframes 0 & 2, the pitch lag is
**                  encoded whereas for subframes 1 & 3, only the difference
**                  with the previous value is encoded (-1, 0, +1 or +2).
**                  The pitch predictor gains are quantized using one of the two
**                  codebooks (85 entries or 170 entries) depending on the
**                  rate and on the pitch lag value.
**                  Finally, the contribution of the pitch predictor is decoded
**                  and subtracted to obtain the residual signal.
**
** Links to text:   Section 2.14
**
** Arguments:
**
**  Q15 emInt16 *Tv      Target vector
**  Q15 emInt16 *ImpResp Impulse response of the combined filter
**  Q15 emInt16 *PrevExc Previous excitation vector
**  LINEDEF *Line   Contains pitch related parameters (open/closed loop lag, gain)
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:
**
**  Q15 emInt16 *Tv     Residual vector
**  LINEDEF *Line  Contains pitch related parameters (closed loop lag, gain)
**
** ∑µªÿ value:    None
**
*/
void  Find_Acbk( NEED_THIS Q15 emInt16 *Tv, Q15 emInt16 *ImpResp, Q15 emInt16 *PrevExc,
				 LINEDEF *Line, Q15 emInt16 Sfc )
{
    emInt32    i,j,k,l  ;

    Q31 emInt32   Acc0,Acc1 ;

    Q15 emInt16   RezBuf[SubFrLen+ClPitchOrd-1] ;
    Q15 emInt16   FltBuf[ClPitchOrd][SubFrLen] ;
    Q31 emInt32   CorBuf[4*(2*ClPitchOrd + ClPitchOrd*(ClPitchOrd-1)/2)] ;
    Q31 emInt32   *lPnt ;

    Q15 emInt16   CorVct[4*(2*ClPitchOrd + ClPitchOrd*(ClPitchOrd-1)/2)] ;
    Q15 emInt16   *sPnt ;

    Q15 emInt16   Olp ;
    Q15 emInt16   Lid ;
    Q15 emInt16   Gid ;
    Q15 emInt16   Hb  ;
    Q15 emInt16   Exp ;
    Q15 emInt16   Bound[2] ;

    Q15 emInt16   Lag1, Lag2;
    Q15 emInt16   off_filt;

	LOG_StackAddr(__FUNCTION__);

    /* Init constants */
    Olp = (*Line).Olp[shr(Sfc, (Q15 emInt16) 1)] ;
    Lid = (Q15 emInt16) Pstep ;
    Gid = (Q15 emInt16) 0 ;
    Hb  = (Q15 emInt16) 3 + (Sfc & (Q15 emInt16) 1 ) ;

    /* For even frames only */
    if ( (Sfc & (Q15 emInt16)1) == (Q15 emInt16) 0 ) {
        if ( Olp == (Q15 emInt16) PitchMin )
            Olp = add( Olp, (Q15 emInt16) 1 ) ;
        if ( Olp > (Q15 emInt16) (PitchMax-5) )
            Olp = (Q15 emInt16)(PitchMax-5) ;
    }

    lPnt = CorBuf ;
    for ( k = 0 ; k < (emInt32 ) Hb ; k ++ ) {

        /* Get residual from the excitation buffer */
        Get_Rez( RezBuf, PrevExc, (Q15 emInt16)(Olp-(Q15 emInt16)Pstep+k) ) ;

        /* Filter the last one using the impulse response */
        for ( i = 0 ; i < SubFrLen ; i ++ ) {
            Acc0 = (Q31 emInt32) 0 ;
            for ( j = 0 ; j <= i ; j ++ )
                Acc0 = L_mac( Acc0, RezBuf[ClPitchOrd-1+j], ImpResp[i-j] ) ;
            FltBuf[ClPitchOrd-1][i] = round( Acc0 ) ;
        }

        /* Update all the others */
        for ( i = ClPitchOrd-2 ; i >= 0 ; i -- ) {
            FltBuf[i][0] = mult_r_1( RezBuf[i], (Q15 emInt16) 0x2000 ) ;
            for ( j = 1 ; j < SubFrLen ; j ++ ) {
                Acc0 = L_deposit_h( FltBuf[i+1][j-1] ) ;
                Acc0 = L_mac( Acc0, RezBuf[i], ImpResp[j] ) ;
                FltBuf[i][j] = round( Acc0 ) ;
            }
        }

        /* Compute the cross with the signal */
        for ( i = 0 ; i < ClPitchOrd ; i ++ ) {
            Acc1 = (Q31 emInt32) 0 ;
            for ( j = 0 ; j < SubFrLen ; j ++ ) {
                Acc0 = L_mult( Tv[j], FltBuf[i][j] ) ;
                Acc1 = L_add( Acc1, L_shr_1( Acc0, (Q15 emInt16) 1 ) ) ;
            }
            *lPnt ++ = L_shl( Acc1, (Q15 emInt16) 1 ) ;
        }

        /* Compute the energies */
        for ( i = 0 ; i < ClPitchOrd ; i ++ ) {
            Acc1 = (Q31 emInt32) 0 ;
            for ( j = 0 ; j < SubFrLen ; j ++ )
                Acc1 = L_mac( Acc1, FltBuf[i][j], FltBuf[i][j] ) ;
            *lPnt ++ = Acc1 ;
        }

        /* Compute the between crosses */
        for ( i = 1 ; i < ClPitchOrd ; i ++ ) {
            for ( j = 0 ; j < i ; j ++ ) {
                Acc1 = (Q31 emInt32) 0 ;
                for ( l = 0 ; l < SubFrLen ; l ++ ) {
                    Acc0 = L_mult( FltBuf[i][l], FltBuf[j][l] ) ;
                    Acc1 = L_add( Acc1, L_shr_1( Acc0, (Q15 emInt16) 1 ) ) ;
                }
                *lPnt ++ = L_shl( Acc1, (Q15 emInt16) 2 ) ;
            }
        }
    }


    /* Find Max and normalize */
    Acc1 = (Q31 emInt32) 0 ;
    for ( i = 0 ; i < Hb*20 ; i ++ ) {
        Acc0 = L_abs(CorBuf[i]) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    Exp = norm_l( Acc1 ) ;
    /* Convert to shorts */
    for ( i = 0 ; i < Hb*20 ; i ++ ) {
        Acc0 = L_shl_1( CorBuf[i], Exp ) ;
        CorVct[i] = round_1( Acc0 ) ;
    }

    /* Test potential error */
    Lag1 = Olp-(Q15 emInt16)Pstep;
    Lag2 = Olp-(Q15 emInt16)Pstep+ Hb -(Q15 emInt16)1;
    off_filt = Test_Err(USE_THIS Lag1, Lag2);
    Bound[0] =  NbFilt085_min + shl(off_filt,2);
    if(Bound[0] > NbFilt085) Bound[0] = NbFilt085;
    Bound[1] =  NbFilt170_min + shl(off_filt,3);
    if(Bound[1] > NbFilt170) Bound[1] = NbFilt170;

    /* Init the search loop */
    Acc1 = (Q31 emInt32) 0 ;

    for ( k = 0 ; k < (emInt32 ) Hb ; k ++ ) {

        /* Select Quantization tables */
        l = 0 ;
        if ( WrkRate == Rate63 ){
            if ( (Sfc & (Q15 emInt16) 1) == (Q15 emInt16) 0 ) {
                if ( (emInt32 )Olp-Pstep+k >= SubFrLen-2 )l ++ ;
            }
            else {
                if ( (emInt32 )Olp >= SubFrLen-2 ) l ++ ;
            }
        }
        else {
            l = 1;
        }


        sPnt = (Q15 emInt16*)AcbkGainTablePtr[l] ;

        for ( i = 0 ; i < (emInt32 ) Bound[l] ; i ++ ) {

            Acc0 = (Q31 emInt32) 0 ;
            for ( j = 0 ; j < 20 ; j ++ )
                Acc0 = L_add_1( Acc0, L_shr_1( L_mult(CorVct[k*20+j], *sPnt ++),
(Q15 emInt16) 1 ) ) ;

            if ( Acc0 > Acc1 ) {
                Acc1 = Acc0 ;
                Gid = (Q15 emInt16) i ;
                Lid = (Q15 emInt16) k ;
            }
        }
    }

    /* Modify Olp for even sub frames */
    if ( (Sfc & (Q15 emInt16) 1 ) == (Q15 emInt16) 0 ) {
        Olp = Olp - (Q15 emInt16) Pstep + Lid ;
        Lid = (Q15 emInt16) Pstep ;
    }

    /* Save Gains and Olp */
    (*Line).Sfs[Sfc].AcLg = Lid ;
    (*Line).Sfs[Sfc].AcGn = Gid ;
    (*Line).Olp[shr(Sfc, (Q15 emInt16) 1)] = Olp ;

    /* Decode the Acbk contribution and subtract it */
    Decod_Acbk( USE_THIS RezBuf, PrevExc, Olp, Lid, Gid ) ;

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_deposit_h( Tv[i] ) ;
        Acc0 = L_shr_1( Acc0, (Q15 emInt16) 1 ) ;

        for ( j = 0 ; j <= i ; j ++ )
            Acc0 = L_msu( Acc0, RezBuf[j], ImpResp[i-j] ) ;
        Acc0 = L_shl( Acc0, (Q15 emInt16) 1 ) ;
        Tv[i] = round( Acc0 ) ;
    }

    return;
}


#endif /* EM_USER_ENCODER_G7231 */


/*
**
** Function:        Get_Rez()
**
** √Ë ˆ:     Gets delayed contribution from the previous excitation
**                  vector.
**
** Links to text:   Sections 2.14, 2.18 & 3.4
**
** Arguments:
**
**  Q15 emInt16 *Tv      delayed excitation
**  Q15 emInt16 *PrevExc Previous excitation vector
**  Q15 emInt16 Lag      Closed loop pitch lag
**
** Outputs:
**
**  Q15 emInt16 *Tv      delayed excitation
**
** ∑µªÿ value:    None
**
*/
void  Get_Rez( Q15 emInt16 *Tv, Q15 emInt16 *PrevExc, Q15 emInt16 Lag )
{
    emInt32    i  ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < ClPitchOrd/2 ; i ++ )
        Tv[i] = PrevExc[PitchMax - (emInt32 ) Lag - ClPitchOrd/2 + i] ;

    for ( i = 0 ; i < SubFrLen+ClPitchOrd/2 ; i ++ )
        Tv[ClPitchOrd/2+i] = PrevExc[PitchMax - (emInt32 )Lag + i%(emInt32 )Lag] ;

    return;
}


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Comp_Info()
**
** √Ë ˆ:     Voiced/unvoiced classifier.
**                  It is based on a cross correlation maximization over the
**                  last 120 samples of the frame and with an index varying
**                  around the decoded pitch lag (from L-3 to L+3). Then the
**                  prediction gain is tested to declare the frame voiced or
**                  unvoiced.
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  Q15 emInt16 *Buff  decoded excitation
**  Q15 emInt16 Olp    Decoded pitch lag
**
** Outputs: None
**
** ∑µªÿ value:
**
**      Q15 emInt16   Estimated pitch value
*/
Q15 emInt16   Comp_Info( Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 *Gain, Q15 emInt16 *ShGain)
{
    emInt32    i,j   ;

    Q31 emInt32   Acc0,Acc1 ;

    Q15 emInt16   Tenr ;
    Q15 emInt16   Ccr,Enr ;
    Q15 emInt16   Indx ;

	LOG_StackAddr(__FUNCTION__);

    /* Normalize the excitation */
    *ShGain = Vec_Norm( Buff, (Q15 emInt16) (PitchMax+Frame) ) ;

    if ( Olp > (Q15 emInt16) (PitchMax-3) )
        Olp = (Q15 emInt16) (PitchMax-3) ;

    Indx = Olp ;

    Acc1 = (Q31 emInt32) 0 ;

    for ( i = (emInt32 )Olp-3 ; i <= (emInt32 )Olp+3 ; i ++ ) {

        Acc0 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < 2*SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Buff[PitchMax+Frame-2*SubFrLen+j],
                                    Buff[PitchMax+Frame-2*SubFrLen-i+j] ) ;

        if ( Acc0 > Acc1 ) {
            Acc1 = Acc0 ;
            Indx = (Q15 emInt16) i ;
        }
    }

    /* Compute target energy */
    Acc0 = (Q31 emInt32) 0 ;
    for ( j = 0 ; j < 2*SubFrLen ; j ++ )
        Acc0 = L_mac( Acc0, Buff[PitchMax+Frame-2*SubFrLen+j],
                                    Buff[PitchMax+Frame-2*SubFrLen+j] ) ;
    Tenr = round( Acc0 ) ;
    *Gain = Tenr;

    /* Compute best energy */
    Acc0 = (Q31 emInt32) 0 ;
    for ( j = 0 ; j < 2*SubFrLen ; j ++ )
        Acc0 = L_mac( Acc0, Buff[PitchMax+Frame-2*SubFrLen-(emInt32 )Indx+j],
                            Buff[PitchMax+Frame-2*SubFrLen-(emInt32 )Indx+j] ) ;

    Ccr = round( Acc1 ) ;

    if ( Ccr <= (Q15 emInt16) 0 )
        return (Q15 emInt16) 0 ;

    Enr = round( Acc0 ) ;

    Acc0 = L_mult( Enr, Tenr ) ;
    Acc0 = L_shr( Acc0, (Q15 emInt16) 3 ) ;

    Acc0 = L_msu( Acc0, Ccr, Ccr ) ;

    if ( Acc0 < (Q31 emInt32) 0 )
        return Indx ;
    else
        return (Q15 emInt16) 0 ;
}

/*
**
** Function:        Regen()
**
** √Ë ˆ:     Performs residual interpolation depending of the frame
**                  classification.
**                  If the frame is previously declared unvoiced, the excitation
**                  is regenerated using a random number generator. Otherwise
**                  a periodic excitation is generated with the period
**                  previously found.
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  Q15 emInt16 *DataBuff  current subframe decoded excitation
**  Q15 emInt16 *Buff     past decoded excitation
**  Q15 emInt16 Lag       Decoded pitch lag from previous frame
**  Q15 emInt16 Gain      Interpolated gain from previous frames
**  Q15 emInt16 Ecount    Number of erased frames
**  Q15 emInt16 *Sd       Random number used in unvoiced cases
**
** Outputs:
**
**  Q15 emInt16 *DataBuff current subframe decoded excitation
**  Q15 emInt16 *Buff     updated past excitation
**
** ∑µªÿ value:    None
**
*/
void     Regen( Q15 emInt16 *DataBuff, Q15 emInt16 *Buff, Q15 emInt16 Lag, Q15 emInt16 Gain,
Q15 emInt16 Ecount, Q15 emInt16 *Sd )
{
    emInt32    i  ;

	LOG_StackAddr(__FUNCTION__);

    /* Test for clearing */
    if ( Ecount >= (Q15 emInt16) ErrMaxNum ) {
        for ( i = 0 ; i < Frame ; i ++ )
            DataBuff[i] = (Q15 emInt16) 0 ;
        for ( i = 0 ; i < Frame+PitchMax ; i ++ )
            Buff[i] = (Q15 emInt16) 0 ;
    }
    else {
        /* Interpolate accordingly to the voicing estimation */
        if ( Lag != (Q15 emInt16) 0 ) {
            /* Voiced case */
            for ( i = 0 ; i < Frame ; i ++ )
                Buff[PitchMax+i] = Buff[PitchMax-(emInt32 )Lag+i] ;
            for ( i = 0 ; i < Frame ; i ++ )
                DataBuff[i] = Buff[PitchMax+i] = mult( Buff[PitchMax+i],
                            (Q15 emInt16) 0x6000 ) ;
        }
        else {
            /* Unvoiced case */
            for ( i = 0 ; i < Frame ; i ++ )
                DataBuff[i] = mult( Gain, Rand_lbc( Sd ) ) ;
            /* Clear buffer to reset memory */
            for ( i = 0 ; i < Frame+PitchMax ; i ++ )
                Buff[i] = (Q15 emInt16) 0 ;
        }
    }

    return;
}

/*
**
** Function:        Comp_Lpf()
**
** √Ë ˆ:     Computes pitch postfilter parameters.
**                  The pitch postfilter lag is first derived (Find_B
**                  and Find_F). Then, the one that gives the largest
**                  contribution is used to calculate the gains (Get_Ind).
**
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Q15 emInt16 *Buff    decoded excitation
**  Q15 emInt16 Olp      Decoded pitch lag
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:
**
**
** ∑µªÿ value:
**
**  PFDEF       Pitch postfilter parameters: PF.Gain    Pitch Postfilter gain
**                                           PF.ScGn    Pitch Postfilter scaling gain
**                                           PF.Indx    Pitch postfilter lag
*/
PFDEF Comp_Lpf( NEED_THIS Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 Sfc )
{
    emInt32    i,j   ;

    PFDEF Pf    ;
    Q31 emInt32   Lcr[5] ;
    Q15 emInt16   Scr[5] ;
    Q15 emInt16   Bindx, Findx ;
    Q15 emInt16   Exp ;

    Q31 emInt32   Acc0,Acc1 ;

	LOG_StackAddr(__FUNCTION__);

    /* Initialize */
    Pf.Indx = (Q15 emInt16) 0 ;
    Pf.Gain = (Q15 emInt16) 0 ;
    Pf.ScGn = (Q15 emInt16) 0x7fff ;

    /* Find both indices */
    Bindx = Find_B( Buff, Olp, Sfc ) ;
    Findx = Find_F( Buff, Olp, Sfc ) ;

    /* Combine the results */
    if ( (Bindx == (Q15 emInt16) 0) && (Findx == (Q15 emInt16) 0) )
        return Pf ;

    /* Compute target energy */
    Acc0 = (Q31 emInt32) 0 ;
    for ( j = 0 ; j < SubFrLen ; j ++ )
        Acc0 = L_mac( Acc0, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+j],
                                    Buff[PitchMax+(emInt32 )Sfc*SubFrLen+j] ) ;
    Lcr[0] = Acc0 ;

    if ( Bindx != (Q15 emInt16) 0 ) {
        Acc0 = (Q31 emInt32) 0 ;
        Acc1 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ ) {
            Acc0 = L_mac( Acc0, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+j],
                        Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Bindx+j] ) ;
            Acc1 = L_mac( Acc1, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Bindx+j],
                        Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Bindx+j] ) ;
        }
        Lcr[1] = Acc0 ;
        Lcr[2] = Acc1 ;
    }
    else {
        Lcr[1] = (Q31 emInt32) 0 ;
        Lcr[2] = (Q31 emInt32) 0 ;
    }

    if ( Findx != (Q15 emInt16) 0 ) {
        Acc0 = (Q31 emInt32) 0 ;
        Acc1 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ ) {
            Acc0 = L_mac( Acc0, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+j],
                        Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Findx+j] ) ;
            Acc1 = L_mac( Acc1, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Findx+j],
                        Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Findx+j] ) ;
        }
        Lcr[3] = Acc0 ;
        Lcr[4] = Acc1 ;
    }
    else {
        Lcr[3] = (Q31 emInt32) 0 ;
        Lcr[4] = (Q31 emInt32) 0 ;
    }

    /* Normalize and convert to shorts */
    Acc1 = 0L ;
    for ( i = 0 ; i < 5 ; i ++ ) {
        Acc0 = Lcr[i] ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    Exp = norm_l( Acc1 ) ;
    for ( i = 0 ; i < 5 ; i ++ ) {
        Acc0 = L_shl( Lcr[i], Exp ) ;
        Scr[i] = extract_h( Acc0 ) ;
    }

    /* Select the best pair */
    if ( (Bindx != (Q15 emInt16) 0) && ( Findx == (Q15 emInt16) 0) )
        Pf = Get_Ind( USE_THIS Bindx, Scr[0], Scr[1], Scr[2] ) ;

    if ( (Bindx == (Q15 emInt16) 0) && ( Findx != (Q15 emInt16) 0) )
        Pf = Get_Ind( USE_THIS Findx, Scr[0], Scr[3], Scr[4] ) ;

    if ( (Bindx != (Q15 emInt16) 0) && ( Findx != (Q15 emInt16) 0) ) {
        Exp = mult_r( Scr[1], Scr[1] ) ;
        Acc0 = L_mult( Exp, Scr[4] ) ;
        Exp = mult_r( Scr[3], Scr[3] ) ;
        Acc1 = L_mult( Exp, Scr[2] ) ;
        if ( Acc0 > Acc1 )
            Pf = Get_Ind( USE_THIS Bindx, Scr[0], Scr[1], Scr[2] ) ;
        else
            Pf = Get_Ind( USE_THIS Findx, Scr[0], Scr[3], Scr[4] ) ;
    }

    return Pf ;
}

/*
**
** Function:        Find_B()
**
** √Ë ˆ:     Computes best pitch postfilter backward lag by
**                  backward cross correlation maximization around the
**                  decoded pitch lag
**                  of the subframe 0 (for subframes 0 & 1)
**                  of the subframe 2 (for subframes 2 & 3)
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Q15 emInt16 *Buff    decoded excitation
**  Q15 emInt16 Olp      Decoded pitch lag
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:     None
**
** ∑µªÿ value:
**
**  Q15 emInt16   Pitch postfilter backward lag
*/
Q15 emInt16   Find_B( Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 Sfc )
{
    emInt32    i,j   ;

    Q15 emInt16   Indx = 0 ;

    Q31 emInt32   Acc0,Acc1 ;

	LOG_StackAddr(__FUNCTION__);

    if ( Olp > (Q15 emInt16) (PitchMax-3) )
        Olp = (Q15 emInt16) (PitchMax-3) ;

    Acc1 = (Q31 emInt32) 0 ;

    for ( i = (emInt32 )Olp-3 ; i <= (emInt32 )Olp+3 ; i ++ ) {

        Acc0 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < SubFrLen ; j ++ )
            Acc0 = L_mac( Acc0, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+j],
                                    Buff[PitchMax+(emInt32 )Sfc*SubFrLen-i+j] ) ;
        if ( Acc0 > Acc1 ) {
            Acc1 = Acc0 ;
            Indx = -(Q15 emInt16) i ;
        }
    }
    return Indx ;
}

/*
**
** Function:        Find_F()
**
** √Ë ˆ:     Computes best pitch postfilter forward lag by
**                  forward cross correlation maximization around the
**                  decoded pitch lag
**                  of the subframe 0 (for subframes 0 & 1)
**                  of the subframe 2 (for subframes 2 & 3)
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Q15 emInt16 *Buff    decoded excitation
**  Q15 emInt16 Olp      Decoded pitch lag
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:     None
**
** ∑µªÿ value:
**
**  Q15 emInt16    Pitch postfilter forward lag
*/
Q15 emInt16   Find_F( Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 Sfc )
{
    emInt32    i,j   ;

    Q15 emInt16   Indx = 0 ;

    Q31 emInt32   Acc0,Acc1 ;

	LOG_StackAddr(__FUNCTION__);

    if ( Olp > (Q15 emInt16) (PitchMax-3) )
        Olp = (Q15 emInt16) (PitchMax-3) ;

    Acc1 = (Q31 emInt32) 0 ;

    for ( i = Olp-3 ; i <= Olp+3 ; i ++ ) {

        Acc0 = (Q31 emInt32) 0 ;
        if ( ((emInt32 )Sfc*SubFrLen+SubFrLen+i) <= Frame ) {
            for ( j = 0 ; j < SubFrLen ; j ++ )
                Acc0 = L_mac( Acc0, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+j],
                            Buff[PitchMax+(emInt32 )Sfc*SubFrLen+i+j] ) ;
        }


        if ( Acc0 > Acc1 ) {
            Acc1 = Acc0 ;
            Indx = (Q15 emInt16) i ;
        }
    }

    return Indx ;
}

/*
**
** Function:        Get_Ind()
**
** √Ë ˆ:     Computes gains of the pitch postfilter.
**                  The gains are calculated using the cross correlation
**                  (forward or backward, the one with the greatest contribution)
**                  and the energy of the signal. Also, a test is performed on
**                  the prediction gain to see whether the pitch postfilter
**                  should be used or not.
**
**
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Q15 emInt16 Ind      Pitch postfilter lag
**  Q15 emInt16 Ten      energy of the current subframe excitation vector
**  Q15 emInt16 Ccr      Crosscorrelation of the excitation
**  Q15 emInt16 Enr      Energy of the (backward or forward) "delayed" excitation
**
** Outputs:     None
**
** ∑µªÿ value:
**
**  PFDEF
**         Q15 emInt16   Indx    Pitch postfilter lag
**         Q15 emInt16   Gain    Pitch postfilter gain
**         Q15 emInt16   ScGn    Pitch postfilter scaling gain
**
*/
PFDEF Get_Ind( NEED_THIS Q15 emInt16 Ind, Q15 emInt16 Ten, Q15 emInt16 Ccr, Q15 emInt16 Enr )
{
    Q31 emInt32   Acc0,Acc1 ;
    Q15 emInt16   Exp   ;

    PFDEF Pf ;

	LOG_StackAddr(__FUNCTION__);


    Pf.Indx = Ind ;

    /* Check valid gain */
    Acc0 = L_mult( Ten, Enr ) ;
    Acc0 = L_shr( Acc0, (Q15 emInt16) 2 ) ;
    Acc1 = L_mult( Ccr, Ccr ) ;

    if ( Acc1 > Acc0 ) {

        if ( Ccr >= Enr )
            Pf.Gain = LpfConstTable[(emInt32 )WrkRate] ;
        else {
            Pf.Gain = div_s( Ccr, Enr ) ;
            Pf.Gain = mult( Pf.Gain, LpfConstTable[(emInt32 )WrkRate] ) ;
        }
        /* Compute scaling gain */
        Acc0 = L_deposit_h( Ten ) ;
        Acc0 = L_shr( Acc0, (Q15 emInt16) 1 ) ;
        Acc0 = L_mac( Acc0, Ccr, Pf.Gain ) ;
        Exp  = mult( Pf.Gain, Pf.Gain ) ;
        Acc1 = L_mult( Enr, Exp ) ;
        Acc1 = L_shr( Acc1, (Q15 emInt16) 1 ) ;
        Acc0 = L_add( Acc0, Acc1 ) ;
        Exp = round( Acc0 ) ;

        Acc1 = L_deposit_h( Ten ) ;
        Acc0 = L_deposit_h( Exp ) ;
        Acc1 = L_shr( Acc1, (Q15 emInt16) 1 ) ;

        if ( Acc1 >= Acc0 )
            Exp = (Q15 emInt16) 0x7fff ;
        else
            Exp = div_l( Acc1, Exp ) ;

        Acc0 = L_deposit_h( Exp ) ;
        Pf.ScGn = Sqrt_lbc( Acc0 ) ;
    }
    else {
        Pf.Gain = (Q15 emInt16) 0 ;
        Pf.ScGn = (Q15 emInt16) 0x7fff ;
    }

    Pf.Gain = mult( Pf.Gain, Pf.ScGn ) ;

    return Pf ;
}

/*
**
** Function:        Filt_Lpf()
**
** √Ë ˆ:     Applies the pitch postfilter for each subframe.
**
** Links to text:   Section 3.6
**
** Arguments:
**
**  Q15 emInt16 *Tv      Pitch postfiltered excitation
**  Q15 emInt16 *Buff    decoded excitation
**  PFDEF Pf        Pitch postfilter parameters
**  Q15 emInt16 Sfc      Subframe index
**
** Outputs:
**
**  Q15 emInt16 *Tv      Pitch postfiltered excitation
**
** ∑µªÿ value: None
**
*/
void  Filt_Lpf( Q15 emInt16 *Tv, Q15 emInt16 *Buff, PFDEF Pf, Q15 emInt16 Sfc )
{
    emInt32    i  ;

    Q31 emInt32   Acc0 ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = L_mult( Buff[PitchMax+(emInt32 )Sfc*SubFrLen+i], Pf.ScGn ) ;
        Acc0 = L_mac( Acc0, Buff[PitchMax+(emInt32 )Sfc*SubFrLen+(emInt32 )Pf.Indx+i],
                                                                Pf.Gain ) ;
        Tv[(emInt32 )Sfc*SubFrLen+i] = round( Acc0 ) ;
    }

    return;
}

/*
**
** Function:        ACELP_LBC_code()
**
** √Ë ˆ:     Find Algebraic codebook for low bit rate LBC encoder
**
** Links to text:   Section 2.16
**
** Arguments:
**
**   Q15 emInt16 X[]              Target vector.     (in Q0)
**   Q15 emInt16 h[]              Impulse response.  (in Q12)
**   Q15 emInt16 T0               Pitch period.
**   Q15 emInt16 code[]           Innovative vector.        (in Q12)
**   Q15 emInt16 gain             Innovative vector gain.   (in Q0)
**   Q15 emInt16 sign             Signs of the 4 pulses.
**   Q15 emInt16 shift            Shift of the innovative vector
**   Q15 emInt16 gain_T0          Gain for pitch synchronous fiter
**
** Inputs :
**
**   Q15 emInt16 X[]              Target vector.     (in Q0)
**   Q15 emInt16 h[]              Impulse response.  (in Q12)
**   Q15 emInt16 T0               Pitch period.
**   Q15 emInt16 gain_T0          Gain for pitch synchronous fiter
**
** Outputs:
**
**   Q15 emInt16 code[]           Innovative vector.        (in Q12)
**   Q15 emInt16 gain             Innovative vector gain.   (in Q0)
**   Q15 emInt16 sign             Signs of the 4 pulses.
**   Q15 emInt16 shift            Shift of the innovative vector.
**
** ∑µªÿ value:
**
**   Q15 emInt16 index            Innovative codebook index
**
*/
Q15 emInt16  ACELP_LBC_code(Q15 emInt16 X[], Q15 emInt16 h[], Q15 emInt16 T0, Q15 emInt16 code[],
        Q15 emInt16 *ind_gain, Q15 emInt16 *shift, Q15 emInt16 *sign, Q15 emInt16 gain_T0)
{
    Q15 emInt16 i, index, gain_q;
    Q15 emInt16 Dn[SubFrLen2], tmp_code[SubFrLen2];
    Q15 emInt16 rr[DIM_RR];

	LOG_StackAddr(__FUNCTION__);

 /*
  * Include fixed-gain pitch contribution into impulse resp. h[]
  * Find correlations of h[] needed for the codebook search.
 */
    for (i = 0; i < SubFrLen; i++)    /* Q13 -->  Q12*/
        h[i] = shr_1(h[i], 1);

    if (T0 < SubFrLen-2) {
        for (i = T0; i < SubFrLen; i++)    /* h[i] += gain_T0*h[i-T0] */
        h[i] = add(h[i], mult_1(h[i-T0], gain_T0));
    }

    Cor_h(h, rr);

 /*
  * Compute correlation of target vector with impulse response.
  */

    Cor_h_X(h, X, Dn);

 /*
  * Find innovative codebook.
  * rr input matrix autocorrelation
  *    output filtered codeword
  */

    index = D4i64_LBC(Dn, rr, h, tmp_code, rr, shift, sign);

 /*
  * Compute innovation vector gain.
  * Include fixed-gain pitch contribution into code[].
  */

    *ind_gain = G_code(X, rr, &gain_q);

    for (i = 0; i < SubFrLen; i++)   {
        code[i] = i_mult(tmp_code[i], gain_q);
    }

    if(T0 < SubFrLen-2)
        for (i = T0; i < SubFrLen; i++)    /* code[i] += gain_T0*code[i-T0] */
            code[i] = add(code[i], mult_1(code[i-T0], gain_T0));


    return index;
}

/*
**
** Function:        Cor_h()
**
** √Ë ˆ:     Compute correlations of h[] needed for the codebook search.
**
** Links to text:   Section 2.16
**
** Arguments:
**
**  Q15 emInt16 h[]              Impulse response.
**  Q15 emInt16 rr[]             Correlations.
**
**  Outputs:
**
**  Q15 emInt16 rr[]             Correlations.
**
**  Return value :          None
*/
void Cor_h(Q15 emInt16 *H, Q15 emInt16 *rr)
{
    Q15 emInt16 *rri0i0, *rri1i1, *rri2i2, *rri3i3;
    Q15 emInt16 *rri0i1, *rri0i2, *rri0i3;
    Q15 emInt16 *rri1i2, *rri1i3, *rri2i3;

    Q15 emInt16 *p0, *p1, *p2, *p3;

    Q15 emInt16 *ptr_hd, *ptr_hf, *ptr_h1, *ptr_h2;
    Q31 emInt32 cor;
    Q15 emInt16 i, k, ldec, l_fin_sup, l_fin_inf;
    Q15 emInt16 h[SubFrLen2];

	LOG_StackAddr(__FUNCTION__);

    /* Scaling for maximum precision */

    cor = 0;
    for(i=0; i<SubFrLen; i++)
        cor = L_mac(cor, H[i], H[i]);

    if(extract_h(cor) > 32000 ) {
        for(i=0; i<SubFrLen; i++) h[i+4] = shr(H[i], 1);
    }
    else {
        k = norm_l(cor);
        k = shr(k, 1);
        for(i=0; i<SubFrLen; i++) h[i+4] = shl(H[i], k);
    }

    for(i=0; i<4; i++) h[i] = 0;

    /* Init pointers */

    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;

    rri0i1 = rri3i3 + NB_POS;
    rri0i2 = rri0i1 + MSIZE;
    rri0i3 = rri0i2 + MSIZE;
    rri1i2 = rri0i3 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri2i3 = rri1i3 + MSIZE;

 /*
  * Compute rri0i0[], rri1i1[], rri2i2[] and rri3i3[]
  */

    p0 = rri0i0 + NB_POS-1;   /* Init pointers to last position of rrixix[] */
    p1 = rri1i1 + NB_POS-1;
    p2 = rri2i2 + NB_POS-1;
    p3 = rri3i3 + NB_POS-1;

    ptr_h1 = h;
    cor    = 0;
    for(i=0;  i<NB_POS; i++) {
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p3-- = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p2-- = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p1-- = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        cor = L_mac(cor, *ptr_h1, *ptr_h1); ptr_h1++;
        *p0-- = extract_h(cor);
    }


 /*
  * Compute elements of: rri0i1[], rri0i3[], rri1i2[] and rri2i3[]
  */

    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Q15 emInt16)1;
    ldec = NB_POS+1;

    ptr_hd = h;
    ptr_hf = ptr_hd + 2;

    for(k=0; k<NB_POS; k++) {

        p3 = rri2i3 + l_fin_sup;
        p2 = rri1i2 + l_fin_sup;
        p1 = rri0i1 + l_fin_sup;
        p0 = rri0i3 + l_fin_inf;
        cor = 0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;

        for(i=k+(Q15 emInt16)1; i<NB_POS; i++ ) {

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p1 = extract_h(cor);

        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }

 /*
  * Compute elements of: rri0i2[], rri1i3[]
  */

    ptr_hd = h;
    ptr_hf = ptr_hd + 4;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Q15 emInt16)1;
    for(k=0; k<NB_POS; k++) {
        p3 = rri1i3 + l_fin_sup;
        p2 = rri0i2 + l_fin_sup;
        p1 = rri1i3 + l_fin_inf;
        p0 = rri0i2 + l_fin_inf;

        cor = 0;
        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        for(i=k+(Q15 emInt16)1; i<NB_POS; i++ ) {
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);

        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p2 = extract_h(cor);


        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }

 /*
  * Compute elements of: rri0i1[], rri0i3[], rri1i2[] and rri2i3[]
  */

    ptr_hd = h;
    ptr_hf = ptr_hd + 6;
    l_fin_sup = MSIZE-1;
    l_fin_inf = l_fin_sup-(Q15 emInt16)1;
    for(k=0; k<NB_POS; k++) {

        p3 = rri0i3 + l_fin_sup;
        p2 = rri2i3 + l_fin_inf;
        p1 = rri1i2 + l_fin_inf;
        p0 = rri0i1 + l_fin_inf;

        ptr_h1 = ptr_hd;
        ptr_h2 =  ptr_hf;
        cor = 0;
        for(i=k+(Q15 emInt16)1; i<NB_POS; i++ ) {

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p3 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p2 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p1 = extract_h(cor);

            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
            *p0 = extract_h(cor);

            p3 -= ldec;
            p2 -= ldec;
            p1 -= ldec;
            p0 -= ldec;
        }
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        cor = L_mac(cor, *ptr_h1, *ptr_h2); ptr_h1++; ptr_h2++;
        *p3 = extract_h(cor);

        l_fin_sup -= NB_POS;
        l_fin_inf--;
        ptr_hf += STEP;
    }

    return;
}

/*
**
**  Function:     Corr_h_X()
**
**  √Ë ˆ:    Compute  correlations of input response h[] with
**                  the target vector X[].
**
**  Links to the text: Section 2.16
**
** Arguments:
**
**      Q15 emInt16 h[]              Impulse response.
**      Q15 emInt16 X[]              Target vector.
**      Q15 emInt16 D[]              Correlations.
**
**  Outputs:
**
**      Q15 emInt16 D[]              Correlations.
**
**  Return value:           None
*/
void Cor_h_X(Q15 emInt16 h[], Q15 emInt16 X[], Q15 emInt16 D[])
{
    Q15 emInt16 i, j;
    Q31 emInt32 s, max;
    Q31 emInt32 y32[SubFrLen];

	LOG_StackAddr(__FUNCTION__);

    /* first keep the result on 32 bits and find absolute maximum */

    max = 0;

    for (i = 0; i < SubFrLen; i++) {
        s = 0;
        for (j = i; j <  SubFrLen; j++)
            s = L_mac(s, X[j], h[j-i]);

        y32[i] = s;

        s = L_abs(s);
        if(s > max) max = s;
    }

    /*
     * Find the number of right shifts to do on y32[]
     * so that maximum is on 13 bits
     */

    j = norm_l(max);
    if( sub(j,16) > 0) j = 16;

    j = sub(18, j);

    for(i=0; i<SubFrLen; i++)
        D[i] = extract_l( L_shr_1(y32[i], j) );

    return;
}

/*
** Function:            Reset_max_time()
**
**  √Ë ˆ:        This function should be called at the beginning
**                      of each frame.
**
**  Links to the text:  Section 2.16
**
**  Arguments:          None
**
**  Inputs:             None
**
**  Outputs:
**
**      Q15 emInt16          extra
**
**  Return value:           None
**
*/
emStatic Q15 emInt16 extra;

void reset_max_time(void)
{

	LOG_StackAddr(__FUNCTION__);

    extra = 120;
    return;
}

/*
**
**  Function:       D4i64_LBC
**
**  √Ë ˆ:       Algebraic codebook for LBC.
**                     -> 17 bits; 4 pulses in a frame of 60 samples
**
**                     The code length is 60, containing 4 nonzero pulses
**                     i0, i1, i2, i3. Each pulse can have 8 possible
**                     positions (positive or negative):
**
**                     i0 (+-1) : 0, 8,  16, 24, 32, 40, 48, 56
**                     i1 (+-1) : 2, 10, 18, 26, 34, 42, 50, 58
**                     i2 (+-1) : 4, 12, 20, 28, 36, 44, 52, (60)
**                     i3 (+-1) : 6, 14, 22, 30, 38, 46, 54, (62)
**
**                     All the pulse can be shifted by one.
**                     The last position of the last 2 pulses falls outside the
**                     frame and signifies that the pulse is not present.
**                     The threshold controls if a section of the innovative
**                     codebook should be searched or not.
**
**  Links to the text: Section 2.16
**
**  Input arguments:
**
**      Q15 emInt16 Dn[]       Correlation between target vector and impulse response h[]
**      Q15 emInt16 rr[]       Correlations of impulse response h[]
**      Q15 emInt16 h[]        Impulse response of filters
**
**  Output arguments:
**
**      Q15 emInt16 cod[]      Selected algebraic codeword
**      Q15 emInt16 y[]        Filtered codeword
**      Q15 emInt16 code_shift Shift of the codeword
**      Q15 emInt16 sign       Signs of the 4 pulses.
**
**  Return value:
**
**      Q15 emInt16   Index of selected codevector
**
*/
Q15 emInt16 D4i64_LBC(Q15 emInt16 Dn[], Q15 emInt16 rr[], Q15 emInt16 h[], Q15 emInt16 cod[],
                 Q15 emInt16 y[], Q15 emInt16 *code_shift, Q15 emInt16 *sign)
{
    Q15 emInt16  i0, i1, i2, i3, ip0, ip1, ip2, ip3;
    Q15 emInt16  i, j, time;
    Q15 emInt16  shif, shift;
    Q15 emInt16  ps0, ps1, ps2, ps3, alp, alp0;
    Q31 emInt32  alp1, alp2, alp3, L32;
    Q15 emInt16  ps0a, ps1a, ps2a;
    Q15 emInt16  ps3c, psc, alpha;
    Q15 emInt16  means, max0, max1, max2, thres;

    Q15 emInt16  *rri0i0, *rri1i1, *rri2i2, *rri3i3;
    Q15 emInt16  *rri0i1, *rri0i2, *rri0i3;
    Q15 emInt16  *rri1i2, *rri1i3, *rri2i3;

    Q15 emInt16  *ptr_ri0i0, *ptr_ri1i1, *ptr_ri2i2, *ptr_ri3i3;
    Q15 emInt16  *ptr_ri0i1, *ptr_ri0i2, *ptr_ri0i3;
    Q15 emInt16  *ptr_ri1i2, *ptr_ri1i3, *ptr_ri2i3;

    Q15 emInt16  *ptr1_ri0i1, *ptr1_ri0i2, *ptr1_ri0i3;
    Q15 emInt16  *ptr1_ri1i2, *ptr1_ri1i3, *ptr1_ri2i3;

    Q15 emInt16  p_sign[SubFrLen2/2];

	LOG_StackAddr(__FUNCTION__);

    /* Init pointers */

    rri0i0 = rr;
    rri1i1 = rri0i0 + NB_POS;
    rri2i2 = rri1i1 + NB_POS;
    rri3i3 = rri2i2 + NB_POS;

    rri0i1 = rri3i3 + NB_POS;
    rri0i2 = rri0i1 + MSIZE;
    rri0i3 = rri0i2 + MSIZE;
    rri1i2 = rri0i3 + MSIZE;
    rri1i3 = rri1i2 + MSIZE;
    rri2i3 = rri1i3 + MSIZE;

 /*
  * Extend the backward filtered target vector by zeros
  */

    for (i = SubFrLen; i < SubFrLen2; i++) Dn[i] = 0;

 /*
  * Chose the sign of the impulse.
  */

    for (i=0; i<SubFrLen; i+=2) {
        if( add(Dn[i],Dn[i+1]) >= 0) {
            p_sign[i/2] = 1;
        }
        else {
            p_sign[i/2] = -1;
            Dn[i] = -Dn[i];
            Dn[i+1] = -Dn[i+1];
        }
    }
    p_sign[30] = p_sign[31] = 1;

 /*
  *   Compute the search threshold after three pulses
  */

    /* odd positions */
    /* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

    max0 = Dn[0];
    max1 = Dn[2];
    max2 = Dn[4];
    for (i = 8; i < SubFrLen; i+=STEP) {
        if (Dn[i]   > max0) max0 = Dn[i];
        if (Dn[i+2] > max1) max1 = Dn[i+2];
        if (Dn[i+4] > max2) max2 = Dn[i+4];
    }
    max0 = add(max0, max1);
    max0 = add(max0, max2);

    /* Find means of Dn[i0]+Dn[i1]+Dn[i2] */

    L32 = 0;
    for (i = 0; i < SubFrLen; i+=STEP) {
        L32 = L_mac(L32, Dn[i], 1);
        L32 = L_mac(L32, Dn[i+2], 1);
        L32 = L_mac(L32, Dn[i+4], 1);
    }
    means =extract_l( L_shr_1(L32, 4));

    /* thres = means + (max0-means)*threshold; */

    thres = sub(max0, means);
    thres = mult(thres, threshold);
    thres = add(thres, means);

    /* even positions */
    /* Find maximum of Dn[i0]+Dn[i1]+Dn[i2] */

    max0 = Dn[1];
    max1 = Dn[3];
    max2 = Dn[5];
    for (i = 9; i < SubFrLen; i+=STEP) {
        if (Dn[i]   > max0) max0 = Dn[i];
        if (Dn[i+2] > max1) max1 = Dn[i+2];
        if (Dn[i+4] > max2) max2 = Dn[i+4];
    }
    max0 = add(max0, max1);
    max0 = add(max0, max2);

    /* Find means of Dn[i0]+Dn[i1]+Dn[i2] */

    L32 = 0;
    for (i = 1; i < SubFrLen; i+=STEP) {
        L32 = L_mac(L32, Dn[i], 1);
        L32 = L_mac(L32, Dn[i+2], 1);
        L32 = L_mac(L32, Dn[i+4], 1);
    }
    means =extract_l( L_shr_1(L32, 4));


    /* max1 = means + (max0-means)*threshold */

    max1 = sub(max0, means);
    max1 = mult(max1, threshold);
    max1 = add(max1, means);

    /* Keep maximum threshold between odd and even position */

    if(max1 > thres) thres = max1;

 /*
  * Modification of rrixiy[] to take signs into account.
  */

    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;
    ptr1_ri0i1 = rri0i1;
    ptr1_ri0i2 = rri0i2;
    ptr1_ri0i3 = rri0i3;

    for(i0=0; i0<SubFrLen/2; i0+=STEP/2) {
        for(i1=2/2; i1<SubFrLen/2; i1+=STEP/2) {
            *ptr_ri0i1++ = i_mult(*ptr1_ri0i1++,
                                    i_mult(p_sign[i0], p_sign[i1]));
            *ptr_ri0i2++ = i_mult(*ptr1_ri0i2++,
                                    i_mult(p_sign[i0], p_sign[i1+1]));
            *ptr_ri0i3++ = i_mult(*ptr1_ri0i3++,
                                    i_mult(p_sign[i0], p_sign[i1+2]));
        }
    }

    ptr_ri1i2 = rri1i2;
    ptr_ri1i3 = rri1i3;
    ptr1_ri1i2 = rri1i2;
    ptr1_ri1i3 = rri1i3;
    for(i1=2/2; i1<SubFrLen/2; i1+=STEP/2) {
        for(i2=4/2; i2<SubFrLen2/2; i2+=STEP/2) {
            *ptr_ri1i2++ = i_mult(*ptr1_ri1i2++,
                                    i_mult(p_sign[i1], p_sign[i2]));
            *ptr_ri1i3++ = i_mult(*ptr1_ri1i3++,
                                    i_mult(p_sign[i1], p_sign[i2+1]));

        }
    }

    ptr_ri2i3 = rri2i3;

    ptr1_ri2i3 = rri2i3;
    for(i2=4/2; i2<SubFrLen2/2; i2+=STEP/2) {
        for(i3=6/2; i3<SubFrLen2/2; i3+=STEP/2)
        *ptr_ri2i3++ = i_mult(*ptr1_ri2i3++, i_mult(p_sign[i2], p_sign[i3]));
    }

 /*
  * Search the optimum positions of the four  pulses which maximize
  *     square(correlation) / energy
  * The search is performed in four  nested loops. At each loop, one
  * pulse contribution is added to the correlation and energy.
  *
  * The fourth loop is entered only if the correlation due to the
  *  contribution of the first three pulses exceeds the preset
  *  threshold.
  */

    /* Default values */
    ip0    = 0;
    ip1    = 2;
    ip2    = 4;
    ip3    = 6;
    shif   = 0;
    psc    = 0;
    alpha  = 32767;
    time   = add_1(max_time, extra);



    /* Four loops to search innovation code. */

    /* Init. pointers that depend on first loop */
    ptr_ri0i0 = rri0i0;
    ptr_ri0i1 = rri0i1;
    ptr_ri0i2 = rri0i2;
    ptr_ri0i3 = rri0i3;

    /* first pulse loop  */
    for (i0 = 0; i0 < SubFrLen; i0 += STEP) {

        ps0  = Dn[i0];
        ps0a = Dn[i0+1];
        alp0 = *ptr_ri0i0++;

        /* Init. pointers that depend on second loop */
        ptr_ri1i1 = rri1i1;
        ptr_ri1i2 = rri1i2;
        ptr_ri1i3 = rri1i3;

        /* second pulse loop */
        for (i1 = 2; i1 < SubFrLen; i1 += STEP) {

            ps1  = add(ps0, Dn[i1]);
            ps1a = add(ps0a, Dn[i1+1]);

            /* alp1 = alp0 + *ptr_ri1i1++ + 2.0 * ( *ptr_ri0i1++); */

            alp1 = L_mult(alp0, 1);
            alp1 = L_mac(alp1, *ptr_ri1i1++, 1);
            alp1 = L_mac(alp1, *ptr_ri0i1++, 2);

            /* Init. pointers that depend on third loop */
            ptr_ri2i2 = rri2i2;
            ptr_ri2i3 = rri2i3;

            /* third pulse loop */
            for (i2 = 4; i2 < SubFrLen2; i2 += STEP) {

                ps2  = add(ps1, Dn[i2]);
                ps2a = add(ps1a, Dn[i2+1]);

                /* alp2 = alp1 + *ptr_ri2i2++
                               + 2.0 * (*ptr_ri0i2++ + *ptr_ri1i2++); */

                alp2 = L_mac(alp1, *ptr_ri2i2++, 1);
                alp2 = L_mac(alp2, *ptr_ri0i2++, 2);
                alp2 = L_mac(alp2, *ptr_ri1i2++, 2);

                /* Decide the shift */

                shift = 0;
                if(ps2a > ps2) {
                    shift = 1;
                    ps2   = ps2a;
                }

                /* Test threshold */

                if ( ps2 > thres) {

                    /* Init. pointers that depend on 4th loop */
                    ptr_ri3i3 = rri3i3;

                    /* 4th pulse loop */
                    for (i3 = 6; i3 < SubFrLen2; i3 += STEP) {

                        ps3 = add(ps2, Dn[i3+shift]);

                        /* alp3 = alp2 + (*ptr_ri3i3++) +
                                         2 x ( (*ptr_ri0i3++) +
                                               (*ptr_ri1i3++) +
                                               (*ptr_ri2i3++) ) */

                        alp3 = L_mac(alp2, *ptr_ri3i3++, 1);
                        alp3 = L_mac(alp3, *ptr_ri0i3++, 2);
                        alp3 = L_mac(alp3, *ptr_ri1i3++, 2);
                        alp3 = L_mac(alp3, *ptr_ri2i3++, 2);
                        alp  = extract_l(L_shr(alp3, 5));

                        ps3c = mult(ps3, ps3);
                        if( L_mult(ps3c, alpha) > L_mult(psc, alp) ) {
                            psc = ps3c;
                            alpha = alp;
                            ip0 = i0;
                            ip1 = i1;
                            ip2 = i2;
                            ip3 = i3;
                            shif = shift;
                        }
                    }  /*  end of for i3 = */

                    time --;
                    if(time <= 0 ) goto end_search;   /* Max time finish */
                    ptr_ri0i3 -= NB_POS;
                    ptr_ri1i3 -= NB_POS;

                }  /* end of if >thres */

                else {
                    ptr_ri2i3 += NB_POS;
                }

            } /* end of for i2 = */

            ptr_ri0i2 -= NB_POS;
            ptr_ri1i3 += NB_POS;

        } /* end of for i1 = */

        ptr_ri0i2 += NB_POS;
        ptr_ri0i3 += NB_POS;

    } /* end of for i0 = */

end_search:

    extra = time;

    /* Set the sign of impulses */

    i0 = p_sign[shr(ip0, 1)];
    i1 = p_sign[shr(ip1, 1)];
    i2 = p_sign[shr(ip2, 1)];
    i3 = p_sign[shr(ip3, 1)];

    /* Find the codeword corresponding to the selected positions */

    for(i=0; i<SubFrLen; i++) cod[i] = 0;

    if(shif > 0) {
        ip0 = add(ip0 ,1);
        ip1 = add(ip1 ,1);
        ip2 = add(ip2 ,1);
        ip3 = add(ip3 ,1);
    }

    cod[ip0] =  i0;
    cod[ip1] =  i1;
    if(ip2<SubFrLen) cod[ip2] = i2;
    if(ip3<SubFrLen) cod[ip3] = i3;

    /* find the filtered codeword */

    for (i = 0; i < SubFrLen; i++) y[i] = 0;

    if(i0 > 0)
        for(i=ip0, j=0; i<SubFrLen; i++, j++)
            y[i] = add(y[i], h[j]);
    else
        for(i=ip0, j=0; i<SubFrLen; i++, j++)
            y[i] = sub(y[i], h[j]);

    if(i1 > 0)
        for(i=ip1, j=0; i<SubFrLen; i++, j++)
            y[i] = add(y[i], h[j]);
    else
        for(i=ip1, j=0; i<SubFrLen; i++, j++)
            y[i] = sub(y[i], h[j]);

    if(ip2 < SubFrLen) {

        if(i2 > 0)
            for(i=ip2, j=0; i<SubFrLen; i++, j++)
                y[i] = add(y[i], h[j]);
        else
            for(i=ip2, j=0; i<SubFrLen; i++, j++)
                y[i] = sub(y[i], h[j]);
    }

    if(ip3 < SubFrLen) {

        if(i3 > 0)
            for(i=ip3, j=0; i<SubFrLen; i++, j++)
                y[i] = add(y[i], h[j]);
        else
            for(i=ip3, j=0; i<SubFrLen; i++, j++)
                y[i] = sub(y[i], h[j]);
    }

    /* find codebook index;  17-bit address */

    *code_shift = shif;

    *sign = 0;
    if(i0 > 0) *sign = add(*sign, 1);
    if(i1 > 0) *sign = add(*sign, 2);
    if(i2 > 0) *sign = add(*sign, 4);
    if(i3 > 0) *sign = add(*sign, 8);

    i = shr(ip0, 3);
    i = add(i, shl(shr(ip1, 3), 3));
    i = add(i, shl(shr(ip2, 3), 6));
    i = add(i, shl(shr(ip3, 3), 9));

    return i;
}

/*
**
**  Function:  G_code()
**
**  √Ë ˆ: Compute the gain of innovative code.
**
**
**  Links to the text: Section 2.16
**
** Input arguments:
**
**      Q15 emInt16 X[]        Code target.  (in Q0)
**      Q15 emInt16 Y[]        Filtered innovation code. (in Q12)
**
** Output:
**
**      Q15 emInt16 *gain_q    Gain of innovation code.  (in Q0)
**
**  Return value:
**
**      Q15 emInt16  index of innovation code gain
**
*/
Q15 emInt16 G_code(Q15 emInt16 X[], Q15 emInt16 Y[], Q15 emInt16 *gain_q)
{
    Q15 emInt16 i;
    Q15 emInt16 xy, yy, exp_xy, exp_yy, gain, gain_nq;
    Q31 emInt32 L_xy, L_yy;
    Q15 emInt16 dist, dist_min;


	LOG_StackAddr(__FUNCTION__);

    /* Scale down Y[] by 8 to avoid overflow */
    for(i=0; i<SubFrLen; i++)
        Y[i] = shr_1(Y[i], 3);

    /* Compute scalar product <X[],Y[]> */
    L_xy = 0L;
    for(i=0; i<SubFrLen; i++)
        L_xy = L_mac(L_xy, X[i], Y[i]);

    exp_xy = norm_l(L_xy);
    xy = extract_h( L_shl_1(L_xy, exp_xy) );

    if(xy <= 0) {
        gain = 0;
        *gain_q =FcbkGainTable[gain];
        return(gain);
    }

    /* Compute scalar product <Y[],Y[]> */
    L_yy = 0L;
    for(i=0; i<SubFrLen; i++)
        L_yy = L_mac(L_yy, Y[i], Y[i]);

    exp_yy = norm_l(L_yy);
    yy     = extract_h( L_shl(L_yy, exp_yy) );

    /* compute gain = xy/yy */
    xy = shr(xy, 1);             /* Be sure xy < yy */
    gain_nq = div_s( xy, yy);

    i = add(exp_xy, 5);          /* Denormalization of division */
    i = sub(i, exp_yy);

    gain_nq = shr_1(gain_nq, i);

    gain = (Q15 emInt16) 0;
    dist_min = sub(gain_nq, FcbkGainTable[0]);
    dist_min = abs_s(dist_min);
    for ( i =  1; i <NumOfGainLev ; i ++ ) {
        dist = sub(gain_nq, FcbkGainTable[i]);
        dist =abs_s(dist);
        if ( dist< dist_min) {
            dist_min = dist;
            gain = (Q15 emInt16) i ;
        }
    }
    *gain_q = FcbkGainTable[gain];

    return(gain);
}

/*
**
**  Function:       search_T0()
**
**  √Ë ˆ:          Gets parameters of pitch synchronous filter
**
**  Links to the text:    Section 2.16
**
**  Arguments:
**
**      Q15 emInt16 T0         Decoded pitch lag
**      Q15 emInt16 Gid        Gain vector index in the adaptive gain vector codebook
**      Q15 emInt16 *gain_T0   Pitch synchronous gain
**
**  Outputs:
**
**      Q15 emInt16 *gain_T0   Pitch synchronous filter gain
**
**  Return Value:
**
**      Q15 emInt16 T0_mod     Pitch synchronous filter lag
*/
Q15 emInt16 search_T0 ( Q15 emInt16 T0, Q15 emInt16 Gid, Q15 emInt16 *gain_T0)
{

    Q15 emInt16 T0_mod;

	LOG_StackAddr(__FUNCTION__);

    T0_mod = T0+epsi170[Gid];
    *gain_T0 = gain170[Gid];

    return(T0_mod);
}


#endif /* EM_USER_ENCODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
