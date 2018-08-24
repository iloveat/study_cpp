/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "cod_cng.h"
**
** √Ë ˆ:     Function prototypes for "cod_cng.c"
**
*/

emExtern	void	Init_Cod_Cng( NEED_THIS0 );
emExtern	void	Cod_Cng( NEED_THIS Q15 emInt16 *DataExc, Q15 emInt16 *Ftyp, LINEDEF *Line, Q15 emInt16 *QntLpc );
emExtern	void	Update_Acf( NEED_THIS Q15 emInt16 *Acfsf, Q15 emInt16 *Shsf );
