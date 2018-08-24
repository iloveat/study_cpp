#include "emPCH.h"

#include "Front_ProcYingWen.h"


emInt16 emCall ProcessYingWen( emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nSumOfYingWen = 0 ,  nWord,nWord2;
	emByte  strNeedCmp[11]="";
	emInt16 nErrorID;
	emInt16 nSumOfShuZi = 0;

	emInt16 nTempOfSum,nSoundShuZi;

	emBool    bHaveOnlyPinYin;
	emByte   *pnShengMuNo;	    //指针传参：有效拼音的声母编码，初始值为0
	emByte   *pnYunMuNo;		//指针传参：有效拼音的韵母编码，初始值为0
	emByte   *pnShengDiaoNo;    //指针传参：有效拼音的声调编码，初始值为0
	emInt16   nValueOfHandelPinYinReturn;
	emByte    PinYin[3] ;

	emByte    cHighByte = 0;                       //用于临时存放表格中的高字节
	emByte    cLowByte = 0;                        //用于临时存放表格中的低字节
	emInt16   nStartOfYinSe = 0;                  //某一拼音所对应的音色起始行号
	emByte    cNumOfYinSe = 0;                    //某一拼音所对应的音色个数
	emInt16   nYinSeNum = 0;                      //某一拼音所对应的音色编号
	emInt16    nTemp; 
	emInt16 nPinYin;
	emInt8 nPromptLen;





	LOG_StackAddr(__FUNCTION__);

	pnShengMuNo = PinYin;
	pnYunMuNo = PinYin + 1;
	pnShengDiaoNo = PinYin + 2;



	nSumOfYingWen = GetYingWenSum( strDataBuf ,  nCurIndexOfBuf );


	//若为识别拼音的控制标记为识别[i1]
	if ( g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy == emTTS_USE_PINYIN_OPEN )
	{//且连续英文串的长度小等于6个  
      if ( nSumOfYingWen > 0 && nSumOfYingWen < 7 )
      {
        nTempOfSum = GetStrSum( strDataBuf,  nCurIndexOfBuf + 2*nSumOfYingWen , DATATYPE_SHUZI );
		if ( nTempOfSum = 1 )
		{//且后跟的连续数字串的长度等于1,则将英文串+数字串的内容传给《识别拼音函数--流程图》函数分析
			bHaveOnlyPinYin = emTrue;
			nValueOfHandelPinYinReturn=GetPinYinCode(strDataBuf,nCurIndexOfBuf,bHaveOnlyPinYin,pnShengMuNo,pnYunMuNo,pnShengDiaoNo );
			if ( nValueOfHandelPinYinReturn != 0 )
			{  //若是合格的拼音格式，则根据此函数返回的声母韵母声调信息按单字发音
			
				//声母韵母声调编号为0时，表示非汉字（虽然在汉字区）或者此汉字无拼音，这时将发空音
				if ( PinYin[0]==0 && PinYin[1]==0 && PinYin[2]==0  )
				{
					;		//不发音，直接丢弃			
					
				}
				else	//按单字发音
				{
					nPinYin = 0x8000 + (*pnYunMuNo)*256 + (*pnShengMuNo)*8 + (*pnShengDiaoNo); //赋值为强制拼音
					WriteToHanZiLink( (const emPByte)"丄", 0 ,POS_CODE_g, nPinYin );     //播放：强制拼音单字   此类拼音强制赋值为汉字“丄”，让它参与到汉字合成中去
				}
                
				nCurIndexOfBuf += nValueOfHandelPinYinReturn;
				return nCurIndexOfBuf;		
			}
            //若不是合格拼音，按普通英文发音
		}
      }
	}

    if ( nCurIndexOfBuf > 1 )
    {  //净重(kg),外形尺寸(mm):400×280×860 (min2) 。 （G nter）。
		nWord  = GetWord(strDataBuf,nCurIndexOfBuf - 2);
		nWord2 = GetWord(strDataBuf,nCurIndexOfBuf + nSumOfYingWen*2);		//hyl 2012-04-13
		if (   (nWord == (emInt16)0xa3a8 && nWord2 == (emInt16)0xA3A9 )		//是符号：  （	）
			|| (nWord == (emInt16)0xa3db && nWord2 == (emInt16)0xA3DD )		//是符号：  ［	］
			|| (nWord == (emInt16)0xa3fb && nWord2 == (emInt16)0xA3FD )		//是符号：  ｛	｝
			|| (nWord == (emInt16)0xa1be && nWord2 == (emInt16)0xA1BF ) )	//是符号：  【	】
		{
		  nTempOfSum = CheckDanWeiTable(strDataBuf, nCurIndexOfBuf);
          if ( nTempOfSum != 0 )//英文串单位
		  {
			  SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：字母单位
			  ClearBuffer( g_hTTS->m_ShuZiBuffer,  11 );			//清空数字缓存中存储的内容
			  nCurIndexOfBuf +=  nTempOfSum * 2 ;
			  return nCurIndexOfBuf;
		  }
		}
    }


	//需判断是否特殊类型： 40.3°C   ， 其中C不发音
	if( nSumOfYingWen == 1 )
	{
		nWord = GetWord(strDataBuf,nCurIndexOfBuf );
		if(nWord == (emInt16)0xa3e3 )
		{
			nWord = GetWord(strDataBuf,nCurIndexOfBuf-2 );
			if(nWord == (emInt16)0xa1e3)   
			{
				nCurIndexOfBuf +=  2 ;
				return nCurIndexOfBuf;
			}
		}
	}





	//需判断是否特殊类型： qq 或 ip
	if( nSumOfYingWen == 2 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf, nCurIndexOfBuf,  nSumOfYingWen*2
			, sizeof(strNeedCmp) ) ;
		if( nErrorID == emTTS_ERR_FAILED )
		{
			//按普通英文发音
			nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf,  nCurIndexOfBuf, nSumOfYingWen );
			return nCurIndexOfBuf;
		}
		else
		{
			//如果恰好是qq
			if( emMemCmp("ｑｑ",strNeedCmp,4   ) == 0 || emMemCmp("ｉｐ" ,strNeedCmp,4 ) == 0)
			{
				g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
				//qq按普通英文发音
				nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );

				return nCurIndexOfBuf;

			}


		}
	}	

	//需判断是道路名：G国道、S省道、X县道、Y乡道、Z专用公路、Q公路：后跟3位数字
	//编号区间：公路路线编号区间国道为Ｇ１０１至Ｇ１９９、Ｇ２０１至Ｇ２９９、Ｇ３０１至Ｇ３９９；
	//			省道为Ｓ１０１至Ｓ１９９、Ｓ２０１至Ｓ２９９、Ｓ３０１至Ｓ３９９；
	//			县、乡、专用公路及其他公路为Ｘ／Ｙ／Ｚ／Ｑ００１至Ｘ／Ｙ／Ｚ／Ｑ９９９。
	//需注意：兼容火车列车表    hyl  2012-03-30
	//			D121次列车。K121次列车。L121次列车。N121次列车。T121次列车。X121次列车。Y121次列车。Z121次列车。
	if( nSumOfYingWen == 1 )
	{

		nTempOfSum = GetStrSum( strDataBuf ,  nCurIndexOfBuf+2,  DATATYPE_SHUZI);
		if( nTempOfSum == 3)
		{
			nWord = GetWord(strDataBuf,nCurIndexOfBuf);
			if(    nWord == (emInt16)0xa3e7 || nWord == (emInt16)0xa3f3 )		//字母： G，  S
				//|| nWord == (emInt16)0xa3f8 || nWord == (emInt16)0xa3f9
				//|| nWord == (emInt16)0xa3fa || nWord == (emInt16)0xa3f1 )
			{
				nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
				g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
				ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+2);
				g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;		

				if( nWord == (emInt16)0xa3e7 )	//ｇ110
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb9fa ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//国道			
				}
				if( nWord == (emInt16)0xa3f3 )	//ｓ110
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcaa1 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//省道				
				}

				//以下为了兼容火车列车表，不能要。   hyl  2012-03-30
				//if( nWord == (emInt16)0xa3f8 )	//ｘ110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcfd8 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//县道					
				//}
				//if( nWord == (emInt16)0xa3f9 )	//ｙ110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcfe7 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//乡道				
				//}
				//if( nWord == (emInt16)0xa3fa )	//ｚ110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd7a8 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xd3c3 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, 0xb9ab ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,6, 0xc2b7 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,8, END_WORD_OF_BUFFER );		//专用公路				
				//}
				//if( nWord == (emInt16)0xa3f1 )	//ｑ110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb9ab ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xc2b7 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//公路			
				//}

				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//播放
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 11);

				nCurIndexOfBuf += 8;
				return nCurIndexOfBuf;
			}
	
		}		
	}	



	//需判断是否特殊类型： www.
	if( nSumOfYingWen == 3 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  nSumOfYingWen *2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID == emTTS_ERR_FAILED )
		{
			//按普通英文发音
			nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );
			return nCurIndexOfBuf;
		}
		else
		{
			//如果恰好是www 
			if( emMemCmp("ｗｗｗ" ,strNeedCmp,6 ) == 0 )
			{				
				nWord = GetWord( strDataBuf , nCurIndexOfBuf + 6 );
				//判断www后面是不是"."
				if( nWord == ( emInt16 )0xa3ae )
				{
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;

					//发音：“www”
					//nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );

					//发音：“三w”
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc8fd); //三
					PutIntoWord( g_hTTS->m_ShuZiBuffer,   2, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：三
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf+4, 1 );

					return nCurIndexOfBuf;

				}
			}
		}
	}


	//是否需要处理提示音
	if( g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy == emTrue )
	{
		if( nSumOfYingWen >=4 && nSumOfYingWen <= 6)
		{

			nPromptLen = JudgePrompt(nSumOfYingWen, strDataBuf, nCurIndexOfBuf);		//判断并播放有效提示音
			if( nPromptLen > 0 )
				return (nCurIndexOfBuf+nPromptLen);
		}
	}


	//需判断是否特殊类型：例如： ISO-8859-1。兼容GB2312-80。
	nWord = GetWord(strDataBuf,nCurIndexOfBuf+nSumOfYingWen*2 );
	if(nWord == (emInt16)0xa3ad )				//字母串后是符号：  -
	{
		if( DATATYPE_SHUZI ==CheckDataType(strDataBuf,nCurIndexOfBuf+nSumOfYingWen*2 +2)) //符号"-"后是数字
		{
			g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
			
			//按普通英文发音
			nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );
			return nCurIndexOfBuf;

		}
	}



	//不属于以上任何一种情况，则按普通英文发音

	nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );  //处理完英文后，指针指向英文串之后的字符
	return nCurIndexOfBuf;
}

////***************************************************************************************************************
////功能：英文后面紧跟3位或以上数字读成号码且1读成幺（允许有1个空格*/)，按普通英文发音
////输入参数：当前英文串emPByte pCurStr，相对位移量emInt16  nCurIndex，英文串长度 nLen
////返回：无返回值
////***************************************************************************************************************
//emInt16 emCall PlayYingWenVoice( emPByte pCurStr, emInt16  nCurIndex , emInt16 nLen )
//{
//
//	emInt16   nShuZiLen;
//
//	emInt16 countUsal;
//
//	LOG_StackAddr(__FUNCTION__);
//
//	//nLen = GetYingWenSum( pCurStr,  nCurIndex );
//	//nWord = GetWord( pCurStr, nCurIndex + 2 * nLen );
//
//	//if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9 )  //英文后第一位是0到9的数字
//	//{
//	//	nWord = GetWord( pCurStr, nCurIndex + 2 * ( nLen + 1 ) );
//	//	if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9 )  //英文后第二位是0到9的数字
//	//	{
//	//		nWord = GetWord( pCurStr, nCurIndex + 2 * ( nLen + 2 ) );
//	//		if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9 )  //英文后第三位是0到9的数字
//	//		{
//	//			g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
//	//			g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
//	//			//nCurIndex += 2 * nLen;
//	//			//return nCurIndex;
//	//		}
//	//	}
//	//}
//
//	nLen = GetStrSum( pCurStr,  nCurIndex ,DATATYPE_YINGWEN);
//	nShuZiLen = GetStrSum( pCurStr, nCurIndex + 2 * nLen, DATATYPE_SHUZI);		//英文串后为3位数字
//	if( nShuZiLen >= 2)
//	{
//		g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
//		g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
//	}
//
//
//
//	//按普通英文发音-- -- 2011-02-15修改  为实现功能：英文字母混合在中文中训练
//	countUsal = WriteToHanZiLink(pCurStr, nCurIndex, POS_CODE_n , 0);		//播放：字母
//
//	g_hTTS->m_ControlSwitch.bIsReadDian = emTrue;
//
//	return countUsal;
//
//
//
//}


//***************************************************************************************************************
//功能：中文句中处理和播放字母串(可能中英混合播放)
//输入参数：当前英文串emPByte pCurStr，相对位移量emInt16  nCurIndex，英文串长度 nLen
//返回： 播放完后最新的中文句中的索引位置
//***************************************************************************************************************
emInt16 emCall PlayYingWenVoice( emPByte pCurStr, emInt16  nCurIndex , emInt16 nLen )
{
	#define MAX_CN_CALL_EN_LEN	23		//中文句中调用英文模块的字符串的最大长度   最大单词长度	  hhh		//deinstitutionalization 22个 internationalization 20个   这两个可忽略：Supercalifragilisticexpealidoshus	33个  Antidisestablishmentarianism	28个
	emByte  pEnText[MAX_CN_CALL_EN_LEN+2];
	emInt16 nShuZiLen, nWord,nNextZiMuLen,countUsal, nEnParaLen, nReturnVal,i;
	emBool  bUseEnModel = emTrue;		//默认：使用英文模型

	LOG_StackAddr(__FUNCTION__);
	nShuZiLen = GetStrSum( pCurStr, nCurIndex + 2 * nLen, DATATYPE_SHUZI);		
	
	if( nShuZiLen > 0 )						//按逐个字母发音（英文串后跟数字） 例如：喜欢GB2312。 喜欢BIG5。喜欢MP3。
	{
		if( nShuZiLen >= 2)
		{
			g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
			g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
		}
		bUseEnModel = emFalse;
	}
	else									//可能按单词发音
	{
		nEnParaLen = nLen;
		memset(pEnText, 0 , MAX_CN_CALL_EN_LEN);
		nWord = GetWord( pCurStr, nCurIndex + 2 * nLen );  

		 //字母串后的字符不是：“-”“&”“空格”
		if( nWord != (emInt16)0xa3ad && nWord != (emInt16)0xa3a6 && nWord != (emInt16)0xa3a0) 
		{	
			if( nLen == 1)							//按逐个字母发音（仅1个字母）  例如：喜欢4S。 喜欢3G。
				bUseEnModel = emFalse;
			else
			{

#if ZIMU_READ_AS_CN_MODEL							//中文句中的逐字字母发音：调用中文音库
				if( nEnParaLen > MAX_CN_CALL_EN_LEN)
					nEnParaLen = MAX_CN_CALL_EN_LEN;	
				for(i =0; i< nEnParaLen; i++)
					pEnText[i] = *(pCurStr + nCurIndex + 2 * i + 1) - 0x80;		//将字符串转换成单字节
				pEnText[i] = 0;
				pEnText[i+1] = 0;
				nReturnVal = 0;			//sqb  目前只做中文合成
				//nReturnVal = EngToLabAndSynth(pEnText,0,emTrue,emTrue,emFalse,emFalse);	//英文的文本分析和合成（最后一个参数为emFalse，内部不会合成声音，仅文本分析）
				if(nReturnVal == 0)					//按逐个字母发音（经英文模块识别，Lab数为零）
					bUseEnModel = emFalse;			//例如：他们在PK。  
				else
					bUseEnModel = emTrue;			//例如：你hold住了吗。  
#endif
			}
		}
		else				//按单词发音（字母串后的字符是：“-”“&”“空格”）
		{
			if( nEnParaLen > MAX_CN_CALL_EN_LEN)
				nEnParaLen = MAX_CN_CALL_EN_LEN;
			else
			{
				nNextZiMuLen = GetStrSum( pCurStr, nCurIndex + 2 * nLen + 2, DATATYPE_YINGWEN);				//每次最多处理2个单词
				if( (nEnParaLen + nNextZiMuLen + 1) < MAX_CN_CALL_EN_LEN )
					nEnParaLen += nNextZiMuLen+1;
			}

			//将“-”“&”“空格”前后的单词作为一个整体   例如：喜欢HR&Adm.喜欢hi-fi.喜欢CD-ROM。他们使用的是Windows Mobile平台。
			for(i =0; i< nEnParaLen; i++)
				pEnText[i] = *(pCurStr + nCurIndex + 2 * i + 1) - 0x80;		//将字符串转换成单字节
			pEnText[i] = 0;
			pEnText[i+1] = 0;
		}			
	}

	if( bUseEnModel == emFalse)  //调用中文音库，按逐个字母发音
	{
		countUsal = WriteToHanZiLink(pCurStr, nCurIndex, POS_CODE_n , 0);		//播放：字母
		g_hTTS->m_ControlSwitch.bIsReadDian = emTrue;
		return countUsal;
	}
	//else						//调用英文音库，按单词发音
	//{
	//	if( g_hTTS->m_HaveEnWordInCnType == 0)
	//		g_hTTS->m_HaveEnWordInCnType = 1;
	//	FirstHandleHanZiBuff(emFalse);
	//	EngToLabAndSynth(pEnText,0,emFalse,emFalse,emTrue,emTrue);	//英文的文本分析和合成
	//	return (nCurIndex + 2 * nEnParaLen);
	//}	
}









