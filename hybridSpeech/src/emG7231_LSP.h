/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "lsp.h"
**
** √Ë ˆ:     Function prototypes for "lpc.c"
**  
*/

emExtern	void	AtoLsp( Q15 emInt16 *LspVect, Q15 emInt16 *Lpc, Q15 emInt16 *PrevLsp );
emExtern	Q31 emInt32	Lsp_Qnt( Q15 emInt16 *CurrLsp, Q15 emInt16 *PrevLsp );
emExtern	Q31 emInt32	Lsp_Svq( Q15 emInt16 *Tv, Q15 emInt16 *Wvect );
emExtern	void	Lsp_Inq( Q15 emInt16 *Lsp, Q15 emInt16 *PrevLsp, Q31 emInt32 LspId, Q15 emInt16 Crc );
emExtern	void	Lsp_Int( Q15 emInt16 *QntLpc, Q15 emInt16 *CurrLsp, Q15 emInt16 *PrevLsp );
emExtern	void	Lsp_Int_Dec( Q15 emInt16 *QntLpc, Q15 emInt16 *CurrLsp, Q15 emInt16 *PrevLsp );
emExtern	void	LsptoA( Q15 emInt16 *Lsp );
emExtern	void	LsptoA_1( Q15 emInt16 *Lsp );
