/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:            "dec_cng.c"
**
** √Ë ˆ:     Comfort noise generation
**                  performed at the decoder part
**
** Functions:       Init_Dec_Cng()
**                  Dec_Cng()
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
#include "emG7231_LSP.h"
#include "emG7231_EXC_LBC.h"
#include "emG7231_BASOP.h"
#include "emG7231_UTIL_CNG.h"
#include "emG7231_DEC_CNG.h"
#include "emG7231_DECOD.h"


#if EM_USER_DECODER_G7231


/*
**
** Function:        Init_Dec_Cng()
**
** √Ë ˆ:     Initialize Dec_Cng emStatic variables
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

void Init_Dec_Cng(NEED_THIS0)
{
    emInt32  i;

	LOG_StackAddr(__FUNCTION__);

    DecCng.PastFtyp = 1;
    DecCng.SidGain = 0;
    for(i=0; i<LpcOrder; i++) DecCng.LspSid[i] = LspDcTable[i] ;
    DecCng.RandSeed = 12345;
    return;
}


/*
**
** Function:           Dec_Cng()
**
** √Ë ˆ:        Receives Ftyp
**                     0  :  for untransmitted frames
**                     2  :  for SID frames
**                     Decodes SID frames
**                     Computes current frame excitation
**                     Computes current frame LSPs
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 Ftyp        Type of silence frame
**  LINEDEF *Line      Coded parameters
**  Q15 emInt16 *DataExc    Current frame excitation
**  Q15 emInt16 *QntLpc     Interpolated frame LPC coefficients
**
** Outputs:
**
**  Q15 emInt16 *DataExc
**  Q15 emInt16 *QntLpc
**
** ∑µªÿ value:       None
**
*/
void Dec_Cng(NEED_THIS Q15 emInt16 Ftyp, LINEDEF *Line, Q15 emInt16 *DataExc, Q15 emInt16 *QntLpc)
{

    Q15 emInt16 temp;
    emInt32  i;

	LOG_StackAddr(__FUNCTION__);

    if(Ftyp == 2) {

 /*
  * SID Frame decoding
  */
        DecCng.SidGain = Dec_SidGain(Line->Sfs[0].Mamp);

        /* Inverse quantization of the LSP */
        Lsp_Inq( DecCng.LspSid, DecStat.PrevLsp, Line->LspId, 0) ;
    }

    else {

/*
 * non SID Frame
 */
        if(DecCng.PastFtyp == 1) {

 /*
  * Case of 1st SID frame erased : quantize-decode
  * energy estimate stored in DecCng.SidGain
  * scaling factor in DecCng.CurGain
  */
            temp = Qua_SidGain(&DecCng.SidGain, &DecCng.CurGain, 0);
            DecCng.SidGain = Dec_SidGain(temp);
        }
    }


    if(DecCng.PastFtyp == 1) {
        DecCng.CurGain = DecCng.SidGain;
    }
    else {
        DecCng.CurGain = extract_h(L_add( L_mult(DecCng.CurGain,0x7000),
                    L_mult(DecCng.SidGain,0x1000) ) ) ;
    }
    Calc_Exc_Rand(USE_THIS DecCng.CurGain, DecStat.PrevExc, DataExc,
                    &DecCng.RandSeed, Line);

    /* Interpolate the Lsp vectors */
    Lsp_Int_Dec( QntLpc, DecCng.LspSid, DecStat.PrevLsp ) ;

    /* Copy the LSP vector for the next frame */
    for ( i = 0 ; i < LpcOrder ; i ++ )
        DecStat.PrevLsp[i] = DecCng.LspSid[i] ;

    return;
}


#endif /* EM_USER_DECODER_G7231 */


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
