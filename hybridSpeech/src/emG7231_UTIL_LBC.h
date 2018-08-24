/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "util_lbc.h"
**
** √Ë ˆ:     Function prototypes for "util_lbc.c"
**  
*/

#ifdef FILE_IO



emExtern	void    Read_lbc( Q15 emPInt16 Dpnt, emInt32  Len, FILE emPtr Fp );
emExtern	void    Write_lbc( Q15 emPInt16 Dpnt, emInt32  Len, FILE emPtr Fp );

emExtern	void    Line_Wr( emStrA, FILE emPtr ) ;
emExtern	emInt32      Line_Rd( emStrA, FILE emPtr ) ;

#endif/*FILE_IO*/

emExtern	void    Rem_Dc( NEED_THIS Q15 emPInt16 Dpnt );
emExtern	Q15 emInt16  Vec_Norm( Q15 emPInt16 Vect, Q15 emInt16 Len );
emExtern	void    Mem_Shift( Q15 emPInt16 PrevDat, Q15 emPInt16 DataBuff );
emExtern	void    Line_Pack( NEED_THIS LINEDEF emPtr Line, emPUInt8 Vout, Q15 emInt16 VadBit );
emExtern	Q15 emInt16* Par2Ser( Q31 emInt32 Inp, Q15 emPInt16 Pnt, emInt32  BitNum );
emExtern	LINEDEF Line_Unpk( NEED_THIS emPUInt8 Vinp, Q15 emPInt16 VadType );
emExtern	Q31 emInt32  Ser2Par( Q15 emPInt16 emPtr Pnt, emInt32  Count );
emExtern	Q31 emInt32  Comp_En( Q15 emPInt16 Dpnt );
emExtern	Q15 emInt16  Sqrt_lbc( Q31 emInt32 Num );
emExtern	Q15 emInt16  Rand_lbc( Q15 emPInt16 p );
emExtern	void    Scale( NEED_THIS Q15 emPInt16 Tv, Q31 emInt32 Sen );
