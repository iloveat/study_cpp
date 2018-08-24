/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "util_cng.c"
**
** ÃèÊö:     General Comfort Noise Generation functions
**
**
** Functions:       Calc_Exc_Rand() Computes random excitation
**                                  used both by coder & decoder
**                  Qua_SidGain()   Quantization of SID gain
**                                  used by coder
**                  Dec_SidGain()   Decoding of SID gain
**                                  used both by coder & decoder
**
** Local functions :
**                  distG()
**                  random_number()
*/

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231


#include "emG7231_TYPEDEF.h"
#include "emG7231_CST_LBC.h"
#include "emG7231_TAB_LBC.h"
#include "emG7231_UTIL_LBC.h"
#include "emG7231_EXC_LBC.h"
#include "emG7231_BASOP.h"
#include "emG7231_UTIL_CNG.h"


/* Declaration of local functions */
emStatic Q15 emInt16 random_number(Q15 emInt16 number_max_p1, Q15 emInt16 *nRandom);

/*
**
** Function:           Calc_Exc_Rand()
**
** ÃèÊö:        Computation of random excitation for inactive frames:
**                     Adaptive codebook entry selected randomly
**                     Higher rate innovation pattern selected randomly
**                     Computes innovation gain to match curGain
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 curGain     current average gain to match
**  Q15 emInt16 *PrevExc    previous/current excitation (updated)
**  Q15 emInt16 *DataEXc    current frame excitation
**  Q15 emInt16 *nRandom    random generator status (input/output)
**
** Outputs:
**
**  Q15 emInt16 *PrevExc
**  Q15 emInt16 *DataExc
**  Q15 emInt16 *nRandom
**
** ·µ»Ø value:       None
**
*/
void Calc_Exc_Rand(NEED_THIS Q15 emInt16 curGain, Q15 emInt16 *PrevExc, Q15 emInt16 *DataExc,
                                      Q15 emInt16 *nRandom, LINEDEF *Line)
{
    emInt32  i, i_subfr, iblk;
    Q15 emInt16 temp, temp2;
    Q15 emInt16 j;
    Q15 emInt16 TabPos[2*NbPulsBlk], TabSign[2*NbPulsBlk];
    Q15 emInt16 *ptr_TabPos, *ptr_TabSign;
    Q15 emInt16 *ptr1, *curExc;
    Q15 emInt16 sh1, x1, x2, inter_exc, delta, b0;
    Q31 emInt32 L_acc, L_c, L_temp;
    Q15 emInt16 tmp[SubFrLen/Sgrid];
    Q15 emInt16 offset[SubFrames];
    Q15 emInt16 tempExc[SubFrLenD];

	LOG_StackAddr(__FUNCTION__);

 /*
  * generate LTP codes
  */
    Line->Olp[0] = random_number(21, nRandom) + (Q15 emInt16)123;
    Line->Olp[1] = random_number(21, nRandom) + (Q15 emInt16)123;
    for(i_subfr=0; i_subfr<SubFrames; i_subfr++) {  /* in [1, NbFilt] */
        Line->Sfs[i_subfr].AcGn = random_number(NbFilt, nRandom) + (Q15 emInt16)1;
    }
    Line->Sfs[0].AcLg = 1;
    Line->Sfs[1].AcLg = 0;
    Line->Sfs[2].AcLg = 1;
    Line->Sfs[3].AcLg = 3;


 /*
  * Random innovation :
  * Selection of the grids, signs and pulse positions
  */

    /* Signs and Grids */
    ptr_TabSign = TabSign;
    ptr1 = offset;
    for(iblk=0; iblk<SubFrames/2; iblk++) {
        temp    = random_number((1 << (NbPulsBlk+2)), nRandom);
        *ptr1++ = temp & (Q15 emInt16)0x0001;
        temp    = shr(temp, 1);
        *ptr1++ = add( (Q15 emInt16) SubFrLen, (Q15 emInt16) (temp & 0x0001) );
        for(i=0; i<NbPulsBlk; i++) {
            *ptr_TabSign++= shl(sub((Q15 emInt16)(temp & (Q15 emInt16)0x0002), 1), 14);
            temp = shr(temp, 1);
        }
    }

    /* Positions */
    ptr_TabPos  = TabPos;
    for(i_subfr=0; i_subfr<SubFrames; i_subfr++) {

        for(i=0; i<(SubFrLen/Sgrid); i++) tmp[i] = (Q15 emInt16)i;
        temp = (SubFrLen/Sgrid);
        for(i=0; i<Nb_puls[i_subfr]; i++) {
            j = random_number(temp, nRandom);
            *ptr_TabPos++ = add(shl(tmp[(emInt32 )j],1), offset[i_subfr]);
            temp = sub(temp, 1);
            tmp[(emInt32 )j] = tmp[(emInt32 )temp];
        }
    }

 /*
  * Compute fixed codebook gains
  */

    ptr_TabPos = TabPos;
    ptr_TabSign = TabSign;
    curExc = DataExc;
    i_subfr = 0;
    for(iblk=0; iblk<SubFrames/2; iblk++) {

        /* decode LTP only */
        Decod_Acbk(USE_THIS curExc, &PrevExc[0], Line->Olp[iblk],
                    Line->Sfs[i_subfr].AcLg, Line->Sfs[i_subfr].AcGn);
        Decod_Acbk(USE_THIS &curExc[SubFrLen], &PrevExc[SubFrLen], Line->Olp[iblk],
            Line->Sfs[i_subfr+1].AcLg, Line->Sfs[i_subfr+1].AcGn);

        temp2 = 0;
        for(i=0; i<SubFrLenD; i++) {
            temp = abs_s(curExc[i]);
            if(temp > temp2) temp2 = temp;
        }
        if(temp2 == 0) sh1 = 0;
        else {
            sh1 = sub(4,norm_s(temp2)); /* 4 bits of margin  */
            if(sh1 < -2) sh1 = -2;
        }

        L_temp = 0L;
        for(i=0; i<SubFrLenD; i++) {
            temp  = shr_1(curExc[i], sh1); /* left if sh1 < 0 */
            L_temp = L_mac(L_temp, temp, temp);
            tempExc[i] = temp;
        }  /* ener_ltp x 2**(-2sh1+1) */

        L_acc = 0L;
        for(i=0; i<NbPulsBlk; i++) {
            L_acc = L_mac(L_acc, tempExc[(emInt32 )ptr_TabPos[i]], ptr_TabSign[i]);
        }
        inter_exc = extract_h(L_shl(L_acc, 1)); /* inter_exc x 2-sh1 */

        /* compute SubFrLenD x curGain**2 x 2**(-2sh1+1)    */
        /* curGain input = 2**5 curGain                     */
        L_acc = L_mult(curGain, SubFrLen);
        L_acc = L_shr(L_acc, 6);
        temp  = extract_l(L_acc);   /* SubFrLen x curGain : avoids overflows */
        L_acc = L_mult(temp, curGain);
        temp = shl(sh1, 1);
        temp = add(temp, 4);
        L_acc = L_shr(L_acc, temp); /* SubFrLenD x curGain**2 x 2**(-2sh1+1) */

        /* c = (ener_ltp - SubFrLenD x curGain**2)/nb_pulses_blk */
        /* compute L_c = c >> 2sh1-1                                */
        L_acc = L_sub(L_temp, L_acc);
        /* x 1/nb_pulses_blk */
        L_c  = L_mls(L_acc, InvNbPulsBlk);

/*
 * Solve EQ(X) = X**2 + 2 b0 X + c
 */
        /* delta = b0 x b0 - c */
        b0 = mult_r_1(inter_exc, InvNbPulsBlk);   /* b0 >> sh1 */
        L_acc = L_msu(L_c, b0, b0);             /* (c - b0**2) >> 2sh1-1 */
        L_acc = L_negate(L_acc);                /* delta x 2**(-2sh1+1) */

        /* Case delta <= 0 */
        if(L_acc <= 0) {  /* delta <= 0 */
            x1 = negate(b0);        /* sh1 */
        }

        /* Case delta > 0 */
        else {
            delta = Sqrt_lbc(L_acc);  /* >> sh1 */
            x1 = sub(delta, b0);      /* x1 >> sh1 */
            x2 = add(b0, delta);      /* (-x2) >> sh1 */
            if(abs_s(x2) < abs_s(x1)) {
                x1 = negate(x2);
            }
        }

        /* Update DataExc */
        sh1 = add(sh1, 1);
        temp = shl_1(x1, sh1);
        if(temp > (2*Gexc_Max)) temp = (2*Gexc_Max);
        if(temp < -(2*Gexc_Max)) temp = -(2*Gexc_Max);
        for(i=0; i<NbPulsBlk; i++) {
            j = *ptr_TabPos++;
            curExc[(emInt32 )j] = add(curExc[(emInt32 )j], mult_1(temp,
                                                (*ptr_TabSign++)) );
        }

        /* update PrevExc */
        ptr1 = PrevExc;
        for(i=SubFrLenD; i<PitchMax; i++)   *ptr1++ = PrevExc[i];
        for(i=0; i<SubFrLenD; i++)  *ptr1++ = curExc[i];

        curExc += SubFrLenD;
        i_subfr += 2;

    } /* end of loop on LTP blocks */

    return;
}

/*
**
** Function:           random_number()
**
** ÃèÊö:        returns a number randomly taken in [0, n]
**                     with np1 = n+1 at input
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 np1
**  Q15 emInt16 *nRandom    random generator status (input/output)
**
** Outputs:
**
**  Q15 emInt16 *nRandom
**
** ·µ»Ø value:       random number in [0, (np1-1)]
**
*/
Q15 emInt16 random_number(Q15 emInt16 np1, Q15 emInt16 *nRandom)
{
    Q15 emInt16 temp;

	LOG_StackAddr(__FUNCTION__);

    temp = Rand_lbc(nRandom) & (Q15 emInt16)0x7FFF;
    temp = mult(temp, np1);
    return(temp);
}

/*
**
** Function:           Qua_SidGain()
**
** ÃèÊö:        Quantization of Sid gain
**                     Pseudo-log quantizer in 3 segments
**                     1st segment : length = 16, resolution = 2
**                     2nd segment : length = 16, resolution = 4
**                     3rd segment : length = 32, resolution = 8
**                     quantizes a sum of energies
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 *Ener        table of the energies
**  Q15 emInt16 *shEner      corresponding scaling factors
**  Q15 emInt16 nq           if nq >= 1 : quantization of nq energies
**                      for SID gain calculation in function Cod_Cng()
**                      if nq = 0 : in function Comp_Info(),
**                      quantization of saved estimated excitation energy
**
** Outputs:             None
**
**
** ·µ»Ø value:       index of quantized energy
**
*/
Q15 emInt16 Qua_SidGain(Q15 emInt16 *Ener, Q15 emInt16 *shEner, Q15 emInt16 nq)
{
    Q15 emInt16 temp, iseg, iseg_p1;
    Q15 emInt16 j, j2, k, exp;
    Q31 emInt32 L_x, L_y;
    Q15 emInt16 sh1;
    Q31 emInt32 L_acc;
    emInt32  i;

	LOG_StackAddr(__FUNCTION__);

    if(nq == 0) {
         /* Quantize energy saved for frame erasure case                */
         /* L_x = 2 x average_ener                                      */
         temp = shl(*shEner, 1);
         temp = sub(16, temp);
         L_acc = L_deposit_l(*Ener);
         L_acc = L_shl(L_acc, temp); /* may overflow, and >> if temp < 0 */
         L_x = L_mls(L_acc, fact[0]);
    }

    else {

 /*
  * Compute weighted average of energies
  * Ener[i] = enerR[i] x 2**(shEner[i]-14)
  * L_x = k[nq] x SUM(i=0->nq-1) enerR[i]
  * with k[nq] =  2 x fact_mul x fact_mul / nq x Frame
  */
         sh1 = shEner[0];
         for(i=1; i<nq; i++) {
             if(shEner[i] < sh1) sh1 = shEner[i];
         }
         for(i=0, L_x=0L; i<nq; i++) {
             temp = sub(shEner[i], sh1);
             temp = shr_1(Ener[i], temp);
             temp = mult_r(fact[nq], temp);
             L_x = L_add(L_x, L_deposit_l(temp));
         }
         temp = sub(15, sh1);
         L_x = L_shl_1(L_x, temp);
    }

    /* Quantize L_x */
    if(L_x >= L_bseg[2]) return(63);

    /* Compute segment number iseg */
    if(L_x >= L_bseg[1]) {
        iseg = 2;
        exp = 4;
    }
    else {
        exp  = 3;
        if(L_x >= L_bseg[0]) iseg = 1;
        else iseg = 0;
    }

    iseg_p1 = add(iseg,1);
    j = shl(1, exp);
    k = shr(j,1);

    /* Binary search in segment iseg */
    for(i=0; i<exp; i++) {
        temp = add(base[iseg], shl(j, iseg_p1));
        L_y = L_mult(temp, temp);
        if(L_x >= L_y) j = add(j, k);
        else j = sub(j, k);
        k = shr(k, 1);
    }

    temp = add(base[iseg], shl(j, iseg_p1));
    L_y = L_mult(temp, temp);
    L_y = L_sub(L_y, L_x);
    if(L_y <= 0L) {
        j2    = add(j, 1);
        temp  = add(base[iseg], shl(j2, iseg_p1));
        L_acc = L_mult(temp, temp);
        L_acc = L_sub(L_x, L_acc);
        if(L_y > L_acc) temp = add(shl(iseg,4), j);
        else temp = add(shl(iseg,4), j2);
    }
    else {
        j2    = sub(j, 1);
        temp  = add(base[iseg], shl(j2, iseg_p1));
        L_acc = L_mult(temp, temp);
        L_acc = L_sub(L_x, L_acc);
        if(L_y < L_acc) temp = add(shl(iseg,4), j);
        else temp = add(shl(iseg,4), j2);
    }
    return(temp);
}

/*
**
** Function:           Dec_SidGain()
**
** ÃèÊö:        Decoding of quantized Sid gain
**                     (corresponding to sqrt of average energy)
**
** Links to text:
**
** Arguments:
**
**  Q15 emInt16 iGain        index of quantized Sid Gain
**
** Outputs:             None
**
** ·µ»Ø value:        decoded gain value << 5
**
*/
Q15 emInt16 Dec_SidGain(Q15 emInt16 iGain)
{
    Q15 emInt16 i, iseg;
    Q15 emInt16 temp;

	LOG_StackAddr(__FUNCTION__);

    iseg = shr(iGain, 4);
    if(iseg == 3) iseg = 2;
    i = sub(iGain, shl(iseg, 4));
    temp = add(iseg, 1);
    temp = shl(i, temp);
    temp = add(temp, base[iseg]);  /* SidGain */
    temp = shl(temp, 5); /* << 5 */
    return(temp);
}


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
