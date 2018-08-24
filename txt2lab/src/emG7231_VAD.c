/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**

** File:        "vad.c"
**
** ����:     Voice Activity Detection
**
** Functions:       Init_Vad()
**                  Vad()
**
**
*/

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_BASOP.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_LSP.h"
#include "emG7231_VAD.h"
#include "emG7231_CODER.h"


#if EM_USER_ENCODER_G7231


void    Init_Vad( NEED_THIS0 )
{
    emInt32  i ;

	LOG_StackAddr(__FUNCTION__);

    VadStat.Hcnt = 3 ;
    VadStat.Vcnt = 0 ;
    VadStat.Penr = 0x00000400L ;
    VadStat.Nlev = 0x00000400L ;

    VadStat.Aen = 0 ;

    VadStat.Polp[0] = 1 ;
    VadStat.Polp[1] = 1 ;
    VadStat.Polp[2] = SubFrLen ;
    VadStat.Polp[3] = SubFrLen ;

    for(i=0; i < LpcOrder; i++) VadStat.NLpc[i] = 0;

}


emBool Comp_Vad( NEED_THIS Q15 emInt16 *Dpnt )
{
    emInt32  i,j ;

    Q31 emInt32  Acc0,Acc1 ;
    Q15 emInt16  Tm0, Tm1, Tm2 ;
    Q15 emInt16  Minp ;

    emBool    VadState = 1 ;

	emStatic emConst Q15 emInt16  ScfTab[11] = {
         9170 ,
         9170 ,
         9170 ,
         9170 ,
        10289 ,
        11544 ,
        12953 ,
        14533 ,
        16306 ,
        18296 ,
        20529 ,
    } ;

	LOG_StackAddr(__FUNCTION__);

    if ( !UseVx )
        return VadState ;

    /* Find Minimum pitch period */
    Minp = PitchMax ;
    for ( i = 0 ; i < 4 ; i ++ ) {
        if ( Minp > VadStat.Polp[i] )
            Minp = VadStat.Polp[i] ;
    }

    /* Check that all are multiplies of the minimum */
    Tm2 = 0 ;
    for ( i = 0 ; i < 4 ; i ++ ) {
        Tm1 = Minp ;
        for ( j = 0 ; j < 8 ; j ++ ) {
            Tm0 = sub( Tm1, VadStat.Polp[i] ) ;
            Tm0 = abs_s( Tm0 ) ;
            if ( Tm0 <= 3 )
                Tm2 ++ ;
            Tm1 = add( Tm1, Minp ) ;
        }
    }

    /* Update adaptation enable counter if not periodic and not sine */
    if ( (Tm2 == 4) || (CodStat.SinDet < 0) )
        VadStat.Aen += 2 ;
    else
        VadStat.Aen -- ;

    /* Clip it */
    if ( VadStat.Aen > 6 )
        VadStat.Aen = 6 ;
    if ( VadStat.Aen < 0 )
        VadStat.Aen = 0 ;

    /* Inverse filter the data */
    Acc1 = 0L ;
    for ( i = SubFrLen ; i < Frame ; i ++ ) {

        Acc0 = L_mult( Dpnt[i], 0x2000 ) ;
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_msu( Acc0, Dpnt[i-j-1], VadStat.NLpc[j] ) ;
        Tm0 = round ( Acc0 ) ;
        Acc1 = L_mac( Acc1, Tm0, Tm0 ) ;
    }

    /* Scale the rezidual energy */
    Acc1 = L_mls( Acc1, (Q15 emInt16) 2913 ) ;

    /* Clip noise level in any case */
    if ( VadStat.Nlev > VadStat.Penr ) {
        Acc0 = L_sub( VadStat.Penr, L_shr( VadStat.Penr, 2 ) ) ;
        VadStat.Nlev = L_add( Acc0, L_shr( VadStat.Nlev, 2 ) ) ;
    }


    /* Update the noise level, if adaptation is enabled */
    if ( !VadStat.Aen ) {
        VadStat.Nlev = L_add( VadStat.Nlev, L_shr( VadStat.Nlev, 5 ) ) ;
    }
    /* Decay Nlev by small amount */
    else {
        VadStat.Nlev = L_sub( VadStat.Nlev, L_shr( VadStat.Nlev,11 ) ) ;
    }

    /* Update previous energy */
    VadStat.Penr = Acc1 ;

    /* CLip Noise Level */
    if ( VadStat.Nlev < 0x00000080L )
        VadStat.Nlev = 0x00000080L ;
    if ( VadStat.Nlev > 0x0001ffffL )
        VadStat.Nlev = 0x0001ffffL ;

    /* Compute the treshold */
    Acc0 = L_shl( VadStat.Nlev, 13 ) ;
    Tm0 = norm_l( Acc0 ) ;
    Acc0 = L_shl( Acc0, Tm0 ) ;
    Acc0 &= 0x3f000000L ;
    Acc0 <<= 1 ;
    Tm1 = extract_h( Acc0 ) ;
    Acc0 = L_deposit_h( ScfTab[Tm0] ) ;
    Acc0 = L_mac_1( Acc0, Tm1, ScfTab[Tm0-1] ) ;
    Acc0 = L_msu( Acc0, Tm1, ScfTab[Tm0] ) ;
    Tm1 = extract_h( Acc0 ) ;
    Tm0 = extract_l( L_shr( VadStat.Nlev, 2 ) ) ;
    Acc0 = L_mult( Tm0, Tm1 ) ;
    Acc0 >>= 11 ;

    /* Compare with the treshold */
    if ( Acc0 > Acc1 )
        VadState = 0 ;

    /* Do the various counters */
    if ( VadState ) {
        VadStat.Vcnt ++ ;
        VadStat.Hcnt ++ ;
    }
    else {
        VadStat.Vcnt -- ;
        if ( VadStat.Vcnt < 0 )
            VadStat.Vcnt = 0 ;
    }

    if ( VadStat.Vcnt >= 2 ) {
        VadStat.Hcnt = 6 ;
        if ( VadStat.Vcnt >= 3 )
            VadStat.Vcnt = 3 ;
    }

    if ( VadStat.Hcnt ) {
        VadState = 1 ;
        if ( VadStat.Vcnt == 0 )
            VadStat.Hcnt -- ;
    }

    /* Update Periodicy detector */
    VadStat.Polp[0] = VadStat.Polp[2] ;
    VadStat.Polp[1] = VadStat.Polp[3] ;

    return VadState ;
}


#endif /* EM_USER_ENCODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
