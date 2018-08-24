#include "emPCH.h"

#include "Front_ProcShuZi.h"


//****************************************************************************************************
//函数功能：遇到连续的纯数字串，通过判断其前后的字符，将纯数字串转换成汉字且进行相应处理
//输入参数：纯数字串，其前后相关联的字符
//输出参数：处理完数字串及其相关后的相对偏移量emInt16 nCurIndexOfBuf
//****************************************************************************************************


emInt16 emCall ProcessShuZi( emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16	nDataType = 0, nDataType1;
    emInt16 nSumOfShuZi = 0;
	emInt16 nSumOfReadHaoMa = 0 ;
	emInt16 nSumOfReadShuZhi = 0 ;
	emInt16 nCurIndex = nCurIndexOfBuf ;
	emInt16 nWord = 0 ;
	emInt16 nWordTemp = 0 ,nWordTemp2 = 0 ,nWordPrev,nWordNext;
	emInt16 nSumOfReadDate = 0 ;
	emInt16 nSumOfReadTime = 0 ;
	emInt16 nSumTemp = 0 , nSumTemp1 = 0,nSumTemp2 = 0;
	emInt16 nTemp = 0, nTemp1 = 0 ;
	emInt16 nTempLong=0;
	emInt16 i,nBaiFenHaoType;

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf, DATATYPE_SHUZI );
   
	if( nSumOfShuZi == 0 )
	{
		return emNull;
	}

    //开始数字的处理
	//first.发声策略为号码时，数字按号码发音
	if( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AS_NUMBER )
    {
	   nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
	   nCurIndexOfBuf += nSumOfReadHaoMa * 2;
	   //数字串后是否为“-”：例手机来电是：139-1234-5678
	   if( (GetWord( strDataBuf,nCurIndexOfBuf) == (emInt16)0xa3ad) || (GetWord( strDataBuf,nCurIndexOfBuf) == (emInt16)0xa1aa) )
	   {//start
		   if( DATATYPE_SHUZI == CheckDataType( strDataBuf, nCurIndexOfBuf + 2) ) 
		   { //start--1   
			   GenPauseNoOutSil();	//制造句尾停顿效果		
			   nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf,nCurIndexOfBuf + 2);
			   if( (GetWord( strDataBuf,nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa3ad) ||(GetWord( strDataBuf,nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa1aa) )
			   {//start--2
				   i = nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ;
				   if( DATATYPE_SHUZI == CheckDataType( strDataBuf, i + 2) )
				   {
					   GenPauseNoOutSil();	//制造句尾停顿效果		
					   nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, i + 2);
					   nCurIndexOfBuf =  i + 2+ nSumOfReadHaoMa*2;
					   return nCurIndexOfBuf;  //end of XXXX-XXXXXXXX-XXXXX
				   }
			   }//end--2
			   nCurIndexOfBuf =  nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ;
			   return nCurIndexOfBuf; //end of XXXX-XXXXXXX
		   }//end--1
	   }//end
	   return nCurIndexOfBuf; //end of XXXXX
    }//end of first,goto exit
	//third:发声策略为自动判断(默认) 或 发声策略为按数值读
	else		//if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO  || g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AS_VALUE  )
	{  
		if ( nSumOfShuZi > 16) //3.1如果数字串的长度大于16位的话
		{
			if( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AS_VALUE )
			{
				//发声策略为按数值读： 如果连续的数字串大于16位的话，那么截取前面的16位读成数值后面的数字丢弃
				nCurIndex += 32;
				for ( i = 0; i < (nSumOfShuZi-16); i++ )
				{
					PutIntoWord( strDataBuf, nCurIndex, (emInt16)DATATYPE_UNKNOWN );
					nCurIndex += 2;
				}
				nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
				nCurIndexOfBuf += nSumOfReadShuZhi * 2;
				return nCurIndexOfBuf; //end of ReadShuZhi
			}
			else
			{
				//发声策略为自动判断： 3.1如果数字串的长度大于16位的话，则数字按号码读
				nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
				nCurIndexOfBuf += nSumOfReadHaoMa * 2;
				return nCurIndexOfBuf; //end of XXXXXXXXXXXXXXXXXReadHaoMa
			}

		}//end of third,goto exit
		else			 //3.2数字串小于16位时，start----
	    {

			 //判断：是否百分号类型
			 //50%。50.4%。30-50%。30.4-50.4%。30-50.4%。30.4-50%。30%-50%。
			 //50﹪。50.4﹪。30-50﹪。30.4-50.4﹪。30-50.4﹪。30.4-50﹪。30﹪-50﹪。
			 //50‰。50.4‰。30-50‰。30.4-50.4‰。30-50.4‰。30.4-50‰。30‰-50‰。
			 nBaiFenHaoType = CheckIsTypeBaiFenHao(strDataBuf, nCurIndex);
			 if( nBaiFenHaoType>0 )
			 {
				 if( nBaiFenHaoType == 1 || nBaiFenHaoType == 2)		//百分号
				 {
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb0d9 ); //百
				 }
				 else													//千分号
				 {
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xc7a7 ); //千
				 }
				 PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb7d6 ); //分
				 PutIntoWord( g_hTTS->m_ShuZiBuffer,4, 0xd6ae ); //之
				 PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, END_WORD_OF_BUFFER );
				 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );	//播放：百分之 或 千分之
				 ClearBuffer( g_hTTS->m_ShuZiBuffer, 7);

				 if( nBaiFenHaoType == 2 || nBaiFenHaoType == 4 )		// 有“-”：  30%-50%
				 {
					 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2 + 2;
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //至
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,2, END_WORD_OF_BUFFER );
					 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );	//播放：至
					 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2 + 2;
				 }
				 else													// 无“-”：  30%
				 {
					 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2 + 2;
				 }
				 return nCurIndexOfBuf;
			 }


             nCurIndex += nSumOfShuZi * 2; 
			 nWord = GetWord( strDataBuf,nCurIndex);


			 //判断：数字串后是否接“μ”
			 if ( nWord == (emInt16)0xa6cc )
			 {//“μ”之后紧跟长度为1的英文串：则读成：数值+“微”+中文单位
				 if ( GetStrSum( strDataBuf, nCurIndex + 2, DATATYPE_YINGWEN ) == 1 )
				 {
                     if ( CheckDanWeiTable( strDataBuf, nCurIndex + 2) != 0 )
                     {
						 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//读数值
						 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcea2 ); //微
						 PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_a);	//播放：微（米）
						 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						 CheckDanWeiTable(strDataBuf, nCurIndex + 2);//将英文单位转汉字
						 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：字母单位
						 ClearBuffer( g_hTTS->m_ShuZiBuffer,  11 );			//清空数字缓存中存储的内容
						 nCurIndexOfBuf += nSumOfReadShuZhi * 2 + 4;
						 return nCurIndexOfBuf; //end of ReadData
                     }
				 }
			 }

			// 判断：数字串后的1个字符X是否在( -  /  \   . )中 
			 if( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae)) 
			 {//日期类型判断开始
                 nSumOfReadDate = CheckDateFormat( strDataBuf, nCurIndexOfBuf ); //返回处理了几个数字
                if ( nSumOfReadDate != 0 )		//日期类型检查值返回不为零时则可判断为是合格的日期类型
                 {
			         	 nCurIndexOfBuf += nSumOfReadDate;
						 return nCurIndexOfBuf; //end of ReadData
                  } //end of XXXX-XX-XX or XXXX/XX/XX or XXXX\XX\XX or XXXX.XX.XX or others
				//判断若不是日期类型，先转“小数点类型”处理
			    if ( nWord == (emInt16)0xa3ae )
				 {//小数点的判断开始 
					nDataType = CheckDataType( strDataBuf, nCurIndex + 2); 
						if ( nDataType == DATATYPE_SHUZI )			
					     {//小数点的后面是数字 start   
							nSumTemp = GetStrSum( strDataBuf, nCurIndex + 2, DATATYPE_SHUZI );
							nWordTemp = GetWord( strDataBuf,nCurIndex + 2 + nSumTemp*2 );
							
							if ( CheckDanWeiTable(strDataBuf, nCurIndex + 2 + nSumTemp*2) != 0 )//数字后有英文串单位
							{ 
								nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//读数值
								PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb5e3 ); //点
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);	//播放：点
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
								nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+nSumOfReadShuZhi*2+2);//读号码
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
								nSumTemp = CheckDanWeiTable(strDataBuf, nCurIndex + 2 + nSumTemp*2);//将英文单位转汉字
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：字母单位
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  11 );			//清空数字缓存中存储的内容
								nCurIndexOfBuf +=  nSumOfReadShuZhi*2+2+nSumOfReadHaoMa*2 + nSumTemp*2;
								return nCurIndexOfBuf; //end of ReadShuZi To 小数数值+英文单位转后的汉字
							}//end of XX.XXkg or XX.XXg or others

							//ip地址等类型：  IP: 192.168.73.129
							if ( (nWordTemp == (emInt16) 0xa3ae) )//小数点后面的数字后还有小数点
							{
								 while ( (nWordTemp == (emInt16) 0xa3ae) )
								 {																
									 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);									 
									 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb5e3 ); //点
									 PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);	//播放：点
									 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
									 nCurIndexOfBuf +=  2+nSumOfReadHaoMa*2;
									 nSumTemp = GetStrSum( strDataBuf, nCurIndexOfBuf, DATATYPE_SHUZI );
									 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp);
								 }

								 if ( CheckDataType( strDataBuf, nCurIndexOfBuf) == DATATYPE_SHUZI )	
								 {
									 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);									 
									 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;

								 }
								 
								 return nCurIndexOfBuf; 
							}


							 //剩下的是：小数点类型：234.11 
							 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
							 nCurIndexOfBuf +=  nSumOfReadShuZhi*2;
							 if ( (GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa3ba)||(GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa1c3) )
							 { //小数后是否有符号“:”
								 g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen = 1;  //《符号：是否处理成比分开关》打开
							 }
							 return nCurIndexOfBuf; //end of ReadShuZi To XX.XX	


					     }//小数点的后面是数字 end
				 }//小数点的判断结束				
			 }//日期类型判断结束


			 //判断：数字后面接有":",时间类型判断开始
			 if ( (nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3) ) 
			 {
				 if ( g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen == 1)
				 {//此时数字串优先读成比分格式不读成时间，跳出时间类型检测,退出数字串的处理程序
					 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//数字串读成数值
					 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
					 return nCurIndexOfBuf; 
				 }//end of ReadShuZi To ShuZhi,it will go to ProcFuHao,Read ":" To "比"


				 //若临近的7个汉字中有“比”，临近的1个汉字中有“以”，则处理成比分格式，且打开比分开关
				 for(i = 0;i < 7; i++)
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf-2-i*2);
					 if(    nWordTemp == (emInt16)0xb1c8   //等于“比” 
						 || (i == 0 && nWordTemp == (emInt16)0xd2d4 ))			//数字前的字是：“以”  例如：世界冠军杨影以22∶21险胜福

					 {
						 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//数字串读成数值
						 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
						 g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen = 1;
						 return nCurIndexOfBuf; 
					 }
				 }


				 //如果读比分开关没打开，即临近的前面的汉字串中不含有“比”字，这时时间格式的优先权高
				 nSumOfReadTime = CheckTimeFormat( strDataBuf, nCurIndexOfBuf ); //返回处理了几个数字及数字相关
				 if ( nSumOfReadTime != 0 )		//是合格的时间类型
				 {
					 nCurIndexOfBuf += nSumOfReadTime * 2;
					 return nCurIndexOfBuf; //end of ReadShuZi To 时间XX:XX:XX
				 } //end of XX:XX:XX
				 //不是时间格式时，则考虑读成“比”
				 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
				 g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen = 1;  //将《符号：是否处理成比分开关》打开
				 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
				 return nCurIndexOfBuf; 
			 }
	

			//判断：数字串是否为“3G” 或则"4S"格式
            if( ((nWord == (emInt16)0xa3e7) || (nWord == (emInt16) 0xa3f3)) && (GetStrSum(strDataBuf, nCurIndexOfBuf+2, DATATYPE_YINGWEN)==1) )
              {
			    nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
				if ( ((nWordTemp == (emInt16)0xa3b3) && (nWord == (emInt16)0xa3e7)) || ((nWordTemp == (emInt16)0xa3b4) && (nWord == (emInt16)0xa3f3)))//3G or 4S
				{
					nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
					nCurIndexOfBuf += nSumOfReadShuZhi*2;
					return nCurIndexOfBuf; //end of ReadShuZi  
				}
              }
		
		     //判断：数字串后跟的是英文串,《查是否英文单位表》处理，根据返回的值将位置指针向前增长，转结束
			 if( DATATYPE_YINGWEN == CheckDataType( strDataBuf, nCurIndex ) ) 
			 {   
				 nSumTemp = GetStrSum( strDataBuf, nCurIndex, DATATYPE_YINGWEN );

				 //若不是此格式：数字串+英文串+数字串  例如：BR50V3。BR50V。U6BC50L6L6gL2Z1K1Z3BC60L7
				 if(    DATATYPE_SHUZI != CheckDataType( strDataBuf, nCurIndex+nSumTemp*2 ) 
					 && DATATYPE_YINGWEN != CheckDataType( strDataBuf, nCurIndex -nSumOfShuZi*2-2 ))  
				 {				 
					 //调用查询数字后的英文是否在英文符号单位表中子函数
					 nSumTemp = CheckDanWeiTable( strDataBuf, nCurIndex );
					 //if( nSumTemp == 1 || nSumTemp == 2 )
					 if( nSumTemp != 0 )
					  { 
						  nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
						  CheckDanWeiTable( strDataBuf, nCurIndex );
						  SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//播放：字母单位
						  ClearBuffer( g_hTTS->m_ShuZiBuffer,  21 );			//清空数字缓存中存储的内容
						  nCurIndexOfBuf +=  nSumOfReadShuZhi*2 + nSumTemp * 2;
						  return nCurIndexOfBuf; //end of ReadShuZi To 整数数值+英文单位转后的汉字
					  }//end of XXkg,XXg or others
				 }
			 }
			//判断：数字串后的字符X是否在(月，日，时，分，秒)中
			 if( nWord == (emInt16)0xd4c2 || nWord == (emInt16)0xc8d5 || nWord == (emInt16)0xcab1 || nWord == (emInt16)0xb7d6 || nWord == (emInt16)0xc3eb)
			 {
			    nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
				if( nWordTemp == (emInt16)0xa3b0 )//数字串出现0开头,按数值处理;级别优先于"以0开头的数字串都读成了号码"  
				{
					nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );     
					nCurIndexOfBuf +=  nSumOfReadShuZhi*2;
					return nCurIndexOfBuf; //end of ReadShuZi To 数值 + 月等
				}//end of 0X月 or 0X日 or 0X时 or 0X分 or 0X秒
			 }

			 if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )
			 {
				//判断：数字串后的字符X是否在(年 折  路 号)中	
				 if( nWord == (emInt16)0xc4ea || nWord == (emInt16)0xd5db || nWord == (emInt16)0xc2b7 || nWord == (emInt16)0xbac5)
				 {   
					 nSumOfReadHaoMa = 0;
					 nTemp = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
					 switch ( nWord )
					 {
					 //“年”：号码 + “年” (“1”一定不能读成幺)
					 case (emInt16)0xc4ea:  
								  if(    (nSumOfShuZi==4 && nWordTemp<(emInt16)0xa3b3)		//“年”前的数字串必须为4位且第1位小于3
									  || (nSumOfShuZi==2 && nWordTemp==(emInt16)0xa3b0) //“年”前的数字串是2位但以0开头
									  || ( nTemp>=(emInt16)91 && nTemp<=(emInt16)99 ))	//“年”前的数字串范围在91至99之间
								   {
									 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
									 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
									 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
									 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
									 break;//end of XXXX年 or 0X年 or 91年 99年
								   }
								  if ( ((GetWord( strDataBuf, nCurIndex + 2 + 2 )) == (emInt16)0xd4c2) || ((GetWord( strDataBuf, nCurIndex + 2 + 2 + 2)) == (emInt16)0xd4c2) )
								  {//满足“年”之后的第2或第3个字符是“月” XX年X月 XX年XX月
									  nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
									  g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
									  nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
									  g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
									  break;//end of XX年X月 XX年XX月
								  }
								  break;
								  
					//“折”：号码 + “折”
					 case (emInt16)0xd5db:  
									 nWordTemp = GetWord( strDataBuf,nCurIndex+2 );		//“折”后面的字
									 if( nWordTemp == (emInt16)0xbfdb )					//是“扣”字  
									 {
										 //例如： 每100元送30折扣券。
										 nSumOfReadHaoMa = 0;		//不处理
									 }
									 else
									 {
										 if( GetStrSum(strDataBuf,nCurIndexOfBuf,DATATYPE_SHUZI) == 1)		
										 {	 //1位数字：按数值读		 例如：1折至2折
											 nSumOfReadHaoMa = ReadDigitShuZhi(strDataBuf,nCurIndexOfBuf);										 
										 }
										 else
										 {	 //大于1位数字：按号码读  例如：95折  15折  91折
											 nTemp1 = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
											 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;			//“1”读成“一”
									 		 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
											 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp1;							//还原
										 }
									 }
									 break;//end of XX折
					//“路”：号码 + “路”(注意：数字必须为3位或以上)(“1”读成幺)			  
					 case (emInt16)0xc2b7:  if ( nSumOfShuZi >= 3 )
									{										 
										 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);

									}break;//end of XXX路
						 //“号”：号码 + “号”(注意：数字必须为3位或以上)(“1”读成幺)			  
					 case (emInt16)0xbac5:  if ( nSumOfShuZi >= 3 )
											{
												nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
											}break;//end of XXX号
					 default: break;
					 }
					 if ( nSumOfReadHaoMa != 0 )
					 {
						 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
						 return nCurIndexOfBuf; //end of ReadShuZi To 号码 + 年 /折 / 路/号
					 }
				 }

				 //判断：数字串后的汉字是否在(数据表：HanZiLetDigitToPhoneTable)中
				 if( DATATYPE_HANZI==(CheckDataType(strDataBuf, nCurIndex)) ) 
				 {
					 //查表看是否在表Table_HanZiLetDigitToPhoneTable中
					 if ( CheckHanZiLetDigitToPhoneTable( strDataBuf,  nCurIndex, 3 ) == emTrue)
					 {//数字串按号码处理 (注意：数字必须为3位或以上)(“1”读成幺)(例如：110国道，1105房间)
						if ( nSumOfShuZi >= 3 )
						{
							nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
							nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							return nCurIndexOfBuf; //end of ReadShuZi To 号码 + 国道 /房间 等
						}
					 }
				 }


				 //判断：数字串后的3个汉字以内是否出现了“室”   //122会议室。915室。412教室。
				 if( GetWord(strDataBuf, nCurIndex) == (emInt16)0xcad2 
					 || GetWord(strDataBuf, nCurIndex+2) == (emInt16)0xcad2 
					 || GetWord(strDataBuf, nCurIndex+4) == (emInt16)0xcad2) 
				 {					 
					if ( nSumOfShuZi >= 3 )
					{
						nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
						nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
						return nCurIndexOfBuf; //end of ReadShuZi To 号码 + 国道 /房间 等
					}
				 }

			 } //endof ： if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )


			 //判断：数字串后的字符X是否在(0xa1a4)中  9·11恐怖袭击   纪念12·9事件	
			 if( nWord == (emInt16)0xa1a4 )
			 {   
				 if ( DATATYPE_SHUZI == CheckDataType( strDataBuf, nCurIndex + 2))
				 {
					 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
					 nCurIndexOfBuf +=  2*nSumOfReadHaoMa + 2;
					 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
					 nCurIndexOfBuf +=  2*nSumOfReadHaoMa ;
					 return nCurIndexOfBuf;
				 }
                 				
			 }

			 if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )
			 {
		 
				 //若: 1)数字后是符号 ” ＂  ） ，2)数字前是符号“（”且紧跟数字且数字后是空格， 则处理成号码 ，且打开读号码开关   
				 //例如：(86)8471 7972。（86）84717972。莫乱打“122”报警电话。莫乱打"122"报警电话。（12小时恭候）。（86 22）52131310。
				 nSumTemp1 = GetStrSum( strDataBuf, nCurIndexOfBuf,DATATYPE_SHUZI );
				 nWordPrev = GetWord( strDataBuf,nCurIndexOfBuf-2 );
				 nWordNext = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp1 );
				 if( nSumTemp1 > 1)		//连续数字个数>1
				 {
					 if(   ((nWordPrev == (emInt16)0xa1b0 || nWordPrev == (emInt16)0xa3a0 ) && nWordNext == (emInt16)0xa1b1) 	//符号对：“ ” ， 前符号是空格也可
						|| ((nWordPrev == (emInt16)0xa3a2 || nWordPrev == (emInt16)0xa3a0 ) && nWordNext == (emInt16)0xa3a2) 	//符号对： "＂ ， 前符号是空格也可
						|| ((nWordPrev == (emInt16)0xa3a8 || nWordPrev == (emInt16)0xa3a0 ) && nWordNext == (emInt16)0xa3a9) 	//符号对： ()  或 （） ， 前符号是空格也可
						|| (nWordPrev == (emInt16)0xa3a8 && CheckDataType( strDataBuf,nCurIndexOfBuf)==DATATYPE_SHUZI && nWordNext==(emInt16)0xa3a0)		//符号 (8				
						)							
					 {
						 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
						 nCurIndexOfBuf +=  nSumTemp1*2;
						 if( nWordPrev != (emInt16)0xa3a8 )
							 nCurIndexOfBuf += 2;
						 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
						 GenPauseNoOutSil();	//制造句尾停顿效果
						 return nCurIndexOfBuf; 			 
					 }
				 }


				 //若数字后是符号 - ，且数字串个数>=3 （但符号-后面不能跟字母和汉字），则处理成号码 ，且打开读号码开关   
				 //例如：139-1111-8888。 需测：30-50%。比分为：12-15 。(86-25)8471 7972。 60G-70G。60-70g。60-70克。
				 nSumTemp1 = GetStrSum( strDataBuf, nCurIndexOfBuf,DATATYPE_SHUZI );
				 nWordNext = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp1 );
				 if(   nWordNext == (emInt16)0xa3ad	)	 //符号 -
				 {

					 nSumTemp2 = GetShuZiSum( strDataBuf,nCurIndexOfBuf+2*nSumTemp1+2 );
					 nDataType1 = CheckDataType( strDataBuf,nCurIndexOfBuf+2*nSumTemp1+2 +2*nSumTemp2);
					 if( nDataType1 != DATATYPE_YINGWEN &&  nDataType1 != DATATYPE_HANZI && nSumTemp2>=3)
					 {
						 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
						 nCurIndexOfBuf +=  nSumTemp1*2+2;
						 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
						 GenPauseNoOutSil();	//制造句尾停顿效果
						 return nCurIndexOfBuf; 
					 }				 			 
				 }			


				 //若连续数字串的个数为10位，前3位为400或800，则识别成号码，且打开读号码开关。
				 //例如：4008101666。 8008100010。
				 if( nSumOfShuZi == 10 ) //长度为10
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
					if( nWordTemp == (emInt16)0xa3b4 || nWordTemp == (emInt16)0xa3b8 )  //第1位是“4”或“8”
					{
						nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+2 );
						if ( nWordTemp == (emInt16)0xa3b0  )  //第2位是“0”
						{
							nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+4 );
							if ( nWordTemp == (emInt16)0xa3b0  )  //第3位是“0”
							{
#if 1						 

								//与其它号码不一样的停顿读法：  4008101666 ->  400-810-1666
								 #define FIRST_STOP_WEI	  3
								 #define SECEND_STOP_WEI  6
								 nSumOfReadHaoMa = GetStrSum(strDataBuf,nCurIndexOfBuf,DATATYPE_SHUZI);
								 *(strDataBuf + nCurIndexOfBuf + FIRST_STOP_WEI*2 + 1) += 10;
								 ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
								 GenPauseNoOutSil();	//制造句尾停顿效果		
								 *(strDataBuf + nCurIndexOfBuf + FIRST_STOP_WEI*2 + 1) -= 10;
								 *(strDataBuf + nCurIndexOfBuf + SECEND_STOP_WEI*2 + 1) += 10;
								 ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+FIRST_STOP_WEI*2);
								 GenPauseNoOutSil();	//制造句尾停顿效果		
								 *(strDataBuf + nCurIndexOfBuf + SECEND_STOP_WEI*2 + 1) -= 10;
								 ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+SECEND_STOP_WEI*2);
								 GenPauseNoOutSil();	//制造句尾停顿效果	
								 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
								 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
								 return nCurIndexOfBuf; 

#else						
								 //与其它号码一样的停顿读法：  4008101666 ->  40-0810-1666
								 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
								 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
								 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
								 return nCurIndexOfBuf; 
#endif
							}
						}
					}
				 }
			 
	            
				 //判断：是否以“0”开头
				 //if(((GetWord(strDataBuf,nCurIndexOfBuf)) == (emInt16)0xa3b0) && (nSumOfShuZi >= 3))
				 if(((GetWord(strDataBuf,nCurIndexOfBuf)) == (emInt16)0xa3b0) && (nSumOfShuZi >= 2))  //hyl 20111228  例子：E07
				 {
					 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
				 }


				 //若数字后是符号 / ，且读号码开关已打开，则处理成号码 ，且打开读号码开关  
				 //例如：086/0551/5331800。
				 nSumTemp1 = GetStrSum( strDataBuf, nCurIndexOfBuf,DATATYPE_SHUZI );
				 nWordNext = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp1 );
				 if(  (nWordNext == (emInt16)0xa3af  && g_hTTS->m_ControlSwitch.m_bIsPhoneNum == emTrue) )	 //符号 ／
				 {
					 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
					 nCurIndexOfBuf +=  nSumTemp1*2+2;
					 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
					 GenPauseNoOutSil();	//制造句尾停顿效果
					 return nCurIndexOfBuf; 			 
				 }			


				 //判断：“后续数字读成号码”全局开关是否打开		
				 if( g_hTTS->m_ControlSwitch.m_bIsPhoneNum == emTrue )
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndex );

					 //为了正确识别以下************************
					 //			 例如：请您到五楼客服中心领取3000元晚会入场券一张。请打客服3000。
					 //			 例如：短信3000条。短信3000元。短信3000。
					 //			 例如：手机3000个。手机3000元。手机3000。
					 //			 例如：拨打电话3000次。拨打电话3000分钟。拨打电话3000秒。拨打电话3000。
					 //			 例如：咨询3000人。咨询95588。 交通银行大厦向南200米
					 //			 例如：ad400。  D121次列车，Z121次列车 
					 //			 例如：拨打电话95588或95578。拨打电话95588与95578。拨打电话95588和95578。
					 //			 例如：来自120多个国家的有关官员和世界银行。来自社会的148名专业指导委员会委员。

					 //且数字串字数>=3,且数字串后不是以下量词，则读成号码
					 if (    nSumOfShuZi >= 3 
						  && nWordTemp != (emInt16)0xd4aa			//元						  
						  && nWordTemp != (emInt16)0xccf5			//条
						  && nWordTemp != (emInt16)0xb8f6			//个
						  && nWordTemp != (emInt16)0xc8cb			//人
						  //&& nWordTemp != (emInt16)0xb4ce			//次		例如：D121次列车，Z121次列车  hyl 2012-03-30
						  && nWordTemp != (emInt16)0xb7d6			//分
						  && nWordTemp != (emInt16)0xc3d7			//米
						  && nWordTemp != (emInt16)0xb6e0			//多
						  && nWordTemp != (emInt16)0xc3fb			//名
						  && nWordTemp != (emInt16)0xcdf2			//万 元	
						  && nWordTemp != (emInt16)0xd2da			//亿 元		例如：2011年整个商业银行净利润为10412亿元  hyl 2012-04-09
						  && nWordTemp != (emInt16)0xc3eb)			//秒
					 {
						 //数字串后是否为“-”：例手机来电是：139-1234-5678， 电话：010-62986600 ）
						 nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf ,nCurIndexOfBuf );
						 if( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa1aa) )
						 {//start
							   if( DATATYPE_SHUZI == CheckDataType( strDataBuf, nCurIndex + 2) ) 
							   { //start--1   

									GenPauseNoOutSil();	//制造句尾停顿效果

									nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf,nCurIndex + 2);
									if( (GetWord( strDataBuf,nCurIndex + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa3ad) ||(GetWord( strDataBuf,nCurIndex + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa1aa) )
									{//start--2
										  i = nCurIndex + 2 + nSumOfReadHaoMa*2 ;
										  if( DATATYPE_SHUZI == CheckDataType( strDataBuf, i + 2) )
										  {
											  GenPauseNoOutSil();	//制造句尾停顿效果

											  nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, i + 2);
											  nCurIndexOfBuf =  i + 2+ nSumOfReadHaoMa*2;
											  return nCurIndexOfBuf;  //end of XXXX-XXXXXXXX-XXXXX
										  }
									}//end--2

									nCurIndexOfBuf =  nCurIndex + 2 + nSumOfReadHaoMa*2 ;
									return nCurIndexOfBuf; //end of XXXX-XXXXXXX
							   }//end--1
						 }//end
					 
						 nCurIndexOfBuf = nCurIndex;
						 return nCurIndexOfBuf; //end of XXXXX
					 }
					 
					 if( nSumOfShuZi < 3 )		//后来加的		例如：ad40。 X86系列。电话55966569，12小时恭候。
					 {
						 if( CheckDataType( strDataBuf ,nCurIndexOfBuf-2)== DATATYPE_YINGWEN)  //hyl  2012-03-30
						 {
							 nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf += nSumOfReadHaoMa*2 ;
							 return nCurIndexOfBuf; 
						 }
					 }
				 }
			 


				 //若连续数字串的个数为11位，只要前2位为：13或15或18，自动识别成手机号码
				 //手机前3为号段统计：130,131,132,133,134,135,136,137,138,139,150,151,152,153,155,156,157,158,159,180,185,186,187,188,189
				 if( nSumOfShuZi == 11 ) //长度为11
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
					if( nWordTemp == (emInt16)0xa3b1 )  //第1位是“1”
					{
						nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+2 );
						if ( nWordTemp == (emInt16)0xa3b3 || nWordTemp == (emInt16)0xa3b5 || nWordTemp == (emInt16)0xa3b8  )  //第2位是“3”或“5”或“8”
						{
							 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
							 return nCurIndexOfBuf; 
						}
					}
				 }



				 //若数字串是7位或以上（最保险的是：7位或8位），且后面的第1个字不是汉字（注意：数字与汉字之间的空格在之前已去除），则识别成电话号码。
				 if( nSumOfShuZi == 7  || nSumOfShuZi == 8  )	//hyl 2012-03-29
				 {
					 //若后字不是汉字，或是汉字“转”和“或”，或刚好等于8位，则处理成号码
					if( DATATYPE_HANZI != CheckDataType( strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2 )   
						|| GetWord( strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2 ) ==  ( emInt16 ) 0xd7aa		
						|| GetWord( strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2 ) ==  ( emInt16 ) 0xbbf2  
						|| nSumOfShuZi == 8 )
					{
							 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
							 return nCurIndexOfBuf; 
					}
				 }

				 //若第1个数字为8，且数字前的字为符号+，且符号+前不是数字，则处理成号码和打开读号码开关
				 //例如： +852 2185 6460 。   +852-28220122。 +86 21 61224911。+86(0)21 6495 1616。+86+755+83667282。2+8。 34 + 86 = 120。
				 nWordPrev = GetWord( strDataBuf,nCurIndexOfBuf-2);
				 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf);
				 if(   nWordPrev == (emInt16)0xa3ab &&  nWordTemp == (emInt16)0xa3b8   //  为：+8
					 && CheckDataType( strDataBuf,nCurIndexOfBuf-4)!=DATATYPE_SHUZI)
				 {
					 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
					 nCurIndexOfBuf +=  nSumTemp1*2;
					 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
					 GenPauseNoOutSil();	//制造句尾停顿效果
					 return nCurIndexOfBuf; 			 
				 }
			 } //endof ： if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )


			 // “/"的特殊处理
			 if ( nWord == (emInt16)0xa3af )//右斜杠的判断开始，考虑几分之几的情况如（4/5: 五分之四） 
			  {
				 nDataType = CheckDataType( strDataBuf, nCurIndex + 2); 
				 if( nDataType == DATATYPE_SHUZI  										//  后面是数字(可以是1位)
					 && CheckDataType( strDataBuf, nCurIndex - 4 ) != DATATYPE_SHUZI)	//	前面只能是1位数字
				 {
					 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndex + 2 );
					 nSumTemp = nSumOfReadShuZhi;
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb7d6 ); //分
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xd6ae ); //之
					 PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, END_WORD_OF_BUFFER );
					 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);	//播放：分之
					 ClearBuffer( g_hTTS->m_ShuZiBuffer, 5);
					 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2+2+nSumTemp*2;
					 return nCurIndexOfBuf; //end of ReadShuZi To X/X读成分数
				 }//end of X/X
			  }

		
			if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )
			{
				 //判断：连续数字串的长度为4位，
				 //则：  数字串按年读（一不能读成幺）
				 if ( nSumOfShuZi == 4 )
				 {
					 //modified by hyl 2012-04-09
					 //例如：该省1955-1956年捕虎171只。年峰值日照时数在1600-2200小时之间。年峰值日照时数在1955-2200小时之间
					 nTemp = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 if (    ( nTemp >= 1900 && nTemp <= 2050)					//数字范围：1900年-2050年
						  && (   (nWord == (emInt16)0xa3ad)						//符号：“-”“～”	“~”	
						      || (nWord == (emInt16)0xa1ab) 
							  || (nWord == (emInt16)0xa3fe)))	
					 {
						 if( CheckDataType(strDataBuf,nCurIndexOfBuf+10) == DATATYPE_SHUZI)  
						 {
							 nTemp = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf+10 );
							 if( nTemp >= 1900 && nTemp <= 2050)							 //符号后跟的是数字范围：1900年-2050年
							 {
								 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
								 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
								 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
								 return nCurIndexOfBuf; 
							 }
						 }
					 }
				 }



				 //无条件将以下5位数识别成号码   ：  10010，10060，10086，12315，17909，17911，17950，17951
				 if ( nSumOfShuZi == 5 )
				 {
					 nTempLong = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 if (    nTempLong == 10010 ||  nTempLong == 10060 || nTempLong == 10086 || nTempLong == 12315
						  || nTempLong == 17909 ||  nTempLong == 17911 || nTempLong == 17950 || nTempLong == 17951) 
					 {
						 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
						 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
						 return nCurIndexOfBuf; 
					 }
				 }


				 //有条件将以下3位数识别成号码   ：  110，114，119，120，122		added by hyl 2012-03-30
				 //例如：  莫乱打122报警电话。网络化技术,将110、119、120、 122 等接处警调度系统。
				 //例如：  110。114。119。120。122。
				 //例如：  共有110个人。支付122元。
				 if ( nSumOfShuZi == 3 )
				 {
					 nTempLong = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 if (    nTempLong == 110 ||  nTempLong == 114 || nTempLong == 119 
						  || nTempLong == 120 ||  nTempLong == 122 ) 
					 {
						 //若本数字串后不是汉字和字母（且前面还有汉字），则读成号码
						 if(    nCurIndexOfBuf >= 2 
							 && CheckDataType( strDataBuf,nCurIndexOfBuf-2) == DATATYPE_HANZI		
							 && CheckDataType( strDataBuf,nCurIndexOfBuf+6) != DATATYPE_HANZI	
							 && CheckDataType( strDataBuf,nCurIndexOfBuf+6) != DATATYPE_YINGWEN)	
						 {
							 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;	//打开读号码开关
							 return nCurIndexOfBuf; 
						 }
					 }
				 }

			 }

			 //若数字串是2位，且以0结尾，且后跟汉字“后”，则处理成号码
			 //例子： 80后的金正恩面对的是一些80后的政治老人。是典型的“80后”
			 if(    nSumOfShuZi == 2												//刚好2位数字
				 && GetWord(strDataBuf,nCurIndexOfBuf+2)== (emInt16)0xa3b0			//以0结尾
				 && GetWord(strDataBuf,nCurIndexOfBuf+4)== (emInt16)0xbaf3			//后跟汉字“后”
				 && (    GetWord(strDataBuf,nCurIndexOfBuf)==(emInt16)0xa3b0		//第1位数字=0，或>=6
				      || GetWord(strDataBuf,nCurIndexOfBuf)>=(emInt16)0xa3b6))	
			 {
				 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
				 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
				 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
				 return nCurIndexOfBuf; 
				 
			 }

			 //域名或邮箱的数字读成号码  例如：ah.12530.com。hhh@263.net。  hyl 2012-04-09
			 if(    nWord  == (emInt16) 0xa3ae 		//数字后跟符号“.”
				 && CheckDataType(strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2+2) == DATATYPE_YINGWEN )	//符号“.”后跟英文字母
			 {
				 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
				 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
				 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
				 return nCurIndexOfBuf; 
			 }


			 //剩下的全部按数值处理
			 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
			 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
			 return nCurIndexOfBuf;//end of Read ShuZi To 数值

		}// end of (nSumOfShuZi <= 16)
   }//end of (g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == ....)
} //end of ProcessShuZi 

//****************************************************************************************************
//  功能：检查数字串是否为合格的日期类型，如果是则播放日期，如果不是则返回值为零
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：处理了数字串中读成日期的数字个数的字符数量 nSumOfReadDate
//****************************************************************************************************
emInt16  emCall  CheckDateFormat( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadDate = 0 ;
	emInt16 nDataType = 0 ;
	emInt16 nSumOfShuZi = 0;
	emInt16 nSumOfTemp = 0;
	emInt16 nWord = 0 ;
	emInt16 nWordTemp = 0 ;
	emInt16 nTemp = 0 ;
	emInt16 nSum = 0 ;
    emInt16 nFuHaoTemp = 0 ;

	LOG_StackAddr(__FUNCTION__);

	nDataType = CheckDataType( pCurStr, nIndex );
	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );

	if( nDataType != DATATYPE_SHUZI )
	{
		return nSumOfReadDate;
	}

	if ( nSumOfShuZi == 4 ) //是否为四位的年
	{
		nWordTemp = ChangeShuZiToInt( pCurStr, nCurIndex );
			if ( nWordTemp >= 1801 && nWordTemp <= 2099)
			{
				nIndex = nCurIndex + nSumOfShuZi * 2;
				nWord = GetWord( pCurStr,nIndex );
				if ( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae) )
				{  
					if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
					{
						nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
						nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
						if ( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 13 )  //符合类型YYYYMM
						{   
							nFuHaoTemp = nWord;//保存时间符号，用以判断是否与后面的时间符号一致
							nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
							ReadDigitHaoMa( pCurStr, nCurIndex );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc4ea );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：年
							ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
							ReadDigitShuZhi( pCurStr, nIndex + 2 );
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4c2 );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：月
							ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
							nIndex = nIndex + 2 + nSumOfTemp * 2;
							nSumOfReadDate = nIndex - nCurIndex ;
							nWord = GetWord( pCurStr, nIndex );
							if ( nWord == nFuHaoTemp ) //时间符号是否与前面的一致 保持格式为XXXX-XX-XX 而格式为XXXX-XX/XX、XXXX-XX.XX、XXXX-XX\XX是不合格的时间格式
							{
								nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
								nWordTemp = ChangeShuZiToInt( pCurStr, nIndex + 2 );
								if( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 32 )
								{
									ReadDigitShuZhi( pCurStr, nIndex + 2 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc8d5 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//播放：”日“ 
									ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
									nIndex = nIndex + 2 + nSumOfTemp * 2;
									nSumOfReadDate = nIndex - nCurIndex ;
									nWord = GetWord( pCurStr, nIndex );
									if( nWord  == (emInt16)0xc8d5 )//YYYYMMDD后跟“日”，不再重复发音
									{
										nSumOfReadDate = nIndex + 2 - nCurIndex  ;
									}
								}
							}
							else if( nWord == (emInt16)0xd4c2 )//YYYYMM后跟“月”，不再重复发音
							{
								nSumOfReadDate = nIndex + 2 - nCurIndex ;
							}
						}
					}

				}
			}
	}

	 if ( nSumOfShuZi == 2 ) //是否为两位的年
	{   
		nIndex = nCurIndex + nSumOfShuZi * 2;
			nWord = GetWord( pCurStr,nIndex );
			if ( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae) )
			{
				if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
				{
					nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
					nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
					if ( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 13 )  //符合类型YYMM
					{   
                        nFuHaoTemp = nWord;//保存时间符号，用以判断是否与后面的时间符号一致
	            		nIndex = nIndex + 2 + nSumOfTemp * 2;
						nWord = GetWord( pCurStr, nIndex );
						if ( nWord == nFuHaoTemp ) //时间符号是否与前面的一致 保持格式为XX-XX-XX 而格式为XX-XX/XX、XX-XX.XX、XX-XX\XX是不合格的时间格式
						{
							nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
							nWordTemp = ChangeShuZiToInt( pCurStr, nIndex + 2 );
							if( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 32 ) //符合类型YYMMDD
							{   
								nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								ReadDigitHaoMa( pCurStr, nCurIndex );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc4ea );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//播放：年
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								ReadDigitShuZhi( pCurStr, nCurIndex + nSumOfShuZi * 2 + 2  );
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4c2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：月
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								ReadDigitShuZhi( pCurStr, nIndex + 2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc8d5 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//播放：”日“ 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								nIndex = nIndex + 2 + nSumOfTemp * 2;
								nSumOfReadDate = nIndex - nCurIndex ;
								nWord = GetWord( pCurStr, nIndex );
								if( nWord  == (emInt16)0xc8d5 )//YYMMDD后跟“日”，不再重复发音
								{
									nSumOfReadDate = nIndex + 2 - nCurIndex  ;
								}
							}
						}
					}
				}

			}
	}

	
	if ( (nSumOfShuZi == 1) || (nSumOfShuZi == 2) )//月在前
	{
		nWordTemp = ChangeShuZiToInt( pCurStr, nCurIndex );
		if ( nWordTemp > 0 && nWordTemp < 13 )
		{
			nIndex = nCurIndex + nSumOfShuZi * 2;
			nWord = GetWord( pCurStr,nIndex );
			if ( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae) )
			{   
				nFuHaoTemp = nWord;//保存时间符号，用以判断是否与后面的时间符号一致
				if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
				{
					nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
					nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
					if ( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 32 )  
					{
						nIndex = nIndex + 2 + nSumOfTemp * 2;
						nWord = GetWord( pCurStr, nIndex );
						if ( nWord == nFuHaoTemp ) //时间符号是否与前面的一致 保持格式为XX-XX-XXXX 而格式为XX-XX/XXXX、XX-XX.XXXX、XX-XX\XXXX是不合格的时间格式
						{
							nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
							nWordTemp = ChangeShuZiToInt( pCurStr, nIndex + 2 );
							if( nSumOfTemp == 4  && nWordTemp > 1800 && nWordTemp < 2100 )
							{
								nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								ReadDigitHaoMa( pCurStr, nIndex + 2 );
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp;
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc4ea );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：“年” 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								ReadDigitShuZhi( pCurStr, nCurIndex );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4c2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：“月” 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								ReadDigitShuZhi( pCurStr, nCurIndex + nSumOfShuZi * 2 + 2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc8d5 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：“日” ”
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								nIndex = nIndex + 2 + nSumOfTemp * 2;
								nSumOfReadDate = nIndex - nCurIndex ;
								if( GetWord( pCurStr,nIndex ) == (emInt16)0xc8d5 )//MMDDYYYY后跟年，不再重复发音
								{
									nSumOfReadDate = nIndex + 2 - nCurIndex  ;
								}
							}
						}
					}
				}
			}
		}
	}
	return nSumOfReadDate;
}

//****************************************************************************************************
//  功能：检查数字串是否为合格的时间类型，如果是则播放时间，如果不是则返回值为零
//	输入参数：当前字符串emPByte pCurStr，相对位移量emInt16  nCurIndex
//	返回：处理了数字串中读成时间的数字及数字相关的个数 nSumOfReadTime
//****************************************************************************************************
emInt16  emCall  CheckTimeFormat( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadTime = 0 ;
	emInt16 nDataType = 0 ;
	emInt16 nSumOfShuZi = 0;
	emInt16 nSumOfTemp = 0;
	emInt16 nWord = 0 ;
	emInt16 nWordTemp = 0;
	emInt16 nTemp = 0 ;
	emInt16 nSum = 0 ;
	emInt16 nNeedAddLen = 0;
	emInt16 nShi;

	LOG_StackAddr(__FUNCTION__);

	nDataType = CheckDataType( pCurStr, nIndex );
	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );

	if( nDataType != DATATYPE_SHUZI )
	{
		return nSumOfReadTime;
	}

	if ( nSumOfShuZi == 2 || nSumOfShuZi == 1 ) 
	{
		nWordTemp = ChangeShuZiToInt( pCurStr, nIndex );
		nShi = nWordTemp;
		if ( nWordTemp < 25 )
		{
			nIndex += nSumOfShuZi * 2;
			nWord = GetWord( pCurStr,nIndex );
			if ( (nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3))
			{
				if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
				{
					nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
					nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
					if ( (nSumOfTemp == 2) && (nWordTemp < 60) )  
					{
						nIndex +=  2 + nSumOfTemp * 2;
                        nWord = GetWord( pCurStr,nIndex );
						if ( ((nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3)) &&((GetStrSum(pCurStr,nIndex + 2,DATATYPE_SHUZI)!=2) || ((GetStrSum(pCurStr,nIndex + 2,DATATYPE_SHUZI)==2)&&(ChangeShuZiToInt(pCurStr,nIndex + 2)>59)))) //不是合格时间格式
						{ 
                          return nSumOfReadTime;
						}  
							//是合格的时间格式

							//“pm”格式 
							if( GetWord( pCurStr,nIndex ) == (emInt16)0xa3f0 &&  GetWord( pCurStr,nIndex+2 ) == (emInt16)0xa3ed  )  
							{
								if(  nShi<=1)			//中午
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd6d0 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xcee7 );
									nNeedAddLen = 4;
								}
								if( nShi>1 && nShi<=6)	//下午
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xcfc2 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xcee7 );
									nNeedAddLen = 4;
								}
								if( nShi>6 )			//晚上
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xcded );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xc9cf );
									nNeedAddLen = 4;
								}

								
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_t);			//播放：中午，下午，晚上
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  5 );			//清空数字缓存中存储的内容
								
							}
							//“am”格式 
							if( GetWord( pCurStr,nIndex ) == (emInt16)0xa3e1 &&  GetWord( pCurStr,nIndex+2 ) == (emInt16)0xa3ed  )	
							{
								if(  nShi<=4)			//凌晨
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e8 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xb3bf );
									nNeedAddLen = 4;
								}
								if( nShi>4 && nShi<=7)	//早上
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4e7 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xc9cf );
									nNeedAddLen = 4;
								}
								if( nShi>7 )			//上午
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc9cf );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xcee7 );
									nNeedAddLen = 4;
								}
								
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_t);			//播放：凌晨，早上，上午
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  5 );			//清空数字缓存中存储的内容
								
							}

							g_hTTS->m_bIsErToLiang = emTrue;
							ReadDigitShuZhi( pCurStr, nCurIndex );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xb5e3 );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);				//播放：“点” 
							ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容

							if( nWordTemp != 0 )			//不是“3:00:14”这种格式
							{
								if ( (nSumOfTemp == 2)&&(nWordTemp < 10) )
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放：“零” （分）
									ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
								}
								ReadDigitShuZhi( pCurStr, nIndex - nSumOfTemp * 2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xb7d6 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：“分” 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
							}
							else							//不是“3:00:14”这种格式； 读成“零分”
							{
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xb7d6 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放：“零分” 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  5 );			//清空数字缓存中存储的内容
							}

							//是否为A:BB+分的格式，若是则后面的分不发音
							if ( nWord == (emInt16)0xb7d6 ) //分
							{
							 nIndex += 2;
							}
                             nSumOfReadTime = nIndex - nCurIndex + nNeedAddLen;

        					if ( (nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3) )
							{
								nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
								nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
								if ((nSumOfTemp == 2)  && (nWordTemp < 60) ) //符合类型A:BB:CC or AA:BB:CC
								{
									if ( (nSumOfTemp == 2)&&(nWordTemp < 10) && (nWordTemp != 0))
									{
										PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
										PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
										SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//播放：“零”（秒） 
										ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容
									}
									
									ReadDigitShuZhi( pCurStr, nIndex + 2 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc3eb );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//播放：“秒” 
									ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//清空数字缓存中存储的内容

									nIndex +=  2 + nSumOfTemp * 2;
									//是否满足a:BB:CC秒的格式，若满足则秒不发音
									if ((GetWord(pCurStr,nIndex))== (emInt16)0xc3eb)
									{
									 nIndex += 2;
									}
									nSumOfReadTime = nIndex - nCurIndex ;
								}
							}
						}
				}
			}
		}
	}
	nSumOfReadTime /= 2;
	return nSumOfReadTime;
}


