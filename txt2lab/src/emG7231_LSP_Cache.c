/*
 * LsptoA_1 函数运算量较高,在实际硬件环境中,可放入cache进行优化.
 */

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_BASOP.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_LSP.h"


#if EM_USER_DECODER_G7231


void  LsptoA_1( Q15 emInt16 *Lsp )
{
    emInt32    i,j   ;

    Q31 emInt32   Acc0,Acc1   ;
    Q15 emInt16   Tmp ;

    Q31 emInt32   P[LpcOrder/2+1] ;
    Q31 emInt32   Q[LpcOrder/2+1] ;


	LOG_StackAddr(__FUNCTION__);

 /*
  * Compute the cosines of the LSP frequencies by table lookup and
  * linear interpolation
  */
    for ( i = 0 ; i < LpcOrder ; i ++ ) {

 /*
  * Do the table lookup using bits [15:7] of the LSP frequency
  */
        j = (emInt32 ) shr( Lsp[i], (Q15 emInt16) 7 ) ;
        Acc0 = L_deposit_h( CosineTable[j] ) ;

 /*
  * Do the linear interpolations using bits [6:0] of the LSP
  * frequency
  */
        Tmp = sub(CosineTable[j+1], CosineTable[j] ) ;
        Acc0 = L_mac( Acc0, Tmp, add( shl( (Q15 emInt16)(Lsp[i] & 0x007f) ,
                                (Q15 emInt16)8 ), (Q15 emInt16) 0x0080 ) ) ;
        Acc0 = L_shl( Acc0, (Q15 emInt16) 1 ) ;
#if 0
        Lsp[i] = negate( round( Acc0 ) ) ;
#else
		{
			Q15 emInt16 temp;
			temp = round( Acc0 );
			Lsp[i] = negate(temp) ;
		}
#endif
    }


 /*
  * Compute the sum and difference polynomials with the real roots
  * removed.  These are computed by polynomial multiplication as
  * follows.  Let the sum polynomial be P(z).  Define the elementary
  * polynomials P_i(z) = 1 - 2cos(w_i) z^{-1} + z^{-2}, for 1<=i<=
  * 5, where {w_i} are the LSP frequencies corresponding to the sum
  * polynomial.  Then P(z) = P_1(z)P_2(z)...P_5(z).  Similarly
  * the difference polynomial Q(z) = Q_1(z)Q_2(z)...Q_5(z).
  */

 /*
  * Initialize the arrays with the coefficients of the product
  * P_1(z)P_2(z) and Q_1(z)Q_2(z).  Scale by 1/8.
  */
    P[0] = (Q31 emInt32) 0x10000000L ;
    P[1] = L_mult( Lsp[0], (Q15 emInt16) 0x2000 ) ;
    P[1] = L_mac( P[1], Lsp[2], (Q15 emInt16) 0x2000 ) ;
    P[2] = L_mult( Lsp[0], Lsp[2] ) ;
    P[2] = L_shr( P[2], (Q15 emInt16) 1 ) ;
    P[2] = L_add( P[2], (Q31 emInt32) 0x20000000L ) ;

    Q[0] = (Q31 emInt32) 0x10000000L ;
    Q[1] = L_mult( Lsp[1], (Q15 emInt16) 0x2000 ) ;
    Q[1] = L_mac( Q[1], Lsp[3], (Q15 emInt16) 0x2000 ) ;
    Q[2] = L_mult( Lsp[1], Lsp[3] ) ;
    Q[2] = L_shr_1( Q[2], (Q15 emInt16) 1 ) ;
    Q[2] = L_add( Q[2], (Q31 emInt32) 0x20000000L ) ;

 /*
  * Compute the intermediate polynomials P_1(z)P_2(z)...P_i(z) and
  * Q_1(z)Q_2(z)...Q_i(z), for i = 2, 3, 4.  Each intermediate
  * polynomial is symmetric, so only the coefficients up to i+1 need
  * by computed.  Scale by 1/2 each iteration for a total of 1/8.
  */
    for ( i = 2 ; i < LpcOrder/2 ; i ++ ) {

        /* Compute coefficient (i+1) */
        Acc0 = P[i] ;
        Acc0 = L_mls_1( Acc0, Lsp[2*i+0] ) ;
        Acc0 = L_add( Acc0, P[i-1] ) ;
        P[i+1] = Acc0 ;

        Acc1 = Q[i] ;
        Acc1 = L_mls_1( Acc1, Lsp[2*i+1] ) ;
        Acc1 = L_add( Acc1, Q[i-1] ) ;
        Q[i+1] = Acc1 ;

        /* Compute coefficients i, i-1, ..., 2 */
        for ( j = i ; j >= 2 ; j -- ) {
            Acc0 = P[j-1] ;
            Acc0 = L_mls_1( Acc0, Lsp[2*i+0] ) ;
            Acc0 = L_add( Acc0, L_shr_1(P[j], (Q15 emInt16) 1 ) ) ;
            Acc0 = L_add( Acc0, L_shr_1(P[j-2], (Q15 emInt16) 1 ) ) ;
            P[j] = Acc0 ;

            Acc1 = Q[j-1] ;
            Acc1 = L_mls_1( Acc1, Lsp[2*i+1] ) ;
            Acc1 = L_add( Acc1, L_shr_1(Q[j], (Q15 emInt16) 1 ) ) ;
            Acc1 = L_add( Acc1, L_shr_1(Q[j-2], (Q15 emInt16) 1 ) ) ;
            Q[j] = Acc1 ;
        }

        /* Compute coefficients 1, 0 */
        P[0] = L_shr( P[0], (Q15 emInt16) 1 ) ;
        Q[0] = L_shr( Q[0], (Q15 emInt16) 1 ) ;

        Acc0 = L_deposit_h( Lsp[2*i+0] ) ;
        Acc0 = L_shr_1( Acc0, (Q15 emInt16) i ) ;
        Acc0 = L_add( Acc0, P[1] ) ;
        Acc0 = L_shr_1( Acc0, (Q15 emInt16) 1 ) ;
        P[1] = Acc0 ;

        Acc1 = L_deposit_h( Lsp[2*i+1] ) ;
        Acc1 = L_shr_1( Acc1, (Q15 emInt16) i ) ;
        Acc1 = L_add( Acc1, Q[1] ) ;
        Acc1 = L_shr_1( Acc1, (Q15 emInt16) 1 ) ;
        Q[1] = Acc1 ;
    }


 /*
  * Convert the sum and difference polynomials to LPC coefficients
  * The LPC polynomial is the sum of the sum and difference
  * polynomials with the real zeros factored in: A(z) = 1/2 {P(z) (1
  * + z^{-1}) + Q(z) (1 - z^{-1})}.  The LPC coefficients are scaled
  * here by 16; the overall scale factor for the LPC coefficients
  * returned by this function is therefore 1/4.
  */
#if 0
    for ( i = 0 ; i < LpcOrder/2 ; i ++ ) {
        Acc0 = P[i] ;
        Acc0 = L_add( Acc0, P[i+1] ) ;
        Acc0 = L_sub( Acc0, Q[i] ) ;
        Acc0 = L_add( Acc0, Q[i+1] ) ;
        Acc0 = L_shl( Acc0, (Q15 emInt16) 3 ) ;
#if 0
        Lsp[i] = negate( round( Acc0 ) ) ;
#else
		{
			Q15 emInt16 tmp;
			tmp =  round( Acc0);
			Lsp[i] = negate(tmp) ;
		}
#endif

        Acc1 = P[i] ;
        Acc1 = L_add( Acc1, P[i+1] ) ;
        Acc1 = L_add( Acc1, Q[i] ) ;
        Acc1 = L_sub( Acc1, Q[i+1] ) ;
        Acc1 = L_shl( Acc1, (Q15 emInt16) 3 ) ;
#if 0
        Lsp[LpcOrder-1-i] = negate( round( Acc1 ) ) ;
#else
		{
			Q15 emInt16 tmp;
			tmp =  round( Acc1 );
			Lsp[LpcOrder-1-i] = negate(tmp) ;
		}
#endif
    }
#else

	for ( i = 0 ; i < LpcOrder/2 ; i ++ ) {
        Acc0 = P[i] ;
        Acc0 = L_add( Acc0, P[i+1] ) ;
        Acc0 = L_sub( Acc0, Q[i] ) ;
        Acc0 = L_add( Acc0, Q[i+1] ) ;
        Acc0 = L_shl( Acc0, (Q15 emInt16) 3 ) ;
        /*Lsp[i] = negate( round( Acc0 ) ) ;*/
#if 0
		Lsp[LpcOrder-1-i] = negate( round( Acc0 ) ) ;
#else
		{
			Q15 emInt16 tmp;
			tmp =  round( Acc0 );
			Lsp[LpcOrder-1-i] = negate(tmp) ;
		}
#endif
		
        Acc1 = P[i] ;
        Acc1 = L_add( Acc1, P[i+1] ) ;
        Acc1 = L_add( Acc1, Q[i] ) ;
        Acc1 = L_sub( Acc1, Q[i+1] ) ;
        Acc1 = L_shl( Acc1, (Q15 emInt16) 3 ) ;
        /*Lsp[LpcOrder-1-i] = negate( round( Acc1 ) ) ;*/
#if 0
		Lsp[i] = negate( round( Acc1 ) ) ;
#else
		{
			Q15 emInt16 tmp;
			tmp =  round( Acc1);
			Lsp[i] = negate(tmp) ;
		}
#endif

    }

#endif

}


#endif /* EM_USER_DECODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
