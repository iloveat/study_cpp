/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "decod.c"
**
** 描述:     Top-level source code for G.723 dual-rate decoder
**
** Functions:       Init_Decod()
**                  Decod()
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
#include "emG7231_DECOD.h"
#include "emG7231_UTIL_LBC.h"
#include "emG7231_LPC.h"
#include "emG7231_LSP.h"
#include "emG7231_EXC_LBC.h"
#include "emG7231_DEC_CNG.h"


#if EM_USER_DECODER_G7231


/*
   This file includes the decod main functions
*/


/*
**
** Function:        Init_Decod()
**
** 描述:     Initializes non-zero state variables
**          for the decod.
**
** Links to text:   Section 2.21
** 
** Arguments:       None
**
** Outputs:     None
** 
** 返回 value:    None
**
*/

void    Init_Decod( NEED_THIS0 )
{
    emInt32    i ;

	LOG_StackAddr(__FUNCTION__);

    /* Initialize decoder data structure with zeros */
    emMemSet(&DecStat, 0,sizeof(DECSTATDEF));

	/* Initialize the previously decoded LSP vector to the DC vector */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        DecStat.PrevLsp[i] = LspDcTable[i] ;

	/* Initialize the gain scaling unit memory to a constant */
	DecStat.Gain = (Q15 emInt16)0x1000;

    return;
}


/*
**
** Function:        Decod()
**
** 描述:     Implements G.723 dual-rate decoder for  a frame
**          of speech
**
** Links to text:   Section 3
**
** Arguments:
**
**  Q15 emInt16 *DataBuff    Empty buffer
**  Q15 emInt16 Vinp[]       Encoded frame (22/26 bytes)
**

** Outputs:
**
**  Q15 emInt16 DataBuff[]   Decoded frame (480 bytes)
**
** 返回 value:
**
**  emBool		decoded code size in bytes or 0 on failed   
**
*/

emInt32     Decod( NEED_THIS Q15 emInt16 *DataBuff, emPUInt8 Vinp )
{
    emInt32    i,j   ;

#if 0 /* 优化掉 */
	Q31 emInt32   Senr ;
#endif
    Q15 emInt16   QntLpc[SubFrames*LpcOrder] ;
    Q15 emInt16   AcbkCont[SubFrLen] ;

    Q15 emInt16   LspVect[LpcOrder] ;
    //Q15 emInt16   Temp[PitchMax+Frame] ;

	Q15 emInt16   *Temp ;

    Q15 emInt16  *Dpnt ;

	/*Q15 emInt16   tempbuf[Frame + LpcOrder];*/

    LINEDEF  Line ;
    /*PFDEF    Pf[SubFrames] ;*/

    Q15 emInt16   Ftyp;

	LOG_StackAddr(__FUNCTION__);


	//hyl 2012-10-16 将栈优化成堆，降低栈的最大空间，但注意：栈比堆的访问快
#if DEBUG_LOG_SWITCH_HEAP
	Temp = (emPInt16) emHeap_AllocZero( ( PitchMax+Frame) * sizeof(emInt16), "解码Decod：《提示音模块  》");
#else
	Temp = (emPInt16) emHeap_AllocZero( ( PitchMax+Frame) * sizeof(emInt16));
#endif

    /*
     * Decode the packed bitstream for the frame.  (Text: Section 4;
     * pars of sections 2.17, 2.18)
     */
    /*Line = Line_Unpk( Vinp, &Ftyp, Crc, WrkRate ) ;*/
	Line = Line_Unpk( USE_THIS Vinp, &Ftyp ) ;

    /*
     * Update the frame erasure count (Text: Section 3.10)
     */
/*
    if ( Line.Crc != (Q15 emInt16) 0 ) {
        if(DecCng->PastFtyp == 1) Ftyp = 1;  / * active * /
        else Ftyp = 0;  / * untransmitted * /
    }
*/

    if(Ftyp != 1) {

        /* Silence frame : do noise generation */
        Dec_Cng(USE_THIS Ftyp, &Line, DataBuff, QntLpc);
    }

    else {

        /*
         * Update the frame erasure count (Text: Section 3.10)
         */
/*
        if ( Line.Crc != (Q15 emInt16) 0 )
            DecStat->. = add( DecStat->Ecount, (Q15 emInt16) 1 ) ;
        else
            DecStat->Ecount = (Q15 emInt16) 0 ;

		if ( DecStat->Ecount > (Q15 emInt16) ErrMaxNum )
            DecStat->Ecount = (Q15 emInt16) ErrMaxNum ;
*/

        /*
         * Decode the LSP vector for subframe 3.  (Text: Section 3.2)
         */
        Lsp_Inq( LspVect, DecStat.PrevLsp, Line.LspId, 0 ) ;

        /*
         * Interpolate the LSP vectors for subframes 0--2.  Convert the
         * LSP vectors to LPC coefficients.  (Text: Section 3.3)
         */
        Lsp_Int_Dec( QntLpc, LspVect, DecStat.PrevLsp ) ;

        /* Copy the LSP vector for the next frame */
        for ( i = 0 ; i < LpcOrder ; i ++ )
            DecStat.PrevLsp[i] = LspVect[i] ;

        /*
         * In case of no erasure, update the interpolation gain memory.
         * Otherwise compute the interpolation gain (Text: Section 3.10)
         */
        /*if ( DecStat.Ecount == (Q15 emInt16) 0 ) {*/
            DecStat.InterGain = add( Line.Sfs[SubFrames-2].Mamp,
                                            Line.Sfs[SubFrames-1].Mamp ) ;
            DecStat.InterGain = shr( DecStat.InterGain, (Q15 emInt16) 1 ) ;
            DecStat.InterGain = FcbkGainTable[DecStat.InterGain] ;
/*
        }
        else
            DecStat->InterGain = mult_r( DecStat->InterGain, (Q15 emInt16) 0x6000 ) ;
*/


        /*
         * Generate the excitation for the frame
         */
        for ( i = 0 ; i < PitchMax ; i ++ )
            Temp[i] = DecStat.PrevExc[i] ;

        Dpnt = &Temp[PitchMax] ;

        /*if ( DecStat->Ecount == (Q15 emInt16) 0 ) {*/

            for ( i = 0 ; i < SubFrames ; i ++ ) {

                /* Generate the fixed codebook excitation for a
                   subframe. (Text: Section 3.5) */
                Fcbk_Unpk( USE_THIS Dpnt, Line.Sfs[i], Line.Olp[i>>1], (Q15 emInt16) i ) ;

                /* Generate the adaptive codebook excitation for a
                   subframe. (Text: Section 3.4) */
                Decod_Acbk( USE_THIS AcbkCont, &Temp[SubFrLen*i], Line.Olp[i>>1],
                                    Line.Sfs[i].AcLg, Line.Sfs[i].AcGn ) ;

                /* Add the adaptive and fixed codebook contributions to
                   generate the total excitation. */
                for ( j = 0 ; j < SubFrLen ; j ++ ) {
                    Dpnt[j] = shl( Dpnt[j], (Q15 emInt16) 1 ) ;
                    Dpnt[j] = add( Dpnt[j], AcbkCont[j] ) ;
                }

                Dpnt += SubFrLen ;
            }

            /* Save the excitation */
            for ( j = 0 ; j < Frame ; j ++ )
                DataBuff[j] = Temp[PitchMax+j] ;

            /* Compute interpolation index. (Text: Section 3.10) */
            /* Use DecCng.SidGain and DecCng.CurGain to store    */
            /* excitation energy estimation                      */
/*
            DecStat->InterIndx = Comp_Info( Temp, Line.Olp[SubFrames/2-1],
                                       &DecCng->SidGain, &DecCng->CurGain ) ;
*/

            /* Compute pitch post filter coefficients.  (Text: Section 3.6) */
/*
            if ( G7231_Decode->UsePf )
                for ( i = 0 ; i < SubFrames ; i ++ )
                    Pf[i] = Comp_Lpf( Temp, Line.Olp[i>>1], (Q15 emInt16) i, G7231_Decode->WrkRate ) ;
*/

            /* Reload the original excitation */
/*
            for ( j = 0 ; j < PitchMax ; j ++ )
                Temp[j] = DecStat->PrevExc[j] ;
            for ( j = 0 ; j < Frame ; j ++ )
                Temp[PitchMax+j] = DataBuff[j] ;
*/
			for ( j = 0 ,i=Frame; j < PitchMax ; j ++ ,i++)
				DecStat.PrevExc[j] = Temp[i] ;

            /* Perform pitch post filtering for the frame.  (Text: Section
               3.6) */
/*
            if ( G7231_Decode->UsePf )
                for ( i = 0 ; i < SubFrames ; i ++ )
                    Filt_Lpf( DataBuff, Temp, Pf[i], (Q15 emInt16) i ) ;
*/

            /* Save Lsps --> LspSid */
            for(i=0; i< LpcOrder; i++)
                DecCng.LspSid[i] = DecStat.PrevLsp[i];
/*
        }
        else {
            / * If a frame erasure has occurred, regenerate the
               signal for the frame. (Text: Section 3.10) * /
            Regen( DataBuff, Temp, DecStat->InterIndx, DecStat->InterGain,
                                        DecStat->Ecount, &DecStat->Rseed ) ;
        }
        / * Update the previous excitation for the next frame * /
        for ( j = 0 ; j < PitchMax ; j ++ )
            DecStat->PrevExc[j] = Temp[Frame+j] ;
*/

        /* Resets random generator for CNG */
        DecCng.RandSeed = 12345;
    }

    /* Save Ftyp information for next frame */
    DecCng.PastFtyp = Ftyp;

    /*
     * Synthesize the speech for the frame
     */
    /*Dpnt = DataBuff ;*/
	Dpnt = DataBuff - LpcOrder;

    for ( i = 0 ; i < SubFrames ; i ++ ) {

        /* Compute the synthesized speech signal for a subframe.
         * (Text: Section 3.7)
         */
		Synt( Dpnt, &QntLpc[i*LpcOrder] ) ;

#if 0 /* 优化掉 */
        if ( UsePf ) {

                 /* Do the formant post filter. (Text: Section 3.8) */
                 Senr = Spf( Dpnt, &QntLpc[i*LpcOrder], DecStat ) ;/*DECSTATDEF *DecStat*/

                 /* Do the gain scaling unit.  (Text: Section 3.9) */
                 Scale( Dpnt, Senr, DecStat ) ;
             }
#endif

        Dpnt += SubFrLen ;
    }

	//hyl 2012-10-16 将栈优化成堆，降低栈的最大空间，但注意：栈比堆的访问快
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(Temp, ( PitchMax+Frame) * sizeof(emInt16), "解码Decod：《提示音模块  》");
#else
	emHeap_Free(Temp, ( PitchMax+Frame) * sizeof(emInt16));
#endif

	
	if ( Ftyp == 1 )
	{
		if(WrkRate == Rate53)
			return 20;
		else
			return 24;
	}
	else if ( Ftyp == 2 )
		return 4;
	else if ( Ftyp == 0)
		return 1;
	else 
		return 0;
}


#endif /* EM_USER_DECODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
