#include <iconv.h>
#include "emPCH.h"


#include "emTTS_Common.h"

#include "emCommon.h"
#include "emDecode.h"

#include "Front_CodeConvert.h"
#include "Front_ProcFuHao.h"
#include "Front_ProcShuZi.h"
#include "Front_ProcYingWen.h"
#include "Front_ProcHZToPinYin.h"
#include "Front_ProcHZWordSeg.h"
#include "Front_ToLab.h"

#include "Rear_Synth.h"

//#include "EnF_Main.h"   //sqb
#include <wchar.h>
#if ARM_LINUX_SWITCH
	#include <sys/time.h>
#endif

#if	EM_SYS_SWITCH_RHYTHM_NEW
		#include "Front_ProcHZRhythm_New.h"
#else
		#include "Front_ProcHZRhythm_Old.h"
#endif


#if DEBUG_LOG_SWITCH_TIME
	#if ARM_MTK_LOG_TIME
		kal_uint32 tick_total,tick_1;
		emUInt32 g_total_time,g_t1_time;
	#else

		#if ARM_LINUX_SWITCH
			struct timeval tv1;
			struct timezone tz1;
		#endif

		emUInt32  g_total_time,g_t1_time;
	#endif
#endif


extern emBool global_bStopTTS;


//强制拼音：共有3类 ： 1. 银行[=hang2]行长   2.银hang2行长   3. 薄丽玲 （未登录词中的多音姓）

//****************************************************************************************************
//函数功能： 《汉字缓冲区》的优先处理  ，即使没有汉字，也能播放Pcm中已有数据
//     共有以下几种情况能触发：《汉字缓冲区》的优先处理  
// 			1。 //控制标记触发（除强制拼音外）
// 		    2。 //英文（提示音，普通英文，拼音）触发，除能解释成汉字的英文外，
// 		    3。 //段尾触发，
// 		    4。 //汉字链表中的汉字超过了MAX_HANZI_COUNT_OF_LINK
//****************************************************************************************************
emTTSErrID  emCall FirstHandleHanZiBuff(emBool bIsSylTailSil)		//是否合成句尾sil
{
	emUInt32 t1,nLen;
	emInt16 i,k;
	emByte emPtr pLabRam;


	LOG_StackAddr(__FUNCTION__);

	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //防止重播放或停止要等待很长时间
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;

	if( g_hTTS->m_nCurIndexOfAllHanZiBuffer>0)
	{

		//配置《RearSynth》函数里的PlayMuteDelay一起使用
		//防止8K模型播放句尾时卡，
		//通过播放句中的静音，凑足播放设备里的1个buff的数据，将上一句遗留的句尾的一点声音播放出去)
		//例如：可以打10086问一下。即可发指定的短信参加活动。
		//例如：可以打10086问。即可发指定的短信参加活动。
		if( g_hTTS->m_CurMuteMs >= 100 )
		{
			t1 = g_hTTS->m_CurMuteMs - 100;			
			PlayMuteDelay(100);			//先播放静音	
			g_hTTS->m_CurMuteMs = t1;
		}



#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 2, "**************************************************************************************************************************",0);
#endif

#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  标准化-模块  ",0);
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"\n\n                   标准化", 0);
#endif

		if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue )			//收到退出命令
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			return g_hTTS->m_nErrorID;
		}

		//释放：PCM输出的缓冲区
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE, "PCM输出缓冲：《出口：销毁》");
#else
		emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE);
#endif		

        WordSeg();		//分词模块
        //WordSeg_third();

		//创建：PCM输出的缓冲区
#if DEBUG_LOG_SWITCH_HEAP
		g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE, "PCM输出缓冲：《入口：创建》");
#else
		g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE);
#endif
		g_hTTS->m_pPCMBufferSize = MAX_OUTPUT_PCM_SIZE;



#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  分词-模块  ",0);
#endif


        ToPinYin();		//字音转换模块
        //ToPinYin_third();


/*
        for(int i = 0; i < 45; i++)
        {
            printf("%d ", g_pTextPinYinCode[i]);
        }
        printf("\n");
*/

		if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue )			//收到退出命令
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			return g_hTTS->m_nErrorID;
		}

#if DEBUG_LOG_POLY_TEST
		Print_Poly_Test(2, "多音");
#endif


#if	EM_SYS_SWITCH_RHYTHM_NEW

		Rhythm_New();			//韵律--新方案


        /////////////////////////////////////////////////////////20180104,zoucheng,test,begin
        /*
        printf("after:\n");
        for(int i = 0; i < 17; i++)
        {
            printf("%d: %d, %d, %d, %d; %c, %c\n",
                   i,
                   g_pTextInfo[i].Pos,
                   g_pTextInfo[i].Len,
                   g_pTextInfo[i].TextType,
                   g_pTextInfo[i].BorderType,
                   g_pTextInfo[i].nBiaoDian[0],
                   g_pTextInfo[i].nBiaoDian[1]);
        }
        printf("\n");
        */

        //测试例句:中文语境下智能度最高的机器人大脑
        //设置[智能度]中智字前后的韵律
        //g_pTextInfo[4].BorderType = 3;
        //g_pTextInfo[5].BorderType = 3;
        //g_pTextInfo[7].BorderType = 1;
        /////////////////////////////////////////////////////////20180104,zoucheng,test,end


#else
		Rhythm_Old();				//韵律--老方案
#endif

		
#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  韵律预测-模块  ",1);
#endif


#if PPH_AFTER_DEAL					//PPH边界划分后处理

		if( g_hTTS->m_ControlSwitch.m_nManualRhythm != emTTS_USE_Manual_Rhythm_OPEN )		//没有打开手动韵律标注[z1]
		{
			//强制调整PPH：1.根据“成对标点强制PPH信息”    例如：他看了《红楼梦》后很开心
			//强制调整PPH：2.两字短韵律清除					例如：/我们/这种沾沾自喜的情绪/在不知不觉当中/传染给了囡囡/。(第1个短韵将清除)
			Rhythm_ForcePPH_Basic();	
		}
#endif


#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	Print_To_Rhythm("log/日志_韵律.log", "a", 1,"分PH：");
#endif


#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  强制PPH调整  ",2);
#endif



		//在汉字链表中：将韵律词中多个词合并成一个节点（pos属性已无意义），去掉PW节点，每个汉字节点即为1个单独的PW	
		MergeWordsOfPW();	

#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  合成韵律词后  ",2);
#endif



		//变音（一，不，连续3声，叠词）
		ChangeTone();

#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  变调后  ",2);
#endif

#if DEBUG_LOG_POLY_TEST
		Print_Poly_Test(2, "变调");
#endif

		if (g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue )			//收到退出命令
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			return g_hTTS->m_nErrorID;
		}

//#if HTRS_LINK_CN_EN
//		if( g_hTTS->m_HaveEnWordInCnType > 0 )		//中文句中有英文单词
//		{
//			nLen = emStrLenA(g_pText)/2;
//
//			if( g_hTTS->m_HaveEnWordInCnType > 1)			//正合成： 中间段 或 句尾段；
//			{
//				//“是”的音节：句首人工增加
//				for(i=nLen-1; i>=0; i--)
//				{
//					g_pTextInfo[i+1].Pos          = g_pTextInfo[i].Pos;
//					g_pTextInfo[i+1].Len          = g_pTextInfo[i].Len;
//					g_pTextInfo[i+1].TextType     = g_pTextInfo[i].TextType;
//					g_pTextInfo[i+1].BorderType   = g_pTextInfo[i].BorderType;
//					g_pTextInfo[i+1].nBiaoDian[0] = g_pTextInfo[i].nBiaoDian[0];
//					g_pTextInfo[i+1].nBiaoDian[1] = g_pTextInfo[i].nBiaoDian[1];
//					g_pTextPinYinCode[i+1]        = g_pTextPinYinCode[i];
//				}
//				g_pTextInfo[0].Pos = 99;
//				g_pTextInfo[0].Len = 2;
//				g_pTextInfo[0].TextType = 1;
//				g_pTextInfo[0].BorderType = BORDER_PPH;		//设置成韵律短语
//				g_pTextInfo[0].nBiaoDian[0] = 0;
//				g_pTextInfo[0].nBiaoDian[1] = 0;
//				g_pTextPinYinCode[0] = 10396;
//				nLen++;
//			}
//
//			if( g_hTTS->m_HaveEnWordInCnType < 3)			//正合成： 句首段 或 中间段 ；
//			{
//				//“是”的音节：句尾人工增加			
//				g_pTextInfo[nLen].Pos = 99;
//				g_pTextInfo[nLen].Len = 2;
//				g_pTextInfo[nLen].TextType = 1;
//				g_pTextInfo[nLen].BorderType = BORDER_PPH;		//设置成韵律短语
//				g_pTextInfo[nLen].nBiaoDian[0] = 0;
//				g_pTextInfo[nLen].nBiaoDian[1] = 0;
//				g_pTextPinYinCode[nLen] = 10396;
//			}
//		}
//#endif

#if DEBUG_LOG_SWITCH_HEAP
		g_pLabRTGlobal = emHeap_AllocZero(sizeof(struct tagLabRTGlobal) , "g_pLabRTGlobal《LAB内存   》");	
		pLabRam = (emByte emPtr)emHeap_AllocZero(MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN, "内部m_LabRam《LAB内存   》");	

#else
		g_pLabRTGlobal = emHeap_AllocZero(sizeof(struct tagLabRTGlobal) );	
		pLabRam = (emByte emPtr)emHeap_AllocZero(MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN);	
#endif
		for( k=0; k<MAX_CN_LAB_LINE_NUM; k++)
			g_pLabRTGlobal->m_LabRam[k] = pLabRam + k * LAB_CN_LINE_LEN;			//赋值指针地址


		ToLab();		//汉字转Lab模块

#if HTRS_LINK_CN_EN
		if( g_hTTS->m_HaveEnWordInCnType > 0 )		//中文句中有英文单词
		{
			if( g_hTTS->m_HaveEnWordInCnType > 1)			//正合成： 中间段 或 句尾段；
			{
				//“是”的音节：句首人工去除
				for(i=1; i<g_pLabRTGlobal->m_MaxLabLine; i++)
					emMemCpy(g_pLabRTGlobal->m_LabRam[i],g_pLabRTGlobal->m_LabRam[i+1],LAB_CN_LINE_LEN);
				g_pLabRTGlobal->m_MaxLabLine--;
			}
			
			if( g_hTTS->m_HaveEnWordInCnType < 3)			//正合成： 句首段 或 中间段 ；
			{
				//“是”的音节：句尾人工去除
				i = g_pLabRTGlobal->m_MaxLabLine-2;
				emMemCpy(g_pLabRTGlobal->m_LabRam[i],g_pLabRTGlobal->m_LabRam[i+1],LAB_CN_LINE_LEN);	//把句尾sil前移
				g_pLabRTGlobal->m_MaxLabLine--;

				//句尾sil不加入合成
				g_pLabRTGlobal->m_MaxLabLine--;
			}

			g_hTTS->m_HaveEnWordInCnType = 2;
		}
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"合并+变音+转Lab", 0);
#endif

#if EM_SYN_SEN_HEAD_SIL
		g_hTTS->m_nParaStartLine = 0;			//从第0个音素开始合成，即合成句首sil
#else
		g_hTTS->m_nParaStartLine = 1;			//从第1个音素开始合成，即不合成句首sil
#endif	

		//后端：语音合成
        g_hTTS->m_bResample = emFalse;			//进入后端前：设置成不要重采样，让后端自己识别重采样
		if( g_pLabRTGlobal->m_MaxLabLine > 2)	//防止合成的都是不发音的汉字  例如：䴘䴙。
		{
			if( bIsSylTailSil == emFalse)
				g_pLabRTGlobal->m_MaxLabLine--;

			g_hTTS->m_TimeRemain = 0.0;
			RearSynth(SYNTH_CN);				
		}
		g_hTTS->m_bResample = emFalse;			//从入后端出来：设置成不要重采样，免得提示音等受影响


#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pLabRam, MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN, "内部m_LabRam《LAB内存   》");	
		emHeap_Free(g_pLabRTGlobal, sizeof(struct tagLabRTGlobal) , "g_pLabRTGlobal《LAB内存   》");		
#else
		emHeap_Free(pLabRam, MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN);	
		emHeap_Free(g_pLabRTGlobal, sizeof(struct tagLabRTGlobal));	
#endif

	
#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"后端--完成", 0);
#endif

		ClearTextItem();	//汉字链表清零
	}

	g_hTTS->m_nCurIndexOfAllHanZiBuffer = 0;
	g_hTTS->m_nCurPointOfPcmInfo = g_hTTS->m_structPromptInfo;

	g_hTTS->m_nHanZiCount_In_HanZiLink = 0;  //个数清零

	g_nLastTextByteIndex = 0;


	return g_hTTS->m_nErrorID;

}




//****************************************************************************************************
// 设置《控制标记》相关的全局参数
//
//****************************************************************************************************
void emCall	InitTextMarkG()					
{

	g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy	= emFalse	;						//提示音处理策略
	g_hTTS->m_ControlSwitch.m_nReadDigitPolicy	= emTTS_READDIGIT_AUTO 	;			//数字处理策略
	g_hTTS->m_ControlSwitch.m_bYaoPolicy		= emTTS_CHNUM1_READ_YAO;			//中文“一”的读法：默认时读成 幺
	g_hTTS->m_ControlSwitch.m_nSpeakStyle		= emTTS_STYLE_NORMAL;				//发音风格： word-by-word 或者 自然方式
	g_hTTS->m_ControlSwitch.m_nVoiceSpeed		= emTTS_SPEED_NORMAL;				//播放速度
	g_hTTS->m_ControlSwitch.m_nVoicePitch		= emTTS_PITCH_NORMAL;				//基频
	g_hTTS->m_ControlSwitch.m_nVolumn			= emTTS_VOLUME_NORMAL; 				//音量大小
	g_hTTS->m_ControlSwitch.m_bPunctuation		= emTTS_PUNCTUATION_NO_READ;		//标点处理策略
	g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = emTTS_USE_PINYIN_CLOSE;         //识别拼音开关关闭
	g_hTTS->m_ControlSwitch.m_bXingShi          = emTTS_USE_XINGSHI_CLOSE;			//读姓氏开关关闭
	g_hTTS->m_ControlSwitch.m_nManualRhythm     = emTTS_USE_Manual_Rhythm_CLOSE;	//韵律标注策略
	g_hTTS->m_ControlSwitch.m_nVoiceMode = emTTS_USE_VoiceMode_CLOSE;		//音效模式关闭
	g_hTTS->m_ControlSwitch.m_nLangType = emTTS_LANG_CN;					//语种：自动选择
	g_hTTS->m_ControlSwitch.m_bZeroPolicy       = emTTS_EN_0_ZERO;					//英文0：默认读成：zero



	g_hTTS->m_ControlSwitch.m_bIsPhoneNum		= emNull;							//数字不按号码读
	g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen   = 0;                               //数字不按比分读

	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emFalse;						//遇到[p*]停顿不输出本句的句尾sil
	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil = emFalse;						//遇到[p*]停顿不输出本句的句首sil

	g_hTTS->m_FirstSynthSegCount = 1;												//本次合成文本第几个分段计数。（例如：本段中1个句子的第1个小分段 = 1）


}



//****************************************************************************************************
// //设置其它全局参数
//
//****************************************************************************************************
void emCall	InitOtherG()	
{

	g_hTTS->m_ControlSwitch.m_nCodePageType		= emTTS_CODEPAGE_GBK ;					//输入文本代码页类型
	g_hTTS->m_ControlSwitch.m_nRoleIndex = emTTS_USE_ROLE_XIAOLIN;				//设置默认发音人

	g_hTTS->m_bResample = emFalse;														//初始化：无需重采样

	g_hTTS->m_ChaBoCount						= 0;
	g_hTTS->m_bIsErToLiang						= emFalse;

	g_hTTS->m_nPrevSenType						= SEN_TYPE_CN;							//初始化设置：前句的语种类别为：中文句

}


//针对Unicode的小头存储方式：做高低字节的转换
//void emCall	ChangeHighLowForUnicodeLE(emInt16 nSize)
//{
//	emPByte pDataOfBasic = (emPUInt8)g_hTTS->m_pc_DataOfBasic;
//	emInt16 nCount = 0;
//	do
//	{	emByte tempByte       = *pDataOfBasic; 			
//		*pDataOfBasic         = *( pDataOfBasic + 1 );   
//		*( pDataOfBasic + 1 ) = tempByte;
//		pDataOfBasic += 2;		
//		nCount = nCount+2;
//	}while(  nCount< nSize);
//}


#if 1
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
// 实现函数：语音合成： emTTS_SynthText
//
// 在当前实例中合成一段文本：当前线程会被堵塞，直到合成结束
//
//****************************************************************************************************

emTTSErrID emCall emTTS_SynthText(
	emHTTS			hTTS,				//[in] handle to an instance
	emCPointer		pcData,				// [in] pointer of text buffer data to be synthesized 
	emSize			nSize )			// [in] size of text buffer data to be synthesized 
{  

	emUInt nIsCheckInfo = 0;
	emInt16  i;
	emUInt32 nProgressBegin, nProgressLen;

	emUInt32 nCodeType;
	int len=0;
    emByte uginput[1024]={0} ;		//utf8 1024大约是300个汉字
    emByte ugout[1024]={0};


#if DEBUG_LOG_SWITCH_HANZI_LINK|DEBUG_LOG_POLY_TEST

	//载入 POS编码查询表 和 拼音编码查询表
	emCharA tempBuffer[50];  //临时数组
	FILE *fp_pinyin = NULL, *fp_pos = NULL;
	emInt32  tt1,tt2;

	fp_pinyin = fopen("..\\irf\\拼音编码查询表.txt", "r");	
	pinyinTableLog.nLineCount = 0;
	while(fgets(tempBuffer, 50, fp_pinyin) != NULL)			//sqb	每次读取文件的一行，50是一行的最大字符数
	{
		tt1 = strchr(tempBuffer,0x09)-tempBuffer;				//sqb 0x09表示\t  0x0a 表示\n  
		tt2 = strchr(tempBuffer,0x0a)-tempBuffer;				//先找\t,找到音节信息，再定位到tempbuffer的下一行
		emMemCpy(pinyinTableLog.pPinYin[pinyinTableLog.nLineCount],tempBuffer,tt1);	//pinyin的值
		pinyinTableLog.pPinYin[pinyinTableLog.nLineCount][tt1] = 0;
		pinyinTableLog.pCode[pinyinTableLog.nLineCount] = atoi(tempBuffer+tt1+1);	//pinyin的编码

		pinyinTableLog.nLineCount++;
	}
	fclose(fp_pinyin);

		

	fp_pos = fopen("..\\irf\\POS值查询表.txt", "r");	
	posTableLog.nLineCount = 0;
	while(fgets(tempBuffer, 50, fp_pos) != NULL)
	{
		tt1 = strchr(tempBuffer,0x09)-tempBuffer;  
		tt2 = strchr(tempBuffer,0x0a)-tempBuffer;             
		emMemCpy(posTableLog.pPos[posTableLog.nLineCount],tempBuffer,tt1);			//pos的值
		posTableLog.pPos[posTableLog.nLineCount][tt1] = 0;
		posTableLog.pCode[posTableLog.nLineCount] = atoi(tempBuffer+tt1+1);			//pos的编码

		posTableLog.nLineCount++;
	}
	fclose(fp_pos);
#endif




#if DEBUG_LOG_SWITCH_TIME
	#if ARM_MTK_LOG_TIME
		kal_get_time(&tick_1);
		g_t1_time = kal_ticks_to_milli_secs(tick_1);
	#else
		#if ARM_LINUX_SWITCH
			gettimeofday(&tv1,&tz1);
			g_t1_time =  (tv1.tv_sec*1000)+(tv1.tv_usec/1000);
		#else
			g_t1_time =0;
		#endif
	#endif

	g_total_time = 0;	
	myTimeTrace_For_Debug(11,(emByte *)"计时开始", 0);
	myTimeTrace_For_Debug(2,(emByte *)"\n\n\n ************************         小计(毫秒)         总计(毫秒)", 0);
	myTimeTrace_For_Debug(1,(emByte *)"进SynthText前******", 0);
#endif


	g_BaseStack = 0;

	LOG_StackAddr(__FUNCTION__);				//打印栈顶信息，调试用

	g_hTTS->m_bStop = emFalse;
	global_bStopTTS = emFalse;

	
#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 1, "新建文件",0);
#endif

#if DEBUG_LOG_POLY_TEST
	Print_Poly_Test(1, "新建文件");
#endif

	if ( pcData == emNull )
		return emTTS_ERR_BASE;

	if ( nSize == 0  )
		return emTTS_ERR_OK;

    //把utf8转gbk放一开始  sqb 2017/6/13
    if(emTTS_CODEPAGE_UTF8==1){
        emUInt16 size = emStrLenA((emStrA) pcData) * sizeof(emCharA);
        emMemCpy(uginput,pcData,size);
        u2g(uginput,strlen(uginput),ugout,1024);
        emMemSet(pcData,0,size);
        emMemCpy(pcData,ugout,size);
    }

	if ( nSize == (emSize)-1 )					//sqb 测试默认给-1值，即此处进if
	{
		emTTS_GetParam(hTTS, emTTS_PARAM_INPUT_CODEPAGE, &nCodeType);
        if(emTTS_CODEPAGE_UTF8==1)
        {
            g_hTTS->m_nNeedSynSize = emStrLenA((emStrA) pcData) * sizeof(char);
        }
        else if( nCodeType ==emTTS_CODEPAGE_UTF16LE  || nCodeType == emTTS_CODEPAGE_UTF16BE )
		{
			nSize = nSize;		//临时加的，免得没有语句

			g_hTTS->m_nNeedSynSize = emStrLenW((emStrW)pcData) * sizeof(emCharW);

		}
		else {
			g_hTTS->m_nNeedSynSize = emStrLen((emStr) pcData) * sizeof(emChar);

		}
	}
	else

		g_hTTS->m_nNeedSynSize = nSize;


	g_hTTS->m_pc_DataOfBasic = pcData;

	g_hTTS->m_nCurIndexOfBasicData = 0;

	g_hTTS->m_FirstSynthSegCount = 1;					//本次合成文本第几个分段计数。（例如：本段中1个句子的第1个小分段 = 1）


	g_hTTS->m_CurMuteMs = 0;		//初始化
	g_hTTS->m_bIsMuteNeedResample = emFalse;
	g_hTTS->m_NextMuteMs = 0;		//初始化


	g_hTTS->m_bIsStartBoFang = emFalse;
	g_hTTS->m_bIsSynFirstTxtZi = emTrue;


	
	//循环进行编码转换处理，全部转成GBK编码，并进行语音合成
	while( global_bStopTTS == emFalse )
	{
		emUInt16 nErrorID = emTTS_ERR_OK;		

		if(  g_hTTS->m_bIsStartBoFang == emFalse )
			g_hTTS->m_CurMuteMs = 0;
		else
		{
			g_hTTS->m_CurMuteMs = g_hTTS->m_NextMuteMs;
			g_hTTS->m_bIsMuteNeedResample = emTrue;
		}

		g_hTTS->m_NextMuteMs = 0;	

		nProgressBegin = g_hTTS->m_nCurIndexOfBasicData;		
#if EM_TRIAL
		if(    g_hTTS->m_ChaBoCount == CHABO_INDEX		//到了插播的第几条
			&& nIsCheckInfo != 2)						//不是在播放客户信息
		{
			GetChaBoText();
		}
		else
		{
			nErrorID = CodeConvertToGBK( );	
		}
		g_hTTS->m_ChaBoCount++;
		if( g_hTTS->m_ChaBoCount == CHABO_EACH_COUNT)
		{
			g_hTTS->m_ChaBoCount = 0;		//  2012-04-23

		}
#else
		nErrorID = CodeConvertToGBK( );
#endif

		g_hTTS->m_FirstSynthSegCount = 1;		//防止音卡  例如：[s10]英文字母混读联通3G精彩在“沃”一名村民称他在自家的房前看到了UFO这是一个处处充满PK的时代行业应用前方600米接近目的地上地城铁站行驶2.5公里右前方行驶到上地信息路欢迎乘坐申新巴士四十九路无人售票车您的消费总额是475.25元收您500元找零24.75元您有新来电电话来自解晓东您有新消息新消息来自13905511861[d]

		if( nErrorID == emTTS_ERR_END_OF_INPUT)
		{
			break;		//编码转换结束，跳出循环,退出TTS
		}

		nProgressLen = g_hTTS->m_nCurIndexOfBasicData - nProgressBegin ;


#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
		//计算转换后的文本最后一个word是否换行符
		for( i =0; i< 1000 ;i=i+2)
		{
			if( GetWord(g_hTTS->m_pDataAfterConvert,i) == (emInt16)0xffff) 
			{
				if(    GetWord(g_hTTS->m_pDataAfterConvert,i-2) == (emInt16)0xa38a
					|| GetWord(g_hTTS->m_pc_DataOfBasic,g_hTTS->m_nCurIndexOfBasicData-2) == (emInt16)0x0d0a 
					|| GetWord(g_hTTS->m_pc_DataOfBasic,g_hTTS->m_nCurIndexOfBasicData) == (emInt16)0x0d0a 
					|| GetWord(g_hTTS->m_pc_DataOfBasic,g_hTTS->m_nCurIndexOfBasicData+2) == (emInt16)0x0d0a )

				{
					//是换行符，在《韵律日志》中也换行
					g_hTTS->bTextTailWrap = emTrue;
				}
				else
				{
					//非换行符，在《韵律日志》中不换行
					g_hTTS->bTextTailWrap = emFalse;
				}
				break;
			}

		}
#endif


#if	EM_PROCESS_CALLBACK_OPEN    //若打开：进度回调开关

		if ( g_pProgressCallbackFun )
		{
			//合成进度回调
			( *  g_pProgressCallbackFun)(NULL, nProgressBegin, nProgressLen);
		}

#endif

		g_hTTS->m_ControlSwitch.bIsReadDian = emFalse;

		if( g_Res.offset_FanToJian != 0 )
		{
			//若有《繁转简表》资源，则进行繁简转换
			FanToJian();		//对g_hTTS->m_pDataAfterConvert里的数据进行
		}


		if( nIsCheckInfo == 0 )			//  2012-04-23
			nIsCheckInfo = CheckInfo();



		g_hTTS->m_nCurIndexOfConvertData = 0;
		g_hTTS->m_nCurPointOfPcmInfo = g_hTTS->m_structPromptInfo;

		if ( nErrorID == emTTS_ERR_FAILED )		//失败： 异常 或 无文本需进行语音合成
		{
			break;		//跳出循环
		}

		g_hTTS->m_ControlSwitch.m_bYiBianYinPolicy = emTTS_USE_YIBIANYIN_OPEN;

		g_hTTS->m_nCurIndexOfAllHanZiBuffer = 0;
		g_hTTS->m_nHanZiCount_In_HanZiLink = 0;		



		//处理错误：[g2]a,b,c.  [g1]a,b,c.
		if( GetWord( g_hTTS->m_pDataAfterConvert, 0 )  == (emInt16)0xa3db  )			//符号为："["
			JudgeCtrlMark(GetFuHaoSum( g_hTTS->m_pDataAfterConvert, 0), g_hTTS->m_pDataAfterConvert,0);		//识别控制标记并设置全局变量

		//sqb  去掉英文句，目前只考虑中文
		//判断是按“中文句”还是按“英文句”处理		
		g_hTTS->m_HaveEnWordInCnType = 0;
		if( DecideSenType() ==  SEN_TYPE_EN)			//处理：英文句
		{
			//HandleEnSen();			//sqb
		}
		else											//处理：中文句
		{
			HandleCnSen();

			if( g_hTTS->m_nCurIndexOfAllHanZiBuffer>0)
				nErrorID = FirstHandleHanZiBuff(emTrue);	//段尾触发，《汉字缓冲区》的优先处理
			else
				if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
					break;			
		}	


		if ( nErrorID == emTTS_ERR_EXIT )		//全部文本转换结束, 退出TTS	
		{
			break;		//跳出循环,退出TTS
		}


	}; //end while


#if	EM_PROCESS_CALLBACK_OPEN    //若打开：进度回调开关
	if ( g_pProgressCallbackFun )
	{
		//合成进度回调
		( *  g_pProgressCallbackFun)(NULL, nProgressBegin+nProgressLen, 0); 
	}
#endif


#if DEBUG_LOG_SWITCH_TIME
	myTimeTrace_For_Debug(12,(emByte *)"计时结束", 0);
#endif


	return emTTS_ERR_OK;
}

//****************************************************************************************************
// 实现函数：创建实例： emTTS_Create 
//	对于每段合成2字的nHeapSize的大小目前为：42K（无音效），62K（有音效）
//****************************************************************************************************
emTTSErrID emCall emTTS_Create(
	emHTTS emPtr	phTTS,				// [out] 实例句柄
	emPointer		pHeap,				// [in] 实例堆
	emSize			nHeapSize,			// [in] 实例堆大小
	emPointer		pCBParam,			// [in] 用户回调参数  输出音频数据文件的指针
	emPResPackDesc	pResPackDesc,		// [in] 资源包描述数组
	emSize			nResPackCount )		// [in] 资源包个数
{

	emInt32 nMinHeap;
	emInt8 bIsLessMin = emFalse;
	emInt8 nFloatType;
	emInt32	*p;
	emInt8	i;


#if EM_8K_MODEL_PLAN							//8K模型配置，内存空间控制
	nMinHeap = USER_HEAP_MIN_CONFIG;
#else											//16K模型配置
	nMinHeap = USER_HEAP_MAX_CONFIG;
#endif

#if EM_USER_SWITCH_VOICE_EFFECT		//带音效
	nMinHeap += USER_HEAP_VOICE_EFFECT;
#endif

	if( nHeapSize < nMinHeap)
	{
#if DEBUG_LOG_SWITCH_ERROR
		g_fLogError = fopen("log/日志_错误.log","a");	
		fprintf(g_fLogError, "%s%d%s%d\n","错误：开辟的堆内存不足!	",nHeapSize," < ", nMinHeap);
		fclose(g_fLogError);
#endif
		return emTTS_ERR_INSUFFICIENT_HEAP;
	}

	g_bIsUserAddHeap = 0;
#if EM_DECISION_HEAP				//是否支持决策树内存
	#if EM_8K_MODEL_PLAN			//8K模型配置		
		nMinHeap += USER_HEAP_MIN_CONFIG_ADD;
		if( nHeapSize >= nMinHeap )
			g_bIsUserAddHeap = 1;				
	#else							//16K模型配置
		nMinHeap += USER_HEAP_MAX_CONFIG_ADD;
		if( nHeapSize >= nMinHeap )
			g_bIsUserAddHeap = 1;				
	#endif
#endif
	
	//创建资源包 
	emResPack_Initialize(&g_ResPack, pResPackDesc);	
	emResCacheLog_Begin();			//开始资源 Cache Log 

	
	///////////////////////////////////////////////
	g_pHeap = (emPHeap)emGridPtr(pHeap);
	/* 初始化堆 */
	emHeap_InitH(g_pHeap, nHeapSize - (EM_PTR_GRID - 1));
	///////////////////////////////////////////////


#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS = (struct emHTTS_G *)emHeap_AllocZero( sizeof(struct emHTTS_G), "全局g_hTTS：《入口：创建》");
#else
	g_hTTS = (struct emHTTS_G *)emHeap_AllocZero( sizeof(struct emHTTS_G));
#endif



	InitTextMarkG();			//设置《控制标记》相关的全局参数
	InitOtherG();				//设置其它全局参数



#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->m_pDataAfterConvert = (emPByte)emHeap_AllocZero( CORE_CONVERT_BUF_MAX_LEN, "转换后的标准文本：《入口：创建》");
#else
	g_hTTS->m_pDataAfterConvert = (emPByte)emHeap_AllocZero( CORE_CONVERT_BUF_MAX_LEN);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_pTextInfo = (struct TextItem *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem ), "汉字链信息：《入口：创建》");
#else
	g_pTextInfo = (struct TextItem *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem ));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	g_pText     = (emByte *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "汉字链文字：《入口：创建》");
#else
	g_pText     = (emByte *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	g_pTextPinYinCode  = (emInt16 *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "汉字链拼音：《入口：创建》");
#else
	g_pTextPinYinCode  = (emInt16 *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif



	// 
#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->pRearG = (emRearG *) emHeap_AllocZero(sizeof(emRearG), "后端pRearG：《入口：创建》");
#else
	g_hTTS->pRearG = (emRearG *) emHeap_AllocZero(sizeof(emRearG));
#endif


/************************************* 目前从emConfig.h中得到的界面参数    *********************************************/

	g_hTTS->pRearG->bIsVQ = EM_INTERFACE_VQ;
	g_hTTS->pRearG->bIsStraight = EM_INTERFACE_IS_STRAIGHT;
	g_hTTS->pRearG->stage = EM_INTERFACE_IS_LSP;

/************************************* 目前从界面带入的参数，以后直接初始化 *********************************************/

	
	g_hTTS->pRearG->beta = 0.4;
	g_hTTS->pRearG->Speech_speed = 1.0;
	g_hTTS->pRearG->f0_mean = 0.0;




	


#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->pInterParam = (emInterParam *) emHeap_AllocZero(sizeof(emInterParam), "后端pInterParam：《入口：创建》");
#else
	g_hTTS->pInterParam = (emInterParam *) emHeap_AllocZero(sizeof(emInterParam));
#endif

	g_hTTS->pInterParam->msd_threshold = 0.45;						///sqb   2016.12.30
	g_hTTS->pInterParam->nState = 10;

	if(g_hTTS->pRearG->stage == MODEL_MGC)
	{

		g_hTTS->pRearG->alpha = 0.42;
		g_hTTS->pRearG->gamma = 0;
	}
	else
	{
		g_hTTS->pRearG->alpha = 0.0;
		g_hTTS->pRearG->gamma = -1;
	}
	g_hTTS->pInterParam->width = 3;
	g_hTTS->pInterParam->win_coefficient[0][0] = 0;
	g_hTTS->pInterParam->win_coefficient[0][1]  = 2;
	g_hTTS->pInterParam->win_coefficient[0][2]  = 0;
	g_hTTS->pInterParam->win_coefficient[1][0] = -1;
	g_hTTS->pInterParam->win_coefficient[1][1]  = 0;
	g_hTTS->pInterParam->win_coefficient[1][2]  = 1;
	g_hTTS->pInterParam->win_coefficient[2][0] = 2;
	g_hTTS->pInterParam->win_coefficient[2][1]  = -4;
	g_hTTS->pInterParam->win_coefficient[2][2]  = 2;
	// param init for hts rear

	g_hTTS->pCBParam = pCBParam;

	
#if EM_RES_READ_REAR_LIB_CONST				//LIB常量仅读后端资源（其它资源还是从irf中读）
	g_hTTS->fResFrontMain       = pResPackDesc->pCBParam;
	if( pResPackDesc->pCBParam == emNull )
		return emTTS_ERR_RESOURCE;

	g_hTTS->fResCurRearMain     = (emPointer)ResourceConstData;
#else
	#if EM_RES_READ_ALL_LIB_CONST			//LIB常量读所有资源（此开关特别：无需irf资源文件，全部从常量读）
		g_hTTS->fResFrontMain   = (emPointer)ResourceConstData;
		g_hTTS->fResCurRearMain = (emPointer)ResourceConstData;
	#else									//irf资源文件读(全部)
		g_hTTS->fResFrontMain   = pResPackDesc->pCBParam;
		g_hTTS->fResCurRearMain = pResPackDesc->pCBParam;
		if( pResPackDesc->pCBParam == emNull )
			return emTTS_ERR_RESOURCE;
	#endif
#endif  

	g_hTTS->m_cbReadRes = pResPackDesc->pfnRead;

		GetTotalResOffset();		//获取所有资源的数据偏移量

	g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;
	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_RearVoiceLib_Cur  ,0);
	fFrontRead(&g_hTTS->pRearG->nModelSample,2,1,g_hTTS->fResFrontMain);								//在此需读入模型采样率，提示音模块需要     2012-2-6


	/************************************* 从资源中读入的参数 *********************************************/

	fRearSeek(g_hTTS->fResCurRearMain,g_Res.offset_RearVoiceLib_Cur  ,FSEEK_START);					//中文模型后端资源库
	fRearRead(&g_hTTS->pRearG->nModelSample,2,1,g_hTTS->fResCurRearMain);							//模型采样率
	fRearRead(&g_hTTS->pInterParam->static_length, sizeof(emInt16), 1, g_hTTS->fResCurRearMain);	//频谱的维数
	fRearRead(&g_hTTS->pRearG->offset_cn, sizeof(g_hTTS->pRearG->offset_cn),1,g_hTTS->fResCurRearMain);	//后端的每个小资源偏移
	
	//后端小资源偏移：相对偏移-->绝对偏移
	for( p = &g_hTTS->pRearG->offset_cn, i=0; i < sizeof(g_hTTS->pRearG->offset_cn)/sizeof(emInt32); i++ )
	{
		*(p++) += g_Res.offset_RearVoiceLib_Cur;
	}
	
	//所有模型float叶节点占的字节数的类型:  值=2：	所占字节全部为2字节
	//										值=3：	基频模型0阶均值占4字节,频谱模型能量维0阶均值占4字节,其余占2个字节
	//										值=4：	所占字节全部为4字节
	//fRearRead(&nFloatType, sizeof(emInt8),1,g_hTTS->fResCurRearMain);				//sqb  2016.12.2
	nFloatType = 4;
	if( nFloatType == 2 )
	{
		g_hTTS->pRearG->nModelFloatLen = 2;
		g_hTTS->pRearG->nModelFloatLeafAdd = 0;
	}
	if( nFloatType == 4 )
	{
		g_hTTS->pRearG->nModelFloatLen = 4;
		g_hTTS->pRearG->nModelFloatLeafAdd = 0;
	}
	if( nFloatType == 3 )
	{
		g_hTTS->pRearG->nModelFloatLen = 2;
		g_hTTS->pRearG->nModelFloatLeafAdd = 2;
	}	
			

	g_hTTS->pRearG->frame_length = g_hTTS->pRearG->nModelSample/200;						//帧长
	g_hTTS->pRearG->nMgcLeafPrmN = g_hTTS->pInterParam->static_length*g_hTTS->pInterParam->width;


	if( g_bIsUserAddHeap == 1)				//用户给了多余的内存（适合内存空间富裕的客户，可将后端22颗决策树全部载入到内存）
	{
		//加载后端的决策树
		LoadRearAllDecision();
	}

#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->m_Info = emHeap_AllocZero( MAX_COMPANY_INFO_LEN, "信息：《入口：创建》");
#else
	g_hTTS->m_Info = emHeap_AllocZero( MAX_COMPANY_INFO_LEN);
#endif

	//创建：PCM输出的缓冲区
#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE, "PCM输出缓冲：《入口：创建》");
#else
	g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE);
#endif
	g_hTTS->m_pPCMBufferSize = MAX_OUTPUT_PCM_SIZE;

	return emTTS_ERR_OK;
}



//****************************************************************************************************
// 释放被创建的实例
//
//****************************************************************************************************
emTTSErrID emCall emTTS_Destroy(
	emHTTS			hTTS )				/* [in] handle to an instance */
{




	//释放：PCM输出的缓冲区
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE, "PCM输出缓冲：《出口：销毁》");
#else
	emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->m_Info, MAX_COMPANY_INFO_LEN,  "信息：《出口：销毁》");
#else
	emHeap_Free(g_hTTS->m_Info, MAX_COMPANY_INFO_LEN);
#endif

	if( g_bIsUserAddHeap == 1)				//用户给了多余的内存（适合内存空间富裕的客户，可将后端22颗决策树全部载入到内存）
	{
		//释放后端的决策树
		FreeRearAllDecision();
	}



#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->pInterParam, sizeof(emInterParam), "后端pInterParam：《出口：销毁》");
#else
	emHeap_Free(g_hTTS->pInterParam, sizeof(emInterParam));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->pRearG, sizeof(emRearG), "后端pRearG：《出口：销毁》");
#else
	emHeap_Free(g_hTTS->pRearG, sizeof(emRearG));
#endif



#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_pTextPinYinCode, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "汉字链拼音：《出口：销毁》");
#else
	emHeap_Free( g_pTextPinYinCode, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_pText, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "汉字链文字：《出口：销毁》");
#else
	emHeap_Free( g_pText, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_pTextInfo, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem), "汉字链信息：《出口：销毁》");
#else
	emHeap_Free( g_pTextInfo, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_hTTS->m_pDataAfterConvert, CORE_CONVERT_BUF_MAX_LEN, "转换后的标准文本：《出口：销毁》");
#else
	emHeap_Free( g_hTTS->m_pDataAfterConvert, CORE_CONVERT_BUF_MAX_LEN);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS,  sizeof(struct emHTTS_G), "全局g_hTTS：《出口：销毁》");
#else
	emHeap_Free(g_hTTS,  sizeof(struct emHTTS_G));
#endif


	emHeap_UninitH(g_pHeap);

	//结束资源 Cache Log 
	emResCacheLog_End();

	return emTTS_ERR_OK;
}


//****************************************************************************************************
// 获取实例的参数
//
//****************************************************************************************************
emTTSErrID emCall emTTS_GetParam(
	emHTTS			hTTS,				/* [in] handle to an instance */
	emUInt32		nParamID,			/* [in] parameter ID */
	emPUInt32		pnParamValue )		/* [out] buffer to receeme the parameter value */
{

	switch(nParamID)
	{

	case emTTS_PARAM_OUTPUT_CALLBACK:
		*pnParamValue = (emUInt32)g_pOutputCallbackFun;
		break;
	case emTTS_PARAM_PROGRESS_CALLBACK:
		*pnParamValue = (emUInt32)g_pProgressCallbackFun;
		break;


	case emTTS_PARAM_INPUT_CODEPAGE: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nCodePageType; 
		break;


	case emTTS_PARAM_ROLE:
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nRoleIndex; 
		break;


	case emTTS_PARAM_USE_PROMPTS: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy; 
		break;
	case  emTTS_PARAM_READ_DIGIT: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nReadDigitPolicy;
		break;
	case emTTS_PARAM_CHINESE_NUMBER_1: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_bYaoPolicy; 
		break;
	case  emTTS_PARAM_SPEAK_STYLE: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nSpeakStyle; 
		break;
	case emTTS_PARAM_VOICE_SPEED: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nVoiceSpeed;
		break;
	case emTTS_PARAM_VOICE_PITCH: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nVoicePitch;
		break;
	case emTTS_PARAM_VOLUME: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nVolumn; 
		break;
	case emTTS_PARAM_PUNCTUATION: 
		*pnParamValue = g_hTTS->m_ControlSwitch.m_bPunctuation; 
		break;
	case emTTS_PARAM_ManualRhythm:
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nManualRhythm; 
		break;
	case emTTS_PARAM_PINYIN:
		*pnParamValue = g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy;; 
		break;
	case emTTS_PARAM_XINGSHI:
		*pnParamValue = g_hTTS->m_ControlSwitch.m_bXingShi ; 
		break;
	case emTTS_PARAM_LANG:
		*pnParamValue = g_hTTS->m_ControlSwitch.m_nLangType ; 
		break;
	case emTTS_PARAM_ZERO:
		*pnParamValue = g_hTTS->m_ControlSwitch.m_bZeroPolicy ; 
		break;


	default:
		break;
	}

	return emTTS_ERR_OK;


}




//****************************************************************************************************
// 设置实例的参数
//
//****************************************************************************************************
emTTSErrID emCall emTTS_SetParam(
	emHTTS			hTTS,				/* [in] handle to an instance */
	emUInt32		nParamID,			/* [in] parameter ID */
	emUInt32		nParamValue )		/* [in] parameter value */
{

	switch( nParamID )
	{

	case emTTS_PARAM_OUTPUT_CALLBACK:
		g_pOutputCallbackFun = (OutputCallbackFun)nParamValue;
		break;
	case emTTS_PARAM_PROGRESS_CALLBACK:
		g_pProgressCallbackFun = (ProgressCallbackFun)nParamValue;
		break;


	case emTTS_PARAM_INPUT_CODEPAGE:
		g_hTTS->m_ControlSwitch.m_nCodePageType =		(emInt16)nParamValue;
		break;

	case emTTS_PARAM_ROLE:
		g_hTTS->m_ControlSwitch.m_nRoleIndex =			 (emInt16)nParamValue;
		break;

	
	case emTTS_PARAM_USE_PROMPTS:
		g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy =	(emInt8)nParamValue;
		break;
	case emTTS_PARAM_READ_DIGIT:
		g_hTTS->m_ControlSwitch.m_nReadDigitPolicy =	(emInt8)nParamValue;
		break;
	case emTTS_PARAM_CHINESE_NUMBER_1:
		g_hTTS->m_ControlSwitch.m_bYaoPolicy =			(emInt8)nParamValue;
		break;
	case emTTS_PARAM_SPEAK_STYLE:
		g_hTTS->m_ControlSwitch.m_nSpeakStyle =			(emInt8)nParamValue;
		break;
	case emTTS_PARAM_VOICE_SPEED:
		g_hTTS->m_ControlSwitch.m_nVoiceSpeed =			(emInt8)nParamValue;
		break;
	case emTTS_PARAM_VOICE_PITCH:
		g_hTTS->m_ControlSwitch.m_nVoicePitch =			(emInt8)nParamValue;
		break;
	case emTTS_PARAM_VOLUME:
		g_hTTS->m_ControlSwitch.m_nVolumn =				(emInt8)nParamValue;
		break;
	case emTTS_PARAM_PUNCTUATION:
		g_hTTS->m_ControlSwitch.m_bPunctuation =		(emInt8)nParamValue;
		break;	
	case emTTS_PARAM_ManualRhythm:
		g_hTTS->m_ControlSwitch.m_nManualRhythm =		(emInt8)nParamValue;
		break;
	case emTTS_PARAM_PINYIN:
		g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = (emInt8)nParamValue;
		break;
	case emTTS_PARAM_XINGSHI:
		g_hTTS->m_ControlSwitch.m_bXingShi  =			(emInt8)nParamValue;
		break;
	case emTTS_PARAM_LANG:
		g_hTTS->m_ControlSwitch.m_nLangType =			(emInt8)nParamValue;
		break;
	case emTTS_PARAM_ZERO:
		g_hTTS->m_ControlSwitch.m_bZeroPolicy =			(emInt8)nParamValue;
		break;


	default:
		break;
	}    
	return emTTS_ERR_OK;
}







emTTSErrID emCall emTTS_GetVersion(emPByte verStr)
{

	emMemCpy(verStr , EM_VERSION,sizeof(EM_VERSION));
	return emTTS_ERR_OK;	
}



//****************************************************************************************************
// 结束实例的TTS合成：堵塞中的emTTS_Run函数会立刻退
//
//****************************************************************************************************
emTTSErrID emCall emTTS_Exit(
	emHTTS			hTTS )				/* [in] handle to an instance */
{



	g_hTTS->m_bStop = emTrue;	
	//退出合成
	return emTTS_ERR_OK;

}


//处理：中文句
void emCall HandleCnSen()
{
	emInt16  t1,nDataTypePrev,nDataTypeNext;
	emInt16 nCurWord;

	ClearTextItem();	//汉字链表清零

		//fhy 130105 去空格
	{
		int i,j;
		emBool	bTypeQu;
		for( i = 2; g_hTTS->m_pDataAfterConvert[i + 2] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[i + 3] != (emByte)END_WORD_OF_BUFFER; i += 2 )
		{
			bTypeQu = emFalse;
			nDataTypePrev = CheckDataType( g_hTTS->m_pDataAfterConvert, i - 2);
			nDataTypeNext = CheckDataType( g_hTTS->m_pDataAfterConvert, i + 2);

			if( nDataTypePrev != nDataTypeNext)			//前后文本类型不一致
			{
				if(    nDataTypePrev == DATATYPE_SHUZI   && nDataTypeNext == DATATYPE_YINGWEN		//前是数字后是字母
					|| nDataTypePrev == DATATYPE_YINGWEN && nDataTypeNext == DATATYPE_SHUZI )	//前是字母后是数字
				{
					bTypeQu = emFalse;
				}
				else
				{
					bTypeQu = emTrue;
				}
			}

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

	//将要处理句中进行处理：字母大写转小写
	g_hTTS->m_nCurIndexOfConvertData = 0;
	while( global_bStopTTS == emFalse )
	{
		if(    *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData    ) == 0xff
			&& *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) >= 0xff )
		{
			break;			//结束跳出
		}

		if(    *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData    ) == 0xa3
			&& *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) >= 0xc1 
			&& *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) <= 0xda )
		{
			*(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) += 0x20;
		}
		g_hTTS->m_nCurIndexOfConvertData += 2;				
	}
	

	//开始对编码转换区的数据进行循环语音合成
	g_hTTS->m_nCurIndexOfConvertData = 0;
	while( global_bStopTTS == emFalse )
	{

		//获取g_hTTS->m_pDataAfterConvert中目前g_hTTS->m_nCurIndexOfConvertData位置处所指字符的数据类型
		g_hTTS->m_nDataType = CheckDataType(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);

		nDataTypePrev  = CheckDataType(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData-2);
		nDataTypeNext  = CheckDataType(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData+2);

		nCurWord = GetWord(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);  //当前符号

		if( g_hTTS->m_nDataType == DATATYPE_END )
		{
			if( g_hTTS->m_HaveEnWordInCnType > 0 )
				g_hTTS->m_HaveEnWordInCnType = 3;
			break;			//遇到结束符，语音合成结束
		}

		switch( g_hTTS->m_nDataType)
		{
			case DATATYPE_HANZI :

				//add by  2012-03-30  
				if(	  (     nCurWord != (emInt16)0xbacd			//汉字：和
						 && nCurWord != (emInt16)0xd3eb)		//汉字：与
					|| nDataTypePrev != DATATYPE_SHUZI
					|| nDataTypeNext != DATATYPE_SHUZI)
				{
					g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen   = 0;     //数字不按比分读

					//例如:  比分为21∶18、21∶14和21∶14。
				}

				//汉字写入到汉字链表里； 汉字的处理函数《FirstHandleHanZiBuff() 》由《控制标记》和《英文》和《段尾》触发
				g_hTTS->m_nCurIndexOfConvertData = WriteToHanZiLink(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData, POS_CODE_kong ,0);
				if( g_ForcePPH.bIsStartCount == 1)
					g_ForcePPH.nCount += g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
				break;
			case DATATYPE_FUHAO :
				t1 = g_hTTS->m_nCurIndexOfConvertData;

				

				//若本字符是：空格 或 “\”，且本字符前是数字或字母，后是数字或字母，则制造句尾停顿效果    20111227

				//例如：读“Power Down模式” 。 50cm 50kg 50V。
				if (   (    nCurWord == (emInt16)INVALID_CODE)		//空格
					&& (    nDataTypePrev == DATATYPE_YINGWEN)						    
					&& (    nDataTypeNext == DATATYPE_YINGWEN
						 || nDataTypeNext == DATATYPE_SHUZI))
				{
					GenPauseNoOutSil();	//制造句尾停顿效果		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}

				//例如：V60/V66/T720/V300/V303/V500/V600/A768/A768I/  。 火车的速度是622km/h。 
				else if (   nCurWord == (emInt16)0xa3af 			//斜杠
					&& nDataTypePrev == DATATYPE_SHUZI
					&& nDataTypeNext == DATATYPE_YINGWEN)
				{
					GenPauseNoOutSil();	//制造句尾停顿效果		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}


				//若本字符是以下符号，且本字符前或字符后是汉字，则制造句尾停顿效果    2012-03-30
				//例如：本报讯(记者李蕾，查小东)对于央企的“不安”，
				//例如：风量(m3/h)
				else if(   (   nCurWord == (emInt16)0xA3A8			//符号： （
							|| nCurWord == (emInt16)0xA1B2)			//符号： 〔
						&& (   nDataTypePrev == DATATYPE_HANZI
							|| nDataTypePrev == DATATYPE_SHUZI
							|| nDataTypePrev == DATATYPE_YINGWEN))								
				{
					GenPauseNoOutSil();	//制造句尾停顿效果		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}
				else if(   (   nCurWord == (emInt16)0xA3A9			//符号： ）
							|| nCurWord == (emInt16)0xA1B3	)		//符号： 〕							   
					   && (    nDataTypeNext == DATATYPE_HANZI
							|| nDataTypeNext == DATATYPE_SHUZI
							|| nDataTypeNext == DATATYPE_YINGWEN))									
				{
					GenPauseNoOutSil();	//制造句尾停顿效果		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}

				else
				{
					g_hTTS->m_nCurIndexOfConvertData = ProcessFuHao(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);
				}
				
				//如果是：成对标点符号  
				if( IsShuangBiaoDian(g_hTTS->m_pDataAfterConvert, t1) )
				{
					//开始：强制PPH的赋值和计数
					if( g_ForcePPH.nCount != 0)	 //不是第1次计数
					{
						//例如：他看了《红楼梦》后很开心
						g_ForcePPH.nSylLen = g_ForcePPH.nCount;
						g_ForcePPH.nCount = 0;
						g_ForcePPH.bIsStartCount = 0;

						
						if( g_ForcePPH.nStartIndex == 0 &&							//成对标点符号 在句子开头
							(g_ForcePPH.nSylLen == 1 || g_ForcePPH.nSylLen == 2) &&	//成对标点符号 内含的字符不超过2个
							GetWord(g_hTTS->m_pDataAfterConvert,g_hTTS->m_nCurIndexOfConvertData-2) == (emInt16)0xa3a9) //是成对符号 （）
						{
							//例如：（1）有强烈的创业愿望和想法。（11）有强烈的创业愿望和想法。（一）有强烈的创业愿望和想法。
							//例如：不应处理：“老李”是一辈子老老实实种地的那种人。
							GenPauseNoOutSil();	//制造句尾停顿效果		
						}
					}
					else						//是第1次计数
					{
						g_ForcePPH.nStartIndex= g_nLastTextByteIndex/2;
						g_ForcePPH.bIsStartCount = 1;
					}

				}
				break;
			case DATATYPE_SHUZI :
				g_hTTS->m_nCurIndexOfConvertData = ProcessShuZi(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);
				if( g_ForcePPH.bIsStartCount == 1)
					g_ForcePPH.nCount += g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
				break;
			case DATATYPE_YINGWEN :
				g_hTTS->m_nCurIndexOfConvertData = ProcessYingWen(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);
				if( g_ForcePPH.bIsStartCount == 1)
					g_ForcePPH.nCount += g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
				break;
			case DATATYPE_UNKNOWN :
				g_hTTS->m_nCurIndexOfConvertData += 2;		//位置向前移动2个字节
				break;
			default:  ;
		};
	}
}


