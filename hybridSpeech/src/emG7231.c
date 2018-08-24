/* G.723.1 编解码定点算法源文件 */

#include "emPCH.h"
#include "emCommon.h"


#if EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231



#include "emG7231_TYPEDEF.h"
#include "emG7231_COD_CNG.h"
#include "emG7231_CODER.h"
#include "emG7231_VAD.h"
#include "emG7231_DEC_CNG.h"
#include "emG7231_DECOD.h"
#include "emG7231.h"


#if EM_USER_ENCODER_G7231


void emCall emG7231_InitCoder( emPointer pInst, emBool bRate5 )
{
#ifdef EM_MT

	PCodec pThis;
	emAssert(EM_G7231_CODER_INSTANCE_SIZE >= sizeof(CCoder));
	emAssert(pInst);
	pThis = (PCodec)pInst;

#endif/*EM_MT*/

	LOG_StackAddr(__FUNCTION__);


	WrkRate =  Rate63;
	UseHp = emFalse;
	UseVx = emTrue;

	Init_Coder( USE_THIS0 );
	Init_Vad( USE_THIS0 );
	Init_Cod_Cng( USE_THIS0 );
}

emUInt8 emCall emG7231_Encode( emPointer pInst, emPInt16 pPcmIn, emPUInt8 pCodeOut )
{
#ifdef EM_MT

	PCodec pThis;
	emAssert(pInst && pPcmIn && pCodeOut);
	pThis = (PCodec)pInst;

#endif/*EM_MT*/

	LOG_StackAddr(__FUNCTION__);

	if ( Coder( USE_THIS pPcmIn, pCodeOut ) )
	{
		/* Check frame type and rate informations */
		switch ( 3 & *pCodeOut )
		{
		case 0:	return 24;	/* active frame, high rate */
		case 1:	return 20;	/* active frame, low rate */
		case 2:	return 4;	/* SID frame */
		case 3:	return 1;	/* untransmitted silence frame */
		}
	}

	return 0;
}


#endif /* EM_USER_ENCODER_G7231 */


#if EM_USER_DECODER_G7231


void emCall emG7231_InitDecoder( emPointer pInst, emBool bRate5 )
{
#ifdef EM_MT

	PCodec pThis;
	emAssert(EM_DECODER_G7231R_INSTANCE_SIZE >= sizeof(CDecoder));
	emAssert(pInst);
	pThis = (PCodec)pInst;

#endif/*EM_MT*/

	LOG_StackAddr(__FUNCTION__);

	WrkRate =  Rate63;
	UsePf = emFalse; /* 默认值为不使用高通滤波器 */

	Init_Decod( USE_THIS0 );
	Init_Dec_Cng( USE_THIS0 );
}

emUInt8 emCall emG7231_Decode( emPointer pInst, emPUInt8 pCodeIn, emPInt16 pPcmOut )
{
	emInt16 pBuffer[Frame + LpcOrder];
	emInt32  i, nRet;

#ifdef EM_MT

	PCodec pThis;
	emAssert(pInst && pCodeIn && pPcmOut);
	pThis = (PCodec)pInst;

#endif/*EM_MT*/

	LOG_StackAddr(__FUNCTION__);

	emMemCpy(pBuffer, DecStat.SyntIirDl, sizeof(Q15 emInt16) * LpcOrder);
	nRet = Decod( USE_THIS pBuffer + LpcOrder, pCodeIn );

	for ( i = 0; i < Frame; ++ i )
		pPcmOut[i] = pBuffer[LpcOrder + i] << 1;

	emMemCpy(DecStat.SyntIirDl, pBuffer + Frame, sizeof(Q15 emInt16) * LpcOrder);
	return nRet;
}


#endif /* EM_USER_DECODER_G7231 */



#endif /* EM_USER_ENCODER_G7231 || EM_USER_DECODER_G7231 */
