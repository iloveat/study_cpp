#include "emPCH.h"

#include "Front_ProcFuHao.h"



emInt16 emCall ProcessFuHao( emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nSumOfFuHao = 0;
	emInt16 nSumOfShuZi = 0;
	emInt16	nSumOfHanZi = 0;
	emInt16	nSumOfYingWen = 0;
	emInt16	nCurIndexSZBuffer = 0;
	emInt16	nWord ,nNextWord;
	emInt16	nWord1,nWord2,nWord3;
	emInt16 nSumOfReadHaoMa = 0;
	emInt16 nDataType,nDataType1,nDataType2;
	emInt16 nTemp = 0 ;
	emInt32 nWordTemp = 0;///控制标记[字母+数字]
	emInt16 i,j;
	

	emBool    bHaveOnlyPinYin;
	emByte   *pnShengMuNo;	    //指针传参：有效拼音的声母编码，初始值为0
	emByte   *pnYunMuNo;		//指针传参：有效拼音的韵母编码，初始值为0
	emByte   *pnShengDiaoNo;    //指针传参：有效拼音的声调编码，初始值为0
	emInt16   nValueOfHandelPinYinReturn;
	emByte    PinYin[3] ;

	LOG_StackAddr(__FUNCTION__);

	pnShengMuNo = PinYin;
	pnYunMuNo = PinYin + 1;
	pnShengDiaoNo = PinYin + 2;



	nSumOfFuHao = GetFuHaoSum( strDataBuf,  nCurIndexOfBuf );
	//判断符号是否为“：”，符号“：”读成“比”的条件为：《符号：是否处理成比分开关》打开，且前后都是数字串
	if ( (GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa3ba)||(GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa1c3) )
	{
		if ( g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen == 1) //读“比”开关打开
		{
			if ( (nCurIndexOfBuf>1) && (CheckDataType( strDataBuf, nCurIndexOfBuf-2) == DATATYPE_SHUZI) && (CheckDataType( strDataBuf, nCurIndexOfBuf+2) == DATATYPE_SHUZI) )
			{  //满足符号前后均为数值时，才处理成比分格式
				PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb1c8 ); //比
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );	//播放： 比
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
				nCurIndexOfBuf += 2;
				return nCurIndexOfBuf;
			}
		}
	}

	//判断符号是否"[="；  到符号来处理的是单独的"[=]"， 前面没有跟汉字
	if((GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa3db)&&(GetWord( strDataBuf,nCurIndexOfBuf + 2 ) == (emInt16)0xa3bd))
	{
		bHaveOnlyPinYin = emFalse;
		nValueOfHandelPinYinReturn=GetPinYinCode(strDataBuf,nCurIndexOfBuf,bHaveOnlyPinYin,pnShengMuNo,pnYunMuNo,pnShengDiaoNo );
		if ( nValueOfHandelPinYinReturn != 0 )//合格的拼音信息
		{
			nCurIndexOfBuf += nValueOfHandelPinYinReturn;
			return nCurIndexOfBuf;
		}
	}

	nWord = GetWord( strDataBuf, nCurIndexOfBuf );

	//识别：控制标记		
	if ( nWord == (emInt16)0xa3db  )		//符号为"["的情况下
	{
		emInt8 nCtrlMarkLen;
		//识别控制标记并设置全局变量
		nCtrlMarkLen = JudgeCtrlMark(nSumOfFuHao,strDataBuf,nCurIndexOfBuf);		
		if( nCtrlMarkLen > 0 )							//有效控制标记
			return (nCurIndexOfBuf+nCtrlMarkLen);
	}


	//遇到“//”直接去掉 不发音    http://www.tts168.com.cn
	if ( ( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 0 ) ) == (emInt16)0xa3af 
		&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 ) ) == (emInt16)0xa3af )
	{
		nCurIndexOfBuf += 4 ;
		return nCurIndexOfBuf;
	}


	//最后一个符号优先判断
	if ( CheckDataType( strDataBuf, nCurIndexOfBuf + 2 ) != (emInt16)DATATYPE_FUHAO			//后1个不是符号
		||  GetWord( strDataBuf, nCurIndexOfBuf + 2 ) == (emInt16)0xa3ad)					//或者：后1个是符号“-”  20120118  hyl
	{
		nWord = GetWord( strDataBuf, nCurIndexOfBuf ) ;

		if( nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa )          //特殊符号“-”   ------6月3日写 或着”—“
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2);//获取特殊符号"-"前一个字符
			nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf - 2);//获取特殊符号“-"前面一个字符的类型

			if( nDataType == (emInt16)DATATYPE_SHUZI)//前一个字符类型为数字
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符
				nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符的类型
				if(nDataType == (emInt16)DATATYPE_SHUZI)//特殊符号”-“后一个字符类型也为数字
				{
					if( g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen == 1 )//比分开关打开读成比
					{
						//读成比
						PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb1c8 ); //比
						PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);	//播放： 比
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						nCurIndexOfBuf += 2;
						return nCurIndexOfBuf;
					}
					else
					{
						//读成至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);	//播放： 至
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						nCurIndexOfBuf += 2;
						return nCurIndexOfBuf;
					}
				}
				if(nDataType == (emInt16)DATATYPE_YINGWEN)//特殊符号”-“后一个字符类型是英文
				{
					//读成杠
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8dc ); //杠
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//播放： 杠 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}

			}

			if( nDataType == (emInt16)DATATYPE_YINGWEN )//前一个字符类型为英文
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符
				nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符的类型
				if( nDataType == (emInt16)DATATYPE_YINGWEN )//后一个字符类型也为英文
				{
					//读成杠
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8dc ); //杠
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//播放： 杠
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}
				if( nDataType == (emInt16)DATATYPE_SHUZI)//后一个字符类型为数字
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 ) //数字后面恰好是℃符号识别成零下
					{
						//处理成零下摄氏度并播放（s-12℃）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
						return nCurIndexOfBuf;

					}
					if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
					{
						nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
						if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) )//小数点后面的数字的后面是℃同时空格的前面不是℃
						{
							//处理成零下摄氏度并播放（诸如：s-12.8℃）
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );//播放：零下
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
							nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值							
							nCurIndexOfBuf += nSumOfShuZi*2 + 2;
							return nCurIndexOfBuf;
						}
					}
					//读成杠
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8dc ); //杠
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//播放： 杠 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}
			}

			if( nDataType == (emInt16)DATATYPE_FUHAO)//前一个字符类型为符号
			{
				if( (nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2)) == (emInt16)0xa1e6 )//特殊符号'-'前一个符号恰好是℃
				{
					nWord = GetWord( strDataBuf,nCurIndexOfBuf + 2);
					nDataType = CheckDataType( strDataBuf,nCurIndexOfBuf + 2 );
					if( nDataType == (emInt16)DATATYPE_SHUZI )//后一个字符类型为数字
					{
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
						if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 ) //数字后面恰好是℃符号
						{
							//读成至（12℃-12℃）
							PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //至
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);	//播放： 至	
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
							ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
						if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) )//小数点后面的数字的后面是℃
							{
								//读成至（12℃-12.9℃）
								PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //至
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);		//播放： 至
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);

								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ; //“-”后面的数字读成数值					
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi +  1 ) ;
								return nCurIndexOfBuf;
							}
						}
					}
				}
				if( nWord != (emInt16)0xa1eb && nWord != (emInt16)0xa987 && nWord != (emInt16)0xa3a5  )
				{
					nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符
					nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符的类型
					if( nDataType == (emInt16)DATATYPE_SHUZI )//后一个字符类型为数字
					{
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
						if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 
							&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6 ) //数字后面恰好是℃符号识别成零下
						{
							//处理成零下摄氏度并播放（好-12℃）
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
							ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
						if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) 
								&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6)//小数点后面的数字的后面是℃
							{
								//处理成零下摄氏度并播放（诸如好-12.8℃）
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
								PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容

								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
								return nCurIndexOfBuf;
							}
						}
						//数字后面不是℃就读成负
						PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8ba ); //负
						PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_v);	//播放： 负（三）
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						nCurIndexOfBuf += 2;
						return nCurIndexOfBuf;
					}
				}

			}


			if( nDataType == (emInt16)DATATYPE_UNKNOWN)//前一个字符类型为未知 诸如句子的5个结束标点
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符
				nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//获取特殊符号”-“后一个字符的类型
				if( nDataType == (emInt16)DATATYPE_SHUZI )//后一个字符类型为数字
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 
						&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6 ) //数字后面恰好是℃符号识别成零下
					{
						//处理成零下摄氏度并播放（好-12℃）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
						return nCurIndexOfBuf;
					}

					//hyl 2012-10-28 处理以下例句
					//例句：-1℃。1℃。南疆最低温度降至-1~-8℃。南疆最低温度降至-1~8℃。-1～-8℃。-1～8℃。-1~-8℃。-1~8℃。南疆最低温度降至-1 ~ -8℃。南疆最低温度降至-1 ~ 8℃。-1 ～ -8℃。-1 ～ 8℃。-1 ~ -8℃。-1 ~ 8℃。1 ～ -8℃。1 ～ 8℃。1～-8℃。1～8℃。1 ~ -8℃。1 ~ 8℃。1~-8℃。1~8℃。
					nTemp = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);
					nWord = GetWord( strDataBuf,  nTemp);
					nNextWord = GetWord( strDataBuf, nTemp+2 );
					if(    nWord == (emInt16)0xa1ab  //符号：～
						|| nWord == (emInt16)0xa3fe  //符号：~
						|| (nWord == (emInt16)0xa3a0 &&  (nNextWord == (emInt16)0xa1ab || nNextWord == (emInt16)0xa3fe) ))//符号：空格～  或 空格~
					{
						for( j=2; j<16;j=j+2)
						{
							nWord = GetWord( strDataBuf, nTemp+j );
							if( nWord == (emInt16)0xa1e6 ) //符号：℃	
								break;
						}

						if( j != 16 )
						{
							//处理成零下摄氏度并播放
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
							ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
					}


					if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
					{
						nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
						if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) 
							&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6)//小数点后面的数字的后面是℃
						{
							//处理成零下摄氏度并播放（诸如好-12.8℃）
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容

							nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
					}


					//数字后面不是℃就读成负
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8ba ); //负
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_v);	//播放： 负（三） 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}
			}

			//特殊符号‘-’不是以上各种情况
			nWord = GetWord( strDataBuf,nCurIndexOfBuf + 2);
			nDataType = CheckDataType( strDataBuf,nCurIndexOfBuf + 2 );
			if( nDataType == (emInt16)DATATYPE_SHUZI )//后一个字符类型为数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
				if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6  ) //数字后面恰好是℃符号
				{
					//处理成零下摄氏度并播放（好-12℃）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
					ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
					return nCurIndexOfBuf;
				}

				//hyl 2012-10-28 处理以下例句
				//例句：-1℃。1℃。南疆最低温度降至-1~-8℃。南疆最低温度降至-1~8℃。-1～-8℃。-1～8℃。-1~-8℃。-1~8℃。南疆最低温度降至-1 ~ -8℃。南疆最低温度降至-1 ~ 8℃。-1 ～ -8℃。-1 ～ 8℃。-1 ~ -8℃。-1 ~ 8℃。1 ～ -8℃。1 ～ 8℃。1～-8℃。1～8℃。1 ~ -8℃。1 ~ 8℃。1~-8℃。1~8℃。
				nTemp = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);
				nWord = GetWord( strDataBuf,  nTemp);
				nNextWord = GetWord( strDataBuf, nTemp+2 );
				if(    nWord == (emInt16)0xa1ab  //符号：～
					|| nWord == (emInt16)0xa3fe  //符号：~
					|| (nWord == (emInt16)0xa3a0 &&  (nNextWord == (emInt16)0xa1ab || nNextWord == (emInt16)0xa3fe) ))//符号：空格～  或 空格~
				{
					for( j=2; j<16;j=j+2)
					{
						nWord = GetWord( strDataBuf, nTemp+j );
						if( nWord == (emInt16)0xa1e6 ) //符号：℃	
							break;
					}

					if( j != 16 )
					{
						//处理成零下摄氏度并播放
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
						return nCurIndexOfBuf;
					}
				}


				if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
				{
					nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
					if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) )//小数点后面的数字的后面是℃
					{
						//处理成零下摄氏度并播放（诸如好-12.8℃）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );//播放：零下
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容

						nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi  + 1 ) ;
						return nCurIndexOfBuf;
					}
				}
				//读成至
				PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //至
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );	//播放： 至
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
				nCurIndexOfBuf += 2;
				return nCurIndexOfBuf;
			}

		}

	

		if ( nWord == (emInt16)0xa1ab || nWord == (emInt16)0xa3fe)//特殊符号"～ " {增加~（上划线）的处理，也读成至}——小张6月1日
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2) ;
			if( nWord == (emInt16)0xa3a0 )//特殊符号" ～ "前面是空格
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4);
				if( nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//空格前面不是数字
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0 )//“～”后面是数字
					{
						//处理成 至  并播放（诸如  ～8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;

					}
				}
				else//空格前面是数字
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0 ) 
					{
						//处理成 至  并播放（诸如4 ～8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;
					}
				}
			}

			else//特殊符号"～"前面不是空格
			{
				if (nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//“～”前面一位不是数字
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0 )//"～"后面是数字
					{
						//处理成 至  并播放（诸如 %～8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;

					}

					//例子：气温0℃～-6℃			//hyl  20120108
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0  && GetWord(strDataBuf, nCurIndexOfBuf + 2)==(emInt16)0xa3ad)  //符号“-”
					{
						//处理成 至  并播放（诸如 %～8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;

					}
				}
				else
				{
					if( GetWord( strDataBuf, nCurIndexOfBuf+2 ) == (emInt16)0xa3ad)		//下一符号是“-”，例如 “-1～-8℃。”   //hyl 2012-10-28
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
					else
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					
					if ( nSumOfShuZi > 0 ) 
					{
						//处理成 至  并播放（诸如4～8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;
					}
				}
			}
		}


		//是"￥"：判断“￥”后面是不是数字
		if ( nWord == (emInt16)0xa3a4 )
		{
			nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
			if ( nSumOfShuZi > 0 )
			{

				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc8cb ); //人
				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , (emInt16)0xc3f1 ); //民
				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  4 , (emInt16)0xb1d2 ); //币
				PutIntoWord( g_hTTS->m_ShuZiBuffer,  6 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);//播放：人民币
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 7 );//清空数字缓存中存储的内容
				nCurIndexOfBuf += 2 ;

				nWord = GetWord( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 );
				if ( nWord == (emInt16)0xa3ae )//数字后面还有小数点(例如￥20.54)
				{
					nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd4aa ); //元
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);//播放：元
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);//清空数字缓存中存储的内容
					nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi ) ;						
				}
				else
				{
					//处理成元+数值并播放(例如￥20)
					ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf  ); //先读数值
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd4aa ); //元
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_n );//播放：元
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					nCurIndexOfBuf = nCurIndexOfBuf + 2 *  nSumOfShuZi ;
				}
				nWord     = GetWord( strDataBuf, nCurIndexOfBuf);
				if ( nWord == (emInt16)0xd4aa  )	//若数值后紧接“元”，丢弃
				{
					nCurIndexOfBuf += 2;
				}
				return nCurIndexOfBuf;
			}
		}

		//是“＄”“￠”“￡”：判断后面是不是数字
		if ( nWord == (emInt16)0xa1e7 || nWord == (emInt16)0xa1e9 || nWord == (emInt16)0xa1ea  )
		{
			if( nWord == (emInt16)0xa1e7 )
			{
				nWord1 = (emInt16)0xc3c0 ;		//=美
				nWord2 = (emInt16)0xd4aa ;		//=元
			}
			if( nWord == (emInt16)0xa1e9 )
			{
				nWord1 = (emInt16)0xc5b7 ;		//=欧
				nWord2 = (emInt16)0xd4aa ;		//=元
			}
			if( nWord == (emInt16)0xa1ea  )
			{
				nWord1 = (emInt16)0xd3a2 ;		//=英
				nWord2 = (emInt16)0xb0f7 ;		//=镑
			}

			nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
			if ( nSumOfShuZi > 0 )
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 2);
				if ( nWord == (emInt16)0xa3ae )//数字后面还有小数点(例如＄20.54)
				{
					nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf + 2 );
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , nWord1); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , nWord2 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  4 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);//播放：美元 欧元 英镑
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
					nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);						
					
				}
				else
				{
						//处理成美元+数值并播放(例如＄20)
						ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf + 2 ); //先读数值
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , nWord1); 
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , nWord2); 
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_n );//播放：美元 欧元 英镑
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);
				}
				nWord     = GetWord( strDataBuf, nCurIndexOfBuf);
				nNextWord = GetWord( strDataBuf, nCurIndexOfBuf+2);
				if ( nWord == nWord1  && nNextWord == nWord2 )	//若数值后紧接“美元”“欧元” “英镑”，丢弃
				{
					nCurIndexOfBuf += 4;
				}
				return nCurIndexOfBuf;
			}
		}


		//---------------------------------------------小万更改6月12号start

		if ( nWord == (emInt16)0xa3af ) //特殊符号"/"
		{
			nSumOfHanZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_HANZI );
			if ( nSumOfHanZi == 1)//“/"后跟的连续汉字串长度为一
			{
				//判断前5个字中是否有数字
				nTemp = 5;
				if( (nCurIndexOfBuf/2) < nTemp)
				{
					nTemp = nCurIndexOfBuf/2;
				}
				for(i = 0; i< nTemp; i++)
				{
					nWord3 = GetWord( strDataBuf, nCurIndexOfBuf-2-2*i);
					if( nWord3 >= (emInt16)0xa3b0 && nWord3 <= (emInt16)0xa3b9 )  //是数字
					{
						//若前5个字中有数字，则“/”读成“每”
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc3bf );//每
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_r);//播放： 每（分钟）
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						nCurIndexOfBuf = nCurIndexOfBuf + 2;
						return nCurIndexOfBuf;
					}
				}
			}

			if ( nSumOfHanZi >= 2 ) 
			{//若X>=2时，查找《汉字量词表》
				if ( CheckHanZiLiangCiTable( strDataBuf, nCurIndexOfBuf + 2 ) != 0 )
				{ //返回值不为零，则在《汉字量词表》中
					//读成每
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc3bf );//每
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_r);//播放： 每（分钟）
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					nCurIndexOfBuf = nCurIndexOfBuf + 2;
					return nCurIndexOfBuf;
				}
			}

			nSumOfYingWen = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_YINGWEN );
			if ( nSumOfYingWen <= 3 )//“/"后跟的连续英文串长度< = 3， 查找《英文量词转汉字表》
			{   
				if ( CheckDanWeiTable( strDataBuf, nCurIndexOfBuf + 2 ) != 0 )
				{
					//读成每
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc3bf );//每
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_r);//播放： 每（分钟）
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
					CheckDanWeiTable( strDataBuf, nCurIndexOfBuf + 2 );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);//播放：字母量词
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 20 );//清空数字缓存中存储的内容
					nCurIndexOfBuf = nCurIndexOfBuf + 2 + nSumOfYingWen * 2;
					return nCurIndexOfBuf;
				}
			}
		}
		//---------------------------------------------小万更改6月12号end



		if ( nWord == (emInt16)0xa3ab ) //是"+"
		{
			//默认读成“加”的需求维持原状
			nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
			if ( nSumOfShuZi >= 13 )
			{
				nCurIndexOfBuf = nCurIndexOfBuf + 2;//"+"不发音
				ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );//调用函数将“+”后面的数字串按号码处理掉
				nCurIndexOfBuf = nCurIndexOfBuf + 2 * nSumOfShuZi;
				return nCurIndexOfBuf;
			}
		}

		if( nWord == (emInt16)0xa3aa ) //是”*“                {默认读成星 ，前后是数字的时候读成乘——小张 6月1日增加}
		{
			if( CheckDataType(strDataBuf, nCurIndexOfBuf - 2) == (emInt16)DATATYPE_SHUZI && CheckDataType(strDataBuf,nCurIndexOfBuf + 2) == (emInt16)DATATYPE_SHUZI)
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xb3cb );//乘
				PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_v);//播放：（五） 乘（四）
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
				nCurIndexOfBuf = nCurIndexOfBuf + 2;
				return nCurIndexOfBuf;
			}

		}
	}


	if( nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa && (nWord = GetWord (strDataBuf,nCurIndexOfBuf + 2)) == (emInt16)0xa3a0)//是"-"或者”—“并且“-”或者”—“后面有一个空格 （增加”—“——小张6月1日）
	{
		nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2) ;
		if( nWord == (emInt16)0xa3a0 )//特殊符号"-"前面是空格
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4);
			if( nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//符号"-"前面的空格前面不是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 )//空格后面是数字

				{
					nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2) );
					if( nWord  == (emInt16)0xa1e6 
						&& ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4)) != (emInt16)0xa1e6))//不带小数的数字后面是℃同时符号"-"前面的空格前面不是℃
					{
						//处理成零下摄氏度并播放（诸如 - 12）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 4 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2 ) ;
						return nCurIndexOfBuf;
					}
					else
					{
						if( nWord == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 6,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && 
								((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 3 ))) == (emInt16)0xa1e6) 
								&& ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4)) != (emInt16)0xa1e6))//小数点后面的数字的后面是℃同时空格的前面不是℃
							{
								//处理成零下摄氏度并播放（诸如-12.8℃）
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
								PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容

								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 4) ;//“-”后面的数字读成数值
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2) ;
								return nCurIndexOfBuf;
							}
						}
						//处理成 至  并播放（诸如 4 - 8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
						return nCurIndexOfBuf;
					}
				}
			}
			else//空格前面是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 ) 
				{
					//处理成 至  并播放（诸如 4- 8）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}

		else//特殊符号"-"前面不是空格
		{
			if (nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//“-”前面一位不是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				//nWord = GetWord(strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2));
				if ( nSumOfShuZi > 0)// && nWord == (emInt16)0xa1e6 )
				{
					nWord = GetWord(strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2));
					if( nWord == (emInt16)0xa1e6 && ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2)) != (emInt16)0xa1e6))//不带小数的数字后面是℃同时“-”前面不是℃
					{
						//处理成零下摄氏度并播放（诸如- 12）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 4 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2 ) ;
						return nCurIndexOfBuf;
					}
					else
					{
						if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2))) == (emInt16)0xa3ae )//数字后面有点（带小数的数字）
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 6,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && 
								((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 3 ))) == (emInt16)0xa1e6)
								&& ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2)) != (emInt16)0xa1e6))//小数点后面的数字的后面是℃同时特殊符号-前面不是℃
							{
								//处理成零下摄氏度并播放（诸如- 12.8）
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //零		
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //下
								PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//播放：零下
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//清空数字缓存中存储的内容
								
								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 4 ) ;//“-”后面的数字读成数值
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2 ) ;
								return nCurIndexOfBuf;
							}
						}
						//处理成 至  并播放（诸如 d- 8）
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
						nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
						return nCurIndexOfBuf;
					}
				}
			}
			else//“-”前面一位是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 ) 
				{
					//处理成 至  并播放（诸如4- 8）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//播放：至 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“-”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}

	}

	if( nWord ==(emInt16)0xa1ab || nWord == (emInt16)0xa3fe )//特殊符号“～” “~”后面接着是空格的情况 {增加（上划线的处理），也读成至}小张——6月1日
	{
		nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2) ;
		if( nWord == (emInt16)0xa3a0 )//特殊符号" ～"前面是空格
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4);
			if( nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//符号"～"前面的空格前面不是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 )//空格后面是数字
					//&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2) ) ) == (emInt16)0xa1e6 )//符号"-"后面的空格前面不是数字
				{
					//处理成 至  并播放（诸如 4 ～ 8）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
			else//空格前面是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 ) 
				{
					//处理成 至  并播放（诸如 4～ 8）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}
		else if( GetWord( strDataBuf, nCurIndexOfBuf + 2) == (emInt16)0xa3a0 )//特殊符号" ～"后面是空格
		{
			if (nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//“～”前面一位不是数字
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				//nWord = GetWord(strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2));
				if ( nSumOfShuZi > 0)// && nWord == (emInt16)0xa1e6 )
				{
					//处理成 至  并播放（诸如 d～ 8）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
			else//“～”前面一位是数字
			{
				if( GetWord( strDataBuf, nCurIndexOfBuf+4 ) == (emInt16)0xa3ad)		//下下一符号是“-”，例如 “-1 ～ -8℃。”   //hyl 2012-10-28
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 6, (emInt16)DATATYPE_SHUZI );
				else
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );


				if ( nSumOfShuZi > 0 ) 
				{
					//处理成 至  并播放（诸如4～ 8）
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //至
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：至 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//“～”后面的数字读成数值
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}

	}


	//是"+" ,  
	if ( nWord == (emInt16)0xa3ab ) 
	{
		nDataType1 = CheckDataType( strDataBuf, nCurIndexOfBuf-2);
		nDataType2 = CheckDataType( strDataBuf, nCurIndexOfBuf+2);
		
		//若前后数据类型不一致，则不读
		if(    nDataType1 != nDataType2)
		{
			nCurIndexOfBuf = nCurIndexOfBuf + 2;//"+"不发音
			return nCurIndexOfBuf;
		}
	}

	//“.”需要读出
	if( nWord == (emInt16)0xa3ae && g_hTTS->m_ControlSwitch.bIsReadDian == emTrue)
	{
		PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xb5e3); //点
		PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放：点 
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//清空数字缓存中存储的内容
		nCurIndexOfBuf += 2 ;
		return nCurIndexOfBuf;
	}



	//连续多个符号“-”或“—”，它们前后跟的都是汉字，则制造句尾停顿效果
	//例子：	也恰说明了中国现在的做法是正确的——虽然已经迟了多年！ 
	//例子：	也恰说明了中国现在的做法是正确的----虽然已经迟了多年！ 
	//例子：	也恰说明了中国现在的做法是正确的—虽然已经迟了多年！ 
	//例子：	也恰说明了中国现在的做法是正确的-虽然已经迟了多年！
	//例子：	G1—京哈高速（北京-哈尔滨）
	nWord = GetWord(strDataBuf,nCurIndexOfBuf);
	if(    (nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa) )			//是“-”或者“—”
		//&& CheckDataType(strDataBuf,nCurIndexOfBuf - 2) == DATATYPE_HANZI)  //且符号之前是汉字  deleted by hyl  2012-03-30
	{
		nTemp = 0;
		for(;;)
		{
			nTemp += 2;
			if( GetWord(strDataBuf,nCurIndexOfBuf+nTemp) != nWord)
				break;			
		}
		if( CheckDataType(strDataBuf,nCurIndexOfBuf+nTemp) == DATATYPE_HANZI)	//且连续多个符号之后是汉字
		{
			GenPauseNoOutSil();	//制造句尾停顿效果		
			nCurIndexOfBuf += nTemp ;
			return nCurIndexOfBuf;
		}
	}

	//“.”后跟汉字，要断句
	//例句：					1．乘客您好。1.乘客您好。
	if(    nWord == (emInt16)0xa3ae  			//是“.”
		&& CheckDataType(strDataBuf,nCurIndexOfBuf+2) == DATATYPE_HANZI)  //且符号之后是汉字
	{
		GenPauseNoOutSil();	//制造句尾停顿效果		
		nCurIndexOfBuf += 2 ;
		return nCurIndexOfBuf;
	}

	//是“＠”   例如：邮箱：tanxh@tts.net					//hyl  2012-04-09
	if(    nWord == (emInt16)0xa3c0 ) 			//是“＠”
	{
		GenPauseNoOutSil();	//制造句尾停顿效果		

		PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xb0ae); //爱
		PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , (emInt16)0xccd8); //特
		PutIntoWord( g_hTTS->m_ShuZiBuffer,   4 , END_WORD_OF_BUFFER );//ShuZiBuffer的结尾符
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//播放
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 6 );//清空数字缓存中存储的内容
		nCurIndexOfBuf += 2 ;

		GenPauseNoOutSil();	//制造句尾停顿效果		

		return nCurIndexOfBuf;
	}



	//剩下的	
	nCurIndexOfBuf = CheckGBKFuhaoToHanziDic( strDataBuf, nCurIndexOfBuf );//查GBK符号读成汉字表
	return nCurIndexOfBuf;
}




//****************************************************************************************************
//  功能：查GBK符号读成汉字表----GBKFuhaoToHanziDic
//	输入参数：当前字符串emStrA pCurStr，相对位移量emInt16  nCurIndex,当前为符号串的地址
//	返回值：返回nCurIndex
//****************************************************************************************************



emInt16  emCall  CheckGBKFuhaoToHanziDic( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nWordTemp = 0 ;
	emInt16 nHangIndex = 0 ;
	emInt16 nTemp = 0 ;
	emInt16 nWord1;
    emByte  nHighOfWord;
	emByte  nLowOfWord;
	emUInt8 nPos;

	LOG_StackAddr(__FUNCTION__);


	nHighOfWord  = *( pCurStr + nCurIndex) ;
	nLowOfWord  = *( pCurStr + nCurIndex + 1) ;

	//判断《韵律标注策略》
	if( g_hTTS->m_ControlSwitch.m_nManualRhythm == emTTS_USE_Manual_Rhythm_OPEN)
	{
		//手动标注：韵律短语“#”：	 记录手动标注的汉字的索引位置到g_hTTS->m_ManualRhythmIndex数组中去
		//注意：“#”起到分pau的作用，分词时只对每个pau进行分词
		if ( nHighOfWord == 0xa3 && nLowOfWord == 0xa3  )	//“#”=0xa3a3
		{
			//本符号是“#”
			g_hTTS->m_ManualRhythmIndex[g_hTTS->m_CurManualRhythmIndex] = (emInt8)g_hTTS->m_nHanZiCount_In_HanZiLink;
			g_hTTS->m_CurManualRhythmIndex++;

			nCurIndex += 2;
			return  nCurIndex;
		}

		//手动标注：韵律词“*”：	 暂不处理，直接将“*”去掉
		if ( nHighOfWord == 0xa3 && nLowOfWord == 0xaa  )	//“*”=0xa3aa
		{
			//本符号是“*”
			nCurIndex += 2;
			return  nCurIndex;
		}
	}
   
	if ( nHighOfWord >= 0xa1 && nHighOfWord <= 0xa3 && nLowOfWord >= 0xa0 && nLowOfWord <= 0xff )
	{//符号在0xa1a0至0xa3ff区
	  nHangIndex = (nHighOfWord -(emByte)0xA1)*96 + (nLowOfWord-(emByte)0xA0) + 1 ;	//行数偏移量
	}

	else if ( nHighOfWord >= 0xa8 && nHighOfWord <= 0xa9 && nLowOfWord >= 0x40 && nLowOfWord <= 0x9f )
	{//符号在0xa840至0xa99f区
		nHangIndex =(emInt16) ((nHighOfWord -(emByte)0xA8)*96 + (nLowOfWord-(emByte)0x40)) + (emInt16)288 + 1;	//行数偏移量
	}

	else
	{
		//不在以上两个区域 则默认不发音
			nCurIndex += 2;
			return  nCurIndex;
	}
     

	nTemp = (emInt16)(g_Res.offset_GbkFuhaoToHanzi + nHangIndex);
	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_GbkFuhaoToHanzi + (nHangIndex-1)*10 , 0);


	fFrontRead(&nWordTemp,1,1,g_hTTS->fResFrontMain);
	fFrontRead(&nPos,1,1,g_hTTS->fResFrontMain);

	if ( nWordTemp == 0 )//无需转汉字
	{
		if ( GetWord(pCurStr , nCurIndex) == (emInt16)INVALID_CODE )
		{
			//GenPauseNoOutSil();	//制造句尾停顿效果	hyl 1220
		}

		nCurIndex += 2;
		return  nCurIndex;
	}

	if ( nWordTemp == 1 )//是否需要转汉字==1
	{
		if (g_hTTS->m_ControlSwitch.m_bPunctuation == emTTS_PUNCTUATION_READ)//且读标点开关打开
		{
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain); //转成的第1个汉字的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain); //转成的第2个汉字的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //转成的第3个汉字的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //转成的第4个汉字的编码
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, nWordTemp );
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 8, END_WORD_OF_BUFFER  ); //ShuZiBuffer的结尾符
			SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,nPos);			//播放：符号转成的汉字 
			ClearBuffer( g_hTTS->m_ShuZiBuffer,  9 );            //清空数字缓存中存储的内容				
		}
		else				//且读标点开关关闭（不读标点时）
		{
			nWord1 =  GetWord( pCurStr, nCurIndex );				
		}
		nCurIndex += 2;
		return  nCurIndex;
	}

	if ( nWordTemp == 2 )  //是否需要转汉字==2时都需要将符号转成汉字
	{
		emInt16  nNextIndexSZBuffer;
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //转成的第1个汉字的编码
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, nWordTemp );
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //转成的第2个汉字的编码
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, nWordTemp );
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //转成的第3个汉字的编码
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, nWordTemp );
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //转成的第4个汉字的编码
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, nWordTemp );
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 8, END_WORD_OF_BUFFER  ); //ShuZiBuffer的结尾符
		nNextIndexSZBuffer = SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,nPos);			//播放：符号转成的汉字
		if( GetWord( (emPByte)g_hTTS->m_ShuZiBuffer, nNextIndexSZBuffer) == (emInt16)0xa1a3 )
		{
			//若转换后的汉字后带的是“。”，则表示本符号是这些类型：ⅰ ⅱ⒈⒉⑴⑵①②㈠㈡ⅠⅡ
			GenPauseNoOutSil();	//制造句尾停顿效果		
		}
		ClearBuffer( g_hTTS->m_ShuZiBuffer,  9 );           //清空数字缓存中存储的内容
		nCurIndex += 2;
		return  nCurIndex;			
	}
	return  nCurIndex;	

}








