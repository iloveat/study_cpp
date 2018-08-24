/*
 *	Decod_Acbk 函数运算量较高,在实际硬件环境中,可放入cache进行优化.
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


/*
**
** Function:        Decod_Acbk()
**
** 描述:     Computes the adaptive codebook contribution from the previous
**                  excitation vector.
**                  With the gain index, the closed loop pitch lag, the jitter
**                  which when added to this pitch lag gives the actual closed
**                  loop value, and after having selected the proper codebook,
**                  the pitch contribution is reconstructed using the previous
**                  excitation buffer.
**
** Links to text:   Sections 2.14, 2.18 & 3.4
**
** Arguments:
**
**  Q15 emInt16 *Tv      Reconstructed excitation vector
**  Q15 emInt16 *PrevExc Previous excitation vector
**  Q15 emInt16 Olp      closed-loop pitch period
**  Q15 emInt16 Lid      Jitter around pitch period
**  Q15 emInt16 Gid      Gain vector index in 5- dimensional
**                      adaptive gain vector codebook
**
** Outputs:
**
**  Q15 emInt16 *Tv      Reconstructed excitation vector
**
** 返回 value:    None
**
*/
void  Decod_Acbk( NEED_THIS Q15 emInt16 *Tv, Q15 emInt16 *PrevExc, Q15 emInt16 Olp, Q15 emInt16 Lid, Q15 emInt16 Gid )
{
#if 0
	emInt32    i,j   ;

    Q31 emInt32   Acc0  ;
    Q15 emInt16   RezBuf[SubFrLen+ClPitchOrd-1] ;
    Q15 emInt16  *sPnt ;

	LOG_StackAddr(__FUNCTION__);

    Get_Rez( RezBuf, PrevExc, (Q15 emInt16)(Olp - (Q15 emInt16)Pstep + Lid) ) ;

    /* Select Quantization tables */
    i = 0 ;
    if ( WrkRate == Rate63 ) {
        if ( Olp >= (Q15 emInt16) (SubFrLen-2) ) i ++ ;
    }
    else {
        i = 1;
    }
    sPnt = AcbkGainTablePtr[i] ;

    sPnt += (emInt32 )Gid*20 ;

    for ( i = 0 ; i < SubFrLen ; i ++ ) {
        Acc0 = (Q31 emInt32) 0 ;
        for ( j = 0 ; j < ClPitchOrd ; j ++ )
            Acc0 = L_mac( Acc0, RezBuf[i+j], sPnt[j] ) ;
        Acc0 = L_shl( Acc0, (Q15 emInt16) 1 ) ;
        Tv[i] = round( Acc0 ) ;
    }

    return;

#else
	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	emInt32    i,j   ;	
    Q31 emInt32   Acc0  ;
    Q15 emInt16   RezBuf[SubFrLen+ClPitchOrd-1] ;
    Q15 emInt16  *sPnt ;

	LOG_StackAddr(__FUNCTION__);

	if(WrkRate == Rate63)
	{
		 Get_Rez( RezBuf, PrevExc, (Q15 emInt16)(Olp - (Q15 emInt16)Pstep + Lid) ) ;
	
		/* Select Quantization tables */
		i = 0 ;
		if ( Olp >= (Q15 emInt16) (SubFrLen-2) ) i ++ ;
		sPnt = (Q15 emInt16*)AcbkGainTablePtr[i] ;
	
		sPnt += (emInt32 )Gid*20 ;
	
		for ( i = 0 ; i < SubFrLen ; i ++ ) {
			Acc0 = (Q31 emInt32) 0 ;
			for ( j = 0 ; j < ClPitchOrd ; j ++ )
				Acc0 = L_mac( Acc0, RezBuf[i+j], sPnt[j] ) ;
			Acc0 = L_shl( Acc0, (Q15 emInt16) 1 ) ;
			Tv[i] = round( Acc0 ) ;
		}
	}
	else
	{
		emInt32   k,sum;
		Q15 emInt16   *PRezBuf=RezBuf;
		Q15 emInt16   Lag;
		/* Get RezBuf */
		Lag = (Q15 emInt16)(Olp - (Q15 emInt16)Pstep + Lid);
		for ( i = 0 ,j=PitchMax - (emInt32 ) Lag - ClPitchOrd/2 ; i < ClPitchOrd/2 ; i ++ ,j++)/*ClPitchOrd/2*/
		{
			RezBuf[i] = PrevExc[j] ;	
		}
		if(SubFrLen+ClPitchOrd/2<=Lag) 
		{
			for ( i = 0 ,k=ClPitchOrd/2,j=PitchMax - (emInt32 )Lag ;i < SubFrLen+ClPitchOrd/2 ; i ++ ,k++)/*SubFrLen+ClPitchOrd/2*/
			{
				RezBuf [k] = PrevExc[j+i] ;
			}
		}
		else
		{
			sum=0 ;
			for ( i = 0 ,k=ClPitchOrd/2,j=PitchMax - (emInt32 )Lag ;i < SubFrLen+ClPitchOrd/2 ; i ++ ,k++)/*SubFrLen+ClPitchOrd/2*/
			{
				if(sum==Lag) sum=0;
				RezBuf[k]=PrevExc[j+sum];
				sum++;
			}
		}
		sPnt = (Q15 emInt16*)AcbkGainTable170+(emInt32 )(Gid)*20 ;
		for ( i = 0 ; i < SubFrLen ; i ++ ,PRezBuf++) 
		{
			Acc0 =  0;
			Acc0 = L_add( Acc0, ((Q31 emInt32)PRezBuf[0] * (Q31 emInt32)sPnt[0] ) );
			Acc0 = L_add( Acc0, ((Q31 emInt32)PRezBuf[1] * (Q31 emInt32)sPnt[1] ) );
			Acc0 = L_add( Acc0, ((Q31 emInt32)PRezBuf[2] * (Q31 emInt32)sPnt[2] ) );
			Acc0 = L_add( Acc0, ((Q31 emInt32)PRezBuf[3] * (Q31 emInt32)sPnt[3] ) );
			Acc0 = L_add( Acc0, ((Q31 emInt32)PRezBuf[4] * (Q31 emInt32)sPnt[4] ) );
			Tv[i] = round( L_shl( Acc0, (Q15 emInt16) 2 ) ) ;
		}
	}
    return;	
#endif
}


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
