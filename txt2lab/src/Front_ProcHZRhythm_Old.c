#include "emPCH.h"

#include "Front_ProcHZRhythm_Old.h"



#if	!EM_SYS_SWITCH_RHYTHM_NEW

//�����Ϸ��������ţ� PW��0.55�� + PPH��0.90,0.65�� + PPH������

void emCall Rhythm_Old()
{

	LOG_StackAddr(__FUNCTION__);

			//Each8ToPau_For_Debug();

			//����Ԥ���1��
			EvalPW(0.55);					//Ŀǰ��0.55���ǲ��Ե����ֵ



		#if DEBUG_LOG_SWITCH_RHYTHM_MIDDLE
			Print_To_Rhythm("log/��־_����.log", "a", 1,"��PW��");
		#endif



			//����Ԥ���2��
			EvalPPH(0.90, 0.65);				//Ŀǰ��0.90��0.65���ǲ��Ե����ֵ
		


		#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
			Print_To_Rhythm("log/��־_����.log", "a", 1,"��PH��");
		#endif

}



emBool IsHaveTeZhengCi( emUInt8  nCurNode )		
{

	emCharA* beforeCiOne[]	= { "��", "˵", "��" };

	emCharA* afterCiOne[]	= { "��", "��","��","��","��","��","��","��","��","��","��","��","��","��",
							"��","ȴ","��","��","��","ͬ","��","��","��","��","��","��"};

	emCharA* afterCiTwo[]	= { "�ȷ�","����","����","����","����","����","����","����","����",
							"����","����","����","����","����","����","��Ȼ","�ñ�","�ο�",
							"����","��ʹ","��Ȼ","����","��ʹ","����","����","����","����",
							"����","�Ǿ�","Ʃ��","ƫƫ","��֪","Ȼ��","���","��ͬ","����",
							"�ǹ�","˵��","�ƺ�","��Ȼ","����","����","Ҫ��","һ��","һ��",
							"�Ա�","�Լ�","����","���","���","��Ϊ","����","����","��ͬ",
							"ԭ��","ֻ��","����","��Ȼ"};

	emInt32  i;
	emUInt nNextNode, nNextNextNode;

	LOG_StackAddr(__FUNCTION__);

	//�ж�PPH�ĵ��֡�ǰ�����ʡ�
	if( g_pTextInfo[nCurNode/2].Len== 2 )
	{
		i = 0;
		while( i < ( sizeof(beforeCiOne)/ sizeof(emCharA *) ))
		{
			if( emMemCmp( &g_pText[nCurNode] , beforeCiOne[i], 2) == 0 )
			{
#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
				Print_To_Rhythm("log/��־_����.log", "a", 1,beforeCiOne[i]);
#endif
				return TRUE;
			}
			i++;
		}
	}

	nNextNode = nCurNode + g_pTextInfo[nCurNode/2].Len;
	if( g_pTextInfo[nNextNode/2].Len > 0 )
	{
		nNextNextNode = nNextNode + g_pTextInfo[nNextNode/2].Len;
		if( g_pTextInfo[nNextNextNode/2].Len > 0 )
		{
			//�ж�PPH�ĵ��֡��������ʡ�
			if( g_pTextInfo[nNextNextNode/2].Len == 2)
			{
				i = 0;
				while( i < ( sizeof(afterCiOne)/ sizeof(emCharA *) ))
				{
					if( emMemCmp( &g_pText[nNextNextNode] , afterCiOne[i], 2) == 0 )
					{
#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
						Print_To_Rhythm("������ɴ�.txt", "a", 1,afterCiOne[i]);
#endif
						return TRUE;
					}
					i++;
				}
			}

			//�ж�PPH��˫�֡��������ʡ�
			if( g_pTextInfo[nNextNextNode/2].Len == 4 )
			{
				i = 0;
				while( i < ( sizeof(afterCiTwo)/ sizeof(emCharA *) ))
				{
					if( emMemCmp( &g_pText[nNextNextNode] , afterCiTwo[i], 4) == 0 )
					{
#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
						Print_To_Rhythm("log/��־_����.log", "a", 1,afterCiTwo[i]);
#endif
						return TRUE;
					}
					i++;
				}
			}
		
		}
	}

	return FALSE;

}



//****************************************************************************************************
//  ���ܣ�Ԥ�����ɴ�  add by songkai
//  ��������ֵ 
//****************************************************************************************************
void emCall EvalPW(float fFaZhi1)
{
	emUInt8  nPrevBounderType = 0;			//�˱߽����1���߽�����		    ��ӦlastBoundaryType
	emUInt8  nPrevPos = 0;					//�˱߽��ǰ�ʴ���				��ӦPos-1
	emUInt8  nPrevPrevPos = 0;				//�˱߽��ǰǰ�ʴ���			��ӦPos-2
	emUInt8  nPrevWLen;						//�˱߽��ǰ�ʳ�				��ӦWLen-1
	float ProbLWSum;	           			//ת�Ƹ��ʣ����ɴʵ�LW��   
	float ProbPWSum;	           			//ת�Ƹ��ʣ����ɴʵ�PW��   
	float ProbPPHSum;						//ת�Ƹ��ʣ����ɴʵ�PPH��  
	emUInt8  nCurPWCount = 0;	
	

	float fSum = 0.0;
	unsigned emCharA BounderQues[PW_PRO_NUM][PW_PRO_VALUE];				//7�ࣺ����ֵ����+����ֵ
	emUInt16 nLineNum = 0;//for test					//��PW���еĵڼ���
	emUInt8 nTmp = 0;
	emUInt8 nTmp2 = 0, nTmp3 = 0;  //pTmp2,pTmp3�ֱ�ָ��ǰ�߽�ĺ�ڵ�,���ڵ�
	emUInt8 nNew = 0;
	emPByte pPW_Table = NULL;

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	pPW_Table = (emPByte)emHeap_AllocZero(PW_LINE_NUM * PW_LINE_BYTE, "����PW��������ģ��  ��");			//�����ڴ�ռ��Ŷ����PW������
#else
	pPW_Table = (emPByte)emHeap_AllocZero(PW_LINE_NUM * PW_LINE_BYTE);			//�����ڴ�ռ��Ŷ����PW������
#endif

	fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPW,  0);					//��λ��PW��ͷ	
	fFrontRead(pPW_Table, PW_LINE_NUM * PW_LINE_BYTE, 1, g_hTTS->fResFrontMain);	//һ���Զ���������

	while( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
	{
		float PWValue = 0.0, LWValue = 0.0, PPHValue = 0.0;
		float D_PWValue = 0.0, D_LWValue = 0.0, D_PPHValue = 0.0;
		float fValue[3] = {0.0, 0.0, 0.0};
		emUInt i = 0;
		ProbLWSum = 0.0, ProbPWSum = 0.0,  ProbPPHSum = 0.0;

		nCurPWCount += g_pTextInfo[nTmp/2].Len/2;
	
		BounderQues[0][0] = LASTBOUNDARYTYPE;    //�˱߽����һ�߽�����ֵ
		if(nTmp == 0)
		{
			BounderQues[0][1] = 2;
		}
		else
		{
			BounderQues[0][1] = nPrevBounderType;
		}

		BounderQues[1][0] = POS_SUB_1;   //�˱߽�ʵ�ǰ�ʴ���
		BounderQues[1][1] = g_pTextInfo[nTmp/2].Pos;
		nPrevPos = g_pTextInfo[nTmp/2].Pos;       

		BounderQues[2][0] = POS_SUB_2;   //�˱߽�ʵ�ǰǰ�ʴ���
		if(nTmp == 0)
		{
			BounderQues[2][1] = START;
		}
		else
		{
			BounderQues[2][1] = nPrevPrevPos;   //ǰǰ�ʴ���
		}
		nPrevPrevPos = g_pTextInfo[nTmp/2].Pos; //�������  
	
		BounderQues[3][0] = WLen_SUB_1;   //�˱߽�ʵ�ǰ�ʴʳ�
		BounderQues[3][1] = g_pTextInfo[nTmp/2].Len/ 2;
		nPrevWLen = BounderQues[3][1];

	
		BounderQues[4][0] = POS_ADD_1;   //�˱߽�ʵĺ�ʴ���
		nTmp += g_pTextInfo[nTmp/2].Len ; //ָ����
		if(g_pTextInfo[nTmp2/2].Len != 0)
		{
			BounderQues[4][1] = g_pTextInfo[nTmp2/2].Pos;
		}
		else
		{
			BounderQues[4][1] = END;	
		}
		

		BounderQues[5][0] = POS_ADD_2;   // �˱߽�ĺ��ʴ���
		if(g_pTextInfo[nTmp2/2].Len != 0 || g_pTextInfo[(nTmp2+g_pTextInfo[nTmp2/2].Len)/2].Len != 0)
		{
			BounderQues[5][1] = END;
		}
		else
		{
			nTmp3 = nTmp2+g_pTextInfo[nTmp2/2].Len;		  //ָ�����
			BounderQues[5][1] = g_pTextInfo[nTmp3/2].Pos;
		}
		

		BounderQues[6][0] = WLen_ADD_1;   //�˱߽�ĺ�ʴʳ�
		if(g_pTextInfo[nTmp2/2].Len != 0)
		{
			BounderQues[6][1] = 0;
		}
		else
		{
			BounderQues[6][1] = g_pTextInfo[nTmp2/2].Len/ 2;
		}
		
		nPrevBounderType = 0;



		for(i = 0; i < PW_PRO_NUM; i++)
		{
			nLineNum = ErFenSearchOne_FromRAM(BounderQues[i], 0, pPW_Table, 1, PW_LINE_NUM, PW_LINE_BYTE);  //ʹ��һ���԰�PW������ڴ����µĶ��ֲ��ҷ�

			if(nLineNum < PW_LINE_NUM)
			{
				emMemCpy(fValue, pPW_Table + nLineNum * PW_LINE_BYTE + 2, 12);            //�������ݳ���

				ProbPWSum += fValue[0];
				ProbLWSum += fValue[1];
				ProbPPHSum += fValue[2];
			}
		}

		D_PWValue  = exp(ProbPWSum);
		D_LWValue = exp(ProbLWSum);
		D_PPHValue = exp(ProbPPHSum);
		fSum = D_PWValue + D_LWValue + D_PPHValue;
		D_PWValue  /= fSum;
		D_LWValue  /= fSum;		
		D_PPHValue /= fSum;
		if(nTmp2 != NULL)
		{
			 //�Ƚ�<PW><PPH><LW>�ĸ���,���ݷ�ֵ�����ж�
			if( D_PWValue == FindMax(D_PWValue, D_LWValue, D_PPHValue) 
				|| ((D_PPHValue == FindMax(D_PWValue, D_LWValue, D_PPHValue)) && (D_PPHValue < fFaZhi1))
				|| (( nCurPWCount >= MAX_PW_LEN_LIMIT ) || (nCurPWCount + g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len/2) > MAX_PW_LEN_LIMIT ))  //ǿ�Ʒ�PW��PW�ĳ��ȳ���ʱ
			{	//����pw��ǩ
				g_pTextInfo[nTmp2/2].BorderType = BORDER_PW;
				nPrevBounderType  = 1;

				nCurPWCount = 0;
			}
			else if(D_PPHValue == FindMax(D_PWValue, D_LWValue, D_PPHValue))
			{	//����PPH��ǩ
				g_pTextInfo[nTmp2/2].BorderType = BORDER_PPH;
				nPrevBounderType  = 1;
				nCurPWCount = 0;
			}
		}

	
		nTmp = nTmp2;
	}

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPW_Table, PW_LINE_NUM * PW_LINE_BYTE, "����PW��������ģ��  ��");			//�����ڴ�ռ��Ŷ����PW������
#else
	emHeap_Free(pPW_Table, PW_LINE_NUM * PW_LINE_BYTE);						//�����ڴ�ռ��Ŷ����PW������
#endif

}


//****************************************************************************************************
//  ���ܣ�Ԥ�����ɶ���  add by songkai
//  ��������ֵ 
//****************************************************************************************************

void emCall EvalPPH(float fFaZhiHigh, float fFaZhiLow )
{
	emUInt8  nPrevBounderType;			//��PW�߽����1���߽�����				��ӦlastBoundaryType
	emUInt8  nPrevPos[3];				//��PW�߽���ϴ���						��ӦPos-1
	emUInt8  nPrevPrevPo[3];			//��PW�߽�����ϴ���					��ӦPos-2
	emUInt8  nPrevWLen[3];				//��PW�߽���ϴʳ�						��ӦWLen-1
	emUInt8  ndFront;					//��PW�߽�ǰ��PPH�߽���ǰ�����ֳ�		��ӦdFront
	emBool	 bNeedAddPPH;
	unsigned emCharA BounderQues[PPH_PRO_NUM][PPH_PRO_VALUE];	//9�ࣺ����ֵ����+����ֵ
	emInt16 nStartNum, nLines;     //��ʼ�к�,������-----�����ӵı���
	emUInt16 nLineNum = 0;				//��PPH���еĵڼ���
	float fTiaoZheng;

	emUInt8	nCurPPHCount = 0;
	emUInt8  nNextPWCount = 0;
	emUInt8  nPosSum = 0, i = 1, nLenSum = 0, j = 0, count = 0;
	emPByte  pPPH_Table_Part1 = NULL, pPPH_Table_Part2 = NULL; //��������ָ��ָ��PPH�����������
	emPByte  pPPH_Type;							//���ǰ6�����б߽�����ֵ
	float *ProbNonPPHSum = NULL;	            //���ǰ6�����б߽�����ֵת�Ƹ��ʣ����ɶ����Non-PPH��   
	float *ProbPPHSum = NULL;	                ////���ǰ6���б߽�����ֵת�Ƹ��ʣ����ɶ����PPH��  
	float fFaZhi;
	emUInt8 nTmp = 0;
	emUInt8 nTmp2 = 0;      //pTmp2ָ����һ�����ɱ߽�
	emUInt8 nTmp3 = 0, nTmp4 = 0, nTmp5 = 0, nTmp6 = 0, nTmp7 = 0,nTmp8 = 0,nTmpNext = 0,nTmpNextNext = 0;

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	pPPH_Table_Part1 = (emPByte)emHeap_AllocZero(PPH_TABLE_LEN_PART_1, "PPH��һ���֣�������ģ��  ��");		//ΪPPH�ĵ�һ���ֿ����ڴ�ռ�
#else
	pPPH_Table_Part1 = (emPByte)emHeap_AllocZero(PPH_TABLE_LEN_PART_1);			//ΪPPH�ĵ�һ���ֿ����ڴ�ռ�
#endif


	fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH,  0);            //��λ���ļ��еĵ�һ����
	fFrontRead(pPPH_Table_Part1, PPH_TABLE_LEN_PART_1, 1, g_hTTS->fResFrontMain);                  //��ȡ����

#if DEBUG_LOG_SWITCH_HEAP
	ProbNonPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM * 4, "����ProbNonPPHSum��������ģ��  ��");
#else
	ProbNonPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM * 4);
#endif
	//emMemSet(ProbNonPPHSum, 0, MAX_NUM * 4);

#if DEBUG_LOG_SWITCH_HEAP
	ProbPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM  * 4, "����ProbPPHSum��������ģ��  ��");
#else
	ProbPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM  * 4);
#endif
	//emMemSet(ProbPPHSum, 0, MAX_NUM * 4);

#if DEBUG_LOG_SWITCH_HEAP
	pPPH_Type = (emPByte)emHeap_AllocZero(MAX_NUM * 9 * 4, "����pPPH_Type��������ģ��  ��");
#else
	pPPH_Type = (emPByte)emHeap_AllocZero(MAX_NUM * 9 * 4);
#endif
	//emMemSet(pPPH_Type, 0, MAX_NUM * 9 * 4);


	

	while( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
	{
		
		if(g_pTextInfo[nTmp/2].BorderType == BORDER_PW)  //�����ǰ�ڵ���PW�ڵ�
		{
			emMemSet(BounderQues, 0, PPH_PRO_NUM * PPH_PRO_VALUE);  //��ʼ��Ϊ0
			emMemSet(nPrevPos, 0, 3);  //��ʼ��Ϊ0
			emMemSet(nPrevPrevPo, 0, 3);  //��ʼ��Ϊ0
			emMemSet(nPrevWLen, 0, 3);  //��ʼ��Ϊ0

			//��PW�߽�����ɴʵ��С���1����2�������� ��n  ���Ĵ���֮��  POS+1
			i = 1;
			j = 0;
			if(g_pTextInfo[nTmp/2].Len != 0)
			{
				nTmp3 = nTmp ;  //ָ���1
				while( g_pTextInfo[nTmp3/2].Len != 0 && g_pTextInfo[nTmp/3].BorderType<= BORDER_LW)   //����һ�����ɴʽڵ�����
				{
					//ֻ��Ҫ���������
					if(i < 4)
					{
						BounderQues[6][i++] = g_pTextInfo[nTmp3/2].Pos;
						//��¼ȡ���������.ͳ�Ƹ���
						j++;
					}
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			BounderQues[6][0] = POS_ADD_1 * 16 + j;
			emMemCpy(pPPH_Type + count * 24, BounderQues[6], 4);

			nTmpNext = nTmp + g_pTextInfo[nTmp/2].Len;		//�൱��pTmp ->Next
			nTmpNextNext = nTmpNext + g_pTextInfo[nTmpNext/2].Len;		//�൱��pTmp ->Next->Next

			//��PW�߽�����ɴʵ��С���2����3�������� ��n  ���Ĵ���֮�ͣ�n=1ʱֵΪ�գ�  POS+2
			i = 1;
			j = 0;
			if( g_pTextInfo[nTmpNext/2].Len!=0 || g_pTextInfo[nTmpNextNext/2].Len!=0)  //����POS+2û�ж�Ӧ�ڵ�
			{
				BounderQues[7][1] = POS_CODE_END;
				j = 1;
			}

			
			if( g_pTextInfo[nTmpNext/2].Len!=0 || g_pTextInfo[nTmpNextNext/2].Len!=0)  
			{
				nTmp3 = nTmpNextNext; //ָ���2
				//������ڴ˽ڵ㲢����PW��PPH�Ļ�
				if(g_pTextInfo[nTmp3/2].Len!=0 &&  g_pTextInfo[nTmp3/2].BorderType>=BORDER_PW )  //����˵POS+2�ĵ�һ����ǡ����PW
				{
					j = 1;  //���ʱ����Ϊ����Ϊ1,ֵΪ0
				}
				//�������PW��PPH�Ļ�,��Ҫ����֪����Ϊֹ
				while(g_pTextInfo[nTmp3/2].Len!=0 && g_pTextInfo[nTmp3/2].BorderType<=BORDER_LW)  //����һ�����ɴʽڵ�����
				{
					//�������
					if(i < 3)  //������2,��3
					{
						BounderQues[7][i++] = g_pTextInfo[nTmp3/2].Pos;
						j++;
					}
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			BounderQues[7][0] = POS_ADD_2 * 16 + j;
			emMemCpy(pPPH_Type + count * 24 + 4, BounderQues[7], 4);

			//��PW�߽�ǰ���ɴʵ��С���1����2�������� ��n  ���Ĵ���֮��   POS-1
			i = 0;
			j = 0;
			nTmp3 = 0; //ָ��ͷ�ڵ�
			//���û��ǰ�߽�Ļ�
			if(g_pTextInfo[nTmp2/2].Len != 0)  //Ҳ����pTmp�ǵ�һ��PW�ڵ�
			{
				//�õ���������ڵ��е���ʼ�ڵ�ָ��
				nTmp3 = MostNearThreeNode(0, nTmp);
				while(nTmp3 != nTmp)
				{
					nPrevPos[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			else
			{
				nTmp3 = nTmp2;
				nTmp3 = MostNearThreeNode(nTmp2+g_pTextInfo[nTmp2/2].Len, nTmp);  //�õ��������
				if( g_pTextInfo[nTmp3/2].BorderType == BORDER_PPH )  //ȥ��ָ��PPH modify 2010-10-14
				{
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
				while(nTmp3 != nTmp)
				{
					nPrevPos[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			BounderQues[3][0] = POS_SUB_1 * 16 + j;
			BounderQues[3][1] = nPrevPos[0];  
			BounderQues[3][2] = nPrevPos[1];  
			BounderQues[3][3] = nPrevPos[2]; 
			emMemCpy(pPPH_Type + count * 24 + 8, BounderQues[3], 4);

			//��PW�߽�ǰ���ɴʵ��С���1����2�������� ��n-1���Ĵ���֮��  ���������     POS-2  
			i = 0;
			j = 0;
			nTmp3 = 0; //ָ��ͷ�ڵ�
			//û��ǰ�߽�ڵ�Ļ�
			if(g_pTextInfo[nTmp2/2].Len == 0)  //Ҳ����pTmp�ǵ�һ��PW�ڵ�
			{
				nTmp3 = MostNearThreeNode(0, nTmp4);
				if(nTmp3 == nTmp4)    //����POS-2û�ж�Ӧ�ڵ�����
				{
					nPrevPrevPo[0] = POS_CODE_START;
					j = 1;
				}
				//pTmp4��ǰһ���ڵ�
				while(nTmp3 != nTmp4)
				{
					nPrevPrevPo[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			else
			{
				nTmp3 = nTmp2;
				if((nTmp2 + g_pTextInfo[nTmp2/2].Len) == nTmp4)  //�����м�ֵ���һ���ʵ����  //modify 2010-10-14
				{
					j = 1;
				}
				nTmp3 = MostNearThreeNode(nTmp2+g_pTextInfo[nTmp2/2].Len, nTmp4);
				if(g_pTextInfo[nTmp3/2].BorderType == BORDER_PPH )  //ȥ��ָ��PPH modify 2010-10-14
				{
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j = 1;  //û�еĻ� ��һ��
				}
				//pTmp4��ǰһ���ڵ�
				while(nTmp3 != nTmp4)
				{
					nPrevPrevPo[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			BounderQues[4][0] = POS_SUB_2 * 16 + j;
			BounderQues[4][1] = nPrevPrevPo[0];  
			BounderQues[4][2] = nPrevPrevPo[1];  
			BounderQues[4][3] = nPrevPrevPo[2];
			emMemCpy(pPPH_Type + count * 24 + 12, BounderQues[4], 4);


			//��PW�߽�����ɴʵ��С���1����2�������� ��n  ���Ĵʳ�֮��  WLEN+1
			i = 1;
			if(g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0)
			{
				nTmp3 += g_pTextInfo[nTmp3/2].Len;  //ָ���1
				while(g_pTextInfo[nTmp3/2].Len!=0 && g_pTextInfo[nTmp3/2].BorderType<=BORDER_LW)   //����һ�����ɴʽڵ�����
				{
					if(i < 4)
					{
						BounderQues[8][1] += g_pTextInfo[nTmp3/2].Len / 2;
						i++;
					}
				nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			BounderQues[8][0] = WLen_ADD_1 * 16 + 1;
			emMemCpy(pPPH_Type + count *24 + 16, BounderQues[8], 4);

			// ��PW�߽�ǰ���ɴʵ��С���1����2�������� ��n  ���Ĵʳ�֮��   WLEN-1
			i = 0;
			nLenSum = 0;
			nTmp3 = 0; //ָ��ͷ�ڵ�
			if(g_pTextInfo[nTmp2/2].Len!=0)  //Ҳ����pTmp�ǵ�һ��PW�ڵ�
			{
				nTmp3 = MostNearThreeNode(0, nTmp);
				while(nTmp3 != nTmp)
				{
					nPrevWLen[i++] = g_pTextInfo[nTmp3/2].Len / 2;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			else
			{
				nTmp5 = nTmp2;
				nTmp5 = MostNearThreeNode(nTmp2+g_pTextInfo[nTmp2/2].Len, nTmp);  //�õ��������
				if( g_pTextInfo[nTmp5/2].BorderType==BORDER_PPH)
				{
					nTmp5 += g_pTextInfo[nTmp5/2].Len;
				}
				while(nTmp5 != nTmp)
				{
					nPrevWLen[i++] =g_pTextInfo[nTmp5/2].Len / 2;
					nTmp5 += g_pTextInfo[nTmp5/2].Len;
				}
			}
			BounderQues[5][0] = WLen_SUB_1 * 16 + 1;
			BounderQues[5][1] = nPrevWLen[0] + nPrevWLen[1] + nPrevWLen[2];
			BounderQues[5][2] = 0;
			BounderQues[5][3] = 0;
			emMemCpy(pPPH_Type + count * 24 + 20, BounderQues[5], 4);

			nTmp2 = nTmp; //ǰһ���߽�ڵ�
			++count;  //ͳ���ж��ٸ��߽�
		}

		nTmp4 = nTmp;   //ǰһ���ڵ�
		nTmp += g_pTextInfo[nTmp/2].Len;
	}



	//����ǰ6�����б߽�����ֵ
	for(i = 0; i < 6; i++)
	{
		//�õ���ʼ����
		emMemCpy(&nStartNum, pPPH_Table_Part1 + i * 4, 2);
		//�õ�������
		emMemCpy(&nLines, pPPH_Table_Part1 + i * 4 + 2, 2);

#if DEBUG_LOG_SWITCH_HEAP
		pPPH_Table_Part2 = (emPByte)emHeap_AllocZero(nLines * PPH_LINE_BYTE, "PPHǰ6��߽�ֵ��������ģ��  ��");
#else
		pPPH_Table_Part2 = (emPByte)emHeap_AllocZero(nLines * PPH_LINE_BYTE);
#endif
		//emMemSet(pPPH_Table_Part2, 0, nLines * PPH_LINE_BYTE);

		fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + nStartNum * PPH_LINE_BYTE, 0);   //��λ
		fFrontRead(pPPH_Table_Part2, nLines * PPH_LINE_BYTE, 1, g_hTTS->fResFrontMain);            //��ȡ����
		//ĳ�����������еı߽���
		for(j = 0; j < count; j++)  //���б߽��ĳ������ֵ
		{
			nLineNum = ErFenSearchTwo_FromRAM(pPPH_Type,  j * 24 + i * 4, pPPH_Table_Part2, 1, nLines, PPH_LINE_BYTE);
			if(nLineNum < nLines)
			{
				float fValue[2] = {0.0, 0.0};
				fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + PPH_TABLE_LEN_PART_2 + AddBaseAddressForSix(i) + nLineNum * 2 * 4, 0); //��λ
				//�õ�NON-PPH��PPHֵ
				fFrontRead(fValue, 8, 1, g_hTTS->fResFrontMain);
				*(ProbNonPPHSum + j) += fValue[0];
				*(ProbPPHSum + j) += fValue[1];
				/**(ProbNonPPHSum + j + i * count * 4) += fValue[0];
				*(ProbPPHSum + j + i * count * 4) += fValue[1];*/
			}
		}

#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pPPH_Table_Part2 , nLines * PPH_LINE_BYTE, "PPHǰ6��߽�ֵ��������ģ��  ��");
#else
		emHeap_Free(pPPH_Table_Part2,  nLines * PPH_LINE_BYTE);
#endif
	}

	count = 0;
	nTmp = 0;
	nTmp2 = 0, nTmp3 = 0, nTmp4 = 0, nTmp5 = 0, nTmp6 = 0, nTmp7 = 0;
	//����ʣ�µĵڶ�����
	emMemCpy(&nStartNum, pPPH_Table_Part1 + 24 , 2);  //ʣ�µĵڶ����ֵ���ʼ��

#if DEBUG_LOG_SWITCH_HEAP
	pPPH_Table_Part2 = (emPByte)emHeap_AllocZero((PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE, "PPH��3��߽�ֵ��������ģ��  ��");
#else
	pPPH_Table_Part2 = (emPByte)emHeap_AllocZero((PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE);
#endif
	//emMemSet(pPPH_Table_Part2, 0, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE);
	
	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + nStartNum * PPH_LINE_BYTE, 0);   //��λ
	fFrontRead(pPPH_Table_Part2, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE, 1, g_hTTS->fResFrontMain);            //��ȡ����
	//�ٹ���ʣ������
	while( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
	{
		if( g_pTextInfo[nTmp/2].BorderType <= BORDER_LW)
		{
			nCurPPHCount += g_pTextInfo[nTmp/2].Len/2;
		}


		if(g_pTextInfo[nTmp/2].BorderType == BORDER_PPH)  //�����ǰ�ڵ���PW�ڵ�
		{
			nCurPPHCount = 0;
		}

		if(g_pTextInfo[nTmp/2].BorderType == BORDER_PW)  //�����ǰ�ڵ���PW�ڵ�
		{
			float PPHValueSum = 0.0, NON_PPHValueSum = 0.0, sum = 0.0 ,D_PPHValue = 0.0, D_NON_PPHValue = 0.0;;
			emMemSet(BounderQues, 0, PPH_PRO_NUM * PPH_PRO_VALUE);  //��ʼ��Ϊ0
			//DBACK:��PW�߽����PPH�߽��е������ֳ�  
			i = 1;
			j = 0;
			if( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
			{
				nTmp3 += g_pTextInfo[nTmp3/2].Len;
				while(g_pTextInfo[nTmp/2].BorderType != BORDER_PPH)
				{
					if(g_pTextInfo[nTmp/2].BorderType != BORDER_PW)
					{
							BounderQues[0][1] += g_pTextInfo[nTmp3/2].Len/ 2;
					}
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					if(g_pTextInfo[nTmp3/2].Len == 0)
						break;
				}
			}
			BounderQues[0][0] = DBACK * 16 + 1;
			// DFRONT:��PW�߽�ǰ��PPH�߽���ǰ�����ֳ� 
			i = 1;
			nLenSum = 0;
			nTmp6 = 0;
			nTmp3 = 0; //ָ��ͷ�ڵ�
			while(1)
			{
				nTmp3 += g_pTextInfo[nTmp3/2].Len;
				if(nTmp3 == nTmp)  //������PPH
				{
					break;
				}
				if(g_pTextInfo[nTmp3/2].BorderType == BORDER_PPH)     //�����м��PPH
				{
					nTmp6 = nTmp3 + g_pTextInfo[nTmp3/2].Len;
				}
			}
			if(g_pTextInfo[nTmp6/2].Len==0)  //������
			{
				nTmp6 = 0;  //ʹ��Ĭ�ϵ�
			}
			while(nTmp6 != nTmp)
			{
				if(g_pTextInfo[nTmp6/2].BorderType != BORDER_PW)
				{
						BounderQues[1][1] += g_pTextInfo[nTmp6/2].Len/ 2;
				}
				nTmp6 += g_pTextInfo[nTmp6/2].Len;
			}
			BounderQues[1][0] = DFRONT * 16 + 1;
			//lastBoundary: ��PW�߽���һ�߽�����ֵ    
			BounderQues[2][0] = LASTBOUNDARYTYPE * 16 + 1;
			if(g_pTextInfo[nTmp2/2].Len == 0)  //Ҳ����pTmp�ǵ�һ��PW�ڵ�
			{
				//���ӿ�ͷĬ�Ϻ���һ��PPH
				BounderQues[2][1] = 2;
			}
			else
			{
				BounderQues[2][1] = 1;
				nTmp7 = nTmp2;
				while(nTmp7!= nTmp)
				{
					if(g_pTextInfo[nTmp7/2].BorderType == BORDER_PPH)
					{
						BounderQues[2][1] = 2;
					}
					nTmp7 += g_pTextInfo[nTmp7/2].Len;
				}
			}
		    for(i = 0; i < 3; i++)  //��������
		    {

				nLines = PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY;
				nLineNum = ErFenSearchTwo_FromRAM(BounderQues[i], 0, pPPH_Table_Part2, 1, nLines, PPH_LINE_BYTE);  //WLEN-1
				if(nLineNum < nLines)
				{
					float fValue[2] = {0.0, 0.0};
//					fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + PPH_TABLE_LEN_PART_2 + AddBaseAddressForThree(i) + nLineNum * 2 * 4, 0); //��λ
					fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + PPH_TABLE_LEN_PART_2 + PPH_TABLE_START_DBACK * 2 * 4 + nLineNum * 2 * 4, 0); //��λ
					fFrontRead(fValue, 8, 1, g_hTTS->fResFrontMain);
					NON_PPHValueSum += fValue[0];
					PPHValueSum += fValue[1];
				}
		   }
	      NON_PPHValueSum += *(ProbNonPPHSum + count);
		  PPHValueSum  += *(ProbPPHSum + count);
		  D_PPHValue = exp(PPHValueSum);
		  D_NON_PPHValue = exp(NON_PPHValueSum);
		  sum = D_PPHValue + D_NON_PPHValue;
		  D_PPHValue /= sum;
		  D_NON_PPHValue /= sum;
		  if(g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0)
		  {
			//������һ��PW�ĳ��ȣ�
			nTmp8 = nTmp + g_pTextInfo[nTmp/2].Len;;
			nNextPWCount = 0;
			while( g_pTextInfo[nTmp8/2].TextType == TextCsItem_TextType_HANZI)
			{
				nNextPWCount += g_pTextInfo[nTmp8/2].Len/2 ;
				nTmp8 += g_pTextInfo[nTmp8/2].Len;
				if( g_pTextInfo[nTmp8/2].Len == 0)
				{
					break;
				}
			}			

			//6�����ϵ��õͷ�ֵ��6�ֻ����µ��ø߷�ֵ
			if( nCurPPHCount <= PPH_BORDER_LEN_FOR_FAZHI )
			{
				fFaZhi = fFaZhiHigh;
			}
			else
			{
				fFaZhi = fFaZhiLow;
			}


			//�ж��Ƿ���Ҫ���PPH�ڵ�
			bNeedAddPPH = FALSE;
			if( (nCurPPHCount + nNextPWCount) > MAX_PPH_LEN_LIMIT)					//PPH�ĳ����趨��PPH��󳤶�
			{
				bNeedAddPPH = TRUE;
			}
			else
			{
				if(  D_PPHValue == FindMax(D_PPHValue, D_NON_PPHValue, -10)  )		//<PPH>�ĸ������
				{
					if( D_PPHValue < fFaZhi )										//<PPH>�ĸ��� < ��ֵ
					{
						if( IsHaveTeZhengCi( nTmp4 )	)							//��������,������PW��ǰ�ڵ�
						{
							bNeedAddPPH = TRUE;
						}
						else
						{
							bNeedAddPPH = FALSE;
						}
					}
					else															//<PPH>�ĸ��� >= ��ֵ
					{
						bNeedAddPPH = TRUE;
					}
				}
				else																//<Non-PPH>�ĸ������
				{
					bNeedAddPPH = FALSE;
				}				
			}			

			//���PPH�ڵ�
			if( bNeedAddPPH == TRUE)			//���ϡ�PPH��
			{
				g_pTextInfo[nTmp/2].BorderType = BORDER_PPH;
				nCurPPHCount = 0;
			}
		  }
		  nTmp2 = nTmp; //ǰһ���߽�ڵ�
		  ++count;
		}
		nTmp4 = nTmp;   //ǰһ���ڵ�
		nTmp += g_pTextInfo[nTmp/2].Len;
	}//end while

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPPH_Table_Part2, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE, "PPH��3��߽�ֵ��������ģ��  ��");
#else
	emHeap_Free(pPPH_Table_Part2, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPPH_Type, MAX_NUM * 9 * 4, "����pPPH_Type��������ģ��  ��");
#else
	emHeap_Free(pPPH_Type, MAX_NUM * 9 * 4);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(ProbPPHSum, MAX_NUM  * 4, "����ProbPPHSum��������ģ��  ��");
#else
	emHeap_Free(ProbPPHSum, MAX_NUM  * 4);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(ProbNonPPHSum, MAX_NUM * 4, "����ProbNonPPHSum��������ģ��  ��");
#else
	emHeap_Free(ProbNonPPHSum, MAX_NUM * 4);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPPH_Table_Part1, PPH_TABLE_LEN_PART_1, "PPH��һ���֣�������ģ��  ��");		//ΪPPH�ĵ�һ���ֿ����ڴ�ռ�
#else
	emHeap_Free(pPPH_Table_Part1, PPH_TABLE_LEN_PART_1);			//ΪPPH�ĵ�һ���ֿ����ڴ�ռ�
#endif


}


//****************************************************************************************************
//  ���ܣ��������������ֵ  add by songkai
//  �������������Ƚϵ�ֵ 
//****************************************************************************************************
float emCall FindMax(float PWValue, float LWValue, float PPHValue)
{
	float max = 0.0;

	LOG_StackAddr(__FUNCTION__);

	if(PWValue > max)
	{
		max = PWValue;
	}
	if(LWValue > max)
	{
		max = LWValue;
	}
	if(PPHValue > max)
	{
		max = PPHValue;
	}
	return max;
}

//****************************************************************************************************
//  ���ܣ�����������ڵ���׵�ַ  add by songkai
//  ����1: ǰһ�����ɴʽڵ��ַ
//  ����2: ��ǰ���ɴʽڵ��ַ
// ����ֵ: �����׵�ַ
//****************************************************************************************************
emUInt8 emCall MostNearThreeNode(emUInt8 nPre, emUInt8 nCur)
{
	emUInt8 i = 0;
	emUInt8 nTmp = 0;

	LOG_StackAddr(__FUNCTION__);

	if(nPre == nCur)
	{
		return nPre;
	}
	nTmp = nPre;
	while(nPre != nCur)
	{
		i++;
		//moify 
		if( g_pTextInfo[nPre/2].BorderType == BORDER_PPH )  //����PPH�������ÿ�ʼλ��ΪPPH����λ��
		{
			nTmp = nPre;
			i = 0;
		}//end modify 2010-10-13
		nPre += g_pTextInfo[nPre/2].Len;
	}
	if(i <= 3)
		return nTmp;
	else
	{
		while(i != 3)
		{
			nTmp += g_pTextInfo[nTmp/2].Len;
			i--;
		}
		return nTmp;
	}
}


/*
����: Ϊÿһ���������ӻ�����ַ
*/
emInt32  AddBaseAddressForSix(emInt32  type)
{

	LOG_StackAddr(__FUNCTION__);

	switch(type)
	{
	case 0:
		return PPH_TABLE_START_POS_ADD_1 * PPH_PRO_VALUE * 2;
		break;
	case 1:
		return PPH_TABLE_START_POS_ADD_2 * PPH_PRO_VALUE * 2;
		break;
	case 2:
		return PPH_TABLE_START_POS_SUB_1 * PPH_PRO_VALUE * 2;
		break;
	case 3:
		return PPH_TABLE_START_POS_SUB_2 * PPH_PRO_VALUE * 2;
		break;
	case 4:
		return PPH_TABLE_START_WLEN_ADD_1 * PPH_PRO_VALUE * 2;
		break;
	case 5:
		return PPH_TABLE_START_WLEN_SUB_1 * PPH_PRO_VALUE * 2;
		break;
	default:
		return -1;
		break;
	}
}

emInt32  AddBaseAddressForThree(emInt32  type)
{

	LOG_StackAddr(__FUNCTION__);

	switch(type)
	{
	case 0:
		return PPH_TABLE_START_DBACK * PPH_PRO_VALUE * 2;
		break;
	case 1:
		return PPH_TABLE_START_DFRONT * PPH_PRO_VALUE * 2;
		break;
	case 2:
		return PPH_TABLE_START_LASTBOUNDARY * PPH_PRO_VALUE * 2;
		break;
	}
}

#endif	//	EM_SYS_SWITCH_RHYTHM_NEW
