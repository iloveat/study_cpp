/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "coder.h"
**
** √Ë ˆ:     Function prototypes and external declarations 
**          for "coder.c"
**  
*/

emExtern	void    Init_Coder( NEED_THIS0 );
emExtern	emBool    Coder( NEED_THIS Q15 emInt16 *DataBuff, emPUInt8 Vout );
