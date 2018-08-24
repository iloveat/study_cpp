/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "util_cng.h"
**
** √Ë ˆ:     Function prototypes for "util_cng.c"
**
*/

emExtern	void	Calc_Exc_Rand(NEED_THIS Q15 emInt16 cur_gain, Q15 emInt16 *PrevExc, Q15 emInt16 *DataExc, Q15 emInt16 *nRandom, LINEDEF *Line);
emExtern	Q15 emInt16	Qua_SidGain(Q15 emInt16 *Ener, Q15 emInt16 *shEner, Q15 emInt16 nq);
emExtern	Q15 emInt16	Dec_SidGain(Q15 emInt16 i_gain);
