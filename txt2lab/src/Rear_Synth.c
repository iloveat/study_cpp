#include "emPCH.h"

#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include "emMath.h"
#include "Rear_Synth.h"
#include "vocoder/common.h"
#include "vocoder/synthesis.h"

extern emBool global_bStopTTS;

//extern FILE *ttsfile = NULL;

//#include "emVioceChange.h"

void wavwritedata(const double *x, int x_length, int fs, int nbit, FILE *fp)
{
	short tmp_signal = 0;
	//fseek(fp, len + 44, SEEK_SET);
	for (int i = 0; i < x_length; ++i) {
		tmp_signal = (short)(MyMaxInt(-32768,
			MyMinInt(32767, (int)(x[i] * 32767))));
		fwrite(&tmp_signal, 2, 1, fp);
	}
}

void 	wavwrite_len(FILE * fOutput, emInt32	nSize)
{	
	int data = 0;
	/*char nData[4];

	*(int *)(nData) = nSize + 32;
	fseek(fOutput, 4L, 0);
	fwrite(nData, 1, 4, fOutput);

	*(int *)(nData) = nSize;
	fseek(fOutput, 40L, 0);
	fwrite(nData, 1, 4, fOutput);*/

	
	data = nSize + 32;
	fseek(fOutput, 4, SEEK_SET);
	fwrite(&data, 1, 4, fOutput);
	fseek(fOutput, 40, SEEK_SET);
	fwrite(&data, 1, 4, fOutput);
}

void wavwrite_start(FILE *fp){
	int x_length = 20000;
	int fs = 16000;
	char text[4] = { 'R', 'I', 'F', 'F' };
	unsigned int long_number = 36 + x_length * 2;
	fwrite(text, 1, 4, fp);
	fwrite(&long_number, 4, 1, fp);

	text[0] = 'W';
	text[1] = 'A';
	text[2] = 'V';
	text[3] = 'E';
	fwrite(text, 1, 4, fp);
	text[0] = 'f';
	text[1] = 'm';
	text[2] = 't';
	text[3] = ' ';
	fwrite(text, 1, 4, fp);

	long_number = 16;
	fwrite(&long_number, 4, 1, fp);
	short short_number = 1;
	fwrite(&short_number, 2, 1, fp);
	short_number = 1;
	fwrite(&short_number, 2, 1, fp);
	long_number = fs;
	fwrite(&long_number, 4, 1, fp);
	long_number = fs * 2;
	fwrite(&long_number, 4, 1, fp);
	short_number = 2;
	fwrite(&short_number, 2, 1, fp);
	short_number = 16;
	fwrite(&short_number, 2, 1, fp);

	text[0] = 'd';
	text[1] = 'a';
	text[2] = 't';
	text[3] = 'a';
	fwrite(text, 1, 4, fp);
	long_number = x_length * 2;
	fwrite(&long_number, 4, 1, fp);


}

//****************************************************************************************************
//��������򣺺ϳ�ǰ�������һ�����ӵ�label�����ʵʱ��Ƶ��wav�ļ���������־
//
//	���أ�				
//			�����룺emTTSErrID    ��=emTTS_ERR_OK����ʾ���κϳɳɹ���  =emTTS_ERR_EXIT����ʾ���κϳɽ������˳�TTS��
//
//****************************************************************************************************

emTTSErrID emCall RearSynth(emInt8 nSynLangType)
{
	emPointer pParameter;
	emInt16 nReadLen=0;
	
	//���ʱ��
	emInt16 starttime;
	emInt16 endtime;
	emInt16 startplaytime;
	
	//�����Ƶ��������
	emInt16 PcmByte[REAR_FRAME_LENGTH];
	emInt32  tempTimeLen;
	//PVoiceEffect pVoiceEffect;
	emInt32 nAllocSizeCodebook, nAllocSizeLf0param, nAllocSizeBapparam;
	emInt16 nLabMaxLine, nStartLineOfPau, nLineCountOfPau;
	emInt16 n, m, t, length;
	float	preGainForSilPau,curGainForSilPau,startGainForSilPau;
	
	//�������Ƶ��Ҷ�ڵ�����ɲ��������ָ��
	float	*wuw, *wu;
	float*  pMgcLeafNode;
	float*  pCmpFeature;

	//״̬�������ϳɵ�lab��
	emInt8 nState;
	emInt8 nMaxLabCount;

	int wav_len = 0;
	//ttsfile = fopen("tts.wav", "wb+");
	//wavwrite_start(ttsfile);
	 
#if (WL_REAR_DEAL_DCT_SHORT_SEN || WL_REAR_DEAL_DCT_LIGHT || WL_REAR_DEAL_DCT_PPH_TAIL)
	//����DCTģ��ı���
	emInt32  m1,m2, n1,nCurStartTime, nSylTotalT;
	float fCurF0;
#endif

#if WL_REAR_DEAL_LOWER_F0
	//���ڻ�Ƶ���ӵı���
	float	last_f0, err_f0, temp_f0;
	emInt16	syl_len, last_len;
	float  temp_f0_1;        //wangcm 2012-03-20
	emInt16 syl_len_1, last_len_1;//wangcm 2012-03-20
#endif

#if EM_SYS_SWITCH_FIX
	//���ڶ����˲��ı���
	EM_Vocoder_fix v_fix;
	emInt32 *spectruml;
	emInt32 e;				//�洢lsp�����ж��������ֵg_pRearRTCalcFilter->m_CmpFeature[t][0]
#else
	//EM_Vocoder v;
	//WorldParameters world_parameters;
#endif

	LOG_StackAddr(__FUNCTION__);

	if(nSynLangType == SYNTH_CN)
		g_bRearSynChn = emTrue;
	if(nSynLangType == SYNTH_ENG)
		g_bRearSynChn = emFalse;


	g_hTTS->m_bIsSynFirstAddPau = emFalse;

	//���Ԥ����  �������ⷢ���˵� 
	RearPrevHandle();		


#if	EM_USER_SWITCH_VOICE_EFFECT			  
	//ʶ����Чģʽ
	if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
	{

#if DEBUG_LOG_SWITCH_HEAP
		pVoiceEffect= (struct tagVoiceEffect  *)emHeap_AllocZero(sizeof(struct tagVoiceEffect  ), "��Ч����ռ䣺�����ģ��  ��");
#else
		pVoiceEffect= (struct tagVoiceEffect  *)emHeap_AllocZero(sizeof(struct tagVoiceEffect  ));
#endif
		//emMemSet(pVoiceEffect,0,sizeof(struct tagVoiceEffect  ));

		VoiceChange_InitPara(pVoiceEffect);
		VoiceChange_Init(pVoiceEffect);		 
	}
#endif


#if EM_SYS_SWITCH_FIX
	//�������ڶ����˲������飬��ʼ��Vocoder

#if DEBUG_LOG_SWITCH_HEAP
	spectruml = (emPInt32) emHeap_AllocZero(g_hTTS->pInterParam->static_length * sizeof(emInt32), "��˶����ף������ģ��  ��");
#else
	spectruml = (emPInt32) emHeap_AllocZero(g_hTTS->pInterParam->static_length * sizeof(emInt32));
#endif

	EM_Vocoder_initialize_fix(&v_fix, g_hTTS->pInterParam->static_length - 1, g_hTTS->pRearG->stage , g_hTTS->pRearG->stage, -1);

#else	

	//EM_Vocoder_initialize(&v, g_hTTS->pInterParam->static_length - 1, g_hTTS->pRearG->stage , g_hTTS->pRearG->stage, -1);

#endif


	pParameter=g_hTTS->pCBParam;
	
	//���ϳ�label�������뵽�ڴ�g_pLabRTGlobal->m_LabRam
	nLabMaxLine = g_pLabRTGlobal->m_MaxLabLine;   

	nLineCountOfPau = 0;

	nStartLineOfPau = g_hTTS->m_nParaStartLine;

#if WL_REAR_DEAL_LOWER_F0
	//���ڻ�Ƶ���ӵı�����ʼ��
	last_f0 = 0.0;
	temp_f0_1 = 0;   //wangcm 2012-03-20
	err_f0 = 0.0;
	temp_f0 = 0.0;			//���ڼ�¼ǰһ�����ڵĻ�Ƶ������ modified by mdj 20120306
#endif


	// ���ݺϳɵ�������������״̬�������ϳɵ�lab�� fhy 121225
	switch(nSynLangType)
	{
	case SYNTH_CN:
		nState  = NSTATE_CN;
		nMaxLabCount = MAX_SYNTH_LAB_COUNT;
		break;
	case SYNTH_ENG:
		nState = NSTATE_ENG;
		nMaxLabCount = MAX_SYNTH_ENG_LAB_COUNT;	
		break;
	case SYN_MIX_LANG:
	default:
		nState = NSTATE_CN;
		nMaxLabCount = MAX_SYNTH_ENG_LAB_COUNT;
	}

	/* �����ڴ��Ƶ��Ҷ�ڵ����� */
#if DEBUG_LOG_SWITCH_HEAP //changed by naxy17
	g_pRearRTGlobal = emHeap_AllocZero( sizeof(struct tagRearRTGlobal), "���ȫ��ʱ�⣺�����ģ��  ��");
	pMgcLeafNode = emHeap_AllocZero( (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float), "�����Ҷ�ڵ㣺�����ģ��  ��");
#else
	g_pRearRTGlobal = emHeap_AllocZero( sizeof(struct tagRearRTGlobal));
	pMgcLeafNode = emHeap_AllocZero( (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float));
#endif
	for(n=0;n<nMaxLabCount+2;n++) 
	{
		for(m=0;m<nState;m++)
			g_pRearRTGlobal->m_MgcLeafNode[n][m] = pMgcLeafNode + (n*nState + m)*g_hTTS->pRearG->nMgcLeafPrmN;
	}



	// �ֶ�ѭ���ϳ�
	while ( TRUE )
	{
		// ��ʼ���ֶ���ʼ����
		nStartLineOfPau += nLineCountOfPau;

		//nStartLineOfPau = nParaStartLine;

#if !EM_ENG_AS_SEN_SYNTH
		//������ǵ�1�Σ������ȥ�ֶ���β����pause�ļ���
		if( nStartLineOfPau > g_hTTS->m_nParaStartLine )
			nStartLineOfPau -= 2;			//2
#endif

		// �������label��β������while
		if(nStartLineOfPau >= nLabMaxLine)
			break;
 
		//����

		//���ڴ�g_pLabRTGlobal->m_LabRam��Ѱ������������С�Σ�������ʱLab�У��ȴ��ֶκϳ�;  nStartLineOfPau = ��ʼ����   nLineCountOfPau = ������
		if( g_bRearSynChn)  //����
			nLineCountOfPau = LocateSegment_Cn(nStartLineOfPau, nLabMaxLine);
		/*else
			nLineCountOfPau = LocateSegment_Eng(nStartLineOfPau, nLabMaxLine);*/

		//����sil�;�βsil������ͬһ���кϳɣ���������
		if( nStartLineOfPau == 0 && (nLineCountOfPau-2)==nLabMaxLine)
			nLineCountOfPau--;			

#if EM_PAU_TO_MUTE						//���е�pauͣ��ֱ���������--2012-09-26
		//Ŀǰ���������ܽ�ʡһЩʱ�䣬�����ɶ���β����һ������������βһ���ģ�
		//��Ҫ�򿪴˿��أ����������´���
		//�����������߳�ǰ2��״̬�Ļ�Ƶ���ף�ʱ�������ߣ�ǰ2��״̬ʱ���̶�Ϊ2+2֡��
		//          ������˲�ǰ2��״̬
		
		if( nLineCountOfPau == 3 && g_pLabRTGlobal->m_LabOneSeg[1][0] == INITIAL_CN_pau )	//���pau�����ϳ�
		{
			//�Ȳ�����1���ڲ��������ݣ����Ĳ���MAX_OUTPUT_PCM_SIZE�����ݣ�
			if( nReadLen > 0 )
			{
				
				ChangePcmVol(VOICE_TYPE_SYNTH, g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������
#if	EM_USER_SWITCH_VOICE_EFFECT			  
				//ʶ����Чģʽ
				if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
				{					  
					  g_hTTS->m_nErrorID = VoiceChange_Run(pVoiceEffect, pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				}
				else
				{
					  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				}
#else
				//��ʶ����Чģʽ
				g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
#endif
				nReadLen = 0;
				emMemSet(g_hTTS->m_pPCMBuffer, 0, g_hTTS->m_pPCMBufferSize);
			}

			//���ž�������ֱ��������1����
			PlayMuteDelay(60);	
			continue;
		}
#endif


		//���߳����ϳɶ�ÿ���ֵ�Ҷ�ڵ�
		g_hTTS->m_TimeRemain = GetLeafNode(nStartLineOfPau , nLineCountOfPau, g_hTTS->m_TimeRemain);  


		if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue)			//�յ��˳����2012-07-13 hyl
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			break;
		}

		// �����ڴ����Ƶ��������
		nAllocSizeLf0param = nLineCountOfPau*g_hTTS->pInterParam->nState * PDF_NODE_FLOAT_COUNT_LF0 * sizeof(float);
#if DEBUG_LOG_SWITCH_HEAP
		g_lf0param = (float *)emHeap_AllocZero(nAllocSizeLf0param, "���LF0�����������ģ��  ��"); 
#else
		g_lf0param = (float *)emHeap_AllocZero(nAllocSizeLf0param); 
#endif

		// ��ȡ��Ƶ����
		GetLf0Param(nLineCountOfPau);

		// �����ڴ��BAP��������
		nAllocSizeBapparam = nLineCountOfPau*g_hTTS->pInterParam->nState * PDF_NODE_FLOAT_COUNT_BAP * sizeof(float);
#if DEBUG_LOG_SWITCH_HEAP
		g_bapparam = (float *)emHeap_AllocZero(nAllocSizeBapparam, "���BAP�����������ģ��  ��");
#else
		g_bapparam = (float *)emHeap_AllocZero(nAllocSizeBapparam);
#endif

		// ��ȡBAP����
		GetBapParam(nLineCountOfPau);

		//sqb 2017-5-26
		/*if (nStartLineOfPau == 1)
		{
		g_pRearRTGlobal->m_PhDuration[1][0] += 10;
		}*/
		//��ȡÿ��С�ֶε���֡�������ֶκϳɣ�
		g_hTTS->pInterParam->length = GetSegmentLength(nStartLineOfPau , nLineCountOfPau );

		// ������ʼ�˲�ʱ��
		starttime = 0;
		for (m = 0; m < g_hTTS->pInterParam->nState; m++)
		{
			starttime += g_pRearRTGlobal->m_PhDuration[0][m];
		}
		// ���㿪ʼ����ʱ�䣨������sil��������ƣ�
		startplaytime = 0;
		//sqb 2016/12/19
		if (g_hTTS->m_FirstSynthSegCount== 1 && (g_pLabRTGlobal->m_LabRam[0][RLAB_C_SM] == SIL_INITIAL_NO))
		{
#if !OUTPUT_FIRST_SIL_IN_TEXT  //�Ƿ��������sil
			for(m=0;m<g_hTTS->pInterParam->nState;m++)
				startplaytime += g_pRearRTGlobal->m_PhDuration[1][m];
#endif
			startplaytime += starttime;
		}
		// ��������˲�ʱ��
		endtime = g_hTTS->pInterParam->length;
		for(m=0;m<g_hTTS->pInterParam->nState;m++)
			endtime -= g_pRearRTGlobal->m_PhDuration[nLineCountOfPau-1][m];


		/* �����ڴ�����ɲ������� */    //sqb  +2  ������1ά��BAP
#if DEBUG_LOG_SWITCH_HEAP
		g_pRearRTCalcFilter = emHeap_AllocZero( sizeof(struct tagRearRTCalcFilter), "��˽����˲�ʱ�⣺�����ģ��  ��");
		
		pCmpFeature = emHeap_AllocZero( g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+2)*sizeof(float), "�����������������ģ��  ��");
		for(n=0;n<g_hTTS->pInterParam->length;n++) 
			g_pRearRTCalcFilter->m_CmpFeature[n] = pCmpFeature + n*(g_hTTS->pInterParam->static_length+2);

#else
		g_pRearRTCalcFilter = emHeap_AllocZero( sizeof(struct tagRearRTCalcFilter));
        pCmpFeature = emHeap_AllocZero( g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+2)*sizeof(float));
		for(n=0;n<g_hTTS->pInterParam->length;n++) 
            g_pRearRTCalcFilter->m_CmpFeature[n] = pCmpFeature + n*(g_hTTS->pInterParam->static_length+2);
#endif

		//hyl 2012-10-15 ��ϡ�FirstHandleHanZiBuff�������е�PlayMuteDelayһ��ʹ��
		//�����ı������ٲ��������������ž���ʱ���õ��Ų�����ǰһ����������������ı�������ʱ��
		//������Ϊ��һ�䴢��һ�㾲�����ݣ���ֹ�������1���ֶκ��2���ֶδ���������
		//ע�⣺ǰ1��ġ���㡷������ͣ�١��ľ���һ���ڴˣ�������1��ľ�ͷ���Ῠ  hyl  2012-04-10 
		//��1�Σ���FirstHandleHanZiBuff()�е�RearSynth���������еġ����ߺ󡷺͡�����ǰ��
		//ע�⣺ֻ�ܷ��� Ϊ"��˽���ʱ�⣺�����ģ��  ��"�����ڴ棨Լ8k��ǰ�棬��������Ӷѿռ�ķ�ֵ����ΪPlayMuteDelay���������ڿ�����4K�Ŀռ�  hyl  2012-04-13
		//����: �������������ˣ����ˣ���23�����Ի���29���ڳ�����Լ7Сʱ�Ļ�����̺��Ļ
		if( g_hTTS->m_CurMuteMs > 0 )
		{
			PlayMuteDelay(g_hTTS->m_CurMuteMs);			//�Ȳ��ž���		
		}


		/* �����ڴ����˽������� */
#if DEBUG_LOG_SWITCH_HEAP
		g_pRearRTCalc = emHeap_AllocZero( sizeof(struct tagRearRTCalc), "��˽���ʱ�⣺�����ģ��  ��");
#else
		g_pRearRTCalc = emHeap_AllocZero( sizeof(struct tagRearRTCalc));
#endif

		//��Ҫ�ı����ڵ�����ʱ���������ڴ˺����ڣ�֮��ֻ����������ʱ����������������ʱ������
		// �������������ת�������msd���õ�msd_flag, 
		ModifyMSD(nStartLineOfPau, nLineCountOfPau);				

		// �ļ�ָ��ָ��Ƶ�׷�����󣬷����ڴ���������飬��ȡ�������ݣ����Է��ڷֶ�ǰ������ٶ�
		fRearSeek(g_hTTS->fResCurRearMain, g_pRearOffset->mgcWuw, 0); 
		nAllocSizeCodebook = g_hTTS->pInterParam->static_length * 10 * sizeof(float);
#if DEBUG_LOG_SWITCH_HEAP
		g_Matrix = (float *)emHeap_AllocZero( nAllocSizeCodebook ,"����׷�����󣺡����ģ��  ��");
#else
		g_Matrix = (float *)emHeap_AllocZero( nAllocSizeCodebook );
#endif
		fRearRead(g_Matrix, nAllocSizeCodebook, 1, g_hTTS->fResCurRearMain);

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"���--����", 0);
#endif



		//�ı����״̬������
		ModifyStateGain(nLineCountOfPau); 



		//�������㣬����̬����ά��ѭ��
		for ( m = 0 ; m < g_hTTS->pInterParam->static_length+2 ; m++ ) 
		{
			
			if( m < g_hTTS->pInterParam->static_length)			//   static_length->static_length-1  
			{
				// ����Ƶ��ϵ��
				wuw = g_Matrix + m * 10;
				wu = wuw + g_hTTS->pInterParam->width + 1;
				
				// ���㳤��Ϊ�����ֶ�
				length = g_hTTS->pInterParam->length;
				
				// ���Ĳ��������ֵ���󣬾���ֽ⣬ǰ������g������g_pRearRTCalcFilter->m_CmpFeature���������g_pRearRTCalcFilter->m_CmpFeature
				Calc_wum( m,length, nStartLineOfPau, nLineCountOfPau, wu);				// �������
				LDL_facorication( length, wuw );				// ����ֽ�
				Forward_substitution( m, length );			// ǰ������g������g_pRearRTCalcFilter->m_CmpFeature
				Backward_substitution ( m,length );		// �������g_pRearRTCalcFilter->m_CmpFeature
			}
			else if (m == g_hTTS->pInterParam->static_length)
			{
				// �����Ƶ
				//fRearRead(g_gv, sizeof(float), 2, g_hTTS->fResCurRearMain);

				//����ͻ�ȡÿ��С�ֶε�����֡�������ֶκϳɣ� 
				length = GetSegmentMsdLength(g_hTTS->pInterParam->length);
				if (length)
				{
					// ���岽��������󣬾���ֽ⣬ǰ������g������g_pRearRTCalcFilter->m_CmpFeature���������g_pRearRTCalcFilter->m_CmpFeature��GV��������
					Calc_wuw_wum_msd(m, length, g_hTTS->pInterParam->length, nStartLineOfPau, nLineCountOfPau);			// �������
					LDL_facorication_msd(length);				// ����ֽ�
					Forward_substitution(m, length);			// ǰ������g������g_pRearRTCalcFilter->m_CmpFeature
					Backward_substitution(m, length);			// �������g_pRearRTCalcFilter->m_CmpFeature
					if (WL_REAR_DEAL_GV_PARA != 0)		//�����˻�Ƶ��GV
						GV_paramgen(m, length, WL_REAR_DEAL_GV_PARA);				// GV��������
				}
				// ��MSD���ȵĻ�Ƶ��ԭΪ�ϳɶγ��ȵĻ�Ƶ
				AddUnvoiced(m, length, g_hTTS->pInterParam->length);
			}
			else{
				//����BAP     add by sqb 2017-4-26
				length = g_hTTS->pInterParam->length;			
				// ���岽��������󣬾���ֽ⣬ǰ������g������g_pRearRTCalcFilter->m_CmpFeature���������g_pRearRTCalcFilter->m_CmpFeature��GV��������
				Calc_wuw_wum_bap(m, length, g_hTTS->pInterParam->length, nStartLineOfPau, nLineCountOfPau);			// �������
				LDL_facorication_bap(length);				// ����ֽ�
				Forward_substitution(m, length);			// ǰ������g������g_pRearRTCalcFilter->m_CmpFeature
				Backward_substitution(m, length);			// �������g_pRearRTCalcFilter->m_CmpFeature
				if (WL_REAR_DEAL_GV_PARA != 0)		//������BAP��GV
					GV_paramgen(m, length, WL_REAR_DEAL_GV_PARA);				// GV��������					
			}			
		}

		if(g_bRearSynChn)	{
#if	WL_REAR_DEAL_R_INITIAL
		//����ri,re,ru��������������ri,re,ru������ص�ǰ��������ƽ��
		ModifyRGain(nStartLineOfPau, nLineCountOfPau); // naxy 1205
#endif
		}	// end of g_bRearSynChn


		/* �ͷž��������ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_Matrix, nAllocSizeCodebook ,"����׷�����󣺡����ģ��  ��");
#else
		emHeap_Free(g_Matrix, nAllocSizeCodebook );
#endif

		/* �ͷź�˽��������ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_pRearRTCalc, sizeof(struct tagRearRTCalc), "��˽���ʱ�⣺�����ģ��  ��");
#else
		emHeap_Free(g_pRearRTCalc, sizeof(struct tagRearRTCalc));
#endif



#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"���--����", 0);
#endif


		if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DCT_SHORT_SEN					//<����>���Ķ̾䴦��ģʽ(��ĸ�Ĳ�����)��ר�Ŵ������־䣬���־�

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//ԭʼ�����ǡ����֡���
		{
			//�̾�ģʽ: �������־�����־�ģ������ͻ�Ƶ����DCTģ�壩			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] <= 2)		//�䳤<=2
			{
				nCurStartTime = starttime;
				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					nSylTotalT = 0;
					for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
						nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];

					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] != FINAL_CN_nil	&&			//��Ϊsil
						g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] <= TONE_CN_sil_pau)		//��ΪӢ����ĸ
					{							
						ChangeShortSenZiGainF0(nCurStartTime,nSylTotalT, m1);							
					}

					nCurStartTime += nSylTotalT;
				}
			}
		}
#endif

#if WL_REAR_DEAL_DCT_PPH_TAIL						//������������������ɶ���βDCTģ�壨������˫�̾䣬��������ĸ��

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//ԭʼ�����ǡ����֡���
			&& g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD )		//���ǣ�һ��һ��Ч��
		{
			//�������ɶ���βDCTģ��			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)			//�䳤>2
			{
				nCurStartTime = starttime;
				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					nSylTotalT = 0;
					for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
						nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];


					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0	&&		//���ɶ���β��PPH��syl�ķ���λ��=1
						g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] != TONE_CN_light   &&		//��Ϊ����  hyl   2012-04-17
						g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] != TONE_CN_letter)			//��ΪӢ����ĸ
					{			

						ChangePphTailDctF0(nCurStartTime,nSylTotalT, m1);							
					}

					nCurStartTime += nSylTotalT;
				}
			}
		}
#endif


#if WL_REAR_DEAL_DCT_LIGHT						//�����������DCTģ�壺�������ɶ���β���������־�����ϣ���������ĸ��

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//ԭʼ�����ǡ����֡���
			&& g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD )		//���ǣ�һ��һ��Ч��
		{
			//��������DCTģ��			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 1)			//�䳤>1  ���־������
			{
				nCurStartTime = starttime;
				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					nSylTotalT = 0;
					for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
						nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];

					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0	&&	//���ɶ���β��PPH��syl�ķ���λ��=1
					    g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)		//����Ϊ����
					{							
						ChangeLightDctF0(nCurStartTime,nSylTotalT, m1);							
					}

					nCurStartTime += nSylTotalT;
				}
			}
		}
#endif

#if WL_REAR_DEAL_LOWER_F0
		//��������ɶ���β��ֻ������ĸ������ĸ�����Ļ�Ƶ�������⡣����β������dct��Ƶģ�壩	��һ��һ��Ч������������β��
		//���ӣ����˻�����˶�����˻���ÿ�����ܳ���50kg��
		syl_len = 0;
		last_len = 0;

		last_len_1 = 0; //wangcm 2012-03-20
		//temp_f0_1 = 0;  //wangcm 2012-03-20
		for (n = 0; n < nLineCountOfPau; n++) 
		{

#if WL_REAR_DEAL_THIRD_TONE_LF0
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE]==TONE_CN_light ||
			  (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE]==TONE_CN_3
			&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_3))		//mdj, 2012-03-13�޸����������Ͳ�����
			{
				for(m=0;m<g_hTTS->pInterParam->nState;m++)
					last_len += g_pRearRTGlobal->m_PhDuration[n][m];
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil 
					&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_letter)		
				{
					for(t=last_len-1;t>0;t--)
					{
						if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]>0)
						{
							last_f0 = exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]);
							break;
						}
					}
				}
				continue;
			}
#endif

			if( g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD						// ���ǣ�һ��һ��Ч��
			 &&(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_lateral  ||		// ��ǰ�����Ǳ�����ĸ��l
			    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal  ||		// ��ǰ�����Ǳ�����ĸ��m��n
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r ||						// ��ǰ��������ĸ��r
			    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero)			// ��ǰ����������ĸ
#if WL_REAR_DEAL_LOWER_F0_ALL
			 &&	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] != TONE_CN_letter					// ǰһ���ڲ�ΪӢ����ĸ
			 && g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH]!=0 )					// ��ǰ���ڲ�����ͷmodified by mdj 2012-2-24���һЩ����λ�����ɴ�β�Ļ�Ƶ����
#else
			 && g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH]==0 )					// ��ǰ��������β 
#endif
			{
				// ������ĸ��r��ĸҪ����������
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal 
					|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r)		
				{
					syl_len = 0;
					
						//�ҵ�������ʼ��״̬
					for(m=0;m<g_hTTS->pInterParam->nState;m++)
						if(g_pRearRTCalc->m_MSDflag[last_len] == UNVOICED)
							break;
						else
							last_len += g_pRearRTGlobal->m_PhDuration[n][m];
							
					//�����ȫ��������ԭǰ��ʱ��
					if(m==g_hTTS->pInterParam->nState)
					{
						for(m=0;m<g_hTTS->pInterParam->nState;m++)
							last_len -= g_pRearRTGlobal->m_PhDuration[n][m];
						m=0;
					}
					//����ǰ����Ƶ��Ѱ��������ʼ״̬���ۼ�ǰ��ʱ��
					else
					{
						if(g_pRearRTCalc->m_MSDflag[last_len-1] == VOICED)
							last_f0 = exp(g_pRearRTCalcFilter->m_CmpFeature[last_len-1][g_hTTS->pInterParam->static_length]);
						for(;m<g_hTTS->pInterParam->nState;m++)
							if(g_pRearRTCalc->m_MSDflag[last_len] == VOICED)
								break;
							else
								last_len += g_pRearRTGlobal->m_PhDuration[n][m];
					}
					//����������ʱ�������Ƶ��ֵ
					for(;m<g_hTTS->pInterParam->nState;m++)
						syl_len += g_pRearRTGlobal->m_PhDuration[n][m];
					err_f0 = last_f0 - exp(g_pRearRTCalcFilter->m_CmpFeature[last_len][g_hTTS->pInterParam->static_length]);
					t = last_len;
				}
				else		// �����l��������ĸ����Ϊ�Ѿ�ǿ����ȫ����������ֱ�Ӽ��㳤�ȼ���
				{
					syl_len = 0;
					for(m=0;m<g_hTTS->pInterParam->nState;m++)
						syl_len += g_pRearRTGlobal->m_PhDuration[n][m];
					for(t=last_len;t<last_len+syl_len;t++)
					{
						if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]>0)
						{
							err_f0 = last_f0 - exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]);
							break;
						}
					}
				}
				if(last_f0>0 && emFabs(err_f0)>5 && emFabs(err_f0)<100)		//��Ƶ��ֵ����5ʱ�ŵ���  naxy, 2011-12-30 �޸�
				{
					if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_NUM_IN_C_SEN] <= 2 && err_f0 < 0 )			//���־������������Ľ��л�Ƶ������ֻ�е����ֻ�Ƶ����ǰ�ֻ�Ƶʱ�� modified by hyl 2012-04-07
						err_f0 = 0;
					else if(  g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_NUM_IN_C_SEN] > 2 &&						
						((last_f0 < (180 + g_hTTS->pRearG->f0_mean) && err_f0 < -10) ||
						(last_f0 > (320 + g_hTTS->pRearG->f0_mean) && err_f0 > 10)))		//mdj 2012-03-20
						err_f0 = 0;
					else
					{

					//if(err_f0 > 30.0  || err_f0 < -10.0)
#if WL_REAR_DEAL_LOWER_F0_ALL
						//modified by mdj 2012-2-24���һЩ����λ�����ɴ�β�Ļ�Ƶ���ӣ����ǿ�����������������ɵ���ʧ��
						if((g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM_METHOD]==SM_CN_METHOD_lateral	||		// ǰһ�����Ǳ�����ĸ��l
						   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM_METHOD]==SM_CN_METHOD_nasal		||		// ǰһ�����Ǳ�����ĸ��m��n
						   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM]==INITIAL_CN_r					||		// ǰһ��������ĸ��r
						   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM_METHOD]==SM_CN_METHOD_zero)				// ǰһ����������ĸ
						&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH]>1 )						// ǰһ����������PPH�ĵڶ������ڣ�����ǰһ���ھ��Ѿ�������Ƶ	
						{
							if((temp_f0 > 0 && err_f0 > 0) || (temp_f0 < 0 && err_f0 < 0))					//�������λ�Ƶ���ڷ���һ����ֻ������֮һ
								err_f0 /= 3;	//ֻ��������֮һ�Ļ�Ƶ��ֵ
							else
								err_f0 /= 2;		//ֻ����һ��Ļ�Ƶ��ֵ
						}
						else
#endif
							err_f0 /= 2;		//ֻ����һ��Ļ�Ƶ��ֵ
					}

					for(;t<last_len+syl_len;t++)
					{
						if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]>0)
						{
							temp_f0 = exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]) + err_f0;
							g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] = log(temp_f0);
						}
					}
#if WL_REAR_DEAL_LOWER_F0_ALL
					temp_f0 = err_f0;																	//����ǰһ�����ڵĻ�Ƶ������
#endif
				}
				// �޸�bug��naxy, 2011-12-30 �޸�
				last_len += syl_len;
				if(g_pRearRTCalcFilter->m_CmpFeature[last_len-1][g_hTTS->pInterParam->static_length]>0)
					last_f0 = exp(g_pRearRTCalcFilter->m_CmpFeature[last_len-1][g_hTTS->pInterParam->static_length]);
			} 
			//��ǰ���ڲ���Ҫ���ӻ�Ƶʱ��Ҫ�ۼ�ǰ��ʱ��������ǰ����Ƶ
			else
			{
				for(m=0;m<g_hTTS->pInterParam->nState;m++)
					last_len += g_pRearRTGlobal->m_PhDuration[n][m];
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil 
					&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_letter)		//naxy, 2011-12-30 �޸�
					for(t=last_len-1;t>0;t--)
						if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]>0)
						{
							last_f0 = exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]);
							break;
						}
			}
		// added by wangcm 2012-03-20
#if WL_REAR_DEAL_TONE3_LINKED_ZHUOSM

			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 1 
				&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_3
				&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_TONE] == TONE_CN_light
				&&(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_lateral
				|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_nasal))
			{
				syl_len_1 = 0;
				temp_f0_1 = 0;
				for(m=0;m<g_hTTS->pInterParam->nState;m++)
					syl_len_1 += g_pRearRTGlobal->m_PhDuration[n][m];
				for(t=last_len_1;t<last_len_1 + syl_len_1;t++)
				{
					if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]>0)
					{
						temp_f0_1 = exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]) + 7;
						g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] = log(temp_f0_1);
					}
				}
			}
			else
			{
				for(m=0;m<g_hTTS->pInterParam->nState;m++)
					last_len_1 += g_pRearRTGlobal->m_PhDuration[n][m];
			}

#endif
		// added by wangcm 2012-03-20
//end of WL_REAR_DEAL_TONE3_LINKED_ZHUOSM
		}
#endif


#if WL_REAR_DEAL_LIGHT_TONE			//�Ƕ̾�����������ģʽ��

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//ԭʼ�����ǡ����֡���
		{
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)		//�Ƕ̾䣨ָ�ǵ��־䣬���־䣩
			{
				nCurStartTime = starttime;				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] != FINAL_CN_nil ) //��Ϊsil
					{
						nSylTotalT = 0;
						for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
							nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];
						if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light )	//����
						{							
							for (n1 = nCurStartTime; n1< (nCurStartTime+nSylTotalT); n1++)
							{
								//�ı�����
								g_pRearRTCalcFilter->m_CmpFeature[n1][0] += log(0.9);	//�������ԭ����90%

								//�ı��Ƶ
								fCurF0 = g_pRearRTCalcFilter->m_CmpFeature[n1][g_hTTS->pInterParam->static_length];
								if( fCurF0 != 0.0 )
								{
									fCurF0  = exp(fCurF0) - 10;		//��Ƶ����10
									g_pRearRTCalcFilter->m_CmpFeature[n1][g_hTTS->pInterParam->static_length] = log(fCurF0);
								}
							}
						}
						nCurStartTime += nSylTotalT;
					}
					else
						nCurStartTime += FRONT_SIL_MUTE;
				}
			}
		}
#endif
		}	// end of g_bRearSynChn
		

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"���--����", 0);
#endif



		if(g_bRearSynChn)
		{
//////////////////////////////////////////////////////////////////////////////////////////////////////
#if !(EM_SYN_SEN_HEAD_SIL || OUTPUT_FIRST_SIL_IN_TEXT )
		// Ϊ�������gong4�������������ÿ���һ֡�Ĳ���  ���磺���򳡡����򳡡����򳡡���w��y��m��n��ĸ��ͷ�ľ��ף�
		if (g_pLabRTGlobal->m_LabOneSeg[1][RLAB_L_SM] == INITIAL_CN_sil &&
			g_pLabRTGlobal->m_LabOneSeg[1][RLAB_C_SM] != INITIAL_CN_sil)
		{
#if EM_SYS_SWITCH_FIX
				v_fix.f0 = 0;
				v_fix.p = 0;
				v_fix.p1 = 0;
				v_fix.pc = 0;
				v_fix.inc = 0;
#else
				//v.p=0;
				//v.p1 = 0;
				//v.pc = 0;
				//v.inc = 0;
#endif
				g_pRearRTCalcFilter->m_CmpFeature[starttime][0] -= 1.0;  //��������һ��   sqb
		}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
		}

/*
		//�˲�ǰ��ʼ��
		HTS_world_Vocoder_initialize(&world_parameters, g_hTTS->pInterParam->static_length - 1, g_hTTS->pRearG->stage,
			g_hTTS->pRearG->stage, 16000, 80, 0.42, 0.0, g_hTTS->pInterParam->length-10*2);

		double world_lf0 = 0.0;
		double wolrd_bap = 0.0;
		double wolrd_mgc[19] = { 0.0 };

		//for (int i = 10; i < g_hTTS->pInterParam->length-10; i++)
		for (int i = starttime ; i < endtime; i++ )
		{
			world_lf0 = (double)g_pRearRTCalcFilter->m_CmpFeature[i][g_hTTS->pInterParam->static_length];
			wolrd_bap = (double)g_pRearRTCalcFilter->m_CmpFeature[i][g_hTTS->pInterParam->static_length + 1];
			for (int j = 0; j < 19; j++)
			{
				wolrd_mgc[j] = (double)(g_pRearRTCalcFilter->m_CmpFeature[i][j]);
			}

			HTS_world_par(&world_parameters, world_lf0,
				wolrd_bap,wolrd_mgc, i-starttime);
		}
		double *y = NULL;
		int y_length = 0;
		y_length = (int)((world_parameters.f0_length)*world_parameters.frame_period / 1000 * world_parameters.fs);				//(world_parameters.f0_length-1)		+ 1;
		y = (double *)malloc(sizeof(double)*y_length);
		WaveformSynthesis(&world_parameters,y,y_length);
*/

        int num_frame = g_hTTS->pInterParam->length;
        int num_spectrum = g_hTTS->pInterParam->static_length;
        int num_column = num_spectrum+1+1; //19+1+1
        const int num_skip = 10;
        const double world_alpha = 0.42;
        const int offline = 1;

        //FILE *fp_lf0 = fopen("mid.lf0", "wb");
        //FILE *fp_bap = fopen("mid.bap", "wb");
        //FILE *fp_mgc = fopen("mid.mgc", "wb");

        float **world_input_param = (float**)malloc(num_frame*sizeof(float*));
        for (int q = 0; q < num_frame; q++)
        {
            world_input_param[q] = (float*)malloc(num_column*sizeof(float));
            memcpy(world_input_param[q], g_pRearRTCalcFilter->m_CmpFeature[q], num_column*sizeof(float));
            //fwrite(world_input_param[q]+num_spectrum, sizeof(float), 1, fp_lf0);
            //fwrite(world_input_param[q]+num_spectrum+1, sizeof(float), 1, fp_bap);
            //fwrite(world_input_param[q], sizeof(float), num_spectrum, fp_mgc);
        }

        //fclose(fp_lf0);
        //fclose(fp_bap);
        //fclose(fp_mgc);

        int y_length = (int)((num_frame-num_skip*2)*FRAMEPERIOD/1000*16000);
        double *y = (double *)malloc(sizeof(double)*y_length);

        HTS_World_Synthesize(world_input_param,
                             world_alpha,
                             num_spectrum,
                             num_frame,
                             starttime,
                             endtime,
                             num_skip,
                             offline,
                             y_length,
                             y);

        for(int i = 0; i < num_frame; i++)
        {
            if(world_input_param[i])
            {
                free(world_input_param[i]);
                world_input_param[i] = NULL;
            }
        }
        if(world_input_param)
        {
            free(world_input_param);
            world_input_param = NULL;
        }


		//�˲�--��ʼ
		// ��ʱ,g_pRearRTCalcFilter->m_CmpFeature[t][0~18]���ŵ�t֡��LSP����������������ʹ��MGC��Ϊ0~24��
		//      g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]���ŵ�t֡�Ļ�Ƶ������Ϊ0������ʹ��MGC��Ϊ25��
		// ��һ���ϳɶε����˲�֡��ѭ��
		startGainForSilPau = 0.20;
		

		for (t = starttime; t< endtime; t++)			//sqb 2016/12/19    starttime
		{
			short tmp_signal = 0;
			for (int i = 0; i <REAR_FRAME_LENGTH; ++i) {
				tmp_signal = (short)(MyMaxInt(-32768,
					MyMinInt(32767, (int)(y[(t - 10)*REAR_FRAME_LENGTH+i] * 32767))));
				emMemCpy(PcmByte+i,&tmp_signal,2);
			}


			//modified by hyl  2012-04-06
			//���浱ǰ֡���޸ĺ������ֵ
			//sqb 2017-5-26
			/*	
			if (g_pRearRTGlobal->m_VADflag[t] == 2 )
				curGainForSilPau = preGainForSilPau;
			else
				curGainForSilPau = g_pRearRTCalcFilter->m_CmpFeature[t][0];
			*/

			if( g_bRearSynChn) {
#if WL_REAR_DEAL_VIRTUAL_QST		//�������ⷢ���˵�һЩ����
			//��Ҫ������ⷢ���˵Ļ�Ƶ�������⣨�����������Ƕ���û���⣬�����������⣩  hyl  2012-04-11
			//���磺[m51][t0][s0]��ӭ�ۿ������ϳ�ϵͳ����ʾ[d]��[m52][t0]��ӭ�ۿ������ϳ�ϵͳ����ʾ[d]��[m54][t0]��ӭ�ۿ������ϳ�ϵͳ����ʾ[d]��
			if( g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] != 0.0)			//����
				if( g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] < 4.0 )		//��Ƶ����
					g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] = 4.0;			//������Ƶ
#endif
			}	// end of g_bRearSynChn


			//modified by hyl  2012-04-11
			//sqb 2017-5-26
			/*
			if( g_pRearRTGlobal->m_VADflag[t] == 2 )
			{
				//��pau�;�βsil��ÿ֡������ֵ�𲽵ݼ�����ֵ����һ��������Ƶ���Ǳ���������Ƶ
				//���β����������  2�����������ˣ�1.β��û���������� 2.������������ 
				g_pRearRTCalcFilter->m_CmpFeature[t][0] = preGainForSilPau - startGainForSilPau;		//�����𽥼���
				curGainForSilPau -= startGainForSilPau;

				g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] = g_pRearRTCalcFilter->m_CmpFeature[t-1][g_hTTS->pInterParam->static_length];	//��Ƶ����ǰ1֡��
				g_pRearRTGlobal->m_VADflag[t] = 0;				//�������
				startGainForSilPau += 0.2;

			}
			*/
			//�����������ֵ
			if( g_pRearRTGlobal->m_VADflag[t] == 0 )				//==0		
			{
				//����򸡵��˲�
#if EM_SYS_SWITCH_FIX
				if ( g_hTTS->pRearG->bIsStraight == TRUE )
					g_pRearRTCalcFilter->m_CmpFeature[t][0] += 11.0;
				for(m = 0 ; m < g_hTTS->pInterParam->static_length ; m++)
					Q15 spectruml[m] = (emInt32)( g_pRearRTCalcFilter->m_CmpFeature[t][m]*(1<<15) );//������Ҳ������Q15�Ķ��㻯

				//�Լ������к�������Ķ��㻯
				if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] == 0.0)
					v_fix.f0 = 0;
				else
				{
#if !EM_CAL_EXP
					Q9 v_fix.f0 = (emInt32)(exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]) * (1<<9));//��exp(lf0)����ΪQ9
#else
					Q9 v_fix.f0 = CalExp(9,  (emInt16)(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]*512));  //2012-10-31 ��û���ã�ÿ���ֶ��˲��ܽ�ʡ15ms���������ж���ı䣩
#endif
				}
			
				//���������к�������Ķ��㻯			
				if (v_fix.use_log_gain)
				{
#if !EM_CAL_EXP
					Q15 e = (emInt32)(exp(g_pRearRTCalcFilter->m_CmpFeature[t][0]) * (1<<15));				//�������ȳ���Qֵ��Ȼ����ȡ��������ʹ�����ڲ����滻����ģ�����뵽��
#else
					Q15 e = CalExp(9,  (emInt16)(g_pRearRTCalcFilter->m_CmpFeature[t][0]*512))<<6;	//2012-10-31 ��û���ã�ÿ���ֶ��˲��ܽ�ʡ15ms���������ж���ı䣩
#endif
				}
				else
					e = (emInt32)g_pRearRTCalcFilter->m_CmpFeature[t][0];	//�˾�����Ͳ���ִ�У�����ֱ��ȥ��

				EM_Vocoder_synthesize_fix(&v_fix, g_hTTS->pInterParam->static_length - 1, e, spectruml, PcmByte);

#else
				/*EM_Vocoder_synthesize(&v, g_hTTS->pInterParam->static_length - 1,
							 g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length],
							 g_pRearRTCalcFilter->m_CmpFeature[t], PcmByte);*/

				/*HTS_world_par(&world_parameters, g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length], 
					g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length+1],
					g_pRearRTCalcFilter->m_CmpFeature[t], t);*/


#endif
			}
			else if(t>startplaytime && !g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil && nStartLineOfPau==0)
			{
				//������[p?]ͣ�٣������sil����֡���0ֵ
				emMemSet(PcmByte,0,REAR_FRAME_LENGTH*2);				
			} 
			else if ( nStartLineOfPau + nLineCountOfPau - 2 == nLabMaxLine && !g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil) 
			{
				//������[p?]ͣ�٣������sil����֡���0ֵ
				emMemSet(PcmByte,0,REAR_FRAME_LENGTH*2);				
			}
			else if( t>startplaytime && (nStartLineOfPau + nLineCountOfPau - 2) < nLabMaxLine )
			{
				//�����м�����ڣ�����ǰ��sil�������0ֵ
				emMemSet(PcmByte,0,REAR_FRAME_LENGTH*2);			
			}
			else
			{
				//��֡�����		
				g_pRearRTGlobal->m_VADflag[t] = 0;

				if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT )   //hyl  2012-04-06
					g_hTTS->m_nErrorID = emTTS_ERR_OK;
				continue;
			}
			
			
			g_pRearRTGlobal->m_VADflag[t] = 0;		//�������㣬������һ�ֶκϳ�

#if EM_8K_MODEL_PLAN								//�״ν���ϳɣ��Ȳ��ž���
			if( g_hTTS->m_bIsSynFirstTxtZi == emTrue)
			{
				PlayMuteDelay(250);		//���״ν���ϳɣ��Ȳ��ž���������200ms����������ֹ����   ���磺"һ�ߴ��ŵ绰"  "��һ�����"
				g_hTTS->m_bIsSynFirstTxtZi = emFalse;
			}
#endif

			//���һ֡��Ƶ����
			emMemCpy(g_hTTS->m_pPCMBuffer+nReadLen, PcmByte, sizeof(emInt16)*g_hTTS->pRearG->frame_length);
			nReadLen += sizeof(emInt16)*g_hTTS->pRearG->frame_length;


			if( (nReadLen+g_hTTS->pRearG->frame_length*2) > MAX_OUTPUT_PCM_SIZE)
			{

				  ChangePcmVol(VOICE_TYPE_SYNTH, g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������

				 

#if	EM_USER_SWITCH_VOICE_EFFECT			  
				  //ʶ����Чģʽ
				  if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
				  {					  
					  g_hTTS->m_nErrorID = VoiceChange_Run(pVoiceEffect, pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				  }
				  else
				  {
					  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				  }
#else
				  //��ʶ����Чģʽ
				  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
#endif


				  nReadLen = 0;
				  emMemSet(g_hTTS->m_pPCMBuffer, 0, g_hTTS->m_pPCMBufferSize);

				  if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
				  {
					  break;
				  }

			}

			//����ǰһ֡���޸ĺ������ֵ
			//preGainForSilPau = curGainForSilPau;
	
		} // for t

        if(y)
        {
		    free(y);
		    y = NULL;
		}
		//HTS_WORLD_clear(&world_parameters);

		/* �ͷ��ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pCmpFeature,g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+1)*sizeof(float), "�����������������ģ��  ��");
		
		emHeap_Free(g_pRearRTCalcFilter, sizeof(struct tagRearRTCalcFilter), "��˽����˲�ʱ�⣺�����ģ��  ��");
		emHeap_Free(g_lf0param, nAllocSizeLf0param, "���LF0�����������ģ��  ��"); 
		emHeap_Free(g_bapparam, nAllocSizeBapparam, "���BAP�����������ģ��  ��");
#else
		emHeap_Free(pCmpFeature,g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+1)*sizeof(float));
		emHeap_Free(g_pRearRTCalcFilter, sizeof(struct tagRearRTCalcFilter));
		emHeap_Free(g_bapparam, nAllocSizeBapparam); 
#endif

		if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
		{
			break;
		}

#if DEBUG_LOG_SWITCH_TIME

		tempTimeLen = g_hTTS->pInterParam->length * 5;

	#if SEG_ADD_PAU_TEN_STAGE_TEN_FRAME
		tempTimeLen -= SEG_TEMP_PAU_TOTAL_FRAME*2*5;
	#else
		tempTimeLen -= (SEG_TEMP_PREV_PAU_TOTAL_FRAME+SEG_TEMP_NEXT_PAU_TOTAL_FRAME)*5;
	#endif

		switch( g_hTTS->m_ControlSwitch.m_nRoleIndex )
		{
			case emTTS_USE_ROLE_XIAOLIN:	//С��
					break;
			case emTTS_USE_ROLE_Virtual_51:	
					tempTimeLen = tempTimeLen/(1-DUR_SCALE_ROLE_Virtual_51);					
					break;
			case emTTS_USE_ROLE_Virtual_52:	
					tempTimeLen = tempTimeLen/(1-DUR_SCALE_ROLE_Virtual_52);		
					break;
			case emTTS_USE_ROLE_Virtual_53:	
					tempTimeLen = tempTimeLen/(1-DUR_SCALE_ROLE_Virtual_53);		
					break;
			case emTTS_USE_ROLE_Virtual_54:	
					tempTimeLen = tempTimeLen/(1-DUR_SCALE_ROLE_Virtual_54);		
					break;
			case emTTS_USE_ROLE_Virtual_55:			
					tempTimeLen = tempTimeLen/(1-DUR_SCALE_ROLE_Virtual_55);		
					break;		
			default:	
					break;
		}


		myTimeTrace_For_Debug(1,(emByte *)"���--���벥��", 0);
		myTimeTrace_For_Debug(2,(emByte *)"		���--�ֶ����ʵ��ʱ����", 0);
		if( nStartLineOfPau != 0)
		{
			myTimeTrace_For_Debug(3, (emByte *)"" , tempTimeLen);
		}
		else
		{
	#if OUTPUT_FIRST_SIL_IN_TEXT
			myTimeTrace_For_Debug(3, (emByte *)"" , tempTimeLen);
	#else
			myTimeTrace_For_Debug(3, (emByte *)"" , tempTimeLen - 250);		
	#endif
		}
#endif

	} // for while


	/* �ͷ��ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pMgcLeafNode, (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float), "�����Ҷ�ڵ㣺�����ģ��  ��");
	emHeap_Free(g_pRearRTGlobal, sizeof(struct tagRearRTGlobal), "���ȫ��ʱ�⣺�����ģ��  ��");
#else
	emHeap_Free(pMgcLeafNode, (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float));
	emHeap_Free(g_pRearRTGlobal, sizeof(struct tagRearRTGlobal));
#endif


	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)
	{
		  //�������Ĳ���MAX_OUTPUT_PCM_SIZE������
		  ChangePcmVol(VOICE_TYPE_SYNTH, g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //�������������������

#if	EM_USER_SWITCH_VOICE_EFFECT			  
		  //ʶ����Чģʽ
		  if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
		  {					  
			  g_hTTS->m_nErrorID = VoiceChange_Run(pVoiceEffect, pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
		  }
		  else
		  {
			  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
		  }
#else
		  //��ʶ����Чģʽ
		  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
#endif
		  nReadLen = 0;
		  emMemSet(g_hTTS->m_pPCMBuffer, 0, g_hTTS->m_pPCMBufferSize);
	}



#if EM_SYS_SWITCH_FIX
	EM_Vocoder_clear_fix(&v_fix);


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(spectruml, g_hTTS->pInterParam->static_length * sizeof(emInt32), "��˶����ף������ģ��  ��");
#else
	emHeap_Free(spectruml, g_hTTS->pInterParam->static_length * sizeof(emInt32));
#endif

#else
	//EM_Vocoder_clear(&v);
#endif

#if	EM_USER_SWITCH_VOICE_EFFECT			  
	//ʶ����Чģʽ
	if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
	{
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pVoiceEffect, sizeof(struct tagVoiceEffect  ), "��Ч����ռ䣺�����ģ��  ��");
#else
		emHeap_Free(pVoiceEffect, sizeof(struct tagVoiceEffect  ));
#endif

	}
#endif


	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emFalse;
	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil = emFalse;

	//wavwrite_len(ttsfile,wav_len);
	//fclose(ttsfile);				//�ر�tts.wav

	return g_hTTS->m_nErrorID; // make the compiler happy
}



//�ͷź�˵ľ�����
void emCall FreeRearAllDecision()
{
	emInt32 nHeapSize;

#if EM_8K_MODEL_PLAN			//8Kģ������
	nHeapSize = USER_HEAP_MIN_CONFIG_ADD;
#else							//16Kģ������
	nHeapSize = USER_HEAP_MAX_CONFIG_ADD;				
#endif

#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_AllTree , nHeapSize , "���о������������ģ��  ��");
#else
		emHeap_Free(g_AllTree , nHeapSize );
#endif
}



//���غ�˵ľ�����
void emCall LoadRearAllDecision()
{
	emInt32 nHeapSize;
	emInt32 nSourceOffset,nTargetOffset;
	emInt32 nAllocSize;
	emInt16 curNodeSum;
	emInt16 i;

#if EM_8K_MODEL_PLAN			//8Kģ������
	nHeapSize = USER_HEAP_MIN_CONFIG_ADD;
#else							//16Kģ������
	nHeapSize = USER_HEAP_MAX_CONFIG_ADD;				
#endif

#if DEBUG_LOG_SWITCH_HEAP
	g_AllTree = (emByte *)emHeap_AllocZero(nHeapSize , "���о������������ģ��  ��");
#else
	g_AllTree = (emByte *)emHeap_AllocZero(nHeapSize );
#endif


	nTargetOffset = 0;


	//��ȡ��1�ţ�������ת����--������
	g_AllTreeOffset[0] = nTargetOffset;
	nSourceOffset = g_hTTS->pRearG->offset_cn.uvpMod;
	fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
	fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
	nTargetOffset += nAllocSize;

	//��ȡ��1�ţ�ʱ��ת����--������
	g_AllTreeOffset[1] = nTargetOffset;
	nSourceOffset = g_hTTS->pRearG->offset_cn.durMod;
	fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
	fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
	nTargetOffset += nAllocSize;

	//��ȡ��10�ţ�Ƶ��ת����--������
	nSourceOffset = g_hTTS->pRearG->offset_cn.mgcMod;
	for(i = 0; i < g_hTTS->pInterParam->nState; i++ )
	{
		g_AllTreeOffset[2+i] = nTargetOffset;		
		fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
		fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
		nTargetOffset += nAllocSize;

		//ָ����һ�þ�����
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //Ƶ������ά0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
			nSourceOffset += 2 + nAllocSize + 
				(g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen + 2) * curNodeSum;		
		else
			nSourceOffset += 2 + nAllocSize + 
				g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen * curNodeSum;	

	}

	


	//��ȡ��10�ţ���Ƶת����--������
	nSourceOffset = g_hTTS->pRearG->offset_cn.lf0Mod;
	for(i = 0; i < g_hTTS->pInterParam->nState; i++ )
	{
		g_AllTreeOffset[12+i] = nTargetOffset;		
		fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
		fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
		nTargetOffset += nAllocSize;
		//ָ����һ�þ�����
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //��Ƶ0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
			nSourceOffset += 2 + nAllocSize
				+ (PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen+2) *curNodeSum;	
		else
			nSourceOffset += 2 + nAllocSize
				+ PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen *curNodeSum;
	}

	
}
