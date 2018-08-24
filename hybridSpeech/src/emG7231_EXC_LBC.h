/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "exc_lbc.h"
**
** √Ë ˆ:     Function prototypes for "exc_lbc.c"
**  
*/

emExtern	Q15 emInt16	Estim_Pitch( Q15 emInt16 *Dpnt, Q15 emInt16 Start );
emExtern	PWDEF	Comp_Pw( Q15 emInt16 *Dpnt, Q15 emInt16 Start, Q15 emInt16 Olp );
emExtern	void	Filt_Pw( Q15 emInt16 *DataBuff, Q15 emInt16 *Dpnt, Q15 emInt16 Start, PWDEF Pw );
emExtern	void	Find_Fcbk( NEED_THIS Q15 emInt16 *Dpnt, Q15 emInt16 *ImpResp, LINEDEF *Line, Q15 emInt16 Sfc );
emExtern	void	Gen_Trn( Q15 emInt16 *Dst, Q15 emInt16 *Src, Q15 emInt16 Olp );
emExtern	void	Find_Best( BESTDEF *Best, Q15 emInt16 *Tv, Q15 emInt16 *ImpResp, Q15 emInt16 Np, Q15 emInt16 Olp );
emExtern	void	Fcbk_Pack( Q15 emInt16 *Dpnt, SFSDEF *Sfs, BESTDEF *Best, Q15 emInt16 Np );
emExtern	void	Fcbk_Unpk( NEED_THIS Q15 emInt16 *Tv, SFSDEF Sfs, Q15 emInt16 Olp, Q15 emInt16 Sfc );
emExtern	void	Find_Acbk( NEED_THIS Q15 emInt16 *Tv, Q15 emInt16 *ImpResp, Q15 emInt16 *PrevExc, LINEDEF *Line, Q15 emInt16 Sfc );
emExtern	void	Get_Rez( Q15 emInt16 *Tv, Q15 emInt16 *PrevExc, Q15 emInt16 Lag );
emExtern	void	Decod_Acbk( NEED_THIS Q15 emInt16 *Tv, Q15 emInt16 *PrevExc, Q15 emInt16 Olp, Q15 emInt16 Lid, Q15 emInt16 Gid );
emExtern	Q15 emInt16	Comp_Info( Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 *Gain, Q15 emInt16 *ShGain);
emExtern	void	Regen( Q15 emInt16 *DataBuff, Q15 emInt16 *Buff, Q15 emInt16 Lag, Q15 emInt16 Gain, Q15 emInt16 Ecount, Q15 emInt16 *Sd );
emExtern	PFDEF	Comp_Lpf( NEED_THIS Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 Sfc );
emExtern	Q15 emInt16	Find_B( Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 Sfc );
emExtern	Q15 emInt16	Find_F( Q15 emInt16 *Buff, Q15 emInt16 Olp, Q15 emInt16 Sfc );
emExtern	PFDEF	Get_Ind( NEED_THIS Q15 emInt16 Ind, Q15 emInt16 Ten, Q15 emInt16 Ccr, Q15 emInt16 Enr );
emExtern	void	Filt_Lpf( Q15 emInt16 *Tv, Q15 emInt16 *Buff, PFDEF Pf, Q15 emInt16 Sfc );
emExtern	void	reset_max_time(void);
emExtern	Q15 emInt16	search_T0 ( Q15 emInt16 T0, Q15 emInt16 Gid, Q15 emInt16 *gain_T0);
emExtern	Q15 emInt16	ACELP_LBC_code(Q15 emInt16 X[], Q15 emInt16 h[], Q15 emInt16 T0, Q15 emInt16 code[], Q15 emInt16 *gain, Q15 emInt16 *shift, Q15 emInt16 *sign, Q15 emInt16 gain_T0);
emExtern	void	Cor_h(Q15 emInt16 *H, Q15 emInt16 *rr);
emExtern	void	Cor_h_X(Q15 emInt16 h[], Q15 emInt16 X[], Q15 emInt16 D[]);
emExtern	Q15 emInt16	D4i64_LBC(Q15 emInt16 Dn[], Q15 emInt16 rr[], Q15 emInt16 h[], Q15 emInt16 cod[], Q15 emInt16 y[], Q15 emInt16 *code_shift, Q15 emInt16 *sign);
emExtern	Q15 emInt16	G_code(Q15 emInt16 X[], Q15 emInt16 Y[], Q15 emInt16 *gain_q);
