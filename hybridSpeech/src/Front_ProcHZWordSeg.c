#include "emPCH.h"

#include "Front_ProcHZWordSeg.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <jni.h>
#include "jnirelated.h"


emInt16 g_MaxLineOfTableSurname	;				//姓氏表的最大行数

emPByte g_pXing_Table ;							//将ROM里的《姓氏表》读入到内存里
emPByte g_pBigram_Table ;						//将ROM里的《二元文发表》的固定Pos1转移到126个不同的Pos2的值一次性取入到内存，占0.252K字节
struct WordBuffer *g_vecWordBuffer ;			//分词模块需要  存储每个字的所有词条，最多50个字
emInt16 *g_pHeadWordSegPinYin;	
emInt16 *g_pCurWordSegPinYin;	

int MY_SIZE = 10;
void print_g_pTextInfo()
{
    //MAX_HANZI_COUNT_OF_LINK_NEED_HEAP
    for(int i = 0; i < MY_SIZE; i++)
    {
        printf("%d: %d, %d, %d, %d, %d; %c, %c\n", i,
               g_pTextInfo[i].Pos,
               g_pTextInfo[i].Len,
               g_pTextInfo[i].TextType,
               g_pTextInfo[i].BorderType,
               g_pTextInfo[i].nBiaoDian[0],
               g_pTextInfo[i].nBiaoDian[1]);
    }
    printf("\n");
}

void print_g_pTextPinYinCode()
{
    //MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2
    for(int i = 0; i < MY_SIZE; i++)
    {
        //printf("%d ", *(emInt16*)(g_pTextPinYinCode+2*i));
        printf("%d ", g_pTextPinYinCode[i]);
    }
    printf("\n");
}

void print_g_pText()
{
    printf("%s\n", g_pText);
    int len = strlen(g_pText);
    for(int i = 0; i < len; i++)
    {
        printf("%%%02X", (int)g_pText[i]);
    }
    printf("\n");
}

void emCall WordSeg_third()
{
    // Get the text to be dealt with, format: "%C4%E3%BA%C3"
    char strGbk[300] = {0};
    int len = strlen(g_pText);
    for(int i = 0; i < len; i++)
    {
        sprintf(strGbk+3*i, "%%%02X", (int)g_pText[i]);
        //printf("%02X", (int)g_pText[i]);
    }
    printf("gbk_segmnt: %s\n", strGbk);

    long long t1 = currentTimeInMilliseconds();
    const char *strRet = jniWordSegment(strGbk);
    long long t2 = currentTimeInMilliseconds();
    printf("jniWordSegment time: %d\n", t2-t1);
    printf("result: %s\n", strRet);

    // 6,48,2,36,2,48,
    int dist[MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2] = {0};
    const char *split = ",";
    char *left = NULL;
    int idx = 0;

    char *p = strtok_r(strRet, split, &left);
    while(p != NULL)
    {
        dist[idx] = atoi(p);
        idx++;
        if(idx % 2 == 0)
        {
            idx += dist[idx-2]-2;
        }
        p = strtok_r(NULL, split, &left);
    }

    for(int i = 0; i < MAX_HANZI_COUNT_OF_LINK_NEED_HEAP; i++)
    {
        //g_pTextInfo[i].Pos = 0;
        g_pTextInfo[i].Pos = dist[2*i+1];
        g_pTextInfo[i].Len = dist[2*i];
        if(dist[2*i] == 0)
        {
            g_pTextInfo[i].TextType = 0;
            g_pTextInfo[i].BorderType = 0;
        }
        else
        {
            g_pTextInfo[i].TextType = 1;
            g_pTextInfo[i].BorderType = 1;
        }
        g_pTextInfo[i].nBiaoDian[0] = 0;
        g_pTextInfo[i].nBiaoDian[1] = 0;
    }

//  print_g_pTextInfo();
//  print_g_pTextPinYinCode();
}


//*****************************************************************************************************
//函数功能： 分词主模块
//*****************************************************************************************************
void emCall WordSeg()
{
//  print_g_pText();
//  print_g_pTextInfo();
//  print_g_pTextPinYinCode();

    emInt16  nPrevPOS;						// 前1节点的词性pos索引
    emInt16  nNextPOS;						// 后1节点的词性pos索引
	emInt16 offsetLine,nCurLen;
	emInt32 nPinYin;

	emByte	firstZi[4];
	emPByte pCurXing_Table ;	

	LOG_StackAddr(__FUNCTION__);

	//初始化《分词模块》，申请空间
	WordSegInit();

	nPrevPOS = POS_CODE_w;						//整句的前1节点的词性pos索引； 是“w”即标点	
    nNextPOS = 0;	

	fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_SurnameRule,  0);					
	fFrontRead(&g_MaxLineOfTableSurname, 1 , 2, g_hTTS->fResFrontMain);	//读入《姓氏表》的最大行数

	
	if( g_pTextInfo[0].Len != 0 )
	{
		//若有控制标记[r1]或[r2]，句首字强制读成姓氏拼音
		if( g_hTTS->m_ControlSwitch.m_bXingShi == emTTS_USE_XINGSHI_JUSHOU_OPEN ||  g_hTTS->m_ControlSwitch.m_bXingShi == emTTS_USE_XINGSHI_AFTER_OPEN )  
		{

			//一次性将整个《姓氏表》读入到内存
#if DEBUG_LOG_SWITCH_HEAP
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"整个姓氏表：《分词模块  》");			//开辟内存空间
#else
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);			//开辟内存空间
#endif
			fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_SurnameRule+BYTE_NO_OF_TABLE_SURNAME_ONE_LINE, 0 );					
			fFrontRead(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE , 1, g_hTTS->fResFrontMain);	//一次性读入整个表

			emMemCpy( firstZi, &g_pText[0], 2);
			firstZi[2] = 0;
			firstZi[3] = 0;
			offsetLine = ErFenSearchTwo_FromRAM( firstZi, 0, g_pXing_Table, 1, g_MaxLineOfTableSurname ,BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);	//二分法搜索

			if( offsetLine != g_MaxLineOfTableSurname)		//找到与第1个字相同的姓
			{
				//定位到已找到行的拼音
				pCurXing_Table = g_pXing_Table + offsetLine*BYTE_NO_OF_TABLE_SURNAME_ONE_LINE + 6;
				nPinYin = ( *(pCurXing_Table+1))*256 +( *(pCurXing_Table));
				g_pTextPinYinCode[0]  = nPinYin;
				if( g_pTextPinYinCode[0] != 0)
				{
					g_pTextPinYinCode[0] = g_pTextPinYinCode[0] | 0x8000;		//强制拼音
				}
			}

			//释放《姓氏表》的内存
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"整个姓氏表：《分词模块  》");
#else
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);
#endif

		}

		//若是[r2](仅紧跟句的句首强制读成姓氏)，则需将g_hTTS->m_ControlSwitch.m_bXingShi开关关闭。  注意：若是[r1]则不需要关
		if( g_hTTS->m_ControlSwitch.m_bXingShi == emTTS_USE_XINGSHI_AFTER_OPEN )
		{
			g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_CLOSE;
		}
	}


	g_nCurTextByteIndex = 0;
	while( g_pTextInfo[g_nCurTextByteIndex/2].Len != 0 )
	{
		nCurLen = g_pTextInfo[g_nCurTextByteIndex/2].Len;

		//若本节点 是文本类型 且 无词性 ， 则进行 分词
		if( g_pTextInfo[g_nCurTextByteIndex/2].TextType == TextCsItem_TextType_HANZI &&  g_pTextInfo[g_nCurTextByteIndex/2].Pos == 0 )
		{
			g_nNextTextByteIndex = g_nCurTextByteIndex + g_pTextInfo[g_nCurTextByteIndex/2].Len;
			if( g_pTextInfo[g_nNextTextByteIndex/2].Len != 0 )
			{
				nNextPOS = g_pTextInfo[g_nNextTextByteIndex/2].Pos;				
			}
			else
			{
				nNextPOS = POS_CODE_w;					//整句的后1节点的词性pos索引； 是“w”即标点	

			}	
#if DEBUG_LOG_SWITCH_RHYTHM_MIDDLE
			if( g_pTextInfo[g_nNextTextByteIndex/2].Len == 0 )
			{
				Print_To_Rhythm("log/日志_韵律.log", "a", 0,"原句：");
			}
#endif


			//一次性将整个《姓氏表》读入到内存
#if DEBUG_LOG_SWITCH_HEAP
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"整个姓氏表：《分词模块  》");			//开辟内存空间
#else
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);			//开辟内存空间
#endif

			fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_SurnameRule,  0);					
			fFrontRead(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE , 1, g_hTTS->fResFrontMain);	//一次性读入整个表


			//构造 当前节点（未拆开的当前节点 或 拆开后的前节点）的所有字的所有词条（查词典）
			MatchWordsOfCurNode();

			//增加用户词典资源中的词条
			if( g_Res.offset_DictCnUser )		//如果此表存在
				AddUserDictWord();				//2012-11-26 hyl

			//释放《姓氏表》的内存
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"整个姓氏表：《分词模块  》");
#else
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);
#endif

#if DEBUG_LOG_SWITCH_TIME
			myTimeTrace_For_Debug(1,(emByte *)"分词--构造词条", 0);
#endif

			//对 当前节点进行二元文法分析，获取最佳分词路线
			AmbiguityAnalysis( g_pTextInfo[g_nCurTextByteIndex/2].Len/2 ,nPrevPOS, nNextPOS);

#if DEBUG_LOG_SWITCH_TIME
			myTimeTrace_For_Debug(1,(emByte *)"分词--最佳路线", 0);
#endif

			//根据最佳分词路线，将汉字链表中的当前节点拆分成多个节点，并将当前节点指向拆分后的最后1个节点
			SegCurNodeToManyNode( g_pTextInfo[g_nCurTextByteIndex/2].Len/2 );

#if DEBUG_LOG_SWITCH_TIME
			myTimeTrace_For_Debug(1,(emByte *)"分词--拆分节点", 0);
#endif

#if DEBUG_LOG_SWITCH_RHYTHM_MIDDLE
			if( g_pTextInfo[g_nNextTextByteIndex/2].Len == 0 )
			{
				Print_To_Rhythm("log/日志_韵律.log", "a", 1,"分词：");
			}
#endif
			
		}

		
		
		//指向下1节点
		g_nCurTextByteIndex += nCurLen;

		//将后词性赋值给下一个前词性
		nPrevPOS = nNextPOS;
	}


	//若是单字句，且分词出来的词性是nrf，则改成词性g，且去除拼音		hyl  2012-04-07
	if( g_pTextInfo[0].Pos == POS_CODE_nrf && g_pTextInfo[0].Len == 2 &&  g_pTextInfo[1].Len == 0 )
	{
		g_pTextInfo[0].Pos = POS_CODE_g;

		if( ( (g_pTextPinYinCode[0]) & 0x8000 ) != 0x8000)	//不是强制拼音
		{
			g_pTextPinYinCode[0] = 0;
			g_pTextPinYinCode[1] = 0;
		}
	}

	//逆初始化《分词模块》，释放空间
	WordSegUnInit();

//  print_g_pText();
//  print_g_pTextInfo();
//  print_g_pTextPinYinCode();
}

void emCall WordSegInit()
{
	emInt16 i;


	LOG_StackAddr(__FUNCTION__);
	


	//申请每个字的词条空间
#if DEBUG_LOG_SWITCH_HEAP
	g_vecWordBuffer = (struct WordBuffer *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ), "每字词条空间：《分词模块  》")  ;	//分词模块需要  存储每个字的所有词条
#else
	g_vecWordBuffer = (struct WordBuffer *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ))  ;	//分词模块需要  存储每个字的所有词条
#endif	
	for( i=0; i<MAX_HANZI_COUNT_OF_LINK; i++)
	{
		(g_vecWordBuffer+i)->nItemCount = 0;
	}

	//申请多音词条的拼音空间
#if DEBUG_LOG_SWITCH_HEAP
	g_pHeadWordSegPinYin = (emInt16 *)emHeap_AllocZero(MAX_COUNT_WordSegPinYin , "多音词条拼音：《分词模块  》");	
#else
	g_pHeadWordSegPinYin = (emInt16 *)emHeap_AllocZero(MAX_COUNT_WordSegPinYin);	
#endif
	g_pCurWordSegPinYin = g_pHeadWordSegPinYin;
	
}

void emCall WordSegUnInit()
{

	//释放多音词条的拼音空间
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_pHeadWordSegPinYin, MAX_COUNT_WordSegPinYin , "多音词条拼音：《分词模块  》");	
#else
	emHeap_Free(g_pHeadWordSegPinYin, MAX_COUNT_WordSegPinYin);	
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_vecWordBuffer, MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ), "每字词条空间：《分词模块  》" );
#else
	emHeap_Free(g_vecWordBuffer, MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ));
#endif


	

}









//*****************************************************************************************************
//函数功能：对 当前节点进行二元文法分析，获取最佳分词路线
//*****************************************************************************************************
void emCall AmbiguityAnalysis(emUInt8 nMaxOfHanZi,				//当前节点的汉字个数			
							  emInt16  iPrevPOSIndex,			// 前1节点的词性pos索引
							  emInt16  iNextPOSIndex)			// 后1节点的词性pos索引

{
	emInt16  i, j,idxNode,idxFstItem, iItemLength, idxSndItem, iNextItemLength, idxNextNode;

	emInt32  dExpense = MIN_EXPENSE	;			// 整句的expense， 极小值
    emInt16  iPrevNode = -1;					// 整句结尾的前1个节点
    emInt16  iPrevItem = -1;					// 整句结尾的前1词条
	emInt32  dValue;		

	struct WordItem t1Item;
	emInt16 nBigramVal;
	emInt32 nAllocSize;

	LOG_StackAddr(__FUNCTION__);

	//将《二元文法表》一次性取入到内存
	nAllocSize = MaxLine_Of_OnePos_Of_TableBigram*2;
#if DEBUG_LOG_SWITCH_HEAP
	g_pBigram_Table = (emPByte)emHeap_AllocZero( nAllocSize , "二元文法表：《分词模块  》");			//开辟内存空间
#else
	g_pBigram_Table = (emPByte)emHeap_AllocZero( nAllocSize);			//开辟内存空间
#endif
	

	//将每个节点的每个词条的nExpense赋值为极小值:MIN_EXPENSE
	for(i = 0 ; i <nMaxOfHanZi;i++)
	{
		for (j = 0; j < g_vecWordBuffer[i].nItemCount; j++)
		{
			g_vecWordBuffer[i].item[j].nExpense = MIN_EXPENSE;
		}
	}


	//初始化第1个节点的所有词条的nPrevItem，nPrevNode，nExpense；（nExpense = 标点"w"转移到本词条的二元文法值 + 本词条词频比ratio）
	ReadBigramMoreValue( iPrevPOSIndex );
	for (j = 0; j < g_vecWordBuffer[0].nItemCount; j++)
    {
		g_vecWordBuffer[0].item[j].nPrevItem = 0;  
        g_vecWordBuffer[0].item[j].nPrevNode = -1; 

		//获取二元文法值
		nBigramVal = GetBigramOneValue( g_vecWordBuffer[0].item[j].nPosIndex );


		g_vecWordBuffer[0].item[j].nExpense = nBigramVal + g_vecWordBuffer[0].item[j].nRatio; 
    }

	//以下三重for循环：通过比较dValue和nExpense值得到最佳的分词路线  
	//		1. nExpense和dValue一般都是负值，开销nExpense越接近于0值，转移路线越佳。
	//		2. 若从X1词条转移到Y词条计算出来的Y词条的nExpense是aa，但如果从X2词条转移到Y词条的dValue的值bb大于aa的话，即bb的值比aa的值更接近于0值，
	//         则说明从X2词条转移到Y词条优于从X1词条转移到Y词条，则将Y词条的nExpense值由aa改成bb，Y词条的前1词条由指向X1词条改成指向X2词条）

	//第1重for循环： 搜索句子中的每个结点；  当前节点idxNode：  从 0 至 句中含有的节点总数 （即字总数）
    for ( idxNode = 0; idxNode < nMaxOfHanZi; idxNode++)
    {

		//第2重for循环： 搜索当前节点的每个词条；  当前词条idxFstItem： 从 0 至 本节点的词条总数
		for (idxFstItem = 0; idxFstItem < g_vecWordBuffer[idxNode].nItemCount; idxFstItem++)
        {
			iItemLength = g_vecWordBuffer[idxNode].item[idxFstItem].nCharNum ;					//得到本词条的词长

			idxNextNode = idxNode+iItemLength;

            if (idxNextNode < nMaxOfHanZi )  //本词条还没有到达句尾
            {   
				//第3重for循环： 搜索第idxNode个节点中第idxFstItem个词条对应的下1节点中的所有词条； 下1词条idxSndItem ：从 0 至 下1节点的词条总数
				//设当前是第[idxNode]个节点，当前第[idxFstItem]个词条的词长为iItemLength；则下1节点为第[idxNextNode]个节点
				ReadBigramMoreValue( g_vecWordBuffer[idxNode].item[idxFstItem].nPosIndex );
				for ( idxSndItem = 0; idxSndItem < g_vecWordBuffer[idxNextNode].nItemCount; idxSndItem++)
                {                    
					iNextItemLength = g_vecWordBuffer[idxNextNode].item[idxSndItem].nCharNum;					//得到：下1词条的词长
   
					//获取二元文法值
					nBigramVal = GetBigramOneValue( g_vecWordBuffer[idxNextNode].item[idxSndItem].nPosIndex );

					//dValue = 当前词条的nExpense值 + 当前词条转移到下1词条的二元文法转移概率 + 下1词条的词频比ratio值
                    dValue = g_vecWordBuffer[idxNode].item[idxFstItem].nExpense + nBigramVal + g_vecWordBuffer[idxNextNode].item[idxSndItem].nRatio;

					//若 dValue值 > 下1词条的nExpense值  （表示：本转移到下1词条的路线优于之前转移到下1词条的路线）
                    if (dValue > g_vecWordBuffer[idxNextNode].item[idxSndItem].nExpense)
                    {
                        g_vecWordBuffer[idxNextNode].item[idxSndItem].nExpense  = dValue;		//将dValue值赋值给下1词条的nExpense值
                        g_vecWordBuffer[idxNextNode].item[idxSndItem].nPrevNode = idxNode;	//将下1词条的前节点指向当前节点
                        g_vecWordBuffer[idxNextNode].item[idxSndItem].nPrevItem = idxFstItem;	//将下1词条的前词条指向当前词条
                    }
                }
            }
            else		//本词条刚好到达了句尾  （之前已有控制，不会超过句尾）
            {   
				ReadBigramMoreValue( 0 );
				//获取二元文法值
				nBigramVal = GetBigramOneValue( iNextPOSIndex );
				 
				//dValue = 当前词条的nExpense值 + 当前词条转移到下1词条的二元文法转移概率   （注意：这里的dValue值不包含：下1词条的词频比ratio值）
                dValue = g_vecWordBuffer[idxNode].item[idxFstItem].nExpense + nBigramVal;
                if (dValue > dExpense)  //本转移到句尾的路线优于之前转移到句尾的路线
                {
                    dExpense  = dValue;			// 将dValue值赋值给整句结尾的dExpense值  
                    iPrevNode = idxNode;		// 将整句结尾的前节点指向当前节点
                    iPrevItem = idxFstItem;		// 将整句结尾的前词条指向当前词条
                }
            }
        }
    }

	//从最后一个节点向前反向搜索，将最佳分词路线中对应的词条移至本结点第1个词条的位置  （即对应词条与本节点的第1个词条互换位置）
	
    while (iPrevNode != 255)
    {
        //对应词条与本节点的第1个词条互换位置
		t1Item = g_vecWordBuffer[iPrevNode].item[iPrevItem];
		g_vecWordBuffer[iPrevNode].item[iPrevItem] = g_vecWordBuffer[iPrevNode].item[0];
		g_vecWordBuffer[iPrevNode].item[0] = t1Item;

        iPrevItem = g_vecWordBuffer[iPrevNode].item[0].nPrevItem;
        iPrevNode = g_vecWordBuffer[iPrevNode].item[0].nPrevNode;
    }

	//将《二元文法表》的内存释放
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_pBigram_Table,nAllocSize, "二元文法表：《分词模块  》" );
#else
	emHeap_Free(g_pBigram_Table,nAllocSize );
#endif

}


//*****************************************************************************************************
//函数功能：根据最佳分词路线，将汉字链表中的当前节点拆分成多个节点，并将当前节点指向拆分后的最后1个节点
//*****************************************************************************************************
void emCall SegCurNodeToManyNode(emUInt8 nMaxOfHanZi)
{
	emInt16 nLen;
	emInt16 nNode=0;
	emInt16 i,nPinYin;
	emByte nTempBiaoDian[2];

	LOG_StackAddr(__FUNCTION__);

	nTempBiaoDian[0] = g_pTextInfo[g_nCurTextByteIndex/2].nBiaoDian[0];
	nTempBiaoDian[1] = g_pTextInfo[g_nCurTextByteIndex/2].nBiaoDian[1];

	while( nNode< nMaxOfHanZi)
	{
		nLen = (g_vecWordBuffer[nNode].item[0].nCharNum)*2;		

		g_pTextInfo[g_nCurTextByteIndex/2+nNode].BorderType = BORDER_LW;

		g_pTextInfo[g_nCurTextByteIndex/2+nNode].Len = nLen;
		g_pTextInfo[g_nCurTextByteIndex/2+nNode].Pos = g_vecWordBuffer[nNode].item[0].nPosIndex;
		g_pTextInfo[g_nCurTextByteIndex/2+nNode].TextType = TextCsItem_TextType_HANZI;	
		g_pTextInfo[g_nCurTextByteIndex/2+nNode].nBiaoDian[0] = 0;
		g_pTextInfo[g_nCurTextByteIndex/2+nNode].nBiaoDian[1] = 0;

		
		if( g_vecWordBuffer[nNode].item[0].pPinYin != NULL )				//分词后的此节点 有 多音词（词典表），或者有 姓氏 强制多音（姓氏表）
		{
			for( i = 0; i< g_vecWordBuffer[nNode].item[0].nCharNum; i++)
			{
				nPinYin = g_pTextPinYinCode[g_nCurTextByteIndex/2+nNode+i];
				if( nPinYin == 0 )			//若没有强制拼音覆盖过来 （银行行[=hang2]长）
				{
					emMemCpy( &g_pTextPinYinCode[g_nCurTextByteIndex/2+nNode+i], g_vecWordBuffer[nNode].item[0].pPinYin+i  ,2);	//先将分词后的拼音拷贝过来
				}
			}
		}	

		if( (nNode + g_vecWordBuffer[nNode].item[0].nCharNum)>= nMaxOfHanZi)
		{
			g_pTextInfo[g_nCurTextByteIndex/2+nNode].nBiaoDian[0] = nTempBiaoDian[0];
			g_pTextInfo[g_nCurTextByteIndex/2+nNode].nBiaoDian[1] = nTempBiaoDian[1];
		}

		nNode += g_vecWordBuffer[nNode].item[0].nCharNum;
	}	
}

//*****************************************************************************************************
//函数功能：构造 当前节点的所有字的所有词条（查词典）
//*****************************************************************************************************
void emCall MatchWordsOfCurNode()
{
	emUInt8 nNodeLen , i ,j;	//
	emUInt8	ch1 , ch2;
	emUInt32 offset;
	emUInt32 nOffsetMore[MAX_SEARCH_WORD_LEN_OF_DICT+1];	//当前字的 1字词 至 4字词 的起始偏移量  nOffsetMore[4]: 下一字的 1字词 的起始偏移量
	emUInt16 nLenMore[MAX_SEARCH_WORD_LEN_OF_DICT];	    //当前字的 1字词 至 4字词 的字节长度
	emUInt32 nCiAllLen;			//当前字 在《词典表》里  所有词条的总长度
	emUInt32  t1;
	emPByte  pMem_Save_CurNodeCiTiao;   
	emByte  chGb2312[MAX_SEARCH_WORD_LEN_OF_DICT*2 +4];
	emInt32 nAllocSize;

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	pMem_Save_CurNodeCiTiao = (emPByte)emHeap_AllocZero( 4096, "本字词典中词条：《分词模块  》");	//一般3K够了，在此开辟大一点，不影响堆的最大空间
#else
	pMem_Save_CurNodeCiTiao = (emPByte)emHeap_AllocZero( 4096);								//一般3K够了，在此开辟大一点，不影响堆的最大空间
#endif

	nNodeLen = g_pTextInfo[g_nCurTextByteIndex/2].Len;

	for( i=0; i< nNodeLen; i=i+2)
	{
		ch1 = g_pText[g_nCurTextByteIndex+i];			//当前汉字的 第1个字节
		ch2 = g_pText[g_nCurTextByteIndex+i+1];		//当前汉字的 第2个字节
		

		//如果 当前汉字 属于 GB2312 汉字
		if (  ch1 >= 0xb0 && ch1 <= 0xf7  && ch2 >= 0xa1 && ch2 <= 0xfe )
		{
			offset =  ( ch1 - 0xb0 ) *94 +  ch2 - 0xa1 ;
			offset *= GB2312_DICT_INDEX_LINE_LEN;								

			//查找《GB2312词条偏移表》，得到当前字的 1字词 至 4字词的 起始偏移量
			fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnPtGB2312Index + offset , 0);	
			fFrontRead(chGb2312, sizeof(emCharA), MAX_SEARCH_WORD_LEN_OF_DICT*2+4, g_hTTS->fResFrontMain );   


			t1 = chGb2312[0]<<15;

			if( MAX_SEARCH_WORD_LEN_OF_DICT >=  MAX_WORD_LEN_OF_DICT)
			{
				for( j = 0; j < MAX_SEARCH_WORD_LEN_OF_DICT; j++)
				{
					nOffsetMore[j] = (chGb2312[j*2 +2]<<8) + chGb2312[j*2 +1] + t1;
				}
				t1 = chGb2312[MAX_SEARCH_WORD_LEN_OF_DICT*2+1]<<15;
				nOffsetMore[MAX_SEARCH_WORD_LEN_OF_DICT]= (chGb2312[MAX_SEARCH_WORD_LEN_OF_DICT*2+3]<<8) + chGb2312[MAX_SEARCH_WORD_LEN_OF_DICT*2+2] + t1;
			}
			else
			{
				for( j = 0; j < (MAX_SEARCH_WORD_LEN_OF_DICT+1); j++)
				{
					nOffsetMore[j] = (chGb2312[j*2 +2]<<8) + chGb2312[j*2 +1] + t1;
				}

			}

			if( (offset / GB2312_DICT_INDEX_LINE_LEN) == (6768-1)) //是GB2312词典索引表中的最后一个字：齄		//hyl  2012-03-23
				nCiAllLen = 0;
			else
				nCiAllLen = nOffsetMore[MAX_SEARCH_WORD_LEN_OF_DICT]  - nOffsetMore[0];

			if( nCiAllLen != 0)		//有当前字的词条
			{

				//将《词典表》里 当前字 的所有词条 一次性 调入到 内存     此3句调入内存语句耗时占《分词模块--构词》的58%  (即50字的合成，若构词耗时84，此3句占49)
				nAllocSize = nCiAllLen;

				
				fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnPt + nOffsetMore[0] , 0);	
				fFrontRead(pMem_Save_CurNodeCiTiao, sizeof(emCharA), nCiAllLen, g_hTTS->fResFrontMain );


				//得到 当前字 相对于 内存中 的所有词条的 1字词 至 4字词的 起始偏移量
				for( j = MAX_SEARCH_WORD_LEN_OF_DICT; j > 0 ; j--)
				{
					nOffsetMore[j] = nOffsetMore[j] - nOffsetMore[0];				
				}
				nOffsetMore[0] = 0;
				for( j = 0; j < MAX_SEARCH_WORD_LEN_OF_DICT ; j++)
				{
					nLenMore[j] = nOffsetMore[j+1] - nOffsetMore[j];				
				}

				//构造 当前字 的 所有词条   此耗时占《分词模块--构词》的36%  (即50字的合成，若构词耗时84，此3句占30)
				GetAllCiTiaoOfCurZi(pMem_Save_CurNodeCiTiao ,nOffsetMore,nLenMore, nCiAllLen , i );			




				if( g_vecWordBuffer[i/2].nItemCount == 0  )		//没有匹配的词条，则构造1个词条
				{
					g_vecWordBuffer[i/2].nItemCount = 1;
					g_vecWordBuffer[i/2].item[0].nPosIndex = POS_CODE_g	;
					g_vecWordBuffer[i/2].item[0].nRatio = RATIO_OF_POS_G_MAKED;
					g_vecWordBuffer[i/2].item[0].nCharNum = 1;
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
					emMemCpy(g_vecWordBuffer[i/2].item[0].pAAAAAAText , &g_pText[g_nCurTextByteIndex+i], 2);
					*(g_vecWordBuffer[i/2].item[0].pAAAAAAText + 2 ) ='\0';
#endif
					g_vecWordBuffer[i/2].item[0].pPinYin = NULL;
				}
			}
			else		//无当前字的词条，则只构造1个词条
			{
				g_vecWordBuffer[i/2].nItemCount = 1;
				g_vecWordBuffer[i/2].item[0].nPosIndex = POS_CODE_g	;
				g_vecWordBuffer[i/2].item[0].nRatio = RATIO_OF_POS_G_MAKED;
				g_vecWordBuffer[i/2].item[0].nCharNum = 1;
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
				emMemCpy(g_vecWordBuffer[i/2].item[0].pAAAAAAText , &g_pText[g_nCurTextByteIndex+i], 2);
				*(g_vecWordBuffer[i/2].item[0].pAAAAAAText + 2 ) ='\0';
#endif
				g_vecWordBuffer[i/2].item[0].pPinYin = NULL;
			}

		}
		else		//当前汉字 不属于 GB2312 汉字， 则只构造1个词条
		{
			g_vecWordBuffer[i/2].nItemCount = 1;
			g_vecWordBuffer[i/2].item[0].nPosIndex = POS_CODE_g	;
			g_vecWordBuffer[i/2].item[0].nRatio = RATIO_OF_POS_G_MAKED;
			g_vecWordBuffer[i/2].item[0].nCharNum = 1;
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
			emMemCpy(g_vecWordBuffer[i/2].item[0].pAAAAAAText , &g_pText[g_nCurTextByteIndex+i], 2);
			*(g_vecWordBuffer[i/2].item[0].pAAAAAAText + 2 ) ='\0';
#endif
			g_vecWordBuffer[i/2].item[0].pPinYin = NULL;
			

		}		
	}

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pMem_Save_CurNodeCiTiao, 4096, "本字词典中词条：《分词模块  》");
#else
	emHeap_Free(pMem_Save_CurNodeCiTiao, 4096);
#endif
}


//****************************************************************************************************
//函数功能： 构造 当前字 的 所有词条 （从 4字词 到 1字词）
//				参数：  pMem_Save_CurNodeCiTiao：	从《词典表》里 取入的 当前字的 所有词条的 内存指针
//				参数：  nOffsetMore：				当前字的 1字词 至 4字词的 相对于参数1的 起始偏移量
//				参数：  nLenMore：					当前字的 1字词 至 4字词的 分别的 词条长度
//				参数：	nCiAllLen ：				从《词典表》里 取入的 当前字的 所有词条的 字节总长度
//				参数：  nNo：						当前字是 本节点 的 第几个字节
// 结果：将所有词条 写入到 g_vecWordBuffer[i]
//*****************************************************************************************************
void emCall GetAllCiTiaoOfCurZi(emPByte pMem_Save_CurNodeCiTiao ,emUInt32 *nOffsetMore,emUInt16 *nLenMore,emUInt32 nCiAllLen , emUInt8 nNo )
{

	emInt8  i ,j;
	emInt8  nSearchMaxWord;
	emUInt8 nLen;
	emUInt8  nCountOfWord, nCountOfCitiao, IsPloyOrDuoCiTiao;
	emUInt8  t1;
	emUInt8 *nCurOffset;
	emUInt8 *nCurMaxOffset;
	emUInt8  nCountOfItem;
	struct WordItem  *itemXing1, *itemXing2;
	struct WordItem  Xing1, Xing2;

	LOG_StackAddr(__FUNCTION__);
	
	itemXing1 = &Xing1;
	itemXing2 = &Xing2;
	Xing1.nCharNum = 0;
	Xing2.nCharNum = 0;

	//获取 当前字的 两个 姓氏词条	
	GetXingCiTiaoOfCurZi(nNo, itemXing1, itemXing2 );


	nCountOfItem = 0;

	nSearchMaxWord = (g_pTextInfo[g_nCurTextByteIndex/2].Len - nNo )/2;	    //当前汉字起始剩余的汉字个数
	if( nSearchMaxWord > MAX_SEARCH_WORD_LEN_OF_DICT)
	{
		nSearchMaxWord = MAX_SEARCH_WORD_LEN_OF_DICT;
	}

	// 从4字词（或剩余汉字的最大字词） 搜索到 1字词
	for(i = nSearchMaxWord -1; i>=0;i--)
	{

		//搜索 词典词条
		nLen = (i+1)*2;
		if( *(nLenMore + i) > 0)			//本字开头的  该几字词 至少 有1个 词条
		{
			nCurOffset = pMem_Save_CurNodeCiTiao +  *(nOffsetMore+i);
			nCurMaxOffset = pMem_Save_CurNodeCiTiao +  *(nOffsetMore+i+1);

			while( nCurOffset < nCurMaxOffset)
			{
				t1 = *(nCurOffset);
				nCurOffset++;
				IsPloyOrDuoCiTiao = t1>> 7 ;
				nCountOfWord = (t1 & 0x70)>>4;
				nCountOfCitiao = (t1 & 0x0f);

				//找到本字开头的 该几字词的 词组
				if( emMemCmp( nCurOffset ,&g_pText[g_nCurTextByteIndex+ nNo +2]   ,nLen-2) == 0 )
				{
					nCurOffset +=  (nCountOfWord -1)*2 ;
					if( IsPloyOrDuoCiTiao == 1)  //是多音词条或多词条的 词组
					{
						nCurOffset +=  1; 
					}
					
					//读取 词条
					for( j=0; j<nCountOfCitiao; j++)
					{
						g_vecWordBuffer[nNo/2].item[ nCountOfItem].nPosIndex = (*(nCurOffset)) & 0x7f;
						g_vecWordBuffer[nNo/2].item[ nCountOfItem].nRatio = ((*(nCurOffset+2))<<8 )+ (*(nCurOffset+1));
						g_vecWordBuffer[nNo/2].item[ nCountOfItem].nCharNum = i+1;
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
						emMemCpy(g_vecWordBuffer[nNo/2].item[ nCountOfItem].pAAAAAAText , &g_pText[g_nCurTextByteIndex + nNo],nLen);
						*(g_vecWordBuffer[nNo/2].item[ nCountOfItem].pAAAAAAText + nLen ) ='\0';
#endif
						if( ((*(nCurOffset)) >> 7) == 1 )  //是多音词条
						{

							g_vecWordBuffer[nNo/2].item[ nCountOfItem].pPinYin = g_pCurWordSegPinYin;
							g_pCurWordSegPinYin += g_vecWordBuffer[nNo/2].item[ nCountOfItem].nCharNum;

							emMemCpy(g_vecWordBuffer[nNo/2].item[ nCountOfItem].pPinYin, nCurOffset+3,nLen);
							nCurOffset += nLen +3;
						}
						else
						{
							g_vecWordBuffer[nNo/2].item[ nCountOfItem].pPinYin = NULL;
							nCurOffset += 3;
						}
						nCountOfItem++;
					}
					break;
				}
				else			//没找到词组
				{
					if( IsPloyOrDuoCiTiao == 1)  //是多音词条或多词条的 词组
					{
						nCurOffset +=  (nCountOfWord -1)*2 ;
						nCurOffset +=  *(nCurOffset) +1; 
					}
					else							//本词组只有1个词条且是基本词条
					{
						nCurOffset +=  (nCountOfWord -1)*2 + 3;
					}
				}
			}
		}

		
		//插入姓氏词条
		if(  itemXing1->nCharNum == (i+1) )
		{
			g_vecWordBuffer[nNo/2].item[nCountOfItem] = *(itemXing1);
			nCountOfItem++;
		}
		if(  itemXing2->nCharNum == (i+1) )
		{
			g_vecWordBuffer[nNo/2].item[nCountOfItem]= *(itemXing2);
			nCountOfItem++;
		}
		

	}
	g_vecWordBuffer[nNo/2].nItemCount = nCountOfItem;
}


//****************************************************************************************************
//函数功能： 获取 当前字 的 两个姓氏词条
//				参数：  nNo：			当前字是 本节点 的 第几个字节
//返回：
//				参数：  xingItem1：		当前字的 姓氏第1词条（2字姓名）
//				参数：  xingItem2:		当前字的 姓氏第2词条（3字姓名）
//*****************************************************************************************************
void emCall GetXingCiTiaoOfCurZi(emUInt8 nNo, struct WordItem  *xingItem1, struct WordItem  *xingItem2 )
{
	emUInt16  offsetLine;
	emUInt16  countOfRemainHanZi;
	emInt16 nRatio;
	emInt32 nPinYin;
	emPByte pCurXing_Table ;	

	emByte	curAndNextZi[4];


	LOG_StackAddr(__FUNCTION__);

	countOfRemainHanZi =( g_pTextInfo[g_nCurTextByteIndex/2].Len - nNo )/2;

	//剩余汉字>=2个，可以组成2字或3字姓名  （目前只查找1字的姓，暂不查找2字的姓（虽然《姓氏表》中有））
	if( countOfRemainHanZi >= 2 )	
	{
		emMemCpy( curAndNextZi, &g_pText[g_nCurTextByteIndex+nNo] , 2);
		curAndNextZi[2] = 0;
		curAndNextZi[3] = 0;
		offsetLine = ErFenSearchTwo_FromRAM( curAndNextZi, 0, g_pXing_Table, 1, g_MaxLineOfTableSurname ,BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);	//二分法搜索

		if( offsetLine != g_MaxLineOfTableSurname)		//找到与当前字相同的姓
		{
			//定位到已找到行的Ratio
			pCurXing_Table = g_pXing_Table + offsetLine*BYTE_NO_OF_TABLE_SURNAME_ONE_LINE + 4;
			nRatio = ( *(pCurXing_Table+1))*256 +( *(pCurXing_Table));
			nPinYin = ( *(pCurXing_Table+3))*256 +( *(pCurXing_Table+2));

			
			//构造2字姓名
			xingItem1->nCharNum = 2;
			xingItem1->nPosIndex = POS_CODE_nr;	//人名
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
			emMemCpy(xingItem1->pAAAAAAText,&g_pText[g_nCurTextByteIndex+nNo] , 4);
			*(xingItem1->pAAAAAAText + 4)='\0';
#endif
			xingItem1->nRatio = nRatio - RATIO_NEED_CUT_OF_TWO_NAME;		//构造两字姓名时，需减掉的ratio值(权重值)		
			if( nPinYin != 0)
			{
				xingItem1->pPinYin = g_pCurWordSegPinYin;
				g_pCurWordSegPinYin += xingItem1->nCharNum;

				//*(xingItem1->pPinYin) =  nPinYin | 0x8000	;		//姓氏多音：强制拼音类型
				*(xingItem1->pPinYin) =  nPinYin 	;
				*(xingItem1->pPinYin+1) = 0;
			}
			else
			{
				xingItem1->pPinYin = NULL;

			}

			if( countOfRemainHanZi >=3 )	
			{
				//构造3字姓名
				xingItem2->nCharNum = 3;
				xingItem2->nPosIndex = POS_CODE_nr;		//人名
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
				emMemCpy(xingItem2->pAAAAAAText,&g_pText[g_nCurTextByteIndex+nNo] , 6);
				*(xingItem2->pAAAAAAText + 6)='\0';
#endif
				xingItem2->nRatio = nRatio - RATIO_NEED_CUT_OF_THREE_NAME;		//构造三字姓名时，需减掉的ratio值(权重值)		
				if( nPinYin != 0)
				{
					xingItem2->pPinYin= g_pCurWordSegPinYin;
					g_pCurWordSegPinYin += xingItem2->nCharNum;

					//*(xingItem2->pPinYin) =  nPinYin | 0x8000	;		//姓氏多音：强制拼音类型
					*(xingItem2->pPinYin) =  nPinYin	;
					*(xingItem2->pPinYin+1) = 0;
					*(xingItem2->pPinYin+2) = 0;
				}
				else
				{
					xingItem2->pPinYin = NULL;

				}
			}
		}		
	}
}

//读取从nPrevPos转移的126个Pos二元文法值
void emCall ReadBigramMoreValue(emUInt8 nPrevPos)
{
	LOG_StackAddr(__FUNCTION__);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_ConvertRatioOfGram + nPrevPos * MaxLine_Of_OnePos_Of_TableBigram*2,  0);					
	fFrontRead( g_pBigram_Table,MaxLine_Of_OnePos_Of_TableBigram*2 , 1, g_hTTS->fResFrontMain);	
}


//从已有的126个二元文法值中获取nNextPos对应的二元文法值
emInt16 emCall GetBigramOneValue(emUInt8 nNextPos)
{
	emInt16  nBigramVal;

	LOG_StackAddr(__FUNCTION__);

	nBigramVal = (*(g_pBigram_Table + nNextPos * 2)) + (*(g_pBigram_Table + nNextPos * 2 +1 ))*256; 
	return nBigramVal;
}

emUInt8 GetInnerPos( emChar nPos)
{
	switch(nPos)
	{
		case 'n':
			return POS_CODE_n;
		case 'v':
			return POS_CODE_v;
		case 'a':
			return POS_CODE_a;
		case 'd':
			return POS_CODE_d;
		default:
			return POS_CODE_g;
	}
}


//增加用户词典资源的词条
void emCall AddUserDictWord()	
{
	emInt16 nWordCount,nT1[2];
	emInt16 nNodeLen,i,j,k,tt1,tt2,kk1,kk2;
	emInt16 nIndex;

	struct tagWordLine 
	{
		   emInt16	word[4];
		   emChar	pos;
		   emChar	ratio;
		   emInt16	spell[4];
	}; 
	struct tagWordLine curWordLine;	

	nNodeLen = g_pTextInfo[g_nCurTextByteIndex/2].Len;
	for( i=0; i< (nNodeLen-2); i=i+2)				//当前汉字节点中汉字逐个循环
	{
		//在《用户词典资源表》中的索引部分找到本字高字节开头的词组的索引
		nIndex = ((*(g_pText+i)) - 0xB0)*2;
		fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnUser+nIndex  , 0);	
		fFrontRead(nT1, 2, 2, g_hTTS->fResFrontMain );
		nWordCount = nT1[1] - nT1[0];

		if( nWordCount != 0)
		{
			//对本字高字节开头所有词组逐条匹配
			for( j = nT1[0]; j< nT1[1]; j++)
			{
				fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnUser+73*2+ nT1[0]*18 , 0);	
				fFrontRead(&curWordLine, sizeof(struct tagWordLine), 1, g_hTTS->fResFrontMain );

				tt1 = strlen((char *)curWordLine.word);
				tt2 = nNodeLen - i;
				if( tt2 >= tt1)
				{
					if( emMemCmp( curWordLine.word , g_pText+i, tt1) == 0 )		//匹配
					{
						kk1 = i/2;
						kk2 = g_vecWordBuffer[kk1].nItemCount;

						//赋值：词条长度，Ratio，词性，拼音
						g_vecWordBuffer[kk1].item[kk2].nCharNum = tt1/2;
						g_vecWordBuffer[kk1].item[kk2].nRatio = -1000;
						g_vecWordBuffer[kk1].item[kk2].nPosIndex = GetInnerPos(curWordLine.pos);
						g_vecWordBuffer[kk1].nItemCount++;
						emMemCpy( &(g_pTextPinYinCode[kk1]),curWordLine.spell,tt1);						
					}
				}
			}
		}
	}
}
