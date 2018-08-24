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
	Print_To_Txt( 3, "  ������  ",0);
#endif

	//���Ĭ��ƴ��--  ����һ�������롶Ĭ��ƴ������һ�������ֵ��ڴ棬���ļ����
	AddDefaultPinYin();							 //add by songkai  


#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  ���Ĭ��ƴ��  ",0);
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"����--Ĭ��ƴ��", 0);
#endif

	//���ݶ��������ı�ƴ��
	ConvertPinYinFromPolyRuler_FromRAM();                             //add by songkai

#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  ��Ӷ��������  ",0);
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"����--��������", 0);
#endif

}



//****************************************************************************************************
//  ���ܣ� ���Ĭ��ƴ��   add by songkai
//         ����һ�������롶Ĭ��ƴ������һ�������ֵ��ڴ棬���ļ����
//****************************************************************************************************
void emCall AddDefaultPinYin()
{
	emUInt16 nWordCount = 0, i = 0;  //ͳ������
	emUInt16 nRowNum = 0;


	LOG_StackAddr(__FUNCTION__);

	g_nP1TextByteIndex = 0 ;
	while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0)
	{
		//�ֳ����Ŀ��ܲ�����ǿ�ƶ���,����û��ǿ�ƶ���
		i = 0;
		nWordCount = g_pTextInfo[g_nP1TextByteIndex/2].Len/2;

		if( CheckDataType(&g_pText[g_nP1TextByteIndex] , 0 ) == DATATYPE_HANZI)					
		{
			while(i < nWordCount) //û��ǿ�ƶ���  g_p1TextCsItem->pPinYinCodeָ�����һ������
			{
				if(g_pTextPinYinCode[g_nP1TextByteIndex/2+i] == 0)  //û��ǿ��ƴ��(���û��ǿ��ƴ��������Ϊ0)
				{					
					//�ֱ�ȡ�����ֵĸ�8λ�͵�8λ
					nRowNum = (g_pText[g_nP1TextByteIndex+2*i] - 0x81) * 191 +  g_pText[g_nP1TextByteIndex+2*i+1] - 0x40; //�����ƫ������
					fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_GBKDefaultPinyin + nRowNum * ROW_SIZE, 0);  //��λ��GBKĬ��ƴ����
					fFrontRead(&g_pTextPinYinCode[g_nP1TextByteIndex/2+i], ROW_SIZE, 1, g_hTTS->fResFrontMain);  //��ȡĬ�϶���(��λ��һ������)
				}
				i++;
			}//end while
		}

		// -- 2011-02-15�޸�  Ϊʵ�ֹ��ܣ�Ӣ����ĸ�����������ѵ��
		if( CheckDataType(&g_pText[g_nP1TextByteIndex] , 0 ) == DATATYPE_YINGWEN)	 //����ͨӢ����ĸ
		{
			while(i < nWordCount) 
			{
				emInt32  tempYunMuCode=  g_pText[g_nP1TextByteIndex+2*i + 1] - 225 + FINAL_CN_letter_a;	//�õ���Ӣ����ĸ����ĸ���
				g_pTextPinYinCode[g_nP1TextByteIndex/2+i] = tempYunMuCode*256 + 8 * INITIAL_CN_letter + TONE_CN_letter;	//�õ���Ӣ����ĸ��ƴ������  25=��ĸ��� 7=�������
				i++;
			}//end while
		}

		g_nP1TextByteIndex += g_pTextInfo[g_nP1TextByteIndex/2].Len;
	}//end while
}




//****************************************************************************************************
//  ���ܣ����ݶ��������ı�ƴ��  add by songkai
//  һ���������������������,ʹ���µĶ��ַ�
//****************************************************************************************************

void emCall	ConvertPinYinFromPolyRuler_FromRAM()
{

	emUInt16 cTextLen = 0;
	emInt16 nLine = -1;  //-1��ʾû���ҵ� 
	emUInt16 i;
	emUInt16 nSize ;
	emPByte PolyIndexTable ;
	emBool bIsFind;
	emInt32 nAllocSize;
	emInt8 *pCurZiCiRule ; //��ǰ�ֻ�ʵĹ���������ʱ���ٴ洢
	PolyFunAlisa *pHeadPolyFunAlisaLink = NULL;



	LOG_StackAddr(__FUNCTION__);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_PolyRuleIndex, 0); //��λ��������
	fFrontRead(&g_WordCount, 2, 1, g_hTTS->fResFrontMain);//���롶�����ʡ��ĸ���
	fFrontRead(&g_CharCount, 2, 1, g_hTTS->fResFrontMain);//���롶�����֡��ĸ���


	nSize = (g_WordCount + g_CharCount) * INTERVAL ;
	nAllocSize = nSize;

#if DEBUG_LOG_SWITCH_HEAP
	PolyIndexTable = (emPByte)emHeap_AllocZero(nAllocSize, "����������������������ģ�顷");  
#else
	PolyIndexTable = (emPByte)emHeap_AllocZero(nAllocSize); 
#endif


#if DEBUG_LOG_SWITCH_HEAP
	pCurZiCiRule = (emInt8 *)emHeap_AllocZero(4096, "��ǰ�ֶ������򣺡�������ģ�顷");  //���ƴ�Լռ1K�ֽ�,�ڴ˶࿪��һЩ��������ȫ�ֶ�
#else
	pCurZiCiRule = (emInt8 *)emHeap_AllocZero(4096);  //���ƴ�Լռ1K�ֽ�,�ڴ˶࿪��һЩ��������ȫ�ֶ�
#endif



#if DEBUG_LOG_SWITCH_HEAP
	pHeadPolyFunAlisaLink = (PolyFunAlisa*)emHeap_AllocZero(COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa),"��������������������ģ�顷");
#else
	pHeadPolyFunAlisaLink = (PolyFunAlisa*)emHeap_AllocZero(COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa));
#endif


	fFrontRead(PolyIndexTable, nSize, 1, g_hTTS->fResFrontMain);//������������
	
	if(g_pTextInfo[0].Len != 0)
	{
		g_nCurTextByteIndex = 0;
		while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
		{	
			cTextLen = g_pTextInfo[g_nCurTextByteIndex/2].Len/ 2;  //TEXT�ɼ��������

			if(cTextLen == 1 )     //�����1�ֶ����� �� ������     
			{
				bIsFind = FALSE;

				//�Ȳ���1�ֶ�����
				nLine = ErFenSearchOne_FromRAM(&g_pText[g_nCurTextByteIndex], 0, PolyIndexTable, 1, g_WordCount, INTERVAL);
				if (nLine != g_WordCount)	//��ʾ�鵽1�ֶ�����
				{
					bIsFind = HandleCurZiOrCi_FromRAM(nLine, 0, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
				}


				if( bIsFind == FALSE)		//��ʾδ�鵽1�ֶ�����
				{
					//��ʼ���Ҷ�����
					nLine = g_WordCount + ErFenSearchOne_FromRAM(&g_pText[g_nCurTextByteIndex], 0, PolyIndexTable + g_WordCount * INTERVAL, 1, g_CharCount, INTERVAL);
					if (nLine != (  g_CharCount + g_WordCount) )	//��ʾ�鵽1�ֶ�����
					{
						HandleCurZiOrCi_FromRAM(nLine, 0, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
					}
				}
			}

			if(cTextLen == 2 )         //�����2�ֶ����� �� ѭ�����Ҷ�����      
			{
				//�Ȳ���2�ֶ�����
				nLine = ErFenSearchTwo_FromRAM(&g_pText[g_nCurTextByteIndex], 0, PolyIndexTable, 1, g_WordCount, INTERVAL); 
				if (nLine != g_WordCount)	//��ʾ�鵽2�ֶ�����
				{
					HandleCurZiOrCi_FromRAM(nLine, 0, 2, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
				}
				else						//��ʾδ�鵽2�ֶ����ʣ�
				{
					//��ʼѭ�����Ҷ�����
					for(i = 0; i< 2; i++)
					{
						nLine = g_WordCount + ErFenSearchOne_FromRAM(&g_pText[g_nCurTextByteIndex + i*2], 0, PolyIndexTable+ g_WordCount * INTERVAL, 1, g_CharCount, INTERVAL);
						if (nLine != (  g_CharCount + g_WordCount) )	//��ʾ�鵽1�ֶ�����
						{
							HandleCurZiOrCi_FromRAM(nLine, i, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
						}
					}//end for
				}//end else
			}

			if(cTextLen > 2 )          //ֻ��ѭ�����Ҷ�����     
			{
				//��ʼѭ�����Ҷ�����
				for(i = 0; i< cTextLen; i++)
				{
					nLine = g_WordCount + ErFenSearchOne_FromRAM( &g_pText[g_nCurTextByteIndex + i*2], 0, PolyIndexTable + g_WordCount * INTERVAL, 1, g_CharCount, INTERVAL); 
					if (nLine != (  g_CharCount + g_WordCount) )	//��ʾ�鵽1�ֶ�����
					{
						HandleCurZiOrCi_FromRAM(nLine, i, 1, PolyIndexTable,pCurZiCiRule,pHeadPolyFunAlisaLink);
					}
				}	
			}//end if					

			g_nCurTextByteIndex += g_pTextInfo[g_nCurTextByteIndex/2].Len;
		}//end inner while
	}//end if


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pHeadPolyFunAlisaLink, COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa),"��������������������ģ�顷");
#else
	emHeap_Free(pHeadPolyFunAlisaLink, COUNT_PolyFunAlisaLink*sizeof(PolyFunAlisa));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pCurZiCiRule, 4096, "��ǰ�ֶ������򣺡�������ģ�顷");  //���ƴ�Լռ1K�ֽ�
#else
	emHeap_Free(pCurZiCiRule, 4096);  //���ƴ�Լռ1K�ֽ�
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(PolyIndexTable, nAllocSize, "����������������������ģ�顷");  
#else
	emHeap_Free(PolyIndexTable, nAllocSize);  
#endif

}


//****************************************************************************************************
//  ���ܣ� ����һ���ֻ�ʵĹ���:��������Ķ�����������   add by songkai
//****************************************************************************************************

emBool emCall HandleCurZiOrCi_FromRAM(emInt16 nLine,				// �������ִʹ�����������ĵڼ���
									emInt16 nCurOffset,				// Ҫ������Ǵ�g_curTextCsItem�еĵڼ����ֿ�ʼ���ֻ��
									emInt16 nMaxHanZi,				// Ҫ������� 1���� ���� 2����
									emPByte  PolyIndexTable,		//���������ڴ���׵�ַ	
									emInt8 *pCurZiCiRule,			//��ǰ�ֶ�������
									PolyFunAlisa *pHeadPolyFunAlisaLink)	//������������



{

	emUInt16 nLen = 0;
	PolyFunAsistant  polyFA;     //�����ṹ��
	PolyFunAlisa *pHead = NULL, *pTemp = NULL, *pCur = NULL, *pFor=NULL; //����ṹ���ǵ�ǰ������/�ʵ�һ�������еĶ���������Ϣ�ڵ�
	PolyFunAlisa  *pCurPolyFunAlisaLink = NULL;
	emUInt16 nTrueEncode[2], nFalseEncode[2];
	emByte  cFunCount, cFunLen, nMaxPro;
	emByte i;	
	emBool nRes = -1;	
	emUInt16 nOffSet = 0;
	emInt8 *pRule;
	emBool bReturnValue = FALSE;
	emInt32 nAllocSizeCurZiCiRule;
	emUInt16 nZiCode;		//��ǰ������


	LOG_StackAddr(__FUNCTION__);

	//�ѵ�ǰ�����ַ��뵽nZiCode��
	emMemCpy(&nZiCode,&g_pText[g_nCurTextByteIndex+ nCurOffset*2],2);          //add by liugang 2011-6-9
	polyFA.Zi_Code = nZiCode;

	emMemCpy(&nOffSet, PolyIndexTable + (nLine ) * INTERVAL + 4,  2);
	emMemCpy(&nLen, PolyIndexTable + (nLine ) * INTERVAL + 6, 2);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_PolyRule + nOffSet, 0);  //��λ��������и���/�ʵ���Ӧ��ʼλ��
	nAllocSizeCurZiCiRule = nLen;

	fFrontRead(pCurZiCiRule, nLen,  1, g_hTTS->fResFrontMain);//�������/�ʵ����й���


	//����������ʼ�������
	//�����Ǵ���ÿһ������Ķ�����������,ѭ��һ�δ���һ������.
	pRule = pCurZiCiRule;
	while(pRule < pCurZiCiRule + nLen)
	{
		//polyFA���㲢����ǰ�����ֿ�����polyFA.Zi_Code  //add by hhh 2011-09-08
		emMemSet(&polyFA, 0 , sizeof(struct PolyFunAsisant));
		polyFA.Zi_Code = nZiCode;

		emMemCpy(nTrueEncode, pRule, 4);	//��ȷ����µĶ���
		pRule += 4 ;   //�ĸ��ֽ�
		emMemCpy(nFalseEncode, pRule, 4);    //��������µĶ���
		pRule += 4;   //�ĸ��ֽ�
		emMemCpy(&cFunCount, pRule, sizeof(cFunCount));      //���ֵ�ĸ���λ�Ǳ������������ȼ�,����λ�����������ж��ٸ���������
		i = cFunCount;
		cFunCount &= 0x1F;  //ȡ������λ,Ҳ�����ж��ٸ��������� 
		nMaxPro = (i & 0xE0) / 32;  //�õ��˱��������������ȼ� 
		++pRule;  //һ���ֽ�

		cFunLen = *(pRule++);  //���������Ĵ洢����
		//���濪ʼ��ȡ������������
		polyFA .FunType = *(pRule ++);  //��������
		
		pCurPolyFunAlisaLink = pHeadPolyFunAlisaLink;
		pHead = pHeadPolyFunAlisaLink;		

		pTemp = pHead;
		pHead -> next = NULL;
		pHead -> IsAnd = (*(pRule) & 0x80) / 128;   //�Ƿ�����һ��������Ĺ�ϵ
		pHead -> IsNotBeforeOperation  = (*(pRule) & 0x40) / 64;  //��������ֵ����1��������ǰ�Ƿ�ȡ��
		polyFA .IsDirectNot = (*(pRule) & 0x20) /32;  //��������ֵ�Ƿ�ֱ��ȡ��
		pHead -> ProbCount = *(pRule)  & 0x07;   //�����������ȼ�
		++pRule;

		polyFA .Para1 = *(pRule++);  //����1     
		polyFA .Para2 = *(pRule++);  //����2
		i = cFunLen - 4;             //����ͷ���ĸ����о��ǲ���3��
		if( i > 0)
		{
			emMemSet(polyFA .Para3, 0, MAX_POLYFA_PARA3_LEN);
			emMemCpy(polyFA .Para3, pRule, emMin(i,MAX_POLYFA_PARA3_LEN));  //����3 
		}
		pRule += i;	
		nRes = HandleFun(&polyFA);  //�ж�һ����������
		if(polyFA.IsDirectNot == 1)  //��Ҫֱ��ȡ��.�ڼ��������ʱ���ȡ��
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
		pHead -> IsResultTrue = nRes;  //�����������������ֵ 
		while(--cFunCount)
		{
			//polyFA���㲢����ǰ�����ֿ�����polyFA.Zi_Code  //add by hhh 2011-09-08
			emMemSet(&polyFA, 0 , sizeof(struct PolyFunAsisant));
			polyFA.Zi_Code = nZiCode;

			pCurPolyFunAlisaLink++;
			pCur = pCurPolyFunAlisaLink;

			pCur -> next = NULL;

			cFunLen = *(pRule++);
			polyFA .FunType = *(pRule ++);  //��������
			pCur -> IsAnd = (*(pRule) & 0x80) / 128;   //�Ƿ�����һ��������Ĺ�ϵ
			pCur -> IsNotBeforeOperation  = (*(pRule) & 0x40) / 64;  //��������ֵ����1��������ǰ�Ƿ�ȡ��
			polyFA .IsDirectNot = (*(pRule) & 0x20) /32;  //��������ֵ�Ƿ�ֱ��ȡ��
			pCur -> ProbCount = *(pRule)  & 0x07;   //�����������ȼ�
			++pRule;
			polyFA .Para1 = *(pRule++);  //����1     
			polyFA .Para2 = *(pRule++);  //����2
			i = cFunLen - 4;             //����ͷ���ĸ����о��ǲ���3��
			if( i > 0)
			{
				emMemSet(polyFA .Para3, 0, MAX_POLYFA_PARA3_LEN);
				emMemCpy(polyFA .Para3, pRule, emMin(i,MAX_POLYFA_PARA3_LEN));  //����3 
			}
			pRule += i;
			nRes = HandleFun(&polyFA);  //�ж�һ����������
 			if(polyFA.IsDirectNot == 1)  //��Ҫֱ��ȡ�����ڼ��������ʱ���ȡ��
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
	
		//���˴����ɹ���һ������Ķ�����������
		nRes = RealHandleRule(pHead, nTrueEncode, nFalseEncode, &nMaxPro,nCurOffset,nMaxHanZi);  //�����Ĵ��������.


		if(nRes == 0)  //ֻҪ��һ���������������
		{
			bReturnValue = TRUE;
			break;
		}
	}//end while


	return bReturnValue;

}




//****************************************************************************************************
//  ���ܣ� ��������еĶ�������   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: ��ǰ����Ϊ�滹��Ϊ��. ����-1Ϊ��,0Ϊ��
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
//  ���ܣ� ����WordLen����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����WordLenGreateThan����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����WordLenLessThan����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����MatchWord����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����MatchChar����   add by hyl   20110626
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
//****************************************************************************************************
emInt8 emCall HandleFunMatchChar(PolyFunAsistant* polyFA)   //ͨ���鿴*.rule�ļ�,����ƥ�䵥��
{
	emInt16 curZiIndex, leftRangeIndex,rightRangeIndex;
	emBool bIsFind = emFalse;
	emByte curZi[3],curBiZi[3];
	emInt16 nCount = 0,i,j,nResult;
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	nCount = g_nCurTextByteIndex;	
	nTmpNode = g_nCurTextByteIndex;

	//�ҵ���ǰҪ����Ķ��������ı����ǵڼ����֣�nCount��
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

	curZiIndex = nCount/2;										//�õ�����ǰ�֡�������
	leftRangeIndex  = curZiIndex + polyFA->Para1;				//�õ�����Χ����ߡ�������
	rightRangeIndex = curZiIndex + polyFA->Para2;				//�õ�����Χ���ұߡ�������
	if( leftRangeIndex  < 0)						leftRangeIndex = 0;
	if( leftRangeIndex > (g_nLastTextByteIndex/2-1))	return -1;			//��û�ҵ�����
	if( rightRangeIndex > (g_nLastTextByteIndex/2-1))	rightRangeIndex = g_nLastTextByteIndex/2-1;

	//�жϡ���Χ�ڵ��֡��Ƿ���polyFA->Para3֮��
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
				return 0;		//�ҵ��ˣ���
			}
		}
	}
	return -1;			//��û�ҵ�����
}



//****************************************************************************************************
//  ���ܣ� ����Pos����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����RealEnd����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
		return ((g_pTextInfo[g_nNextTextByteIndex/2].Len != 0) ? -1: 0);  //����������һ����,��ô�Ͳ���next�Ͳ���NULL
	}
}

//****************************************************************************************************
//  ���ܣ� ����PosEnd����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����WordEnd����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����PrecedeVerb����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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

	//�����һ�ʵĴ����Ƕ���
	if(g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_v || g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_vd || g_pTextInfo[g_nNextTextByteIndex/2].Pos  == POS_CODE_vn) 
	{
		return 0;
	}
	return -1;

}

//****************************************************************************************************
//  ���ܣ� ����SucceedVerb����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
			//���ǰһ�ʵĴ����Ƕ���
			if(g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_v || g_pTextInfo[g_nP2TextByteIndex/2].Pos == POS_CODE_vd || g_pTextInfo[g_nP2TextByteIndex/2].Pos  == POS_CODE_vn) 
			{
				return 0;
			}
		}
		return -1;
	}
}

//****************************************************************************************************
//  ���ܣ� ����PrecedeNoun����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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

	//�����һ�ʵĴ����Ƕ���
	if(g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_n || g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_nr || g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_ns || 
	   g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_nt|| g_pTextInfo[g_nNextTextByteIndex/2].Pos == POS_CODE_nz)
	{
		return 0;
	}
	return -1;	
}


//****************************************************************************************************
//  ���ܣ� ����SucceedNoun����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
			//���ǰһ�ʵĴ���������
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
//  ���ܣ� ����WordBeginWith����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� ����WordEndWith����   add by songkai
//  ����1:�����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
				if(!emMemCmp(p1, &g_pText[nTmpNode+i], emStrLenA(p1)))  //����������β��
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
//  ���ܣ� ����LastPunc����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
//****************************************************************************************************

emInt8 emCall HandleFunLastPunc(PolyFunAsistant* polyFA)
{
	emUInt16 nTmpNode = 0;

	LOG_StackAddr(__FUNCTION__);

	//�˺��������ã����������ﲻҪ��д�˺���
	return -1;
	
}

//****************************************************************************************************
//  ���ܣ� ����NextPun����   add by songkai
//  ����1: �����ṹ��ָ��
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
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
//  ���ܣ� �ƶ���ָ���ڵ�   add by songkai
//  ����1: ����ڵ�ǰ�ڵ��ƶ����ٸ��ڵ�
//  ����ֵ: ������ڷ��ؽڵ���������,�����ڷ���0xff
//****************************************************************************************************
emUInt8 emCall GetSpecailNode(emInt16 nArg)
{
	emInt16 nTmpNode , nTmpNode2 ;
	emInt16 nCount = 0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = g_nCurTextByteIndex; //�õ���ǰ�ڵ�ָ��
	if(nArg == 0)
	{
		return nTmpNode;
	}
	else if(nArg > 0)  //���Ƶ�ָ���ڵ�
	{
			while(nArg)  
			{
				g_nNextTextByteIndex = nTmpNode + g_pTextInfo[nTmpNode/2].Len;
				if(g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)   //������ָ���ڵ�
				{
					return NO_FIND_NODE;
				}
				--nArg;

				nTmpNode = g_nNextTextByteIndex;

			}//ѭ��������,nTmpNode�ƶ���ָ���ڵ�
			return nTmpNode;
	}
	else   //ǰ�Ƶ�ָ���ڵ�
	{
		nTmpNode2 = 0;
		while(1)  //���ȼ��㵱ǰ�ڵ�ǰ���м����ڵ�
		{
			if(nTmpNode2 == nTmpNode)
			{
				break;	
			}
			g_nNextTextByteIndex = nTmpNode2 + g_pTextInfo[nTmpNode2/2].Len;
			nTmpNode2 = g_nNextTextByteIndex;
			nCount++;   //��¼�ڵ���
		}
		if(nCount - emAbs(nArg) ==0)  //���ǰ�Ƶ�ͷ
		{
			return  0;
		}
		else if(nCount - emAbs(nArg) < 0)  //������
		{
			return NO_FIND_NODE;		
		}
		else
		{
			nCount -= emAbs(nArg); //��ͷ��Ҫ���Ƽ����ڵ�
			nTmpNode2 = 0;  //ָ��ͷ���
			while(nCount)
			{
				--nCount;

				g_nNextTextByteIndex = nTmpNode2 + g_pTextInfo[nTmpNode2/2].Len;
				nTmpNode2 = g_nNextTextByteIndex;
		
			}//ѭ��������,nTmpNode2�ƶ���ָ���ڵ�
			return nTmpNode2;
		}
	}
}


//****************************************************************************************************
//  ���ܣ� �ƶ������һ���ڵ�   add by songkai
//  ����ֵ: ���ؽڵ���������
//****************************************************************************************************
emUInt8 emCall GetLastNode()
{
	emUInt16 nTmpNode = 0,nNextNode=0;

	LOG_StackAddr(__FUNCTION__);

	nTmpNode = g_nCurTextByteIndex; //�õ���ǰ�ڵ�ָ��
	nNextNode = nTmpNode +g_pTextInfo[nTmpNode/2].Len;
	while( g_pTextInfo[nNextNode/2].Len != 0 )   // �������޸ģ�2010-12-13
	{
		nTmpNode = nNextNode;
		nNextNode = nTmpNode +g_pTextInfo[nTmpNode/2].Len;
	}
	return nTmpNode;
}

//****************************************************************************************************
//  ���ܣ� ��������еĶ�����������   add by songkai  //�ҵ�����������,�Ѷ�����ֵ����ǰ��ȫ������ڵ��ƴ���ֶ�
//  ����ֵ: ��ǰ����Ϊ�滹��Ϊ��. ����-1Ϊ��,0Ϊ��
//****************************************************************************************************
emBool emCall RealHandleRule(PolyFunAlisa *pHead,		//  ����: ������������ͷָ��
							 emUInt16 *nTrueEncode,		//  ����: ����Ϊ���ʱ��Ķ�����ַ
							 emUInt16 *nFalseEncode,		//  ����: ����Ϊ�ٵ�ʱ��Ķ�����ַ
							 emByte* nMaxPro,			//  ����: �����������ȼ�
							emInt16 nCurOffset,			// Ҫ������Ǵ�g_curTextCsItem�еĵڼ����ֿ�ʼ���ֻ��
							emInt16 nMaxHanZi)			// Ҫ������� 1���� ���� 2����

{
	PolyFunAlisa *pTmp = NULL, *p1 = NULL, *p2 = NULL, *pFor = NULL, *pCurHead=NULL;
	emInt32 nPinYinForce;
	emInt8 nMax = *nMaxPro; //������ȼ�

	LOG_StackAddr(__FUNCTION__);

	//ֻ��һ�����������ڵ�
	if(pHead ->next == NULL)
	{
		if(pHead -> IsResultTrue == 0)  //�������������TRUE,Ҳ��������������TRUE
		{
			nPinYinForce = g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] & 0x8000;
			
			if( nPinYinForce != 0x8000 ) //Ĭ�ϣ���ǿ��ƴ��
			{
				g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset]= *nTrueEncode; //�ѹ���Ķ�������
				if( nMaxHanZi == 2)			//2�ִ�
				{
					g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset + 1] = *(nTrueEncode + 1); //�ѹ���Ķ�������
				}
			}
		}
		else  //�������������FALSE,Ҳ��������������FALSE
		{
			nPinYinForce = g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] & 0x8000;
			if( nPinYinForce != 0x8000 ) //Ĭ�ϣ���ǿ��ƴ��
			{
				if(*nFalseEncode == 0)
				{
					return -1;  //�������򲻳���
				}
				g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] = *nFalseEncode; //�ѹ���Ķ�������
				if( nMaxHanZi == 2)			//2�ִ�
				{
					g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset+1] = *(nFalseEncode + 1); //�ѹ���Ķ�������
				}
			}
		}
		return 0;
	}

	//�ж�����������ڵ�
	pTmp = pHead;
	pCurHead = pHead;
	do
	{
		pFor = pCurHead;
		while(pTmp -> next != NULL)  //�������һ���ڵ�
		{
			while(pTmp -> next != NULL && pTmp -> ProbCount != nMax) //�ڶ������������в���������ȼ���
			{
				pFor  = pTmp;  //ָ�����ȼ���ߵ�ǰһ���ڵ�
				pTmp = pTmp -> next;
			}//end while
			if(pTmp -> ProbCount == nMax)  //�ҵ�������ȼ���
			{
				p1 = pTmp;
				p2 = pTmp -> next;
				if(p2 != NULL)
				{
					if(p1 -> IsAnd == 1)  //������
					{  
						if(p2 -> IsResultTrue == 0)
						{
							p2 -> IsResultTrue = p1 -> IsResultTrue;//����ֵ
						}
					}
					else
					{
						if(p2 -> IsResultTrue == -1)  //������
						{
							p2 -> IsResultTrue = p1 -> IsResultTrue;//����ֵ	
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
					
					pTmp = p2;//���ֻʣ��pTmp�ڵ�
				}
			}//end if
		}//end while
		--nMax; //�´�ѭ���ݼ�
		pTmp = pCurHead;

	}while(pTmp -> next != NULL);//end do
//���ֻʣ����pTmp�ڵ���
	if(pTmp -> IsResultTrue == 0)  //Ϊ��������
	{
		nPinYinForce =  g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset]& 0x8000;
		if( nPinYinForce != 0x8000 ) //Ĭ�ϣ���ǿ��ƴ��
		{
			 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] = *nTrueEncode; //�ѹ���Ķ�������
			if( nMaxHanZi == 2)			//2�ִ�
			{
				 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset+1] = *(nTrueEncode + 1); //�ѹ���Ķ�������
			}
		}
		return 0;
	}
	else
	{
		nPinYinForce =  g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] & 0x8000;
		if( nPinYinForce != 0x8000 ) //Ĭ�ϣ���ǿ��ƴ��
		{
			if(*nFalseEncode == 0)
			{
				return -1;  //��������û��ΪFALSE����µĶ���.
			}
			 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset] = *nFalseEncode; //�ѹ���Ķ�������
			if( nMaxHanZi == 2)			//2�ִ�
			{
				 g_pTextPinYinCode[g_nCurTextByteIndex/2+ nCurOffset+1] = *(nFalseEncode + 1); //�ѹ���Ķ�������
			}
		}
		return 0;
	}
}





//****************************************************************************************************
//  ���ܣ� ����Pos������PosEnd����ʱ�����⴦��   add by songkai
//  ����1: ��ǰ�ڵ��posֵ
//  ����ֵ: �������Ϊ��(0)���Ǽ�(-1)
//  ˵���� 
//���� ����������⡷�еġ������������ġ�POS(-1,"nr")���͡�POSEnd("n")����2�������⴦��
//����2��Ĵ�����������ߵĴ���ʱ�� �������滻���ұߵ�1����2�����ԡ�  ����Ϊ�����������û���ұߵĴ��ԣ����ʵ����У�
//���ɵ�Ч����   POS(-1,"nr")     ---��      �� POS(-1,"nr") | POS(-1,"nrf")| POS(-1,"nrg") ��
//                 POSEnd("n")      ---��      �� POSEnd("n")  | POSEnd("nf") | POSEnd("nh")  ��

//���	  �ұ�1     �ұ�2
//e  16  ��  eg  17
//i  22  ��  ic  25   im   27
//j  30  ��  jm  33
//l  37  ��  lc  40   lm   42
//n  48  ��  nf  50   nh   52
//nr 53  ��  nrf 54   nrg  55  
//nz 59  ��  nzg 60
//v  94  ��  vv  104
//w 106  ��  wy  118
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
