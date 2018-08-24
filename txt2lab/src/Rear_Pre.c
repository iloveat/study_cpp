#include "emPCH.h"


#include "Rear_Pre.h"
//#include "EngF_Common.h"   //sqb

//预定义的pause label——中文pau
//emStatic emConst emByte	g_LabPau[LAB_CN_LINE_LEN]={0x02, 0x01, 0x06, 0x0D, 0x15, 0x02, 0x03, 0x1B, 0x03, 0x07, 0x05, 0x01, 0x02, 0x05, 0x01, 0x05,
//0x01, 0x09, 0x08, 0x04, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

emStatic emConst emByte	g_LabPau[LAB_CN_LINE_LEN] = { 0x02, 0x01, 0x07, 0x05, 0x01, 0x02, 0x0D, 0x15, 0x05, 0x01, 0x05, 0x01, 0x03, 0x1B,
0x09, 0x08, 0x04, 0x03, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


//根据英文静音lab（sp）设置的pau lab


	//来源实际sp的lab（好像更饱满），但将前音素+后音素+前前音素+后后音素改成255没有影响
/*emStatic emConst emByte	g_LabPauEng[LAB_EN_LINE_LEN] = {
53,22,22,28,3,0,1,0,0,1,1,255,0,3,1,3,2,2,2,1,0,1,8,3,3,0,0,0,0,
8,2,4,2,9,4,12,3,3,5,0,2,5,5,2,255,0,1,2,0,1,255,1,255,1,255,2,255,
1,1,255,255,255,255,255,255,255,255,2,11,23,31,255,255,255,71,2,11,
23,31,255,255,255,71,2,11,255,255,48,56,255,73,2,12,25,36,48,56,66,255};	*/

	////构造的sp（好像饱满度差点），但将前音素+后音素+前前音素+后后音素改成255没有影响
	//emStatic emConst emByte	g_LabPauEng[LAB_EN_LINE_LEN] = {							
	//53,22,22,28,3,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,1,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};



//sqb 2017/1/6
//计算 当前音素所在单词的音素数（从当前音素到单词末尾）
/*
static emInt16 LabNumOfWord(emInt16 nStartLine)
{
	emInt16 i, nPh = 0, nPhLine;			//nPh 单词的音素数
	emInt16 nSYL, nPHIndex,nSYLIndex;
	emBool bIsWordTail;

	LOG_StackAddr(__FUNCTION__);

	
	nSYL    = g_pLabRTGlobal->m_LabRam[nStartLine][E_RLAB_SYL_NUM_IN_C_PW];		//当前单词中的音节数目
	nPhLine = nStartLine;														//起始行数

	if((nSYL&0x00FF) == 0x00FF)			//表明是sil
		nPh = 1;			
	else
	{
		bIsWordTail = emTrue;
		//按单词的音节循环
		for(i = g_pLabRTGlobal->m_LabRam[nStartLine][ELAB_I01_FwdPos_SylInWord]; i< nSYL; i++)					//从单词的当前音节 到 最大音节
		{
			if( (nPh  + g_pLabRTGlobal->m_LabRam[nPhLine][E_RLAB_PH_NUM_IN_C_SYL]) >= MAX_SYNTH_ENG_LAB_COUNT)   //若合成音节数 >= 合成一段的最大英文lab行，则退出（长单词内再次分段）
			{
				bIsWordTail = emFalse;
				break;
			}

			nPh     += g_pLabRTGlobal->m_LabRam[nPhLine][E_RLAB_PH_NUM_IN_C_SYL];
			nPhLine += g_pLabRTGlobal->m_LabRam[nPhLine][E_RLAB_PH_NUM_IN_C_SYL];	

			
		}


		if( g_pLabRTGlobal->m_LabRam[nPhLine-1][E_RLAB_R_BORDER_IN_WORD]>=BD_L3 && bIsWordTail == emTrue)		//word的右边界：是PPH边界  且是单词尾
		{
			nPh++;				//加sp行			原来的前端无sp行
			nPhLine++;	
		}

	}

	return nPh;
}

// 英文lab按单词分段
emInt16 LocateSegment_Eng(emInt16 nStartLine, emInt16 nMaxLine)
{
	emInt16 i, nLineCount=0;
	emInt16 nLabSeg;

	LOG_StackAddr(__FUNCTION__);

	//设置分段合成决策用的后端模型（英文模型）和状态数（5）
	g_pRearOffset = &g_hTTS->pRearG->offset_eng;
	g_hTTS->pInterParam->nState = NSTATE_ENG;

#if EM_ENG_AS_SEN_SYNTH			//按句分段合成
	for(i = 0; i< nMaxLine+1 ; i++)			//循环从LabRam中读取
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount++], g_pLabRTGlobal->m_LabRam[i], LAB_EN_LINE_LEN);
#else
	
	// 在分段的第一个位置插入pause
	emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPauEng, LAB_EN_LINE_LEN); 
	nLineCount++;

	//获取合成单词的lab行数
	nLabSeg = LabNumOfWord(nStartLine);	


	//循环从LabRam中读取
	for(i = nStartLine; i< (nMaxLine+1) ; i++)
	{
		if( nLineCount == (nLabSeg + 1 ))
		{
			// 如果音节数目到达限制，插入预定义pause，退出循环
			emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPauEng, LAB_EN_LINE_LEN);
			nLineCount++;
			break;
			
		}
		// 拷贝一行label
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_pLabRTGlobal->m_LabRam[i], LAB_EN_LINE_LEN);
		nLineCount++;
	}
#endif

	g_hTTS->m_FirstSynthSegCount++;		//本次合成文本第几个分段计数。（例如：本段中1个句子的第1个小分段 = 1）

	return nLineCount;
}
*/

//计算 当前音节所在韵律词的音节数（从当前音节到韵律词末尾）
//static emInt16 LabNumOfPW(emInt16 nStartLine)
//{
//	emInt16 i, nSyl, nPW,nPWIndex;			//nSyl 韵律词的音节数
//
//	LOG_StackAddr(__FUNCTION__);
//
//	// 获取当前韵律词的音节数目
//	nPW      = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_NUM_IN_C_PW]; 
//	// 获取音节在当前韵律词中的位置
//	nPWIndex = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_FWD_POS_IN_PW];
//
//	if(nPW == 0)			//表明是sil或pau
//		nSyl = 1;			
//	else
//	{
//		// 若当前韵律词的音节数大于3，则输出3个音节
//		if( (nPW - nPWIndex)  > MAX_SYNTH_LAB_COUNT_PW)
//			nSyl = MAX_SYNTH_LAB_COUNT_PW;		
//		else
//			nSyl = (nPW - nPWIndex);
//	}
//
//	return nSyl;
//}
//
//

#if EM_PW_SEGMENT_OPEN 			//按韵律词分段合成 + 大配置或小配置

//从内存g_pLabRTGlobal->m_LabRam中寻找满足条件的小段，存入临时Lab中，等待分段合成
emInt16	LocateSegment_Cn(emInt16 nStartLine, emInt16 nMaxLine)
{
	emInt16 i, nLineCount=0;
	emInt16 nSyl,nPW, nPWIndex,nNextPWIndex,nNextPW;

	LOG_StackAddr(__FUNCTION__);

	//设置分段合成决策用的后端模型（中文模型）和状态数（10）
	g_pRearOffset = &g_hTTS->pRearG->offset_cn;
	g_hTTS->pInterParam->nState = NSTATE_CN;

	// 在分段的第一个位置插入pause
	emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN); 
	nLineCount++;

	//虚拟发音人必须每3字一合成，低于3字会卡  
	//与王林[m3]对比:  滤波需重采样工作量增大；决策时间不变；总时长增大重采样后再复原会增加解算滤波时间
	//综合考虑：每3字一合成效果好（不会卡）
	if( g_hTTS->m_ControlSwitch.m_nRoleIndex !=  emTTS_USE_ROLE_XIAOLIN )
	{
		// hyl 2012-04-10
		if( nStartLine >= 1 && nStartLine != (nMaxLine-1) )
		{
			if( (nMaxLine-2) == 3 )					 //3字句（加前后sil共5个音节）：按3+1合成（否则会卡） hyl  2012-04-12	例如：[m51][s10]你的书。你的鱼。[m3][d] 
			{
				nSyl = 3;
			}
			else if( (nMaxLine - nStartLine) == 4)   //最后剩4个音节（含sil），按2+2合成
			{
				nSyl = 2;	
			}
			else if( (nStartLine+3) <= nMaxLine )		
			{
				nSyl = 3;						//一般情况下，都按3字合成
			}
			else					
			{
				nSyl = nMaxLine - nStartLine;	//最后剩下的音节合成
			}
		}
		else
			nSyl = 1;
	}
	else
	{
		if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD )				//hyl 2012-03-27
		{
			//是：一字一顿效果
			if( nStartLine >= 1 )
				nSyl =  EM_STYLE_WORD_SYNTH_LAB_COUNT;		//每次合成的音节数基本为2(1汉字+1Pau)
			else
				nSyl = 1;


		}
		else
		{
			// 获取当前韵律词的音节数目
			nPW      = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_NUM_IN_C_PW]; 
			// 获取音节在当前韵律词中的位置
			nPWIndex = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_FWD_POS_IN_PW]; 

			if(nPW == 0)			//表明是sil或pau
				nSyl = 1;			
			else
			{
				// 若当前韵律词的音节数大于3，则输出3个音节
				if( (nPW - nPWIndex)  > MAX_SYNTH_LAB_COUNT)
					nSyl = MAX_SYNTH_LAB_COUNT;		
				else
					nSyl = (nPW - nPWIndex);
			}

			
			if( g_hTTS->m_FirstSynthSegCount == 1 && nStartLine == 1  
				&& nMaxLine != 3)   //不是1字句
			{
				//进入emTTS_SynthText函数首次合成的音节数的最小值（否则会有声音卡的情况）
				//FIRST_MIN_SYNTH_LAB_COUNT必须为3（若等于2，以下例句等会卡）  hyl  2012-04-12
				//例如：你的余额。你的书包。[s10]我的书包。他的余额。你的书。[d]    （因为句首2个字的音太短（第2个字为轻声））
				if( nSyl <  FIRST_MIN_SYNTH_LAB_COUNT)   
					nSyl =  FIRST_MIN_SYNTH_LAB_COUNT;
			}


			//若是合成文本中的第2个小分段合成，且前1个分段是2个字节，且目前分段nSyl为3个字节，则修改nSyl为2个字节
			//hyl 2012-03-31
			/*if( g_hTTS->m_FirstSynthSegCount == 2 && nStartLine == 3 && nSyl == 3)
				nSyl = 2;
			*/
			
			
#if !EM_SYNTH_ONE_WORD_PW				//不允许合成单字韵律词
			if( nSyl <= 2 )				//2012-10-08
			{	
				if( (nStartLine+nSyl) <= (nMaxLine-1) )		//不是句尾的最后1个汉字
				{		
					//若当前要合成的字数是1，则尽量将后面的字一起合成    hyl  2012-03-27
					nNextPW		 = g_pLabRTGlobal->m_LabRam[nStartLine+nSyl][RLAB_SYL_NUM_IN_C_PW];	//获取当前韵律词的音节数目	

					if( nNextPW == 0 )			//表明是sil或pau
						nNextPW = 1;			

					if( (nNextPW+nSyl) <= MAX_SYNTH_LAB_COUNT)
						nSyl += nNextPW;
					else
						nSyl = 2;   //2012-10-12
				}		
			}
#endif

			//
			//if( (nMaxLine - nStartLine) <= MAX_SYNTH_LAB_COUNT )			//当句尾只剩 < (2个字+sil)，则一起合成		2012-04-05
			//	nSyl = nMaxLine - nStartLine;
			//else if ( (nMaxLine - 2) == FIRST_MIN_SYNTH_LAB_COUNT )			//3字句（加前后sil共5个音节）：按3+1合成（否则会卡） hyl  2012-04-12  例如：[s10]你的书。[d]	
			//	nSyl = FIRST_MIN_SYNTH_LAB_COUNT;
			//else if( (nMaxLine - nStartLine) == (MAX_SYNTH_LAB_COUNT+1) )	//当句尾只剩 < (3个字+sil)，则按2+2合成		2012-04-05
			//	nSyl = 2;			

			//防卡：遇到pau，强制nSyl=3  hyl 2012-10-08
			if( nSyl == 2 )
			{
				if(    g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_SM] == INITIAL_CN_pau
					|| g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_SM] == INITIAL_CN_pau)
				{
					nSyl = 3;
				}

			}			
		}
	}	


	//   防止死机（内存不够）：例如：[m54][s0]XSX。[m55][s0]XSX。[m3][d]
	//此小分段若要合成3个音节，只要前两个音节是以下字母（x,w,s）
	//(这些字母的音长超过了最长汉字的音长（例如：晃，闯）)，则强制合成2个音节
	//防止《后端组合特征：《后端模块  》开辟空间多大（与音节时长有关），造成死机。
	//sqb  2017-5-26
	/*
	if( nSyl == 3 )
	{
		if(		(   g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_w)
			 &&	(   g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_w))
	
		{
			nSyl = 2;
		}
	}
	*/
	//循环从LabRam中读取
	for(i = nStartLine; i< (nMaxLine+1) ; i++)
	{
		// 如果音节数目到达限制，插入预定义pause，退出循环
		if( nLineCount == (nSyl + 1 ) )						
		{
			emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN);	
			nLineCount++;
			break;
		}
		// 拷贝一行label
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_pLabRTGlobal->m_LabRam[i], LAB_CN_LINE_LEN);	
		nLineCount++;
	}

	g_hTTS->m_FirstSynthSegCount++;		//本次合成文本第几个分段计数。（例如：本段中1个句子的第1个小分段 = 1）

	return nLineCount;
}
#endif


#if !EM_PW_SEGMENT_OPEN					//按固定字数分段合成
	
//从内存g_pLabRTGlobal->m_LabRam中寻找满足条件的小段，存入临时Lab中，等待分段合成	
emInt16	LocateSegment_Cn(emInt16 nStartLine, emInt16 nMaxLine) 
{
	emInt16 i, nLineCount=0;
	emInt16 nSyl;

	LOG_StackAddr(__FUNCTION__);

	emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN);
	nLineCount++;
	if(g_hTTS->m_FirstSynthSegCount == 1)
	{
		nSyl = FIRST_SYNTH_LAB_COUNT;
	}
	else
		nSyl = MAX_SYNTH_LAB_COUNT;

	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD )				//hyl 2012-03-27
	{
		//是：一字一顿效果
		if( nStartLine >= 1 )
			nSyl =  EM_STYLE_WORD_SYNTH_LAB_COUNT;		

	}


	//hyl 2012-10-08   防止死机（内存不够）：例如：[m54][s0]XSX。[m55][s0]XSX。[m3][d]
	//此小分段若要合成3个音节，只要前两个音节是以下字母（x,w,s）
	//(这些字母的音长超过了最长汉字的音长（例如：晃，闯）)，则强制合成2个音节
	//防止《后端组合特征：《后端模块  》开辟空间多大（与音节时长有关），造成死机。
	if( nSyl == 3 )
	{
		if(		(   g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_w)
			 &&	(   g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_w))
	
		{
			nSyl = 2;
		}
	}

	if( (nMaxLine - nStartLine) == 4)   //最后剩4个音节（含sil），按2+2合成  2012-10-08
		nSyl = 2;	


	for(i = nStartLine+1; i<= nMaxLine ; i++)
	{
		if( nLineCount == (nSyl + 1 ))
		{
			emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN);
			nLineCount++;
			break;
			
		}
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_pLabRTGlobal->m_LabRam[i-1], LAB_CN_LINE_LEN);
		nLineCount++;
	}


	if(nLineCount <= (nSyl + 1))
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount++], g_LabPau, LAB_CN_LINE_LEN);

	g_hTTS->m_FirstSynthSegCount++;		//本次合成文本第几个分段计数。（例如：本段中1个句子的第1个小分段 = 1）
	return nLineCount;
}
#endif




//获取每个小分段的总帧数（按分段合成）
emInt16 GetSegmentLength(emInt16 nStartLineOfPau ,emInt16 nLineCountOfPau)
{
	emInt16 i, j, t, length=0;

	LOG_StackAddr(__FUNCTION__);

	if(g_bRearSynChn)		//中文合成
	{
		for(i = 0; i< nLineCountOfPau ; i++)
		{
			
			//改为：将句尾的sil由原来的仅决策不输出改成输出前5帧（25ms），为改善句尾音节的杂音（突然终止，没有归于零）
			
			// 此处为句尾输出的sil帧数，可调节，但注意不能太大（LSP模型会出现句尾超大杂音，MGC模型不会）
					
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == SIL_INITIAL_NO)
			{
				if(nStartLineOfPau + nLineCountOfPau - 2 == g_pLabRTGlobal->m_MaxLabLine )	//是句尾sil
				{
					//modified by hyl  2012-04-06
					for(t= 0 ;t< (LAST_SIL_MUTE/5) ;t++)
					{
						if( t > 5)		//hyl 2012-04-10  由2改成5
							g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1：不输出声音
						else
							g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2：输出声音但能量改小
					}				
				}
				else													//是句首sil
				{
					//句首sil输出控制  t = ?  ( 从第几帧起输出0)  hyl  2012-04-05	
					for(t=0;t< ((FRONT_SIL_MUTE/5)) ;t++)				  //最好最后1帧要输出（否则m，n，w开头的句首音会有gong4音）
						g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1：不输出	

				}
			}


			for(j=0; j< g_hTTS->pInterParam->nState ; j++)
			{
				//策略：Sil的时长固定，不同的标点符号应静音多长时间，由前端模块去控制  20110527
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == SIL_INITIAL_NO)	
				{
					if( nStartLineOfPau == 0 )
						g_pRearRTGlobal->m_PhDuration[i][j] = FRONT_SIL_MUTE/50;		//一句话的句首sil在后端的时长
					else
						g_pRearRTGlobal->m_PhDuration[i][j] = LAST_SIL_MUTE/50;			//一句话的句尾sil在后端的时长
				}

				//策略：缩短Pau的时长  20110526
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == PAU_INITIAL_NO)	
				{
					g_pRearRTGlobal->m_PhDuration[i][j] = (emInt32 )( ( g_pRearRTGlobal->m_PhDuration[i][j] + 2 ) / 3 );  //需保证每个状态至少有1帧
					
					if(  i!=0 && i!=(nLineCountOfPau-1))  //且不是前后加的小pau
					{
						//pau输出控制  j >= ?  ( 从第几状态起输出0)	
						//modified by hyl  2012-04-06
						if( j >= 2 )			//hyl 2012-04-10  由2改成2
						{
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1：不输出声音
						}
						else
						{
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2：输出声音但能量改小
						}
					}
				}			

				length += g_pRearRTGlobal->m_PhDuration[i][j];

			}
		}
	}
	else		//英文合成
	{
		//以下是英文的：参照中文的策略

		for(i = 0; i< nLineCountOfPau ; i++)
		{			
			if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silt)  //是句尾sil
			{
				for(t= 0 ;t< (LAST_SIL_MUTE/5) ;t++)
				{
					if( t > 5)		
						g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1：不输出声音
					else
						g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2：输出声音但能量改小
				}				
			}

			if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silh)  //是句首sil
			{
				for(t=0;t< ((FRONT_SIL_MUTE/5)) ;t++)				 
					g_pRearRTGlobal->m_VADflag[length+t] = 1;			//=1：不输出	
			}

			for(j=0; j< g_hTTS->pInterParam->nState ; j++)
			{
				if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silh )	
					g_pRearRTGlobal->m_PhDuration[i][j] = FRONT_SIL_MUTE/25;		//一句话的句首sil在后端的时长

				if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silt)	
					g_pRearRTGlobal->m_PhDuration[i][j] = LAST_SIL_MUTE/25;			//一句话的句尾sil在后端的时长

#if HTRS_REAR_DEAL_SP
				if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_sp)	
				{					
					if(  i!=0 && i!=(nLineCountOfPau-1))  //不是前后加的小pau
					{	
						g_pRearRTGlobal->m_PhDuration[i][j] *= 2;		//sp的时长增大一倍
						if( j >= 2 )			
						{							
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1：不输出声音
						}
						else
						{
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2：输出声音但能量改小
						}
					}
					else										 
						g_pRearRTGlobal->m_PhDuration[i][j] = 1;  //是前后加的小pau,需保证每个状态至少有1帧

				}	
#endif

				length += g_pRearRTGlobal->m_PhDuration[i][j];

			}
		}
	}
	return length;
}

//获取每个小分段的浊音帧数（按分段合成） 
emInt16 GetSegmentMsdLength( emInt16 PauLength)
{
	emInt16 t, length=0;

	LOG_StackAddr(__FUNCTION__);

	//按整个合成段循环
	for(t = 0; t< PauLength ; t++)
	{
		//遇到浊音帧累加
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED) 
		{
			length ++;

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_GONG4_GONG4

			//以下if：为改善gong4音：修改MSD策略：所有浊音帧再提前一帧
			if ( t>2 && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED && g_pRearRTCalc->m_MSDflag[t-2] == UNVOICED ) 
			{
				g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
				length++;				
			}
#endif
			}	// end of g_bRearSynChn
		}
	}
	return length;
}

