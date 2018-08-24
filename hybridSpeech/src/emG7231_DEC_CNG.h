/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "dec_cng.h"
**
** √Ë ˆ:     Function prototypes for "dec_cng.c"
**
*/

emExtern	void	Init_Dec_Cng( NEED_THIS0 );
emExtern	void	Dec_Cng( NEED_THIS Q15 emInt16 Ftyp, LINEDEF *Line, Q15 emInt16 *DataExc, Q15 emInt16 *QntLpc );
