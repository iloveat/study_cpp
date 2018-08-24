#include "emPCH.h"


#include "emTTS_Common.h"

extern emBool global_bStopTTS;


//*****************************************	 全局变量定义  ************************************************************************

emInt8				g_bIsUserAddHeap;					//用户是否提供了多余的内存量（适合内存空间富裕的客户，可将后端22颗决策树全部载入到内存）

struct emHTTS_G		*g_hTTS;							//emTTS的全局总体变量
PLabRTGlobal		g_pLabRTGlobal;						//LAB内存全局变量
struct tagIsHeap	*g_pHeap;							//堆的全局指针

struct TextItem     *g_pTextInfo;						//文本链表：汉字信息
emByte				*g_pText;							//文本链表：汉字
emInt16				*g_pTextPinYinCode;					//文本链表：拼音编码并拼音编码 （最高1位:拼音类型）  -----  拼音类型： 0：默认；1：强制拼音
emUInt8				g_nCurTextByteIndex;				//文本链表：当前字的索引
emUInt8				g_nLastTextByteIndex,  g_nNextTextByteIndex, g_nP1TextByteIndex, g_nP2TextByteIndex;  //文本链表：汉字信息索引

struct ForcePPHInfo	g_ForcePPH;							//强制PPH（因：成对标点符号）   例如：他看了《红楼梦》后很开心

emUInt32	       (*g_pOutputCallbackFun)( emPointer , emUInt16,emPByte,emSize);  //音频回调函数指针，sqb 32->16
emUInt32	       (*g_pProgressCallbackFun)( emPointer , emUInt32,emSize);  //音频回调函数指针


#if EM_USER_VOICE_RESAMPLE
TReSampleData		g_objResample[1];					// 重采样对象 
#endif


#if DEBUG_LOG_SWITCH_ERROR
FILE				*g_fLogError;						//错误日志文件指针
#endif


//音频输出函数
emUInt32	emTTS_OutputVoice(emPointer pParameter, emUInt16 nCode, emPByte pcData, emSize nSize)
{
	emUInt32 nErrorID;
	emInt16 *nPcmOut;
	emSize nSizeOut;
	emSize nReadLen;

	#define RESAMPLE_COUNT_EACH_TIME		1024		//每次重采样的采样点个数:注意：设置成256时，会有杂音 （ [m55]美国当地时间20日。）

#if DEBUG_LOG_SWITCH_TIME
	//myTimeTrace_For_Debug(1,(emByte *)"后端--小段播前", 0);
#endif

	LOG_StackAddr(__FUNCTION__);

	nReadLen = nSize;

	g_hTTS->m_bIsStartBoFang = emTrue;

	//已停止则不再输出 
	if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue)
	{
		return emTTS_ERR_EXIT;

	}


#if !EM_USER_VOICE_RESAMPLE			//正常输出，不重采样

	nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, pcData, nSize); 

	return nErrorID;


#else								//重采样输出

	if( g_hTTS->m_bResample != emTrue)					//若不是虚拟发音人，无需重采样
	{
		nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, pcData, nSize); 

#if DEBUG_LOG_SWITCH_TIME
		//myTimeTrace_For_Debug(1,(emByte *)"后端--小段播后", 0);
#endif

		return nErrorID;
	}
	else
	{
		nReadLen = nReadLen >> 1;

#if DEBUG_LOG_SWITCH_HEAP
		nPcmOut = (emPInt16)emHeap_AllocZero( RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16), "重采样输出：《解码模块  》");
#else
		nPcmOut = (emPInt16)emHeap_AllocZero( RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16));
#endif

		while ( nReadLen > RESAMPLE_COUNT_EACH_TIME )
		{

			nSizeOut = ReSample(g_objResample, pcData , nPcmOut, RESAMPLE_COUNT_EACH_TIME);

			g_hTTS->m_nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, (emPByte)nPcmOut, nSizeOut*2 ); 
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				return emTTS_ERR_EXIT;
			}
			nReadLen -= RESAMPLE_COUNT_EACH_TIME;
			pcData = (emPointer)((emPInt16)pcData + RESAMPLE_COUNT_EACH_TIME);
		}

		nSizeOut = ReSample(g_objResample, pcData, nPcmOut, nReadLen);

		g_hTTS->m_nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, (emPByte)nPcmOut, nSizeOut*2 ); 
		if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
		{
			return emTTS_ERR_EXIT;
		}
		
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free( nPcmOut, RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16), "重采样输出：《解码模块  》");
#else
		emHeap_Free( nPcmOut, RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16));
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"后端--小段播后", 0);
#endif

		return emTTS_ERR_OK;

	}

#endif		//end: EM_USER_VOICE_RESAMPLE
}



// 功能：		判断当前句最终识别成什么句（2选1）：中文句，英文句
// 用到的数据：	1. g_hTTS->m_pDataAfterConvert，即转换后的一句的内容；
//				2. 选择的语种类别g_hTTS->m_ControlSwitch.m_nLangType，受语种控制标记（[g1][g2][g0]）控制
//				3. 前一句的语种类别g_hTTS->m_nPrevSenType
// 返回值：		SEN_TYPE_CN（中文句）；SEN_TYPE_EN（英文句）。并实时更新g_hTTS->m_nPrevSenType的值，即记录当前句的语种类别
emInt8	DecideSenType()
{
	emInt16 nIndex, nSumYW;
	emInt16 nDataType, nWordp, nWordn;
	emInt8 nSenType;
	
	// 中文句判断：至少含有1个汉字（含汉字一定是中文句）
	// 例子：他们在PK。UFO是不明飞行物。 Tom和Hellen是好朋友。他们使用的是Windows Mobile平台。Windows Mobile平台很好用。
	nIndex=(emInt16)0;
	while((nDataType = CheckDataType(g_hTTS->m_pDataAfterConvert,nIndex)) != (emInt16)DATATYPE_END)
	{
		if(nDataType == (emInt16)DATATYPE_HANZI)
		{
			g_hTTS->m_nPrevSenType = SEN_TYPE_CN;
			return SEN_TYPE_CN;
		}
		nIndex += 2;
	}


	/** 【不分句的符号后的空格已被去掉，影响英文句的判断】 **/

	// 英文句判断：：有1个连续的英文串（长度>1），且其前后没有数字和符号"."（但句尾的符号"."不算）
	// 例子：apple e.g. ok.hello. Adm. 5 is 5.ok. ok .	is a.Mrs. Hellen.【Mrs. Hellen.】Mrs（【】中的会被认为是不识别句）.
	//       HR&Adm.hi-fi.CD-ROM.E-mail.
	nIndex = (emInt16)0;
	while((nDataType = CheckDataType(g_hTTS->m_pDataAfterConvert,nIndex)) != (emInt16)DATATYPE_END)
	{
		if(nDataType == (emInt16)DATATYPE_YINGWEN)
		{
			nSumYW = GetStrSum(g_hTTS->m_pDataAfterConvert,nIndex,(emInt16)DATATYPE_YINGWEN);
			if(nSumYW > 1)		// 有1个连续的英文串（长度>1）
			{
				nWordp = GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)(nIndex-2));			// 取连续字母串的前一个字
				nWordn = GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)(nIndex+2*nSumYW));	// 取连续字母串的后一个字
				if((((nWordp < (emInt16)0x3a3b0)		// 前没有数字和符号"."(0xa3ae)
					|| (nWordp > (emInt16)0xa3b9))
					&& (nWordp != (emInt16)0xa3ae))
					&&
					(((nWordn < (emInt16)0xa3b0)		// 后没有数字和符号"."（但句尾的符号"."不算）
					|| (nWordn > (emInt16)0xa3b9))
					&& ((nWordn != (emInt16)0xa3ae) ||		// 句末的“.”（0xa3ae）有两种情况（尚不清楚原因）：1)句末的“.”后直接跟结束符（0xffff）；2)句末的“.”加一个空格（0xa3a0）后再跟结束符（0xffff）
						((GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)nIndex+2*nSumYW+2) == (emInt16)END_WORD_OF_BUFFER)
						|| 
						((GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)nIndex+2*nSumYW+2) == (emInt16)0xa3a0)
						&&
						(GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)nIndex+2*nSumYW+4) == (emInt16)END_WORD_OF_BUFFER))))))
				{
					g_hTTS->m_nPrevSenType = SEN_TYPE_EN;
					return SEN_TYPE_EN;
				}
			}
			nIndex = nIndex + 2*nSumYW;
		}
		else
			nIndex += 2;

	}


	// 不识别句（受控制标记影响）
	// 例子：WWW.sohu.com.cn.3.5kg. mp3.
	//       a. 3.5kg. b. 120cm. .W.C. WWW.sohu.com 。： WWW.sohu.com.cn 。 3.5kg .
	switch (g_hTTS->m_ControlSwitch.m_nLangType)
	{
	case (emInt8)emTTS_LANG_CN:			// 选择汉语语种[g1]：按中文句处理
		nSenType = SEN_TYPE_CN; break;
	case (emInt8)emTTS_LANG_EN:			// 选择英文语种[g2]：按英文句处理
		nSenType = SEN_TYPE_EN; break;
	case (emInt8)emTTS_LANG_AUTO:		// 若选择自动识别语种[g0]：延续前1句的读法（前1句是中文句，则按中文句处理；类推）
		nSenType = g_hTTS->m_nPrevSenType; break;
	default:
		 nSenType = SEN_TYPE_CN;
	}

	g_hTTS->m_nPrevSenType = nSenType;		// 记录当前句的语种类别

	return nSenType;
	
}



//功能    ：判断并播放有效提示音
//调用前提：1)打开提示音标记；2)连续英文串个数为4至6个； 给定文本必须是全角小写字母
//返回值  ：为0：表示不是有效的提示音；>0表示：有效提示音的长度且提示音已发音
emInt8 JudgePrompt(emInt16 nSumOfYingWen,emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nErrorID;
	emInt16 nWord,nWord2;
	emByte  strNeedCmp[11]="";
	emInt16 nTemp; 
	emByte  buffSoundShuZi[4];
	emInt16 nSoundShuZi;

	LOG_StackAddr(__FUNCTION__);

	//需判断是否特殊类型： msga, msgb,....
	if( nSumOfYingWen == 4 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  ( nSumOfYingWen - 1 )*2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID != emTTS_ERR_FAILED )
		{
			if( emMemCmp("ｍｓｇ",strNeedCmp,6  ) == 0 )
			{
				//调用提示音msg发音
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 6);
				nTemp = PlayPromptPreRecordVoice( PROMPT_MSG, 0, nWord - 'ａ', g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);
				if( nTemp != 0 )				
					return 8;	//是有效提示音，已发完音
			}
		}
	}

	//需判断是否特殊类型： sounda, soundb,....
	if( nSumOfYingWen == 6 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  ( nSumOfYingWen - 1 )*2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID != emTTS_ERR_FAILED )
		{
			if( emMemCmp("ｓｏｕｎｄ" ,strNeedCmp,10 ) == 0 )
			{
				//调用提示音sound发音
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 10);
				if( nWord  > (emInt16)0xa3f3 )
					nTemp = PlayPromptPreRecordVoice( PROMPT_SOUND, 1, nWord - 'ａ' - 1 , g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);
				else if( nWord  == (emInt16)0xa3f3 )		//注意： sounds不是有效提示音  (0xa3f3: 'ｓ')
					nTemp = 0;
				else
					nTemp = PlayPromptPreRecordVoice( PROMPT_SOUND, 1, nWord - 'ａ', g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);

				if( nTemp != 0 )
					return 12;		//是有效提示音，已发完音
			}
		}
	}

	//需判断是否特殊类型： sound101, sound102, sound103,....
	if( nSumOfYingWen == 5 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  ( nSumOfYingWen )*2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID != emTTS_ERR_FAILED )
		{
			if( emMemCmp("ｓｏｕｎｄ", strNeedCmp,10   ) == 0 )
			{
				//若sounda后面带3位数字，则按预录音发音
				if(    CheckDataType(strDataBuf ,nCurIndexOfBuf+10)==DATATYPE_SHUZI
					&& GetShuZiSum(strDataBuf ,nCurIndexOfBuf+10)==3 )
				{
					buffSoundShuZi[0] = GetWord(strDataBuf,nCurIndexOfBuf+10)-0xa3b0;		//0xa3b0 = 字符 “0”
					buffSoundShuZi[1] = GetWord(strDataBuf,nCurIndexOfBuf+12)-0xa3b0;
					buffSoundShuZi[2] = GetWord(strDataBuf,nCurIndexOfBuf+14)-0xa3b0;
					buffSoundShuZi[3] = 0;

					nSoundShuZi  = buffSoundShuZi[0]*100+buffSoundShuZi[1]*10+buffSoundShuZi[2];		//hyl 2012-03-26 修改：不适用atoi函数实现（安凯平台用atoi函数会出问题）

					if( nSoundShuZi < 1000 && nSoundShuZi > 100 &&  (emInt16)(nSoundShuZi/100)*100 != nSoundShuZi )
					{
						//调用提示音sound发音
						nTemp = (emInt16)(nSoundShuZi/100);		//除100后取整   (求得提示音的二级类别)
						nSoundShuZi -= nTemp*100 + 1;			//除100后取余数 (求得提示音类别下的相对个数偏移)
						nTemp = PlayPromptPreRecordVoice( PROMPT_SOUND,nTemp, nSoundShuZi, g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);
						
						if( nTemp != 0 )
							return 16;			//是有效提示音，已发完音					
					}
				}
			}
		}
	}

	return 0;	//不是有效提示音
}


//功能    ：识别控制标记并设置全局变量
//调用前提：当前符号是“[”
//返回值  ：为0：表示不是有效的控制标记；>0表示：有效控制标记的长度且已设置全局控制变量
emInt8 JudgeCtrlMark(emInt16 nSumOfFuHao,emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nNWord,nNNWord,nWord,nSumOfShuZi,nWordTemp;

	if( CheckDataType( strDataBuf, nCurIndexOfBuf + 2 ) == DATATYPE_YINGWEN )		//“[”后面的字符是: 字母
	{ 
		nNWord  = GetWord( strDataBuf, nCurIndexOfBuf + 2 );
		nNNWord = GetWord( strDataBuf, nCurIndexOfBuf + 4 );

		//第1个字母：大写转小写
		if(  nNWord >= (emInt16)0xa3c1 && nNWord <= (emInt16)0xa3da)		
			nNWord += 0x20;				

		if( nNWord == (emInt16)0xa3e4 && nNNWord == (emInt16)0xa3dd)  //判断是否是：[d]
		{
			IsValidMarkAndPlayPrev();		//是有效的控制标记，在改变参数之前对PCM中已有的数据进行语音播放
			InitTextMarkG();				//设置《控制标记》相关的全局参数			
			return 6;						//是有效控制标记
		}

		//"[*"后面是 数字的情况下
		if ( nNNWord >= (emInt16)0xa3b0 && nNNWord <= (emInt16)0xa3b9 )
		{	
			nSumOfShuZi = GetShuZiSum( strDataBuf, nCurIndexOfBuf + 4 );//后面有连续的几位数字
			nWord = GetWord( strDataBuf, nCurIndexOfBuf + 4 + 2 * nSumOfShuZi );
			if ( nWord == ( emInt16 ) 0xa3dd  )//判断是否是“]”控制标记；有效的控制标记
			{
				nWordTemp = ChangeShuZiToLong( strDataBuf, nCurIndexOfBuf + 4 );

				if( nNWord == (emInt16)0xa3f0 )  //[p*]
					g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;						//遇到[p*]停顿不输出本句的句尾sil

				IsValidMarkAndPlayPrev();

				if( nNWord == (emInt16)0xa3f0 )  //[p*]
					g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil   = emTrue;					//遇到[p*]停顿不输出本句的句首sil
	
				switch (nNWord)
				{					
					case ( emInt16 ) 0xa3e2:	//[b	设置标点是否读出
						if(nWordTemp==1)								
							g_hTTS->m_ControlSwitch.m_bPunctuation = emTTS_PUNCTUATION_READ ;
						else						
							g_hTTS->m_ControlSwitch.m_bPunctuation = emTTS_PUNCTUATION_NO_READ ;
						break;		

					case ( emInt16 ) 0xa3e5:	//[e]	设置音效模式//[e*](*=0/1/2/3/4/5/6)
						if((nWordTemp>=0)&&(nWordTemp<=6))
							g_hTTS->m_ControlSwitch.m_nVoiceMode=(emInt8)nWordTemp;
						else
							g_hTTS->m_ControlSwitch.m_nVoiceMode=emTTS_USE_VoiceMode_CLOSE;///音效模式 关闭
						break;

					case ( emInt16 ) 0xa3e6:	//[f	设置文本朗读方式
						if(nWordTemp==0)
							g_hTTS->m_ControlSwitch.m_nSpeakStyle = emTTS_STYLE_WORD ;
						else
							g_hTTS->m_ControlSwitch.m_nSpeakStyle = emTTS_STYLE_NORMAL ;
						break;							

					case ( emInt16 ) 0xa3e9:	//[i	设置识别拼音处理策略
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = emTTS_USE_PINYIN_OPEN;
						else
							g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = emTTS_USE_PINYIN_CLOSE ;
						break;

					case ( emInt16 ) 0xa3ed:	//[m*]	选择发音人///[m*] (*=3，51，52，53，54，55)
						if(    nWordTemp == 3  || nWordTemp == 51 || nWordTemp == 52 
							|| nWordTemp == 53 || nWordTemp == 54 || nWordTemp == 55 ) 
						{
							g_hTTS->m_ControlSwitch.m_nRoleIndex=(emInt16)nWordTemp;
						}
						else
							g_hTTS->m_ControlSwitch.m_nRoleIndex=emTTS_USE_ROLE_XIAOLIN;

#if EM_8K_MODEL_PLAN	//8K模型配置		只支持发音人[m3] [m51] [m52]
						if( nWordTemp == 53 || nWordTemp == 54 || nWordTemp == 55 ) 
							g_hTTS->m_ControlSwitch.m_nRoleIndex=emTTS_USE_ROLE_XIAOLIN;
#endif
						break;

					case ( emInt16 ) 0xa3ee:	//"[n"	设置数字处理策略						
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_nReadDigitPolicy = emTTS_READDIGIT_AS_NUMBER ;
						else if(nWordTemp==2)
							g_hTTS->m_ControlSwitch.m_nReadDigitPolicy = emTTS_READDIGIT_AS_VALUE ;
						else
							g_hTTS->m_ControlSwitch.m_nReadDigitPolicy = emTTS_READDIGIT_AUTO ;	
						break;

					case ( emInt16 ) 0xa3f0 :	//[p*]	设置停顿[p*]
						g_hTTS->m_CurMuteMs = nWordTemp;		
						break;

					case ( emInt16 ) 0xa3f2:	//[r]	姓氏策略						
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_JUSHOU_OPEN ;	//以后每句的句首强制读成姓氏
						else if(nWordTemp==2)
							g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_AFTER_OPEN ;		//仅紧跟句的句首强制读成姓氏
						else
							g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_CLOSE;			//自动判断
						break;

					case ( emInt16 ) 0xa3f3 :	//"[s"	设置语速
						if(nWordTemp>emTTS_SPEED_MAX)
							nWordTemp=emTTS_SPEED_NORMAL;
						g_hTTS->m_ControlSwitch.m_nVoiceSpeed = (emInt8)nWordTemp ;
						break;

					case ( emInt16 ) 0xa3f4:	//"[t"	设置语调
						if(nWordTemp>emTTS_PITCH_MAX)
							nWordTemp=emTTS_PITCH_NORMAL;
						g_hTTS->m_ControlSwitch.m_nVoicePitch = (emInt8)nWordTemp ;
						break;

					case ( emInt16 ) 0xa3f6:	//"[v"	设置音量（含提示音量）											
						if(nWordTemp>emTTS_VOLUME_MAX)
							nWordTemp=emTTS_VOLUME_NORMAL;
						g_hTTS->m_ControlSwitch.m_nVolumn = (emInt8)nWordTemp ;					
						break;

					case ( emInt16 ) 0xa3f8:	//"[x"	设置提示音处理策略						
						if(nWordTemp==0)
							g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy = emTTS_USE_PROMPTS_CLOSE ;
						else
							g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy = emTrue;//emTTS_USE_PROMPTS_OPEN ;
						break;			

					case ( emInt16 ) 0xa3f9:	//"[y"	设置号码中“1”的读法
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI ;
						else
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO ;
						break;

					case ( emInt16 ) 0xa3fa:	//"[z"	设置韵律标注处理策略
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_nManualRhythm=emTTS_USE_Manual_Rhythm_OPEN;
						else
							g_hTTS->m_ControlSwitch.m_nManualRhythm=emTTS_USE_Manual_Rhythm_CLOSE;
						break;
					case ( emInt16 ) 0xa3ef:	//"[o"	设置英文0的号码读法
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bZeroPolicy=emTTS_EN_0_OU;
						else
							g_hTTS->m_ControlSwitch.m_bZeroPolicy=emTTS_EN_0_ZERO;
						break;

					case ( emInt16 ) 0xa3e7:	//"[g"	设置语种
						if(nWordTemp==1)
						{
							g_hTTS->m_ControlSwitch.m_nLangType=emTTS_LANG_CN;
							g_hTTS->m_nPrevSenType = SEN_TYPE_CN;		
						}
						else if(nWordTemp==2)
						{
							g_hTTS->m_ControlSwitch.m_nLangType=emTTS_LANG_EN;
							g_hTTS->m_nPrevSenType = SEN_TYPE_EN;	
						}
						else
							g_hTTS->m_ControlSwitch.m_nLangType=emTTS_LANG_AUTO;
						break;
					default:
						return 0;	//不是有效的控制标记：符合控制标记表示规范，但不在范围内
					
				}
				return (nSumOfShuZi*2 + 6);	//是有效的控制标记
			}			
		}	
	}

	return 0;		//不是有效的控制标记：不符合控制标记表示规范
}