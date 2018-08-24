#include "emPCH.h"
#include "emDecode.h"



/* ��ǰ�����֡��ü��� */
#define G7231_START_FRAMES		(200)

#define  Encode_len				240
#define  Decode_le				24

#define	L_Encode				40		//40��emCharA���ݣ���80�ֽڣ�
#define	L_Decode				20		//ѹ����40��emCharA���ݣ�����ѹ��֮

#define EM_ACPCM_FRAME_CODES    40

#if EM_USER_DECODER_ADPCM

//��ADPCM�������Ƶ�Ƚ����ٲ���
//ÿ20���ֽڽ�ѹ��80�ֽ�
emTTSErrID emCall DecodePromptPreRecordAdpcmAndPlay(
	emInt16 nParaSample,					//������
	emInt32 nTableOffset)					//��������ƫ����
{
	


	emInt32 i,j,t1,nLenOfPrompt, nReadLen;
	FILE			*fCurFile;
	emInt32  nPcmBaseOffset;
	emInt32  nMaxReadSizeAccordSample;	
	emPointer pParameter;



	emPADPCMDecoder pDecoder;	/* ���������� */
	emPUInt8  pSample;	/* PCM ������ */
	emInt32  nb_frame;

	LOG_StackAddr(__FUNCTION__);

	pParameter = g_hTTS->pCBParam ;	

	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //hyl  2012-04-06
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;


	nb_frame=0;

	/* ��ʼ�������� */
#if DEBUG_LOG_SWITCH_HEAP
	pDecoder = (emPADPCMDecoder)emHeap_AllocZero(sizeof(struct tagADPCMDecoder), "ADPcm��������������ģ��  ��");
#else
	pDecoder = (emPADPCMDecoder)emHeap_AllocZero(sizeof(struct tagADPCMDecoder));
#endif
	
	/* �������PCM���ݻ����� */
#if DEBUG_LOG_SWITCH_HEAP
	pSample = (emPUInt8)emHeap_AllocZero(sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES, "ADPcm����PCM����������ģ��  ��");
#else
	pSample = (emPUInt8)emHeap_AllocZero(sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES);
#endif
	
	emADPCM_InitDecoder(pDecoder);


	t1 = ( g_hTTS->m_nCurPointOfPcmInfo - g_hTTS->m_structPromptInfo );
	for( i=0; i< t1; i++ )
	{	
		fCurFile = g_hTTS->fResFrontMain;
		nPcmBaseOffset = nTableOffset;

		fFrontSeek(fCurFile, ( nPcmBaseOffset + g_hTTS->m_structPromptInfo[i].nPromptOffset ) , 0);
		nLenOfPrompt = g_hTTS->m_structPromptInfo[i].nPromptLen;

		while(  nLenOfPrompt > 0)
		{	

			if( nParaSample != g_hTTS->pRearG->nModelSample)	//��ʾ����������ģ�͵Ĳ����ʲ���ȣ�������ز���
			{
				
				if( nParaSample == 8000 &&  g_hTTS->pRearG->nModelSample == 16000)			//���8K�ز�����16K�ģ�����򵥵Ĳ����㷨
				{
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE/2) ;
					nReadLen = 0;
					while(nLenOfPrompt >= L_Decode )    
					{
							fFrontRead(pSample, sizeof(emCharA), L_Decode, fCurFile) ;

							emADPCM_Decode(pDecoder, pSample, L_Decode, g_hTTS->m_pPCMBuffer + nReadLen);	//ADPCM����
							nReadLen += L_Encode*2;
							if( (nReadLen + L_Encode*2) > nMaxReadSizeAccordSample)
							{
								for(j=(nReadLen-2);j>=0;j=j-2)					//8K�����ʵ���Ƶת��16K�����ʵ���Ƶ
								{
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
								}
								ChangePcmVol(VOICE_TYPE_PROMPT, g_hTTS->m_pPCMBuffer , nReadLen*2 , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
								g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //�ص�--������������HMM�ϳɵģ�
								nReadLen = 0;
							}

							nLenOfPrompt = nLenOfPrompt - L_Decode;
					}
					if( nReadLen>0)
					{
						for(j=(nReadLen-2);j>=0;j=j-2)					//8K�����ʵ���Ƶת��16K�����ʵ���Ƶ
						{
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
						}
						ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
						g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
					}					
				}
				else				//�����ģ��ø��ӵ��ز����㷨
				{


					
				}

			}
			else		//��ʾ����������ģ�͵Ĳ�������ȣ������ز�����ֱ�����
			{
				nMaxReadSizeAccordSample = MAX_OUTPUT_PCM_SIZE;
				nReadLen = 0;
				while(nLenOfPrompt >= L_Decode )    
				{
						fFrontRead(pSample, sizeof(emCharA), L_Decode, fCurFile);

						emADPCM_Decode(pDecoder, pSample, L_Decode, g_hTTS->m_pPCMBuffer + nReadLen);	//ADPCM����
						nReadLen += L_Encode*2;
						if( (nReadLen + L_Encode*2) > nMaxReadSizeAccordSample)
						{
							ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
							g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
							nReadLen = 0;
						}

						nLenOfPrompt = nLenOfPrompt - L_Decode;
				}
				if( nReadLen>0)
				{
					ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
					g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
				}
				
			}			
			
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				break;
			}
		}	
		if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
		{
			break;
		}
	}	

	/* �ͷŽ��뻺���� */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pSample, sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES, "ADPcm����PCM����������ģ��  ��");
#else
	emHeap_Free( pSample, sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES);
#endif

	/* �ͷŽ����� */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pDecoder, sizeof(struct tagADPCMDecoder), "ADPcm��������������ģ��  ��");
#else
	emHeap_Free( pDecoder, sizeof(struct tagADPCMDecoder));
#endif


	return g_hTTS->m_nErrorID;
}

#endif



//��G7231�������Ƶ�Ƚ����ٲ���
//ÿ24���ֽڽ�ѹ��480�ֽ�
emTTSErrID emCall DecodePromptPreRecordG7231AndPlay(
	emInt16 nParaSample,					//������
	emInt32 nTableOffset)					//��������ƫ����
{

	emInt32 i,j,t1,nLenOfPrompt, nReadLen;
	FILE			*fCurFile;
	emInt32  nPcmBaseOffset;
	emInt32  nMaxReadSizeAccordSample;	
	emPByte pDecoder;	/* ���������� */
	emPUInt8 pSample;	/* PCM ������ */
	emInt32  nb_frame;

	emPointer pParameter;

	LOG_StackAddr(__FUNCTION__);

	pParameter = g_hTTS->pCBParam ;	
	
	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //hyl  2012-04-06
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;

	
	/* ��ʼ�������� */
#if DEBUG_LOG_SWITCH_HEAP
	pDecoder = (emPByte)emHeap_AllocZero(EM_DECODER_G7231R_INSTANCE_SIZE , "G7231��������������ģ��  ��");
#else
	pDecoder = (emPByte)emHeap_AllocZero(EM_DECODER_G7231R_INSTANCE_SIZE);
#endif

	/* ������뻺���� */
#if DEBUG_LOG_SWITCH_HEAP
	pSample = (emPUInt8)emHeap_AllocZero(sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE, "G7231����PCM����������ģ��  ��");
#else
	pSample = (emPUInt8)emHeap_AllocZero(sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE);
#endif

	emG7231_InitDecoder(pDecoder, nParaSample);
	nb_frame=0;


	t1 = ( g_hTTS->m_nCurPointOfPcmInfo - g_hTTS->m_structPromptInfo );
	for( i=0; i< t1; i++ )
	{	
		fCurFile = g_hTTS->fResFrontMain;
		nPcmBaseOffset = nTableOffset;

		fFrontSeek(fCurFile, ( nPcmBaseOffset + g_hTTS->m_structPromptInfo[i].nPromptOffset ) , 0);
		nLenOfPrompt = g_hTTS->m_structPromptInfo[i].nPromptLen;

		while(  nLenOfPrompt > 0)
		{	

			//��ʾ����������ģ�͵Ĳ����ʲ���ȣ�������ز���
			if( g_hTTS->pRearG->nModelSample != 11000 		//Ŀǰ���ԣ���ʱ����ģ��Ϊ11Kʱ����ʾ�����ز������Ժ���Ҫɾ��
				&& nParaSample != g_hTTS->pRearG->nModelSample )
			{
				
				if( nParaSample == 8000 &&  g_hTTS->pRearG->nModelSample == 16000)			//���8K�ز�����16K�ģ�����򵥵Ĳ����㷨
				{
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE/2) ;
					nReadLen = 0;
					while(nLenOfPrompt >= Decode_le )    //
					{
							fFrontRead(pSample, sizeof(emCharA), Decode_le, fCurFile);
							emG7231_Decode(pDecoder, pSample, (emPInt16)(g_hTTS->m_pPCMBuffer + nReadLen));	//G7231����
							nReadLen += Encode_len*2;
							if( (nReadLen + Encode_len*2) > nMaxReadSizeAccordSample)
							{
								for(j=(nReadLen-2);j>=0;j=j-2)					//8K�����ʵ���Ƶת��16K�����ʵ���Ƶ
								{
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
								}
								ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen*2 , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
								g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //�ص�--������������HMM�ϳɵģ�
								nReadLen = 0;
							}

							nLenOfPrompt = nLenOfPrompt - Decode_le;
					}
					if( nReadLen>0)
					{
						for(j=(nReadLen-2);j>=0;j=j-2)					//8K�����ʵ���Ƶת��16K�����ʵ���Ƶ
						{
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
						}
						ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
						g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //�ص�--������������HMM�ϳɵģ�
					}					
				}


///////////////////////////////////////////////////////////////////////////////////////
				if( nParaSample == 16000 &&  g_hTTS->pRearG->nModelSample == 8000)			//���16K�ز�����8K�ģ�����򵥵Ĳ����㷨
				{
					
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE) ;
					nReadLen = 0;
					while(nLenOfPrompt >= Decode_le )    //
					{
							fFrontRead(pSample, sizeof(emCharA), Decode_le, fCurFile);

							emG7231_Decode(pDecoder, pSample, (emPInt16)(g_hTTS->m_pPCMBuffer + nReadLen));	//G7231����
							nReadLen += Encode_len*2;
							if( (nReadLen + Encode_len*2) > nMaxReadSizeAccordSample)
							{
								for(j=4;j<=nReadLen;j=j+4)					//16K�����ʵ���Ƶת��8K�����ʵ���Ƶ
								{
									emMemCpy(g_hTTS->m_pPCMBuffer+j/2,g_hTTS->m_pPCMBuffer+j,2);
								}
								ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
								g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen/2);  //�ص�--������������HMM�ϳɵģ�
								nReadLen = 0;
							}

							nLenOfPrompt = nLenOfPrompt - Decode_le;
					}
					if( nReadLen>0)
					{
						for(j=2;j<=nReadLen;j=j+2)					//16K�����ʵ���Ƶת��8K�����ʵ���Ƶ
						{
							emMemCpy(g_hTTS->m_pPCMBuffer+j/2,g_hTTS->m_pPCMBuffer+j,2);
						}
						ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
						g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen/2);  //�ص�--������������HMM�ϳɵģ�
					}		
									
				}


				//�����ģ��ø��ӵ��ز����㷨
				{
					
				}

			}
			else		//��ʾ����������ģ�͵Ĳ�������ȣ������ز�����ֱ�����
			{
				nMaxReadSizeAccordSample = MAX_OUTPUT_PCM_SIZE;
				nReadLen = 0;
				while(nLenOfPrompt >= Decode_le )    //
				{
						fFrontRead(pSample, sizeof(emCharA), Decode_le, fCurFile);
						emG7231_Decode(pDecoder, pSample, (emPInt16)(g_hTTS->m_pPCMBuffer + nReadLen) );	//G7231����
						nReadLen += Encode_len*2;
						if( (nReadLen + Encode_len*2) > nMaxReadSizeAccordSample)
						{
							ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
							g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
							nReadLen = 0;
						}
						nLenOfPrompt = nLenOfPrompt - Decode_le;
				}
				if( nReadLen>0)
				{
					ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		
					g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
				}
				
			}			
			
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				break;
			}
		}	
		if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
		{
			break;
		}
	}	

	/* �ͷŽ��뻺���� */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pSample,sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE, "G7231����PCM����������ģ��  ��");
#else
	emHeap_Free(pSample,sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE);
#endif

	/* �ͷŽ����� */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pDecoder, EM_DECODER_G7231R_INSTANCE_SIZE , "G7231��������������ģ��  ��");
#else
	emHeap_Free(pDecoder, EM_DECODER_G7231R_INSTANCE_SIZE);
#endif


	return  g_hTTS->m_nErrorID;
}


//��PCM�������Ƶֱ�Ӳ���
emTTSErrID emCall DecodePromptPreRecordPcmAndPlay(
	emInt16 nParaSample,					//������
	emInt32 nTableOffset)					//��������ƫ����
{

	emInt32 i,j,t1,nLenOfPrompt, nReadLen;
	FILE			*fCurFile;
	emInt32  nPcmBaseOffset;
	emInt32  nMaxReadSizeAccordSample;	

	emPointer pParameter;

	LOG_StackAddr(__FUNCTION__);

	pParameter = g_hTTS->pCBParam ;	
	
	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //hyl  2012-04-06
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;

	t1 = ( g_hTTS->m_nCurPointOfPcmInfo - g_hTTS->m_structPromptInfo );
	for( i=0; i< t1; i++ )
	{	
		fCurFile = g_hTTS->fResFrontMain;
		nPcmBaseOffset = nTableOffset;

		fFrontSeek(fCurFile, ( nPcmBaseOffset + g_hTTS->m_structPromptInfo[i].nPromptOffset ) , 0);
		nLenOfPrompt = g_hTTS->m_structPromptInfo[i].nPromptLen;

		while(  nLenOfPrompt > 0)
		{	

			if( nParaSample != g_hTTS->pRearG->nModelSample)	//��ʾ����������ģ�͵Ĳ����ʲ���ȣ�������ز���
			{
				
				if( nParaSample == 8000 &&  g_hTTS->pRearG->nModelSample == 16000)			//���8K�ز�����16K�ģ�����򵥵Ĳ����㷨
				{
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE/2) ;
					if( nLenOfPrompt > nMaxReadSizeAccordSample )
					{
						nReadLen = nMaxReadSizeAccordSample;
						nLenOfPrompt -= nReadLen;					
					}
					else
					{
						nReadLen = ( (emInt32)( nLenOfPrompt/2) )*2;
						nLenOfPrompt = 0 ; 
					}
							
					emMemSet(g_hTTS->m_pPCMBuffer,0,MAX_OUTPUT_PCM_SIZE);
					fFrontRead(g_hTTS->m_pPCMBuffer, sizeof(emCharA), nReadLen, fCurFile ); 	
					
					for(j=(nReadLen-2);j>=0;j=j-2)					//8K�����ʵ���Ƶת��16K�����ʵ���Ƶ
					{
						emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
						emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
					}
					ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen*2 , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������	
					
					g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //�ص�--������������HMM�ϳɵģ�
				}
				else				//�����ģ��ø��ӵ��ز����㷨
				{
										

					
				}

			}
			else		//��ʾ����������ģ�͵Ĳ�������ȣ������ز�����ֱ�����
			{
				nMaxReadSizeAccordSample = MAX_OUTPUT_PCM_SIZE;
				if( nLenOfPrompt > nMaxReadSizeAccordSample )
				{
					nReadLen = nMaxReadSizeAccordSample;
					nLenOfPrompt -= nReadLen;					
				}
				else
				{
					nReadLen = ( (emInt32)( nLenOfPrompt/2) )*2;
					nLenOfPrompt = 0 ; 
				}
						
				emMemSet(g_hTTS->m_pPCMBuffer,0,MAX_OUTPUT_PCM_SIZE);
				fFrontRead(g_hTTS->m_pPCMBuffer, sizeof(emCharA), nReadLen, fCurFile ); 	
				ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������		


				g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
			}

			
			
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				return emTTS_ERR_EXIT;
			}
		}			
	}	



	return emTTS_ERR_OK;


}







