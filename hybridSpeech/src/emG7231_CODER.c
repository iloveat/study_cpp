/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "coder.c"
**
** √Ë ˆ:     Top-level source code for G.723 dual-rate coder
**
** Functions:       Init_Coder()
**                  Coder()
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
#include "emG7231_CODER.h"
#include "emG7231_LPC.h"
#include "emG7231_LSP.h"
#include "emG7231_EXC_LBC.h"
#include "emG7231_UTIL_LBC.h"
#include "emG7231_VAD.h"
#include "emG7231_COD_CNG.h"
#include "emG7231_TAME.h"


#if EM_USER_ENCODER_G7231


/*
   This file includes the coder main functions
*/


/*
**
** Function:        Init_Coder()
**
** √Ë ˆ:     Initializes non-zero state variables
**          for the coder.
**
** Links to text:   Section 2.21
** 
** Arguments:       None
**
** Outputs:     None
** 
** ∑µªÿ value:    None
**
*/

void  Init_Coder( NEED_THIS0 )
{
    emInt32    i;

	LOG_StackAddr(__FUNCTION__);

    /* Initialize encoder data structure with zeros */
    emMemSet(&CodStat, 0,sizeof(CODSTATDEF));

    /* Initialize the previously decoded LSP vector to the DC vector */
    for ( i = 0; i < LpcOrder; i ++ )
        CodStat.PrevLsp[i] = LspDcTable[i];

    /* Initialize the taming procedure */
    for(i=0; i<SizErr; i++) CodStat.Err[i] = Err0;

    return;
}


/*
**
** Function:        Coder()
**
** √Ë ˆ:     Implements G.723 dual-rate coder for    a frame
**          of speech
**
** Links to text:   Section 2
**
** Arguments:
**
**  Q15 emInt16 DataBuff[]   frame (480 bytes)
**

** Outputs:
**
**  Q15 emInt16 Vout[]       Encoded frame (20/24 bytes)
**
** ∑µªÿ value:
**
**  emBool     encoded code size in bytes or 0 on failed   
**
*/
emBool  Coder( NEED_THIS Q15 emInt16 *DataBuff, emPUInt8 Vout )
{
    emInt32      i, j;

    /*
      Local variables
    */
    Q15 emInt16   UnqLpc[SubFrames * LpcOrder];
    Q15 emInt16   QntLpc[SubFrames * LpcOrder];
    Q15 emInt16   PerLpc[2 * SubFrames * LpcOrder];

    Q15 emInt16   LspVect[LpcOrder];
    LINEDEF  Line;
    PWDEF    Pw[SubFrames];

    Q15 emInt16   ImpResp[SubFrLen];

    Q15 emPInt16  Dpnt;

	Q15 emInt16	 Ftyp	= 1;

    Q15 emInt16   DpntBuffer[PitchMax+Frame];

	LOG_StackAddr(__FUNCTION__);

    /*
      Coder Start
    */
    Line.Crc = (Q15 emInt16) 0;

    Rem_Dc( USE_THIS DataBuff );

    /* Compute the Unquantized Lpc set for whole frame */
    Comp_Lpc( USE_THIS UnqLpc, CodStat.PrevDat, DataBuff );

    /* Convert to Lsp */
    AtoLsp( LspVect, &UnqLpc[LpcOrder * (SubFrames - 1)], CodStat.PrevLsp );

    /* Compute the Vad */
    Ftyp = (Q15 emInt16) Comp_Vad( USE_THIS DataBuff );

    /* VQ Lsp vector */
    Line.LspId = Lsp_Qnt( LspVect, CodStat.PrevLsp );

    Mem_Shift( CodStat.PrevDat, DataBuff );

    /* Compute Perceptual filter Lpc coefficients */
    Wght_Lpc( PerLpc, UnqLpc );

    /* Apply the perceptual weighting filter */
    Error_Wght( USE_THIS DataBuff, PerLpc );

    /* Compute Open loop pitch estimates */
	Dpnt = DpntBuffer;

    /* Construct the buffer */
    for ( i = 0; i < PitchMax; i ++ )
        Dpnt[i] = CodStat.PrevWgt[i];
    for ( i = 0; i < Frame; i ++ )
        Dpnt[PitchMax + i] = DataBuff[i];

    Vec_Norm( Dpnt, (Q15 emInt16) (PitchMax+Frame) );

    j = PitchMax;
    for ( i = 0; i < SubFrames / 2; i ++ ) {
        Line.Olp[i] = Estim_Pitch( Dpnt, (Q15 emInt16) j );
        VadStat.Polp[i + 2] = Line.Olp[i];
        j += 2 * SubFrLen;
    }

    if (Ftyp != 1) {

        /* Case of inactive signal */


        /* Save PrevWgt */
        for ( i = 0; i < PitchMax; i ++ )
            CodStat.PrevWgt[i] = DataBuff[i + Frame - PitchMax];



        /* CodCng => Ftyp = 0 (untransmitted) or 2 (SID) */
        Cod_Cng( USE_THIS DataBuff, &Ftyp, &Line, QntLpc );

        /* Update the ringing delays */
        Dpnt = DataBuff;
        for( i = 0; i < SubFrames; i++ ) {

            /* Update exc_err */
            Update_Err( USE_THIS Line.Olp[i >> 1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn );

            Upd_Ring( USE_THIS Dpnt, &QntLpc[i * LpcOrder], &PerLpc[i * 2 * LpcOrder], CodStat.PrevErr );

            Dpnt += SubFrLen;
        }
    }

    else {

        /*
         Case of Active signal  (Ftyp=1)
        */

        /* Compute the Hmw */
        j = PitchMax;
        for ( i = 0; i < SubFrames; i ++ ) {
            Pw[i] = Comp_Pw( Dpnt, (Q15 emInt16) j, Line.Olp[i >> 1] );
            j += SubFrLen;
        }

        /* Reload the buffer */
        for ( i = 0; i < PitchMax; i ++ )
            Dpnt[i] = CodStat.PrevWgt[i];
        for ( i = 0; i < Frame; i ++ )
            Dpnt[PitchMax + i] = DataBuff[i];

        /* Save PrevWgt */
        for ( i = 0; i < PitchMax; i ++ )
            CodStat.PrevWgt[i] = Dpnt[Frame + i];

        /* Apply the Harmonic filter */
        j = 0;
        for ( i = 0; i < SubFrames; i ++ ) {
            Filt_Pw( DataBuff, Dpnt, (Q15 emInt16) j , Pw[i] );
            j += SubFrLen;
        }


        /* Inverse quantization of the LSP */
        Lsp_Inq( LspVect, CodStat.PrevLsp, Line.LspId, Line.Crc );

        /* Interpolate the Lsp vectors */
        Lsp_Int( QntLpc, LspVect, CodStat.PrevLsp );

        /* Copy the LSP vector for the next frame */
        for ( i = 0; i < LpcOrder; i ++ )
            CodStat.PrevLsp[i] = LspVect[i];

        /*
         Start the sub frame processing loop
        */
        Dpnt = DataBuff;

        for ( i = 0; i < SubFrames; i ++ ) {

            /* Compute full impulse response */
            Comp_Ir( ImpResp, &QntLpc[i * LpcOrder], &PerLpc[i * 2 * LpcOrder], Pw[i] );

            /* Subtract the ringing of previous sub-frame */
            Sub_Ring( USE_THIS Dpnt, &QntLpc[i * LpcOrder], &PerLpc[i * 2 * LpcOrder], CodStat.PrevErr, Pw[i] );

            /* Compute adaptive code book contribution */
            Find_Acbk( USE_THIS Dpnt, ImpResp, CodStat.PrevExc, &Line, (Q15 emInt16) i );

            /* Compute fixed code book contribution */
            Find_Fcbk( USE_THIS Dpnt, ImpResp, &Line, (Q15 emInt16) i );

            /* Reconstruct the excitation */
            Decod_Acbk( USE_THIS ImpResp, CodStat.PrevExc, Line.Olp[i >> 1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn );

            for ( j = SubFrLen; j < PitchMax; j ++ )
                CodStat.PrevExc[j - SubFrLen] = CodStat.PrevExc[j];

            for ( j = 0; j < SubFrLen; j ++ ) {
                Dpnt[j] = shl( Dpnt[j], (Q15 emInt16) 1 );
                Dpnt[j] = add( Dpnt[j], ImpResp[j] );
                CodStat.PrevExc[PitchMax - SubFrLen + j] = Dpnt[j];
            }

            /* Update exc_err */
            Update_Err( USE_THIS Line.Olp[i >> 1], Line.Sfs[i].AcLg, Line.Sfs[i].AcGn );

            /* Update the ringing delays */
            Upd_Ring( USE_THIS Dpnt, &QntLpc[i * LpcOrder], &PerLpc[i * 2 * LpcOrder], CodStat.PrevErr );

            Dpnt += SubFrLen;
        }  /* end of subframes loop */

        /*
         Save Vad information and reset CNG random generator
        */
        CodCng.PastFtyp = 1;
        CodCng.RandSeed = 12345;

    } /* End of active frame case */

    /* Pack the Line structure */
    Line_Pack( USE_THIS &Line, Vout, Ftyp );

    return emTrue;
}


#endif /* EM_USER_ENCODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
