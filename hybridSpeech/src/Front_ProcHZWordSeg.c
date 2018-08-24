#include "emPCH.h"

#include "Front_ProcHZWordSeg.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <jni.h>
#include "jnirelated.h"


emInt16 g_MaxLineOfTableSurname	;				//���ϱ���������

emPByte g_pXing_Table ;							//��ROM��ġ����ϱ����뵽�ڴ���
emPByte g_pBigram_Table ;						//��ROM��ġ���Ԫ�ķ����Ĺ̶�Pos1ת�Ƶ�126����ͬ��Pos2��ֵһ����ȡ�뵽�ڴ棬ռ0.252K�ֽ�
struct WordBuffer *g_vecWordBuffer ;			//�ִ�ģ����Ҫ  �洢ÿ���ֵ����д��������50����
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
//�������ܣ� �ִ���ģ��
//*****************************************************************************************************
void emCall WordSeg()
{
//  print_g_pText();
//  print_g_pTextInfo();
//  print_g_pTextPinYinCode();

    emInt16  nPrevPOS;						// ǰ1�ڵ�Ĵ���pos����
    emInt16  nNextPOS;						// ��1�ڵ�Ĵ���pos����
	emInt16 offsetLine,nCurLen;
	emInt32 nPinYin;

	emByte	firstZi[4];
	emPByte pCurXing_Table ;	

	LOG_StackAddr(__FUNCTION__);

	//��ʼ�����ִ�ģ�顷������ռ�
	WordSegInit();

	nPrevPOS = POS_CODE_w;						//�����ǰ1�ڵ�Ĵ���pos������ �ǡ�w�������	
    nNextPOS = 0;	

	fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_SurnameRule,  0);					
	fFrontRead(&g_MaxLineOfTableSurname, 1 , 2, g_hTTS->fResFrontMain);	//���롶���ϱ����������

	
	if( g_pTextInfo[0].Len != 0 )
	{
		//���п��Ʊ��[r1]��[r2]��������ǿ�ƶ�������ƴ��
		if( g_hTTS->m_ControlSwitch.m_bXingShi == emTTS_USE_XINGSHI_JUSHOU_OPEN ||  g_hTTS->m_ControlSwitch.m_bXingShi == emTTS_USE_XINGSHI_AFTER_OPEN )  
		{

			//һ���Խ����������ϱ����뵽�ڴ�
#if DEBUG_LOG_SWITCH_HEAP
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"�������ϱ����ִ�ģ��  ��");			//�����ڴ�ռ�
#else
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);			//�����ڴ�ռ�
#endif
			fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_SurnameRule+BYTE_NO_OF_TABLE_SURNAME_ONE_LINE, 0 );					
			fFrontRead(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE , 1, g_hTTS->fResFrontMain);	//һ���Զ���������

			emMemCpy( firstZi, &g_pText[0], 2);
			firstZi[2] = 0;
			firstZi[3] = 0;
			offsetLine = ErFenSearchTwo_FromRAM( firstZi, 0, g_pXing_Table, 1, g_MaxLineOfTableSurname ,BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);	//���ַ�����

			if( offsetLine != g_MaxLineOfTableSurname)		//�ҵ����1������ͬ����
			{
				//��λ�����ҵ��е�ƴ��
				pCurXing_Table = g_pXing_Table + offsetLine*BYTE_NO_OF_TABLE_SURNAME_ONE_LINE + 6;
				nPinYin = ( *(pCurXing_Table+1))*256 +( *(pCurXing_Table));
				g_pTextPinYinCode[0]  = nPinYin;
				if( g_pTextPinYinCode[0] != 0)
				{
					g_pTextPinYinCode[0] = g_pTextPinYinCode[0] | 0x8000;		//ǿ��ƴ��
				}
			}

			//�ͷš����ϱ����ڴ�
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"�������ϱ����ִ�ģ��  ��");
#else
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);
#endif

		}

		//����[r2](��������ľ���ǿ�ƶ�������)�����轫g_hTTS->m_ControlSwitch.m_bXingShi���عرա�  ע�⣺����[r1]����Ҫ��
		if( g_hTTS->m_ControlSwitch.m_bXingShi == emTTS_USE_XINGSHI_AFTER_OPEN )
		{
			g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_CLOSE;
		}
	}


	g_nCurTextByteIndex = 0;
	while( g_pTextInfo[g_nCurTextByteIndex/2].Len != 0 )
	{
		nCurLen = g_pTextInfo[g_nCurTextByteIndex/2].Len;

		//�����ڵ� ���ı����� �� �޴��� �� ����� �ִ�
		if( g_pTextInfo[g_nCurTextByteIndex/2].TextType == TextCsItem_TextType_HANZI &&  g_pTextInfo[g_nCurTextByteIndex/2].Pos == 0 )
		{
			g_nNextTextByteIndex = g_nCurTextByteIndex + g_pTextInfo[g_nCurTextByteIndex/2].Len;
			if( g_pTextInfo[g_nNextTextByteIndex/2].Len != 0 )
			{
				nNextPOS = g_pTextInfo[g_nNextTextByteIndex/2].Pos;				
			}
			else
			{
				nNextPOS = POS_CODE_w;					//����ĺ�1�ڵ�Ĵ���pos������ �ǡ�w�������	

			}	
#if DEBUG_LOG_SWITCH_RHYTHM_MIDDLE
			if( g_pTextInfo[g_nNextTextByteIndex/2].Len == 0 )
			{
				Print_To_Rhythm("log/��־_����.log", "a", 0,"ԭ�䣺");
			}
#endif


			//һ���Խ����������ϱ����뵽�ڴ�
#if DEBUG_LOG_SWITCH_HEAP
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"�������ϱ����ִ�ģ��  ��");			//�����ڴ�ռ�
#else
			g_pXing_Table = (emPByte)emHeap_AllocZero(g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);			//�����ڴ�ռ�
#endif

			fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_SurnameRule,  0);					
			fFrontRead(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE , 1, g_hTTS->fResFrontMain);	//һ���Զ���������


			//���� ��ǰ�ڵ㣨δ�𿪵ĵ�ǰ�ڵ� �� �𿪺��ǰ�ڵ㣩�������ֵ����д�������ʵ䣩
			MatchWordsOfCurNode();

			//�����û��ʵ���Դ�еĴ���
			if( g_Res.offset_DictCnUser )		//����˱����
				AddUserDictWord();				//2012-11-26 hyl

			//�ͷš����ϱ����ڴ�
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE,"�������ϱ����ִ�ģ��  ��");
#else
			emHeap_Free(g_pXing_Table, g_MaxLineOfTableSurname * BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);
#endif

#if DEBUG_LOG_SWITCH_TIME
			myTimeTrace_For_Debug(1,(emByte *)"�ִ�--�������", 0);
#endif

			//�� ��ǰ�ڵ���ж�Ԫ�ķ���������ȡ��ѷִ�·��
			AmbiguityAnalysis( g_pTextInfo[g_nCurTextByteIndex/2].Len/2 ,nPrevPOS, nNextPOS);

#if DEBUG_LOG_SWITCH_TIME
			myTimeTrace_For_Debug(1,(emByte *)"�ִ�--���·��", 0);
#endif

			//������ѷִ�·�ߣ������������еĵ�ǰ�ڵ��ֳɶ���ڵ㣬������ǰ�ڵ�ָ���ֺ�����1���ڵ�
			SegCurNodeToManyNode( g_pTextInfo[g_nCurTextByteIndex/2].Len/2 );

#if DEBUG_LOG_SWITCH_TIME
			myTimeTrace_For_Debug(1,(emByte *)"�ִ�--��ֽڵ�", 0);
#endif

#if DEBUG_LOG_SWITCH_RHYTHM_MIDDLE
			if( g_pTextInfo[g_nNextTextByteIndex/2].Len == 0 )
			{
				Print_To_Rhythm("log/��־_����.log", "a", 1,"�ִʣ�");
			}
#endif
			
		}

		
		
		//ָ����1�ڵ�
		g_nCurTextByteIndex += nCurLen;

		//������Ը�ֵ����һ��ǰ����
		nPrevPOS = nNextPOS;
	}


	//���ǵ��־䣬�ҷִʳ����Ĵ�����nrf����ĳɴ���g����ȥ��ƴ��		hyl  2012-04-07
	if( g_pTextInfo[0].Pos == POS_CODE_nrf && g_pTextInfo[0].Len == 2 &&  g_pTextInfo[1].Len == 0 )
	{
		g_pTextInfo[0].Pos = POS_CODE_g;

		if( ( (g_pTextPinYinCode[0]) & 0x8000 ) != 0x8000)	//����ǿ��ƴ��
		{
			g_pTextPinYinCode[0] = 0;
			g_pTextPinYinCode[1] = 0;
		}
	}

	//���ʼ�����ִ�ģ�顷���ͷſռ�
	WordSegUnInit();

//  print_g_pText();
//  print_g_pTextInfo();
//  print_g_pTextPinYinCode();
}

void emCall WordSegInit()
{
	emInt16 i;


	LOG_StackAddr(__FUNCTION__);
	


	//����ÿ���ֵĴ����ռ�
#if DEBUG_LOG_SWITCH_HEAP
	g_vecWordBuffer = (struct WordBuffer *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ), "ÿ�ִ����ռ䣺���ִ�ģ��  ��")  ;	//�ִ�ģ����Ҫ  �洢ÿ���ֵ����д���
#else
	g_vecWordBuffer = (struct WordBuffer *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ))  ;	//�ִ�ģ����Ҫ  �洢ÿ���ֵ����д���
#endif	
	for( i=0; i<MAX_HANZI_COUNT_OF_LINK; i++)
	{
		(g_vecWordBuffer+i)->nItemCount = 0;
	}

	//�������������ƴ���ռ�
#if DEBUG_LOG_SWITCH_HEAP
	g_pHeadWordSegPinYin = (emInt16 *)emHeap_AllocZero(MAX_COUNT_WordSegPinYin , "��������ƴ�������ִ�ģ��  ��");	
#else
	g_pHeadWordSegPinYin = (emInt16 *)emHeap_AllocZero(MAX_COUNT_WordSegPinYin);	
#endif
	g_pCurWordSegPinYin = g_pHeadWordSegPinYin;
	
}

void emCall WordSegUnInit()
{

	//�ͷŶ���������ƴ���ռ�
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_pHeadWordSegPinYin, MAX_COUNT_WordSegPinYin , "��������ƴ�������ִ�ģ��  ��");	
#else
	emHeap_Free(g_pHeadWordSegPinYin, MAX_COUNT_WordSegPinYin);	
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_vecWordBuffer, MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ), "ÿ�ִ����ռ䣺���ִ�ģ��  ��" );
#else
	emHeap_Free(g_vecWordBuffer, MAX_HANZI_COUNT_OF_LINK* sizeof(struct WordBuffer ));
#endif


	

}









//*****************************************************************************************************
//�������ܣ��� ��ǰ�ڵ���ж�Ԫ�ķ���������ȡ��ѷִ�·��
//*****************************************************************************************************
void emCall AmbiguityAnalysis(emUInt8 nMaxOfHanZi,				//��ǰ�ڵ�ĺ��ָ���			
							  emInt16  iPrevPOSIndex,			// ǰ1�ڵ�Ĵ���pos����
							  emInt16  iNextPOSIndex)			// ��1�ڵ�Ĵ���pos����

{
	emInt16  i, j,idxNode,idxFstItem, iItemLength, idxSndItem, iNextItemLength, idxNextNode;

	emInt32  dExpense = MIN_EXPENSE	;			// �����expense�� ��Сֵ
    emInt16  iPrevNode = -1;					// �����β��ǰ1���ڵ�
    emInt16  iPrevItem = -1;					// �����β��ǰ1����
	emInt32  dValue;		

	struct WordItem t1Item;
	emInt16 nBigramVal;
	emInt32 nAllocSize;

	LOG_StackAddr(__FUNCTION__);

	//������Ԫ�ķ���һ����ȡ�뵽�ڴ�
	nAllocSize = MaxLine_Of_OnePos_Of_TableBigram*2;
#if DEBUG_LOG_SWITCH_HEAP
	g_pBigram_Table = (emPByte)emHeap_AllocZero( nAllocSize , "��Ԫ�ķ������ִ�ģ��  ��");			//�����ڴ�ռ�
#else
	g_pBigram_Table = (emPByte)emHeap_AllocZero( nAllocSize);			//�����ڴ�ռ�
#endif
	

	//��ÿ���ڵ��ÿ��������nExpense��ֵΪ��Сֵ:MIN_EXPENSE
	for(i = 0 ; i <nMaxOfHanZi;i++)
	{
		for (j = 0; j < g_vecWordBuffer[i].nItemCount; j++)
		{
			g_vecWordBuffer[i].item[j].nExpense = MIN_EXPENSE;
		}
	}


	//��ʼ����1���ڵ�����д�����nPrevItem��nPrevNode��nExpense����nExpense = ���"w"ת�Ƶ��������Ķ�Ԫ�ķ�ֵ + ��������Ƶ��ratio��
	ReadBigramMoreValue( iPrevPOSIndex );
	for (j = 0; j < g_vecWordBuffer[0].nItemCount; j++)
    {
		g_vecWordBuffer[0].item[j].nPrevItem = 0;  
        g_vecWordBuffer[0].item[j].nPrevNode = -1; 

		//��ȡ��Ԫ�ķ�ֵ
		nBigramVal = GetBigramOneValue( g_vecWordBuffer[0].item[j].nPosIndex );


		g_vecWordBuffer[0].item[j].nExpense = nBigramVal + g_vecWordBuffer[0].item[j].nRatio; 
    }

	//��������forѭ����ͨ���Ƚ�dValue��nExpenseֵ�õ���ѵķִ�·��  
	//		1. nExpense��dValueһ�㶼�Ǹ�ֵ������nExpenseԽ�ӽ���0ֵ��ת��·��Խ�ѡ�
	//		2. ����X1����ת�Ƶ�Y�������������Y������nExpense��aa���������X2����ת�Ƶ�Y������dValue��ֵbb����aa�Ļ�����bb��ֵ��aa��ֵ���ӽ���0ֵ��
	//         ��˵����X2����ת�Ƶ�Y�������ڴ�X1����ת�Ƶ�Y��������Y������nExpenseֵ��aa�ĳ�bb��Y������ǰ1������ָ��X1�����ĳ�ָ��X2������

	//��1��forѭ���� ���������е�ÿ����㣻  ��ǰ�ڵ�idxNode��  �� 0 �� ���к��еĽڵ����� ������������
    for ( idxNode = 0; idxNode < nMaxOfHanZi; idxNode++)
    {

		//��2��forѭ���� ������ǰ�ڵ��ÿ��������  ��ǰ����idxFstItem�� �� 0 �� ���ڵ�Ĵ�������
		for (idxFstItem = 0; idxFstItem < g_vecWordBuffer[idxNode].nItemCount; idxFstItem++)
        {
			iItemLength = g_vecWordBuffer[idxNode].item[idxFstItem].nCharNum ;					//�õ��������Ĵʳ�

			idxNextNode = idxNode+iItemLength;

            if (idxNextNode < nMaxOfHanZi )  //��������û�е����β
            {   
				//��3��forѭ���� ������idxNode���ڵ��е�idxFstItem��������Ӧ����1�ڵ��е����д����� ��1����idxSndItem ���� 0 �� ��1�ڵ�Ĵ�������
				//�赱ǰ�ǵ�[idxNode]���ڵ㣬��ǰ��[idxFstItem]�������Ĵʳ�ΪiItemLength������1�ڵ�Ϊ��[idxNextNode]���ڵ�
				ReadBigramMoreValue( g_vecWordBuffer[idxNode].item[idxFstItem].nPosIndex );
				for ( idxSndItem = 0; idxSndItem < g_vecWordBuffer[idxNextNode].nItemCount; idxSndItem++)
                {                    
					iNextItemLength = g_vecWordBuffer[idxNextNode].item[idxSndItem].nCharNum;					//�õ�����1�����Ĵʳ�
   
					//��ȡ��Ԫ�ķ�ֵ
					nBigramVal = GetBigramOneValue( g_vecWordBuffer[idxNextNode].item[idxSndItem].nPosIndex );

					//dValue = ��ǰ������nExpenseֵ + ��ǰ����ת�Ƶ���1�����Ķ�Ԫ�ķ�ת�Ƹ��� + ��1�����Ĵ�Ƶ��ratioֵ
                    dValue = g_vecWordBuffer[idxNode].item[idxFstItem].nExpense + nBigramVal + g_vecWordBuffer[idxNextNode].item[idxSndItem].nRatio;

					//�� dValueֵ > ��1������nExpenseֵ  ����ʾ����ת�Ƶ���1������·������֮ǰת�Ƶ���1������·�ߣ�
                    if (dValue > g_vecWordBuffer[idxNextNode].item[idxSndItem].nExpense)
                    {
                        g_vecWordBuffer[idxNextNode].item[idxSndItem].nExpense  = dValue;		//��dValueֵ��ֵ����1������nExpenseֵ
                        g_vecWordBuffer[idxNextNode].item[idxSndItem].nPrevNode = idxNode;	//����1������ǰ�ڵ�ָ��ǰ�ڵ�
                        g_vecWordBuffer[idxNextNode].item[idxSndItem].nPrevItem = idxFstItem;	//����1������ǰ����ָ��ǰ����
                    }
                }
            }
            else		//�������պõ����˾�β  ��֮ǰ���п��ƣ����ᳬ����β��
            {   
				ReadBigramMoreValue( 0 );
				//��ȡ��Ԫ�ķ�ֵ
				nBigramVal = GetBigramOneValue( iNextPOSIndex );
				 
				//dValue = ��ǰ������nExpenseֵ + ��ǰ����ת�Ƶ���1�����Ķ�Ԫ�ķ�ת�Ƹ���   ��ע�⣺�����dValueֵ����������1�����Ĵ�Ƶ��ratioֵ��
                dValue = g_vecWordBuffer[idxNode].item[idxFstItem].nExpense + nBigramVal;
                if (dValue > dExpense)  //��ת�Ƶ���β��·������֮ǰת�Ƶ���β��·��
                {
                    dExpense  = dValue;			// ��dValueֵ��ֵ�������β��dExpenseֵ  
                    iPrevNode = idxNode;		// �������β��ǰ�ڵ�ָ��ǰ�ڵ�
                    iPrevItem = idxFstItem;		// �������β��ǰ����ָ��ǰ����
                }
            }
        }
    }

	//�����һ���ڵ���ǰ��������������ѷִ�·���ж�Ӧ�Ĵ�������������1��������λ��  ������Ӧ�����뱾�ڵ�ĵ�1����������λ�ã�
	
    while (iPrevNode != 255)
    {
        //��Ӧ�����뱾�ڵ�ĵ�1����������λ��
		t1Item = g_vecWordBuffer[iPrevNode].item[iPrevItem];
		g_vecWordBuffer[iPrevNode].item[iPrevItem] = g_vecWordBuffer[iPrevNode].item[0];
		g_vecWordBuffer[iPrevNode].item[0] = t1Item;

        iPrevItem = g_vecWordBuffer[iPrevNode].item[0].nPrevItem;
        iPrevNode = g_vecWordBuffer[iPrevNode].item[0].nPrevNode;
    }

	//������Ԫ�ķ������ڴ��ͷ�
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_pBigram_Table,nAllocSize, "��Ԫ�ķ������ִ�ģ��  ��" );
#else
	emHeap_Free(g_pBigram_Table,nAllocSize );
#endif

}


//*****************************************************************************************************
//�������ܣ�������ѷִ�·�ߣ������������еĵ�ǰ�ڵ��ֳɶ���ڵ㣬������ǰ�ڵ�ָ���ֺ�����1���ڵ�
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

		
		if( g_vecWordBuffer[nNode].item[0].pPinYin != NULL )				//�ִʺ�Ĵ˽ڵ� �� �����ʣ��ʵ���������� ���� ǿ�ƶ��������ϱ�
		{
			for( i = 0; i< g_vecWordBuffer[nNode].item[0].nCharNum; i++)
			{
				nPinYin = g_pTextPinYinCode[g_nCurTextByteIndex/2+nNode+i];
				if( nPinYin == 0 )			//��û��ǿ��ƴ�����ǹ��� ��������[=hang2]����
				{
					emMemCpy( &g_pTextPinYinCode[g_nCurTextByteIndex/2+nNode+i], g_vecWordBuffer[nNode].item[0].pPinYin+i  ,2);	//�Ƚ��ִʺ��ƴ����������
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
//�������ܣ����� ��ǰ�ڵ�������ֵ����д�������ʵ䣩
//*****************************************************************************************************
void emCall MatchWordsOfCurNode()
{
	emUInt8 nNodeLen , i ,j;	//
	emUInt8	ch1 , ch2;
	emUInt32 offset;
	emUInt32 nOffsetMore[MAX_SEARCH_WORD_LEN_OF_DICT+1];	//��ǰ�ֵ� 1�ִ� �� 4�ִ� ����ʼƫ����  nOffsetMore[4]: ��һ�ֵ� 1�ִ� ����ʼƫ����
	emUInt16 nLenMore[MAX_SEARCH_WORD_LEN_OF_DICT];	    //��ǰ�ֵ� 1�ִ� �� 4�ִ� ���ֽڳ���
	emUInt32 nCiAllLen;			//��ǰ�� �ڡ��ʵ����  ���д������ܳ���
	emUInt32  t1;
	emPByte  pMem_Save_CurNodeCiTiao;   
	emByte  chGb2312[MAX_SEARCH_WORD_LEN_OF_DICT*2 +4];
	emInt32 nAllocSize;

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	pMem_Save_CurNodeCiTiao = (emPByte)emHeap_AllocZero( 4096, "���ִʵ��д��������ִ�ģ��  ��");	//һ��3K���ˣ��ڴ˿��ٴ�һ�㣬��Ӱ��ѵ����ռ�
#else
	pMem_Save_CurNodeCiTiao = (emPByte)emHeap_AllocZero( 4096);								//һ��3K���ˣ��ڴ˿��ٴ�һ�㣬��Ӱ��ѵ����ռ�
#endif

	nNodeLen = g_pTextInfo[g_nCurTextByteIndex/2].Len;

	for( i=0; i< nNodeLen; i=i+2)
	{
		ch1 = g_pText[g_nCurTextByteIndex+i];			//��ǰ���ֵ� ��1���ֽ�
		ch2 = g_pText[g_nCurTextByteIndex+i+1];		//��ǰ���ֵ� ��2���ֽ�
		

		//��� ��ǰ���� ���� GB2312 ����
		if (  ch1 >= 0xb0 && ch1 <= 0xf7  && ch2 >= 0xa1 && ch2 <= 0xfe )
		{
			offset =  ( ch1 - 0xb0 ) *94 +  ch2 - 0xa1 ;
			offset *= GB2312_DICT_INDEX_LINE_LEN;								

			//���ҡ�GB2312����ƫ�Ʊ����õ���ǰ�ֵ� 1�ִ� �� 4�ִʵ� ��ʼƫ����
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

			if( (offset / GB2312_DICT_INDEX_LINE_LEN) == (6768-1)) //��GB2312�ʵ��������е����һ���֣���		//hyl  2012-03-23
				nCiAllLen = 0;
			else
				nCiAllLen = nOffsetMore[MAX_SEARCH_WORD_LEN_OF_DICT]  - nOffsetMore[0];

			if( nCiAllLen != 0)		//�е�ǰ�ֵĴ���
			{

				//�����ʵ���� ��ǰ�� �����д��� һ���� ���뵽 �ڴ�     ��3������ڴ�����ʱռ���ִ�ģ��--���ʡ���58%  (��50�ֵĺϳɣ������ʺ�ʱ84����3��ռ49)
				nAllocSize = nCiAllLen;

				
				fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnPt + nOffsetMore[0] , 0);	
				fFrontRead(pMem_Save_CurNodeCiTiao, sizeof(emCharA), nCiAllLen, g_hTTS->fResFrontMain );


				//�õ� ��ǰ�� ����� �ڴ��� �����д����� 1�ִ� �� 4�ִʵ� ��ʼƫ����
				for( j = MAX_SEARCH_WORD_LEN_OF_DICT; j > 0 ; j--)
				{
					nOffsetMore[j] = nOffsetMore[j] - nOffsetMore[0];				
				}
				nOffsetMore[0] = 0;
				for( j = 0; j < MAX_SEARCH_WORD_LEN_OF_DICT ; j++)
				{
					nLenMore[j] = nOffsetMore[j+1] - nOffsetMore[j];				
				}

				//���� ��ǰ�� �� ���д���   �˺�ʱռ���ִ�ģ��--���ʡ���36%  (��50�ֵĺϳɣ������ʺ�ʱ84����3��ռ30)
				GetAllCiTiaoOfCurZi(pMem_Save_CurNodeCiTiao ,nOffsetMore,nLenMore, nCiAllLen , i );			




				if( g_vecWordBuffer[i/2].nItemCount == 0  )		//û��ƥ��Ĵ���������1������
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
			else		//�޵�ǰ�ֵĴ�������ֻ����1������
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
		else		//��ǰ���� ������ GB2312 ���֣� ��ֻ����1������
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
	emHeap_Free(pMem_Save_CurNodeCiTiao, 4096, "���ִʵ��д��������ִ�ģ��  ��");
#else
	emHeap_Free(pMem_Save_CurNodeCiTiao, 4096);
#endif
}


//****************************************************************************************************
//�������ܣ� ���� ��ǰ�� �� ���д��� ���� 4�ִ� �� 1�ִʣ�
//				������  pMem_Save_CurNodeCiTiao��	�ӡ��ʵ���� ȡ��� ��ǰ�ֵ� ���д����� �ڴ�ָ��
//				������  nOffsetMore��				��ǰ�ֵ� 1�ִ� �� 4�ִʵ� ����ڲ���1�� ��ʼƫ����
//				������  nLenMore��					��ǰ�ֵ� 1�ִ� �� 4�ִʵ� �ֱ�� ��������
//				������	nCiAllLen ��				�ӡ��ʵ���� ȡ��� ��ǰ�ֵ� ���д����� �ֽ��ܳ���
//				������  nNo��						��ǰ���� ���ڵ� �� �ڼ����ֽ�
// ����������д��� д�뵽 g_vecWordBuffer[i]
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

	//��ȡ ��ǰ�ֵ� ���� ���ϴ���	
	GetXingCiTiaoOfCurZi(nNo, itemXing1, itemXing2 );


	nCountOfItem = 0;

	nSearchMaxWord = (g_pTextInfo[g_nCurTextByteIndex/2].Len - nNo )/2;	    //��ǰ������ʼʣ��ĺ��ָ���
	if( nSearchMaxWord > MAX_SEARCH_WORD_LEN_OF_DICT)
	{
		nSearchMaxWord = MAX_SEARCH_WORD_LEN_OF_DICT;
	}

	// ��4�ִʣ���ʣ�຺�ֵ�����ִʣ� ������ 1�ִ�
	for(i = nSearchMaxWord -1; i>=0;i--)
	{

		//���� �ʵ����
		nLen = (i+1)*2;
		if( *(nLenMore + i) > 0)			//���ֿ�ͷ��  �ü��ִ� ���� ��1�� ����
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

				//�ҵ����ֿ�ͷ�� �ü��ִʵ� ����
				if( emMemCmp( nCurOffset ,&g_pText[g_nCurTextByteIndex+ nNo +2]   ,nLen-2) == 0 )
				{
					nCurOffset +=  (nCountOfWord -1)*2 ;
					if( IsPloyOrDuoCiTiao == 1)  //�Ƕ��������������� ����
					{
						nCurOffset +=  1; 
					}
					
					//��ȡ ����
					for( j=0; j<nCountOfCitiao; j++)
					{
						g_vecWordBuffer[nNo/2].item[ nCountOfItem].nPosIndex = (*(nCurOffset)) & 0x7f;
						g_vecWordBuffer[nNo/2].item[ nCountOfItem].nRatio = ((*(nCurOffset+2))<<8 )+ (*(nCurOffset+1));
						g_vecWordBuffer[nNo/2].item[ nCountOfItem].nCharNum = i+1;
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
						emMemCpy(g_vecWordBuffer[nNo/2].item[ nCountOfItem].pAAAAAAText , &g_pText[g_nCurTextByteIndex + nNo],nLen);
						*(g_vecWordBuffer[nNo/2].item[ nCountOfItem].pAAAAAAText + nLen ) ='\0';
#endif
						if( ((*(nCurOffset)) >> 7) == 1 )  //�Ƕ�������
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
				else			//û�ҵ�����
				{
					if( IsPloyOrDuoCiTiao == 1)  //�Ƕ��������������� ����
					{
						nCurOffset +=  (nCountOfWord -1)*2 ;
						nCurOffset +=  *(nCurOffset) +1; 
					}
					else							//������ֻ��1���������ǻ�������
					{
						nCurOffset +=  (nCountOfWord -1)*2 + 3;
					}
				}
			}
		}

		
		//�������ϴ���
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
//�������ܣ� ��ȡ ��ǰ�� �� �������ϴ���
//				������  nNo��			��ǰ���� ���ڵ� �� �ڼ����ֽ�
//���أ�
//				������  xingItem1��		��ǰ�ֵ� ���ϵ�1������2��������
//				������  xingItem2:		��ǰ�ֵ� ���ϵ�2������3��������
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

	//ʣ�຺��>=2�����������2�ֻ�3������  ��Ŀǰֻ����1�ֵ��գ��ݲ�����2�ֵ��գ���Ȼ�����ϱ����У���
	if( countOfRemainHanZi >= 2 )	
	{
		emMemCpy( curAndNextZi, &g_pText[g_nCurTextByteIndex+nNo] , 2);
		curAndNextZi[2] = 0;
		curAndNextZi[3] = 0;
		offsetLine = ErFenSearchTwo_FromRAM( curAndNextZi, 0, g_pXing_Table, 1, g_MaxLineOfTableSurname ,BYTE_NO_OF_TABLE_SURNAME_ONE_LINE);	//���ַ�����

		if( offsetLine != g_MaxLineOfTableSurname)		//�ҵ��뵱ǰ����ͬ����
		{
			//��λ�����ҵ��е�Ratio
			pCurXing_Table = g_pXing_Table + offsetLine*BYTE_NO_OF_TABLE_SURNAME_ONE_LINE + 4;
			nRatio = ( *(pCurXing_Table+1))*256 +( *(pCurXing_Table));
			nPinYin = ( *(pCurXing_Table+3))*256 +( *(pCurXing_Table+2));

			
			//����2������
			xingItem1->nCharNum = 2;
			xingItem1->nPosIndex = POS_CODE_nr;	//����
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
			emMemCpy(xingItem1->pAAAAAAText,&g_pText[g_nCurTextByteIndex+nNo] , 4);
			*(xingItem1->pAAAAAAText + 4)='\0';
#endif
			xingItem1->nRatio = nRatio - RATIO_NEED_CUT_OF_TWO_NAME;		//������������ʱ���������ratioֵ(Ȩ��ֵ)		
			if( nPinYin != 0)
			{
				xingItem1->pPinYin = g_pCurWordSegPinYin;
				g_pCurWordSegPinYin += xingItem1->nCharNum;

				//*(xingItem1->pPinYin) =  nPinYin | 0x8000	;		//���϶�����ǿ��ƴ������
				*(xingItem1->pPinYin) =  nPinYin 	;
				*(xingItem1->pPinYin+1) = 0;
			}
			else
			{
				xingItem1->pPinYin = NULL;

			}

			if( countOfRemainHanZi >=3 )	
			{
				//����3������
				xingItem2->nCharNum = 3;
				xingItem2->nPosIndex = POS_CODE_nr;		//����
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
				emMemCpy(xingItem2->pAAAAAAText,&g_pText[g_nCurTextByteIndex+nNo] , 6);
				*(xingItem2->pAAAAAAText + 6)='\0';
#endif
				xingItem2->nRatio = nRatio - RATIO_NEED_CUT_OF_THREE_NAME;		//������������ʱ���������ratioֵ(Ȩ��ֵ)		
				if( nPinYin != 0)
				{
					xingItem2->pPinYin= g_pCurWordSegPinYin;
					g_pCurWordSegPinYin += xingItem2->nCharNum;

					//*(xingItem2->pPinYin) =  nPinYin | 0x8000	;		//���϶�����ǿ��ƴ������
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

//��ȡ��nPrevPosת�Ƶ�126��Pos��Ԫ�ķ�ֵ
void emCall ReadBigramMoreValue(emUInt8 nPrevPos)
{
	LOG_StackAddr(__FUNCTION__);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_ConvertRatioOfGram + nPrevPos * MaxLine_Of_OnePos_Of_TableBigram*2,  0);					
	fFrontRead( g_pBigram_Table,MaxLine_Of_OnePos_Of_TableBigram*2 , 1, g_hTTS->fResFrontMain);	
}


//�����е�126����Ԫ�ķ�ֵ�л�ȡnNextPos��Ӧ�Ķ�Ԫ�ķ�ֵ
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


//�����û��ʵ���Դ�Ĵ���
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
	for( i=0; i< (nNodeLen-2); i=i+2)				//��ǰ���ֽڵ��к������ѭ��
	{
		//�ڡ��û��ʵ���Դ���е����������ҵ����ָ��ֽڿ�ͷ�Ĵ��������
		nIndex = ((*(g_pText+i)) - 0xB0)*2;
		fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnUser+nIndex  , 0);	
		fFrontRead(nT1, 2, 2, g_hTTS->fResFrontMain );
		nWordCount = nT1[1] - nT1[0];

		if( nWordCount != 0)
		{
			//�Ա��ָ��ֽڿ�ͷ���д�������ƥ��
			for( j = nT1[0]; j< nT1[1]; j++)
			{
				fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_DictCnUser+73*2+ nT1[0]*18 , 0);	
				fFrontRead(&curWordLine, sizeof(struct tagWordLine), 1, g_hTTS->fResFrontMain );

				tt1 = strlen((char *)curWordLine.word);
				tt2 = nNodeLen - i;
				if( tt2 >= tt1)
				{
					if( emMemCmp( curWordLine.word , g_pText+i, tt1) == 0 )		//ƥ��
					{
						kk1 = i/2;
						kk2 = g_vecWordBuffer[kk1].nItemCount;

						//��ֵ���������ȣ�Ratio�����ԣ�ƴ��
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
