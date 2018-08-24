#include "emPCH.h"

#include "Front_ProcHZToPinYin.h"

#include "emMath.h"

#include <string.h>
#include <jni.h>
#include <sys/time.h>

long long currentTimeInMilliseconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}


#define NO_FIND_NODE	0xff

#define  COUNT_PolyFunAlisaLink		50

emUInt16		g_WordCount;
emUInt16		g_CharCount;


void emCall ToPinYin_third()
{
    // Get the text to be dealt with, format: "%C4%E3%BA%C3"
    char strGbk[300] = {0};
    int len = strlen(g_pText);
    for(int i = 0; i < len; i++)
    {
        sprintf(strGbk+3*i, "%%%02X", (int)g_pText[i]);
    }
    printf("gbk_pinyin: %s\n", strGbk);

    long long t1 = currentTimeInMilliseconds();
    const char *strRet = jniSentence2Pinyin(strGbk);
    long long t2 = currentTimeInMilliseconds();
    printf("jniSentence2Pinyin time: %d\n", t2-t1);
    printf("code: %s\n", strRet);

    // 6570,5738,3929,3980,2962,1107,
    const char *split = ",";
    char *left = NULL;
    int idx = 0;

    char *p = strtok_r(strRet, split, &left);
    while(p != NULL)
    {
        g_pTextPinYinCode[idx] = (emInt16)atoi(p);
        idx++;
        p = strtok_r(NULL, split, &left);
    }
}


void emCall	ToPinYin()
{

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  调试用  ",0);
#endif

	//添加默认拼音--  不是一次性载入《默认拼音表》的一二级汉字到内存，从文件查表
	AddDefaultPinYin();							 //add by songkai  


#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  添加默认拼音  ",0);
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"多音--默认拼音", 0);
#endif

	//根据多音规则库改变拼音
	ConvertPinYinFromPolyRuler_FromRAM();                             //add by songkai

#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  添加多音规则后  ",0);
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"多音--多音规则", 0);
#endif

}



//****************************************************************************************************
//  功能： 添加默认拼音   add by songkai
//         不是一次性载入《默认拼音表》的一二级汉字到内存，从文件查表
//****************************************************************************************************
void emCall AddDefaultPinYin()
{
	emUInt16 nWordCount = 0, i = 0;  //统计字数
	emUInt16 nRowNum = 0;


	LOG_StackAddr(__FUNCTION__);

	g_nP1TextByteIndex = 0 ;
	while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0)
	{
		//分出来的可能部分有强制读音,部分没有强制读音
		i = 0;
		nWordCount = g_pTextInfo[g_nP1TextByteIndex/2].Len/2;

		if( CheckDataType(&g_pText[g_nP1TextByteIndex] , 0 ) == DATATYPE_HANZI)					
		{
			while(i < nWordCount) //没有强制读音  g_p1TextCsItem->pPinYinCode指向的是一个汉字
			{
				if(g_pTextPinYinCode[g_nP1TextByteIndex/2+i] == 0)  //没有强制拼音(如果没有强制拼音就设置为0)
				{					
					//分别取出汉字的高8位和低8位
					nRowNum = (g_pText[g_nP1TextByteIndex+2*i] - 0x81) * 191 +  g_pText[g_nP1TextByteIndex+2*i+1] - 0x40; //计算出偏移行数
					fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_GBKDefaultPinyin + nRowNum * ROW_SIZE, 0);  //定位到GBK默认拼音表
					fFrontRead(&g_pTextPinYinCode[g_nP1TextByteIndex/2+i], ROW_SIZE, 1, g_hTTS->fResFrontMain);  //读取默认读音(单位是一个汉字)
				}
				i++;
			}//end while
		}

		// -- 2011-02-15修改  为实现功能：英文字母混合在中文中训练
		if( CheckDataType(&g_pText[g_nP1TextByteIndex] , 0 ) == DATATYPE_YINGWEN)	 //是普通英文字母
		{
			while(i < nWordCount) 
			{
				emInt32  tempYunMuCode=  g_pText[g_nP1TextByteIndex+2*i + 1] - 225 + FINAL_CN_letter_a;	//得到该英文字母的韵母编号
				g_pTextPinYinCode[g_nP1TextByteIndex/2+i] = tempYunMuCode*256 + 8 * INITIAL_CN_letter + TONE_CN_letter;	//得到该英文字母的拼音编码  25=声母编号 7=声调编号
				i++;
			}//end while
		}

		g_nP1TextByteIndex += g_pTextInfo[g_nP1TextByteIndex/2].Len;
	}//end while
}




//****************************************************************************************************
//  功能：根据多音规则库改变拼音  add by songkai
//  一次性载入多音规则索引表,使用新的二分法
//****************************************************************************************************

void emCall	ConvertPinYinFromPolyRuler_FromRAM()
{

	emUInt16 cTextLen = 0;
	emInt16 nLine = -1;  //-1表示没有找到 
	emUInt16 i;
	emUInt16 nSize ;
	emPByte PolyIndexTable ;
	emBool bIsFind;
	emInt32 nAllocSize;
	emInt8 *pCurZiCiRule ; //当前字或词的规则数据临时开辟存储
	PolyFunAlisa *pHeadPolyFunAlisaLink = NULL;



	LOG_StackAddr(__FUNCTION__);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_PolyRuleIndex, 0); //定位到索引表
	fFrontRead(&g_WordCount, 2, 1, g_hTTS->fResFrontMain);//读入《多音词》的个数
	fFrontRead(&g_CharCount, 2, 1, g_hTTS->fResFrontMain);//读入《多音字》的个数


	nSize = (g_WordCount + g_CharCount) * INTERVAL ;
	nAllocSize = nSize;

#if DEBUG_LOG_SWITCH_HEAP
	PolyIndexTable = (emPByte)emHeap_AllocZero(nAllocSize, "多音规则索引表：《多音字模块》");  
#else
	PolyIndexTable = (emPByte)emHeap_AllocZero(nAllocSize); 
#endif


#if DEBUG_LOG_SWITCH_HEAP
	pCurZiCiRule = (emInt8 *)emHeap_AllocZero(4096, "当前字多音规则：《多音字模块》");  //估计大约占1K字节,在此多开辟一些，不增大全局堆
#else
	pCurZiCiRule = (emInt8 *)emHeap_AllocZero(4096);  //估计大约占1K字节,在此多开辟一些，不增大全局堆
#endif



#if DEBUG_LOG_SWITCH_HEAP
	pHeadPolyFunAlisaLink = (PolyFunAlisa*)emHeap_AllocZero(COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa),"多音函数链表：《多音字模块》");
#else
	pHeadPolyFunAlisaLink = (PolyFunAlisa*)emHeap_AllocZero(COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa));
#endif


	fFrontRead(PolyIndexTable, nSize, 1, g_hTTS->fResFrontMain);//读入索引数据
	
	if(g_pTextInfo[0].Len != 0)
	{
		g_nCurTextByteIndex = 0;
		while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
		{	
			cTextLen = g_pTextInfo[g_nCurTextByteIndex/2].Len/ 2;  //TEXT由几个字组成

			if(cTextLen == 1 )     //需查找1字多音词 和 多音字     
			{
				bIsFind = FALSE;

				//先查找1字多音词
				nLine = ErFenSearchOne_FromRAM(&g_pText[g_nCurTextByteIndex], 0, PolyIndexTable, 1, g_WordCount, INTERVAL);
				if (nLine != g_WordCount)	//表示查到1字多音词
				{
					bIsFind = HandleCurZiOrCi_FromRAM(nLine, 0, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
				}


				if( bIsFind == FALSE)		//表示未查到1字多音词
				{
					//则开始查找多音字
					nLine = g_WordCount + ErFenSearchOne_FromRAM(&g_pText[g_nCurTextByteIndex], 0, PolyIndexTable + g_WordCount * INTERVAL, 1, g_CharCount, INTERVAL);
					if (nLine != (  g_CharCount + g_WordCount) )	//表示查到1字多音字
					{
						HandleCurZiOrCi_FromRAM(nLine, 0, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
					}
				}
			}

			if(cTextLen == 2 )         //需查找2字多音词 和 循环查找多音字      
			{
				//先查找2字多音词
				nLine = ErFenSearchTwo_FromRAM(&g_pText[g_nCurTextByteIndex], 0, PolyIndexTable, 1, g_WordCount, INTERVAL); 
				if (nLine != g_WordCount)	//表示查到2字多音词
				{
					HandleCurZiOrCi_FromRAM(nLine, 0, 2, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
				}
				else						//表示未查到2字多音词，
				{
					//则开始循环查找多音字
					for(i = 0; i< 2; i++)
					{
						nLine = g_WordCount + ErFenSearchOne_FromRAM(&g_pText[g_nCurTextByteIndex + i*2], 0, PolyIndexTable+ g_WordCount * INTERVAL, 1, g_CharCount, INTERVAL);
						if (nLine != (  g_CharCount + g_WordCount) )	//表示查到1字多音字
						{
							HandleCurZiOrCi_FromRAM(nLine, i, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
						}
					}//end for
				}//end else
			}

			if(cTextLen > 2 )          //只需循环查找多音字     
			{
				//则开始循环查找多音字
				for(i = 0; i< cTextLen; i++)
				{
					nLine = g_WordCount + ErFenSearchOne_FromRAM( &g_pText[g_nCurTextByteIndex + i*2], 0, PolyIndexTable + g_WordCount * INTERVAL, 1, g_CharCount, INTERVAL); 
					if (nLine != (  g_CharCount + g_WordCount) )	//表示查到1字多音字
					{
						HandleCurZiOrCi_FromRAM(nLine, i, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
					}
				}	
			}//end if					

			g_nCurTextByteIndex += g_pTextInfo[g_nCurTextByteIndex/2].Len;
		}//end inner while
	}//end if


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pHeadPolyFunAlisaLink, COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa),"多音函数链表：《多音字模块》");
#else
	emHeap_Free(pHeadPolyFunAlisaLink, COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pCurZiCiRule, 4096, "当前字多音规则：《多音字模块》");  //估计大约占1K字节
#else
	emHeap_Free(pCurZiCiRule, 4096);  //估计大约占1K字节
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(PolyIndexTable, nAllocSize, "多音规则索引表：《多音字模块》");  
#else
	emHeap_Free(PolyIndexTable, nAllocSize);  
#endif

}


//****************************************************************************************************
//  功能： 处理一个字或词的规则:创建规则的多音函数链表   add by songkai
//****************************************************************************************************

emBool emCall HandleCurZiOrCi_FromRAM(emInt16 nLine,				// 《多音字词规则库索引表》的第几行
									emInt16 nCurOffset,				// 要处理的是从g_curTextCsItem中的第几个字开始的字或词
									emInt16 nMaxHanZi,				// 要处理的是 1个字 还是 2个字
									emPByte  PolyIndexTable,		//索引表在内存的首地址	
									emInt8 *pCurZiCiRule,			//当前字多音规则
									PolyFunAlisa *pHeadPolyFunAlisaLink)	//多音函数链表



{

	emUInt16 nLen = 0;
	PolyFunAsistant  polyFA;     //辅助结构体
	PolyFunAlisa *pHead = NULL, *pTemp = NULL, *pCur = NULL, *pFor=NULL; //这个结构体是当前多音字/词的一条规则中的多音函数信息节点
	PolyFunAlisa  *pCurPolyFunAlisaLink = NULL;
	emUInt16 nTrueEncode[2], nFalseEncode[2];
	emByte  cFunCount, cFunLen, nMaxPro;
	emByte i;	
	emBool nRes = -1;	
	emUInt16 nOffSet = 0;
	emInt8 *pRule;
	emBool bReturnValue = FALSE;
	emInt32 nAllocSizeCurZiCiRule;
	emUInt16 nZiCode;		//当前多音字


	LOG_StackAddr(__FUNCTION__);

	//把当前多音字放入到nZiCode中
	emMemCpy(&nZiCode,&g_pText[g_nCurTextByteIndex+ nCurOffset*2],2);          //add by liugang 2011-6-9
	polyFA.Zi_Code = nZiCode;

	emMemCpy(&nOffSet, PolyIndexTable + (nLine ) * INTERVAL + 4,  2);
	emMemCpy(&nLen, PolyIndexTable + (nLine ) * INTERVAL + 6, 2);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_PolyRule + nOffSet, 0);  //定位到规则表中该字/词的相应起始位置
	nAllocSizeCurZiCiRule = nLen;

	fFrontRead(pCurZiCiRule, nLen,  1, g_hTTS->fResFrontMain);//读入该字/词的所有规则


	//接下来处理开始处理规则
	//以下是创建每一条规则的多音函数链表,循环一次创建一个链表.
	pRule = pCurZiCiRule;
	while(pRule < pCurZiCiRule + nLen)
	{
		//polyFA清零并将当前多音字拷贝到polyFA.Zi_Code  //add by hhh 2011-09-08
		emMemSet(&polyFA, 0 , sizeof(struct PolyFunAsisant));
		polyFA.Zi_Code = nZiCode;

		emMemCpy(nTrueEncode, pRule, 4);	//正确情况下的读音
		pRule += 4 ;   //四个字节
		emMemCpy(nFalseEncode, pRule, 4);    //错误情况下的读音
		pRule += 4;   //四个字节
		emMemCpy(&cFunCount, pRule, sizeof(cFunCount));      //这个值的高三位是本规则的最高优先级,低五位是这条规则有多少个多音函数
		i = cFunCount;
		cFunCount &= 0x1F;  //取出低五位,也就是有多少个多音函数 
		nMaxPro = (i & 0xE0) / 32;  //得到了本条规则的最高优先级 
		++pRule;  //一个字节

		cFunLen = *(pRule++);  //多音函数的存储长度
		//下面开始读取多音函数部分
		polyFA .FunType = *(pRule ++);  //函数类型
		
		pCurPolyFunAlisaLink = pHeadPolyFunAlisaLink;
		pHead = pHeadPolyFunAlisaLink;		

		pTemp = pHead;
		pHead -> next = NULL;
		pHead -> IsAnd = (*(pRule) & 0x80) / 128;   //是否与下一函数是与的关系
		pHead -> IsNotBeforeOperation  = (*(pRule) & 0x40) / 64;  //本函数的值与下1函数运算前是否取反
		polyFA .IsDirectNot = (*(pRule) & 0x20) /32;  //本函数的值是否直接取反
		pHead -> ProbCount = *(pRule)  & 0x07;   //本函数的优先级
		++pRule;

		polyFA .Para1 = *(pRule++);  //参数1     
		polyFA .Para2 = *(pRule++);  //参数2
		i = cFunLen - 4;             //除了头部四个还有就是参数3了
		if( i > 0)
		{
			emMemSet(polyFA .Para3, 0, MAX_POLYFA_PARA3_LEN);
			emMemCpy(polyFA .Para3, pRule, emMin(i,MAX_POLYFA_PARA3_LEN));  //参数3 
		}
		pRule += i;	
		nRes = HandleFun(&polyFA);  //判断一个多音函数
		if(polyFA.IsDirectNot == 1)  //需要直接取反.在加入链表的时候就取反
		{
			if(nRes == 0)
			{
				nRes = -1;
			}
			else 
			{
				nRes = 0;
			}	
		}
		pHead -> IsResultTrue = nRes;  //这个多音函数的最终值 
		while(--cFunCount)
		{
			//polyFA清零并将当前多音字拷贝到polyFA.Zi_Code  //add by hhh 2011-09-08
			emMemSet(&polyFA, 0 , sizeof(struct PolyFunAsisant));
			polyFA.Zi_Code = nZiCode;

			pCurPolyFunAlisaLink++;
			pCur = pCurPolyFunAlisaLink;

			pCur -> next = NULL;

			cFunLen = *(pRule++);
			polyFA .FunType = *(pRule ++);  //函数类型
			pCur -> IsAnd = (*(pRule) & 0x80) / 128;   //是否与下一函数是与的关系
			pCur -> IsNotBeforeOperation  = (*(pRule) & 0x40) / 64;  //本函数的值与下1函数运算前是否取反
			polyFA .IsDirectNot = (*(pRule) & 0x20) /32;  //本函数的值是否直接取反
			pCur -> ProbCount = *(pRule)  & 0x07;   //本函数的优先级
			++pRule;
			polyFA .Para1 = *(pRule++);  //参数1     
			polyFA .Para2 = *(pRule++);  //参数2
			i = cFunLen - 4;             //除了头部四个还有就是参数3了
			if( i > 0)
			{
				emMemSet(polyFA .Para3, 0, MAX_POLYFA_PARA3_LEN);
				emMemCpy(polyFA .Para3, pRule, emMin(i,MAX_POLYFA_PARA3_LEN));  //参数3 
			}
			pRule += i;
			nRes = HandleFun(&polyFA);  //判断一个多音函数
 			if(polyFA.IsDirectNot == 1)  //需要直接取反的在加入链表的时候就取反
			{
				if(nRes == 0)
					nRes = -1;
				else 
					nRes = 0;
			}
			pCur -> IsResultTrue = nRes;
			pTemp -> next = pCur;
			pTemp = pCur;
		}//end while
	
		//至此创建成功了一条规则的多音函数链表
		nRes = RealHandleRule(pHead, nTrueEncode, nFalseEncode, &nMaxPro,nCurOffset,nMaxHanZi);  //真正的处理规则函数.


		if(nRes == 0)  //只要有一条规则满足就跳出
		{
			bReturnValue = TRUE;
			break;
		}
	}//end while


	return bReturnValue;

}




//****************************************************************************************************
//  功能： 处理规则中的多音函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 当前函数为真还是为假. 返回-1为假,0为真
//****************************************************************************************************

emBool emCall HandleFun(PolyFunAsistant*  polyFA)
{
	emBool nRes = -1;


	LOG_StackAddr(__FUNCTION__);

	switch(polyFA -> FunType)
	{

	case WORD_LEN:  //==1
		nRes = HandleFunWordLen(polyFA);
		break;
	case WORD_LEN_GREATE_THAN:  //==2
	    nRes = HandleFunWordLenGreateThan(polyFA);
		break;

	case WORD_LEN_LESS_THAN:  //==3
		nRes = HandleFunWordLenLessThan(polyFA);
		break;
	case MATCH_WORD:  //==4
		nRes = HandleFunMatchWord(polyFA);
		break;

	case MATCH_CHAR: //==5   
		nRes = HandleFunMatchChar(polyFA);
		break;

	case POS: //==6
		nRes = HandleFunPos(polyFA);
		break;

	case REAL_END://==7
		nRes = HandleFunRealEnd(polyFA);
		break;

	case POS_END://==8
		nRes = HandleFunPosEnd(polyFA);
		break;
		
	case WORD_END: //==9
		nRes = HandleFunWordEnd(polyFA);
		break;

	case PRECEDE_VERB://==10
		nRes = HandleFunPrecedeVerb(polyFA);
		break;

	case SUCCEED_VERB:  //==11 
		nRes = HandleFunSucceedVerb(polyFA);
		break;

	case PRECEDE_NOUN:  //==12
		nRes = HandleFunPrecedeNoun(polyFA);
		break;

	case SUCCEED_NOUN: //==13 
		nRes = HandleFunSucceedNoun(polyFA);
		break;

	case WORD_BEGIN_WITH:  //==14
		nRes = HandleFunWordBeginWith(polyFA);
		break;

	case WORD_END_WITH:  //==15  
		nRes = HandleFunWordEndWith(polyFA);
		break;

	case LAST_PUNC: //==16  
		nRes = HandleFunLastPunc(polyFA);
		break;

	case NEXT_PUNC: //==17
		nRes = HandleFunNextPunc(polyFA);
		break;

	default:
		break;
	}
	return nRes;
}

//****************************************************************************************************
//  功能： 处理WordLen函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************
emInt8 emCall HandleFunWordLen(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		return ((g_pTextInfo[nTmpNode/2].Len / 2 == polyFA -> Para2) ? 0: -1);
	}
}

//****************************************************************************************************
//  功能： 处理WordLenGreateThan函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunWordLenGreateThan(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		return ((g_pTextInfo[nTmpNode/2].Len / 2 > polyFA -> Para2) ? 0: -1);
	}
}

//****************************************************************************************************
//  功能： 处理WordLenLessThan函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunWordLenLessThan(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		return ((g_pTextInfo[nTmpNode/2].Len / 2 < polyFA -> Para2) ? 0: -1);
	}
}

//****************************************************************************************************
//  功能： 处理MatchWord函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************


emInt8 emCall HandleFunMatchWord(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;
	emInt i = 0;
	emPByte p1=NULL, p2=NULL;
	emInt8 pPara3[MAX_POLYFA_PARA3_LEN+1];
	emInt  nLen;

	LOG_StackAddr(__FUNCTION__);
	
	nLen = emStrLenA(polyFA ->Para3);

	for(i = polyFA ->Para1; i <= polyFA ->Para2; i++)
	{
		nTmpNode = GetSpecailNode(i);
		if(nTmpNode == NO_FIND_NODE)
		{
			continue;
		}

		emMemCpy(pPara3,polyFA->Para3,nLen);
		pPara3[nLen] = '\0';

		p1 = strtok(pPara3, ",");
		while(p1 != NULL)
		{
			if( emStrLenA(p1)==g_pTextInfo[nTmpNode/2].Len)
			{
				if(!emMemCmp(&g_pText[nTmpNode], p1,g_pTextInfo[nTmpNode/2].Len))
				{
					return 0;
				}
			}
			p2 = strtok(NULL, ",");
			p1 = p2;
		}
	}
	return -1;
}

//****************************************************************************************************
//  功能： 处理MatchChar函数   add by hyl   20110626
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************
emInt8 emCall HandleFunMatchChar(PolyFunAsistant* polyFA)   //通过查看*.rule文件,发现匹配单字
{
	emInt16 curZiIndex, leftRangeIndex,rightRangeIndex;
	emBool bIsFind = emFalse;
	emByte curZi[3],curBiZi[3];
	emInt16 nCount = 0,i,j,nResult;
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nCount = g_nCurTextByteIndex;	
	nTmpNode = g_nCurTextByteIndex;

	//找到当前要处理的多音字在文本中是第几个字（nCount）
	for(i=0;i< g_pTextInfo[g_nCurTextByteIndex/2].Len; i=i+2)
	{
		if( polyFA->Zi_Code != ( g_pText[g_nCurTextByteIndex+i+1]*256 +g_pText[g_nCurTextByteIndex+i] ))  
		{
			nCount += 2;
		}
		else
		{
			break;						
		}
	}

	curZiIndex = nCount/2;										//得到《当前字》的索引
	leftRangeIndex  = curZiIndex + polyFA->Para1;				//得到《范围最左边》的索引
	rightRangeIndex = curZiIndex + polyFA->Para2;				//得到《范围最右边》的索引
	if( leftRangeIndex  < 0)						leftRangeIndex = 0;
	if( leftRangeIndex > (g_nLastTextByteIndex/2-1))	return -1;			//都没找到：假
	if( rightRangeIndex > (g_nLastTextByteIndex/2-1))	rightRangeIndex = g_nLastTextByteIndex/2-1;

	//判断《范围内的字》是否在polyFA->Para3之中
	for(i= leftRangeIndex; i<=rightRangeIndex; i++)
	{
		curZi[0] = g_pText[2*i];
		curZi[1] = g_pText[2*i+1];
		curZi[2] = 0;
		for(j=0;j<emStrLenA(polyFA->Para3);j=j+2)
		{
			curBiZi[0]=*( polyFA->Para3+j);
			curBiZi[1]=*( polyFA->Para3 + j + 1);
			curBiZi[2]=0;
			nResult = emMemCmp(curZi,curBiZi,2);
			if( nResult == 0)
			{
				return 0;		//找到了：真
			}
		}
	}
	return -1;			//都没找到：假
}



//****************************************************************************************************
//  功能： 处理Pos函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunPos(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		//return ((NeedConvertPos(g_pTextInfo[nTmpNode/2].Pos) == polyFA -> Para2) ? 0: -1); 
		//modified by hhh 20110917

		if( NeedConvertPos(g_pTextInfo[nTmpNode/2].Pos) == polyFA -> Para2
			|| g_pTextInfo[nTmpNode/2].Pos == polyFA -> Para2 )
		{
			return 0;
		}
		else
		{
			return -1;
		}		
	}
}

//****************************************************************************************************
//  功能： 处理RealEnd函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunRealEnd(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		g_nNextTextByteIndex = nTmpNode + g_pTextInfo[nTmpNode/2].Len;
		return ((g_pTextInfo[g_nNextTextByteIndex/2].Len != 0) ? -1: 0);  //如果存在最后一个词,那么就不是next就不是NULL
	}
}

//****************************************************************************************************
//  功能： 处理PosEnd函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunPosEnd(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetLastNode();

	//return ((NeedConvertPos(g_pTextInfo[nTmpNode/2].Pos) == polyFA -> Para1) ? 0: -1); 
	//modified by hhh 20110917
	if( NeedConvertPos(g_pTextInfo[nTmpNode/2].Pos) == polyFA -> Para1
		|| g_pTextInfo[nTmpNode/2].Pos == polyFA -> Para1 )
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

//****************************************************************************************************
//  功能： 处理WordEnd函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunWordEnd(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;
	emPByte p1=NULL, p2=NULL;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetLastNode();
	p1 = strtok(polyFA ->Para3, ",");
	while(p1 != NULL)
	{
		if( emStrLenA(p1)==g_pTextInfo[nTmpNode/2].Len)
		{
			if(!emMemCmp(&g_pText[nTmpNode], p1, g_pTextInfo[nTmpNode/2].Len))
			{
				return 0;
			}
		}
		p2 = strtok(NULL, ",");
		p1 = p2;
	} 
	return -1;
}

//****************************************************************************************************
//  功能： 处理PrecedeVerb函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunPrecedeVerb(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	g_nNextTextByteIndex = nTmpNode + g_pTextInfo[nTmpNode/2].Len;
	if(g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)
	{
		return -1;
	}

	//如果下一词的词性是动词
	if(g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_v || g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_vd || g_pTextInfo[g_nNextTextByteIndex/2].Pos  == POS_CODE_vn) 
	{
		return 0;
	}
	return -1;

}

//****************************************************************************************************
//  功能： 处理SucceedVerb函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunSucceedVerb(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;
	emUInt16 nBeforeNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		nBeforeNode = GetSpecailNode(polyFA -> Para1 - 1 );
		g_nP2TextByteIndex = nBeforeNode;

		if(g_pTextInfo[g_nP2TextByteIndex/2].Len != 0)
		{
			//如果前一词的词性是动词
			if(g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_v || g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_vd || g_pTextInfo[g_nP2TextByteIndex/2].Pos  == POS_CODE_vn) 
			{
				return 0;
			}
		}
		return -1;
	}
}

//****************************************************************************************************
//  功能： 处理PrecedeNoun函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunPrecedeNoun(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
		if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	g_nNextTextByteIndex = nTmpNode + g_pTextInfo[nTmpNode/2].Len;
	if(g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)
	{
		return -1;
	}

	//如果下一词的词性是动词
	if(g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_n || g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_nr || g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_ns || 
	   g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_nt|| g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_nz)
	{
		return 0;
	}
	return -1;	
}


//****************************************************************************************************
//  功能： 处理SucceedNoun函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunSucceedNoun(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;
	emUInt16 nBeforeNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		nBeforeNode = GetSpecailNode(polyFA -> Para1 - 1 );
		g_nP2TextByteIndex = nBeforeNode;

		if(g_pTextInfo[g_nP2TextByteIndex/2].Len != 0)
		{
			//如果前一词的词性是名词
			if(g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_n || g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_nr || g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_ns ||
			   g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_nt|| g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_nz) 
			{
				 return 0;
			}
		}
		return -1;
	}
}

//****************************************************************************************************
//  功能： 处理WordBeginWith函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunWordBeginWith(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;
	emPByte p1=NULL, p2=NULL;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		p1 = strtok(polyFA ->Para3, ",");
		while(p1 != NULL)
		{
			if( emStrLenA(p1)<=g_pTextInfo[nTmpNode/2].Len)
			{
				if(!emMemCmp(&g_pText[nTmpNode], p1, emStrLenA(p1)))
				{
					return 0;
				}
			}
			p2 = strtok(NULL, ",");
			p1 = p2;
		} 
		return -1;		
	}
}


//****************************************************************************************************
//  功能： 处理WordEndWith函数   add by songkai
//  参数1:辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************
emInt8 emCall HandleFunWordEndWith(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;
	emPByte p1 = NULL, p2 = NULL;
	emInt16 i = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(polyFA -> Para1);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		p1 = strtok(polyFA -> Para3, ",");
		while(p1 != NULL)
		{
			i = g_pTextInfo[nTmpNode/2].Len - emStrLenA(p1);
			
			if( emStrLenA(p1)<=g_pTextInfo[nTmpNode/2].Len)
			{
				if(!emMemCmp(p1, &g_pText[nTmpNode+i], emStrLenA(p1)))  //如果是以其结尾的
				{
					return 0;
				}
			}
			p2 = strtok(NULL, ",");
			p1 = p2;
		}
		return -1;
	}
}



//****************************************************************************************************
//  功能： 处理LastPunc函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunLastPunc(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	//此函数不再用，多音规则里不要再写此函数
	return -1;
	
}

//****************************************************************************************************
//  功能： 处理NextPun函数   add by songkai
//  参数1: 辅助结构体指针
//  返回值: 函数结果为真(0)还是假(-1)
//****************************************************************************************************

emInt8 emCall HandleFunNextPunc(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = GetSpecailNode(0);
	if(nTmpNode == NO_FIND_NODE)
	{
		return -1;
	}
	else
	{
		if( !emMemCmp(g_pTextInfo[nTmpNode/2].nBiaoDian,polyFA -> Para3,2))
			return 0;
		else
			return -1;
		
	}
}


//****************************************************************************************************
//  功能： 移动到指定节点   add by songkai
//  参数1: 相对于当前节点移动多少个节点
//  返回值: 如果存在返回节点首字索引,不存在返回0xff
//****************************************************************************************************
emUInt8 emCall GetSpecailNode(emInt16 nArg)
{
	emInt16 nTmpNode , nTmpNode2 ;
	emInt16 nCount = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = g_nCurTextByteIndex; //得到当前节点指针
	if(nArg == 0)
	{
		return nTmpNode;
	}
	else if(nArg > 0)  //后移到指定节点
	{
			while(nArg)  
			{
				g_nNextTextByteIndex = nTmpNode + g_pTextInfo[nTmpNode/2].Len;
				if(g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)   //不存在指定节点
				{
					return NO_FIND_NODE;
				}
				--nArg;

				nTmpNode = g_nNextTextByteIndex;

			}//循环结束后,nTmpNode移动到指定节点
			return nTmpNode;
	}
	else   //前移到指定节点
	{
		nTmpNode2 = 0;
		while(1)  //首先计算当前节点前面有几个节点
		{
			if(nTmpNode2 == nTmpNode)
			{
				break;	
			}
			g_nNextTextByteIndex = nTmpNode2 + g_pTextInfo[nTmpNode2/2].Len;
			nTmpNode2 = g_nNextTextByteIndex;
			nCount++;   //记录节点数
		}
		if(nCount - emAbs(nArg) ==0)  //如果前移到头
		{
			return  0;
		}
		else if(nCount - emAbs(nArg) < 0)  //不存在
		{
			return NO_FIND_NODE;		
		}
		else
		{
			nCount -= emAbs(nArg); //从头点要后移几个节点
			nTmpNode2 = 0;  //指向头结点
			while(nCount)
			{
				--nCount;

				g_nNextTextByteIndex = nTmpNode2 + g_pTextInfo[nTmpNode2/2].Len;
				nTmpNode2 = g_nNextTextByteIndex;
		
			}//循环结束后,nTmpNode2移动到指定节点
			return nTmpNode2;
		}
	}
}


//****************************************************************************************************
//  功能： 移动到最后一个节点   add by songkai
//  返回值: 返回节点首字索引
//****************************************************************************************************
emUInt8 emCall GetLastNode()
{
	emUInt16 nTmpNode = 0,nNextNode=0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = g_nCurTextByteIndex; //得到当前节点指针
	nNextNode = nTmpNode +g_pTextInfo[nTmpNode/2].Len;
	while( g_pTextInfo[nNextNode/2].Len != 0 )   // 那兴宇修改，2010-12-13
	{
		nTmpNode = nNextNode;
		nNextNode = nTmpNode +g_pTextInfo[nTmpNode/2].Len;
	}
	return nTmpNode;
}

//****************************************************************************************************
//  功能： 处理规则中的多音函数链表   add by songkai  //找到读音后跳出,把读音赋值给当前的全局链表节点的拼音字段
//  返回值: 当前规则为真还是为假. 返回-1为假,0为真
//****************************************************************************************************
emBool emCall RealHandleRule(PolyFunAlisa *pHead,		//  参数: 多音函数链表头指针
							 emUInt16 *nTrueEncode,		//  参数: 规则为真的时候的读音地址
							 emUInt16 *nFalseEncode,		//  参数: 规则为假的时候的读音地址
							 emByte* nMaxPro,			//  参数: 规则的最高优先级
							emInt16 nCurOffset,			// 要处理的是从g_curTextCsItem中的第几个字开始的字或词
							emInt16 nMaxHanZi)			// 要处理的是 1个字 还是 2个字

{
	PolyFunAlisa *pTmp = NULL, *p1 = NULL, *p2 = NULL, *pFor = NULL, *pCurHead=NULL;
	emInt32 nPinYinForce;
	emInt8 nMax = *nMaxPro; //最大优先级

	LOG_StackAddr(__FUNCTION__);

	//只有一个多音函数节点
	if(pHead ->next == NULL)
	{
		if(pHead -> IsResultTrue == 0)  //多音函数结果是TRUE,也就是这条规则是TRUE
		{
			nPinYinForce = g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] & 0x8000;
			
			if( nPinYinForce != 0x8000 ) //默认：非强制拼音
			{
				g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset]= *nTrueEncode; //把规则的读音给它
				if( nMaxHanZi == 2)			//2字词
				{
					g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset + 1] = *(nTrueEncode + 1); //把规则的读音给它
				}
			}
		}
		else  //多音函数结果是FALSE,也就是这条规则是FALSE
		{
			nPinYinForce = g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] & 0x8000;
			if( nPinYinForce != 0x8000 ) //默认：非强制拼音
			{
				if(*nFalseEncode == 0)
				{
					return -1;  //这条规则不成立
				}
				g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] = *nFalseEncode; //把规则的读音给它
				if( nMaxHanZi == 2)			//2字词
				{
					g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset+1] = *(nFalseEncode + 1); //把规则的读音给它
				}
			}
		}
		return 0;
	}

	//有多个多音函数节点
	pTmp = pHead;
	pCurHead = pHead;
	do
	{
		pFor = pCurHead;
		while(pTmp -> next != NULL)  //不是最后一个节点
		{
			while(pTmp -> next != NULL && pTmp -> ProbCount != nMax) //在多音函数链表中查找最高优先级的
			{
				pFor  = pTmp;  //指向优先级最高的前一个节点
				pTmp = pTmp -> next;
			}//end while
			if(pTmp -> ProbCount == nMax)  //找到最高优先级的
			{
				p1 = pTmp;
				p2 = pTmp -> next;
				if(p2 != NULL)
				{
					if(p1 -> IsAnd == 1)  //与的情况
					{  
						if(p2 -> IsResultTrue == 0)
						{
							p2 -> IsResultTrue = p1 -> IsResultTrue;//函数值
						}
					}
					else
					{
						if(p2 -> IsResultTrue == -1)  //或的情况
						{
							p2 -> IsResultTrue = p1 -> IsResultTrue;//函数值	
						}
					}
					if(p1 == pFor)
					{
						pFor = p2;
						pCurHead = pFor;
					}
					else
					{
						pFor ->next = p2;
					}
					
					pTmp = p2;//最后只剩下pTmp节点
				}
			}//end if
		}//end while
		--nMax; //下次循环递减
		pTmp = pCurHead;

	}while(pTmp -> next != NULL);//end do
//最后只剩下了pTmp节点了
	if(pTmp -> IsResultTrue == 0)  //为真的情况下
	{
		nPinYinForce =  g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset]& 0x8000;
		if( nPinYinForce != 0x8000 ) //默认：非强制拼音
		{
			 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] = *nTrueEncode; //把规则的读音给它
			if( nMaxHanZi == 2)			//2字词
			{
				 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset+1] = *(nTrueEncode + 1); //把规则的读音给它
			}
		}
		return 0;
	}
	else
	{
		nPinYinForce =  g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] & 0x8000;
		if( nPinYinForce != 0x8000 ) //默认：非强制拼音
		{
			if(*nFalseEncode == 0)
			{
				return -1;  //这条规则没有为FALSE情况下的读音.
			}
			 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] = *nFalseEncode; //把规则的读音给它
			if( nMaxHanZi == 2)			//2字词
			{
				 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset+1] = *(nFalseEncode + 1); //把规则的读音给它
			}
		}
		return 0;
	}
}





//****************************************************************************************************
//  功能： 处理Pos函数和PosEnd函数时的特殊处理   add by songkai
//  参数1: 当前节点的pos值
//  返回值: 函数结果为真(0)还是假(-1)
//  说明： 
//对于 《多音规则库》中的《多音函数》的《POS(-1,"nr")》和《POSEnd("n")》这2类需特殊处理：
//当这2类的词性是以下左边的词性时， 它可以替换成右边的1个或2个词性。  （因为多音规则库中没有右边的词性，但词典中有）
//需达成的效果：   POS(-1,"nr")     ---》      （ POS(-1,"nr") | POS(-1,"nrf")| POS(-1,"nrg") ）
//                 POSEnd("n")      ---》      （ POSEnd("n")  | POSEnd("nf") | POSEnd("nh")  ）

//左边	  右边1     右边2
//e  16  ：  eg  17
//i  22  ：  ic  25   im   27
//j  30  ：  jm  33
//l  37  ：  lc  40   lm   42
//n  48  ：  nf  50   nh   52
//nr 53  ：  nrf 54   nrg  55  
//nz 59  ：  nzg 60
//v  94  ：  vv  104
//w 106  ：  wy  118
//****************************************************************************************************

emUInt8 emCall NeedConvertPos(emUInt8 inPos)
{
	emUInt8 rePos;

	LOG_StackAddr(__FUNCTION__);

	switch(inPos)
	{
	case POS_CODE_nrf:
	case POS_CODE_nrg:
		rePos = POS_CODE_nr;
		break;
	case POS_CODE_nh:
	case POS_CODE_nf:
		rePos = POS_CODE_n;
		break;
	case POS_CODE_nzg:
		rePos = POS_CODE_nz;
		break;
	case POS_CODE_vv:
		rePos = POS_CODE_v;
		break;
	case POS_CODE_wy:
		rePos = POS_CODE_w;
		break;
	case POS_CODE_ic:
	case POS_CODE_im:
		rePos = POS_CODE_i;
		break;
	case POS_CODE_eg:
		rePos = POS_CODE_e;
		break;
	case POS_CODE_jm:
		rePos = POS_CODE_j;
		break;
	case POS_CODE_lc:
	case POS_CODE_lm:
		rePos = POS_CODE_l;
		break;
	default:
		rePos = inPos;
		break;
	}
	return rePos;
}
