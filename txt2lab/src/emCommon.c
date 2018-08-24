#include "emPCH.h"

#include "emCommon.h"
#include "emDecode.h"



//播放：静音 （包括：标点符号的静音，制造句尾效果的静音）
//注意：前1句的句号的静音一定要在下1句的滤波前再播，这样下1句的句头不会卡
//参数： nMuteMs：静音的毫秒数
emTTSErrID emCall PlayMuteDelay(emInt32 nMuteMs)
{
	emUInt32  nReadLen,nMuteLen;
	emPointer pParameter;

	LOG_StackAddr(__FUNCTION__);

	if( nMuteMs> 0)
	{
		if( g_hTTS->m_bIsMuteNeedResample == emTrue)		//若属于：需重采样型的静音（例如：2.标点停顿；	3. 制造句尾停顿效果 GenPauseNoOutSil()）（1.设置[p1000]；无需重采样）
		{
			//若是虚拟发音人则需调整静音长度
			switch( g_hTTS->m_ControlSwitch.m_nRoleIndex )
			{				
				case emTTS_USE_ROLE_Virtual_51:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_51;	break;
				case emTTS_USE_ROLE_Virtual_52:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_52;	break;
				case emTTS_USE_ROLE_Virtual_53:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_53;	break;
				case emTTS_USE_ROLE_Virtual_54:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_54;	break;
				case emTTS_USE_ROLE_Virtual_55:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_55;	break;
				default:	break;			
			}
		}


		pParameter = g_hTTS->pCBParam ;	

		nMuteLen =(emUInt32) ( ((float)nMuteMs/ 500)  * g_hTTS->pRearG->nModelSample );	//hyl 2012-03-23

		while(  nMuteLen > 0)
		{	
			if( nMuteLen > MAX_OUTPUT_PCM_SIZE )
			{
				nReadLen = MAX_OUTPUT_PCM_SIZE;
				nMuteLen -= nReadLen;					
			}
			else
			{
				nReadLen = ( (emInt32)( nMuteLen/2) )*2;
				nMuteLen = 0 ; 
			}
					
			emMemSet(g_hTTS->m_pPCMBuffer,0,MAX_OUTPUT_PCM_SIZE);
			g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //回调--播放声音（非HMM合成的）
			
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				return emTTS_ERR_EXIT;
			}

		}	

		g_hTTS->m_CurMuteMs = 0;	
		g_hTTS->m_bIsMuteNeedResample = emFalse;

#if DEBUG_LOG_SWITCH_TIME										//hyl 2012-04-10
			myTimeTrace_For_Debug(1,(emByte *)"【插播】--静音", 0);
			myTimeTrace_For_Debug(2,(emByte *)"		静音：实际时长：", 0);
			myTimeTrace_For_Debug(3, (emByte *)"" , nMuteMs);
#endif

	}

	return emTTS_ERR_OK;
}


//****************************************************************************************************
//  功能：用二分法在数据表中查找2个字节的内容										add by songkai
//	输入参数：pCurStr 是所查找内容所在的字符串，nCurIndex 该内容在字符串pCurStr 中的索引位置
//            nTable是所检索的表格在内存中的首地址，nSize是表格中一行所占据的字节数 
//            nStartNum和nTotalNum分别是表格中用于搜索的起始行号和总搜索行数
//	返回参数：内容所在行数相对于搜索起始行的行数偏移量 (若返回nTotalNum，表示没找到)
//****************************************************************************************************
emUInt16 ErFenSearchOne_FromRAM( emPByte pCurStr, emInt16 nCurIndex, emPByte nTable, emInt16 nStartNum, emUInt16 nTotalNum, emByte nSize )
{
	emUInt16 cMin,cMax,cMiddle, i = 0;
	emByte cHighByte,cLowByte;
	emByte cNextHighByte,cNextLowBte;

	LOG_StackAddr(__FUNCTION__);

	cMin = 0;
	cMax = nTotalNum -1;


	//检查搜索范围的第一行
	cLowByte = *(nTable + i);		      //取出第一个字节
	cHighByte = *(nTable + i + 1);        //取出第二个字节
	cNextLowBte = *(nTable + i + 2);      //取出第三个字节      // add by liugang 2011-5-26
	cNextHighByte = *(nTable + i + 3);    //取出第三个字节      // add by liugang 2011-5-26
	if( cLowByte == *( pCurStr + nCurIndex + 1 ) &&  cHighByte == *( pCurStr + nCurIndex ) && cNextLowBte == 0 && cNextHighByte == 0 )
	{	
		return cMin;
	}
	if( cMax != 0 )
	{

		//将位置指针移到表格中搜索范围的最后一行位所在位置，检查是否在该行
		cLowByte = *(nTable + ( nStartNum - 1 + cMax) * nSize);
		cHighByte = *(nTable + (nStartNum -1 + cMax) * nSize + 1);
		if( cLowByte == *( pCurStr + nCurIndex + 1 ) &&  cHighByte == *( pCurStr + nCurIndex )  && cNextLowBte == 0 && cNextHighByte == 0 )
		{		
			return cMax;
		}
	}
	if( cMax >= 2 ) //搜索范围大于两行时
	{
		cMiddle = ( cMin + cMax) / 2;
		do
		{		
			cLowByte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize);
			cHighByte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 1);
			cNextLowBte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 2);    // add by liugang 2011-5-26
			cNextHighByte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 3);  // add by liugang 2011-5-26
			if( cLowByte == *( pCurStr + nCurIndex + 1 ) &&  cHighByte == *( pCurStr + nCurIndex ) && cNextLowBte == 0 && cNextHighByte == 0  )
			{		
				return cMiddle;
			}
			if( cHighByte < *( pCurStr + nCurIndex ) || ( cHighByte == *( pCurStr + nCurIndex ) && cLowByte < *( pCurStr + nCurIndex + 1 )) )
			{   //查找的汉字编码大于表中汉字编码
				cMin = cMiddle;
			}
			else 
			{   //查找的汉字编码小于表中汉字编码
				cMax = cMiddle;
			}
			cMiddle = ( cMin + cMax) / 2;
		}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );
	}	
	return nTotalNum;
}


//****************************************************************************************************
//  功能：用二分法在数据表中查找某4个相连的字节        add by songkai
//	输入参数：pCurStr 是所查找汉字所在的字符串，nCurIndex 为第一个汉字在字符串pCurStr 中的索引位置
//            nTable是所检索的表格在内存中的首地址，nSize是表格中一行所占据的字节数 
//            nStartNum和nTotalNum分别是表格中用于搜索的起始行号和总搜索行数
//	返回参数：汉字所在行数相对于搜索起始行的行数偏移量  (若返回nTotalNum，表示没找到)
//****************************************************************************************************
emUInt16 ErFenSearchTwo_FromRAM( emPByte pCurStr, emInt16 nCurIndex, emPByte nTable, emInt16 nStartNum, emUInt16 nTotalNum, emByte nSize )
{
	emUInt16 cMin,cMax,cMiddle;
	emByte strCodeInTable[5],strCodeSearching[5];
	emByte i;

	LOG_StackAddr(__FUNCTION__);

	cMin = 0;
	cMax = nTotalNum -1;
	for( i = 0; i < 4; i++ )
	{
		strCodeSearching[i] = *( pCurStr + nCurIndex + i);
	}
	strCodeSearching[4] = '\0';

	strCodeInTable[4] = '\0';

	//将文件位置指针移到表格中搜索起始行位置，检查是否在该行
	strCodeInTable[1] = *(nTable + (nStartNum - 1) * nSize);
	strCodeInTable[0] = *(nTable + (nStartNum - 1) * nSize + 1);
	strCodeInTable[3] = *(nTable + (nStartNum - 1) * nSize + 2);
	strCodeInTable[2] = *(nTable + (nStartNum - 1) * nSize + 3);	

	if( emMemCmp( strCodeInTable, strCodeSearching,4 ) == 0 )
	{	
		return cMin;
	}
	if( cMax != 0 )                                                 //将文件位置指针移到表格中搜索起始行位置，检查是否在该行
	{

		strCodeInTable[1] = *(nTable + ( nStartNum - 1 + cMax) * nSize);
		strCodeInTable[0] = *(nTable + ( nStartNum - 1 + cMax) * nSize + 1);
		strCodeInTable[3] = *(nTable + ( nStartNum - 1 + cMax) * nSize + 2);
		strCodeInTable[2] = *(nTable + ( nStartNum - 1 + cMax) * nSize + 3);
		if( emMemCmp( strCodeInTable, strCodeSearching,4 ) == 0 )
		{		
			return cMax;
		}
	}
	if( cMax >= 2 )
	{
		cMiddle = ( cMin + cMax) / 2;
		do
		{			

			strCodeInTable[1] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize);
			strCodeInTable[0] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 1);
			strCodeInTable[3] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 2);
			strCodeInTable[2] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 3);

			if( emMemCmp( strCodeInTable, strCodeSearching ,4) == 0 )
			{		
				return cMiddle;
			}
			if( emMemCmp( strCodeInTable, strCodeSearching ,4) < 0 )		//hyl 2012-03-28 SYN6658的库函数问题
			{   //查找的汉字编码大于表中汉字编码
				cMin = cMiddle;
			}
			else 
			{   //查找的汉字编码小于表中汉字编码
				cMax = cMiddle;
			}
			cMiddle = ( cMin + cMax) / 2;
		}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );
	}	
	return nTotalNum;
}





//****************************************************************************************************
// 根据音量级数改变音量  (每2个字节循环更改音量)
//		参数：	emPByte  pStart,		// 输入：需改变音量的存储空间的起始地址； 同时为输出； 通过指针传参
//		参数：	emInt32 nLen, 			// 输入：需改变音量的从存储空间的起始地址开始的字节个数； 
//		参数：	emInt32  nVolXiShu,		// 音量级数
//****************************************************************************************************

void emCall	ChangePcmVol(emInt8 nVoiceType, emPByte  pStart, emInt32 nLen, emInt32  nVol)
{

	 emInt32  nNormalFix;	//正常音系数
	 emInt32  nPromptFix;	//提示音系数
	 emInt32  nVolXiShu;		//音量系数
	 emInt32 nLong;

	 emInt32  i, nMinVolumnValue, nMaxVolumnValue, nSum1;


	 LOG_StackAddr(__FUNCTION__);

	 switch( nVol )	//音量级别    
	 {	
		 //hyl  超过标准音量的略作调整（最低了最大的）  2012-04-16
		case   5:	 nNormalFix =10;	nPromptFix = 5;		break;		//标准音量
		case   6:	 nNormalFix =12;	nPromptFix = 6;		break;
		case   7:	 nNormalFix =14;	nPromptFix = 7;		break;
		case   8:	 nNormalFix =16;	nPromptFix = 8;		break;
		case   9:	 nNormalFix =18;	nPromptFix = 9;		break;
		case  10:	 nNormalFix =20;	nPromptFix =10;		break;		//最大音量

		case   4:	 nNormalFix = 8;	nPromptFix = 4;		break;
		case   3:	 nNormalFix = 6;	nPromptFix = 3;		break;
		case   2:	 nNormalFix = 4;	nPromptFix = 2;		break;
		case   1:	 nNormalFix = 2;	nPromptFix = 1;		break;
		case   0:	 nNormalFix = 0;	nPromptFix = 0;		break;		//静音		

		default	:	 nNormalFix =15;	nPromptFix = 7;		break;


		//case   5:	 nNormalFix =10;	nPromptFix = 5;		break;		//标准音量
		//case   6:	 nNormalFix =13;	nPromptFix = 7;		break;
		//case   7:	 nNormalFix =16;	nPromptFix = 8;		break;
		//case   8:	 nNormalFix =18;	nPromptFix = 9;		break;
		//case   9:	 nNormalFix =20;	nPromptFix =10;		break;
		//case  10:	 nNormalFix =22;	nPromptFix =11;		break;		//最大音量
	 }

	 if( nVoiceType == VOICE_TYPE_PROMPT)		//声音类型：提示音
	 {
		 nVolXiShu = nPromptFix;
	 }
	 else
	 {
		 nVolXiShu = nNormalFix;
	 }
	
	 nMinVolumnValue =  -32768 + 500;
	 nMaxVolumnValue =  32768 - 500;


	 //根据音量系数：改变音量
	 for( i=0; i< nLen ; i=i+2 )			
	 {
		 nSum1 = *((emInt16 *)(pStart + i));

		 nLong = ( nSum1 * nVolXiShu/10);	

		 //防音量溢出处理
		 if( nLong > nMaxVolumnValue)
			 nLong = nMaxVolumnValue;
		 if( nLong < nMinVolumnValue )
			 nLong = nMinVolumnValue;

		 *((emInt16 *)(pStart + i) ) = (emInt16)nLong;	
	 }

}



//****************************************************************************************************
//函数功能： 是有效的控制标记，在改变参数之前对PCM中已有的数据进行语音播放
//****************************************************************************************************
void emCall IsValidMarkAndPlayPrev()
{

	LOG_StackAddr(__FUNCTION__);

	FirstHandleHanZiBuff(emTrue);	//控制标记触发（除强制拼音外），《汉字缓冲区》的优先处理	
	//EnTextSynTrigger();		//有效控制标记前：触发英文合成   sqb

}


//****************************************************************************************************
//  
//  功能：播放提示音或预录音
//			返回值： 0：不是有效提示音		1：是有效提示音，且发完音
//****************************************************************************************************

emInt16  emCall  PlayPromptPreRecordVoice(
	emInt8 nPromptType,						//音的主类别：  PROMPT_MSG 或 PROMPT_SOUND
	emInt8 nMoreType,						//音的二级类别 例如：PROMPT_SOUND的二级类别有 1：sound1**  2：sound2**  等等
	emInt16 nPromptNo,						//第几个音
	struct promptInfo emPtr nCurIndex,		//音的信息：长度，偏移，解码类型
	emInt32 nIndexTableOffset)				//需查索引表的偏移量（最新：索引表和数据表合一了）
{
	#define  PROMPT_INDEX_HEAD_LEN  30				//提示音索引头占多少个双字节（不算提示音定位特征值）	
	#define	 PROMPT_FEATURE_LEN		24				//提示音定位特征值：长度
	#define	 PROMPT_INDEX_LEN		(4<<10)		//提示音索引部分的大小


	emInt16 head[PROMPT_INDEX_HEAD_LEN];
	emInt16 nDeodeType, nSample, nChannel, nBit;
	emInt16 nIndexOffset;
	emInt32 nPromptOffset,nPromptLen;

	emUInt16 msgStruct[1][2];		//二维的第1个数：个数偏移；  二维的第2个数：总个数； 
	emUInt16 soundStruct[9][2];		//二维的第1个数：个数偏移；  二维的第2个数：总个数； 

	emInt32 nTableOffset;



	LOG_StackAddr(__FUNCTION__);

	if( nIndexTableOffset == 0 )
	{
		//没有《提示音资源表》
		return 0;
	}

	nTableOffset = nIndexTableOffset + PROMPT_INDEX_LEN;	//固定提示音数据部分的开始位置（提示音索引部分的长度固定）

	PlayMuteDelay(g_hTTS->m_CurMuteMs);		//先播放静音

	FirstHandleHanZiBuff(emTrue);	//英文（提示音，普通英文，拼音）触发，除能解释成汉字的英文外，《汉字缓冲区》的优先处理
	//EnTextSynTrigger();		//播放提示音前：触发英文合成   sqb

#if DEBUG_LOG_SWITCH_TIME										//hyl 2012-04-12
			myTimeTrace_For_Debug(1,(emByte *)"【提示音】--开始", 0);
#endif

	fFrontSeek(g_hTTS->fResFrontMain, nIndexTableOffset+PROMPT_FEATURE_LEN  ,0);

	//读取提示音索引表的60个头字节（不算提示音定位特征值）
	fFrontRead(head, 2, PROMPT_INDEX_HEAD_LEN, g_hTTS->fResFrontMain);


	nDeodeType   = head[6];
	nSample     = head[7];
	nChannel    = head[8];
	nBit        = head[9];

	emMemCpy(msgStruct,&head[10], 4);
	emMemCpy(soundStruct, &head[12], 36);

	if( nPromptType == PROMPT_MSG)
	{
		if( nPromptNo >= msgStruct[0][1] )  //超过了总个数
		{
			//不是有效提示音			
			return 0;
		}

		nIndexOffset = 8 *  nPromptNo ;		
	}
	else if( nPromptType == PROMPT_SOUND)
	{
		if( nPromptNo >= soundStruct[nMoreType-1][1] )  //超过了总个数
		{
			//不是有效提示音
			return 0;
		}

		nIndexOffset = 8 * ( soundStruct[nMoreType-1][0] + nPromptNo);
	}
	
	//读取当前提示音的：起始偏移，总长度
	fFrontSeek(g_hTTS->fResFrontMain, nIndexOffset , 1);
	fFrontRead(&nPromptOffset, sizeof(emInt32), 1, g_hTTS->fResFrontMain);
	fFrontRead(&nPromptLen, sizeof(emInt32), 1, g_hTTS->fResFrontMain);

	nCurIndex->nPromptDecodeType = nDeodeType;	
	nCurIndex -> nPromptLen      =  nPromptLen;
	nCurIndex -> nPromptOffset   = nPromptOffset; 

	nCurIndex++;
	g_hTTS->m_nCurPointOfPcmInfo = nCurIndex;

	switch( nDeodeType)
	{
		case PROMPT_DECODE_ADPCM:
#if EM_USER_DECODER_ADPCM
			DecodePromptPreRecordAdpcmAndPlay(nSample, nTableOffset);
#endif

			break;
		case PROMPT_DECODE_G7231:
			DecodePromptPreRecordG7231AndPlay(nSample, nTableOffset);	

			break;
		case PROMPT_DECODE_PCM:
			DecodePromptPreRecordPcmAndPlay(nSample, nTableOffset);		
			break;
		default:
			break;
	}

	g_hTTS->m_nCurPointOfPcmInfo = g_hTTS->m_structPromptInfo;
	g_hTTS->m_nHanZiCount_In_HanZiLink = 0;  //个数清零

	g_nLastTextByteIndex = 0;

	ClearTextItem();	//汉字链表清零

#if DEBUG_LOG_SWITCH_TIME										//hyl 2012-04-12
			myTimeTrace_For_Debug(1,(emByte *)"【提示音】--结束", 0);
			myTimeTrace_For_Debug(2,(emByte *)"		提示音：实际时长：", 0);
			myTimeTrace_For_Debug(3, (emByte *)"" , g_hTTS->m_structPromptInfo[0].nPromptLen*1000/1600);
#endif

	PlayMuteDelay(100);		//强制播放100ms的静音  hyl 2012-04-12  防止音卡。  例如：王晓东来电话了soundj。你快点跑步去叫爸爸妈妈来接电话呀。

	return  1 ;
}






//****************************************************************************************************
//函数功能：截取源字符串strSource中的某段字符串，并将其复制到目的字符串数组strDest中
//          从emByte *strSource的第nStart个开始复制，截取nLen个到emByte *strDestt中
//输入的参数：emByte *strDest, emByte *strSource, emInt16 nStart,  emInt16 nLen ,  emInt16 nSizeOfDest
//改变的参数：从emByte *strSource的第nStart个开始复制，截取nLen个到emByte *strDestt中
//输出的参数： 复制字符串成功emTTS_ERR_OK或复制字符串失败emTTS_ERR_FAILED
//****************************************************************************************************
emTTSErrID  emCall CpyPartStr( emByte *strDest, emByte *strSource, emInt16 nStart,  emInt16 nLen ,  emInt16 nSizeOfDest)
{
	emInt16  i;

	LOG_StackAddr(__FUNCTION__);

	if( emStrLenA( strSource) < ( nStart + nLen ) )
	{
		return emTTS_ERR_FAILED;
	}

	if( nSizeOfDest <  ( nLen ) )
	{
		return emTTS_ERR_FAILED;
	}


	for( i = 0; i <  nLen ; i++ )
	{
		strDest[i] = strSource[ nStart + i ];

	}
	//strDest[i] = '\0';

	return emTTS_ERR_OK;
}

//****************************************************************************************************
//  功能：获取当前字符的是否是《成对标点符号类型》  即pCurStr中目前nCurIndex位置处所指字符
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：是 ，否
//****************************************************************************************************
emBool emCall  IsShuangBiaoDian( emPByte pCurStr, emInt16  nCurIndex)
{
	emInt16 nIndex = nCurIndex;
	emInt16	nWord = 0;	
	emInt16	nDataType = 0;

	LOG_StackAddr(__FUNCTION__);

	nWord = *(pCurStr+nIndex);		
	nWord =  nWord << 8;		//获取当前双字节的高字节

	nWord = nWord + (*(pCurStr+(++nIndex)) & (emInt16)0x00ff);		//获取的当前双字节字符

	if ( ( nWord >= (emInt16)0xa140 && nWord < (emInt16)0xa3b0)	)		//是符号
	{	 
		
		if(	   nWord == (emInt16)0xA3A2	// "
			|| nWord == (emInt16)0xA1AE	// ‘
			|| nWord == (emInt16)0xA1AF	// ’
			|| nWord == (emInt16)0xA1B0	// “
			|| nWord == (emInt16)0xA1B1	// ”
			//|| nWord == (emInt16)0xA3A8	// （			deleted by hyl  2012-03-30  在之前已调用 GenPauseNoOutSil();	//制造句尾停顿效果		
			//|| nWord == (emInt16)0xA3A9	// ）
			//|| nWord == (emInt16)0xA1B2	// 〔
			//|| nWord == (emInt16)0xA1B3	// 〕
			|| nWord == (emInt16)0xA1B4	// 〈
			|| nWord == (emInt16)0xA1B5	// 〉
			|| nWord == (emInt16)0xA1B6	// 《
			|| nWord == (emInt16)0xA1B7	// 》
			|| nWord == (emInt16)0xA1BE	// 【
			|| nWord == (emInt16)0xA1BF	// 】
			|| nWord == (emInt16)0xA3FB	// ｛
			|| nWord == (emInt16)0xA3FD	// ｝
			//|| nWord == (emInt16)0xA3DB	// ［		控制标记也用它，不能参与
			//|| nWord == (emInt16)0xA3DD	// ］
		   )
		{
			return emTrue;
		}
	}
	else
	{
		return emFalse;
	}
	return nDataType;

}

//****************************************************************************************************
//  功能：获取当前字符的数据类型即pCurStr中目前nCurIndex位置处所指字符的数据类型
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：16位整形的数据类型nDataType  符号类型或汉字类型或英文类型或数字类型
//注意：判断的是GBK的双字节字符类型，识别英文大小写
//****************************************************************************************************
emInt16 emCall  CheckDataType( emPByte pCurStr, emInt16  nCurIndex)
{	

	emInt16 nIndex = nCurIndex;
	emInt16	nWord = 0;	
	emInt16	nDataType = 0;

	LOG_StackAddr(__FUNCTION__);

	nWord = *(pCurStr+nIndex);		
	nWord =  nWord << 8;		//获取当前双字节的高字节

	nWord = nWord + (*(pCurStr+(++nIndex)) & (emInt16)0x00ff);		//获取的当前双字节字符

	if ( nWord == (emInt16)END_WORD_OF_BUFFER )
	{	 
		nDataType = (emInt16)DATATYPE_END;
	}
	else if( ( nWord >= (emInt16)0x8140 && nWord <= (emInt16)0xa0fe ) 
		|| ( nWord >=(emInt16)0xaa40 && nWord <= (emInt16)0xfefe ) )
	{
		nDataType = (emInt16)DATATYPE_HANZI;
	}
	else if( ( nWord >= (emInt16)0xa3c1 && nWord <= (emInt16)0xa3da ) 
		|| ( nWord >= (emInt16)0xa3e1 && nWord <= (emInt16)0xa3fa ) )
	{
		nDataType = (emInt16)DATATYPE_YINGWEN;	//含大小写
	}
	else if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9)
	{
		nDataType = (emInt16)DATATYPE_SHUZI;
	}
	else if (  nWord >= (emInt16)0xa140 && nWord <= (emInt16)0xa9ff )	//此区域的数字和字母在之前已识别
	{
		nDataType = (emInt16)DATATYPE_FUHAO;
	}
	else 
	{
		nDataType = (emInt16)DATATYPE_UNKNOWN;
	}

	return nDataType;
}



//****************************************************************************************************
//  功能：将BUFFER中的内容清空
//	输入参数：emPByte pCurStr, emInt16 n , n为清空buffer中的内容的长度
//	返回：无返回值
//****************************************************************************************************
void ClearBuffer( emPByte pCurStr, emInt16 n )
{

	LOG_StackAddr(__FUNCTION__);

	for( ; n >= 0 ; n--)
		*( pCurStr + n ) = 0;
}

//****************************************************************************************************
//  功能：得到指向地址的字内容
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex	
//	返回：16位整形的字emInt16	nWord
//****************************************************************************************************
emInt16 emCall  GetWord( emPByte pCurStr, emUInt32  nCurIndex)
{	
	emUInt32 nIndex = nCurIndex;
	emInt16	nWord = 0;	

	LOG_StackAddr(__FUNCTION__);

    if(nCurIndex>256)
        return nWord;

	nWord = *(pCurStr+nIndex);		
	nWord =  nWord << 8;		//获取当前双字节的高字节
	nWord = nWord + (*(pCurStr+(++nIndex)) & (emInt16)0x00ff);		//获取的当前双字节字符
	return nWord;
}

//****************************************************************************************************
//  功能：将一个字的内容存放到指向地址的内存中，
//        将地址为（pCurStr+nCurIndex）,所指向处的字内容设为emInt16  nWord	
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex ,需要放入的字内容emInt16  nWord	
//	返回：无返回值
//****************************************************************************************************
void PutIntoWord( emPByte pCurStr, emInt16  nCurIndex,emInt16  nWord )
{	
	emInt16  nWordtemp = nWord ;
	emInt16  nIndex = nCurIndex;

	LOG_StackAddr(__FUNCTION__);

	(*(pCurStr+nIndex)) = (emByte)(nWordtemp >> 8);			//存放高字节
	nIndex += 1;
	(*(pCurStr+nIndex)) = (emByte)nWord;		//存放低字节	
}

//****************************************************************************************************
//  功能：计算连续串的长度
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex ，数据类型emInt16	nDataType
//	返回：连续串的个数nSumOfStr
//****************************************************************************************************
emInt16 emCall  GetStrSum( emPByte pCurStr, emInt16  nCurIndex, emInt16	nDataType )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfStr = 0 ;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == nDataType )
		{
			nSumOfStr++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == nDataType );

	return nSumOfStr;
}
//****************************************************************************************************
//  功能：将数字转换后的汉字播放，即播放g_hTTS->m_ShuZiBuffer[60]内部汉字
//	输入参数：数组g_hTTS->m_ShuZiBuffer[60],emInt16 nCurIndexSZBuffer
//	返回：无返回值  
//****************************************************************************************************
emInt16 emCall SoundSZBuffer( emPByte pCurStr, emInt16 nCurIndexSZBuffer, emInt16 nPos )
{
	emInt16  nNextIndex;

	LOG_StackAddr(__FUNCTION__);

	nNextIndex = WriteToHanZiLink( g_hTTS->m_ShuZiBuffer,nCurIndexSZBuffer , nPos, 0);

	return nNextIndex;

}



//****************************************************************************************************
//  功能：将本汉字串pCurStr（nCurIndexSZBuffer开始的）组成1个节点写入到汉字链表中去
//        参数 nPos = POS_CODE_kong					： 表示本汉字串是原始汉字串  
//             nPos != POS_CODE_kong				： 表示本汉字串是转换过来的汉字串   （例如：数字，符号转换过来的），这些是带了词性过来的
//             nPos != POS_CODE_kong 且 nPinYin！=0 ： 表示本汉字串就1个单字，且强制拼音  （例如： 胡xiao3静）
//		  返回： 写完汉字链表后指向的nNextCurIndexSZBuffer
//****************************************************************************************************
emInt16 emCall WriteToHanZiLink(emPByte pCurStr, emInt16 nCurIndexSZBuffer,emInt16 nPos, emInt16 nPinYin)
{
	
	emInt16 nLen, nHanZiCount,nTextType;

	emInt16 nNextCurIndexSZBuffer;

	emByte   *pnShengMuNo=0;	    //指针传参：有效拼音的声母编码，初始值为0
	emByte   *pnYunMuNo=0;		//指针传参：有效拼音的韵母编码，初始值为0
	emByte   *pnShengDiaoNo=0;    //指针传参：有效拼音的声调编码，初始值为0
	emInt16   nValueOfHandelPinYinReturn;
	emByte    PinYin[3] ;

	

	LOG_StackAddr(__FUNCTION__);

	pnShengMuNo = PinYin;
	pnYunMuNo = PinYin + 1;
	pnShengDiaoNo = PinYin + 2;

	nNextCurIndexSZBuffer = nCurIndexSZBuffer;

	

	nHanZiCount = GetHanZiSum( pCurStr , nCurIndexSZBuffer );  //获取连续汉字串的汉字个数

	nTextType = TextCsItem_TextType_HANZI;

	//若是普通英文字母，也当成汉字处理  -- 2011-02-15修改  为实现功能：英文字母混合在中文中训练
	if( nHanZiCount == 0)
	{
		nHanZiCount = GetYingWenSum( pCurStr , nCurIndexSZBuffer );  //获取连续英文串的汉字个数

		if( nHanZiCount >= 6)				//当英文字母个数：小于等于5个：一次性读完； 大于等于6个：每次读4个 ；
		{
			nHanZiCount = 4;
		}
		nTextType = TextCsItem_TextType_ZIMU;

	}

	//判断是否是“一字一顿”发音风格。 若是：汉字链表里的最大个数只能是(MAX_HANZI_COUNT_OF_LINK/2)
	//：（因为一字一顿风格在每个字后要强行加PAU，防止m_LabRam[MAX_HANZI_COUNT_OF_LINK + 20][]越界
	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD)
	{
		
		if( (g_hTTS->m_nHanZiCount_In_HanZiLink + nHanZiCount)> (MAX_HANZI_COUNT_OF_LINK/2)  && nPos == POS_CODE_kong )
		{
			nHanZiCount = (MAX_HANZI_COUNT_OF_LINK/2) - g_hTTS->m_nHanZiCount_In_HanZiLink;	//则只能取部分汉字串
		}
	}
	else
	{
		//若需处理的汉字串是来源于原始汉字串 且其汉字个数加上汉字链表里的汉字个数超过MAX_HANZI_COUNT_OF_LINK
		if( (g_hTTS->m_nHanZiCount_In_HanZiLink + nHanZiCount)> MAX_HANZI_COUNT_OF_LINK  && nPos == POS_CODE_kong )
		{
			nHanZiCount = MAX_HANZI_COUNT_OF_LINK - g_hTTS->m_nHanZiCount_In_HanZiLink;	//则只能取部分汉字串
			g_hTTS->m_FirstSynthSegCount = 1;		//2012-04-16   防止音卡  例如：[s10]英文字母混读联通3G精彩在“沃”一名村民称他在自家的房前看到了UFO这是一个处处充满PK的时代行业应用前方600米接近目的地上地城铁站行驶2.5公里右前方行驶到上地信息路欢迎乘坐申新巴士四十九路无人售票车您的消费总额是475.25元收您500元找零24.75元您有新来电电话来自解晓东您有新消息新消息来自13905511861[d]

		}
	}

	if( nHanZiCount > 0 )
	{
		g_hTTS->m_nHanZiCount_In_HanZiLink += nHanZiCount;

		g_hTTS->m_nCurIndexOfAllHanZiBuffer += nHanZiCount;

		nLen = nHanZiCount * 2; 
		nNextCurIndexSZBuffer = nLen + nCurIndexSZBuffer;	


		//构造汉字链表的下1节点  -- 新增
		g_pTextInfo[g_nLastTextByteIndex/2].Pos = nPos;
		g_pTextInfo[g_nLastTextByteIndex/2].Len = nLen;
		g_pTextInfo[g_nLastTextByteIndex/2].TextType = nTextType;
		if( !emMemCmp( pCurStr + nCurIndexSZBuffer+nLen,"》",2))
		{
			emMemCpy(g_pTextInfo[g_nLastTextByteIndex/2].nBiaoDian , "》",2);
		}
		emMemCpy( &g_pText[g_nLastTextByteIndex], pCurStr + nCurIndexSZBuffer ,nLen); //拷贝汉字串
		if( nPinYin != 0 )											//判断这种情况：  银hang2行长
		{
			emMemCpy(&g_pTextPinYinCode[g_nLastTextByteIndex/2], &nPinYin,2);		
		}
		else
		{
			//判断符号是否"[="  ，若是合格的强制拼音，应一起写入到汉字buff ； 若有多个"[="合格的强制拼音，以第一个为有效，其余丢弃
			while((GetWord(  pCurStr,nCurIndexSZBuffer+nLen ) == (emInt16)0xa3db)&&(GetWord(  pCurStr,nCurIndexSZBuffer+nLen + 2 ) == (emInt16)0xa3bd))
			{
				(*pnShengMuNo)		= 255;		//初始化 hyl 2012-03-27
				(*pnYunMuNo)		= 255;
				(*pnShengDiaoNo)	= 255;

				nValueOfHandelPinYinReturn=GetPinYinCode(pCurStr,nCurIndexSZBuffer+nLen,emFalse,pnShengMuNo,pnYunMuNo,pnShengDiaoNo );
				if (   nValueOfHandelPinYinReturn != 0									//合格的拼音信息
					&& CheckDataType(pCurStr, nCurIndexSZBuffer) !=DATATYPE_YINGWEN)	//且本字不是英文字母
				{
					nNextCurIndexSZBuffer += nValueOfHandelPinYinReturn;
					if( (*pnYunMuNo)<100 && (*pnShengMuNo)<100 && (*pnShengDiaoNo)<100)
					{
						//有效拼音
						g_pTextPinYinCode[(g_nLastTextByteIndex+g_pTextInfo[g_nLastTextByteIndex/2].Len)/2 -1 ] = 0x8000 + (*pnYunMuNo)*256 + (*pnShengMuNo)*8 + (*pnShengDiaoNo); //赋值为强制拼音
					}
					else
					{
						//无效拼音
						g_pTextPinYinCode[(g_nLastTextByteIndex+g_pTextInfo[g_nLastTextByteIndex/2].Len)/2 -1 ] = 0;
					}
					//再搜索强制拼音后面是否还有汉字，有的话当成连续的汉字串一起处理
					nCurIndexSZBuffer = nNextCurIndexSZBuffer;
					nHanZiCount = GetHanZiSum( pCurStr , nNextCurIndexSZBuffer );  //获取连续汉字串的汉字个数
					if( nHanZiCount > 0 )
					{
						g_hTTS->m_nHanZiCount_In_HanZiLink += nHanZiCount;		
						g_hTTS->m_nCurIndexOfAllHanZiBuffer += nHanZiCount;
						nLen = nHanZiCount * 2; 
						nNextCurIndexSZBuffer = nLen + nCurIndexSZBuffer;	
						//将汉字链表的本节点的相关值进行相应的改变						
						emMemCpy( &g_pText[g_nLastTextByteIndex+g_pTextInfo[g_nLastTextByteIndex/2].Len], pCurStr + nCurIndexSZBuffer ,nLen); //拷贝汉字串
						g_pTextInfo[g_nLastTextByteIndex/2].Len += nLen;
					}
				}
				else
				{
					break;
				}
			}	
		}
		

		g_hTTS->m_nCurIndexOfConvertData = nNextCurIndexSZBuffer;	

		if( nPos == POS_CODE_kong)       //来源于原始汉字串，不是来源于转换来的汉字串
		{
			//判断g_Text汉字串：是否需要让后续的数字读成号码；若变的话，直接在函数里赋值
			
			JustHanZiLetDigitToPhone();
		}

		g_nCurTextByteIndex = g_nLastTextByteIndex;

		g_nLastTextByteIndex += g_pTextInfo[g_nLastTextByteIndex/2].Len;



	}

	//判断是否是“一字一顿”发音风格。 若是：汉字链表里的最大个数只能是(MAX_HANZI_COUNT_OF_LINK/2)
	//：（因为一字一顿风格在每个字后要强行加PAU，防止m_LabRam[MAX_HANZI_COUNT_OF_LINK + 20][]越界
	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD)
	{
		if (g_hTTS->m_nHanZiCount_In_HanZiLink >= (MAX_HANZI_COUNT_OF_LINK/2) ) 
		{
			g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;	//遇到[p*]停顿不输出本句的句尾sil
			FirstHandleHanZiBuff(emTrue);	
		}
	}
	else
	{
		if (g_hTTS->m_nHanZiCount_In_HanZiLink >= MAX_HANZI_COUNT_OF_LINK ) //若汉字链表中的汉字个数大于MAX_HANZI_COUNT_OF_LINK
		{
			g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;	//遇到[p*]停顿不输出本句的句尾sil
			FirstHandleHanZiBuff(emTrue);	//汉字链表中的汉字个数大于50个：触发，《汉字缓冲区》的优先处理
			g_hTTS->m_FirstSynthSegCount = 1;		//2012-04-16   防止音卡  例如：[s10]英文字母混读联通3G精彩在“沃”一名村民称他在自家的房前看到了UFO这是一个处处充满PK的时代行业应用前方600米接近目的地上地城铁站行驶2.5公里右前方行驶到上地信息路欢迎乘坐申新巴士四十九路无人售票车您的消费总额是475.25元收您500元找零24.75元您有新来电电话来自解晓东您有新消息新消息来自13905511861[d]

		}
	}

	return nNextCurIndexSZBuffer;	
}


//****************************************************************************************************
//函数功能： 清除汉字链表
//****************************************************************************************************
void emCall ClearTextItem()
{
	emInt8 i;

	LOG_StackAddr(__FUNCTION__);

	for(i=0;i<MAX_HANZI_COUNT_OF_LINK_NEED_HEAP;i++)
	{
		g_pTextInfo[i].BorderType = 0;
		g_pTextInfo[i].Len = 0;
		g_pTextInfo[i].Pos = 0;
		g_pTextInfo[i].TextType = 0;
		g_pTextInfo[i].nBiaoDian[0] = 0;
		g_pTextInfo[i].nBiaoDian[1] = 0;

	}
	emMemSet(g_pText,			  0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);				
	emMemSet(g_pTextPinYinCode, 0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);	
	g_nLastTextByteIndex=0; 
	g_nCurTextByteIndex=0; 
	g_nNextTextByteIndex=0; 
	g_nP1TextByteIndex=0; 
	g_nP2TextByteIndex=0; 

	emMemSet( g_hTTS->m_ManualRhythmIndex, 0, MAX_MANUAL_PPH_COUNT_IN_SEN);
	g_hTTS->m_CurManualRhythmIndex = 0;

	g_ForcePPH.nStartIndex	= 0;
	g_ForcePPH.nSylLen	= 0;
	g_ForcePPH.nCount	= 0;
	g_ForcePPH.bIsStartCount= 0;
}

//****************************************************************************************************
//  功能：判断g_Text汉字串：是否需要让后续的数字读成号码；若变的话，直接在函数里赋值
//****************************************************************************************************

void emCall JustHanZiLetDigitToPhone() 
{
	emInt16 nLastHanZiNum,nSumOfHanZi,nSumOfShuZi;
	emByte i;                                   //循环变量
	emInt16 nDataType = 0;                      //记录字符的类型
	emInt16 nDataTypeAfterHanZi = 0;  
	emInt16 nWord = 0;                          //临时存放一个汉字（或其它符号）

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetShuZiSum( g_hTTS->m_pDataAfterConvert,g_hTTS->m_nCurIndexOfConvertData );
	nDataTypeAfterHanZi = CheckDataType( g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);

	nSumOfHanZi = emStrLenA(g_pText)/2;

	//汉字串处理完毕，判断是否需要将将“后续数字读成号码”开关打开	
	nLastHanZiNum = ( nSumOfHanZi >= (emInt16) KEYWORD_SEARCH_COUNT ) ? (emInt16) KEYWORD_SEARCH_COUNT : nSumOfHanZi;


	//若之前读号码开关是打开的，且接下来的汉字串仅为“转”“或” “到” “呼”，则读号码开关继续打开
	if( g_hTTS->m_ControlSwitch.m_bIsPhoneNum == emTrue )
	{		
		nWord = GetWord( g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData -2);
		if( nWord == ( emInt16 ) 0xd7aa ||  nWord == ( emInt16 ) 0xbbf2 ||  nWord == ( emInt16 ) 0xb5bd ||  nWord == ( emInt16 ) 0xbaf4)
		{  
			if(CheckDataType( g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData -4) == DATATYPE_SHUZI)
			{
				g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
				g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
				return;
			}
		}

	}

	g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emFalse;
	
	//检查该汉字串后5个汉字中是否包含“市话”“长途”“灵通”“手机”“电话”中任意一个词。
	//若包含，且该汉字串后出现的第一个汉字不是“元”，将“后续数字读成号码”开关打开，并且1必须读成“幺”
		if(    (nSumOfShuZi >= 3  && nDataTypeAfterHanZi == DATATYPE_SHUZI)
		|| nDataTypeAfterHanZi != DATATYPE_SHUZI)		//要么汉字后带3位以上数字，要么带的不是数字
	{		
		for( i = 0; i <= ( nLastHanZiNum - 2 ); i++ )
		{ //检查该汉字串后6个汉字中是否包含数据表Table_HanZiLetDigitToPhone中的第四类表                                《本汉字词在数字串之前的N个汉字中出现--但须处理元》
			if ( CheckHanZiLetDigitToPhoneTable( g_pText,  (nSumOfHanZi-nLastHanZiNum + i)*2 , 4 ) == emTrue)
			{//找到热词，判断汉字串后的第一个汉字是否为“元”
				i = g_hTTS->m_nCurIndexOfConvertData-2;  
				do
				{	
					i += 2;
					nDataType = CheckDataType( g_hTTS->m_pDataAfterConvert,i);
					
				}while( nDataType != DATATYPE_HANZI && nDataType != DATATYPE_END );
				nWord = GetWord(g_hTTS->m_pDataAfterConvert, i);
				if( nWord == ( emInt16 ) 0xd4aa )
				{   //汉字串后第一个汉字是“元”，符合条件，将“后续数字读成号码”开关打开，并且1必须读成“幺”，退出函数
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emFalse;
				}
				else 
				{
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
					g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
				}
			}
		}   
	}

	//检查该汉字串的最后两个汉字是否包含在数据表Table_HanZiLetDigitToPhone，且后面紧跟3位或3位以上数字--                  《本汉字词紧跟在数字串之前》
	//若满足条件，将“后续数字读成号码”开关打开，并且1必须读成“幺”
	if(    nLastHanZiNum >= 2
		&& nDataTypeAfterHanZi == DATATYPE_SHUZI
		&& nSumOfShuZi >= 3)
	{   //用遍历法在Table_HanZiLetDigitToPhone中查找汉字串最后两个字。该表格共15行，每行占4个字节
		if ( CheckHanZiLetDigitToPhoneTable(  g_pText, (nSumOfHanZi-2 )*2, 2 ) == emTrue)
		{//满足条件，将“后续数字读成号码”开关打开，并且1必须读成“幺”，退出函数
			g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
			g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
		}
	}

	//检查该汉字串后6个汉字中是否包含数据表Table_HanZiLetDigitToPhone中的任意一词,   《本汉字词在数字串之前的N个汉字中出现》
	//若有，将“后续数字读成号码”开关打开，并且1必须读成“幺”	
	if( nLastHanZiNum >= 2 )
	{
		//要么汉字后带3位以上数字，要么带的不是数字
		if(    (nSumOfShuZi >= 3  && nDataTypeAfterHanZi == DATATYPE_SHUZI)
			|| nDataTypeAfterHanZi != DATATYPE_SHUZI)
		{
			//当用于搜索的汉字串末尾汉字个数大于1时，先用遍历法检查是否包含数据表Table_HanZiLetDigitToPhone中的任意一个两字词
			for( i = 0; i <= ( nLastHanZiNum - 2 ); i++ )
			{
				if ( CheckHanZiLetDigitToPhoneTable( g_pText,(nSumOfHanZi-nLastHanZiNum + i)*2 , 1 ) == emTrue )
				{   

					//满足条件，将“后续数字读成号码”开关打开，并且1必须读成“幺”，退出函数			
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;		
					g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
				}
			} 
		}
	}
}


//****************************************************************************************************
//  功能：判断此节点的汉字串：是否需要让后续的数字读成号码
//	输入参数：gbk罗马数字gbk编码 emInt16 nLuoMaShuZi 
//	返回：汉字的gbk编码 emInt16 nHanZiOfShuZi 
//****************************************************************************************************
emInt16  emCall  ChangeShuZiToHanZi( emInt16 nLuoMaShuZi )
{
	emInt16 nHanZiOfShuZi =0;

	LOG_StackAddr(__FUNCTION__);

	if( nLuoMaShuZi > (emInt16)0xa3b9 || nLuoMaShuZi < (emInt16)0xa3b0 )
	{   
		return  emTTS_ERR_EXIT; //转换的内容不是数字则退出
	}
	else 
	{
		switch( nLuoMaShuZi )
		{ 
		case (emInt16)0xa3b9: nHanZiOfShuZi =  (emInt16)0xbec5;break;//9转九
		case (emInt16)0xa3b8: nHanZiOfShuZi =  (emInt16)0xb0cb;break;//8转八
		case (emInt16)0xa3b7: nHanZiOfShuZi =  (emInt16)0xc6df;break;//7转七
		case (emInt16)0xa3b6: nHanZiOfShuZi =  (emInt16)0xc1f9;break;//6转六
		case (emInt16)0xa3b5: nHanZiOfShuZi =  (emInt16)0xcee5;break;//5转五
		case (emInt16)0xa3b4: nHanZiOfShuZi =  (emInt16)0xcbc4;break;//4转四
		case (emInt16)0xa3b3: nHanZiOfShuZi =  (emInt16)0xc8fd;break;//3转三
		case (emInt16)0xa3b2: nHanZiOfShuZi =  (emInt16)0xb6fe;break;//2转二
		case (emInt16)0xa3b1: 
			{
				if ( g_hTTS->m_ControlSwitch.m_bYaoPolicy == emTTS_CHNUM1_READ_YAO)
				{
					nHanZiOfShuZi =  (emInt16)0xe7db;break;//1转幺
				}
				nHanZiOfShuZi =  (emInt16)0xd2bb;break;//1转一
			}
		case (emInt16)0xa3b0: nHanZiOfShuZi =  (emInt16)0xc1e3;break;//0转零
		}
	}
	return nHanZiOfShuZi;
}
//****************************************************************************************************
//  功能：将数字串转化为整形数 
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex，即数字串的起始位置
//	返回：emInt32值   
//****************************************************************************************************
emInt32 emCall  ChangeShuZiToInt( emPByte pCurStr, emInt16  nCurIndex )
{
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfShuZi = 0;
	emInt32 nReturnValue = 0 ;
	emInt16 nWordTemp = 0 ;

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	nReturnValue = GetWord( pCurStr,nIndex );
	nReturnValue = nReturnValue - 0xa3b0;
	if ( nSumOfShuZi <= 5 )
	{  
		while ( nSumOfShuZi )
		{   
			nSumOfShuZi -= 1;
			if ( nSumOfShuZi == 0)
			{
				break;
			}
			else
			{
				nIndex += 2;
				nWordTemp = GetWord( pCurStr,nIndex );
				nReturnValue = nReturnValue  * 10 + (nWordTemp - 0xa3b0) ;
			}
		} 
	}
	return nReturnValue;
}

//****************************************************************************************************
//  功能：将数字串转化为长整形数 
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex，即数字串的起始位置
//	返回：emInt32值    
//****************************************************************************************************
emInt32 emCall  ChangeShuZiToLong( emPByte pCurStr, emInt16  nCurIndex )
{
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfShuZi = 0;
	emInt16 nWordTemp = 0 ;
	emInt32 nResult = 0;

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	nWordTemp = GetWord( pCurStr,nIndex ) - 0xa3b0;

	nResult = nWordTemp;
	  
	while ( nSumOfShuZi )
	{   
		nSumOfShuZi -= 1;
		if ( nSumOfShuZi == 0)
		{
			break;
		}
		else
		{
			nIndex += 2;
			nWordTemp = GetWord( pCurStr,nIndex )  - 0xa3b0;
			nResult = nResult  * 10 + nWordTemp ;
		}
	} 

	return nResult;
}
//****************************************************************************************************
//  功能：将数字串按号码发声
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：处理了数字串中读成号码的个数 nSumOfReadHaoMa
//  改变的全局参数：g_hTTS->m_ShuZiBuffer
//****************************************************************************************************
emInt16 emCall  ReadDigitHaoMa( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadHaoMa = 0 ;
	emInt16 ntempCount = 0;
	emInt16	nDataType = 0;
	emInt16 nLuoMaShuZi;
	emInt16 nHanZiOfShuZi;
	emInt16 nCurIndexSZBuffer = 0;
	emInt16 nLeaveShuZiLen;  
	emInt16 nTotalShuZiLen;  

	LOG_StackAddr(__FUNCTION__);


	g_hTTS->m_ControlSwitch.m_bYiBianYinPolicy = emTTS_USE_YIBIANYIN_CLOSE;		//将“一”的变音全局变量值关闭，即“一”不参与变音，3声连变等还照样 

	nLeaveShuZiLen = GetShuZiSum( pCurStr, nIndex)*2;  
	nTotalShuZiLen = nLeaveShuZiLen;

	do
	{
		nDataType = CheckDataType(pCurStr, nIndex);
		if( nDataType != DATATYPE_SHUZI ) 
		{   
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, END_WORD_OF_BUFFER );
		}

		else 
		{
			nLuoMaShuZi = GetWord( pCurStr,nIndex );
			nHanZiOfShuZi = ChangeShuZiToHanZi( nLuoMaShuZi );
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, nHanZiOfShuZi );
			nCurIndexSZBuffer += 2;
			nLeaveShuZiLen -= 2;
			nIndex += 2;
			ntempCount += 1;  			

			//控制每4位读1次（从后算起）  例如：13910008888 会读成： 139  1000  8888； 但总位数小于等于5位时不在分开：例如：电话：10086。请走101国道
			//但当前头只剩1位时，这1位和后面4位一起读（例如：QQ：568790234）
			if( ((emInt32 )(nLeaveShuZiLen/8)*8) == (nLeaveShuZiLen)  && nTotalShuZiLen>=10 && (nTotalShuZiLen-nLeaveShuZiLen)>=4 && nLeaveShuZiLen>0)  //hyl 1220    
			{				
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, END_WORD_OF_BUFFER );
				nCurIndexSZBuffer = 0;
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, POS_CODE_m  );			//播放：号码
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容

				GenPauseNoOutSil();	//制造句尾停顿效果		
				
			}
			if( ntempCount != 29 )		//判断是否超出数字缓存的最大存储量60 ：g_hTTS->m_ShuZiBuffer[60]即30个数字
			{
				continue;
			}
			else 
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, END_WORD_OF_BUFFER );
			}
		}

		nCurIndexSZBuffer = 0;
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, POS_CODE_m  );			//播放：号码
		ClearBuffer( g_hTTS->m_ShuZiBuffer, (ntempCount+1) * 2 );			//清空数字缓存中存储的内容
		nSumOfReadHaoMa += ntempCount;
		ntempCount = 0;
		nDataType = CheckDataType( pCurStr, nIndex );
	}while( nDataType == DATATYPE_SHUZI );


	g_hTTS->m_ControlSwitch.m_bYiBianYinPolicy = emTTS_USE_YIBIANYIN_OPEN; //还原进入函数前的“一”的变音全局变量值

	return nSumOfReadHaoMa;
}

//****************************************************************************************************
//  功能：将数字串按小数点数值发声
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：处理了数字串中读成小数点数值的个数 nSumOfReadShuZhi
//  注意：若没有小数点的数值，这个函数也能正确读出
//****************************************************************************************************
emInt16 emCall  ReadDigitShuZhiAddDian( emPByte pCurStr, emInt16  nCurIndex )
{
	 emInt16 nSumOfReadShuZhi, nTemp, nSumOfReadHaoMa, nTotalSum;

	 LOG_StackAddr(__FUNCTION__);

	 nSumOfReadShuZhi = ReadDigitShuZhi( pCurStr, nCurIndex );
	 if( GetWord( pCurStr,nCurIndex+nSumOfReadShuZhi*2 ) == (emInt16) 0xa3ae)		//后面有小数点
	 {
		 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb5e3 ); //点
		 PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
		 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);	//播放：点
		 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
		 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
		 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
		 nSumOfReadHaoMa = ReadDigitHaoMa(pCurStr,nCurIndex+nSumOfReadShuZhi*2+2);
		 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
		 nTotalSum =  nSumOfReadShuZhi+1+nSumOfReadHaoMa;
		 return nTotalSum;

	 }
	 else
	 {
		 nTotalSum =  nSumOfReadShuZhi;
		 return nTotalSum;
	 }
}

//****************************************************************************************************
//  功能：将数字串按数值发声
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：处理了数字串中读成数值的个数 nSumOfReadShuZhi
//****************************************************************************************************
emInt16 emCall  ReadDigitShuZhi( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadShuZhi = 0 ;
	emInt16 nSumOfTemp = 0 ;
	emInt16 nCountTemp = 0;
	//emInt16	nTemp_gValue ;
	emInt16 i;
	emInt16 nSoundSZOffset;

	LOG_StackAddr(__FUNCTION__);

	
	ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容

	nSumOfReadShuZhi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	for ( i = 0; i < nSumOfReadShuZhi; i++ )
	{
		if ( GetWord( pCurStr, nIndex + 2*i ) == (emInt16)0xa3b0 )
		{
			nCountTemp += 1; //数字串前0的个数
			continue;
		}
		break;
	}
	//数字串全为0，只读一个“零”的音
	if ( nCountTemp == nSumOfReadShuZhi )
	{
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放： 数值
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容
	}

	nIndex += nCountTemp * 2;
	nSumOfTemp = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	while( (nSumOfTemp <= 16) && (nSumOfTemp > 12) )
	{
		g_hTTS->m_bIsErToLiang = emTrue;
		nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex, nSumOfTemp - 12);
		nIndex += (nSumOfTemp - 12) * 2 ;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset, 0xcdf2 ); //单位“万”
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset + 2, END_WORD_OF_BUFFER );
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放： “万”
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容
		nSumOfTemp = 12;
	}
	while( (nSumOfTemp <= 12) && (nSumOfTemp > 8) )
	{
		g_hTTS->m_bIsErToLiang = emTrue;
		nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex, nSumOfTemp - 8);
		nIndex += (nSumOfTemp - 8) * 2 ;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset, 0xd2da ); //单位“亿”
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset + 2, END_WORD_OF_BUFFER );
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放“亿”
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容
		nSumOfTemp = 8;
	}
	while( (nSumOfTemp <= 8) && (nSumOfTemp > 4) )
	{   
		for ( i = 0,nCountTemp = 0; i < 4; i ++ )
		{
			if ( GetWord( pCurStr, nIndex + 2*i ) == (emInt16)0xa3b0 )
			{
				nCountTemp += 1; //数字串0的个数
				continue;
			}
			break;
		}
		if ( nCountTemp != 4 ) //数字串中0的个数不为4时
		{
			g_hTTS->m_bIsErToLiang = emTrue;
			nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex , nSumOfTemp - 4);
			nIndex += (nSumOfTemp - 4) * 2 ;
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset, 0xcdf2 ); //单位“万”
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset + 2, END_WORD_OF_BUFFER );
			SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );			//播放： “万”
			ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容
			nSumOfTemp = 4;
		}
		else 
		{
			if ( GetWord( pCurStr, nIndex + 2*4 ) != (emInt16)0xa3b0 )
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放： 数值
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容
			}
			nIndex += (nSumOfTemp - 4) * 2 ;
			nSumOfTemp = 4;
		}
	}
	//读1-4位的数字串
	if(  CheckDataType( pCurStr,nIndex+2*nSumOfTemp ) == DATATYPE_HANZI   )
	{
		g_hTTS->m_bIsErToLiang = emTrue;		
	}
	nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex, nSumOfTemp);

	SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放： 数值
	ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//清空数字缓存中存储的内容


	return nSumOfReadShuZhi;
}
//****************************************************************************************************
//  功能：将不大于4个数字的数字串按数值发音
//			参数1：当前字符串emPByte pCurStr，
//			参数2：相对位移量emInt16  nCurIndex
//			参数3：读取数字的个数emInt16 nSumOfRead
//	返回：无返回值
//****************************************************************************************************
emInt16 emCall  ReadDigitShuZhiOfFour( emPByte pCurStr, emInt16  nCurIndex, emInt16 nSumOfRead)
{
	emInt16 nIndex = nCurIndex;
	emInt16 nIndexOfBuf = 0;
	emInt16 nCopyYaoPolicy = 0 ;
	emInt16 nWordTemp = 0,nNextWord ,nNextNextWord,nPrevWord;
	emInt16 nWord = 0 ;
	emBool  bIsHaveLiang = emFalse;		//是否已经有1个“二”转成了“两”

	LOG_StackAddr(__FUNCTION__);

	nCopyYaoPolicy = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
	g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;

	//“二”转读成“两”的规则：
	//规则1.仅在数值中才有可能转，号码中不转
	//规则2.本4位一读中，首个数字是“二”，且出现在千位或百位，将转成“两”；
	//规则3.本4位一读中，一共就只有个位的“二”（千位百位十位都没有数），但后面至少还有4位数或后面是给定的汉字或在“2:30”中，则这个“二”也应转成“两”
	switch ( nSumOfRead )
	{
	case 4: //剩四位数
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );
		//“二”转化成“两”判断
		if( nWordTemp == (emInt16)0xb6fe && bIsHaveLiang == emFalse)		//等于“二”
		{				
			nWordTemp = (emInt16)0xc1bd;		//转化成“两”
		}
		bIsHaveLiang = emTrue;
		nIndex += 2;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
		nIndexOfBuf += 2;
		if ( nWordTemp != (emInt16)0xc1e3 )
		{
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xc7a7 );//千
			nIndexOfBuf += 2;
			SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );		
			ClearBuffer( g_hTTS->m_ShuZiBuffer, 10);	
			nIndexOfBuf = 0;
		}
	case 3: //剩三位数
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );
		//“二”转化成“两”判断
		if( nWordTemp == (emInt16)0xb6fe && bIsHaveLiang == emFalse)		//等于“二”
		{					
			nWordTemp = (emInt16)0xc1bd;		//转化成“两”
		}
		bIsHaveLiang = emTrue;
		nIndex += 2;
		if( nSumOfRead > 3 )
		{
			nWord =  GetWord((emPByte)g_hTTS->m_ShuZiBuffer, nIndexOfBuf-2) ;
			if ( nWordTemp == (emInt16)0xc1e3 && nWord == (emInt16)0xc1e3 )
			{
			}
			else
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
				nIndexOfBuf += 2;
				if ( nWordTemp != (emInt16)0xc1e3)
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xb0d9 );//百
					nIndexOfBuf += 2;
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );		
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 10);	
					nIndexOfBuf = 0;
				}
			}
		}
		else 
		{
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
			nIndexOfBuf += 2;
			if ( nWordTemp != (emInt16)0xc1e3)
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xb0d9 );//百
				nIndexOfBuf += 2;
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );		
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 10);	
				nIndexOfBuf = 0;
			}
		}
	case 2: //剩两位数
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );		
		nIndex += 2;
		if( nSumOfRead > 2 )
		{
			nWord = GetWord((emPByte)g_hTTS->m_ShuZiBuffer, nIndexOfBuf-2) ;
			if ( nWordTemp == (emInt16)0xc1e3 && nWord == (emInt16)0xc1e3 )
			{
			}
			else
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
				nIndexOfBuf += 2;
				if ( nWordTemp != (emInt16)0xc1e3)
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xcaae );//十
					nIndexOfBuf += 2;
				}
			}
		}
		else 
		{
			if( nWordTemp == (emInt16)0xd2bb )//两位数,十位数为一时，一不发音
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xcaae );//十
				nIndexOfBuf += 2;
			}
			else 
			{   
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
				nIndexOfBuf += 2;
				if( nWordTemp != (emInt16)0xc1e3 )
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xcaae );//十
					nIndexOfBuf += 2;
				}
			}
		}
	case 1://剩一位数
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );
		//“二”转化成“两”判断
		if( nWordTemp == (emInt16)0xb6fe && bIsHaveLiang == emFalse && g_hTTS->m_bIsErToLiang == emTrue && nSumOfRead==1 )		//等于“二”
		{	
			//规则3：  “二”转读成“两”的规则
			//例子： 直接回复1或2即可。2个人。2间房。2两酒。2扇门。2把刀。2桶水。第2个人。第2间房。第2两酒。第2扇门。第2把刀。第2桶水。
			//注意：后字不能是：月,日,号  ； 前字不能是：第
			nPrevWord = GetWord(pCurStr, nIndex-2);		//前字	
			nNextWord = GetWord(pCurStr, nIndex+2);		//后字
			nNextNextWord = GetWord(pCurStr, nIndex+4);	//后后字
			if(    CheckDataType(pCurStr, nIndex+2) == DATATYPE_SHUZI	//后字是：   数字
				|| nNextWord == (emInt16)0xa3ba							//后字是：   ：
				|| (    nPrevWord != (emInt16)0xb5da							//前字不是： 第
				     && (   nNextWord == (emInt16)0xb8f6						//后字是：   个
						 || nNextWord == (emInt16)0xbce4						//后字是：   间
						 || nNextWord == (emInt16)0xc9c8						//后字是：   扇
						 || nNextWord == (emInt16)0xb0d1						//后字是：   把
						 || nNextWord == (emInt16)0xb4ce						//后字是：   次
						 || nNextWord == (emInt16)0xc3d7						//后字是：   米
						 || nNextWord == (emInt16)0xc3eb						//后字是：   秒
						 || nNextWord == (emInt16)0xc4ea						//后字是：   年
						 || nNextWord == (emInt16)0xc8cb						//后字是：   人
						 || nNextWord == (emInt16)0xccec						//后字是：   天
						 || nNextWord == (emInt16)0xccf5						//后字是：   条
						 || nNextWord == (emInt16)0xcebb						//后字是：   位
						 || nNextWord == (emInt16)0xbdef						//后字是：   斤
						 || nNextWord == (emInt16)0xbfc3						//后字是：   棵
						 || nNextWord == (emInt16)0xbfc5						//后字是：   颗
						 || nNextWord == (emInt16)0xc1a3						//后字是：   粒
						 || nNextWord == (emInt16)0xc3fb						//后字是：   名
						 || nNextWord == (emInt16)0xd2b3						//后字是：   页
						 || nNextWord == (emInt16)0xd6bb						//后字是：   只
						 || nNextWord == (emInt16)0xd6dc						//后字是：   周
						 || nNextWord == (emInt16)0xcdb0						//后字是：   桶		
						 || (nNextWord == (emInt16)0xb7d6 && nNextNextWord == (emInt16)0xd6d3) 		//后字是：   分钟
						 || (nNextWord == (emInt16)0xb9ab && nNextNextWord == (emInt16)0xc0ef) 		//后字是：   公里
						 || (nNextWord == (emInt16)0xc7a7 && nNextNextWord == (emInt16)0xbfcb) 		//后字是：   千克
						 || (nNextWord == (emInt16)0xc7a7 && nNextNextWord == (emInt16)0xc3d7) 		//后字是：   千米
						 || (nNextWord == (emInt16)0xd0a1 && nNextNextWord == (emInt16)0xcab1) 		//后字是：   小时
						 || (nNextWord == (emInt16)0xb9ab && nNextNextWord == (emInt16)0xbdef)))) 	//后字是：   公斤
						 	
			{
				nWordTemp = (emInt16)0xc1bd;		//转化成“两”
			}
		}
		bIsHaveLiang = emTrue;
		nIndex += 2;
		if( nSumOfRead > 1 )
		{   
			nWord = GetWord((emPByte)g_hTTS->m_ShuZiBuffer, nIndexOfBuf-2);
			if ( nWordTemp == (emInt16)0xc1e3 ) //&& nWord == (emInt16)0xc1e3 )
			{   
				if ( nWord == (emInt16)0xc1e3 )
				{
					nIndexOfBuf -= 2;
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, END_WORD_OF_BUFFER );
					break;
				}
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, END_WORD_OF_BUFFER );
				break;
			}
		}

		PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
		nIndexOfBuf += 2;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, END_WORD_OF_BUFFER );
		break;

	default: break;
	}

	g_hTTS->m_ControlSwitch.m_bYaoPolicy = nCopyYaoPolicy;

	g_hTTS->m_bIsErToLiang = emFalse;

	return nIndexOfBuf;
}

//****************************************************************************************************
//  功能：计算连续汉字串的长度，2个字节为一个汉字
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：汉字串的汉字个数nSumOfHanZi
//****************************************************************************************************
emInt16 emCall  GetHanZiSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfHanZi = 0 ;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_HANZI )
		{
			nSumOfHanZi++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_HANZI );

	return nSumOfHanZi;
}

//****************************************************************************************************
//  功能：计算连续英文串的长度，2个字节为一个英文
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：英文串的英文个数nSumOfYingWen
//****************************************************************************************************
emInt16 emCall  GetYingWenSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfYingWen = 0;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_YINGWEN )
		{
			nSumOfYingWen++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_YINGWEN );

	return nSumOfYingWen;
}

//****************************************************************************************************
//  功能：计算连续符号串的长度，2个字节为一个符号
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：符号串的符号个数nSumOfFuHao
//****************************************************************************************************
emInt16 emCall  GetFuHaoSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfFuHao = 0 ;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_FUHAO )
		{
			nSumOfFuHao++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_FUHAO );

	return nSumOfFuHao;
}

//****************************************************************************************************
//  功能：计算连续数字串的长度，2个字节为一个数字
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex	
//	返回：数字串的数字个数nSumOfShuZi
//****************************************************************************************************
emInt16 emCall  GetShuZiSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfShuZi = 0;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_SHUZI )
		{
			nSumOfShuZi++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_SHUZI );

	return nSumOfShuZi;
}

//****************************************************************************************************
//  功能：检查输入的字符串是否是有效的拼音，并传递有效的拼音编码。   新系统
//	输入参数：参见如下函数
//	返回： 若是合格的拼音格式，返回值为“输入拼音串”整个串的长度，若不是，返回值为0，
//  
//****************************************************************************************************
emInt16  emCall  GetPinYinCode( 
							  emPByte  pCurStr, 		//当前处理的buffer的起始位置
							  emInt16  nCurIndex,  	    //当前处理到buffer的第几个字符
							  emBool   bHaveOnlyPinYin, //[=cha1]: 赋值false； cha1：赋值true
							  emByte   *pnShengMuNo,	    //指针传参：有效拼音的声母编码，初始值为0
							  emByte   *pnYunMuNo,		//指针传参：有效拼音的韵母编码，初始值为0
							  emByte   *pnShengDiaoNo   //指针传参：有效拼音的声调编码，初始值为0
							  )
{
	emInt16 nIndex        = nCurIndex;
	emInt16 nlenOfYingWen = 0;
	emInt16 nlenOfShuZi   = 0;
	emInt16 nlen          = 0;
	emInt16 nWordTempofSZ   = 0;
	emInt16 nWordTempofYW   = 0;
	emInt16 nPianYiHangShu  = 0;
	emInt16 nQiShiHangShu   = 0;
	emInt16 nSumOfHangShu   = 0;
	emInt16 i = 0,j = 0;
	emInt16 nWord           = 0;
	emByte cHighByte = 0;                       //用于临时存放表格中的高字节
	emByte cLowByte = 0;                        //用于临时存放表格中的低字节

	LOG_StackAddr(__FUNCTION__);


	if ( bHaveOnlyPinYin == emFalse) // 例如：[=cha1]
	{
		nIndex += 4;
	}

	nlenOfYingWen = GetStrSum( pCurStr, nIndex, DATATYPE_YINGWEN );
	if ( nlenOfYingWen == 0 || nlenOfYingWen > 6 )
	{    
		return nlen;
	}

	nlenOfShuZi   = GetStrSum( pCurStr, nIndex + nlenOfYingWen*2 , DATATYPE_SHUZI );
	nWordTempofSZ = ChangeShuZiToInt( pCurStr, nIndex + nlenOfYingWen*2 );
	if ( nlenOfShuZi != 1 || (nlenOfShuZi == 1 && nWordTempofSZ == 0) || nWordTempofSZ > 5 )
	{    
		return nlen;
	}

	if ( bHaveOnlyPinYin == emFalse) // [=cha1]
	{   //判断数字串后的字符是否为“]”
		if ( GetWord(pCurStr, nIndex + nlenOfYingWen*2 + nlenOfShuZi*2 ) != (emInt16)0xa3dd ) 
		{
			return nlen;
		}
	}

	nWordTempofYW = GetWord(pCurStr, nIndex );
	nPianYiHangShu = ( nWordTempofYW - (emInt16)0xa3e1 ) * 6 + nlenOfYingWen;

	//根据“偏移行数”查找《拼音查询索引表》，找到“起始行数”和“行数数量”
	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_PinYinInquireIndex + (nPianYiHangShu-1)* 4 ,0);
	fFrontRead(&nQiShiHangShu,2,1,g_hTTS->fResFrontMain);	//起始行数	
	fFrontRead(&nSumOfHangShu,2,1,g_hTTS->fResFrontMain);	//行数数量

	//再根据“起始行数”和“行数数量”从《拼音查询表》中搜索拼音
	nlen = nIndex ;

	for ( i = 0 ; i < nSumOfHangShu ; i++ )
	{ 
		nIndex = nlen;
		fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_PinYinInquire + (nQiShiHangShu-1 + i) * 8 ,0);
		do 
		{   
			j = 0;

			fFrontRead(&cLowByte,1,1,g_hTTS->fResFrontMain);
			fFrontRead(&cHighByte,1,1,g_hTTS->fResFrontMain);

			while( j < 2 ) 
		 {
			 nWordTempofYW = GetWord(pCurStr, nIndex ); 
			 if ( nWordTempofYW - cHighByte  == (emInt16)0xa380 )
			 {
				 nIndex += 2;
				 cHighByte = cLowByte;
				 j++;
				 continue;
			 }
			 break;
		 }
		} while ( j == 2 && (nIndex - nlen) <= 2*nlenOfYingWen );


		if ( (nIndex - nlen) == 2*nlenOfYingWen )//找到了匹配的拼音了，将声母，韵母，声调编号传递给参数
		{  
			fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_PinYinInquire + (nQiShiHangShu-1 + i) * 8 + 6,0);


			fFrontRead(pnYunMuNo,1,1,g_hTTS->fResFrontMain);
			fFrontRead(pnShengMuNo,1,1,g_hTTS->fResFrontMain);


			*pnShengDiaoNo = (emByte)nWordTempofSZ;
			nlen = 0;
			if ( bHaveOnlyPinYin == emFalse )
		 {
			 nlen =  2*2 + 2 ;
		 }
			nlen += nlenOfYingWen*2 + nlenOfShuZi*2 ;
			return nlen;
		}

	}

	//没有搜索到拼音（纯拼音：cha1），返回0值
	if ( bHaveOnlyPinYin == emTrue )
	{
		nlen = 0;
		return nlen;
	}
	//若bHaveOnlyPinYin=false（ 控制拼音：[=cha1] ），返回值为“输入拼音串”整个串的长度
	nlen = 2*2 + nlenOfYingWen*2 + nlenOfShuZi*2 + 2 ;
	return nlen;
}

//****************************************************************************************************
//  功能：查英文单位表 Table_Char1JustAfterNumToHanZi或Table_Char2JustAfterNumToHanZi
//        判断数字串后的英文是否在此表中
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex,当前为英文串的起始地址
//	返回值：当英文符号单位在表中时返回值为英文符号单位的个数,否则返回0个英文符号单位
//  改变量：如果返回值不为0时，则改变了全局变量g_hTTS->m_ShuZiBuffer的内容；否则不改变
//****************************************************************************************************
emInt16  emCall  CheckDanWeiTable( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nSumOfYWDW = 0 ;
	emInt16 nWordOne = 0 ;
	emInt16 nWordTwo = 0 ;
	emInt16 nWordThree = 0 ;
	emInt16 nWordTemp = 0 ;
	emInt16 i ;
	emInt16 StartLine;
	emInt16	LineCount;
	emByte cMin,cMax,cMiddle;
	emBool bFlag = emFalse;

	LOG_StackAddr(__FUNCTION__);

	cMin = 0;


	nSumOfYWDW = GetStrSum( pCurStr, nCurIndex, DATATYPE_YINGWEN );
	if ( nSumOfYWDW > 3 )
	{
		nSumOfYWDW = 0;
		return  nSumOfYWDW;
	}

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_EnglishLiangCiToHanZi+ 4*(nSumOfYWDW - 1), 0);
	fFrontRead(&StartLine,2,1,g_hTTS->fResFrontMain);
	fFrontRead(&LineCount,2,1,g_hTTS->fResFrontMain);

	cMax = LineCount -1;

	if ( cMax >= 2 ) //搜索范围大于两行时
	{
		cMiddle = ( cMin + cMax) / 2;
		do
		{		
			fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_EnglishLiangCiToHanZi + ( StartLine - 1 + cMiddle) * 14, 0 );
			fFrontRead(&nWordOne,2,1,g_hTTS->fResFrontMain);
			switch ( nSumOfYWDW )
			{
			case 3:
				fFrontRead(&nWordTwo,2,1,g_hTTS->fResFrontMain);
				fFrontRead(&nWordThree,2,1,g_hTTS->fResFrontMain);
				break;
			case 2:
				fFrontRead(&nWordTwo,2,1,g_hTTS->fResFrontMain);
				break;
			}

			if( nWordOne == GetWord(pCurStr, nCurIndex) )
			{	
				if ( nWordTwo == 0)
				{//已找到
					break;
					//return cMiddle;
				}

				if ( nWordTwo == GetWord(pCurStr, nCurIndex+ 2) )
				{
					if ( nWordThree == 0 )
					{//已找到

						break;
						//return cMiddle;
					}
					if ( nWordThree == GetWord(pCurStr, nCurIndex+ 4) )
					{//已找到
						break;
						//return cMiddle; 
					}
					//未找到
					nSumOfYWDW = 0;
					return nSumOfYWDW;
				}

				if ( nWordTwo < GetWord(pCurStr, nCurIndex+ 2) )
				{ //查找的第二个英文的编码大于表中第二个英文的编码
					cMin = cMiddle;
				}

				else
				{//查找的第二个英文的编码小于表中第二个英文的编码
					cMax = cMiddle;
				}
			}

			if( nWordOne < GetWord(pCurStr, nCurIndex) )
			{   //查找的第一个英文的编码大于表中第一个英文的编码
				cMin = cMiddle;
			}

			if( nWordOne > GetWord(pCurStr, nCurIndex) )
			{   //查找的第一个英文的编码小于表中第一个英文的编码
				cMax = cMiddle;
			}

			cMiddle = ( cMin + cMax) / 2;
		}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );
	}	

	for( i = 0; i < 2 ; i++ )
	{
		fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_EnglishLiangCiToHanZi + ( StartLine - 1 + cMiddle) * 14, 0 );
		fFrontRead(&nWordOne,2,1,g_hTTS->fResFrontMain);
		fFrontRead(&nWordTwo,2,1,g_hTTS->fResFrontMain);
		fFrontRead(&nWordThree,2,1,g_hTTS->fResFrontMain);

		if ( (nWordOne == GetWord(pCurStr, nCurIndex)) && (nWordTwo==0 || (nWordTwo==GetWord(pCurStr, nCurIndex+2))) && (nWordThree==0 || (nWordThree==GetWord(pCurStr, nCurIndex+4))) )
		{   //找到后，将英文对应的汉字信息写入g_hTTS->m_ShuZiBuffer
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //汉字1的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //汉字2的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //汉字3的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //汉字4的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, nWordTemp );
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 8, END_WORD_OF_BUFFER  ); //ShuZiBuffer的结尾符
			return  nSumOfYWDW;
		}

		cMiddle =  cMax ;
	}

	nSumOfYWDW = 0;
	return  nSumOfYWDW;
} 

//****************************************************************************************************
//  功能：查《汉字量词表》 
//        判断汉字是否在此表中
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex,当前为汉字串的起始地址
//	返回值：当汉字量词的第一个字在表中时返回值为1,否则返回0
//****************************************************************************************************
emInt16  emCall  CheckHanZiLiangCiTable( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nWordOne = 0 ;
	emInt16 nWordTwo = 0 ;
	emInt16 nWordTemp;
	emInt16 i = 0 ;
	emInt16 bFlag =  0;

	LOG_StackAddr(__FUNCTION__);

	nWordOne = GetWord(pCurStr, nCurIndex);
	nWordTwo = GetWord(pCurStr, nCurIndex + 2);

	for ( i = 0; i < 80 ; i ++ )
	{   
		fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_HanZiLiangCi + i*4, 0);
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);
		if ( nWordTemp == nWordOne )
		{
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);			
			if (nWordTemp == nWordTwo )
			{
					bFlag = 1;
					return bFlag;
			}
			if ( nWordTemp == (emInt16) 0 )
			{
				bFlag = 1;
				return bFlag;
			}
		}
	}

	return  bFlag;
} 

//****************************************************************************************************
//  功能：查 汉字因素让数字读成号码表,包括 《本汉字词在数字串之前的N个汉字中出现》、《本汉字词紧跟在数字串之前》、 《本汉字词紧跟在数字串之后》
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex,  
// nTypeTable代表是要查询上述三表中的哪一类表,
//			nTypeTable=1，	查询《本汉字词在数字串之前的N个汉字中出现》表				--汉字处理中有用
//			nTypeTable=2，	查询《本汉字词紧跟在数字串之前》表							--汉字处理中有用
//			nTypeTable=3，	查询《本汉字词紧跟在数字串之后》表							--数字处理中有用
//			nTypeTable=4，	查询《本汉字词在数字串之前的N个汉字中出现--但须处理元》		--汉字处理中有用
//	返回：    如果要查找的汉字串在表中，则返回emTrue,否则返回emFalse
//****************************************************************************************************
emInt16  emCall  CheckHanZiLetDigitToPhoneTable( emPByte pCurStr, emInt16  nCurIndex ,emByte nTypeTable )
{   
	emInt16 nWordOne = 0 ;
	emInt16 nWordTwo = 0 ;
	emInt16 nWordTempOne = 0 ;
	emInt16 nWordTempTwo = 0 ;
	emInt16 i,j=0 ;
	emInt16 nStartLine ;//起始行数
	emInt16 nSumOfLine ;//行数数量

	LOG_StackAddr(__FUNCTION__);

	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_HanZiLetDigitToPhone + (nTypeTable - 1) * 4 ,0);
	fFrontRead(&nStartLine,2,1,g_hTTS->fResFrontMain);
	fFrontRead(&nSumOfLine,2,1,g_hTTS->fResFrontMain);


	nWordOne = GetWord( pCurStr, nCurIndex );
	nWordTwo = GetWord( pCurStr, nCurIndex + 2 );
	for ( i = 0; i < nSumOfLine; i++ )  //30为表Table_HanZiJustAfterNumOpenPhone的行数
	{
		fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_HanZiLetDigitToPhone + (nStartLine - 1)*4 + i * 4 ,0);
		fFrontRead(&nWordTempOne,2,1,g_hTTS->fResFrontMain);
		if ( nWordTempOne == nWordOne )  
		{
			fFrontRead(&nWordTempTwo,2,1,g_hTTS->fResFrontMain);			
			if ( nWordTempTwo == nWordTwo || nWordTempTwo==0x0000)	//在表中找到汉字相关内容
			{    
				if ( (nWordTempOne == (emInt16)0xc0b4) && (nWordTempTwo == (emInt16)0xd7d4) )
				{//是关键字“来自”
					for ( j = 0;  (GetWord(pCurStr, nCurIndex+2*j)!=((emInt16)0xa3ac)) && (CheckDataType(pCurStr,nCurIndex+2*j) != (emInt16)DATATYPE_SHUZI) && j<=7; j++ );
					if ( GetWord(pCurStr, nCurIndex+2*j)==((emInt16)0xa3ac) )//退出循环的原因是因为逗号吗
					{//如果是逗号的话，返回值为真，按号码处理
						return emTrue ;
					}
					if ( CheckDataType(pCurStr,nCurIndex+2*j) == (emInt16)DATATYPE_SHUZI )//退出循环的原因是因为没有逗号的情况下遇见了数字吗
					{//如果是的话，判断数字串的个数
						if ( GetStrSum( pCurStr, nCurIndex+2*j , DATATYPE_SHUZI ) < 7 )
						{//返回值为假，按数值处理
							return emFalse;
						}
					}
				}
				return emTrue ;
			}
		}
	}

	return emFalse;
}



//制造句尾停顿效果		
void emCall GenPauseNoOutSil()	
{
	emBool isDanZi = emFalse;

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	emBool b1;
	b1 = g_hTTS->bTextTailWrap;
	g_hTTS->bTextTailWrap = emFalse;
#endif

	LOG_StackAddr(__FUNCTION__);

	//制造停顿的效果；没有这两句前后内容会连读，不会停顿
	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;	//遇到[p*]停顿不输出本句的句尾sil

	if( emStrLenA(g_pText) == 2 )		
		isDanZi = emTrue;


	FirstHandleHanZiBuff(emTrue);	//《汉字缓冲区》的优先处理

	//hyl 2012-04-10
	//先播放100ms静音，防止音卡。  例如：例如: 阿拉伯国家联盟（阿盟）第23届首脑会议29日在持续了约7小时的会议议程后闭幕
	//模拟句尾sil的100ms，这样就总体与顿号的350ms是同样地概念，只要顿号的不卡，则《制造停顿》的也不会卡
	//例如：  缤纷泰国--新贵四晚。缤纷泰国、新贵四晚。
	PlayMuteDelay(100);			

	//若是单字，则停顿需多加200ms，否则SYN6658会卡    hyl 2012-03-31
	//例如：您的余额为￠39.40。（1）有强烈的创业愿望和想法。
	if( isDanZi == emTrue)		
		PlayMuteDelay(200);


#if EM_SYN_SEN_HEAD_SIL
	g_hTTS->m_CurMuteMs = 100 ;							//不能低于，否则芯片会卡。  例如：缤纷泰国--新贵四晚五日之旅	
#else
	g_hTTS->m_CurMuteMs = 150 ;							//不能低于，否则芯片会卡。  例如：缤纷泰国--新贵四晚五日之旅	
#endif

	
	






	g_hTTS->m_bIsMuteNeedResample = emTrue;

		

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	g_hTTS->bTextTailWrap = b1;
#endif

}


//****************************************************************************************************
//  功能：是否百分号类型
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：百分号类型
//			返回：0：非百分号千分号类型
//			返回：1：无“-”百分号类型	：50%
//			返回：2：有“-”百分号类型	：30-50%
//			返回：3：无“-”千分号类型	：50‰
//			返回：4：有“-”千分号类型	：30-50‰
//****************************************************************************************************
emInt16 emCall CheckIsTypeBaiFenHao( emPByte pCurStr, emInt16  nCurIndex )
{
	emBool bIsHaveZhi = emFalse;
	emInt16 nSum1;
	emInt16 nWord;
	emInt16 nIndex = nCurIndex;

	LOG_StackAddr(__FUNCTION__);
	
	nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
	nIndex += nSum1*2;
	nWord = GetWord( pCurStr,nIndex);
	if( nWord == (emInt16)0xa3ae)										//符号： “.”
	{
		nIndex +=  2;
		nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
		nIndex += nSum1*2;
		nWord = GetWord( pCurStr,nIndex);
	}
	if( nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa )          //符号：“-” 或 “—”
	{
		bIsHaveZhi = emTrue;
		nIndex +=  2;
		nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
		nIndex += nSum1*2 ;
		nWord = GetWord( pCurStr,nIndex);
		if( nWord == (emInt16)0xa3ae)										//符号： “.”
		{
			nIndex +=  2;
			nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
			nIndex += nSum1*2 ;
			nWord = GetWord( pCurStr,nIndex);
		}
	}
	if ( (nWord == (emInt16) 0xa3a5) || (nWord == (emInt16) 0xa987)  )//数字后有百分号%,﹪
	{
		if(  bIsHaveZhi == emTrue)
			return  2;
		else
			return  1;
	}
	if ( (nWord == (emInt16) 0xa1eb) )//数字后有千分号‰
	{
		if(  bIsHaveZhi == emTrue)
			return  4;
		else
			return  3;
	}

	return 0;
}
