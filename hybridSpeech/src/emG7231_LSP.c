/*
**
** File:    lsp.c
**
** 描述: Functions that implement line spectral pair 
**      (LSP) operations.  
**
** Functions:
**
**  Converting between linear predictive coding (LPC) coefficients
**  and LSP frequencies:
**
**      AtoLsp()
**      LsptoA()
**
**  Vector quantization (VQ) of LSP frequencies:
**
**      Lsp_Qnt()
**      Lsp_Svq()
**      Lsp_Inq()
**
**  Interpolation of LSP frequencies:
**
**      Lsp_Int()
*/

/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_BASOP.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_LSP.h"


#if EM_USER_ENCODER_G7231


/*
**
** Function:            AtoLsp()
**
** 描述:     Transforms 10 LPC coefficients to the 10
**          corresponding LSP frequencies for a subframe.
**          This transformation is done once per frame,
**          for subframe 3 only.  The transform algorithm
**          generates sum and difference polynomials from
**          the LPC coefficients.  It then evaluates the
**          sum and difference polynomials at uniform
**          intervals of pi/256 along the unit circle.
**          Intervals where a sign change occurs are
**          interpolated to find the zeros of the
**          polynomials, which are the LSP frequencies.
**
** Links to text:   Section 2.5
**
** Arguments:       
**
**  Q15 emInt16 *LspVect     Empty Buffer
**  Q15 emInt16 Lpc[]        Unquantized LPC coefficients (10 words)
**  Q15 emInt16 PrevLsp[]    LSP frequencies from the previous frame (10 words)
**
** Outputs:
**
**  Q15 emInt16 LspVect[]    LSP frequencies for the current frame (10 words)
**
** 返回 value:        None
**
**/
void AtoLsp( Q15 emInt16 *LspVect, Q15 emInt16 *Lpc, Q15 emInt16 *PrevLsp )
{

    emInt32    i,j,k ;

    Q31 emInt32   Lpq[LpcOrder+2] ;
    Q15 emInt16   Spq[LpcOrder+2] ;

    Q15 emInt16   Exp   ;
    Q15 emInt16   LspCnt ;

    Q31 emInt32   PrevVal,CurrVal   ;
    Q31 emInt32   Acc0,Acc1   ;

	LOG_StackAddr(__FUNCTION__);


 /*
  * Perform a bandwidth expansion on the LPC coefficients.  This
  * scales the poles of the LPC synthesis filter by a factor of
  * 0.994.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        LspVect[i] = mult_r_1( Lpc[i], BandExpTable[i] ) ;


 /*
  * Compute the sum and difference polynomials with the roots at z =
  * -1 (sum) or z = +1 (difference) removed.  Let these polynomials
  * be P(z) and Q(z) respectively, and let their coefficients be
  * {p_i} amd {q_i}.  The coefficients are stored in the array Lpq[]
  * as follows: p_0, q_0, p_1, q_1, ..., p_5, q_5.  There is no need
  * to store the other coefficients because of symmetry.
  */


 /*
  * Set p_0 = q_0 = 1.  The LPC coefficients are already scaled by
  *  1/4.  P(z) and Q(z) are scaled by an additional scaling factor of
  *  1/16, for an overall factor of 1/64 = 0x02000000L.
  */

    Lpq[0] = Lpq[1] = (Q31 emInt32) 0x02000000L ;

 /*
  * This loop computes the coefficients of P(z) and Q(z).  The long
  * division (to remove the real zeros) is done recursively.
  */
    for ( i = 0 ; i < LpcOrder/2 ; i ++ ) {

        /* P(z) */
        Acc0 = L_negate( Lpq[2*i+0] ) ;
        Acc1 = L_deposit_h( LspVect[i] ) ;
        Acc1 = L_shr_1( Acc1, (Q15 emInt16) 4 ) ;
        Acc0 = L_sub( Acc0, Acc1 ) ;
        Acc1 = L_deposit_h( LspVect[LpcOrder-1-i] ) ;
        Acc1 = L_shr_1( Acc1, (Q15 emInt16) 4 ) ;
        Acc0 = L_sub( Acc0, Acc1 ) ;
        Lpq[2*i+2] = Acc0 ;

        /* Q(z) */
        Acc0 = Lpq[2*i+1] ;
        Acc1 = L_deposit_h( LspVect[i] ) ;
        Acc1 = L_shr_1( Acc1, (Q15 emInt16) 4 ) ;

        Acc0 = L_sub( Acc0, Acc1 ) ;
        Acc1 = L_deposit_h( LspVect[LpcOrder-1-i] ) ;
        Acc1 = L_shr_1( Acc1, (Q15 emInt16) 4 ) ;
        Acc0 = L_add( Acc0, Acc1 ) ;
        Lpq[2*i+3] = Acc0 ;
    }

 /*
  * Divide p_5 and q_5 by 2 for proper weighting during polynomial
  * evaluation.
  */
    Lpq[LpcOrder+0] = L_shr_1( Lpq[LpcOrder+0], (Q15 emInt16) 1 ) ;
    Lpq[LpcOrder+1] = L_shr_1( Lpq[LpcOrder+1], (Q15 emInt16) 1 ) ;

 /*
  * Normalize the polynomial coefficients and convert to shorts
  */

    /* Find the maximum */
    Acc1 = L_abs( Lpq[0] ) ;
    for ( i = 1 ; i < LpcOrder+2 ; i ++ ) {
        Acc0 = L_abs( Lpq[i] ) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    /* Compute the normalization factor */
    Exp = norm_l( Acc1 ) ;


    /* Normalize and convert to shorts */
    for ( i = 0 ; i < LpcOrder+2 ; i ++ ) {
        Acc0 = L_shl( Lpq[i], Exp ) ;
        Spq[i] = round( Acc0 ) ;
    }

 /*
  * Initialize the search loop
  */

 /*
  * The variable k is a flag that indicates which polynomial (sum or
  * difference) the algorithm is currently evaluating.  Start with
  * the sum.
  */
    k = 0 ;

    /* Evaluate the sum polynomial at frequency zero */
    PrevVal = (Q31 emInt32) 0 ;
    for ( j = 0 ; j <= LpcOrder/2 ; j ++ )
        PrevVal = L_mac_1( PrevVal, Spq[2*j], CosineTable[0] ) ;


 /*
  * Search loop.  Evaluate P(z) and Q(z) at uniform intervals of
  * pi/256 along the unit circle.  Check for zero crossings.  The
  * zeros of P(w) and Q(w) alternate, so only one of them need by
  * evaluated at any given step.
  */
    LspCnt = (Q15 emInt16) 0 ;
    for ( i = 1 ; i < CosineTableSize/2 ; i ++ ) {

        /* Evaluate the selected polynomial */
        CurrVal = (Q31 emInt32) 0 ;
        for ( j = 0 ; j <= LpcOrder/2 ; j ++ )
            CurrVal = L_mac_1( CurrVal, Spq[LpcOrder-2*j+k],
                                    CosineTable[i*j%CosineTableSize] ) ;

        /* Check for a sign change, indicating a zero crossing */
        if ( (CurrVal ^ PrevVal) < (Q31 emInt32) 0 ) {

 /*
  * Interpolate to find the bottom 7 bits of the
  * zero-crossing frequency
  */
            Acc0 = L_abs( CurrVal ) ;
            Acc1 = L_abs( PrevVal ) ;
            Acc0 = L_add( Acc0, Acc1 ) ;

            /* Normalize the sum */
            Exp = norm_l( Acc0 ) ;
            Acc0 = L_shl( Acc0, Exp ) ;
            Acc1 = L_shl( Acc1, Exp ) ;

            Acc1 = L_shr( Acc1, (Q15 emInt16) 8 ) ;

            LspVect[LspCnt] = div_l( Acc1, extract_h( Acc0 ) ) ;

 /*
  * Add the upper part of the zero-crossing frequency,
  * i.e. bits 7-15
  */
            Exp = shl( (Q15 emInt16) (i-1), (Q15 emInt16) 7 ) ;
            LspVect[LspCnt] = add( LspVect[LspCnt], Exp ) ;
            LspCnt ++ ;

            /* Check if all zeros have been found */
            if ( LspCnt == (Q15 emInt16) LpcOrder )
                break ;

 /*
  * Switch the pointer between sum and difference polynomials
  */
            k ^= 1 ;

 /*
  * Evaluate the new polynomial at the current frequency
  */
            CurrVal = (Q31 emInt32) 0 ;
            for ( j = 0 ; j <= LpcOrder/2 ; j ++ )
                CurrVal = L_mac_1( CurrVal, Spq[LpcOrder-2*j+k],
                                    CosineTable[i*j%CosineTableSize] ) ;
        }

        /* Update the previous value */
        PrevVal = CurrVal ;
    }


 /*
  * Check if all 10 zeros were found.  If not, ignore the results of
  * the search and use the previous frame's LSP frequencies instead.
  */
    if ( LspCnt != (Q15 emInt16) LpcOrder ) {
        for ( j = 0 ; j < LpcOrder ; j ++ )
            LspVect[j] = PrevLsp[j] ;
    }

    return ;
}

/*
**
** Function:            Lsp_Qnt()
**
** 描述:     Vector quantizes the LSP frequencies.  The LSP
**          vector is divided into 3 sub-vectors, or
**          bands, of dimension 3, 3, and 4.  Each band is
**          quantized separately using a different VQ
**          table.  Each table has 256 entries, so the
**          quantization generates three indices of 8 bits
**          each.  (Only the LSP vector for subframe 3 is
**          quantized per frame.)
**
** Links to text:   Section 2.5
**
** Arguments:       
**
**  Q15 emInt16 CurrLsp[]    Unquantized LSP frequencies for the current frame (10 words)
**  Q15 emInt16 PrevLsp[]    LSP frequencies from the previous frame (10 words)
**
** Outputs:             Quantized LSP frequencies for the current frame (10 words)
**
** 返回 value:
**
**  Q31 emInt32      Long word packed with the 3 VQ indices.  Band 0
**          corresponds to bits [23:16], band 1 corresponds
**          to bits [15:8], and band 2 corresponds to bits [7:0].
**          (Bit 0 is the least significant.)
**
*/
Q31 emInt32   Lsp_Qnt( Q15 emInt16 *CurrLsp, Q15 emInt16 *PrevLsp )
{
    emInt32    i ;

    Q15 emInt16   Wvect[LpcOrder] ;

    Q15 emInt16   Tmp0,Tmp1   ;
    Q15 emInt16   Exp   ;


	LOG_StackAddr(__FUNCTION__);

 /*
  * Compute the VQ weighting vector.  The weights assign greater
  * precision to those frequencies that are closer together.
  */

    /* Compute the end differences */
    Wvect[0] = sub( CurrLsp[1], CurrLsp[0] ) ;
    Wvect[LpcOrder-1] = sub( CurrLsp[LpcOrder-1], CurrLsp[LpcOrder-2] ) ;

    /* Compute the rest of the differences */
    for ( i = 1 ; i < LpcOrder-1 ; i ++ ) {
        Tmp0 = sub( CurrLsp[i+1], CurrLsp[i] ) ;
        Tmp1 = sub( CurrLsp[i], CurrLsp[i-1] ) ;
        if ( Tmp0 > Tmp1 )
            Wvect[i] = Tmp1 ;
        else
            Wvect[i] = Tmp0 ;
    }

    /* Invert the differences */
    Tmp0 = (Q15 emInt16) 0x0020 ;
    for ( i = 0 ; i < LpcOrder ; i ++ ) {

        if ( Wvect[i] > Tmp0 )
            Wvect[i] = div_s( Tmp0, Wvect[i] ) ;
        else
            Wvect[i] = EM_MAX_INT16 ;
    }

    /* Normalize the weight vector */
    Tmp0 = (Q15 emInt16) 0 ;
    for ( i = 0 ; i < LpcOrder ; i ++ )
        if ( Wvect[i] > Tmp0 )
            Tmp0 = Wvect[i] ;

    Exp = norm_s( Tmp0 ) ;
    for ( i = 0 ; i < LpcOrder ; i ++ )
        Wvect[i] = shl( Wvect[i], Exp ) ;


 /*
  * Compute the VQ target vector.  This is the residual that remains
  * after subtracting both the DC and predicted
  * components.
  */

 /*
  * Subtract the DC component from both the current and previous LSP
  * vectors.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ ) {
        CurrLsp[i] = sub( CurrLsp[i], LspDcTable[i] ) ;
        PrevLsp[i] = sub( PrevLsp[i], LspDcTable[i] ) ;
    }

 /*
  * Generate the prediction vector and subtract it.  Use a constant
  * first-order predictor based on the previous  LSP
  * vector.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ ) {
        Tmp0 = mult_r_1( PrevLsp[i], (Q15 emInt16) LspPrd0 ) ;
        CurrLsp[i] = sub( CurrLsp[i], Tmp0 ) ;
    }

 /*
  * Add the DC component back to the previous LSP vector.  This
  * vector is needed in later routines.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        PrevLsp[i] = add( PrevLsp[i], LspDcTable[i] ) ;

 /*
  * Do the vector quantization for all three bands
  */
    return Lsp_Svq( CurrLsp, Wvect ) ;
}

/*
**
** Function:            Lsp_Svq()
**
** 描述:     Performs the search of the VQ tables to find
**          the optimum LSP indices for all three bands.
**          For each band, the search finds the index which 
**          minimizes the weighted squared error between 
**          the table entry and the target.
**
** Links to text:   Section 2.5
**
** Arguments:       
**
**  Q15 emInt16 Tv[]     VQ target vector (10 words)
**  Q15 emInt16 Wvect[]      VQ weight vector (10 words)
**
** Outputs:         None
**
** 返回 value:    
**
**  Q31 emInt32      Long word packed with the 3 VQ indices.  Band 0
**          corresponds to bits [23:16], band 1 corresponds
**          to bits [15:8], and band 2 corresponds to bits [7:0].
**              
*/
Q31 emInt32   Lsp_Svq( Q15 emInt16 *Tv, Q15 emInt16 *Wvect )
{
    emInt32    i,j,k ;

    Q31 emInt32   Rez,Indx    ;
    Q31 emInt32   Acc0,Acc1   ;

    Q15 emInt16   Tmp[LpcOrder] ;
    Q15 emInt16  *LspQntPnt  ;


	LOG_StackAddr(__FUNCTION__);

 /*
  * Initialize the return value
  */
    Rez = (Q31 emInt32) 0 ;

 /*
  * Quantize each band separately
  */
    for ( k = 0 ; k < LspQntBands ; k ++ ) {

 /*
  * Search over the entire VQ table to find the index that
  * minimizes the error.
  */

        /* Initialize the search */
        Acc1 = (Q31 emInt32) -1 ;
        Indx = (Q31 emInt32) 0 ;
        LspQntPnt = (Q15 emInt16*)BandQntTable[k] ;

        for ( i = 0 ; i < LspCbSize ; i ++ ) {

 /*
  * Generate the metric, which is the negative error with the
  * constant component removed.
  */
            for ( j = 0 ; j < BandInfoTable[k][1] ; j ++ )
                Tmp[j] = mult_r_1( Wvect[BandInfoTable[k][0]+j],
                                                            LspQntPnt[j] ) ;

            Acc0 = (Q31 emInt32) 0 ;
            for ( j = 0 ; j < BandInfoTable[k][1] ; j ++ )
                Acc0 = L_mac( Acc0, Tv[BandInfoTable[k][0]+j], Tmp[j] ) ;
            Acc0 = L_shl( Acc0, (Q15 emInt16) 1 ) ;
            for ( j = 0 ; j < BandInfoTable[k][1] ; j ++ )
                Acc0 = L_msu( Acc0, LspQntPnt[j], Tmp[j] ) ;

            LspQntPnt += BandInfoTable[k][1] ;

 /*
  * Compare the metric to the previous maximum and select the
  * new index
  */
            if ( Acc0 > Acc1 ) {
                Acc1 = Acc0 ;
                Indx = (Q31 emInt32) i ;
            }
        }

 /*
  * Pack the result with the optimum index for this band
  */
        Rez = L_shl( Rez, (Q15 emInt16) LspCbBits ) ;
        Rez = L_add( Rez, Indx ) ;
    }

    return Rez ;
}


#endif /* EM_USER_ENCODER_G7231 */


/*
**
** Function:            Lsp_Inq()
**
** 描述:     Performs inverse vector quantization of the
**          LSP frequencies.  The LSP vector is divided
**          into 3 sub-vectors, or bands, of dimension 3,
**          3, and 4.  Each band is inverse quantized
**          separately using a different VQ table.  Each
**          table has 256 entries, so each VQ index is 8
**          bits.  (Only the LSP vector for subframe 3 is
**          quantized per frame.)
**
** Links to text:   Sections 2.6, 3.2
**
** Arguments:
**
**  Q15 emInt16 *Lsp     Empty buffer
**  Q15 emInt16 PrevLsp[]    Quantized LSP frequencies from the previous frame
**               (10 words)
**  Q31 emInt32 LspId        Long word packed with the 3 VQ indices.  Band 0
**               corresponds to bits [23:16], band 1 corresponds
**               to bits [15:8], and band 2 corresponds to bits
**               [7:0].
**  Q15 emInt16 Crc      Frame erasure indicator
**
** Outputs:
**
**  Q15 emInt16 Lsp[]        Quantized LSP frequencies for current frame (10
**               words)
**
** 返回 value:         None
**
*/
void Lsp_Inq( Q15 emInt16 *Lsp, Q15 emInt16 *PrevLsp, Q31 emInt32 LspId, Q15 emInt16 Crc )
{
#if 0
    emInt32   i,j   ;

    Q15 emInt16  *LspQntPnt  ;


    Q15 emInt16   Scon  ;
    Q15 emInt16   Lprd  ;

    Q15 emInt16   Tmp   ;
    emBool     Test  ;


	LOG_StackAddr(__FUNCTION__);

 /*
  * Check for frame erasure.  If a frame erasure has occurred, the
  * resulting VQ table entries are zero.  In addition, a different
  * fixed predictor and minimum frequency separation are used.
  */
/*    if ( Crc == (Q15 emInt16) 0 ) {*/
        Scon = (Q15 emInt16) 0x0100 ;
        Lprd = LspPrd0 ;
/*
		}
		else {
			LspId = (Q31 emInt32) 0 ;
			Scon = (Q15 emInt16) 0x0200 ;
			Lprd = LspPrd1 ;
		}
*/


 /*
  * Inverse quantize the 10th-order LSP vector.  Each band is done
  * separately.
  */
    for ( i = LspQntBands-1; i >= 0 ; i -- ) {

 /*
  * Get the VQ table entry corresponding to the transmitted index
  */
        Tmp = (Q15 emInt16) ( LspId & (Q31 emInt32) 0x000000ff ) ;
        LspId >>= 8 ;

        LspQntPnt = BandQntTable[i] ;

        for ( j = 0 ; j < BandInfoTable[i][1] ; j ++ )
            Lsp[BandInfoTable[i][0] + j] =
                                LspQntPnt[Tmp*BandInfoTable[i][1] + j] ;
    }

 /*
  * Subtract the DC component from the previous frame's quantized
  * vector
  */
    for ( j = 0 ; j < LpcOrder ; j ++ )
        PrevLsp[j] = sub(PrevLsp[j], LspDcTable[j] ) ;

 /*
  * Generate the prediction vector using a fixed first-order
  * predictor based on the previous frame's quantized
  * vector
  */
    for ( j = 0 ; j < LpcOrder ; j ++ ) {
        Tmp = mult_r_1( PrevLsp[j], Lprd ) ;
        Lsp[j] = add( Lsp[j], Tmp ) ;
    }

 /*
  * Add the DC component back to the previous quantized vector,
  * which is needed in later routines
  */
    for ( j = 0 ; j < LpcOrder ; j ++ ) {
        PrevLsp[j] = add( PrevLsp[j], LspDcTable[j] ) ;
        Lsp[j] = add( Lsp[j], LspDcTable[j] ) ;
    }


 /*
  * Perform a stability test on the quantized LSP frequencies.  This
  * test checks that the frequencies are ordered, with a minimum
  * separation between each.  If the test fails, the frequencies are
  * iteratively modified until the test passes.  If after 10
  * iterations the test has not passed, the previous frame's
  * quantized LSP vector is used.
  */
    for ( i = 0 ; i < LpcOrder ; i ++ ) {

        /* Check the first frequency */
        if ( Lsp[0] < (Q15 emInt16) 0x180 )
            Lsp[0] = (Q15 emInt16) 0x180 ;

        /* Check the last frequency */
        if ( Lsp[LpcOrder-1] > (Q15 emInt16) 0x7e00 )
            Lsp[LpcOrder-1] = (Q15 emInt16) 0x7e00 ;

        /* Perform the modification */
        for ( j = 1 ; j < LpcOrder ; j ++ ) {

            Tmp = add( Scon, Lsp[j-1] ) ;
            Tmp = sub( Tmp, Lsp[j] ) ;
            if ( Tmp > (Q15 emInt16) 0 ) {
                Tmp = shr( Tmp, (Q15 emInt16) 1 ) ;
                Lsp[j-1] = sub( Lsp[j-1], Tmp ) ;
                Lsp[j] = add( Lsp[j], Tmp ) ;
            }
        }

        Test = False ;

 /*
  * Test the modified frequencies for stability.  Break out of
  * the loop if the frequencies are stable.
  */
        for ( j = 1 ; j < LpcOrder ; j ++ ) {
            Tmp = add( Lsp[j-1], Scon ) ;
            Tmp = sub( Tmp, (Q15 emInt16) 4 ) ;
            Tmp = sub( Tmp, Lsp[j] ) ;
            if ( Tmp > (Q15 emInt16) 0 )
                Test = True ;
        }

        if ( Test == False )
            break ;
    }


 /*
  * 返回 the result of the stability check.  True = not stable,
  * False = stable.
  */
    if ( Test == True) {
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Lsp[j] = PrevLsp[j] ;
    }

    return;
#else
	emInt32   i,j   ;
    Q15 emInt16  *LspQntPnt  ;
    Q15 emInt16   Scon  ;
    Q15 emInt16   Lprd  ;
    Q15 emInt16   Tmp   ;
    emBool     Test  ;
	

	LOG_StackAddr(__FUNCTION__);
	
    Scon = (Q15 emInt16) 0x0100 ;
    Lprd = LspPrd0 ;
	Test = emFalse ;
	
	/*
	* Inverse quantize the 10th-order LSP vector.  Each band is done
	* separately.
	*/
    for ( i = LspQntBands-1; i >= 0 ; i -- ) 
	{
		/*  * Get the VQ table entry corresponding to the transmitted index  */
        Tmp = (Q15 emInt16) ( LspId & (Q31 emInt32) 0xff ) ;
        LspId >>= 8 ;
		
        LspQntPnt = (Q15 emInt16*)BandQntTable[i] ;
		
        for ( j = 0 ; j < BandInfoTable[i][1] ; j ++ )
            Lsp[BandInfoTable[i][0] + j] =
			LspQntPnt[Tmp*BandInfoTable[i][1] + j] ;
    }
	/*
    Lsp[j] = Lsp[j] + (PrevLsp[j] - LspDcTable[j])*Lprd + LspDcTable[j];
	*/
    for ( j = 0 ; j < LpcOrder ; j ++ ) 
	{
		Lsp[j]=add( Lsp[j],mult_r( sub(PrevLsp[j],LspDcTable[j]) ,Lprd ) );
		Lsp[j]=add(Lsp[j],LspDcTable[j]);
	}
	/*
	* Perform a stability test on the quantized LSP frequencies.  This
	* test checks that the frequencies are ordered, with a minimum
	* separation between each.  If the test fails, the frequencies are
	* iteratively modified until the test passes.  If after 10
	* iterations the test has not passed, the previous frame's
	* quantized LSP vector is used.
	*/
    for ( i = 0 ; i < LpcOrder ; i ++ ) {
		
        /* Check the first frequency */
        if ( Lsp[0] < (Q15 emInt16) 0x180 )
            Lsp[0] = (Q15 emInt16) 0x180 ;
		
        /* Check the last frequency */
        if ( Lsp[LpcOrder-1] > (Q15 emInt16) 0x7e00 )
            Lsp[LpcOrder-1] = (Q15 emInt16) 0x7e00 ;
		
        /* Perform the modification */
        for ( j = 1 ; j < LpcOrder ; j ++ ) {
			
            Tmp = add( Scon, Lsp[j-1] ) ;
            Tmp = sub( Tmp, Lsp[j] ) ;
            if ( Tmp > (Q15 emInt16) 0 ) {
                Tmp = shr( Tmp, (Q15 emInt16) 1 ) ;
                Lsp[j-1] = sub( Lsp[j-1], Tmp ) ;
                Lsp[j] = add( Lsp[j], Tmp ) ;
            }
        }
		
        Test = emFalse ;
		
		/*
		* Test the modified frequencies for stability.  Break out of
		* the loop if the frequencies are stable.
		*/
        for ( j = 1 ; j < LpcOrder ; j ++ ) {
            Tmp = add( Lsp[j-1], Scon ) ;
            Tmp = sub( Tmp, (Q15 emInt16) 4 ) ;
            Tmp = sub( Tmp, Lsp[j] ) ;
            if ( Tmp > (Q15 emInt16) 0 )
                Test = emTrue ;
        }
		
        if ( !Test )
            break ;
    }
	
	
	/*
	* 返回 the result of the stability check.  True = not stable,
	* False = stable.
	*/
    if ( Test ) {
        for ( j = 0 ; j < LpcOrder ; j ++ )
            Lsp[j] = PrevLsp[j] ;
    }
	
    return;
#endif
}

/*
**
** Function:            Lsp_Int()
**
** 描述:     Computes the quantized LPC coefficients for a
**          frame.  First the quantized LSP frequencies
**          for all subframes are computed by linear
**          interpolation.  These frequencies are then
**          transformed to quantized LPC coefficients.
**
** Links to text:   Sections 2.7, 3.3
**
** Arguments:
**
**  Q15 emInt16 *QntLpc      Empty buffer
**  Q15 emInt16 CurrLsp[]    Quantized LSP frequencies for the current frame,
**               subframe 3 (10 words)
**  Q15 emInt16 PrevLsp[]    Quantized LSP frequencies for the previous frame,
**               subframe 3 (10 words)
**
** Outputs:
**
**  Q15 emInt16 QntLpc[]     Quantized LPC coefficients for current frame, all
**               subframes (40 words)
**
** 返回 value:        None
**
*/

void  Lsp_Int( Q15 emInt16 *QntLpc, Q15 emInt16 *CurrLsp, Q15 emInt16 *PrevLsp )
{
#if 0
    emInt32    i,j   ;

    Q15 emInt16   Tmp   ;
    Q15 emInt16  *Dpnt  ;

    Q31 emInt32   Acc0  ;


	LOG_StackAddr(__FUNCTION__);

 /*
  * Initialize the interpolation factor
  */
    Tmp = (Q15 emInt16) (EM_MIN_INT16 / SubFrames ) ;

    Dpnt = QntLpc ;


 /*
  * Do for all subframes
  */
    for ( i = 0 ; i < SubFrames ; i ++ ) {

 /*
  * Compute the quantized LSP frequencies by linear interpolation
  * of the frequencies from subframe 3 of the current and
  * previous frames
  */
        for ( j = 0 ; j < LpcOrder ; j ++ ) {
            Acc0 = L_deposit_h( PrevLsp[j] ) ;
            Acc0 = L_mac( Acc0, Tmp, PrevLsp[j] ) ;
            Acc0 = L_msu( Acc0, Tmp, CurrLsp[j] ) ;
            Dpnt[j] = round( Acc0 ) ;
        }

 /*
  * Convert the quantized LSP frequencies to quantized LPC
  * coefficients
  */
        LsptoA( Dpnt ) ;
        Dpnt += LpcOrder ;

        /* Update the interpolation factor */
        Tmp = add_1( Tmp, (Q15 emInt16) (EM_MIN_INT16 / SubFrames ) ) ;
    }
#else
	emInt32    i;
	
    emInt16  *Dpnt0, *Dpnt1, *Dpnt2, *Dpnt3;

	LOG_StackAddr(__FUNCTION__);
	
	Dpnt0 = QntLpc ;
	Dpnt1 = Dpnt0 + LpcOrder;
	Dpnt2 = Dpnt1 + LpcOrder;
	Dpnt3 = Dpnt2 + LpcOrder;
	
	/*对2.7公式8的变形*/
	
	for (i = 0 ; i < LpcOrder; i ++ )
	{
		Dpnt3[i] = CurrLsp[i];
		Dpnt1[i] = (PrevLsp[i] + CurrLsp[i]) >> 1;
		Dpnt0[i] = (PrevLsp[i] + Dpnt1[i]) >> 1;
		Dpnt2[i] = (Dpnt1[i] + CurrLsp[i]) >> 1;
	}
	
    LsptoA( Dpnt0);
	LsptoA( Dpnt1);
	LsptoA( Dpnt2);
	LsptoA( Dpnt3);
#endif
}


#if EM_USER_DECODER_G7231


void  Lsp_Int_Dec( Q15 emInt16 *QntLpc, Q15 emInt16 *CurrLsp, Q15 emInt16 *PrevLsp )
{
	emInt32    i;

	Q15 emInt16  *Dpnt0, *Dpnt1, *Dpnt2, *Dpnt3;

	LOG_StackAddr(__FUNCTION__);

	Dpnt0 = QntLpc ;
	Dpnt1 = Dpnt0 + LpcOrder;
	Dpnt2 = Dpnt1 + LpcOrder;
	Dpnt3 = Dpnt2 + LpcOrder;

	/* 对2.7公式8的变形 */

	for (i = 0 ; i < LpcOrder; i ++ )
	{
		Dpnt3[i] = CurrLsp[i];
		Dpnt1[i] = extract_l(L_add(PrevLsp[i], CurrLsp[i]) >> 1);
		Dpnt0[i] = extract_l(L_add(PrevLsp[i], Dpnt1[i]) >> 1);
		Dpnt2[i] = extract_l(L_add(Dpnt1[i], CurrLsp[i]) >> 1);
	}

    LsptoA_1(Dpnt0);
	LsptoA_1(Dpnt1);
	LsptoA_1(Dpnt2);
	LsptoA_1(Dpnt3);
}

#endif /* EM_USER_DECODER_G7231 */

/*
**
** Function:            LsptoA()
**
** 描述:     Converts LSP frequencies to LPC coefficients
**          for a subframe.  Sum and difference
**          polynomials are computed from the LSP
**          frequencies (which are the roots of these
**          polynomials).  The LPC coefficients are then
**          computed by adding the sum and difference
**          polynomials.
**          
** Links to text:   Sections 2.7, 3.3
**
** Arguments:       
**
**  Q15 emInt16 Lsp[]        LSP frequencies (10 words)
**
** Outputs:
**
**  Q15 emInt16 Lsp[]        LPC coefficients (10 words)
**
** 返回 value:        None
** 
*/

void  LsptoA( Q15 emInt16 *Lsp )
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
#if 1
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
		Lsp[LpcOrder-1-i] = negate( round( Acc0 ) ) ;
		
        Acc1 = P[i] ;
        Acc1 = L_add( Acc1, P[i+1] ) ;
        Acc1 = L_add( Acc1, Q[i] ) ;
        Acc1 = L_sub( Acc1, Q[i+1] ) ;
        Acc1 = L_shl( Acc1, (Q15 emInt16) 3 ) ;
        /*Lsp[LpcOrder-1-i] = negate( round( Acc1 ) ) ;*/
		Lsp[i] = negate( round( Acc1 ) ) ;
    }

#endif
}


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
