#include "emPCH.h"
#include "emDecode.h"



/* 提前解多少帧获得激励 */
#define G7231_START_FRAMES		(200)

#define  Encode_len				240
#define  Decode_le				24

#define	L_Encode				40		//40个emCharA数据（即80字节）
#define	L_Decode				20		//压缩成40个emCharA数据，，解压反之

#define EM_ACPCM_FRAME_CODES    40

#if EM_USER_DECODER_ADPCM

//对ADPCM编码的音频先解码再播放
//每20个字节解压成80字节
emTTSErrID emCall DecodePromptPreRecordAdpcmAndPlay(
	emInt16 nParaSample,					//采样率
	emInt32 nTableOffset)					//需查音表的偏移量
{
	


	emInt32 i,j,t1,nLenOfPrompt, nReadLen;
	FILE			*fCurFile;
	emInt32  nPcmBaseOffset;
	emInt32  nMaxReadSizeAccordSample;	
	emPointer pParameter;



	emPADPCMDecoder pDecoder;	/* 解码器对象 */
	emPUInt8  pSample;	/* PCM 缓冲区 */
	emInt32  nb_frame;

	LOG_StackAddr(__FUNCTION__);

	pParameter = g_hTTS->pCBParam ;	

	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //hyl  2012-04-06
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;


	nb_frame=0;

	/* 初始化解码器 */
#if DEBUG_LOG_SWITCH_HEAP
	pDecoder = (emPADPCMDecoder)emHeap_AllocZero(sizeof(struct tagADPCMDecoder), "ADPcm解码器：《解码模块  》");
#else
	pDecoder = (emPADPCMDecoder)emHeap_AllocZero(sizeof(struct tagADPCMDecoder));
#endif
	
	/* 分配解码PCM数据缓冲区 */
#if DEBUG_LOG_SWITCH_HEAP
	pSample = (emPUInt8)emHeap_AllocZero(sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES, "ADPcm解码PCM区：《解码模块  》");
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

			if( nParaSample != g_hTTS->pRearG->nModelSample)	//提示音与后端声音模型的采样率不相等，需进行重采样
			{
				
				if( nParaSample == 8000 &&  g_hTTS->pRearG->nModelSample == 16000)			//针对8K重采样成16K的：用最简单的采样算法
				{
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE/2) ;
					nReadLen = 0;
					while(nLenOfPrompt >= L_Decode )    
					{
							fFrontRead(pSample, sizeof(emCharA), L_Decode, fCurFile) ;

							emADPCM_Decode(pDecoder, pSample, L_Decode, g_hTTS->m_pPCMBuffer + nReadLen);	//ADPCM解码
							nReadLen += L_Encode*2;
							if( (nReadLen + L_Encode*2) > nMaxReadSizeAccordSample)
							{
								for(j=(nReadLen-2);j>=0;j=j-2)					//8K采样率的音频转成16K采样率的音频
								{
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
								}
								ChangePcmVol(VOICE_TYPE_PROMPT, g_hTTS->m_pPCMBuffer , nReadLen*2 , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
								g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //回调--播放声音（非HMM合成的）
								nReadLen = 0;
							}

							nLenOfPrompt = nLenOfPrompt - L_Decode;
					}
					if( nReadLen>0)
					{
						for(j=(nReadLen-2);j>=0;j=j-2)					//8K采样率的音频转成16K采样率的音频
						{
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
						}
						ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
						g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
					}					
				}
				else				//其它的：用复杂的重采样算法
				{


					
				}

			}
			else		//提示音与后端声音模型的采样率相等，无需重采样，直接输出
			{
				nMaxReadSizeAccordSample = MAX_OUTPUT_PCM_SIZE;
				nReadLen = 0;
				while(nLenOfPrompt >= L_Decode )    
				{
						fFrontRead(pSample, sizeof(emCharA), L_Decode, fCurFile);

						emADPCM_Decode(pDecoder, pSample, L_Decode, g_hTTS->m_pPCMBuffer + nReadLen);	//ADPCM解码
						nReadLen += L_Encode*2;
						if( (nReadLen + L_Encode*2) > nMaxReadSizeAccordSample)
						{
							ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
							g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
							nReadLen = 0;
						}

						nLenOfPrompt = nLenOfPrompt - L_Decode;
				}
				if( nReadLen>0)
				{
					ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
					g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
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

	/* 释放解码缓冲区 */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pSample, sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES, "ADPcm解码PCM区：《解码模块  》");
#else
	emHeap_Free( pSample, sizeof(emPUInt8) * EM_ACPCM_FRAME_CODES);
#endif

	/* 释放解码器 */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pDecoder, sizeof(struct tagADPCMDecoder), "ADPcm解码器：《解码模块  》");
#else
	emHeap_Free( pDecoder, sizeof(struct tagADPCMDecoder));
#endif


	return g_hTTS->m_nErrorID;
}

#endif



//对G7231编码的音频先解码再播放
//每24个字节解压成480字节
emTTSErrID emCall DecodePromptPreRecordG7231AndPlay(
	emInt16 nParaSample,					//采样率
	emInt32 nTableOffset)					//需查音表的偏移量
{

	emInt32 i,j,t1,nLenOfPrompt, nReadLen;
	FILE			*fCurFile;
	emInt32  nPcmBaseOffset;
	emInt32  nMaxReadSizeAccordSample;	
	emPByte pDecoder;	/* 编码器对象 */
	emPUInt8 pSample;	/* PCM 缓冲区 */
	emInt32  nb_frame;

	emPointer pParameter;

	LOG_StackAddr(__FUNCTION__);

	pParameter = g_hTTS->pCBParam ;	
	
	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //hyl  2012-04-06
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;

	
	/* 初始化解码器 */
#if DEBUG_LOG_SWITCH_HEAP
	pDecoder = (emPByte)emHeap_AllocZero(EM_DECODER_G7231R_INSTANCE_SIZE , "G7231解码器：《解码模块  》");
#else
	pDecoder = (emPByte)emHeap_AllocZero(EM_DECODER_G7231R_INSTANCE_SIZE);
#endif

	/* 分配解码缓冲区 */
#if DEBUG_LOG_SWITCH_HEAP
	pSample = (emPUInt8)emHeap_AllocZero(sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE, "G7231解码PCM区：《解码模块  》");
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

			//提示音与后端声音模型的采样率不相等，需进行重采样
			if( g_hTTS->pRearG->nModelSample != 11000 		//目前测试：暂时：当模型为11K时，提示音不重采样，以后本行要删除
				&& nParaSample != g_hTTS->pRearG->nModelSample )
			{
				
				if( nParaSample == 8000 &&  g_hTTS->pRearG->nModelSample == 16000)			//针对8K重采样成16K的：用最简单的采样算法
				{
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE/2) ;
					nReadLen = 0;
					while(nLenOfPrompt >= Decode_le )    //
					{
							fFrontRead(pSample, sizeof(emCharA), Decode_le, fCurFile);
							emG7231_Decode(pDecoder, pSample, (emPInt16)(g_hTTS->m_pPCMBuffer + nReadLen));	//G7231解码
							nReadLen += Encode_len*2;
							if( (nReadLen + Encode_len*2) > nMaxReadSizeAccordSample)
							{
								for(j=(nReadLen-2);j>=0;j=j-2)					//8K采样率的音频转成16K采样率的音频
								{
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
									emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
								}
								ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen*2 , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
								g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //回调--播放声音（非HMM合成的）
								nReadLen = 0;
							}

							nLenOfPrompt = nLenOfPrompt - Decode_le;
					}
					if( nReadLen>0)
					{
						for(j=(nReadLen-2);j>=0;j=j-2)					//8K采样率的音频转成16K采样率的音频
						{
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
							emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
						}
						ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
						g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //回调--播放声音（非HMM合成的）
					}					
				}


///////////////////////////////////////////////////////////////////////////////////////
				if( nParaSample == 16000 &&  g_hTTS->pRearG->nModelSample == 8000)			//针对16K重采样成8K的：用最简单的采样算法
				{
					
					nMaxReadSizeAccordSample = (emInt32)( MAX_OUTPUT_PCM_SIZE) ;
					nReadLen = 0;
					while(nLenOfPrompt >= Decode_le )    //
					{
							fFrontRead(pSample, sizeof(emCharA), Decode_le, fCurFile);

							emG7231_Decode(pDecoder, pSample, (emPInt16)(g_hTTS->m_pPCMBuffer + nReadLen));	//G7231解码
							nReadLen += Encode_len*2;
							if( (nReadLen + Encode_len*2) > nMaxReadSizeAccordSample)
							{
								for(j=4;j<=nReadLen;j=j+4)					//16K采样率的音频转成8K采样率的音频
								{
									emMemCpy(g_hTTS->m_pPCMBuffer+j/2,g_hTTS->m_pPCMBuffer+j,2);
								}
								ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
								g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen/2);  //回调--播放声音（非HMM合成的）
								nReadLen = 0;
							}

							nLenOfPrompt = nLenOfPrompt - Decode_le;
					}
					if( nReadLen>0)
					{
						for(j=2;j<=nReadLen;j=j+2)					//16K采样率的音频转成8K采样率的音频
						{
							emMemCpy(g_hTTS->m_pPCMBuffer+j/2,g_hTTS->m_pPCMBuffer+j,2);
						}
						ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
						g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen/2);  //回调--播放声音（非HMM合成的）
					}		
									
				}


				//其它的：用复杂的重采样算法
				{
					
				}

			}
			else		//提示音与后端声音模型的采样率相等，无需重采样，直接输出
			{
				nMaxReadSizeAccordSample = MAX_OUTPUT_PCM_SIZE;
				nReadLen = 0;
				while(nLenOfPrompt >= Decode_le )    //
				{
						fFrontRead(pSample, sizeof(emCharA), Decode_le, fCurFile);
						emG7231_Decode(pDecoder, pSample, (emPInt16)(g_hTTS->m_pPCMBuffer + nReadLen) );	//G7231解码
						nReadLen += Encode_len*2;
						if( (nReadLen + Encode_len*2) > nMaxReadSizeAccordSample)
						{
							ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
							g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
							nReadLen = 0;
						}
						nLenOfPrompt = nLenOfPrompt - Decode_le;
				}
				if( nReadLen>0)
				{
					ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		
					g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
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

	/* 释放解码缓冲区 */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pSample,sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE, "G7231解码PCM区：《解码模块  》");
#else
	emHeap_Free(pSample,sizeof(emPUInt8) * EM_G7231_MAX_CODE_SIZE);
#endif

	/* 释放解码器 */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pDecoder, EM_DECODER_G7231R_INSTANCE_SIZE , "G7231解码器：《解码模块  》");
#else
	emHeap_Free(pDecoder, EM_DECODER_G7231R_INSTANCE_SIZE);
#endif


	return  g_hTTS->m_nErrorID;
}


//对PCM编码的音频直接播放
emTTSErrID emCall DecodePromptPreRecordPcmAndPlay(
	emInt16 nParaSample,					//采样率
	emInt32 nTableOffset)					//需查音表的偏移量
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

			if( nParaSample != g_hTTS->pRearG->nModelSample)	//提示音与后端声音模型的采样率不相等，需进行重采样
			{
				
				if( nParaSample == 8000 &&  g_hTTS->pRearG->nModelSample == 16000)			//针对8K重采样成16K的：用最简单的采样算法
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
					
					for(j=(nReadLen-2);j>=0;j=j-2)					//8K采样率的音频转成16K采样率的音频
					{
						emMemCpy(g_hTTS->m_pPCMBuffer+2*j+2,g_hTTS->m_pPCMBuffer+j,2);
						emMemCpy(g_hTTS->m_pPCMBuffer+2*j,g_hTTS->m_pPCMBuffer+j,2);
					}
					ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen*2 , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量	
					
					g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen*2);  //回调--播放声音（非HMM合成的）
				}
				else				//其它的：用复杂的重采样算法
				{
										

					
				}

			}
			else		//提示音与后端声音模型的采样率相等，无需重采样，直接输出
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
				ChangePcmVol(VOICE_TYPE_PROMPT,  g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量		


				g_hTTS->m_nErrorID = emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
			}

			
			
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				return emTTS_ERR_EXIT;
			}
		}			
	}	



	return emTTS_ERR_OK;


}







