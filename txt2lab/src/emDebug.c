#include "emPCH.h"

#include "emDebug.h"

#if ARM_LINUX_SWITCH
	#include <sys/time.h>
#endif


#if DEBUG_LOG_SWITCH_TIME

	#if ARM_MTK_LOG_TIME
		    kal_uint32 tick_2, tick_3;
			kal_uint32 g_t2_time, g_t3_time;
	#else

		#if !ANKAI_TOTAL_SWITCH && !ARM_LINUX_SWITCH
			#include <windows.h>
			#if !ARM_WINCE_TIME	
				#pragma comment(lib, "winmm.lib")
			#endif
		#endif

		#if !ARM_LINUX_SWITCH
			unsigned long  g_t2_time,g_t3_time; 
		#else
			struct timeval tv;
			struct timezone tz;
			suseconds_t g_t2_time,g_t3_time;
		#endif

		FILE *g_fLogTime;
	#endif

#endif


#if ( DEBUG_LOG_SWITCH_RHYTHM_MIDDLE ||  DEBUG_LOG_SWITCH_RHYTHM_RESULT )
//****************************************************************************************************
//  功能：打印链表内容到一个文件中  add by songkai
//  参数：文件名
//****************************************************************************************************
void emCall Print_To_Rhythm(emCharA *pTitle,				//文件名
							emCharA *mode,					//写文件的方式
							emInt32  isHaveBorder,			//汉字链表中是否已标注了“PW”“PPH”边界
							emCharA *pHead)				//句前加的句头
{

	emCharA cText[4000];
	emCharA tempBuffer[50];  //临时数组
	emPInt8 p1 = NULL, p2 = NULL, retString = NULL;
	emInt32  nLen = 0;
	FILE *fp_pos = NULL;
	FILE *fp = fopen(pTitle, mode);
	

	fp_pos = fopen("..\\irf\\POS值查询表.txt", "r");


	g_nP1TextByteIndex = 0 ;

	while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0)
	{
		if( isHaveBorder == TRUE )
		{
			if( (!emMemCmp( pHead, "分词：",6) || g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PW)  &&  g_nP1TextByteIndex!=0)
			{
				emMemCpy(cText+nLen, "|", 1);
				nLen += 1;
			}
			if( g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PPH)
			{
				if( g_nP1TextByteIndex!=0)
				{
					emMemCpy(cText+nLen, "◆",2);
					nLen += 2;
				}
			}
		}

		emMemCpy(cText+nLen, &g_pText[g_nP1TextByteIndex], g_pTextInfo[g_nP1TextByteIndex/2].Len);
		nLen += (g_pTextInfo[g_nP1TextByteIndex/2].Len);

		fFrontSeek(fp_pos,0,0);
		while((retString=fgets(tempBuffer, 50, fp_pos)) != NULL)
		{
			p1 = strtok(tempBuffer, "\t");  //POS的值
			p2 = strtok(NULL, "\n");        //POS的编码
			if(p2 != NULL && g_pTextInfo[g_nP1TextByteIndex/2].Pos == atoi(p2))
			{
				
#if DEBUG_LOG_SWITCH_POS_IN_RHYTHM
				emMemCpy(cText+nLen, p1,emStrLenA(p1));
				nLen += emStrLenA(p1);
#endif
				break; //找到就跳出
			}
		}
		if(retString == NULL)  //文件查找结束仍然没有找到对应值
		{
			emMemCpy(cText+nLen, " ",1);
			nLen += 1;
		}
		if( isHaveBorder == 0)
		{
			if( emMemCmp( pHead, "原句：",6))
			{				
				;
			}
		}


	
		g_nP1TextByteIndex += g_pTextInfo[g_nP1TextByteIndex/2].Len; 
	}


	if( !emMemCmp( pHead, "分词：",6))
	{
		emMemCpy(cText+nLen, "|",1);
		nLen += 1;
	}
	emMemCpy(cText+nLen, "\0", 1);
	if( emMemCmp( pHead, "原句：",6))
	{	
		if( g_hTTS->bTextTailWrap == emTrue )
			fprintf(fp, "%s%s%s%s",pHead,  "【",cText,"。】\n");
		else
			fprintf(fp, "%s%s%s%s",pHead,  "【",cText,"。】");
	}
	else
	{
		fprintf(fp, "%s%s%s%s%s","\n",pHead,"【",cText,"】\n\n");
	}
	
	
	fclose(fp);
	fclose(fp_pos);
}
#endif



#if DEBUG_LOG_SWITCH_TIME

//****************************************************************************************************
//  功能：
//        参数： nPara： 1：输出：文本+计时   2： 输出：文本	3： 输出：整数
//						11:计时开始			 12：计时结束
//****************************************************************************************************
#if ARM_MTK_LOG_TIME

void myTimeTrace_For_Debug(emInt16 nPara,emByte *Text, emInt16 nCount)
{
	switch(nPara)
	{
		case 1:
			kal_get_time(&tick_2);									//mtk平台tracert方式打印
			g_t2_time = kal_ticks_to_milli_secs(tick_2);

			g_t3_time=g_t2_time-g_t1_time;
			g_t1_time = g_t2_time;
			if( g_t3_time< 5000)
			{
				g_total_time += g_t3_time;
			}
			
			kal_prompt_trace(MOD_IVTTS, "%s         	%d          	%d",Text,g_t3_time,g_total_time);
			break;
		case 2:
			kal_prompt_trace(MOD_IVTTS, "%s",Text);			
			break;
		case 3:
			kal_prompt_trace(MOD_IVTTS, "%d",nCount);
			break;
		case 11:
			kal_prompt_trace(MOD_IVTTS,"计时开始");
			break;
		case 12:
			kal_prompt_trace(MOD_IVTTS, "计时结束！");
			break;
		default:
			break;
	}
}

#else

void myTimeTrace_For_Debug(emInt16 nPara,emByte *Text, emInt16 nCount)
{
	switch(nPara)
	{
		case 1:

#if ANKAI_TOTAL_SWITCH
			g_t2_time = GetTickCount();			//WinCE版：纯C环境下  或 安凯平台下
#else
			#if ARM_LINUX_SWITCH
				gettimeofday(&tv,&tz);			//ARM Linux平台下
				g_t2_time =  (tv.tv_sec*1000)+(tv.tv_usec/1000);	
			#else				

				#if !ARM_WINCE_TIME	
					g_t2_time = timeGetTime();			//PC版   ：纯C环境下
				#else
					g_t2_time = GetTickCount();			//Wince ：
				#endif
			#endif
#endif

			g_t3_time=g_t2_time-g_t1_time;
			g_t1_time = g_t2_time;
			if( g_t3_time< 5000)
			{
				g_total_time += g_t3_time;
			}
			fprintf(g_fLogTime, "\n%25s：	%10d           %10d", Text, g_t3_time,g_total_time);		
			break;
		case 2:
			fprintf(g_fLogTime, "%s", Text);
			break;
		case 3:
			fprintf(g_fLogTime, "%d", nCount);
			break;
		case 11:
#if ARM_LINUX_SWITCH
			g_fLogTime=fopen("log/日志_计时.txt","a");			//计时开始
#else
			g_fLogTime=fopen("log/日志_计时.log","a");			//计时开始
#endif
			break;
		case 12:
			fclose(g_fLogTime);			//计时结束
			break;
		default:
			break;
	}
}
#endif
#endif


#if DEBUG_LOG_SWITCH_HANZI_LINK
//****************************************************************************************************
//  功能：打印汉字链表的内容   add by songkai
//        参数： nPara： 1：新建文件    2： 合成汉字缓冲区开始     3： 合成过程中每个模块的输出
//				 bIsOutputBorder:  0:不输出边界； 1：输出PW和PPH边界；  2：仅输出PPH边界
//****************************************************************************************************
void emCall Print_To_Txt(emInt16 nPara, emByte *Text,emBool bIsOutputBorder)
{

	FILE  *fp_output = NULL;
	emPInt8 p1 = NULL, p2 = NULL, retString = NULL;
	emUInt8 nPinYinCount, nCount;
	emInt16 nPinYin,i;

	emByte	t_Text[MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2];


	if( nPara != 1)
	{
		fp_output = fopen("log/日志_汉字链表.log", "a");
	}
	else
	{
		fp_output = fopen("log/日志_汉字链表.log", "w");
	}

	fprintf(fp_output, "%s%s%s\n\n", "********",Text, "************************");

	if( nPara == 3)
	{		
		if(g_pTextInfo[0].Len != 0)
		{
			g_nP1TextByteIndex = 0 ;
			fprintf(fp_output, "%s\t%s\t%s\t%s\t%s\t%s\t%s\n\n","边界" ,"内容          ", "pos值", "长度", "类型", "标点", "拼音");
			while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0 && g_nP1TextByteIndex<=MAX_HANZI_COUNT_OF_LINK*2)
			{
				nPinYinCount=0, nCount=0;

				if( g_nP1TextByteIndex!=0 && bIsOutputBorder==1)
				{
					if( g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PW)
						fprintf(fp_output, "%s\n", "ＰＷ");
				}

				if( g_nP1TextByteIndex!=0 && (bIsOutputBorder==1 || bIsOutputBorder==2))
				{
					if( g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PPH)
						fprintf(fp_output, "%s\n", "ＰＰＨ");

				}

				emMemSet(t_Text, 0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
				if( g_pTextInfo[g_nP1TextByteIndex/2].Len >6)
				{
					emMemCpy(t_Text, &g_pText[g_nP1TextByteIndex],g_pTextInfo[g_nP1TextByteIndex/2].Len);
					fprintf(fp_output, "\t%s\t", t_Text);
				}
				else
				{
					emMemCpy(t_Text, &g_pText[g_nP1TextByteIndex],g_pTextInfo[g_nP1TextByteIndex/2].Len);
					fprintf(fp_output, "\t%s      \t", t_Text);
				}
				nPinYinCount = g_pTextInfo[g_nP1TextByteIndex/2].Len / 2;  //根据汉字个数计算出有几个拼音


				for(i= 0; i < posTableLog.nLineCount; i++)
				{
					if(posTableLog.pCode[i] != NULL && g_pTextInfo[g_nP1TextByteIndex/2].Pos == posTableLog.pCode[i])
					{
						fprintf(fp_output, "%s\t", posTableLog.pPos[i]);
						break; //找到就跳出
					}
				}


				if( i == posTableLog.nLineCount)  //文件查找结束仍然没有找到对应值
				{
					fprintf(fp_output, "%s\t", "无Pos");
				}

				fprintf(fp_output, "%d\t%d\t",(g_pTextInfo[g_nP1TextByteIndex/2].Len)/2, g_pTextInfo[g_nP1TextByteIndex/2].TextType);

				emMemSet(t_Text, 0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
				emMemCpy(t_Text, g_pTextInfo[g_nP1TextByteIndex/2].nBiaoDian,2);
				fprintf(fp_output, "%s\t", t_Text);

				if( 1 )   //有拼音的话
				{			
					while(nPinYinCount)      //循环处理每个拼音
					{
						nPinYin = g_pTextPinYinCode[g_nP1TextByteIndex/2+nCount];
						if( nPinYin == 0)
						{
							fprintf(fp_output, "--\t");
						}
						else
						{

							if( (nPinYin & 0x8000) == 0x8000)  //判断pPinYinCode的最高位是否为1
							{
								nPinYin = nPinYin & 0x7FFF;			//如果最高位是1,得到拼音编码	
							}

							i = ErFenSearchPinPinLog( nPinYin );
							fprintf(fp_output, "%s ", pinyinTableLog.pPinYin[i]);
						}
						
						nCount++;
						nPinYinCount--;
					}

				}
				else  //本身就没有对拼音字段赋值
				{
					fprintf(fp_output, "%s\t", "无拼音");
				}

				fprintf(fp_output, "%s", "\n");
				g_nP1TextByteIndex += g_pTextInfo[g_nP1TextByteIndex/2].Len;
			}	
		}

		fprintf(fp_output, "\n\n");
	}
	fclose(fp_output);
}
#endif


#if DEBUG_LOG_POLY_TEST	
//****************************************************************************************************
//  功能：打印多音字测试用的拼音信息
//        参数： nPara： 1：新建文件    2： 输出信息
//        参数： Text：  输出信息中加的前缀
//****************************************************************************************************
void Print_Poly_Test(emInt16 nPara, emByte *Text)
{
	emCharA tempBuffer[50];  //临时数组
	FILE   *fp_output = NULL;
	emPInt8 p1 = NULL, p2 = NULL, retString = NULL;
	emUInt8 nPinYinCount, nCount;
	emInt16 nPinYin,i;

	if( nPara == 1)
	{
		fp_output = fopen("log/日志_多音字测试-过程-temp.txt", "w");
	}
	else
	{
		fp_output = fopen("log/日志_多音字测试-过程-temp.txt", "a");


		fprintf(fp_output, "%s\t%s\n", Text, g_pText);
		fprintf(fp_output, "%s\t", Text);
		
		if(g_pTextInfo[0].Len != 0)
		{
			g_nP1TextByteIndex = 0 ;
			while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0 && g_nP1TextByteIndex<=MAX_HANZI_COUNT_OF_LINK*2)
			{
				nPinYinCount=0, nCount=0;

				nPinYinCount = g_pTextInfo[g_nP1TextByteIndex/2].Len / 2;  //根据汉字个数计算出有几个拼音

				while(nPinYinCount)      //循环处理每个拼音
				{
					nPinYin = g_pTextPinYinCode[g_nP1TextByteIndex/2+nCount];
					if( (nPinYin & 0x8000) == 0x8000)  //判断pPinYinCode的最高位是否为1
					{
						nPinYin = nPinYin & 0x7FFF;			//如果最高位是1,得到拼音编码	
					}
	
					i = ErFenSearchPinPinLog( nPinYin );
					fprintf(fp_output, "%s ", pinyinTableLog.pPinYin[i]);

					nCount++;
					nPinYinCount--;
				}
				g_nP1TextByteIndex += g_pTextInfo[g_nP1TextByteIndex/2].Len;
			}	
		}
		fprintf(fp_output, "\n");

	}
	fclose(fp_output);
}
#endif



#if DEBUG_LOG_SWITCH_HANZI_LINK|DEBUG_LOG_POLY_TEST
//二分法查找输入的拼音在《拼音查询表》中所在的行数
emUInt16 ErFenSearchPinPinLog( emInt32  nPinYin)
{
	emUInt16 cMin,cMax,cMiddle;


	cMin = 0;
	cMax = pinyinTableLog.nLineCount -1;

	cMiddle = ( cMin + cMax) / 2;
	do
	{		
		if( nPinYin == pinyinTableLog.pCode[cMiddle] )
		{		
			return cMiddle;
		}
		if( nPinYin > pinyinTableLog.pCode[cMiddle]  )
		{   //查找的大于表中的
			cMin = cMiddle;
		}
		else 
		{   //查找的小于表中的
			cMax = cMiddle;
		}
		cMiddle = ( cMin + cMax) / 2;
	}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );

	return pinyinTableLog.nLineCount;
}
#endif