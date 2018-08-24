/*
    ITU-T G.723 Speech Coder   ANSI-C Source Code     Version 5.00
    copyright (c) 1995, AudioCodes, DSP Group, France Telecom,
    Universite de Sherbrooke.  All rights reserved.
*/

/*
**
** File:        "tab_lbc.h"
**
** √Ë ˆ:  This file contains emExtern declarations of the tables used
**               by the SG15 LBC Coder for 6.3/5.3 kbps.
**
*/

emExtern emConst Q15 emInt16   HammingWindowTable[LpcFrame];
emExtern emConst Q15 emInt16   BinomialWindowTable[LpcOrder] ;
emExtern emConst Q15 emInt16   BandExpTable[LpcOrder] ;
emExtern emConst Q15 emInt16   CosineTable[CosineTableSize] ;
emExtern emConst Q15 emInt16   LspDcTable[LpcOrder] ;
emExtern emConst Q15 emInt16   BandInfoTable[LspQntBands][2] ;
emExtern emConst Q15 emInt16   Band0Tb8[LspCbSize*3] ;
emExtern emConst Q15 emInt16   Band1Tb8[LspCbSize*3] ;
emExtern emConst Q15 emInt16   Band2Tb8[LspCbSize*4] ;
emExtern emConst Q15 emInt16 *emConst BandQntTable[LspQntBands] ;
emExtern emConst Q15 emInt16   PerFiltZeroTable[LpcOrder] ;
emExtern emConst Q15 emInt16   PerFiltPoleTable[LpcOrder] ;
emExtern emConst Q15 emInt16   PostFiltZeroTable[LpcOrder] ;
emExtern emConst Q15 emInt16   PostFiltPoleTable[LpcOrder];
emExtern emConst Q15 emInt16   Nb_puls[4];
emExtern emConst Q15 emInt16   FcbkGainTable[NumOfGainLev] ;
emExtern emConst Q31 emInt32   MaxPosTable[4] ;
emExtern emConst Q31 emInt32   CombinatorialTable[MaxPulseNum][SubFrLen/Sgrid] ;
emExtern emConst Q15 emInt16   AcbkGainTable085[85*20] ;
emExtern emConst Q15 emInt16   AcbkGainTable170[170*20] ;
emExtern emConst Q15 emInt16 *emConst AcbkGainTablePtr[2] ;
emExtern emConst Q15 emInt16   LpfConstTable[2] ;
emExtern emConst Q15 emInt16   epsi170[170] ;
emExtern emConst Q15 emInt16   gain170[170] ;
emExtern emConst Q15 emInt16   tabgain170[170];
emExtern emConst Q15 emInt16   tabgain85[85];
emExtern emConst Q15 emInt16   fact[4];
emExtern emConst Q31 emInt32   L_bseg[3];
emExtern emConst Q15 emInt16   base[3];
