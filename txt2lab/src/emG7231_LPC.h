/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "lpc.h"
**
** √Ë ˆ:     Function prototypes for "lpc.c"
**  
*/

emExtern	void    Comp_Lpc( NEED_THIS Q15 emInt16 *UnqLpc, Q15 emInt16 *PrevDat, Q15 emInt16 *DataBuff );
emExtern	Q15 emInt16  Durbin( Q15 emInt16 *Lpc, Q15 emInt16 *Corr, Q15 emInt16 Err, Q15 emInt16 *Pk2 );
emExtern	void    Wght_Lpc( Q15 emInt16 *PerLpc, Q15 emInt16 *UnqLpc );
emExtern	void    Error_Wght( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *PerLpc );
emExtern	void    Comp_Ir( Q15 emInt16 *ImpResp, Q15 emInt16 *QntLpc, Q15 emInt16 *PerLpc, PWDEF Pw );
emExtern	void    Sub_Ring( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *QntLpc, Q15 emInt16 *PerLpc, Q15 emInt16 *PrevErr, PWDEF Pw );
emExtern	void    Upd_Ring( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *QntLpc, Q15 emInt16 *PerLpc, Q15 emInt16 *PrevErr );

#if 0
	emExtern	void    Synt( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *Lpc);
#else
	emExtern	void    Synt( Q15 emInt16 *Dpnt, Q15 emInt16 *Lpc);
#endif

#if 0
	emExtern	Q31 emInt32  Spf( NEED_THIS Q15 emInt16 *Tv, Q15 emInt16 *Lpc );
#endif
