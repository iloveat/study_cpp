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
//后端主程序：合成前端输出的一个句子的label，输出实时音频、wav文件和若干日志
//
//	返回：				
//			错误码：emTTSErrID    （=emTTS_ERR_OK：表示本次合成成功；  =emTTS_ERR_EXIT：表示本次合成结束，退出TTS）
//
//****************************************************************************************************

emTTSErrID emCall RearSynth(emInt8 nSynLangType)
{
	emPointer pParameter;
	emInt16 nReadLen=0;
	
	//输出时间
	emInt16 starttime;
	emInt16 endtime;
	emInt16 startplaytime;
	
	//输出音频数据数组
	emInt16 PcmByte[REAR_FRAME_LENGTH];
	emInt32  tempTimeLen;
	//PVoiceEffect pVoiceEffect;
	emInt32 nAllocSizeCodebook, nAllocSizeLf0param, nAllocSizeBapparam;
	emInt16 nLabMaxLine, nStartLineOfPau, nLineCountOfPau;
	emInt16 n, m, t, length;
	float	preGainForSilPau,curGainForSilPau,startGainForSilPau;
	
	//保存矩阵、频谱叶节点和生成参数数组的指针
	float	*wuw, *wu;
	float*  pMgcLeafNode;
	float*  pCmpFeature;

	//状态数、最大合成的lab数
	emInt8 nState;
	emInt8 nMaxLabCount;

	int wav_len = 0;
	//ttsfile = fopen("tts.wav", "wb+");
	//wavwrite_start(ttsfile);
	 
#if (WL_REAR_DEAL_DCT_SHORT_SEN || WL_REAR_DEAL_DCT_LIGHT || WL_REAR_DEAL_DCT_PPH_TAIL)
	//用于DCT模板的变量
	emInt32  m1,m2, n1,nCurStartTime, nSylTotalT;
	float fCurF0;
#endif

#if WL_REAR_DEAL_LOWER_F0
	//用于基频连接的变量
	float	last_f0, err_f0, temp_f0;
	emInt16	syl_len, last_len;
	float  temp_f0_1;        //wangcm 2012-03-20
	emInt16 syl_len_1, last_len_1;//wangcm 2012-03-20
#endif

#if EM_SYS_SWITCH_FIX
	//用于定点滤波的变量
	EM_Vocoder_fix v_fix;
	emInt32 *spectruml;
	emInt32 e;				//存储lsp参数中定点的能量值g_pRearRTCalcFilter->m_CmpFeature[t][0]
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

	//后端预处理  设置虚拟发音人等 
	RearPrevHandle();		


#if	EM_USER_SWITCH_VOICE_EFFECT			  
	//识别音效模式
	if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
	{

#if DEBUG_LOG_SWITCH_HEAP
		pVoiceEffect= (struct tagVoiceEffect  *)emHeap_AllocZero(sizeof(struct tagVoiceEffect  ), "音效输出空间：《后端模块  》");
#else
		pVoiceEffect= (struct tagVoiceEffect  *)emHeap_AllocZero(sizeof(struct tagVoiceEffect  ));
#endif
		//emMemSet(pVoiceEffect,0,sizeof(struct tagVoiceEffect  ));

		VoiceChange_InitPara(pVoiceEffect);
		VoiceChange_Init(pVoiceEffect);		 
	}
#endif


#if EM_SYS_SWITCH_FIX
	//开辟用于定点滤波的数组，初始化Vocoder

#if DEBUG_LOG_SWITCH_HEAP
	spectruml = (emPInt32) emHeap_AllocZero(g_hTTS->pInterParam->static_length * sizeof(emInt32), "后端定点谱：《后端模块  》");
#else
	spectruml = (emPInt32) emHeap_AllocZero(g_hTTS->pInterParam->static_length * sizeof(emInt32));
#endif

	EM_Vocoder_initialize_fix(&v_fix, g_hTTS->pInterParam->static_length - 1, g_hTTS->pRearG->stage , g_hTTS->pRearG->stage, -1);

#else	

	//EM_Vocoder_initialize(&v, g_hTTS->pInterParam->static_length - 1, g_hTTS->pRearG->stage , g_hTTS->pRearG->stage, -1);

#endif


	pParameter=g_hTTS->pCBParam;
	
	//将合成label内容载入到内存g_pLabRTGlobal->m_LabRam
	nLabMaxLine = g_pLabRTGlobal->m_MaxLabLine;   

	nLineCountOfPau = 0;

	nStartLineOfPau = g_hTTS->m_nParaStartLine;

#if WL_REAR_DEAL_LOWER_F0
	//用于基频连接的变量初始化
	last_f0 = 0.0;
	temp_f0_1 = 0;   //wangcm 2012-03-20
	err_f0 = 0.0;
	temp_f0 = 0.0;			//用于记录前一个音节的基频调整量 modified by mdj 20120306
#endif


	// 根据合成的语言类型设置状态数和最大合成的lab数 fhy 121225
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

	/* 分配内存给频谱叶节点数组 */
#if DEBUG_LOG_SWITCH_HEAP //changed by naxy17
	g_pRearRTGlobal = emHeap_AllocZero( sizeof(struct tagRearRTGlobal), "后端全局时库：《后端模块  》");
	pMgcLeafNode = emHeap_AllocZero( (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float), "后端谱叶节点：《后端模块  》");
#else
	g_pRearRTGlobal = emHeap_AllocZero( sizeof(struct tagRearRTGlobal));
	pMgcLeafNode = emHeap_AllocZero( (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float));
#endif
	for(n=0;n<nMaxLabCount+2;n++) 
	{
		for(m=0;m<nState;m++)
			g_pRearRTGlobal->m_MgcLeafNode[n][m] = pMgcLeafNode + (n*nState + m)*g_hTTS->pRearG->nMgcLeafPrmN;
	}



	// 分段循环合成
	while ( TRUE )
	{
		// 初始化分段起始行数
		nStartLineOfPau += nLineCountOfPau;

		//nStartLineOfPau = nParaStartLine;

#if !EM_ENG_AS_SEN_SYNTH
		//如果不是第1次，则需减去分段首尾插入pause的计数
		if( nStartLineOfPau > g_hTTS->m_nParaStartLine )
			nStartLineOfPau -= 2;			//2
#endif

		// 如果到达label结尾则跳出while
		if(nStartLineOfPau >= nLabMaxLine)
			break;
 
		//决策

		//从内存g_pLabRTGlobal->m_LabRam中寻找满足条件的小段，存入临时Lab中，等待分段合成;  nStartLineOfPau = 起始行数   nLineCountOfPau = 总行数
		if( g_bRearSynChn)  //中文
			nLineCountOfPau = LocateSegment_Cn(nStartLineOfPau, nLabMaxLine);
		/*else
			nLineCountOfPau = LocateSegment_Eng(nStartLineOfPau, nLabMaxLine);*/

		//句首sil和句尾sil不能在同一段中合成（否则会出错）
		if( nStartLineOfPau == 0 && (nLineCountOfPau-2)==nLabMaxLine)
			nLineCountOfPau--;			

#if EM_PAU_TO_MUTE						//句中的pau停顿直接输出静音--2012-09-26
		//目前这样处理能节省一些时间，但韵律短语尾会有一点杂音（跟句尾一样的）
		//若要打开此开关，还需做如下处理
		//处理方法：决策出前2个状态的基频和谱（时长不决策，前2个状态时长固定为2+2帧）
		//          解算和滤波前2个状态
		
		if( nLineCountOfPau == 3 && g_pLabRTGlobal->m_LabOneSeg[1][0] == INITIAL_CN_pau )	//针对pau单独合成
		{
			//先播放上1音节残留的数据（最后的不足MAX_OUTPUT_PCM_SIZE的数据）
			if( nReadLen > 0 )
			{
				
				ChangePcmVol(VOICE_TYPE_SYNTH, g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量
#if	EM_USER_SWITCH_VOICE_EFFECT			  
				//识别音效模式
				if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
				{					  
					  g_hTTS->m_nErrorID = VoiceChange_Run(pVoiceEffect, pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				}
				else
				{
					  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				}
#else
				//不识别音效模式
				g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
#endif
				nReadLen = 0;
				emMemSet(g_hTTS->m_pPCMBuffer, 0, g_hTTS->m_pPCMBufferSize);
			}

			//播放静音，并直接跳到下1音节
			PlayMuteDelay(60);	
			continue;
		}
#endif


		//决策出本合成段每个字的叶节点
		g_hTTS->m_TimeRemain = GetLeafNode(nStartLineOfPau , nLineCountOfPau, g_hTTS->m_TimeRemain);  


		if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue)			//收到退出命令，2012-07-13 hyl
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			break;
		}

		// 分配内存给基频参数数组
		nAllocSizeLf0param = nLineCountOfPau*g_hTTS->pInterParam->nState * PDF_NODE_FLOAT_COUNT_LF0 * sizeof(float);
#if DEBUG_LOG_SWITCH_HEAP
		g_lf0param = (float *)emHeap_AllocZero(nAllocSizeLf0param, "后端LF0参数：《后端模块  》"); 
#else
		g_lf0param = (float *)emHeap_AllocZero(nAllocSizeLf0param); 
#endif

		// 获取基频参数
		GetLf0Param(nLineCountOfPau);

		// 分配内存给BAP参数数组
		nAllocSizeBapparam = nLineCountOfPau*g_hTTS->pInterParam->nState * PDF_NODE_FLOAT_COUNT_BAP * sizeof(float);
#if DEBUG_LOG_SWITCH_HEAP
		g_bapparam = (float *)emHeap_AllocZero(nAllocSizeBapparam, "后端BAP参数：《后端模块  》");
#else
		g_bapparam = (float *)emHeap_AllocZero(nAllocSizeBapparam);
#endif

		// 获取BAP参数
		GetBapParam(nLineCountOfPau);

		//sqb 2017-5-26
		/*if (nStartLineOfPau == 1)
		{
		g_pRearRTGlobal->m_PhDuration[1][0] += 10;
		}*/
		//获取每个小分段的总帧数（按分段合成）
		g_hTTS->pInterParam->length = GetSegmentLength(nStartLineOfPau , nLineCountOfPau );

		// 计算起始滤波时间
		starttime = 0;
		for (m = 0; m < g_hTTS->pInterParam->nState; m++)
		{
			starttime += g_pRearRTGlobal->m_PhDuration[0][m];
		}
		// 计算开始播放时间（即句首sil的输出控制）
		startplaytime = 0;
		//sqb 2016/12/19
		if (g_hTTS->m_FirstSynthSegCount== 1 && (g_pLabRTGlobal->m_LabRam[0][RLAB_C_SM] == SIL_INITIAL_NO))
		{
#if !OUTPUT_FIRST_SIL_IN_TEXT  //是否输出句首sil
			for(m=0;m<g_hTTS->pInterParam->nState;m++)
				startplaytime += g_pRearRTGlobal->m_PhDuration[1][m];
#endif
			startplaytime += starttime;
		}
		// 计算结束滤波时间
		endtime = g_hTTS->pInterParam->length;
		for(m=0;m<g_hTTS->pInterParam->nState;m++)
			endtime -= g_pRearRTGlobal->m_PhDuration[nLineCountOfPau-1][m];


		/* 分配内存给生成参数数组 */    //sqb  +2  增加了1维的BAP
#if DEBUG_LOG_SWITCH_HEAP
		g_pRearRTCalcFilter = emHeap_AllocZero( sizeof(struct tagRearRTCalcFilter), "后端解算滤波时库：《后端模块  》");
		
		pCmpFeature = emHeap_AllocZero( g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+2)*sizeof(float), "后端组合特征：《后端模块  》");
		for(n=0;n<g_hTTS->pInterParam->length;n++) 
			g_pRearRTCalcFilter->m_CmpFeature[n] = pCmpFeature + n*(g_hTTS->pInterParam->static_length+2);

#else
		g_pRearRTCalcFilter = emHeap_AllocZero( sizeof(struct tagRearRTCalcFilter));
        pCmpFeature = emHeap_AllocZero( g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+2)*sizeof(float));
		for(n=0;n<g_hTTS->pInterParam->length;n++) 
            g_pRearRTCalcFilter->m_CmpFeature[n] = pCmpFeature + n*(g_hTTS->pInterParam->static_length+2);
#endif

		//hyl 2012-10-15 配合《FirstHandleHanZiBuff》函数中的PlayMuteDelay一起使用
		//先做文本分析再播静音：这样播放静音时不用等着播放完前一句的声音，利用了文本分析的时间
		//这样能为下一句储存一点静音数据，防止播放完第1个分段后第2个分段处理来不及
		//注意：前1句的《标点》或《制造停顿》的静音一定在此，这样下1句的句头不会卡  hyl  2012-04-10 
		//第1段：在FirstHandleHanZiBuff()中的RearSynth（）函数中的《决策后》和《解算前》
		//注意：只能放在 为"后端解算时库：《后端模块  》"开辟内存（约8k）前面，否则会增加堆空间的峰值，因为PlayMuteDelay（）函数内开辟了4K的空间  hyl  2012-04-13
		//例如: 阿拉伯国家联盟（阿盟）第23届首脑会议29日在持续了约7小时的会议议程后闭幕
		if( g_hTTS->m_CurMuteMs > 0 )
		{
			PlayMuteDelay(g_hTTS->m_CurMuteMs);			//先播放静音		
		}


		/* 分配内存给后端结算数组 */
#if DEBUG_LOG_SWITCH_HEAP
		g_pRearRTCalc = emHeap_AllocZero( sizeof(struct tagRearRTCalc), "后端解算时库：《后端模块  》");
#else
		g_pRearRTCalc = emHeap_AllocZero( sizeof(struct tagRearRTCalc));
#endif

		//若要改变音节的总体时长，必须在此函数内，之后只允许：音节内时长调整，但音节总时长不变
		// 根据清浊音最佳转换点调整msd，得到msd_flag, 
		ModifyMSD(nStartLineOfPau, nLineCountOfPau);				

		// 文件指针指向频谱方差矩阵，分配内存给矩阵数组，读取矩阵数据，可以放在分段前以提高速度
		fRearSeek(g_hTTS->fResCurRearMain, g_pRearOffset->mgcWuw, 0); 
		nAllocSizeCodebook = g_hTTS->pInterParam->static_length * 10 * sizeof(float);
#if DEBUG_LOG_SWITCH_HEAP
		g_Matrix = (float *)emHeap_AllocZero( nAllocSizeCodebook ,"后端谱方差矩阵：《后端模块  》");
#else
		g_Matrix = (float *)emHeap_AllocZero( nAllocSizeCodebook );
#endif
		fRearRead(g_Matrix, nAllocSizeCodebook, 1, g_hTTS->fResCurRearMain);

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"后端--决策", 0);
#endif



		//改变个别状态的能量
		ModifyStateGain(nLineCountOfPau); 



		//参数解算，按静态特征维数循环
		for ( m = 0 ; m < g_hTTS->pInterParam->static_length+2 ; m++ ) 
		{
			
			if( m < g_hTTS->pInterParam->static_length)			//   static_length->static_length-1  
			{
				// 计算频谱系数
				wuw = g_Matrix + m * 10;
				wu = wuw + g_hTTS->pInterParam->width + 1;
				
				// 解算长度为整个分段
				length = g_hTTS->pInterParam->length;
				
				// 分四步：计算均值矩阵，矩阵分解，前代法求g，存入g_pRearRTCalcFilter->m_CmpFeature，后代法求g_pRearRTCalcFilter->m_CmpFeature
				Calc_wum( m,length, nStartLineOfPau, nLineCountOfPau, wu);				// 计算矩阵
				LDL_facorication( length, wuw );				// 矩阵分解
				Forward_substitution( m, length );			// 前代法求g，存入g_pRearRTCalcFilter->m_CmpFeature
				Backward_substitution ( m,length );		// 后代法求g_pRearRTCalcFilter->m_CmpFeature
			}
			else if (m == g_hTTS->pInterParam->static_length)
			{
				// 计算基频
				//fRearRead(g_gv, sizeof(float), 2, g_hTTS->fResCurRearMain);

				//解算和获取每个小分段的浊音帧数（按分段合成） 
				length = GetSegmentMsdLength(g_hTTS->pInterParam->length);
				if (length)
				{
					// 分五步：计算矩阵，矩阵分解，前代法求g，存入g_pRearRTCalcFilter->m_CmpFeature，后代法求g_pRearRTCalcFilter->m_CmpFeature，GV参数计算
					Calc_wuw_wum_msd(m, length, g_hTTS->pInterParam->length, nStartLineOfPau, nLineCountOfPau);			// 计算矩阵
					LDL_facorication_msd(length);				// 矩阵分解
					Forward_substitution(m, length);			// 前代法求g，存入g_pRearRTCalcFilter->m_CmpFeature
					Backward_substitution(m, length);			// 后代法求g_pRearRTCalcFilter->m_CmpFeature
					if (WL_REAR_DEAL_GV_PARA != 0)		//若开了基频的GV
						GV_paramgen(m, length, WL_REAR_DEAL_GV_PARA);				// GV参数计算
				}
				// 将MSD长度的基频还原为合成段长度的基频
				AddUnvoiced(m, length, g_hTTS->pInterParam->length);
			}
			else{
				//解算BAP     add by sqb 2017-4-26
				length = g_hTTS->pInterParam->length;			
				// 分五步：计算矩阵，矩阵分解，前代法求g，存入g_pRearRTCalcFilter->m_CmpFeature，后代法求g_pRearRTCalcFilter->m_CmpFeature，GV参数计算
				Calc_wuw_wum_bap(m, length, g_hTTS->pInterParam->length, nStartLineOfPau, nLineCountOfPau);			// 计算矩阵
				LDL_facorication_bap(length);				// 矩阵分解
				Forward_substitution(m, length);			// 前代法求g，存入g_pRearRTCalcFilter->m_CmpFeature
				Backward_substitution(m, length);			// 后代法求g_pRearRTCalcFilter->m_CmpFeature
				if (WL_REAR_DEAL_GV_PARA != 0)		//若开了BAP的GV
					GV_paramgen(m, length, WL_REAR_DEAL_GV_PARA);				// GV参数计算					
			}			
		}

		if(g_bRearSynChn)	{
#if	WL_REAR_DEAL_R_INITIAL
		//减少ri,re,ru清音能量，并对ri,re,ru清音相关的前后能量做平滑
		ModifyRGain(nStartLineOfPau, nLineCountOfPau); // naxy 1205
#endif
		}	// end of g_bRearSynChn


		/* 释放矩阵数组内存 */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_Matrix, nAllocSizeCodebook ,"后端谱方差矩阵：《后端模块  》");
#else
		emHeap_Free(g_Matrix, nAllocSizeCodebook );
#endif

		/* 释放后端结算数组内存 */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_pRearRTCalc, sizeof(struct tagRearRTCalc), "后端解算时库：《后端模块  》");
#else
		emHeap_Free(g_pRearRTCalc, sizeof(struct tagRearRTCalc));
#endif



#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"后端--解算", 0);
#endif


		if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DCT_SHORT_SEN					//<王林>中文短句处理模式(字母的不处理)：专门处理：单字句，两字句

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//原始音库是《王林》的
		{
			//短句模式: 调整单字句和两字句的：能量和基频（用DCT模板）			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] <= 2)		//句长<=2
			{
				nCurStartTime = starttime;
				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					nSylTotalT = 0;
					for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
						nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];

					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] != FINAL_CN_nil	&&			//不为sil
						g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] <= TONE_CN_sil_pau)		//不为英文字母
					{							
						ChangeShortSenZiGainF0(nCurStartTime,nSylTotalT, m1);							
					}

					nCurStartTime += nSylTotalT;
				}
			}
		}
#endif

#if WL_REAR_DEAL_DCT_PPH_TAIL						//王林音库后处理：中文韵律短语尾DCT模板（不含单双短句，不处理字母）

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//原始音库是《王林》的
			&& g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD )		//不是：一字一顿效果
		{
			//中文韵律短语尾DCT模板			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)			//句长>2
			{
				nCurStartTime = starttime;
				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					nSylTotalT = 0;
					for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
						nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];


					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0	&&		//韵律短语尾：PPH中syl的反序位置=1
						g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] != TONE_CN_light   &&		//不为轻声  hyl   2012-04-17
						g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] != TONE_CN_letter)			//不为英文字母
					{			

						ChangePphTailDctF0(nCurStartTime,nSylTotalT, m1);							
					}

					nCurStartTime += nSylTotalT;
				}
			}
		}
#endif


#if WL_REAR_DEAL_DCT_LIGHT						//王林音库后处理：DCT模板：中文韵律短语尾轻声（两字句或以上，不处理字母）

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//原始音库是《王林》的
			&& g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD )		//不是：一字一顿效果
		{
			//中文轻声DCT模板			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 1)			//句长>1  两字句或以上
			{
				nCurStartTime = starttime;
				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					nSylTotalT = 0;
					for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
						nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];

					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0	&&	//韵律短语尾：PPH中syl的反序位置=1
					    g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)		//必须为轻声
					{							
						ChangeLightDctF0(nCurStartTime,nSylTotalT, m1);							
					}

					nCurStartTime += nSylTotalT;
				}
			}
		}
#endif

#if WL_REAR_DEAL_LOWER_F0
		//解决：韵律短语尾（只对零声母和浊声母处理）的基频连接问题。（韵尾都用了dct基频模板）	（一字一顿效果：不处理韵尾）
		//例子：托运货物。托运动物。托运货物每件不能超过50kg。
		syl_len = 0;
		last_len = 0;

		last_len_1 = 0; //wangcm 2012-03-20
		//temp_f0_1 = 0;  //wangcm 2012-03-20
		for (n = 0; n < nLineCountOfPau; n++) 
		{

#if WL_REAR_DEAL_THIRD_TONE_LF0
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE]==TONE_CN_light ||
			  (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE]==TONE_CN_3
			&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_3))		//mdj, 2012-03-13修改连续三声就不调。
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

			if( g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD						// 不是：一字一顿效果
			 &&(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_lateral  ||		// 当前音节是边音声母，l
			    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal  ||		// 当前音节是鼻音声母，m、n
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r ||						// 当前音节是声母，r
			    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero)			// 当前音节是零声母
#if WL_REAR_DEAL_LOWER_F0_ALL
			 &&	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] != TONE_CN_letter					// 前一音节不为英文字母
			 && g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH]!=0 )					// 当前音节不是韵头modified by mdj 2012-2-24解决一些不是位于韵律词尾的基频连接
#else
			 && g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH]==0 )					// 当前音节是韵尾 
#endif
			{
				// 鼻音声母和r声母要跳过清音段
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal 
					|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r)		
				{
					syl_len = 0;
					
						//找到清音开始的状态
					for(m=0;m<g_hTTS->pInterParam->nState;m++)
						if(g_pRearRTCalc->m_MSDflag[last_len] == UNVOICED)
							break;
						else
							last_len += g_pRearRTGlobal->m_PhDuration[n][m];
							
					//如果是全浊音，则还原前导时长
					if(m==g_hTTS->pInterParam->nState)
					{
						for(m=0;m<g_hTTS->pInterParam->nState;m++)
							last_len -= g_pRearRTGlobal->m_PhDuration[n][m];
						m=0;
					}
					//保存前导基频，寻找浊音起始状态，累加前导时长
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
					//计算浊音段时长，求基频差值
					for(;m<g_hTTS->pInterParam->nState;m++)
						syl_len += g_pRearRTGlobal->m_PhDuration[n][m];
					err_f0 = last_f0 - exp(g_pRearRTCalcFilter->m_CmpFeature[last_len][g_hTTS->pInterParam->static_length]);
					t = last_len;
				}
				else		// 如果是l或者零声母，因为已经强制了全浊音，所以直接计算长度即可
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
				if(last_f0>0 && emFabs(err_f0)>5 && emFabs(err_f0)<100)		//基频差值大于5时才调整  naxy, 2011-12-30 修改
				{
					if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_NUM_IN_C_SEN] <= 2 && err_f0 < 0 )			//两字句以下有条件的进行基频调整（只有当后字基频低于前字基频时） modified by hyl 2012-04-07
						err_f0 = 0;
					else if(  g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_NUM_IN_C_SEN] > 2 &&						
						((last_f0 < (180 + g_hTTS->pRearG->f0_mean) && err_f0 < -10) ||
						(last_f0 > (320 + g_hTTS->pRearG->f0_mean) && err_f0 > 10)))		//mdj 2012-03-20
						err_f0 = 0;
					else
					{

					//if(err_f0 > 30.0  || err_f0 < -10.0)
#if WL_REAR_DEAL_LOWER_F0_ALL
						//modified by mdj 2012-2-24解决一些不是位于韵律词尾的基频连接，但是可能由于连续调节造成调节失控
						if((g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM_METHOD]==SM_CN_METHOD_lateral	||		// 前一音节是边音声母，l
						   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM_METHOD]==SM_CN_METHOD_nasal		||		// 前一音节是鼻音声母，m、n
						   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM]==INITIAL_CN_r					||		// 前一音节是声母，r
						   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_SM_METHOD]==SM_CN_METHOD_zero)				// 前一音节是零声母
						&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH]>1 )						// 前一音节至少是PPH的第二个音节，这样前一音节就已经调过基频	
						{
							if((temp_f0 > 0 && err_f0 > 0) || (temp_f0 < 0 && err_f0 < 0))					//连续两次基频调节方向一样就只调三分之一
								err_f0 /= 3;	//只补偿三分之一的基频差值
							else
								err_f0 /= 2;		//只补偿一半的基频差值
						}
						else
#endif
							err_f0 /= 2;		//只补偿一半的基频差值
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
					temp_f0 = err_f0;																	//保留前一个音节的基频调节量
#endif
				}
				// 修改bug，naxy, 2011-12-30 修改
				last_len += syl_len;
				if(g_pRearRTCalcFilter->m_CmpFeature[last_len-1][g_hTTS->pInterParam->static_length]>0)
					last_f0 = exp(g_pRearRTCalcFilter->m_CmpFeature[last_len-1][g_hTTS->pInterParam->static_length]);
			} 
			//当前音节不需要连接基频时，要累加前导时长，更新前导基频
			else
			{
				for(m=0;m<g_hTTS->pInterParam->nState;m++)
					last_len += g_pRearRTGlobal->m_PhDuration[n][m];
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil 
					&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_letter)		//naxy, 2011-12-30 修改
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


#if WL_REAR_DEAL_LIGHT_TONE			//非短句中轻声处理模式：

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//原始音库是《王林》的
		{
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)		//非短句（指非单字句，两字句）
			{
				nCurStartTime = starttime;				
				for( m1 = 1; m1< (nLineCountOfPau-1); m1++)
				{
					if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] != FINAL_CN_nil ) //不为sil
					{
						nSylTotalT = 0;
						for(n1=0;n1<g_hTTS->pInterParam->nState;n1++)
							nSylTotalT += g_pRearRTGlobal->m_PhDuration[m1][n1];
						if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light )	//轻声
						{							
							for (n1 = nCurStartTime; n1< (nCurStartTime+nSylTotalT); n1++)
							{
								//改变能量
								g_pRearRTCalcFilter->m_CmpFeature[n1][0] += log(0.9);	//音量变成原来的90%

								//改变基频
								fCurF0 = g_pRearRTCalcFilter->m_CmpFeature[n1][g_hTTS->pInterParam->static_length];
								if( fCurF0 != 0.0 )
								{
									fCurF0  = exp(fCurF0) - 10;		//基频降低10
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
		myTimeTrace_For_Debug(1,(emByte *)"后端--后处理", 0);
#endif



		if(g_bRearSynChn)
		{
//////////////////////////////////////////////////////////////////////////////////////////////////////
#if !(EM_SYN_SEN_HEAD_SIL || OUTPUT_FIRST_SIL_IN_TEXT )
		// 为处理句首gong4音的情况，更改每句第一帧的参数  例如：网球场。网球场。网球场。（w，y，m，n声母开头的句首）
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
				g_pRearRTCalcFilter->m_CmpFeature[starttime][0] -= 1.0;  //能量减少一点   sqb
		}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
		}

/*
		//滤波前初始化
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


		//滤波--开始
		// 此时,g_pRearRTCalcFilter->m_CmpFeature[t][0~18]存着第t帧的LSP，包括能量。（如使用MGC则为0~24）
		//      g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]存着第t帧的基频，清音为0。（如使用MGC则为25）
		// 按一个合成段的总滤波帧数循环
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
			//保存当前帧的修改后的能量值
			//sqb 2017-5-26
			/*	
			if (g_pRearRTGlobal->m_VADflag[t] == 2 )
				curGainForSilPau = preGainForSilPau;
			else
				curGainForSilPau = g_pRearRTCalcFilter->m_CmpFeature[t][0];
			*/

			if( g_bRearSynChn) {
#if WL_REAR_DEAL_VIRTUAL_QST		//处理虚拟发音人的一些问题
			//主要针对虚拟发音人的基频过低问题（若不调整，非定点没问题，但定点有问题）  hyl  2012-04-11
			//例如：[m51][t0][s0]欢迎观看语音合成系统的演示[d]。[m52][t0]欢迎观看语音合成系统的演示[d]。[m54][t0]欢迎观看语音合成系统的演示[d]。
			if( g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] != 0.0)			//浊音
				if( g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] < 4.0 )		//基频过低
					g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] = 4.0;			//调整基频
#endif
			}	// end of g_bRearSynChn


			//modified by hyl  2012-04-11
			//sqb 2017-5-26
			/*
			if( g_pRearRTGlobal->m_VADflag[t] == 2 )
			{
				//即pau和句尾sil的每帧的能量值逐步递减（两值必须一样），基频还是保持浊音基频
				//解决尾部杂音问题  2种情况都解决了：1.尾部没正常结束， 2.结束后还有噪音 
				g_pRearRTCalcFilter->m_CmpFeature[t][0] = preGainForSilPau - startGainForSilPau;		//能量逐渐减少
				curGainForSilPau -= startGainForSilPau;

				g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] = g_pRearRTCalcFilter->m_CmpFeature[t-1][g_hTTS->pInterParam->static_length];	//基频复制前1帧的
				g_pRearRTGlobal->m_VADflag[t] = 0;				//输出声音
				startGainForSilPau += 0.2;

			}
			*/
			//若需输出正常值
			if( g_pRearRTGlobal->m_VADflag[t] == 0 )				//==0		
			{
				//定点或浮点滤波
#if EM_SYS_SWITCH_FIX
				if ( g_hTTS->pRearG->bIsStraight == TRUE )
					g_pRearRTCalcFilter->m_CmpFeature[t][0] += 11.0;
				for(m = 0 ; m < g_hTTS->pInterParam->static_length ; m++)
					Q15 spectruml[m] = (emInt32)( g_pRearRTCalcFilter->m_CmpFeature[t][m]*(1<<15) );//这里能也进行了Q15的定点化

				//对激励进行函数外面的定点化
				if(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length] == 0.0)
					v_fix.f0 = 0;
				else
				{
#if !EM_CAL_EXP
					Q9 v_fix.f0 = (emInt32)(exp(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]) * (1<<9));//对exp(lf0)定点为Q9
#else
					Q9 v_fix.f0 = CalExp(9,  (emInt16)(g_pRearRTCalcFilter->m_CmpFeature[t][g_hTTS->pInterParam->static_length]*512));  //2012-10-31 暂没启用（每个分段滤波能节省15ms，声音略有丁点改变）
#endif
				}
			
				//对能量进行函数外面的定点化			
				if (v_fix.use_log_gain)
				{
#if !EM_CAL_EXP
					Q15 e = (emInt32)(exp(g_pRearRTCalcFilter->m_CmpFeature[t][0]) * (1<<15));				//这里是先乘上Q值，然后再取整，这里使用了在测试替换浮点模块中想到的
#else
					Q15 e = CalExp(9,  (emInt16)(g_pRearRTCalcFilter->m_CmpFeature[t][0]*512))<<6;	//2012-10-31 暂没启用（每个分段滤波能节省15ms，声音略有丁点改变）
#endif
				}
				else
					e = (emInt32)g_pRearRTCalcFilter->m_CmpFeature[t][0];	//此句根本就不会执行，可以直接去掉

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
				//当遇到[p?]停顿，不输出sil：本帧输出0值
				emMemSet(PcmByte,0,REAR_FRAME_LENGTH*2);				
			} 
			else if ( nStartLineOfPau + nLineCountOfPau - 2 == nLabMaxLine && !g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil) 
			{
				//当遇到[p?]停顿，不输出sil：本帧输出0值
				emMemSet(PcmByte,0,REAR_FRAME_LENGTH*2);				
			}
			else if( t>startplaytime && (nStartLineOfPau + nLineCountOfPau - 2) < nLabMaxLine )
			{
				//若是中间的音节，即非前后sil：则输出0值
				emMemSet(PcmByte,0,REAR_FRAME_LENGTH*2);			
			}
			else
			{
				//本帧不输出		
				g_pRearRTGlobal->m_VADflag[t] = 0;

				if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT )   //hyl  2012-04-06
					g_hTTS->m_nErrorID = emTTS_ERR_OK;
				continue;
			}
			
			
			g_pRearRTGlobal->m_VADflag[t] = 0;		//数组置零，用于下一分段合成

#if EM_8K_MODEL_PLAN								//首次进入合成，先播放静音
			if( g_hTTS->m_bIsSynFirstTxtZi == emTrue)
			{
				PlayMuteDelay(250);		//在首次进入合成，先播放静音（测试200ms不够），防止音卡   例如："一边打着电话"  "这一着真高"
				g_hTTS->m_bIsSynFirstTxtZi = emFalse;
			}
#endif

			//输出一帧音频数据
			emMemCpy(g_hTTS->m_pPCMBuffer+nReadLen, PcmByte, sizeof(emInt16)*g_hTTS->pRearG->frame_length);
			nReadLen += sizeof(emInt16)*g_hTTS->pRearG->frame_length;


			if( (nReadLen+g_hTTS->pRearG->frame_length*2) > MAX_OUTPUT_PCM_SIZE)
			{

				  ChangePcmVol(VOICE_TYPE_SYNTH, g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量

				 

#if	EM_USER_SWITCH_VOICE_EFFECT			  
				  //识别音效模式
				  if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
				  {					  
					  g_hTTS->m_nErrorID = VoiceChange_Run(pVoiceEffect, pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				  }
				  else
				  {
					  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
				  }
#else
				  //不识别音效模式
				  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
#endif


				  nReadLen = 0;
				  emMemSet(g_hTTS->m_pPCMBuffer, 0, g_hTTS->m_pPCMBufferSize);

				  if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
				  {
					  break;
				  }

			}

			//保存前一帧的修改后的能量值
			//preGainForSilPau = curGainForSilPau;
	
		} // for t

        if(y)
        {
		    free(y);
		    y = NULL;
		}
		//HTS_WORLD_clear(&world_parameters);

		/* 释放内存 */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pCmpFeature,g_hTTS->pInterParam->length*(g_hTTS->pInterParam->static_length+1)*sizeof(float), "后端组合特征：《后端模块  》");
		
		emHeap_Free(g_pRearRTCalcFilter, sizeof(struct tagRearRTCalcFilter), "后端解算滤波时库：《后端模块  》");
		emHeap_Free(g_lf0param, nAllocSizeLf0param, "后端LF0参数：《后端模块  》"); 
		emHeap_Free(g_bapparam, nAllocSizeBapparam, "后端BAP参数：《后端模块  》");
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
			case emTTS_USE_ROLE_XIAOLIN:	//小林
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


		myTimeTrace_For_Debug(1,(emByte *)"后端--解码播放", 0);
		myTimeTrace_For_Debug(2,(emByte *)"		后端--分段输出实际时长：", 0);
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


	/* 释放内存 */
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pMgcLeafNode, (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float), "后端谱叶节点：《后端模块  》");
	emHeap_Free(g_pRearRTGlobal, sizeof(struct tagRearRTGlobal), "后端全局时库：《后端模块  》");
#else
	emHeap_Free(pMgcLeafNode, (nMaxLabCount+2)*nState*g_hTTS->pRearG->nMgcLeafPrmN*sizeof(float));
	emHeap_Free(g_pRearRTGlobal, sizeof(struct tagRearRTGlobal));
#endif


	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)
	{
		  //播放最后的不足MAX_OUTPUT_PCM_SIZE的数据
		  ChangePcmVol(VOICE_TYPE_SYNTH, g_hTTS->m_pPCMBuffer , nReadLen , g_hTTS->m_ControlSwitch.m_nVolumn );  //根据音量级别更改音量

#if	EM_USER_SWITCH_VOICE_EFFECT			  
		  //识别音效模式
		  if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
		  {					  
			  g_hTTS->m_nErrorID = VoiceChange_Run(pVoiceEffect, pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
		  }
		  else
		  {
			  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
		  }
#else
		  //不识别音效模式
		  g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);
#endif
		  nReadLen = 0;
		  emMemSet(g_hTTS->m_pPCMBuffer, 0, g_hTTS->m_pPCMBufferSize);
	}



#if EM_SYS_SWITCH_FIX
	EM_Vocoder_clear_fix(&v_fix);


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(spectruml, g_hTTS->pInterParam->static_length * sizeof(emInt32), "后端定点谱：《后端模块  》");
#else
	emHeap_Free(spectruml, g_hTTS->pInterParam->static_length * sizeof(emInt32));
#endif

#else
	//EM_Vocoder_clear(&v);
#endif

#if	EM_USER_SWITCH_VOICE_EFFECT			  
	//识别音效模式
	if( g_hTTS->m_ControlSwitch.m_nVoiceMode != emTTS_USE_VoiceMode_CLOSE)
	{
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pVoiceEffect, sizeof(struct tagVoiceEffect  ), "音效输出空间：《后端模块  》");
#else
		emHeap_Free(pVoiceEffect, sizeof(struct tagVoiceEffect  ));
#endif

	}
#endif


	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emFalse;
	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil = emFalse;

	//wavwrite_len(ttsfile,wav_len);
	//fclose(ttsfile);				//关闭tts.wav

	return g_hTTS->m_nErrorID; // make the compiler happy
}



//释放后端的决策树
void emCall FreeRearAllDecision()
{
	emInt32 nHeapSize;

#if EM_8K_MODEL_PLAN			//8K模型配置
	nHeapSize = USER_HEAP_MIN_CONFIG_ADD;
#else							//16K模型配置
	nHeapSize = USER_HEAP_MAX_CONFIG_ADD;				
#endif

#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_AllTree , nHeapSize , "所有决策树：《后端模块  》");
#else
		emHeap_Free(g_AllTree , nHeapSize );
#endif
}



//加载后端的决策树
void emCall LoadRearAllDecision()
{
	emInt32 nHeapSize;
	emInt32 nSourceOffset,nTargetOffset;
	emInt32 nAllocSize;
	emInt16 curNodeSum;
	emInt16 i;

#if EM_8K_MODEL_PLAN			//8K模型配置
	nHeapSize = USER_HEAP_MIN_CONFIG_ADD;
#else							//16K模型配置
	nHeapSize = USER_HEAP_MAX_CONFIG_ADD;				
#endif

#if DEBUG_LOG_SWITCH_HEAP
	g_AllTree = (emByte *)emHeap_AllocZero(nHeapSize , "所有决策树：《后端模块  》");
#else
	g_AllTree = (emByte *)emHeap_AllocZero(nHeapSize );
#endif


	nTargetOffset = 0;


	//读取：1颗：清浊音转换点--决策树
	g_AllTreeOffset[0] = nTargetOffset;
	nSourceOffset = g_hTTS->pRearG->offset_cn.uvpMod;
	fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
	fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
	nTargetOffset += nAllocSize;

	//读取：1颗：时长转换点--决策树
	g_AllTreeOffset[1] = nTargetOffset;
	nSourceOffset = g_hTTS->pRearG->offset_cn.durMod;
	fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
	fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
	nTargetOffset += nAllocSize;

	//读取：10颗：频谱转换点--决策树
	nSourceOffset = g_hTTS->pRearG->offset_cn.mgcMod;
	for(i = 0; i < g_hTTS->pInterParam->nState; i++ )
	{
		g_AllTreeOffset[2+i] = nTargetOffset;		
		fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
		fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
		nTargetOffset += nAllocSize;

		//指向下一棵决策树
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //频谱能量维0阶均值按4字节读，其余按2字节读
			nSourceOffset += 2 + nAllocSize + 
				(g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen + 2) * curNodeSum;		
		else
			nSourceOffset += 2 + nAllocSize + 
				g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen * curNodeSum;	

	}

	


	//读取：10颗：基频转换点--决策树
	nSourceOffset = g_hTTS->pRearG->offset_cn.lf0Mod;
	for(i = 0; i < g_hTTS->pInterParam->nState; i++ )
	{
		g_AllTreeOffset[12+i] = nTargetOffset;		
		fRearSeek(g_hTTS->fResCurRearMain,nSourceOffset ,0);		
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);	
		fRearRead(g_AllTree+nTargetOffset,nAllocSize,1,g_hTTS->fResCurRearMain);
		nTargetOffset += nAllocSize;
		//指向下一棵决策树
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //基频0阶均值按4字节读，其余按2字节读
			nSourceOffset += 2 + nAllocSize
				+ (PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen+2) *curNodeSum;	
		else
			nSourceOffset += 2 + nAllocSize
				+ PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen *curNodeSum;
	}

	
}
