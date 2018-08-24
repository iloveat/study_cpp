/* 
 *	Synt 函数运算量较高,在实际硬件环境中,可放入cache进行优化.
 */

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_BASOP.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_CODER.h"
#include "emG7231_DECOD.h"
#include "emG7231_UTIL_LBC.h"
#include "emG7231_LPC.h"
#include "emG7231_COD_CNG.h"


#if EM_USER_DECODER_G7231


/*
**
** Function:        Synt()
**
** 描述:     Implements the decoder synthesis filter for a
**          subframe.  This is a tenth-order IIR filter.
**
** Links to text:   Section 3.7
**
** Arguments:       
**
**  Q15 emInt16 Dpnt[]       Pitch-postfiltered excitation for the current
**               subframe ppf[n] (60 words)
**  Q15 emInt16 Lpc[]        Quantized LPC coefficients (10 words)
**  
** Inputs:
**
**  DecStat.SyntIirDl[] Synthesis filter memory from previous
subframe (10 words)
**
** Outputs:     
**
**  Q15 emInt16 Dpnt[]       Synthesized speech vector sy[n]
**  DecStat.SyntIirDl[] Updated synthesis filter memory 
**
** 返回 value:    None
**
*/

#if 0

void     Synt( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *Lpc )
{
    emInt32    i,j   ;

    Q31 emInt32   Acc0  ;


	LOG_StackAddr(__FUNCTION__);

 /*
  * Do for all elements in the subframe
  */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

 /*
  * Input the current subframe's excitation
  */
        Acc0 = L_deposit_h( Dpnt[i] ) ;
        Acc0 = L_shr( Acc0, (Q15 emInt16) 3 ) ;

 /*
  * Synthesis
  */

        /* Filter */
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Acc0 = L_mac( Acc0, Lpc[j], DecStat.SyntIirDl[j] ) ;

        /* Update memory */
        for ( j = LpcOrder-1 ; j > 0 ; j -- )
            DecStat.SyntIirDl[j] = DecStat.SyntIirDl[j-1] ;

        Acc0 = L_shl( Acc0, (Q15 emInt16) 2 ) ;

        DecStat.SyntIirDl[0] = round( Acc0 ) ;

 /*
  * Scale output if postfilter is off.  (Otherwise output is
  * scaled by the gain scaling unit.)
  */
        if ( UsePf )
            Dpnt[i] = DecStat.SyntIirDl[0] ;
        else
            Dpnt[i] = shl( DecStat.SyntIirDl[0], (Q15 emInt16) 1 ) ;

    }

}

#else

void     Synt( Q15 emInt16 *Dpnt, Q15 emInt16 *Lpc)
{
   	Q31 emInt32   Acc0; 
	emInt32    i; 
	Q15 emInt16 *PDpnt=Dpnt+LpcOrder; 

	LOG_StackAddr(__FUNCTION__);

	for ( i = 0 ; i < SubFrLen ; i ++ ,PDpnt++,Dpnt++) 
	{
		Acc0 = L_deposit_h( *PDpnt ) ;
		/*Acc0 = L_shr( Acc0, (Q15 emInt16) 3 ) ;*/
		Acc0 = L_shr( Acc0, (Q15 emInt16) 4 ) ;
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[0] * (Q31 emInt32)Dpnt[0]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[1] * (Q31 emInt32)Dpnt[1]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[2] * (Q31 emInt32)Dpnt[2]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[3] * (Q31 emInt32)Dpnt[3]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[4] * (Q31 emInt32)Dpnt[4]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[5] * (Q31 emInt32)Dpnt[5]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[6] * (Q31 emInt32)Dpnt[6]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[7] * (Q31 emInt32)Dpnt[7]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[8] * (Q31 emInt32)Dpnt[8]));
		Acc0 = L_add( Acc0, ((Q31 emInt32)Lpc[9] * (Q31 emInt32)Dpnt[9]));
		/*Acc0 = L_shl( Acc0, (Q15 emInt16) 2 ) ;*/
		Acc0 = L_shl( Acc0, (Q15 emInt16) 3 ) ;
		*PDpnt = round(Acc0);
	}

}


#endif


#endif /* EM_USER_DECODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
