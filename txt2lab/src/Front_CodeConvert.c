#include "emPCH.h"
//#include <iconv.h>          //linux编码转换
#include <memory.h>
#include <string.h>
#include "Front_CodeConvert.h"



//****************************************************************************************************
//编码转换主程序： 根据编码类型，调用编码转换子函数
//
//	返回：				
//			错误码：emTTSErrID    （=emTTS_ERR_OK：表示本次编码转换成功；  =emTTS_ERR_END_OF_INPUT：表示本次编码转换结束，退出TTS）
//
//****************************************************************************************************
emTTSErrID emCall CodeConvertToGBK( )  //进行编码转换，全部转成GBK编码
{

#define		ASCII_JuHao			0x002e
#define		ASCII_MaoHao		0x003a
#define		GBK_MaoHao			0xa3ba
#define		BIG5_MaoHao			0xa147 
#define		UNICODE_MaoHao		0xff1a 


	emInt16	pEndPuncCodeSet[4][MAX_END_PUNC][2] = 
	{	
		//标点：ASCII码-编码：pEndPuncCodeSet[0]	
	  {	{ASCII_JuHao,	PUNC_MUTE_JuHao},			//句号		：ASCII编码	：单字节
		{0x003f,		PUNC_MUTE_WenHao},			//问号		：ASCII编码	：单字节
		{0x0021,		PUNC_MUTE_GanTanHao},		//感叹号	：ASCII编码	：单字节
		{0x000a,		PUNC_MUTE_HuanHangHao},		//换行号	：ASCII编码	：单字节
		{0x003b,		PUNC_MUTE_FenHao},			//分号		：ASCII编码	：单字节
		{0x002c,		PUNC_MUTE_DouHao},			//逗号		：ASCII编码	：单字节
		{ASCII_MaoHao,  PUNC_MUTE_MaoHao},			//冒号		：ASCII编码	：单字节
		{0x0000,		0},							//结束
		{0x0000,		0},							//结束
		{0x0000,		0}							//结束	
	  },
	
		//标点：GBK-编码：pEndPuncCodeSet[1]
	  {	{0xa1a3, PUNC_MUTE_JuHao },			//句号		：GBK编码	：双字节
		{0xa3bf, PUNC_MUTE_WenHao },		//问号		：GBK编码	：双字节
		{0xa3a1, PUNC_MUTE_GanTanHao },		//感叹号	：GBK编码	：双字节
		{0xa1ad, PUNC_MUTE_ShengLueHao },	//省略号	：GBK编码	：双字节
		{0xa3bb, PUNC_MUTE_FenHao },		//分号		：GBK编码	：双字节
		{0xa3ac, PUNC_MUTE_DouHao },		//逗号		：GBK编码	：双字节
		{GBK_MaoHao, PUNC_MUTE_MaoHao },	//冒号		：GBK编码	：双字节
		{0xa1a2, PUNC_MUTE_DunHao },		//顿号		：GBK编码	：双字节
		{0xa38a, PUNC_MUTE_HuanHangHao},	//换行号	：GBK编码	：双字节		 
		{0x0000, 0}							//结束
	  },
	
		//标点：BIG5-编码：pEndPuncCodeSet[2]	
	  {	{0xa143, PUNC_MUTE_JuHao },			//句号		：BIG5编码	：双字节
		{0xa148, PUNC_MUTE_WenHao },		//问号		：BIG5编码	：双字节
		{0xa149, PUNC_MUTE_GanTanHao },		//感叹号	：BIG5编码	：双字节
		{0xa14b, PUNC_MUTE_ShengLueHao },	//省略号	：BIG5编码	：双字节
		{0xa146, PUNC_MUTE_FenHao },		//分号		：BIG5编码	：双字节
		{0xa141, PUNC_MUTE_DouHao },		//逗号		：BIG5编码	：双字节
		{BIG5_MaoHao, PUNC_MUTE_MaoHao },	//冒号		：BIG5编码	：双字节
		{0xa142, PUNC_MUTE_DunHao },		//顿号		：BIG5编码	：双字节
		{0x0000, 0},						//结束
		{0x0000, 0}							//结束
	  },
	
		//标点：UNICODE-编码：pEndPuncCodeSet[3]
	  {	{0x3002, PUNC_MUTE_JuHao },			//句号		：UNICODE编码	：双字节
		{0xff1f, PUNC_MUTE_WenHao },		//问号		：UNICODE编码	：双字节
		{0xff01, PUNC_MUTE_GanTanHao },		//感叹号	：UNICODE编码	：双字节
		{0x2026, PUNC_MUTE_ShengLueHao },	//省略号	：UNICODE编码	：双字节
		{0xff1b, PUNC_MUTE_FenHao },		//分号		：UNICODE编码	：双字节
		{0xff0c, PUNC_MUTE_DouHao },		//逗号		：UNICODE编码	：双字节
		{UNICODE_MaoHao, PUNC_MUTE_MaoHao },//冒号		：UNICODE编码	：双字节
		{0x3001, PUNC_MUTE_DunHao },		//顿号		：UNICODE编码	：双字节
		{0x0000, 0},						//结束
		{0x0000, 0}							//结束
	  }
	};

	LOG_StackAddr(__FUNCTION__);
	
	g_hTTS->m_bConvertWWW = emFalse;

	switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
	{
	case emTTS_CODEPAGE_GBK :
		g_hTTS->m_nErrorID = ConvertGbkOrGB2312ToGbk(pEndPuncCodeSet[0][0] );
		break;
	case emTTS_CODEPAGE_GB2312 :
		g_hTTS->m_nErrorID = ConvertGbkOrGB2312ToGbk(pEndPuncCodeSet[0][0]);
		break;
	case emTTS_CODEPAGE_BIG5 :
		g_hTTS->m_nErrorID = ConvertBig5ToGbk(pEndPuncCodeSet[0][0]);
		break;		
	case emTTS_CODEPAGE_UTF16BE :
	case emTTS_CODEPAGE_UTF16LE :
		g_hTTS->m_nErrorID = ConvertUnicodeToGbk(pEndPuncCodeSet[0][0]);
		break;
	default: 
		g_hTTS->m_nErrorID = emTTS_ERR_END_OF_INPUT;
		break;
	}

	//转换结束，抛弃编码转换后数据区中的满足条件的空格
	if( *(g_hTTS->m_pDataAfterConvert) != 0xff)		//不是结束符（即本次转换有数据）    必须判断，否则有些情况死机  例如：[b1]、。,
	{
		if( (g_hTTS->m_nErrorID == emTTS_ERR_OK) )
		{
			QuKongGe( );	
		}
	}

	return g_hTTS->m_nErrorID;
}




//返回值 :  1:代表本单字节或双字节是分句标点； 0:代表不是
emBool emCall IsEndPuncCode(emInt16* pParaEndPuncCodeSet ,	//分段标点的二维数组（第1维：属于那种编码的标点，第2维：每个标点的编码值）
							emInt nByteCount,				//1:标点是单字节； 2：标点是双字节
							emByte nSumOfConverted)			//代表已转换了多少字节
{
	emInt8	i, nPuncCodeIndex = 1;
	emInt16 nCurMuteMs;  
	emUInt16 nMaoHaoCode, nWord = 0,nCurPunc;

	emByte cH_DataOfConvert;
	emByte cL_DataOfConvert;

	emByte cH_DataOfBasic;                     
	emByte cL_DataOfBasic;    

	emByte cH_Next;                     
	emByte cL_Next;  
	emBool bRseult;

	LOG_StackAddr(__FUNCTION__);

	if( g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE) 	//若是Unicode小头方式
	{
		cH_DataOfBasic =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 1);                     
		cL_DataOfBasic =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData);  

		cH_Next        =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 3);       
		cL_Next        =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 2);       
	}
	else
	{
		cH_DataOfBasic =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData);                     
		cL_DataOfBasic =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 1);          

		cH_Next        =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 2);   
		cL_Next        =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 3);       
	}


	nWord = ( (emInt16)cH_DataOfBasic << 8 ) + ( (emInt16)cL_DataOfBasic & (emInt16)0x00ff );

	switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
	{
		case emTTS_CODEPAGE_GBK :			nPuncCodeIndex = 1;			break;
		case emTTS_CODEPAGE_GB2312 :		nPuncCodeIndex = 1;			break;
		case emTTS_CODEPAGE_BIG5 :			nPuncCodeIndex = 2;			break;		
		case emTTS_CODEPAGE_UTF16BE :	nPuncCodeIndex = 3;			break;
		case emTTS_CODEPAGE_UTF16LE :	nPuncCodeIndex = 3;			break;
		default:							nPuncCodeIndex = 1;			break;
	}
	if( nByteCount == 1 )		//ASCII码：标点结束符
	{
		if(    g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16BE
			|| g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE)				
		{
			if( cH_DataOfBasic == 0x00)
				cH_DataOfBasic = cL_DataOfBasic;
			else									//不是正常的Unicode中ASCII区域的字符
			{
				g_hTTS->m_NextMuteMs = 0;
				return FALSE;
			}
		}

		i = 0;
		while(TRUE)
		{
			nCurPunc   = pParaEndPuncCodeSet[2*i];			//以一维数组的形式访问三维数组
			nCurMuteMs = pParaEndPuncCodeSet[2*i + 1];		//以一维数组的形式访问三维数组

			i++;

			if( nCurPunc == 0x0000 )		//结束了
				break;

			//仅需比较单字节
			if(  cH_DataOfBasic == (emByte)nCurPunc  && nSumOfConverted >= 2)				//第0维存储：ASCII标点符
			{
				//若当前ASCTT字节为：单冒号；且之前1个字为数字；则不能当做句子结束符。（例如：比分为3:2。）
				if( cH_DataOfBasic==((emByte)ASCII_MaoHao) )		
				{
					cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 2) ;
					cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 1) ;
					//if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //为数字
					//	break;
					if( cH_DataOfConvert==0xa3 && 
						(cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) || 
						( cL_DataOfBasic>= 0x30 && cL_DataOfBasic<= 0x39 ))//为数字
						break;		//sqb 20140804 modify
					if(  cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xa0 &&  nSumOfConverted >= 4)  //为空格
					{
						cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 4) ;
						cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 3) ;
						if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //为数字
							break;
					}
					if ( cH_DataOfConvert==0xa3 && cL_DataOfBasic == 0x20 &&    //sqb 20140804 modify
						cH_Next >= 0x30 && cH_Next <= 0x39 )
						break;
				}

				//判断当前正在转换的ASCII码的“.”是否应该分句，bResult为emTrue 分句，emFalse 不分句
				if( cH_DataOfBasic== ((emByte)ASCII_JuHao)  )		//当前ASCTT字节为：点
				{					
					bRseult = IsDotSplitSen(cL_DataOfBasic, cH_Next, cL_Next, nSumOfConverted);
					if( bRseult == emFalse)
						break;
				}


				g_hTTS->m_NextMuteMs = nCurMuteMs;
				return TRUE;
			}	
		}
		g_hTTS->m_NextMuteMs = 0;
		return FALSE;
	}

	if( nByteCount == 2 )		//双字节：标点结束符
	{
		i = 0;
		switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
		{
			case emTTS_CODEPAGE_GBK :			nMaoHaoCode = GBK_MaoHao;			break;
			case emTTS_CODEPAGE_GB2312 :		nMaoHaoCode = GBK_MaoHao;			break;
			case emTTS_CODEPAGE_BIG5 :			nMaoHaoCode = BIG5_MaoHao;			break;		
			case emTTS_CODEPAGE_UTF16BE :	nMaoHaoCode = UNICODE_MaoHao;		break;
			case emTTS_CODEPAGE_UTF16LE :	nMaoHaoCode = UNICODE_MaoHao;		break;
			default:							nMaoHaoCode = GBK_MaoHao;			break;
		}
		while(TRUE)
		{
			nCurPunc   = pParaEndPuncCodeSet[nPuncCodeIndex*MAX_END_PUNC*2 + i*2];		//以一维数组的形式访问三维数组  
			nCurMuteMs = pParaEndPuncCodeSet[nPuncCodeIndex*MAX_END_PUNC*2 + i*2 + 1];	//以一维数组的形式访问三维数组  		

			if(  nWord == nCurPunc )
			{
				//若当前双字节为：冒号；且之前1个字为数字；则不能当做句子结束符。（例如：时间9:30分）
				if( nWord == nMaoHaoCode)
				{
					if( nSumOfConverted >= 2 )
					{
						cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 2) ;
						cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 1) ;
						if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //为数字
						{
							break;
						}

						if(  cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xa0 &&  nSumOfConverted >= 4)  //为空格
						{
							cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 4) ;
							cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 3) ;
							if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //为数字
							{
								break;
							}
						}
					}
				}

				g_hTTS->m_NextMuteMs = nCurMuteMs;
				return TRUE;
			}
			i++;
			if( nCurPunc == 0x0000 )
			{
				break;
			}
			
		}
		g_hTTS->m_NextMuteMs = 0;
		return FALSE;
	}

	return TRUE;
}

/*********************************************************/
//代码转换:从一种编码转为另一种编码
#if 0
int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
    iconv_close(cd);
    return 0;
}
//UNICODE码转为GB2312码
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312码转为UNICODE码
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
    return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

#endif
//****************************************************************************************************
//编码转换子函数：  GBK或者GB2312 --> GBK
//
//内容：	从原始文本数据g_hTTS->m_pc_DataOfBasic中读取部分文本数据换到g_hTTS->m_pDataAfterConvert
//			满足以下三个条件之一，就结束本次编码转换，同时g_hTTS->m_nCurIndexOfBasicData前移记录已经转换到何处。
//				1. 遇到以五个标点其中的一个（五个标点为：，；。！？  包含双字节标点和单字节标点），返回emTTS_ERR_OK
//				2. g_hTTS->m_pDataAfterConvert已经填充满（仅留1字节填充结束符），最大为CORE_CONVERT_BUF_MAX_LEN），返回emTTS_ERR_OK
//				3. g_hTTS->m_nCurIndexOfBasicData指针已经指向了g_hTTS->m_pc_DataOfBasic的尾部，即全部转换结束    ，返回emTTS_ERR_OK
//
//	引用到的全局变量：	g_hTTS->m_pc_DataOfBasic			：从参数中获取的原始文本数据的指针 //						
//						g_hTTS->m_nCurIndexOfBasicData	：当前处理到的基本文本数据的索引位置
//						g_hTTS->m_pDataAfterConvert		：编码转换后的文本数据的指针
//
//	返回：				错误码emTTSErrID
//							=emTTS_ERR_OK		：表示本次编码转换成功；  
//							=emTTS_ERR_END_OF_INPUT		：表示本次编码转换结束，退出TTS，即本次循环转换中的最后一次（遇到基本文本数据的尾）
//
//  改变的全局变量：	g_hTTS->m_nCurIndexOfBasicData
//
//****************************************************************************************************
emTTSErrID emCall ConvertGbkOrGB2312ToGbk(emInt16* pParaEndPuncCodeSet )
{

	emPByte pDataOfBasic = (emPByte)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData; //本次转换开始时原始文本数据的指针
	emPByte pDataAfterConvert = g_hTTS->m_pDataAfterConvert;                          //编码转换后的文本数据的指针
	emByte i;                                                                //去除数据转换区连续空格字符时采用的循环变量
	emByte nSumOfConverted = 0;                                              //已存储到编码转换后文本数据区的字节数
	emBool bDouhao = 0 ;                                                      //标志变量：flag =1，不对单字节逗号做编码转换；flag =0，对单字节逗号进行编码转换后退出本次编码转换 
	emTTSErrID  nTypeOfemTTS_ERR;                                            //返回值
    //size_t rc = 0;
    //emByte input[255]={0} ;
    //char out[255]={0};

#if EM_ENG_PURE_LOG	
	FILE* pfEnExit;
#endif


	LOG_StackAddr(__FUNCTION__);
/*
    if(emTTS_CODEPAGE_UTF8==1){
        //memcpy(input,pDataOfBasic,g_hTTS->m_nNeedSynSize/2*3);
        //u2g(input,strlen(input),out,255);        //sqb 2017/6/5
        u2g(pDataOfBasic,strlen(pDataOfBasic),out,255);
        pDataOfBasic = out;
    }
*/

	//清空编码转换后的文本数据区
	for( i = 0; i < (emByte)CORE_CONVERT_BUF_MAX_LEN; i++ )
	{
		g_hTTS->m_pDataAfterConvert[i]  = 0;
	}

	//原始文本无数据，本次编码转换失败，退出本函数，并返回emTTS_ERR_END_OF_INPUT
	if( g_hTTS->m_nCurIndexOfBasicData >= g_hTTS->m_nNeedSynSize-1)
		return emTTS_ERR_END_OF_INPUT;

	//原始文本有数据，开始编码转换
	do 
	{
		emByte cH_DataOfBasic =  *pDataOfBasic;                           //先读取的原始文本数据区的一个字节
		emByte cL_DataOfBasic =  *( pDataOfBasic + 1 );                   //后读取的原始文本数据区的一个字节

        //memcpy(&temp,pDataOfBasic,2);
        //temp=temp<<4;
        //emByte cH_DataOfBasic =  *(pDataOfBasic+1);
        //emByte cL_DataOfBasic =  *pDataOfBasic;

		nSumOfConverted = pDataAfterConvert - g_hTTS->m_pDataAfterConvert ;


		if (( cH_DataOfBasic >= 0x20 && cH_DataOfBasic <= 0x7f) || ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F ))
		{
			//ASCII码或者控制字符
			if ( IsEndPuncCode( pParaEndPuncCodeSet, 1,nSumOfConverted ) == TRUE )
			{
				//遇到单字节结束标点
				if( cH_DataOfBasic == 0x2c )					
				{   //遇到单字节逗号

					bDouhao = QuDouHao( pDataOfBasic, nSumOfConverted, bDouhao);
					if( bDouhao )
					{   //满足条件，跳过该逗号
						pDataOfBasic++;
						g_hTTS->m_nCurIndexOfBasicData++;
						continue;	
					}	
				}
				//遇到单字节分号、问号、叹号、句号和不需要跳过的逗号加0xa380转换为相应的双字节标点，结束本次编码转换，返回emTTS_ERR_OK
				*pDataAfterConvert = 0xa3;
				*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;

				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				nTypeOfemTTS_ERR = emTTS_ERR_OK;
				pDataOfBasic++;
				break;		
			}
			else
			{
				if ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F )
				{
					//原始文本为控制字符，转换为双字节空格
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;

					if ( cH_DataOfBasic == 0x0a )
					{//遇到单字节换行符时，转换为双字节空格，结束本次编码转换，返回emTTS_ERR_OK  作为结束符
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData++;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic++;
						break;		
					}

				}
				else
				{
					//原始文本为ASCII码，加0xA380转成两字节（但美元符号0x24是加0xA1C3）
					if( cH_DataOfBasic == 0x24 )
					{
						*pDataAfterConvert = 0xa1;
						*(pDataAfterConvert + 1) = 0xc3 + cH_DataOfBasic;
					}
					else 
					{
						*pDataAfterConvert = 0xa3;
						*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;						
					}		
				}
                
				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) ) //若不是数字   2012-03-29
				{
					bDouhao = 0;
				}
				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				pDataOfBasic++;		

				//20140218增加  防英文长句在单词内被切断（在允许最长的长度前几个个字内只要有空格就提前结束）
				if( cH_DataOfBasic == 0x20 && (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) > (CORE_CONVERT_BUF_MAX_LEN - 20) )
				{
#if EM_ENG_PURE_LOG 
					pfEnExit = fopen("log/日志_内存_异常.log","a");
					fprintf(pfEnExit,"【编码转换时超--空格处】：超过分句的最大长度，在空格处拆分，循环合成\n");
					fclose(pfEnExit);
#endif
					break;
				}

				continue;  
			}
		}	
		else 
		{
			//if( cH_DataOfBasic == 0xa3 && (cL_DataOfBasic == 0xac || cL_DataOfBasic == 0xbb || cL_DataOfBasic == 0xbf || cL_DataOfBasic == 0xa1) 
			if ( IsEndPuncCode( pParaEndPuncCodeSet, 2 ,nSumOfConverted ) == TRUE )
			{

				//遇到双字节结束标点(分号、叹号、问号)，将双字节标点存储到转换数据区，结束本次编码转换，返回emTTS_ERR_OK
				*pDataAfterConvert = cH_DataOfBasic;
				*(pDataAfterConvert + 1) = cL_DataOfBasic;

				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData += 2;
				nTypeOfemTTS_ERR = emTTS_ERR_OK;
				pDataOfBasic += 2;
				break;		
			}
			else 
			{
				if( (cH_DataOfBasic >= 0xa1 && cH_DataOfBasic <= 0xa9 && cL_DataOfBasic >= 0x40 && cL_DataOfBasic <= 0xfe) //符号区范围拓展了
					|| ( ((cH_DataOfBasic >= 0x81 && cH_DataOfBasic <= 0xa0) || (cH_DataOfBasic >= 0xaa && cH_DataOfBasic <= 0xfe)) && cL_DataOfBasic >= 0x40 && cL_DataOfBasic <= 0xfe) ) //汉字区
				{					
					*pDataAfterConvert = cH_DataOfBasic;
					*(pDataAfterConvert + 1) = cL_DataOfBasic;
				}
				else
				{
					//不可识别的数据，已读入的两个字节一起转换为双字节的空格
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
				}

				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //若不是数字  
				{
					bDouhao = 0;
				}
				g_hTTS->m_nCurIndexOfBasicData += 2;
				pDataAfterConvert += 2;
				pDataOfBasic += 2;
				continue;
			}
		}		

	}while(   (g_hTTS->m_nCurIndexOfBasicData < g_hTTS->m_nNeedSynSize)
		   && (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) < (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16) );


	if( (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) > (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16)  )
	{
		;
#if EM_ENG_PURE_LOG 
		pfEnExit = fopen("log/日志_内存_异常.log","a");
		fprintf(pfEnExit,"【编码转换时超--最大处】：超过分句的最大长度，在最大处拆分，循环合成\n");
		fclose(pfEnExit);
#endif
	}

	//在转换数据区尾部填充结尾字符
	*pDataAfterConvert = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );		//故意填成0xff，为了以后好判断
	*(pDataAfterConvert + 1) = (emByte)( (emInt16)END_WORD_OF_BUFFER );

	//设置编码转换结果的类型	
	//本次编码转换成功
	nTypeOfemTTS_ERR = emTTS_ERR_OK;




	//编码转换成功或者结束，退出该函数，并返回emTTS_ERR_OK（编码成功）或emTTS_ERR_END_OF_INPUT（编码结束）
	return nTypeOfemTTS_ERR;
}
//****************************************************************************************************
//编码转换子函数：  BIG5 --> GBK
//
//其它说明请参照函数ConvertGbkOrGB2312ToGbk的说明
//*****************************************************************************************************
emTTSErrID  emCall ConvertBig5ToGbk( emInt16* pParaEndPuncCodeSet )
{
	emPByte pDataOfBasic = (emPByte)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData; //本次转换开始时原始文本数据的指针
	emPByte pDataAfterConvert = g_hTTS->m_pDataAfterConvert;                          //编码转换后的文本数据的指针	
	emByte  cSumOfInvalidCode = 0;                                           //记录转换数据区连续空格字符的个数
	emByte i;                                                            //去除数据转换区连续空格字符时采用的循环变量
	emByte nSumOfConverted = 0;                                              //已存储到编码转换后文本数据区的字节数
	emBool bDouhao = 0;                                                      //标志变量：flag =1，不对单字节逗号做编码转换；flag =0，对单字节逗号进行编码转换后退出本次编码转换 
	emByte nSumOfShuZi = 0;                                                  //原始文本数据区中，逗号后紧跟的数字位数
	emTTSErrID  nTypeOfemTTS_ERR;                                            //函数的返回值
	emInt16 nWord = 0;                                                       //用于存储从输入文本数据读取的两个字节
	emInt16 nOffset = 0;                                                     //输入我那本数据对应的GBK编码在g_Res.offset_CodeBig5ToGbk中的行偏移量

	LOG_StackAddr(__FUNCTION__);

	//清空编码转换后的文本数据区
	for( i = 0; i < (emByte)CORE_CONVERT_BUF_MAX_LEN; i++ )
	{
		g_hTTS->m_pDataAfterConvert[i]  = 0;
	}

	//原始文本无数据，本次编码转换失败，退出本函数，并返回emTTS_ERR_END_OF_INPUT
	if( g_hTTS->m_nCurIndexOfBasicData >= g_hTTS->m_nNeedSynSize)
		return emTTS_ERR_END_OF_INPUT;

	//原始文本有数据，进行编码转换处理
	do 
	{
		emByte cH_DataOfBasic =  *pDataOfBasic;                           //先读取的原始文本数据区的一个字节
		emByte cL_DataOfBasic =  *( pDataOfBasic + 1 );                   //后读取的原始文本数据区的一个字节
		nSumOfConverted = pDataAfterConvert - g_hTTS->m_pDataAfterConvert ;



		//输入文本数据需要转换
		if (( cH_DataOfBasic >= 0x20 && cH_DataOfBasic <= 0x7f) || ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F ))
		{
			//输入文本数据为单字节ASCII码(0x20～0x7f)或者控制字符(0x00～0x1f)
			if ( IsEndPuncCode( pParaEndPuncCodeSet, 1 ,nSumOfConverted ) == TRUE )
			{				
				//遇到单字节结束标点
				if( cH_DataOfBasic == 0x2c )					
				{   //遇到单字节逗号

					bDouhao = QuDouHao( pDataOfBasic, nSumOfConverted, bDouhao);
					if( bDouhao )
					{   //满足条件，跳过该逗号
						pDataOfBasic++;
						g_hTTS->m_nCurIndexOfBasicData++;
						continue;	
					}	
				}
				//遇到单字节分号、问号、叹号、句号和不需要跳过的逗号，加0xa380转换为相应的双字节标点，结束本次编码转换，返回emTTS_ERR_OK
				*pDataAfterConvert = 0xa3;
				*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;

				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				nTypeOfemTTS_ERR = emTTS_ERR_OK;
				pDataOfBasic++;
				break;		
			}
			else
			{
				if ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F )
				{
					//输入文本数据为控制字符,转换为双字节空格0xa3a0
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					if ( cH_DataOfBasic == 0x0a )
					{//遇到单字节换行符时，转换为双字节空格，结束本次编码转换，返回emTTS_ERR_OK  作为结束符
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData++;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic++;
						break;		
					}
				}
				else
				{
					//输入文本数据为ASCII码，加0xA380转成两字节（但美元符号0x24是加0xA1C3）
					if( cH_DataOfBasic == 0x24 )
					{
						*pDataAfterConvert = 0xa1;
						*(pDataAfterConvert + 1) = 0xc3 + cH_DataOfBasic;
					}
					else 
					{
						*pDataAfterConvert = 0xa3;
						*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;						
					}		
				}

				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )   //若不是数字
				{
					bDouhao = 0;
				}
				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				pDataOfBasic++;				
				continue;  
			}
		}	
		else 
		{   //输入文本数据为双字节符号或汉字
			if( cH_DataOfBasic >= 0xa1 && cH_DataOfBasic <= 0xf9 )
			{   //输入文本数据高字节在BIG5的有效编码范围内
				nWord = ( (emInt16)cH_DataOfBasic << 8 ) + ( (emInt16)cL_DataOfBasic & (emInt16)0x00ff );
				if ( IsEndPuncCode( pParaEndPuncCodeSet, 2,nSumOfConverted  ) == TRUE )
				{
					//输入文本数据为双字节结束标点，将双字节标点存储到转换数据区，结束本次编码转换，返回emTTS_ERR_OK
					nOffset = ( cH_DataOfBasic - (emByte) 0xa1 ) * 191 + cL_DataOfBasic - (emByte) 0x40;

					if( g_Res.offset_CodeBig5ToGbk == 0)
					{
						//若没有《Big5转GBK表》，则填0
						*(pDataAfterConvert) = 0;
						*(pDataAfterConvert+1) = 0;
					}
					else
					{
						fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_CodeBig5ToGbk + nOffset * 2, 0);						
						fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
						fFrontRead(pDataAfterConvert,1,1,g_hTTS->fResFrontMain);
					}

					pDataAfterConvert += 2;
					g_hTTS->m_nCurIndexOfBasicData += 2;
					nTypeOfemTTS_ERR = emTTS_ERR_OK;
					pDataOfBasic += 2;
					break;
				}
				else 
				{   //输入文本数据是双字节符号或汉字，且不是结束标点
					if( cL_DataOfBasic >= 0x40 && cL_DataOfBasic <= 0xfe )
					{   //输入文本数据低字节在BIG5的有效编码范围内
						nOffset = ( cH_DataOfBasic - (emByte) 0xa1 ) * 191 + cL_DataOfBasic - (emByte) 0x40;

						if( g_Res.offset_CodeBig5ToGbk == 0)
						{
							//若没有《Big5转GBK表》，则填0
							*(pDataAfterConvert) = 0;
							*(pDataAfterConvert+1) = 0;
						}
						else
						{
							fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_CodeBig5ToGbk + nOffset * 2, 0);						
							fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
							fFrontRead(pDataAfterConvert,1,1,g_hTTS->fResFrontMain);	
						}
						
						if( *pDataAfterConvert == 0x00 &&  *(pDataAfterConvert + 1) == 0x00 )
						{
							g_hTTS->m_nCurIndexOfBasicData += 2;							
							pDataOfBasic += 2;
							continue;
						}
					}
					else
					{
						//不可识别的数据，已读入的两个字节一起转换为双字节的空格
						*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
						*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					}

					if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) ) //若不是数字   2012-03-29
					{
						bDouhao = 0;
					}
					g_hTTS->m_nCurIndexOfBasicData += 2;
					pDataAfterConvert += 2;
					pDataOfBasic += 2;
					continue;
				}
			}
			else
			{
				//不可识别的数据，已读入的两个字节一起转换为双字节的空格
				*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
				*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;

				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //若不是数字   2012-03-29
				{
					bDouhao = 0;
				}

				g_hTTS->m_nCurIndexOfBasicData += 2;
				pDataAfterConvert += 2;
				pDataOfBasic += 2;
				continue;
			}				
		}

	}while(    (g_hTTS->m_nCurIndexOfBasicData < g_hTTS->m_nNeedSynSize)
		   &&  (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) < (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16) );

	//在转换数据区尾部填充结尾字符
	*pDataAfterConvert = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
	*(pDataAfterConvert + 1) = (emByte)( (emInt16)END_WORD_OF_BUFFER );


	//本次编码转换成功
	nTypeOfemTTS_ERR = emTTS_ERR_OK;



	//编码转换成功或者结束，退出该函数，并返回emTTS_ERR_OK（编码成功）或emTTS_ERR_END_OF_INPUT（编码结束）
	return nTypeOfemTTS_ERR;
}

//****************************************************************************************************
//编码转换：  UNICODE --> GBK
//
//内容：	从原始文本数据g_hTTS->m_pc_DataOfBasic中读取部分文本数据换到g_hTTS->m_pDataAfterConvert
//			满足以下三个条件之一，就结束本次编码转换，同时g_hTTS->m_nCurIndexOfBasicData前移记录已经转换到何处。
//				1. 遇到以五个标点其中的一个（五个标点为：，；。！？  包含双字节标点和单字节标点），返回emTTS_ERR_OK
//				2. g_hTTS->m_pDataAfterConvert已经填充满（仅留1字节填充结束符），最大为CORE_CONVERT_BUF_MAX_LEN），返回emTTS_ERR_OK
//				3. g_hTTS->m_nCurIndexOfBasicData指针已经指向了g_hTTS->m_pc_DataOfBasic的尾部，即全部转换结束    ，返回emTTS_ERR_OK
//
//	引用到的全局变量：	g_hTTS->m_pc_DataOfBasic			：从参数中获取的原始文本数据的指针 //						
//						g_hTTS->m_nCurIndexOfBasicData	：当前处理到的基本文本数据的索引位置
//						g_hTTS->m_pDataAfterConvert		：编码转换后的文本数据的指针
//
//	返回：				错误码emTTSErrID
//							=emTTS_ERR_OK		：表示本次编码转换成功；  
//							=emTTS_ERR_END_OF_INPUT		：表示本次编码转换结束，退出TTS，即本次循环转换中的最后一次（遇到基本文本数据的尾）
//
//  改变的全局变量：	g_hTTS->m_nCurIndexOfBasicData
//
//****************************************************************************************************
emTTSErrID emCall  ConvertUnicodeToGbk(emInt16* pParaEndPuncCodeSet )
{
	emPByte pDataOfBasic = (emPUInt8)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData;//本次转换开始时原始文本数据的指针
	emPByte pDataAfterConvert = g_hTTS->m_pDataAfterConvert;                          //编码转换后的文本数据的指针
	emUInt8  cSumOfInvalidCode = 0;                                          //记录转换数据区连续空格字符的个数
	emUInt8 i;                                                           //去除数据转换区连续空格字符时采用的循环变量
	emByte nSumOfConverted = 0;                                              //已存储到编码转换后文本数据区的字节数
	emBool bDouhao = 0;                                                      //标志变量：flag =1，不对单字节逗号做编码转换；flag =0，对单字节逗号进行编码转换后退出本次编码转换 
	emTTSErrID  nTypeOfemTTS_ERR;                                            //返回值

	LOG_StackAddr(__FUNCTION__);

	for( i = 0; i < (emUInt8)CORE_CONVERT_BUF_MAX_LEN; i++ )
	{
		g_hTTS->m_pDataAfterConvert[i]  = 0;
	}

	//原始文本无数据，本次编码转换失败，退出本函数，并返回emTTS_ERR_END_OF_INPUT
	if( g_hTTS->m_nCurIndexOfBasicData >= g_hTTS->m_nNeedSynSize)
		return emTTS_ERR_END_OF_INPUT;

	//原始文本有数据，开始编码转换
	do 
	{
		emByte cH_DataOfBasic;
		emByte cL_DataOfBasic;
		if( g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE) 	//若是Unicode小头方式
		{
			cL_DataOfBasic =  *pDataOfBasic;                       
			cH_DataOfBasic =  *( pDataOfBasic + 1 );  
		}
		else
		{
			cH_DataOfBasic =  *pDataOfBasic;                       
			cL_DataOfBasic =  *( pDataOfBasic + 1 );               
		}
		

		nSumOfConverted = pDataAfterConvert - g_hTTS->m_pDataAfterConvert ;


		if( cH_DataOfBasic == 0 && cL_DataOfBasic == 0 )		//原Unicode编码为0x0000
		{
			//转成GBK编码0xa3a0(空格)
			*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
			*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;

			pDataAfterConvert += 2;
			g_hTTS->m_nCurIndexOfBasicData += 2;
			nTypeOfemTTS_ERR = emTTS_ERR_OK;
			pDataOfBasic += 2;
			continue;
		}



		if(  IsEndPuncCode( pParaEndPuncCodeSet, 1 ,nSumOfConverted ) == TRUE ||  IsEndPuncCode( pParaEndPuncCodeSet, 2,nSumOfConverted  ) == TRUE ) 
		{
			//五个标点，
			emUInt16 nWordOfTemp = 0 ;
			emUInt16 nQiShiHangIndex = 0 ;
			emUInt16 nHangIndex = 0 ;


			nWordOfTemp = ((emUInt16)cH_DataOfBasic<<8) + ((emUInt16)cL_DataOfBasic); 			
			
			if( nWordOfTemp == (emUInt)0x002c )
			{   //遇到半角逗号（0x002c），检查是否满足跳过该逗号的条件：若满足，则跳过该逗号；否则，对改逗号进行编码转换后退出编码转换
				bDouhao = QuDouHao( pDataOfBasic, nSumOfConverted, bDouhao);
				if( bDouhao )
				{   //满足条件，跳过该逗号
					g_hTTS->m_nCurIndexOfBasicData += 2;
					pDataOfBasic += 2;
					continue;	
				}	
			}

			//符号区域的行数偏移量 = 0xX – 0x00
            nQiShiHangIndex =  cH_DataOfBasic - (emByte)0x00;
            fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nQiShiHangIndex)*2, 0);

			fFrontRead(&nHangIndex,2,1,g_hTTS->fResFrontMain);
			
			//整个符号所在的行数偏移量 =  该区域的起始行号 + 0xY – 0x00
			
			nHangIndex =  nHangIndex + cL_DataOfBasic - (emByte)0x00; ;//行数偏移量
			
			//查询符号转GBK表
			fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nHangIndex-1)*2, 0);
			fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
			fFrontRead(pDataAfterConvert,1,1,g_hTTS->fResFrontMain);

			pDataAfterConvert += 2;
			g_hTTS->m_nCurIndexOfBasicData += 2;
			nTypeOfemTTS_ERR = emTTS_ERR_OK;
			pDataOfBasic += 2;
			break;

		}
		else 
		{
              if ( (cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x3f) || (cH_DataOfBasic >= 0xfe && cH_DataOfBasic <= 0xff) )//符号
			  {

				//查询符号转GBK表，
				emUInt16 nWordOfTemp = 0 ;
				emUInt16 nQiShiHangIndex = 0 ;
				emUInt16 nHangIndex = 0 ;


				//符号区域的行数偏移量 = 0xX – 0x00
                nQiShiHangIndex =  cH_DataOfBasic - (emByte)0x00;
                fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nQiShiHangIndex)*2, 0);
				fFrontRead(&nHangIndex,2,1,g_hTTS->fResFrontMain);

				if ( nHangIndex == 0 ) //若为0，则属于不可识别的Unicode码
				{//与下一个字节一起转成GBK编码0xa3a0(空格)
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					if (  cH_DataOfBasic == 0x00 && cL_DataOfBasic == 0x0a )
					{//为换行符时 当作结束符处理 
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData += 2;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic += 2;
						break;
					}
				}
				else
				{
					//整个符号所在的行数偏移量 =  该区域的起始行号 + 0xY – 0x00

					nHangIndex =  nHangIndex + cL_DataOfBasic - (emByte)0x00; ;//行数偏移量

					//查询符号转GBK表
					fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nHangIndex-1)*2, 0);
					fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
					fFrontRead(pDataAfterConvert ,1,1,g_hTTS->fResFrontMain);
				}
				
				if( *pDataAfterConvert == 0x00 &&  *(pDataAfterConvert + 1) == 0x00 )
				{
					if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) ) //若不是数字   2012-03-29
					{
						bDouhao = 0;
					}

					g_hTTS->m_nCurIndexOfBasicData += 2;			
					pDataOfBasic += 2;
					continue;
				}
			}
			else 
			{
				if( cH_DataOfBasic >= 0x4e && cH_DataOfBasic <= 0x9f )//汉字区
				{
					//查询汉字转GBK表
					emUInt16 nWordOfTemp = 0 ;
					emUInt16 nHangIndex = 0 ;

					nHangIndex = ((emUInt16)cH_DataOfBasic<<8) + ((emUInt16)cL_DataOfBasic); 
					nHangIndex = nHangIndex - (emUInt16)0x4e00 + 1;	//行数偏移量

					fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeHanziUnicodeToGbk + (nHangIndex-1)*2, 0);
					fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
					fFrontRead(pDataAfterConvert ,1,1,g_hTTS->fResFrontMain);
					
					if( *pDataAfterConvert == 0x00 &&  *(pDataAfterConvert + 1) == 0x00 )
					{   
						if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //若不是数字   2012-03-29
						{
							bDouhao = 0;
						}
						g_hTTS->m_nCurIndexOfBasicData += 2;			
						pDataOfBasic += 2;
						continue;
					}
				}

				else//不识别
				{
					//与下一个字节一起转成GBK编码0xa3a0(空格)
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					if (  cH_DataOfBasic == 0x00 && cL_DataOfBasic == 0x0a )
					{//为换行符时 当作结束符处理 
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData += 2;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic += 2;
						break;
					}
				}
			}

			if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //若不是数字   2012-03-29
			{
				bDouhao = 0;
			}

			g_hTTS->m_nCurIndexOfBasicData += 2;
			pDataAfterConvert += 2;
			pDataOfBasic += 2;
			continue;
		}
	}while(   (g_hTTS->m_nCurIndexOfBasicData < g_hTTS->m_nNeedSynSize)
		   && (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) < (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16));
	
	//在转换数据区尾部填充结尾字符
	*pDataAfterConvert = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
	*(pDataAfterConvert + 1) = (emByte)( (emInt16)END_WORD_OF_BUFFER );


	//本次编码转换成功
	nTypeOfemTTS_ERR = emTTS_ERR_OK;



	//编码转换成功或者结束，退出该函数，并返回emTTS_ERR_OK（编码成功）或emTTS_ERR_END_OF_INPUT（编码结束）
	return nTypeOfemTTS_ERR;
}

//****************************************************************************************************
// 半角逗号处理：    考虑：千位分隔符
// 内容：   当原始文本数据指针pDataOfBasic指向的数据为半角逗号时，需对逗号前后的数据进行判断。
//         满足以下条件时，本函数返回值为1，在编码转换函数中不对该逗号进行转换，直接处理逗号后的下一字符：
//         1、该逗号为编码转换处理的第一个字符
//         2、flag = 1，该逗号前面1个符号为数字，且逗号后为3位数字
//         3、flag = 0，该逗号前面1个符号为数字，逗号前面第4个符号不是数字也不是"-"，逗号后面为3位数字
//         其它情况下，本函数返回值为0.在编码转换函数中把该逗号作为结束符号处理
//    
//
//	引用到的全局变量： g_hTTS->m_pDataAfterConvert     编码转换后的文本数据的指针
//  输入参数：         pDataOfBasic           原始数据指针，它当前指向逗号所在地址
//                     nSumOfConverted        编码转换后的数据区中已经存储的编码转换后字符数
//                     flag                   标志变量，用于记录前一个逗号是否被跳过
//
//	返回值：  该逗号不满足被跳过的条件时，函数返回0；否则，返回1
//****************************************************************************************************
emBool  emCall QuDouHao( emPByte pDataOfBasic, emByte nSumOfConverted, emBool flag)
{
	emByte nSumOfShuZi = 0;                                           //（原始文本数据区指针）pDataOfBasic中，逗号后紧跟的数字位数
	emByte i;                                                         //用于循环

	LOG_StackAddr(__FUNCTION__);

	//若逗号前为1~3位数字，并且逗号后紧跟3位数字，则将该逗号去掉，继续转换；否则将逗号转换后，结束本次编码转换
	switch(flag)
	{
	case 0://本次编码转换第一次处理逗号，需要检查逗号前是否为1~3位连续数字串
		//若编码转换处理的第一个字符就是逗号，退出本函数，并返回值1
		//若编码转换后的文本区已存储的字符数大于等于4，需检查编码转换文本区指针所在位置前的第4个字符是否为数字：
		//若为数字，则不满足取逗号的条件，退出本函数并返回0；若不是数字，则需作进一步判断

		if(  (nSumOfConverted >= 8) && (CheckDataType( g_hTTS->m_pDataAfterConvert, nSumOfConverted - 8) == (emInt16)DATATYPE_SHUZI
			||  GetWord( g_hTTS->m_pDataAfterConvert, nSumOfConverted - 8) == (emInt16)0xa3ad  ) )		//"－"		//hhh 2014-07-09		[g2]010-82572-234,010-82572-2343
		{
			return 0;
		}	
		if( nSumOfConverted == 0 )
		{
			return 1;
		}
	case 1://第一次处理逗号且逗号前面第4位不是数字，或者前一个逗号在编码转换时被跳过，则需检查逗号前1位是否为数字
	default:
		if( (nSumOfConverted >= 2) && (CheckDataType( g_hTTS->m_pDataAfterConvert, nSumOfConverted - 2) == (emInt16)DATATYPE_SHUZI)  )
		{   //逗号前1位是数字,继续判断逗号后是否紧跟3位数字	
			switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
			{
			case emTTS_CODEPAGE_GBK :					
			case emTTS_CODEPAGE_GB2312 :
				for( i = 1; ;)
				{   //计算逗号后紧跟的数字位数
					if( *( pDataOfBasic + i ) >= 0x30 && *( pDataOfBasic + i ) <= 0x39 )
					{   //单字节数字
						nSumOfShuZi ++;
						i++;								
					}
					else
					{
						if( *( pDataOfBasic + i ) == 0xa3 && ( *( pDataOfBasic + i + 1) >= 0xb0 && *( pDataOfBasic + i + 1) <= 0xb9 ) )
						{   //双字节数字
							nSumOfShuZi ++;
							i += 2;
						}
						else 
						{
							break;
						}							
					}
				}	
				break;
			case emTTS_CODEPAGE_BIG5 :
				for( i = 1; ;)
				{   //计算逗号后紧跟的数字位数
					if( *( pDataOfBasic + i ) >= 0x30 && *( pDataOfBasic + i ) <= 0x39 )
					{   //单字节数字
						nSumOfShuZi ++;
						i++;								
					}
					else
					{
						if( *( pDataOfBasic + i ) == 0xa2 && ( *( pDataOfBasic + i + 1) >= 0xaf && *( pDataOfBasic + i + 1) <= 0xb8 ) )
						{   //双字节数字
							nSumOfShuZi ++;
							i += 2;
						}
						else 
						{
							break;
						}							
					}
				}	
				break;
			case emTTS_CODEPAGE_UTF16BE :
				for( i = 2; ;i += 2)
				{
					if( *( pDataOfBasic + i ) == 0x00 && ( *( pDataOfBasic + i + 1) >= 0x30 && *( pDataOfBasic + i + 1) <= 0x39 ) )
					{   //双字节数字
						nSumOfShuZi ++;						
					}
					else 
					{
						if( *( pDataOfBasic + i ) == 0xff && ( *( pDataOfBasic + i + 1) >= 0x10 && *( pDataOfBasic + i + 1) <= 0x19 ) )
						{   //双字节数字
							nSumOfShuZi ++;

						}
						else 
						{
							break;
						}	
					}
				}
				break;	
			case emTTS_CODEPAGE_UTF16LE :
				for( i = 2; ;i += 2)
				{
					if( *( pDataOfBasic + i + 1) == 0x00 && ( *( pDataOfBasic + i) >= 0x30 && *( pDataOfBasic + i) <= 0x39 ) )
					{   //双字节数字
						nSumOfShuZi ++;						
					}
					else 
					{
						if( *( pDataOfBasic + i + 1) == 0xff && ( *( pDataOfBasic + i) >= 0x10 && *( pDataOfBasic + i) <= 0x19 ) )
						{   //双字节数字
							nSumOfShuZi ++;
						}
						else 
						{
							break;
						}	
					}
				}
				break;	
			}			
			if( nSumOfShuZi == 3)
			{   //逗号后紧跟3位数字，退出本函数并返回1
				return 1;
			}
			else 
			{   //逗号后紧跟连续数字个数不为3，退出本函数并返回0
				return 0;
			}
		}
		else 
		{  //逗号前1位不是数字，退出本函数并返回0
			return 0;
		}

	}

}

//****************************************************************************************************
// 去空格处理：  
// 内容：  按下列顺序，抛弃编码转换区中满足下列条件的空格：
//         1. 首先，对于连续空格串，只保留一个空格字符；
//         2. 然后，若空格前后的数据类型不同类（但前是数字后是字母除外），则抛弃空格。
//	引用到的全局变量： g_hTTS->m_pDataAfterConvert     编码转换后的文本数据的指针
//  改变的全局变量：   g_hTTS->m_pDataAfterConvert
//	返回值：  无返回值
//****************************************************************************************************

void emCall	QuKongGe( )
{
	emByte  cSumOfInvalidCode = 0;                                           //记录转换数据区连续空格字符的个数
	emByte i,j,k;                                                            //去除数据转换区连续空格字符时采用的循环变量
	emInt16	nPreType,nNextType;
	emBool	bTypeQu = emFalse;

	LOG_StackAddr(__FUNCTION__);

	//对于连续空格串，只保留一个空格
	for( i = 0; g_hTTS->m_pDataAfterConvert[i] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[i + 1] != (emByte)END_WORD_OF_BUFFER; i += 2 )
	{

		//记录连续空格数目  //GBK空格  ：  0xa1a1 或 0xa3a0 
		if( ((g_hTTS->m_pDataAfterConvert[i] == (emByte)(INVALID_CODE >> 8)) && (g_hTTS->m_pDataAfterConvert[i + 1] == (emByte)INVALID_CODE) )
			|| ((g_hTTS->m_pDataAfterConvert[i] == (emByte)(0xa1a1 >> 8)) && (g_hTTS->m_pDataAfterConvert[i + 1] == (emByte)0xa1a1) ) )
		{
			cSumOfInvalidCode += 0x01;
			continue;
		}
		//出现连续空格后，保留第一个空格，然后将连续空格后的数据连接到保留的空格的后面，并将空格数目置零
		if( cSumOfInvalidCode >= (emInt8)2 || cSumOfInvalidCode*2 == i )
		{
			if( cSumOfInvalidCode*2 == i )		//句首的空格：全部去掉
				k = cSumOfInvalidCode * 2;			
			else								//非句首的空格：只保留1个
				k = (cSumOfInvalidCode - 1) * 2;

			for( j = i - k; g_hTTS->m_pDataAfterConvert[j + k] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[j + k + 1] != (emByte)END_WORD_OF_BUFFER; j += 2 )
			{
				g_hTTS->m_pDataAfterConvert[j] = g_hTTS->m_pDataAfterConvert[j + k];
				g_hTTS->m_pDataAfterConvert[j + 1] = g_hTTS->m_pDataAfterConvert[j + k + 1];	
			}
			g_hTTS->m_pDataAfterConvert[j ] = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
			g_hTTS->m_pDataAfterConvert[j + 1] = (emByte)( (emInt16)END_WORD_OF_BUFFER );
			i = i - k;
			cSumOfInvalidCode = 0;
		}
		else
		{
			cSumOfInvalidCode = 0;
		}
	}
	//若连续空格和结束字符之间无其他数据，则在连续空格的第二个空格处填充结尾字符
	if( cSumOfInvalidCode >= (emInt8)2 )
	{
		k = (cSumOfInvalidCode - 1) * 2;
		g_hTTS->m_pDataAfterConvert[i - k ] = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
		g_hTTS->m_pDataAfterConvert[i - k + 1] = (emByte)( (emInt16)END_WORD_OF_BUFFER );
	} 

	//若空格前后的数据类型不同类（但：前是数字后是字母除外，前是字母后是数字除外）或前后都是汉字，则将空格去掉  //GBK空格  ：  0xa1a1 或 0xa3a0 
	//例如：V60  V66 T720。50cm 50kg 50V。
	for( i = 2; g_hTTS->m_pDataAfterConvert[i + 2] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[i + 3] != (emByte)END_WORD_OF_BUFFER; i += 2 )
	{
		bTypeQu = emFalse;
		nPreType = CheckDataType( g_hTTS->m_pDataAfterConvert, i - 2);
		nNextType = CheckDataType( g_hTTS->m_pDataAfterConvert, i + 2);
		/*
		if( nPreType != nNextType   || (nPreType==DATATYPE_HANZI && nNextType==DATATYPE_HANZI))
		{
			if(    nPreType == DATATYPE_SHUZI   && nNextType == DATATYPE_YINGWEN		//前是数字后是字母
				|| nPreType == DATATYPE_YINGWEN && nNextType == DATATYPE_SHUZI )	//前是字母后是数字
			{
				bTypeQu = emFalse;
			}
			else
			{
				bTypeQu = emTrue;
			}

		}
		*/
		

		// fhy 130105 只去除汉字左右的空格
		if( nPreType==DATATYPE_HANZI || nNextType==DATATYPE_HANZI)
			bTypeQu = emTrue;
		else
			bTypeQu = emFalse;

		if( (((g_hTTS->m_pDataAfterConvert[i] == (emByte)(INVALID_CODE >> 8)) && (g_hTTS->m_pDataAfterConvert[i + 1] == (emByte)INVALID_CODE))
			|| ((g_hTTS->m_pDataAfterConvert[i] == (emByte)(0xa1a1 >> 8)) && (g_hTTS->m_pDataAfterConvert[i + 1] == (emByte)0xa1a1)))
			&&  bTypeQu == emTrue )
		{
			for( j = i; g_hTTS->m_pDataAfterConvert[j + 2] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[j + 3] != (emByte)END_WORD_OF_BUFFER; j += 2 )
			{
				g_hTTS->m_pDataAfterConvert[j] = g_hTTS->m_pDataAfterConvert[j + 2];
				g_hTTS->m_pDataAfterConvert[j + 1] = g_hTTS->m_pDataAfterConvert[j + 3];	
			}
			g_hTTS->m_pDataAfterConvert[j ] = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
			g_hTTS->m_pDataAfterConvert[j + 1] = (emByte)( (emInt16)END_WORD_OF_BUFFER );
			i = i - 2;		
		}
	}
}



//繁简转换： 对g_hTTS->m_pDataAfterConvert里的数据进行
void emCall		FanToJian()		
{
	emInt32 i,nRowNum;
	emUInt16 nJianCode;

	LOG_StackAddr(__FUNCTION__);

	i = 0;
	while( *(g_hTTS->m_pDataAfterConvert + 2 * i) != (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 ) )
	{
		nRowNum = (*(g_hTTS->m_pDataAfterConvert + 2 * i) - 0x81) * 191 +  *(g_hTTS->m_pDataAfterConvert + 2 * i + 1) - 0x40; //计算出偏移行数

		fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_FanToJian + nRowNum * 2, 0);  //定位
		fFrontRead(&nJianCode, 2, 1, g_hTTS->fResFrontMain);  //读取
		if( nJianCode != 0)
		{
			*(g_hTTS->m_pDataAfterConvert + 2 * i)  = nJianCode>>8;
			*(g_hTTS->m_pDataAfterConvert + 2 * i +1 )  = nJianCode&0xff;
		}
		i++;
	}
}




emUInt8 emCall	CheckInfo()
{

	emInt16 nTotalLen,i;
	emUInt nReturn = 1;

	#define INFO_VERSION_LEN		40			//预备播放版本号的字节

	if( emStrLenA(g_hTTS->m_pDataAfterConvert) == (COMPANY_INFO_NEED_COMPARE_LEN+2) )
	{
		emMemSet(g_hTTS->m_Info ,0, MAX_COMPANY_INFO_LEN);

		fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_CompanyInfo ,0);
		fFrontRead(&nTotalLen, 2, 1, g_hTTS->fResFrontMain);
		if( nTotalLen >= (MAX_COMPANY_INFO_LEN-INFO_VERSION_LEN))
			nTotalLen = MAX_COMPANY_INFO_LEN - INFO_VERSION_LEN;		
		fFrontRead(g_hTTS->m_Info, 1, nTotalLen, g_hTTS->fResFrontMain);

		for( i=0; i<MAX_COMPANY_INFO_LEN; i++)
		{
			if(  g_hTTS->m_Info[i] != 0)
				g_hTTS->m_Info[i] ^= 0x55;
			else
				break;
		}
		//与固定文本匹配上
		if( !emMemCmp(g_hTTS->m_pDataAfterConvert, g_hTTS->m_Info, COMPANY_INFO_NEED_COMPARE_LEN) )
		{
			//加上：播放版本号（自动获取版本号）
			emMemCpy(g_hTTS->m_Info+nTotalLen,"版本号：",8);
			emMemCpy(g_hTTS->m_Info+nTotalLen+8,EM_VERSION,sizeof(EM_VERSION));
			nTotalLen += (sizeof(EM_VERSION) + 8);

			g_hTTS->m_pc_DataOfBasic = g_hTTS->m_Info;
			nReturn = 2;
			g_hTTS->m_nNeedSynSize = nTotalLen ;
		}
	}

	return nReturn;

}


void emCall	 GetChaBoText()
{
	emInt16 i,nCount;
	emUInt8 nByte, nMod;
	emInt16 nOffset,nLen;

	emMemSet(g_hTTS->m_Info ,0, MAX_COMPANY_INFO_LEN);	

	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_InsertPlayInfo,0);
	fFrontRead(&nCount, sizeof(emInt16), 1, g_hTTS->fResFrontMain);

	//通过对第2字节求余数，得到要插播的是第几条插播信息
	nByte = *(g_hTTS->m_pDataAfterConvert+1);
	nMod = nByte - nByte/nCount*nCount;		//取余

	fFrontSeek(g_hTTS->fResFrontMain, nMod * 4 ,1);
	fFrontRead(&nOffset, sizeof(emInt16), 1, g_hTTS->fResFrontMain);
	fFrontRead(&nLen, sizeof(emInt16), 1, g_hTTS->fResFrontMain);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_InsertPlayInfo+nOffset ,0);
	fFrontRead(g_hTTS->m_Info, nLen, 1, g_hTTS->fResFrontMain);

	for( i=0; i<nLen; i++)
		g_hTTS->m_Info[i] ^= 0x55;

	g_hTTS->m_Info[i] =   0xa1;
	g_hTTS->m_Info[i+1] = 0xa3;
	g_hTTS->m_Info[i+2] = 0xff;
	g_hTTS->m_Info[i+3] = 0xff;
	g_hTTS->m_Info[i+4] = 0x00;
	g_hTTS->m_Info[i+5] = 0x00;

	emMemCpy(g_hTTS->m_pDataAfterConvert, g_hTTS->m_Info,i+6);

}

//判断当前正在转换的ASCII码的“.”是否应该分句
//返回值： emFalse: 不分句；  emTrue: 分句；
emBool emCall IsDotSplitSen(emByte cL_DataOfBasic,
							emByte cH_Next, 
							emByte cL_Next, 
							emByte nSumOfConverted)
{
	emPByte cP1_DataOfConvert;	// 转换后数据
	emBool bResult = emTrue;
	emBool bUcode = emFalse;
	emInt16 i,j;
#define LEN 26
	emInt16 AbbrDot[LEN][4] = {
		{0xa3cd, 0xa3f2},	// Mr
		{0xa3cd, 0xa3f3},	// Ms
		{0xa3c4, 0xa3f2},	// Dr
		{0xa3ce, 0xa3ef},	// No
		{0xa3ca, 0xa3f2},	// Jr
		{0xa3d3, 0xa3f2},   //Sr	//gaogao 2014.7.5 modify
		{0xa3d3, 0xa3f4},	// St
		{0xa3c7, 0xa3e5, 0xa3ee},	// Gen
		{0xa3cd, 0xa3f2, 0xa3f3},	// Mrs
		{0xa3ed, 0xa3e7, 0xa3f2},	// mgr
		{0xa3c1, 0xa3f0, 0xa3f0},	// App
		{0xa3ca, 0xa3e1, 0xa3ee},	// Jan
		{0xa3c6, 0xa3e5, 0xa3e2},	// Feb
		{0xa3cd, 0xa3e1, 0xa3f2},	// Mar
		{0xa3c1, 0xa3f0, 0xa3f2},	// Apr
		{0xa3ca, 0xa3f5, 0xa3ee},	// Jun
		{0xa3ca, 0xa3f5, 0xa3ec},	// Jul
		{0xa3c1, 0xa3f5, 0xa3e7},	// Aug
		{0xa3d3, 0xa3e5, 0xa3f0},	// Sep
		{0xa3cf, 0xa3e3, 0xa3f4},	// Oct
		{0xa3ce, 0xaeef, 0xa3f6},	// Nov
		{0xa3c4, 0xa3e5, 0xa3e3},	// Dec
		{0xa3c1, 0xa3e3, 0xa3f4, 0xa3f6},	// Actv
		{0xa3c3, 0xa3ef, 0xa3ee, 0xa3f4},	// Cont
		{0xa3d0, 0xa3f2, 0xa3ef, 0xa3e6},	//Prof
	};


	LOG_StackAddr(__FUNCTION__);
	

	cP1_DataOfConvert = g_hTTS->m_pDataAfterConvert + nSumOfConverted - 2;
	
	if(g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16BE
		|| g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE)
		bUcode = emTrue;

	// “.”后面为空格或Tab键
	if((bUcode && cH_Next == 0x00 && (cL_Next == SPACE_CODE || cL_Next == TAB_CODE))
		|| (!bUcode && (cL_DataOfBasic == SPACE_CODE || cL_DataOfBasic == TAB_CODE)))
	{
		emInt16 t;

		//前为以下特殊缩写的，不分句。  （Mr.  Dr.  Gen.  Mrs.  Prof. ）  例如：Mr. Andrew    Dr. Mathew    Gen. Philips    Mrs. Hellen    Prof. Adams
		for(i=LEN-1,j=strlen(AbbrDot[0])-1; i>=0; i--,j=strlen(AbbrDot[0])-1)
		{
			while(!AbbrDot[i][j]) j--;
			if(j >= (short)(nSumOfConverted/2)) continue;
			t=0;
			while(j >= 0)
			{
				if(GetWord(cP1_DataOfConvert,t) == AbbrDot[i][j])
				{
					if(j==0)
					{
						if((nSumOfConverted/2 + t-1 >= 0) && (CheckDataType(cP1_DataOfConvert,t-2) == (emInt16)DATATYPE_YINGWEN))
							break;
						else
							return emFalse;
					}
					t -= 2;
					j--;
				}else
					break;
			}
		}

		if((CheckDataType(cP1_DataOfConvert,0) == (emInt16)DATATYPE_YINGWEN)	//gaogao 2014.7.2 modify 
			&& (CheckDataType(cP1_DataOfConvert,-2) != (emInt16)DATATYPE_YINGWEN) 
			&& (CheckDataType(cP1_DataOfConvert,-4) == (emInt16)DATATYPE_YINGWEN)
			&& (CheckDataType(cP1_DataOfConvert,-6) != (emInt16)DATATYPE_YINGWEN))
		{
			return emFalse;		//不分句： 前跟长度为1的字母串且这个缩写的长度至少为2个字母时，不分句。 例如：W.C. C.V. M.A. i.e. e.g. S.O.S.  1520 B.C. A.C.  3:00 a.m.    2:00 p.m.   3:00 a.m.   2:00 p.m.	
		}
		else
		{
			//其余，分句		例如： a.  b.  c. you are there. you. are. there. I am here.  158.   Ph.D.	filename.txt	http://mail.163.com	my_name@example.com 3.5kg.  123.23.   ３.５kg.  １２３.２３.  it is 3.it is 4.
			//                         This is W.C. I come to the U.S.
			return emTrue;		
		}
	
	}
	else			
	{
		/*
		// 处理点后无空格或Tab键的情况
		// aisound： 1.若符号点不是其所在的连续字符串中的最后一个点时， 则不分句
		//			 2.若符号点为连续字符串的最后的一个点时， 且其后连的不是com或cn，则不分句且点读成dot
		//			 3.其他,分句不发音
		if((CheckDataType(cP1_DataOfConvert,0) == (emInt16)DATATYPE_YINGWEN)	// 前跟长度为1的字母串时，不分句。 例如：W.C.   C.V.   M.A.	  i.e.   e.g.	  S.O.S.  1520 B.C.    A.C. 158.    3:00 a.m.    2:00 p.m.   3:00 a.m.   2:00 p.m.	【error】： Ph.D.	filename.txt	http://mail.163.com	my_name@example.com
			&& (CheckDataType(cP1_DataOfConvert,-2) != (emInt16)DATATYPE_YINGWEN))
		{
			bResult = emFalse;
		}
		else if(CheckDataType(cP1_DataOfConvert,0) == (emInt16)DATATYPE_SHUZI)	// 前字为数字时，不分句。例如：3.5kg.  123.23.   ３.５kg.  １２３.２３.  【error】：it is 3.it is 4.
		{
			bResult = emFalse;
		}
		else if((g_hTTS->m_bConvertWWW == emTrue)								// 之前出现过www，则不分句。（注意：区别大小写）例如：WWW.sohu.com.cn。wWw.tTs168.net.cn。WwW.263.org.cn。www.SOHU.gov.cn。
				|| ((nSumOfConverted/2 >= 3) &&
					(((GetWord(cP1_DataOfConvert,0) == (emInt16)0xa3f7) ||
					(GetWord(cP1_DataOfConvert,0) == (emInt16)0xa3d7))
					&& ((GetWord(cP1_DataOfConvert,-2) == (emInt16)0xa3f7) ||
					(GetWord(cP1_DataOfConvert,-2) == (emInt16)0xa3d7))
					&& ((GetWord(cP1_DataOfConvert,-4) == (emInt16)0xa3f7) ||
					(GetWord(cP1_DataOfConvert,-4) == (emInt16)0xa3d7)))))
		{
			if((nSumOfConverted/2 > 3) && (CheckDataType(cP1_DataOfConvert,-6) == (emInt16)DATATYPE_YINGWEN))	//awww.123.cn
			{
				bResult = emTrue;
			}
			else		//www.123.cn 1www.123.cn #www.123.cn
			{
				bResult = emFalse;
 				g_hTTS->m_bConvertWWW = emTrue;
			}
		}
		else																	//其余情况下都分句。例如：ab.ab。 欢迎.光临。you.are.pic. see.you。Tom is here.you are there. it is ok.23 is ok.	
			bResult = emTrue;

			*/

		bResult = emFalse;			//后面不是空格或tab键，则一律不分句

		return bResult;
	}
}
	
