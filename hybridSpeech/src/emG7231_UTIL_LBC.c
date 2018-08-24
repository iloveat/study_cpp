/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:    util_lbc.c
**
** 描述: utility functions for the lbc codec
**
** Functions:
**
**  I/O functions:
**
**      Read_lbc()
**      Write_lbc()
**
**  High-pass filtering:
**
**      Rem_Dc()
**
**  Miscellaneous signal processing functions:
**
**      Vec_Norm()
**      Mem_Shift()
**      Comp_En()
**      Scale()
**
**  Bit stream packing/unpacking:
**
**      Line_Pack()
**      Line_Unpk()
**
**  Mathematical functions:
**
**      Sqrt_lbc()
**      Rand_lbc()
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


#ifdef FILE_IO





/*
**
** Function:        Read_lbc()
**
** 描述:     Read in a file
**
** Links to text:   Sections 2.2 & 4
**
** Arguments:
**
**  Q15 emPInt16 Dpnt
**  emInt32      Len
**  FILE emPtr Fp
**
** Outputs:
**
**  Q15 emPInt16 Dpnt
**
** 返回 value:    None
**
*/

void  Read_lbc( Q15 emPInt16 Dpnt, emInt32  Len, FILE emPtr Fp )
{
    emInt32    i  ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < Len ; i ++ )
        Dpnt[i] = (Q15 emInt16) 0 ;

    fFrontRead ( (emStrA)Dpnt, sizeof(Q15 emInt16), Len, Fp ) ;

    return;
}

/*
**
** Function:        Write_lbc()
**
** 描述:     Write a file
**
** Links to text:   Section
**
** Arguments:
**
**  Q15 emPInt16 Dpnt
**  emInt32      Len
**  FILE emPtr Fp
**
** Outputs:         None
**
** 返回 value:    None
**
*/
void    Write_lbc( Q15 emPInt16 Dpnt, emInt32  Len, FILE emPtr Fp )
{

	LOG_StackAddr(__FUNCTION__);

    fwrite( (emStrA)Dpnt, sizeof(Q15 emInt16), Len, Fp ) ;
}

void    Line_Wr( emStrA Line, FILE emPtr Fp )
{
    Q15 emInt16  Info ;
    emInt32      Size   ;

	LOG_StackAddr(__FUNCTION__);

    Info = Line[0] & (Q15 emInt16)0x0003 ;

    /* Check frame type and rate informations */
    switch (Info) {

        case 0x0002 : {   /* SID frame */
            Size  = 4;
            break;
        }

        case 0x0003 : {  /* untransmitted silence frame */
            Size  = 1;
            break;
        }

        case 0x0001 : {   /* active frame, low rate */
            Size  = 20;
            break;
        }

        default : {  /* active frame, high rate */
            Size  = 24;
        }
    }
    fwrite( Line, Size , 1, Fp ) ;
}

emInt32  Line_Rd( emStrA Line, FILE emPtr Fp )
{
    Q15 emInt16  Info ;
    emInt32      Size   ;

	LOG_StackAddr(__FUNCTION__);

    if(fFrontRead( Line, 1,1, Fp ) != 1) return(-1);

    Info = Line[0] & (Q15 emInt16)0x0003 ;

    /* Check frame type and rate informations */
    switch(Info) {

        /* Active frame, high rate */
        case 0 : {
            Size  = 23;
            break;
        }

        /* Active frame, low rate */
        case 1 : {
            Size  = 19;
            break;
        }

        /* Sid Frame */
        case 2 : {
            Size  = 3;
            break;
        }

        /* untransmitted */
        default : {
            return(0);
        }
    }
    fFrontRead( &Line[1], Size , 1, Fp ) ;
    return(0);
}


#endif /* FILE_IO */


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Rem_Dc()
**
** 描述:     High-pass filtering
**
** Links to text:   Section 2.3
**
** Arguments:
**
**  Q15 emIntP16 Dpnt
**
** Inputs:
**
**  CodStat.hpfZdl  FIR filter memory from previous frame (1 word)
**  CodStat.hpfPdl  IIR filter memory from previous frame (1 word)
**
** Outputs:
**
**  Q15 emPInt16 Dpnt
**
** 返回 value:    None
**
*/
void  Rem_Dc( NEED_THIS Q15 emPInt16 Dpnt )
{
    emInt32    i  ;


    Q31 emInt32   Acc0,Acc1 ;

	LOG_StackAddr(__FUNCTION__);

    if ( UseHp ) {
        for ( i = 0 ; i < Frame ; i ++ ) {

            /* Do the Fir and scale by 2 */
            Acc0 = L_mult( Dpnt[i], (Q15 emInt16) 0x4000 ) ;
            Acc0 = L_mac ( Acc0, CodStat.HpfZdl, (Q15 emInt16) 0xc000 ) ;
            CodStat.HpfZdl = Dpnt[i] ;

            /* Do the Iir part */
            Acc1 = L_mls( CodStat.HpfZdl, (Q15 emInt16) 0x7f00 ) ;
            Acc0 = L_add( Acc0, Acc1 ) ;
            CodStat.HpfZdl = (Q15 emInt16) Acc0 ;
            Dpnt[i] = round(Acc0) ;
        }
    }
    else {
        for ( i = 0 ; i < Frame ; i ++ )
            Dpnt[i] = shr( Dpnt[i], (Q15 emInt16) 1 ) ;
    }

    return;
}

/*
**
** Function:        Vec_Norm()
**
** 描述:     Vector normalization
**
** Links to text:
**
** Arguments:
**
**  Q15 emPInt16 Vect
**  Q15 emInt16 Len
**
** Outputs:
**
**  Q15 emPInt16 Vect
**
** 返回 value:  The power of 2 by which the data vector multiplyed.
**
*/
Q15 emInt16  Vec_Norm( Q15 emPInt16 Vect, Q15 emInt16 Len )
{
    emInt32    i  ;

    Q15 emInt16  Acc0,Acc1   ;
    Q15 emInt16  Exp   ;
    Q15 emInt16  Rez ;
    Q31 emInt32  Temp  ;

    emStatic emConst Q15 emInt16 ShiftTable[16] = {
      0x0001 ,
      0x0002 ,
      0x0004 ,
      0x0008 ,
      0x0010 ,
      0x0020 ,
      0x0040 ,
      0x0080 ,
      0x0100 ,
      0x0200 ,
      0x0400 ,
      0x0800 ,
      0x1000 ,
      0x2000 ,
      0x4000 ,
      0x7fff
   } ;

	LOG_StackAddr(__FUNCTION__);

    /* Find absolute maximum */
    Acc1 = (Q15 emInt16) 0 ;
    for ( i = 0 ; i < Len ; i ++ ) {
        Acc0 = abs_s( Vect[i] ) ;
        if ( Acc0 > Acc1 )
            Acc1 = Acc0 ;
    }

    /* Get the shift count */
    Rez = norm_s( Acc1 ) ;
    Exp = ShiftTable[Rez] ;

    /* Normalize all the vector */
    for ( i = 0 ; i < Len ; i ++ ) {
        Temp = L_mult( Exp, Vect[i] ) ;
        Temp = L_shr_1( Temp, 4 ) ;
        Vect[i] = extract_l( Temp ) ;
    }

    Rez = sub( Rez, (Q15 emInt16) 3) ;
    return Rez ;
}

/*
**
** Function:        Mem_Shift()
**
** 描述:     Memory shift, update of the high-passed input speech signal
**
** Links to text:
**
** Arguments:
**
**  Q15 emPInt16 PrevDat
**  Q15 emPInt16 DataBuff
**
** Outputs:
**
**  Q15 emPInt16 PrevDat
**  Q15 emPInt16 DataBuff
**
** 返回 value:    None
**
*/
void  Mem_Shift( Q15 emPInt16 PrevDat, Q15 emPInt16 DataBuff )
{
    emInt32    i  ;

    Q15 emInt16   Dpnt[Frame+LpcFrame-SubFrLen] ;

	LOG_StackAddr(__FUNCTION__);

    /*  Form Buffer  */
    for ( i = 0 ; i < LpcFrame-SubFrLen ; i ++ )
        Dpnt[i] = PrevDat[i] ;
    for ( i = 0 ; i < Frame ; i ++ )
        Dpnt[i+LpcFrame-SubFrLen] = DataBuff[i] ;

    /* Update PrevDat */
    for ( i = 0 ; i < LpcFrame-SubFrLen ; i ++ )
        PrevDat[i] = Dpnt[Frame+i] ;

    /* Update DataBuff */
    for ( i = 0 ; i < Frame ; i ++ )
        DataBuff[i] = Dpnt[(LpcFrame-SubFrLen)/2+i] ;

    return;
}

/*
**
** Function:        Line_Pack()
**
** 描述:     Packing coded parameters in bitstream of 16-bit words
**
** Links to text:   Section 4
**
** Arguments:
**
**  LINEDEF emPtr Line     Coded parameters for a frame
**  emStrA    Vout     bitstream chars
**  Q15 emInt16   VadBit   Voice Activity Indicator
**
** Outputs:
**
**  Q15 emPInt16 Vout
**
** 返回 value:    None
**
*/
void    Line_Pack( NEED_THIS LINEDEF emPtr Line, emPUInt8 Vout, Q15 emInt16 Ftyp )
{
    emInt32      i ;
    emInt32      BitCount ;

    Q15 emInt16  BitStream[192] ;
    Q15 emPInt16 Bsp = BitStream ;
    Q31 emInt32  Temp ;

	LOG_StackAddr(__FUNCTION__);

    /* Clear the output vector */
    for ( i = 0 ; i < 24 ; i ++ )
        Vout[i] = 0 ;

 /*
  * Add the coder rate info and frame type info to the 2 msb
  * of the first word of the frame.
  * The signaling is as follows:
  *     Ftyp  WrkRate => X1X0
  *       1     Rate63     00  :   High Rate
  *       1     Rate53     01  :   Low  Rate
  *       2       x        10  :   Silence Insertion Descriptor frame
  *       0       x        11  :   Used only for simulation of
  *                                 untransmitted silence frames
  */
    switch (Ftyp) {

        case 0 : {
            Temp = 0x00000003L;
            break;
        }

        case 2 : {
            Temp = 0x00000002L;
            break;
        }

        default : {
            if ( WrkRate == Rate63 )
                Temp = 0x00000000L ;
            else
                Temp = 0x00000001L ;
            break;
        }
    }

    /* Serialize Control info */
    Bsp = Par2Ser( Temp, Bsp, 2 ) ;


    /* Check for Speech/NonSpeech case */
    if ( Ftyp == 1 ) {

    /* 24 bit LspId */
    Temp = (*Line).LspId ;
    Bsp = Par2Ser( Temp, Bsp, 24 ) ;

 /*
  * Do the part common to both rates
  */

        /* Adaptive code book lags */
        Temp = (Q31 emInt32) (*Line).Olp[0] - (Q31 emInt32) PitchMin ;
        Bsp = Par2Ser( Temp, Bsp, 7 ) ;

        Temp = (Q31 emInt32) (*Line).Sfs[1].AcLg ;
        Bsp = Par2Ser( Temp, Bsp, 2 ) ;

        Temp = (Q31 emInt32) (*Line).Olp[1] - (Q31 emInt32) PitchMin ;
        Bsp = Par2Ser( Temp, Bsp, 7 ) ;

        Temp = (Q31 emInt32) (*Line).Sfs[3].AcLg ;
        Bsp = Par2Ser( Temp, Bsp, 2 ) ;

        /* Write combined 12 bit index of all the gains */
        for ( i = 0 ; i < SubFrames ; i ++ ) {
            Temp = (*Line).Sfs[i].AcGn*NumOfGainLev + (*Line).Sfs[i].Mamp ;
            if ( WrkRate == Rate63 )
                Temp += (Q31 emInt32) (*Line).Sfs[i].Tran << 11 ;
            Bsp = Par2Ser( Temp, Bsp, 12 ) ;
        }

        /* Write all the Grid indices */
        for ( i = 0 ; i < SubFrames ; i ++ )
            *Bsp ++ = (*Line).Sfs[i].Grid ;

        /* High rate only part */
        if ( WrkRate == Rate63 ) {

            /* Write the reserved bit as 0 */
            *Bsp ++ = (Q15 emInt16) 0 ;

            /* Write 13 bit combined position index */
            Temp = (*Line).Sfs[0].Ppos >> 16 ;
            Temp = Temp * 9 + ( (*Line).Sfs[1].Ppos >> 14) ;
            Temp *= 90 ;
            Temp += ((*Line).Sfs[2].Ppos >> 16) * 9 + ( (*Line).Sfs[3].Ppos >> 14 ) ;
            Bsp = Par2Ser( Temp, Bsp, 13 ) ;

            /* Write all the pulse positions */
            Temp = (*Line).Sfs[0].Ppos & 0x0000ffffL ;
            Bsp = Par2Ser( Temp, Bsp, 16 ) ;

            Temp = (*Line).Sfs[1].Ppos & 0x00003fffL ;
            Bsp = Par2Ser( Temp, Bsp, 14 ) ;

            Temp = (*Line).Sfs[2].Ppos & 0x0000ffffL ;
            Bsp = Par2Ser( Temp, Bsp, 16 ) ;

            Temp = (*Line).Sfs[3].Ppos & 0x00003fffL ;
            Bsp = Par2Ser( Temp, Bsp, 14 ) ;

            /* Write pulse amplitudes */
            Temp = (Q31 emInt32) (*Line).Sfs[0].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 6 ) ;

            Temp = (Q31 emInt32) (*Line).Sfs[1].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 5 ) ;

            Temp = (Q31 emInt32) (*Line).Sfs[2].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 6 ) ;

            Temp = (Q31 emInt32) (*Line).Sfs[3].Pamp ;
            Bsp = Par2Ser( Temp, Bsp, 5 ) ;
        }

        /* Low rate only part */
        else {

            /* Write 12 bits of positions */
            for ( i = 0 ; i < SubFrames ; i ++ ) {
                Temp = (*Line).Sfs[i].Ppos ;
                Bsp = Par2Ser( Temp, Bsp, 12 ) ;
            }

            /* Write 4 bit Pamps */
            for ( i = 0 ; i < SubFrames ; i ++ ) {
                Temp = (*Line).Sfs[i].Pamp ;
                Bsp = Par2Ser( Temp, Bsp, 4 ) ;
            }
        }

    }

    else if(Ftyp == 2) {   /* SID frame */

        /* 24 bit LspId */
        Temp = (*Line).LspId ;
        Bsp = Par2Ser( Temp, Bsp, 24 ) ;

        /* Do Sid frame gain */
        Temp = (Q31 emInt32)(*Line).Sfs[0].Mamp ;
        Bsp = Par2Ser( Temp, Bsp, 6 ) ;
    }

    /* Write out active frames */
    if ( Ftyp == 1 ) {
        if ( WrkRate == Rate63 )
            BitCount = 192 ;
        else
            BitCount = 160 ;
    }
    /* Non active frames */
    else if ( Ftyp == 2 )
        BitCount = 32 ;
    else
        BitCount = 2;

    for ( i = 0 ; i < BitCount ; i ++ )
        Vout[i>>3] ^= BitStream[i] << (i & 0x0007) ;

    return;
}

Q15 emInt16* Par2Ser( Q31 emInt32 Inp, Q15 emInt16 *Pnt, emInt32  BitNum )
{
    emInt32  i   ;
    Q15 emInt16  Temp ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < BitNum ; i ++ ) {
        Temp = (Q15 emInt16) Inp & (Q15 emInt16)0x0001 ;
        Inp >>= 1 ;
        *Pnt ++ = Temp ;
    }

    return Pnt ;
}


#endif /* EM_USER_ENCODER_G7231 */


#if EM_USER_DECODER_G7231


/*
**
** Function:        Line_Unpk()
**
** 描述:     unpacking of bitstream, gets coding parameters for a frame
**
** Links to text:   Section 4
**
** Arguments:
**
**  emPUInt8   Vinp        bitstream chars
**  Q15 emPInt16 VadType
**
** Outputs:
**
**  Q15 emPInt16 VadType
**
** 返回 value:
**
**  LINEDEF             coded parameters
**     Q15 emInt16   Crc
**     Q31 emInt32   LspId
**     Q15 emInt16   Olp[SubFrames/2]
**     SFSDEF   Sfs[SubFrames]
**
*/
LINEDEF  Line_Unpk( NEED_THIS emPUInt8 Vinp, Q15 emPInt16 Ftyp )
{
    emInt32    i  ;
    Q15 emInt16  BitStream[192] ;

    Q15 emPInt16 Bsp = BitStream ;
    volatile LINEDEF Line ;
    Q31 emInt32  Temp ;
    Q15 emInt16  Info;
    Q15 emInt16 Bound_AcGn;
	Q15 emInt16 FrType;

	LOG_StackAddr(__FUNCTION__);

	/* 检查码流率设定 */
	FrType = Vinp[0] & 0x03;
	if( FrType==0 && WrkRate==Rate53 )
	{/* 码流率设定错误修正设定 */
		WrkRate = Rate63;
	}
	if( FrType==1 && WrkRate==Rate63 )
	{/* 码流率设定错误修正设定 */
		WrkRate = Rate53;
	}

    if(WrkRate == Rate53)
	{
		
		Info = Vinp[0] & 0x03;
		if ( Info == 3 ) 
		{
			*Ftyp = 0;
			Line.LspId = 0L;    /* Dummy : to avoid Borland C3.1 warning */
			return Line;
		}
		
		/* Decode the LspId */
		Line.LspId =( ((Q31 emInt32)Vinp[0]) >> 2) + (((Q31 emInt32)Vinp[1]) << 6 );
		Line.LspId += ((Q31 emInt32)Vinp[2]) << 14;
		Line.LspId += ((Q31 emInt32)(Vinp[3]&0x03)) << 22;/*24 bits*/
		
		if ( Info == 2 ) 
		{
			/* Decode the Noise Gain */
			Line.Sfs[0].Mamp = Vinp[3] >> 2;
			*Ftyp = 2;
			return Line ;
		}
		
		/*
		 * Decode the common information to both rates
		 */
		*Ftyp = 1;
		
		/* Decode the adaptive codebook lags */
		Line.Olp[0] =( ((Q15 emInt16)Vinp[3]) >> 2 ) + (((Q15 emInt16) (Vinp[4]&0x01)) <<6 );/*7 bits*/
		Line.Olp[0]+=(Q15 emInt16) PitchMin;
		Line.Sfs[1].AcLg =((Q15 emInt16)(Vinp[4] & 0x06)) >> 1;/*2 bits*/
		Line.Olp[1] =( ((Q15 emInt16) Vinp[4]) >> 3) + (((Q15 emInt16) (Vinp[5] & 0x03)) << 5 );
		Line.Olp[1] += (Q15 emInt16) PitchMin ;/*7 bits*/
		Line.Sfs[3].AcLg = (((Q15 emInt16) Vinp[5]) >> 2) & 0x03;/*2 bits*/
		Line.Sfs[0].AcLg = 1 ;
		Line.Sfs[2].AcLg = 1 ;
		
		/* Decode the combined gains accordingly to the rate */
/*
		if(WrkRate == Rate53)
		{
*/
		Temp =(((Q31 emInt32)Vinp[5]) >> 4 ) + (((Q31 emInt32)Vinp[6]) <<4 );/*12 bits*/
		Line.Sfs[0].Tran = 0 ;
		Line.Sfs[0].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
		Line.Sfs[0].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
		Temp =(Q31 emInt32)Vinp[7] + (((Q31 emInt32)(Vinp[8] &0x0f)) <<8 );/*12 bits*/
		Line.Sfs[1].Tran = 0 ;
		Line.Sfs[1].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
		Line.Sfs[1].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
		Temp =( ((Q31 emInt32)Vinp[8]) >> 4 ) + (((Q31 emInt32) Vinp[9]) <<4 );/*12 bits*/
		Line.Sfs[2].Tran = 0 ;
		Line.Sfs[2].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
		Line.Sfs[2].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
		Temp =(Q31 emInt32) Vinp[10] + (((Q31 emInt32) (Vinp[11] &0x0f)) <<8 );/*12 bits*/
		Line.Sfs[3].Tran = 0 ;
		Line.Sfs[3].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
		Line.Sfs[3].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
/*
		}
		else
		{
			if(Line.Olp[0] < (SubFrLen-2))
			{
				Temp =(((Q31 emInt32)Vinp[5]) >> 4 ) + (((Q31 emInt32)Vinp[6]) <<4 );/ *12 bits* /
				Line.Sfs[0].Tran = (Q15 emInt16)(Temp >> 11) ;
				Temp &= 0x000007ffL ;
				Line.Sfs[0].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[0].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
				Temp =(Q31 emInt32)Vinp[7] + (((Q31 emInt32)(Vinp[8] &0x0f)) <<8 );/ *12 bits* /
				Line.Sfs[1].Tran = (Q15 emInt16)(Temp >> 11) ;
				Temp &= 0x000007ffL ;
				Line.Sfs[1].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[1].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
		
			}
			else
			{
				Temp =(((Q31 emInt32)Vinp[5]) >> 4 ) + (((Q31 emInt32)Vinp[6]) <<4 );/ *12 bits* /
				Line.Sfs[0].Tran = (Q15 emInt16)(Temp >> 11) ;
				Temp &= 0x000007ffL ;
				Line.Sfs[0].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[0].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
				Temp =(Q31 emInt32)Vinp[7] + (((Q31 emInt32)(Vinp[8] &0x0f)) <<8 );/ *12 bits* /
				Line.Sfs[1].Tran = (Q15 emInt16)(Temp >> 11) ;
				Temp &= 0x000007ffL ;
				Line.Sfs[1].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[1].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
			}
			if(Line.Olp[1] < (SubFrLen-2))
			{
				Temp =( ((Q31 emInt32)Vinp[8]) >> 4 ) + (((Q31 emInt32) Vinp[9]) <<4 );/ *12 bits* /
				Line.Sfs[2].Tran = 0 ;
				Line.Sfs[2].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[2].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
				Temp =(Q31 emInt32) Vinp[10] + (((Q31 emInt32) (Vinp[11] &0x0f)) <<8 );/ *12 bits* /
				Line.Sfs[3].Tran = 0 ;
				Line.Sfs[3].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[3].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
			}
			else
			{
				Temp =( ((Q31 emInt32)Vinp[8]) >> 4 ) + (((Q31 emInt32) Vinp[9]) <<4 );/ *12 bits* /
				Line.Sfs[2].Tran = 0 ;
				Line.Sfs[2].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[2].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
				Temp =(Q31 emInt32) Vinp[10] + (((Q31 emInt32) (Vinp[11] &0x0f)) <<8 );/ *12 bits* /
				Line.Sfs[3].Tran = 0 ;
				Line.Sfs[3].AcGn = (Q15 emInt16) (Temp / (Q15 emInt16)NumOfGainLev) ;
				Line.Sfs[3].Mamp = (Q15 emInt16) (Temp % (Q15 emInt16)NumOfGainLev) ;
			}
		
		
		
		
		
			for ( i = 0 ; i < SubFrames ; i ++ ) {
		
		        Temp = Ser2Par( &Bsp, 12 ) ;
		        Line.Sfs[i].Tran = 0 ;
		        if ( (WrkRate == Rate63) && (Line.Olp[i>>1] < (SubFrLen-2) ) ) {
		            Line.Sfs[i].Tran = (Q15 emInt16)(Temp >> 11) ;
		            Temp &= 0x000007ffL ;
		        }
		        Line.Sfs[i].AcGn = (Q15 emInt16)(Temp / (Q15 emInt16)NumOfGainLev) ;
		        Line.Sfs[i].Mamp = (Q15 emInt16)(Temp % (Q15 emInt16)NumOfGainLev) ;
		
		    }
*/
		
		
		/* Decode the grids */
		Line.Sfs[0].Grid=((Q15 emInt16)(Vinp[11] >> 4)) & 0x01;/*one bit*/
		Line.Sfs[1].Grid=((Q15 emInt16)(Vinp[11] >> 5)) & 0x01;/*one bit*/
		Line.Sfs[2].Grid=((Q15 emInt16)(Vinp[11] >> 6)) & 0x01;/*one bit*/
		Line.Sfs[3].Grid=((Q15 emInt16)(Vinp[11] >> 7)) & 0x01;/*one bit*/
/*
			for ( i = 0 ; i < SubFrames ; i ++ )
				Line.Sfs[i].Grid = *Bsp ++ ;
		 if (Info == 0) {  / *Rate63部分* /

				/ * Skip the reserved bit * /
				/ *Bsp ++ ;* /

				/ * Decode 13 bit combined position index * /
				/ *Temp = Ser2Par( &Bsp, 13 ) ;* /
				Temp = ((emUInt32)Vinp[12] >> 1) + (((emUInt32)Vinp[13] & 0x3f) << 7);
				Line.Sfs[0].Ppos = ( Temp/90 ) / 9 ;
				Line.Sfs[1].Ppos = ( Temp/90 ) % 9 ;
				Line.Sfs[2].Ppos = ( Temp%90 ) / 9 ;
				Line.Sfs[3].Ppos = ( Temp%90 ) % 9 ;

				/ * Decode all the pulse positions * /
				/ *Line.Sfs[0].Ppos = ( Line.Sfs[0].Ppos << 16 ) + Ser2Par( &Bsp, 16 ) ;* /
				Line.Sfs[0].Ppos = ( Line.Sfs[0].Ppos << 16 ) + (emUInt32)(Vinp[13] >> 6) + ((emUInt32)Vinp[14] << 2) + ((emUInt32)(Vinp[15] & 0x3f) << 10);
				/ *Line.Sfs[1].Ppos = ( Line.Sfs[1].Ppos << 14 ) + Ser2Par( &Bsp, 14 ) ;* /
				Line.Sfs[1].Ppos = ( Line.Sfs[1].Ppos << 14 ) + (emUInt32)(Vinp[15] >> 6) + ((emUInt32)Vinp[16] << 2) + ((emUInt32)(Vinp[17] & 0x0f) << 10) ;
				/ *Line.Sfs[2].Ppos = ( Line.Sfs[2].Ppos << 16 ) + Ser2Par( &Bsp, 16 ) ;* /
				Line.Sfs[2].Ppos = ( Line.Sfs[2].Ppos << 16 ) + (emUInt32)(Vinp[17] >> 4) + ((emUInt32)Vinp[18] << 4) + ((emUInt32)(Vinp[19] & 0x0f) << 12);
				/ *Line.Sfs[3].Ppos = ( Line.Sfs[3].Ppos << 14 ) + Ser2Par( &Bsp, 14 ) ;* /
				Line.Sfs[3].Ppos = ( Line.Sfs[3].Ppos << 14 ) + (emUInt32)(Vinp[19] >> 4) + ((emUInt32)Vinp[20] << 4) + ((emUInt32)(Vinp[21] & 0x03) << 12);

				/ * Decode pulse amplitudes * /
				/ *Line.Sfs[0].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 6 ) ;* /
				Line.Sfs[0].Pamp = (emUInt16)(Vinp[21] >> 2);
				/ *Line.Sfs[1].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 5 ) ;* /
				Line.Sfs[1].Pamp = ((emUInt16)Vinp[22] & 0x1f);
				/ *Line.Sfs[2].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 6 ) ;* /
				Line.Sfs[2].Pamp = ((emUInt16)Vinp[22] >> 5) + (((emUInt16)Vinp[23] & 0x07) << 3);
				/ *Line.Sfs[3].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 5 ) ;* /
				Line.Sfs[3].Pamp = (emUInt16)Vinp[23] >> 3;
			}

			else {
*/
		
		
        /* Decode the positions */
		Line.Sfs[0].Ppos=(Q31 emInt32)Vinp[12] + (((Q31 emInt32)(Vinp[13]&0x0f)) <<8 );/*12 bits*/
		Line.Sfs[1].Ppos=( ((Q31 emInt32)Vinp[13]) >>4 ) + (((Q31 emInt32)Vinp[14]) <<4 );/*12 bits*/
		Line.Sfs[2].Ppos=(Q31 emInt32)Vinp[15] + (((Q31 emInt32)(Vinp[16]&0x0f)) <<8 );/*12 bits*/
		Line.Sfs[3].Ppos=( ((Q31 emInt32)Vinp[16]) >>4 ) + (((Q31 emInt32)Vinp[17]) <<4 );/*12 bits*/
/*
		for ( i = 0 ; i < SubFrames ; i ++ )
			Line.Sfs[i].Ppos = Ser2Par( &Bsp, 12 ) ;
*/
		
        /* Decode the amplitudes */
		Line.Sfs[0].Pamp=((Q15 emInt16)Vinp[18]) & 0x0f;/*4 bits*/
		Line.Sfs[1].Pamp=((Q15 emInt16)Vinp[18]) >> 4;/*4 bits*/
		Line.Sfs[2].Pamp=((Q15 emInt16)Vinp[19]) & 0x0f;/*4 bits*/
		Line.Sfs[3].Pamp=((Q15 emInt16)Vinp[19]) >> 4;/*4 bits*/
/*
        for ( i = 0 ; i < SubFrames ; i ++ )
            Line.Sfs[i].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 4 ) ;
    }
*/
	}
	else
	{
		
		/*Line.Crc = Crc;*/
		/*if(Crc != 0) return Line;*/
		
		/* Unpack the byte info to BitStream vector */
		for ( i = 0 ; i < 192 ; i ++ )
			BitStream[i] = ( Vinp[i>>3] >> (i & (Q15 emInt16)0x0007) ) & (Q15 emInt16)1 ;
		
		/* Decode the frame type and rate info */
		Info = (Q15 emInt16)Ser2Par( &Bsp, 2 ) ;
		
		if ( Info == 3 ) {
			*Ftyp = 0;
			Line.LspId = 0L;    /* Dummy : to avoid Borland C3.1 warning */
			return Line;
		}
		
		/* Decode the LspId */
		Line.LspId = Ser2Par( &Bsp, 24 ) ;
		
		if ( Info == 2 ) {
			/* Decode the Noise Gain */
			Line.Sfs[0].Mamp = (Q15 emInt16)Ser2Par( &Bsp, 6);
			*Ftyp = 2;
			return Line ;
		}
		
		/*
		* Decode the common information to both rates
		*/
		*Ftyp = 1;
		
		/* Decode the bit-rate */
		WrkRate = (Info == 0) ? Rate63 : Rate53;
		
		/* Decode the adaptive codebook lags */
		Temp = Ser2Par( &Bsp, 7 ) ;
		/* Test if forbidden code */
		if( Temp <= 123) {
			Line.Olp[0] = (Q15 emInt16) Temp + (Q15 emInt16)PitchMin ;
		}
		else {
			/* transmission error */
			Line.Crc = 1;
			return Line ;
		}
		
		Line.Sfs[1].AcLg = (Q15 emInt16) Ser2Par( &Bsp, 2 ) ;
		
		Temp = Ser2Par( &Bsp, 7 ) ;
		/* Test if forbidden code */
		if( Temp <= 123) {
			Line.Olp[1] = (Q15 emInt16) Temp + (Q15 emInt16)PitchMin ;
		}
		else {
			/* transmission error */
			Line.Crc = 1;
			return Line ;
		}
		
		Line.Sfs[3].AcLg = (Q15 emInt16) Ser2Par( &Bsp, 2 ) ;
		
		Line.Sfs[0].AcLg = 1 ;
		Line.Sfs[2].AcLg = 1 ;
		
		/* Decode the combined gains accordingly to the rate */
		for ( i = 0 ; i < SubFrames ; i ++ ) {
			
			Temp = Ser2Par( &Bsp, 12 ) ;
			
			Line.Sfs[i].Tran = 0 ;
			Bound_AcGn = NbFilt170 ;
			if ( (WrkRate == Rate63) && (Line.Olp[i>>1] < (SubFrLen-2) ) ) {
				Line.Sfs[i].Tran = (Q15 emInt16)(Temp >> 11) ;
				Temp &= 0x000007ffL ;
				Bound_AcGn = NbFilt085 ;
			}
			Line.Sfs[i].AcGn = (Q15 emInt16)(Temp / (Q15 emInt16)NumOfGainLev) ;
			if(Line.Sfs[i].AcGn < Bound_AcGn ) {
				Line.Sfs[i].Mamp = (Q15 emInt16)(Temp % (Q15 emInt16)NumOfGainLev) ;
			}
			else {
				/* error detected */
				Line.Crc = 1;
				return Line ;
			}
		}
		
		/* Decode the grids */
		for ( i = 0 ; i < SubFrames ; i ++ )
			Line.Sfs[i].Grid = *Bsp ++ ;
		
		if (Info == 0) {
			
			/* Skip the reserved bit */
			Bsp ++ ;
			
			/* Decode 13 bit combined position index */
			Temp = Ser2Par( &Bsp, 13 ) ;
			Line.Sfs[0].Ppos = ( Temp/90 ) / 9 ;
			Line.Sfs[1].Ppos = ( Temp/90 ) % 9 ;
			Line.Sfs[2].Ppos = ( Temp%90 ) / 9 ;
			Line.Sfs[3].Ppos = ( Temp%90 ) % 9 ;
			
			/* Decode all the pulse positions */
			Line.Sfs[0].Ppos = ( Line.Sfs[0].Ppos << 16 ) + Ser2Par( &Bsp, 16 ) ;
			Line.Sfs[1].Ppos = ( Line.Sfs[1].Ppos << 14 ) + Ser2Par( &Bsp, 14 ) ;
			Line.Sfs[2].Ppos = ( Line.Sfs[2].Ppos << 16 ) + Ser2Par( &Bsp, 16 ) ;
			Line.Sfs[3].Ppos = ( Line.Sfs[3].Ppos << 14 ) + Ser2Par( &Bsp, 14 ) ;
			
			/* Decode pulse amplitudes */
			Line.Sfs[0].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 6 ) ;
			Line.Sfs[1].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 5 ) ;
			Line.Sfs[2].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 6 ) ;
			Line.Sfs[3].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 5 ) ;
		}
		
		else {
			
			/* Decode the positions */
			for ( i = 0 ; i < SubFrames ; i ++ )
				Line.Sfs[i].Ppos = Ser2Par( &Bsp, 12 ) ;
			
			/* Decode the amplitudes */
			for ( i = 0 ; i < SubFrames ; i ++ )
				Line.Sfs[i].Pamp = (Q15 emInt16)Ser2Par( &Bsp, 4 ) ;
		}
	}
   return Line ;
}

Q31 emInt32  Ser2Par( Q15 emPInt16 emPtr Pnt, emInt32  Count )
{
    emInt32      i ;
    Q31 emInt32  Rez = 0L ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < Count ; i ++ ) {
        Rez += (Q31 emInt32) **Pnt << i ;
        (*Pnt) ++ ;
    }
    return Rez ;
}


#endif /* EM_USER_DECODER_G7231 */


#if EM_USER_ENCODER_G7231


/*
**
** Function:        Comp_En()
**
** 描述:     Compute energy of a subframe vector
**
** Links to text:
**
** Arguments:
**
**  Q15 emPInt16 Dpnt
**
** Outputs:         None
**
** 返回 value:
**
**      Q31 emInt32 energy
**
*/
Q31 emInt32   Comp_En( Q15 emPInt16 Dpnt )
{
    emInt32    i ;
    Q31 emInt32   Rez ;
    Q15 emInt16   Temp[SubFrLen] ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < SubFrLen ; i ++ )
        Temp[i] = shr_1( Dpnt[i], (Q15 emInt16) 2 ) ;

    Rez = (Q31 emInt32) 0 ;
    for ( i = 0 ; i < SubFrLen ; i ++ )
        Rez = L_mac( Rez, Temp[i], Temp[i] ) ;

    return Rez ;
}


#endif /* EM_USER_ENCODER_G7231 */


/*
**
** Function:        Sqrt_lbc()
**
** 描述:     Square root computation
**
** Links to text:
**
** Arguments:
**
**  Q31 emInt32 Num
**
** Outputs:     None
**
** 返回 value:
**
**  Q15 emInt16 square root of num
**
*/
Q15 emInt16   Sqrt_lbc( Q31 emInt32 Num )
{
    emInt32    i  ;

    Q15 emInt16   Rez = (Q15 emInt16) 0 ;
    Q15 emInt16   Exp = (Q15 emInt16) 0x4000 ;

    Q31 emInt32   Acc ;

	LOG_StackAddr(__FUNCTION__);

    for ( i = 0 ; i < 14 ; i ++ ) {

        Acc = L_mult( add(Rez, Exp), add(Rez, Exp) ) ;
        if ( Num >= Acc )
            Rez = add( Rez, Exp ) ;

        Exp = shr( Exp, (Q15 emInt16) 1 ) ;
    }
    return Rez ;
}

/*
**
** Function:        Rand_lbc()
**
** 描述:     Generator of random numbers
**
** Links to text:   Section 3.10.2
**
** Arguments:
**
**  Q15 emInt16 *p
**
** Outputs:
**
**  Q15 emPInt16 p
**
** 返回 value:
**
**  Q15 emInt16 random number
**
*/
Q15 emInt16   Rand_lbc( Q15 emPInt16 p )
{
    Q31 emInt32   Temp ;

	LOG_StackAddr(__FUNCTION__);

    Temp = L_deposit_l( *p ) ;
    Temp &= (Q31 emInt32) 0x0000ffff ;
    Temp = Temp*(Q31 emInt32)521 + (Q31 emInt32) 259 ;
    *p = extract_l( Temp ) ;
    return extract_l( Temp ) ;
}


#if 0 /* 优化掉 */

#if EM_USER_DECODER_G7231


/*
**
** Function:        Scale()
**
** 描述:     Postfilter gain scaling
**
** Links to text:   Section 3.9
**
** Arguments:
**
**  Q15 emPInt16 Tv
**  Q31 emInt32 Sen
**
**  Inputs:
**
**  Q15 emInt16 DecStat.Gain
**
** Outputs:
**
**  Q15 emPInt16 Tv
**
** 返回 value:    None
**
*/
void  Scale( NEED_THIS Q15 emPInt16 Tv, Q31 emInt32 Sen )
{
    emInt32    i ;

    Q31 emInt32   Acc0,Acc1   ;
    Q15 emInt16   Exp,SfGain  ;

	LOG_StackAddr(__FUNCTION__);


    Acc0 = Sen ;
    Acc1 = Comp_En( Tv ) ;

    /* Normalize both */
    if ( (Acc1 != (Q31 emInt32) 0) && (Acc0 != (Q31 emInt32) 0 ) ) {

        Exp = norm_l( Acc1 ) ;
        Acc1 = L_shl( Acc1, Exp ) ;

        SfGain = norm_l( Acc0 ) ;
        Acc0 = L_shl( Acc0, SfGain ) ;
        Acc0 = L_shr( Acc0, (Q15 emInt16) 1 ) ;
        Exp = sub( Exp, SfGain ) ;
        Exp = add( Exp, (Q15 emInt16) 1 ) ;
        Exp = sub( (Q15 emInt16) 6, Exp ) ;
        if ( Exp < (Q15 emInt16) 0 )
            Exp = (Q15 emInt16) 0 ;

        SfGain = extract_h( Acc1 ) ;

        SfGain = div_l( Acc0, SfGain ) ;

        Acc0 = L_deposit_h( SfGain ) ;

        Acc0 = L_shr( Acc0, Exp ) ;

        SfGain = Sqrt_lbc( Acc0 ) ;
    }
    else
        SfGain = 0x1000 ;

    /* Filter the data */
    for ( i = 0 ; i < SubFrLen ; i ++ ) {

        /* Update gain */
        Acc0 = L_deposit_h( DecStat.Gain ) ;
        Acc0 = L_msu( Acc0, DecStat.Gain, (Q15 emInt16) 0x0800 ) ;
        Acc0 = L_mac( Acc0, SfGain, (Q15 emInt16) 0x0800 ) ;
        DecStat.Gain = round( Acc0 ) ;

        Exp = add( DecStat.Gain, shr( DecStat.Gain, (Q15 emInt16) 4) ) ;

        Acc0 = L_mult( Tv[i], Exp ) ;
        Acc0 = L_shl_1( Acc0, (Q15 emInt16) 4 ) ;
        Tv[i] = round_1( Acc0 ) ;
    }

    return;
}


#endif /* EM_USER_DECODER_G7231 */


#endif


#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
