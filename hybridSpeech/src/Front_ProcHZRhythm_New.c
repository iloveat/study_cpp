
#include "emPCH.h"

#include "Front_ProcHZRhythm_New.h"





void emCall Rhythm_New()		//�����·���
{
	emInt32  i = 0;
	emInt16 allNodes;


	PRhythmRes pRhythmRes;
	PRhythmRT	pRhythmRT;
	emInt8 nIndex,j;

	LOG_StackAddr(__FUNCTION__);

	/* �����ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
	pRhythmRT = emHeap_AllocZero( sizeof(struct tagRhythmRT), "���ɷ���ʱ�⣺������ģ��  ��");
#else
	pRhythmRT = emHeap_AllocZero( sizeof(struct tagRhythmRT));
#endif
	//emMemSet(pRhythmRT,0,sizeof(struct tagRhythmRT));

#if DEBUG_LOG_SWITCH_HEAP
	pRhythmRT->m_tSylInfo = emHeap_AllocZero( 256*sizeof(TCESylInfo), "���ɷ���������Ϣ��������ģ��  ��");
#else
	pRhythmRT->m_tSylInfo = emHeap_AllocZero( 256*sizeof(TCESylInfo));
#endif
	//emMemSet(pRhythmRT->m_tSylInfo,0,256*sizeof(TCESylInfo));

#if DEBUG_LOG_SWITCH_HEAP
	pRhythmRes = emHeap_AllocZero( sizeof(struct tagRhythmRes) , "���ɷ�����Դ��Ϣ��������ģ��  ��");
#else
	pRhythmRes = emHeap_AllocZero( sizeof(struct tagRhythmRes));
#endif
	//emMemSet(pRhythmRes,0,sizeof(struct tagRhythmRes));

#if DEBUG_LOG_SWITCH_HEAP
	g_ResPWRule=emHeap_AllocZero(sizeof(struct tagRuleRes),  "PW������Դ��������ģ��  ��" );
#else
	g_ResPWRule=emHeap_AllocZero(sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPWRule->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack),  "PW������Դ����������ģ��  ��" );
#else
	g_ResPWRule->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHC45=emHeap_AllocZero(sizeof(struct tagRuleRes),  "PPH��ԴC45��������ģ��  ��" );
#else
	g_ResPPHC45=emHeap_AllocZero(sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHC45->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack),  "PPH��ԴC45����������ģ��  ��" );
#else
	g_ResPPHC45->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHRatio=emHeap_AllocZero(sizeof(struct tagRuleRes),  "PPH������Դ��������ģ��  ��" );
#else
	g_ResPPHRatio=emHeap_AllocZero(sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHRatio->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack),  "PPH������Դ����������ģ��  ��" );
#else
	g_ResPPHRatio->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack));
#endif


	Front_Rhythm_RT_To_Link( pRhythmRT);				//�������� ת pRhythmRT	


	Front_Rhythm_PW_Init( pRhythmRes);

#if PPH_AFTER_DEAL							//PPH�߽绮�ֺ���
	//����PWƴ��ǰ��ԭʼ����
	//����PWƴ��ǰ��ԭʼRhythm
	allNodes = GetSylCount(pRhythmRT);
	for(i = 0 ; i < allNodes; i++)
	{
		pRhythmRT->m_tPoSBeforePW[i]    = pRhythmRT->m_tSylInfo[i].m_tPoS;	
		pRhythmRT->m_tRhythmBeforePW[i] = pRhythmRT->m_tRhythm[i];
	}
#endif 

	Front_Rhythm_PW( pRhythmRes, pRhythmRT);			//���ݹ���ƴ�ӡ����ɴʡ���Ҳ��ı��﷨�ʵĴʳ�

#if DEBUG_LOG_SWITCH_TIME
	myTimeTrace_For_Debug(1,(emByte *)"����--ƴ���ɴ�", 0);
#endif

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	//Print_To_Rhythm("log/��־_����.log", "a", 1,"");
#endif

	//�жϡ����ɱ�ע���ԡ�
	if( g_hTTS->m_ControlSwitch.m_nManualRhythm == emTTS_USE_Manual_Rhythm_OPEN)
	{
		//�ֶ���ע����
		for( i= 0; i< MAX_MANUAL_PPH_COUNT_IN_SEN; i++)
		{
			nIndex = g_hTTS->m_ManualRhythmIndex[i];
			if( nIndex != 0)
			{
				//���ֶ���ע�����óɣ����ɶ���߽�
				pRhythmRT->m_tSylInfo[nIndex].m_tBoundary = BdBreath;

				//�������ɴ��ڲ��ֶ�������PPH�߽磬��ǿ���޸����ɴʳ�
				for( j = nIndex-1;; j--)
				{
					//�������䣺	[z1]��#�ǵĳ�ŵ��#����ÿһ#��ϸ#�ڵ���#����  
					//�������䣺	[z1]��#��#��#��#ŵ#��#��#��#ÿ#һ#��#ϸ#��#��#��#����
					if( pRhythmRT->m_tRhythm[j] > 0)
					{
						//���ɴ�û�б��и������  (pRhythmRT->m_tRhythm[j]&0x0F)Ϊ���ɴʳ�
						//���䣺	[z1]���ǵĳ�ŵ#������#ÿһ��ϸ�ڵ�������
						if( (pRhythmRT->m_tRhythm[j]&0x0F) == (nIndex - j) )	
						{
							break;
						}

						//������������ĵڶ���#���﷨�ʱ��и��	��Ƕ��ʽ����һ���﷨�ʣ�Ҳ��һ�����ɴ�
						//���䣺	[z1]��ӭʹ#�����������з���Ƕ��#ʽ�����ϳ�ϵͳ
						if( pRhythmRT->m_tWordLen[j] != (nIndex - j) )
						{
							pRhythmRT->m_tRhythm[nIndex]   = pRhythmRT->m_tRhythm[j] - (nIndex - j);
							pRhythmRT->m_tWordLen[nIndex]  = pRhythmRT->m_tWordLen[j] - (nIndex - j);

							pRhythmRT->m_tRhythm[j]  = (nIndex - j);
							pRhythmRT->m_tWordLen[j]  = (nIndex - j);
							break;
						}

						//������������ĵ�һ��#�����ɴʱ��и��	��ʹ�á���һ�����ɴʣ���ʹ���͡��á��ֱ����﷨��
						//���䣺	[z1]��ӭʹ#�����������з���Ƕ��#ʽ�����ϳ�ϵͳ
						if( pRhythmRT->m_tWordLen[j] != (pRhythmRT->m_tRhythm[j]&0x0F)  )
						{
							pRhythmRT->m_tRhythm[nIndex]   = pRhythmRT->m_tRhythm[j] - pRhythmRT->m_tWordLen[j];							
							pRhythmRT->m_tRhythm[j]  = pRhythmRT->m_tWordLen[j];
							break;
						}

						break;
						
					}

				}
			}
		}
	}
	else
	{
		//�����Զ���ע����
		Front_Rhythm_PPH_Init(  pRhythmRes );
		Front_Rhythm_PPH( pRhythmRes,pRhythmRT);			//�����������ɶ��
	}



	Front_Rhythm_Link_To_RT( pRhythmRT);				//pRhythmRT ת �������� 

//#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
//	Print_To_Rhythm("log/��־_����.log", "a", 1,"");
//#endif

	

	/* �ͷ��ڴ� */	
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHRatio->m_pResPack, sizeof(struct tagResPack),  "PPH������Դ����������ģ��  ��" );
#else
	emHeap_Free(g_ResPPHRatio->m_pResPack, sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHRatio, sizeof(struct tagRuleRes),  "PPH������Դ��������ģ��  ��" );
#else
	emHeap_Free(g_ResPPHRatio, sizeof(struct tagRuleRes));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHC45->m_pResPack, sizeof(struct tagResPack),  "PPH��ԴC45����������ģ��  ��" );
#else
	emHeap_Free(g_ResPPHC45->m_pResPack, sizeof(struct tagResPack) );
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHC45, sizeof(struct tagRuleRes),  "PPH��ԴC45��������ģ��  ��" );
#else
	emHeap_Free(g_ResPPHC45, sizeof(struct tagRuleRes));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPWRule->m_pResPack,sizeof(struct tagResPack),  "PW������Դ����������ģ��  ��" );
#else
	emHeap_Free(g_ResPWRule->m_pResPack,sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPWRule,sizeof(struct tagRuleRes),  "PW������Դ��������ģ��  ��" );
#else
	emHeap_Free(g_ResPWRule,sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRhythmRes, sizeof(struct tagRhythmRes) , "���ɷ�����Դ��Ϣ��������ģ��  ��");
#else
	emHeap_Free( pRhythmRes, sizeof(struct tagRhythmRes));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRhythmRT->m_tSylInfo, 256*sizeof(TCESylInfo), "���ɷ���������Ϣ��������ģ��  ��");
#else
	emHeap_Free( pRhythmRT->m_tSylInfo, 256*sizeof(TCESylInfo));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRhythmRT,sizeof(struct tagRhythmRT), "���ɷ���ʱ�⣺������ģ��  ��");
#else
	emHeap_Free( pRhythmRT,sizeof(struct tagRhythmRT));
#endif


#if DEBUG_LOG_SWITCH_TIME
	myTimeTrace_For_Debug(1,(emByte *)"����--��������", 0);
#endif

}


// ************************************************************************************************************************** 
// ǿ�Ƶ���PPH��1.���ݡ��ɶԱ��ǿ��PPH��Ϣ��  ��	�����ݣ��ɶԱ���ǰ����λ�� �� ǰ����֮������ڸ���   ������PPH
// Ŀǰ��֧��һ�ԳɶԱ���ʶ���������Լ����ϱ�㣬��ʶ�����һ��
//                                                                                                                            
//  /��Ϊ����ķ�/�ء������ࡱ/									/��Ϊ����ķɿ�/�������ࡱ/
//	/һֻ����/һֱ����/���ҵ��Ժ��С�������/��û�г���������/		/һֻ����/һֱ����/���ҵ��Ժ���/����������û�г���������/
//	/�����˰˺š������̴�5������/���ع��ӽ�����ˮ������Ϯ��/		/�����˰˺š�/�����̴�5������/���ع��ӽ�����ˮ������Ϯ��/
//      
//ǿ�Ƶ���PPH��2.���ֶ��������			���磺/����/����մմ��ϲ������/�ڲ�֪��������/��Ⱦ��������/��(��1�����Ͻ����)
//										���磺/��ǰ/����������֮��/��
// **************************************************************************************************************************                                                    */


void emCall Rhythm_ForcePPH_Basic()
{
//ǿ��PPH�ĵ�������:
//											//�ɶԱ�㣺����1���������������>1,��ǰ��㲻�Ǿ��ף�����ǰ��㴦����������
//											//�ɶԱ�㣺����2����ǰ���ͺ���֮���PPH��������ɾ��
#define PPH_PREV_COUNT_DELETE	4			//�ɶԱ�㣺����3����ǰ���ǰ�ļ��������ڳ��ֵ�PPHֱ��ɾ��
#define PPH_NEXT_COUNT_MOVE		4			//�ɶԱ�㣺����4���ں����ļ��������ڳ��ֵ�PPH�����Ƶ����㴦
#define PPH_NEXT_COUNT_INSERT	7			//�ɶԱ�㣺����5���ں����ļ���������δ����PPH����ֱ���ں��㴦����PPH
#define PPH_FINAL_COUNT_INSERT	12			//�ɶԱ�㣺����6��ִ�������Ϲ������ǰ�����֮���PPH�����������ڼ����֣���ֱ���ں��㴦����PPH
	emInt16 i,count=0, nPPHSylCount;

	LOG_StackAddr(__FUNCTION__);

	//ǿ�Ƶ���PPH��1.���ݡ��ɶԱ��ǿ��PPH��Ϣ��  ��
	if( g_ForcePPH.nSylLen > 0)
	{
		//ʵ�֣�����2
		if(g_ForcePPH.nSylLen <= 10 )		//����ǰ����������<=10��֮���PPHȫ��ɾ��
		{
			for( i = g_ForcePPH.nStartIndex+1 ; i<(g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen-1 );i++)
			{
				if( g_pTextInfo[i].BorderType == BORDER_PPH)
				{
					g_pTextInfo[i].BorderType = BORDER_PW;
				}
			}
		}
		else								//����ǰ����������>10��
		{
			//ǰ�����4�������ڵ�PPHɾ����
			for( i = g_ForcePPH.nStartIndex+1 ; i<=(g_ForcePPH.nStartIndex + 4 );i++)
			{
				if( g_pTextInfo[i].BorderType == BORDER_PPH)
				{
					g_pTextInfo[i].BorderType = BORDER_PW;
				}
			}
			//����ǰ��4�������ڵ�PPHɾ��
			for( i = (g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen-1 ) ; i>=(g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen -4);i--)
			{
				if( g_pTextInfo[i].BorderType == BORDER_PPH)
				{
					g_pTextInfo[i].BorderType = BORDER_PW;
				}
			}
		}

		//ʵ�֣�����3
		count=0;
		for( i = g_ForcePPH.nStartIndex ; i>0; i--)
		{
			if( g_pTextInfo[i].BorderType == BORDER_PPH)
			{
				g_pTextInfo[i].BorderType = BORDER_PW;
			}
			count++;
			if( count>PPH_PREV_COUNT_DELETE)
				break;
		}

		//ʵ�֣�����1
		if( g_ForcePPH.nSylLen != 1 && g_ForcePPH.nStartIndex != 0)
			g_pTextInfo[g_ForcePPH.nStartIndex ].BorderType = BORDER_PPH;

		//ͳ�ƺ��㴦��֮���PPH�߽�֮���ж��ٸ����ڣ�������count��
		count=0;
		for( i = g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen; i< g_nLastTextByteIndex/2 ;i++)
		{
			count++;
			if( g_pTextInfo[i+1].BorderType == BORDER_PPH)
			{
				break;
			}				
		}

		//ʵ�֣�����4
		if( count <= PPH_NEXT_COUNT_MOVE && i != (g_nLastTextByteIndex/2) )
		{
			g_pTextInfo[g_ForcePPH.nStartIndex+g_ForcePPH.nSylLen ].BorderType = BORDER_PPH;

			g_pTextInfo[i+1].BorderType = BORDER_PW;
		}

		//ʵ�֣�����5
		if( count > (PPH_NEXT_COUNT_INSERT) )
		{
			g_pTextInfo[g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen ].BorderType = BORDER_PPH;
		}

		//ͳ��ǰ��㴦��֮���PPH�߽�֮���ж��ٸ����ڣ�������count��
		count=0;
		for( i = g_ForcePPH.nStartIndex ; i< g_nLastTextByteIndex/2 ;i++)
		{
			count++;
			if( g_pTextInfo[i+1].BorderType == BORDER_PPH)
			{
				break;
			}				
		}

		//ʵ�֣�����6
		if( count > (PPH_FINAL_COUNT_INSERT) )
		{
			g_pTextInfo[g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen ].BorderType = BORDER_PPH;
		}
	}


	//ǿ�Ƶ���PPH��2.���ֶ��������	
	count = 0;
	nPPHSylCount = 0;
	//�ж�
	if( g_hTTS->m_ControlSwitch.m_nManualRhythm == emTTS_USE_Manual_Rhythm_CLOSE)  //�����ֶ����ɱ�ע�ǹرյ�
	{
		while(g_pTextInfo[count].Len != 0 && count<=MAX_HANZI_COUNT_OF_LINK)
		{
			if( g_pTextInfo[count].BorderType == BORDER_PPH)
			{
				if(    nPPHSylCount<=2							//ǰ1���ɶ��ﳤ��<=2
					|| (g_nLastTextByteIndex/2 - count) <= 2 )	//���ɶ���߽��ھ�β��2����
				{
					g_pTextInfo[count].BorderType = BORDER_PW;	//���ֶ��������
				}

				nPPHSylCount = g_pTextInfo[count].Len/2;		
			}
			else
			{
				nPPHSylCount += g_pTextInfo[count].Len/2;			
			}
			count += g_pTextInfo[count].Len/2;
		}
	}

}

void Front_Rhythm_RT_To_Link( PRhythmRT  pRhythmRT)			//��������  ת  pRhythmRT
{
	emUInt8 nWordCount = 0, i = 0,j;  

	emConst emUInt8 nNewPos[128] =					//����ת����
   {0,	1,	3,	1,	1,	1,	1,	2,	3,	3,			//  0 -  9     ע�⣺adת��d���ã���adת��a�ã�
	3,	3,	3,	3,	3,	3,	19,	19,	4,	4,			// 10 - 19
	0,	14,	1,	1,	1,	1,	1,	1,	6,	13,			// 20 - 29
	6,	6,	3,	6,	6,	6,	15,	1,	1,	1,			// 30 - 39
	1,	3,	1,	6,	13,	5,	5,	5,	6,	6,			// 40 - 49
	6,	6,	6,	20,	6,	6,	6,	6,	6,	6,			// 50 - 59		ע�⣺nrf��nrgת��n���ã���nrf��nrgת��ngp�ã�
	6,	16,	8,	8,	9,	9,	9,	9,	9,	9,			// 60 - 69
	9,	9,	9,	9,	9,	9,	17,	17,	18,	17,			// 70 - 79
	17,	10,	11,	11,	11,	12,	12,	12,	12,	12,			// 80 - 89
	12,	12,	12,	12,	13,	13,	13,	13,	13,	13,			// 90 - 99
	6,	13,	13,	13,	13,	13,	0,	0,	0,	0,			//100 -109
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,			//110 -119
	0,	0,	19,	19,	1,	1,	0,	0};					//120 -127

	LOG_StackAddr(__FUNCTION__);

	g_nP1TextByteIndex = 0;

	pRhythmRT->m_nTextLen = 0;
	pRhythmRT->m_nSylCount = 0 ;

	i = 0;
	pRhythmRT->m_cText = g_pText;
	while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0)
	{		
		nWordCount = g_pTextInfo[g_nP1TextByteIndex/2].Len / 2;  //ͳ������
		pRhythmRT->m_nTextLen += nWordCount*2;
		pRhythmRT->m_nSylCount += nWordCount ;

		//дÿ���ʵĴ�ͷ��Ϣ
		pRhythmRT->m_tRhythm[i] = nWordCount;
		pRhythmRT->m_tWordLen[i] = nWordCount;

		/*  ��ʼ��m_nFrCost����4�ֽ��һ��һ����=60,=70,=80, = (nRatio+10000)/100)���� ��=0 �� =60 �������һЩ���������PPH̫�飩   */
		pRhythmRT->m_nFrCost[i] = 60;								//������ 
		pRhythmRT->m_tSylInfo[i].m_iSylText = i*2;
		pRhythmRT->m_tSylInfo[i].m_tBoundary = BdNull;
		pRhythmRT->m_tSylInfo[i].m_tTone = 0;
		pRhythmRT->m_tSylInfo[i].m_tPoS = nNewPos[g_pTextInfo[g_nP1TextByteIndex/2].Pos];		//������
		i++;

		for(j = 1;j<nWordCount;j++)
		{
			//дÿ���ʵķǴ�ͷ��Ϣ
			pRhythmRT->m_tRhythm[i]  = -1;
			pRhythmRT->m_tWordLen[i] = -1;
			pRhythmRT->m_nFrCost[i] = 60;						//������ 
			pRhythmRT->m_tSylInfo[i].m_iSylText = i*2;
			pRhythmRT->m_tSylInfo[i].m_tBoundary = BdNull;
			pRhythmRT->m_tSylInfo[i].m_tTone = 0;
			pRhythmRT->m_tSylInfo[i].m_tPoS = 0;
			i++;
		}
		g_nNextTextByteIndex = g_nP1TextByteIndex + g_pTextInfo[g_nP1TextByteIndex/2].Len;
		g_nP1TextByteIndex = g_nNextTextByteIndex;
	}

	//д��β��Ϣ
	pRhythmRT->m_tRhythm[i]  = 0;
	pRhythmRT->m_tWordLen[i] = 0;
	pRhythmRT->m_nFrCost[i] = 0;
	pRhythmRT->m_tSylInfo[i].m_iSylText = i*2;
	pRhythmRT->m_tSylInfo[i].m_tBoundary = BdNull;
	pRhythmRT->m_tSylInfo[i].m_tTone = 0;
	pRhythmRT->m_tSylInfo[i].m_tPoS = 0;
	i++;
}



void Front_Rhythm_Link_To_RT(PRhythmRT pRhythmRT)		//pRhythmRT ת �������� 
{
	emInt16 i = 0,count, nRhythmLen;
	emInt8  nOldNextLen, nNewCurLen, nNewNextLen;	

	emUInt8 nBefore = 0, nNext = 0, nCur = 0; 
	emUInt8 nNew = 0;

	LOG_StackAddr(__FUNCTION__);

	nCur = 0;
	nBefore = nCur;

	while( i < pRhythmRT->m_nSylCount )
	{		
		if( pRhythmRT->m_tRhythm[i] > 0 )
		{
			count = 0;

			count += (g_pTextInfo[nCur/2].Len)/2;
			nRhythmLen = ((pRhythmRT->m_tRhythm[i])&0x0F);
			while( count <  nRhythmLen)
			{
				if( pRhythmRT->m_tSylInfo[i+count].m_tBoundary == BdBreath )
				{
					g_pTextInfo[nCur/2 + count].BorderType = BORDER_PPH;
				}
				
				g_nNextTextByteIndex = nCur + g_pTextInfo[nCur/2].Len;
				nCur = g_nNextTextByteIndex;
				count += (g_pTextInfo[nCur/2].Len)/2;
			}
			
			if( (i+nRhythmLen) == pRhythmRT->m_nSylCount  )
			{
				break;
			}

			g_nNextTextByteIndex = nCur + g_pTextInfo[nCur/2].Len;
			nNext = g_nNextTextByteIndex;

			if( count >  nRhythmLen )			//������ϣ�4�ִ�+1�ִʣ�����ɣ�2�ִ�+3�ִ� �ȵ�
			{
				nOldNextLen = g_pTextInfo[nNext/2].Len;
				nNewCurLen	= nRhythmLen*2;
				nNewNextLen = nOldNextLen + ( count - nRhythmLen )*2;

				g_pTextInfo[nCur/2].Len = nNewCurLen;				
				g_pTextInfo[(nCur + nNewCurLen)/2].Len = nNewNextLen;
				g_pTextInfo[(nCur + nNewCurLen)/2].Pos = g_pTextInfo[nNext/2].Pos;
				g_pTextInfo[(nCur + nNewCurLen)/2].BorderType = g_pTextInfo[nNext/2].BorderType;
				g_pTextInfo[(nCur + nNewCurLen)/2].TextType= g_pTextInfo[nNext/2].TextType;
				g_pTextInfo[(nCur + nNewCurLen)/2].nBiaoDian[0] = g_pTextInfo[nCur/2].nBiaoDian[0];
				g_pTextInfo[(nCur + nNewCurLen)/2].nBiaoDian[1] = g_pTextInfo[nCur/2].nBiaoDian[1];
				g_pTextInfo[nCur/2].nBiaoDian[0] = 0;
				g_pTextInfo[nCur/2].nBiaoDian[1] = 0;
				nNext = nCur + nNewCurLen;
			}			

			if( pRhythmRT->m_tSylInfo[i+nRhythmLen].m_tBoundary == BdBreath )
			{
				g_pTextInfo[nNext/2].BorderType = BORDER_PPH;
			}
			else
			{
				g_pTextInfo[nNext/2].BorderType = BORDER_PW;
			}		
			nCur = nNext;
			i += nRhythmLen;			
		}
	}
}