#include "emPCH.h"


#include "Rear_Pre.h"
//#include "EngF_Common.h"   //sqb

//Ԥ�����pause label��������pau
//emStatic emConst emByte	g_LabPau[LAB_CN_LINE_LEN]={0x02, 0x01, 0x06, 0x0D, 0x15, 0x02, 0x03, 0x1B, 0x03, 0x07, 0x05, 0x01, 0x02, 0x05, 0x01, 0x05,
//0x01, 0x09, 0x08, 0x04, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

emStatic emConst emByte	g_LabPau[LAB_CN_LINE_LEN] = { 0x02, 0x01, 0x07, 0x05, 0x01, 0x02, 0x0D, 0x15, 0x05, 0x01, 0x05, 0x01, 0x03, 0x1B,
0x09, 0x08, 0x04, 0x03, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };


//����Ӣ�ľ���lab��sp�����õ�pau lab


	//��Դʵ��sp��lab�������������������ǰ����+������+ǰǰ����+������ظĳ�255û��Ӱ��
/*emStatic emConst emByte	g_LabPauEng[LAB_EN_LINE_LEN] = {
53,22,22,28,3,0,1,0,0,1,1,255,0,3,1,3,2,2,2,1,0,1,8,3,3,0,0,0,0,
8,2,4,2,9,4,12,3,3,5,0,2,5,5,2,255,0,1,2,0,1,255,1,255,1,255,2,255,
1,1,255,255,255,255,255,255,255,255,2,11,23,31,255,255,255,71,2,11,
23,31,255,255,255,71,2,11,255,255,48,56,255,73,2,12,25,36,48,56,66,255};	*/

	////�����sp���������Ȳ�㣩������ǰ����+������+ǰǰ����+������ظĳ�255û��Ӱ��
	//emStatic emConst emByte	g_LabPauEng[LAB_EN_LINE_LEN] = {							
	//53,22,22,28,3,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,1,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	//255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};



//sqb 2017/1/6
//���� ��ǰ�������ڵ��ʵ����������ӵ�ǰ���ص�����ĩβ��
/*
static emInt16 LabNumOfWord(emInt16 nStartLine)
{
	emInt16 i, nPh = 0, nPhLine;			//nPh ���ʵ�������
	emInt16 nSYL, nPHIndex,nSYLIndex;
	emBool bIsWordTail;

	LOG_StackAddr(__FUNCTION__);

	
	nSYL    = g_pLabRTGlobal->m_LabRam[nStartLine][E_RLAB_SYL_NUM_IN_C_PW];		//��ǰ�����е�������Ŀ
	nPhLine = nStartLine;														//��ʼ����

	if((nSYL&0x00FF) == 0x00FF)			//������sil
		nPh = 1;			
	else
	{
		bIsWordTail = emTrue;
		//�����ʵ�����ѭ��
		for(i = g_pLabRTGlobal->m_LabRam[nStartLine][ELAB_I01_FwdPos_SylInWord]; i< nSYL; i++)					//�ӵ��ʵĵ�ǰ���� �� �������
		{
			if( (nPh  + g_pLabRTGlobal->m_LabRam[nPhLine][E_RLAB_PH_NUM_IN_C_SYL]) >= MAX_SYNTH_ENG_LAB_COUNT)   //���ϳ������� >= �ϳ�һ�ε����Ӣ��lab�У����˳������������ٴηֶΣ�
			{
				bIsWordTail = emFalse;
				break;
			}

			nPh     += g_pLabRTGlobal->m_LabRam[nPhLine][E_RLAB_PH_NUM_IN_C_SYL];
			nPhLine += g_pLabRTGlobal->m_LabRam[nPhLine][E_RLAB_PH_NUM_IN_C_SYL];	

			
		}


		if( g_pLabRTGlobal->m_LabRam[nPhLine-1][E_RLAB_R_BORDER_IN_WORD]>=BD_L3 && bIsWordTail == emTrue)		//word���ұ߽磺��PPH�߽�  ���ǵ���β
		{
			nPh++;				//��sp��			ԭ����ǰ����sp��
			nPhLine++;	
		}

	}

	return nPh;
}

// Ӣ��lab�����ʷֶ�
emInt16 LocateSegment_Eng(emInt16 nStartLine, emInt16 nMaxLine)
{
	emInt16 i, nLineCount=0;
	emInt16 nLabSeg;

	LOG_StackAddr(__FUNCTION__);

	//���÷ֶκϳɾ����õĺ��ģ�ͣ�Ӣ��ģ�ͣ���״̬����5��
	g_pRearOffset = &g_hTTS->pRearG->offset_eng;
	g_hTTS->pInterParam->nState = NSTATE_ENG;

#if EM_ENG_AS_SEN_SYNTH			//����ֶκϳ�
	for(i = 0; i< nMaxLine+1 ; i++)			//ѭ����LabRam�ж�ȡ
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount++], g_pLabRTGlobal->m_LabRam[i], LAB_EN_LINE_LEN);
#else
	
	// �ڷֶεĵ�һ��λ�ò���pause
	emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPauEng, LAB_EN_LINE_LEN); 
	nLineCount++;

	//��ȡ�ϳɵ��ʵ�lab����
	nLabSeg = LabNumOfWord(nStartLine);	


	//ѭ����LabRam�ж�ȡ
	for(i = nStartLine; i< (nMaxLine+1) ; i++)
	{
		if( nLineCount == (nLabSeg + 1 ))
		{
			// ���������Ŀ�������ƣ�����Ԥ����pause���˳�ѭ��
			emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPauEng, LAB_EN_LINE_LEN);
			nLineCount++;
			break;
			
		}
		// ����һ��label
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_pLabRTGlobal->m_LabRam[i], LAB_EN_LINE_LEN);
		nLineCount++;
	}
#endif

	g_hTTS->m_FirstSynthSegCount++;		//���κϳ��ı��ڼ����ֶμ����������磺������1�����ӵĵ�1��С�ֶ� = 1��

	return nLineCount;
}
*/

//���� ��ǰ�����������ɴʵ����������ӵ�ǰ���ڵ����ɴ�ĩβ��
//static emInt16 LabNumOfPW(emInt16 nStartLine)
//{
//	emInt16 i, nSyl, nPW,nPWIndex;			//nSyl ���ɴʵ�������
//
//	LOG_StackAddr(__FUNCTION__);
//
//	// ��ȡ��ǰ���ɴʵ�������Ŀ
//	nPW      = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_NUM_IN_C_PW]; 
//	// ��ȡ�����ڵ�ǰ���ɴ��е�λ��
//	nPWIndex = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_FWD_POS_IN_PW];
//
//	if(nPW == 0)			//������sil��pau
//		nSyl = 1;			
//	else
//	{
//		// ����ǰ���ɴʵ�����������3�������3������
//		if( (nPW - nPWIndex)  > MAX_SYNTH_LAB_COUNT_PW)
//			nSyl = MAX_SYNTH_LAB_COUNT_PW;		
//		else
//			nSyl = (nPW - nPWIndex);
//	}
//
//	return nSyl;
//}
//
//

#if EM_PW_SEGMENT_OPEN 			//�����ɴʷֶκϳ� + �����û�С����

//���ڴ�g_pLabRTGlobal->m_LabRam��Ѱ������������С�Σ�������ʱLab�У��ȴ��ֶκϳ�
emInt16	LocateSegment_Cn(emInt16 nStartLine, emInt16 nMaxLine)
{
	emInt16 i, nLineCount=0;
	emInt16 nSyl,nPW, nPWIndex,nNextPWIndex,nNextPW;

	LOG_StackAddr(__FUNCTION__);

	//���÷ֶκϳɾ����õĺ��ģ�ͣ�����ģ�ͣ���״̬����10��
	g_pRearOffset = &g_hTTS->pRearG->offset_cn;
	g_hTTS->pInterParam->nState = NSTATE_CN;

	// �ڷֶεĵ�һ��λ�ò���pause
	emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN); 
	nLineCount++;

	//���ⷢ���˱���ÿ3��һ�ϳɣ�����3�ֻῨ  
	//������[m3]�Ա�:  �˲����ز������������󣻾���ʱ�䲻�䣻��ʱ�������ز������ٸ�ԭ�����ӽ����˲�ʱ��
	//�ۺϿ��ǣ�ÿ3��һ�ϳ�Ч���ã����Ῠ��
	if( g_hTTS->m_ControlSwitch.m_nRoleIndex !=  emTTS_USE_ROLE_XIAOLIN )
	{
		// hyl 2012-04-10
		if( nStartLine >= 1 && nStartLine != (nMaxLine-1) )
		{
			if( (nMaxLine-2) == 3 )					 //3�־䣨��ǰ��sil��5�����ڣ�����3+1�ϳɣ�����Ῠ�� hyl  2012-04-12	���磺[m51][s10]����顣����㡣[m3][d] 
			{
				nSyl = 3;
			}
			else if( (nMaxLine - nStartLine) == 4)   //���ʣ4�����ڣ���sil������2+2�ϳ�
			{
				nSyl = 2;	
			}
			else if( (nStartLine+3) <= nMaxLine )		
			{
				nSyl = 3;						//һ������£�����3�ֺϳ�
			}
			else					
			{
				nSyl = nMaxLine - nStartLine;	//���ʣ�µ����ںϳ�
			}
		}
		else
			nSyl = 1;
	}
	else
	{
		if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD )				//hyl 2012-03-27
		{
			//�ǣ�һ��һ��Ч��
			if( nStartLine >= 1 )
				nSyl =  EM_STYLE_WORD_SYNTH_LAB_COUNT;		//ÿ�κϳɵ�����������Ϊ2(1����+1Pau)
			else
				nSyl = 1;


		}
		else
		{
			// ��ȡ��ǰ���ɴʵ�������Ŀ
			nPW      = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_NUM_IN_C_PW]; 
			// ��ȡ�����ڵ�ǰ���ɴ��е�λ��
			nPWIndex = g_pLabRTGlobal->m_LabRam[nStartLine][RLAB_SYL_FWD_POS_IN_PW]; 

			if(nPW == 0)			//������sil��pau
				nSyl = 1;			
			else
			{
				// ����ǰ���ɴʵ�����������3�������3������
				if( (nPW - nPWIndex)  > MAX_SYNTH_LAB_COUNT)
					nSyl = MAX_SYNTH_LAB_COUNT;		
				else
					nSyl = (nPW - nPWIndex);
			}

			
			if( g_hTTS->m_FirstSynthSegCount == 1 && nStartLine == 1  
				&& nMaxLine != 3)   //����1�־�
			{
				//����emTTS_SynthText�����״κϳɵ�����������Сֵ����������������������
				//FIRST_MIN_SYNTH_LAB_COUNT����Ϊ3��������2����������ȻῨ��  hyl  2012-04-12
				//���磺�������������[s10]�ҵ����������������顣[d]    ����Ϊ����2���ֵ���̫�̣���2����Ϊ��������
				if( nSyl <  FIRST_MIN_SYNTH_LAB_COUNT)   
					nSyl =  FIRST_MIN_SYNTH_LAB_COUNT;
			}


			//���Ǻϳ��ı��еĵ�2��С�ֶκϳɣ���ǰ1���ֶ���2���ֽڣ���Ŀǰ�ֶ�nSylΪ3���ֽڣ����޸�nSylΪ2���ֽ�
			//hyl 2012-03-31
			/*if( g_hTTS->m_FirstSynthSegCount == 2 && nStartLine == 3 && nSyl == 3)
				nSyl = 2;
			*/
			
			
#if !EM_SYNTH_ONE_WORD_PW				//������ϳɵ������ɴ�
			if( nSyl <= 2 )				//2012-10-08
			{	
				if( (nStartLine+nSyl) <= (nMaxLine-1) )		//���Ǿ�β�����1������
				{		
					//����ǰҪ�ϳɵ�������1���������������һ��ϳ�    hyl  2012-03-27
					nNextPW		 = g_pLabRTGlobal->m_LabRam[nStartLine+nSyl][RLAB_SYL_NUM_IN_C_PW];	//��ȡ��ǰ���ɴʵ�������Ŀ	

					if( nNextPW == 0 )			//������sil��pau
						nNextPW = 1;			

					if( (nNextPW+nSyl) <= MAX_SYNTH_LAB_COUNT)
						nSyl += nNextPW;
					else
						nSyl = 2;   //2012-10-12
				}		
			}
#endif

			//
			//if( (nMaxLine - nStartLine) <= MAX_SYNTH_LAB_COUNT )			//����βֻʣ < (2����+sil)����һ��ϳ�		2012-04-05
			//	nSyl = nMaxLine - nStartLine;
			//else if ( (nMaxLine - 2) == FIRST_MIN_SYNTH_LAB_COUNT )			//3�־䣨��ǰ��sil��5�����ڣ�����3+1�ϳɣ�����Ῠ�� hyl  2012-04-12  ���磺[s10]����顣[d]	
			//	nSyl = FIRST_MIN_SYNTH_LAB_COUNT;
			//else if( (nMaxLine - nStartLine) == (MAX_SYNTH_LAB_COUNT+1) )	//����βֻʣ < (3����+sil)����2+2�ϳ�		2012-04-05
			//	nSyl = 2;			

			//����������pau��ǿ��nSyl=3  hyl 2012-10-08
			if( nSyl == 2 )
			{
				if(    g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_SM] == INITIAL_CN_pau
					|| g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_SM] == INITIAL_CN_pau)
				{
					nSyl = 3;
				}

			}			
		}
	}	


	//   ��ֹ�������ڴ治���������磺[m54][s0]XSX��[m55][s0]XSX��[m3][d]
	//��С�ֶ���Ҫ�ϳ�3�����ڣ�ֻҪǰ����������������ĸ��x,w,s��
	//(��Щ��ĸ����������������ֵ����������磺�Σ�����)����ǿ�ƺϳ�2������
	//��ֹ�������������������ģ��  �����ٿռ���������ʱ���йأ������������
	//sqb  2017-5-26
	/*
	if( nSyl == 3 )
	{
		if(		(   g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_w)
			 &&	(   g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_w))
	
		{
			nSyl = 2;
		}
	}
	*/
	//ѭ����LabRam�ж�ȡ
	for(i = nStartLine; i< (nMaxLine+1) ; i++)
	{
		// ���������Ŀ�������ƣ�����Ԥ����pause���˳�ѭ��
		if( nLineCount == (nSyl + 1 ) )						
		{
			emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN);	
			nLineCount++;
			break;
		}
		// ����һ��label
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_pLabRTGlobal->m_LabRam[i], LAB_CN_LINE_LEN);	
		nLineCount++;
	}

	g_hTTS->m_FirstSynthSegCount++;		//���κϳ��ı��ڼ����ֶμ����������磺������1�����ӵĵ�1��С�ֶ� = 1��

	return nLineCount;
}
#endif


#if !EM_PW_SEGMENT_OPEN					//���̶������ֶκϳ�
	
//���ڴ�g_pLabRTGlobal->m_LabRam��Ѱ������������С�Σ�������ʱLab�У��ȴ��ֶκϳ�	
emInt16	LocateSegment_Cn(emInt16 nStartLine, emInt16 nMaxLine) 
{
	emInt16 i, nLineCount=0;
	emInt16 nSyl;

	LOG_StackAddr(__FUNCTION__);

	emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN);
	nLineCount++;
	if(g_hTTS->m_FirstSynthSegCount == 1)
	{
		nSyl = FIRST_SYNTH_LAB_COUNT;
	}
	else
		nSyl = MAX_SYNTH_LAB_COUNT;

	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD )				//hyl 2012-03-27
	{
		//�ǣ�һ��һ��Ч��
		if( nStartLine >= 1 )
			nSyl =  EM_STYLE_WORD_SYNTH_LAB_COUNT;		

	}


	//hyl 2012-10-08   ��ֹ�������ڴ治���������磺[m54][s0]XSX��[m55][s0]XSX��[m3][d]
	//��С�ֶ���Ҫ�ϳ�3�����ڣ�ֻҪǰ����������������ĸ��x,w,s��
	//(��Щ��ĸ����������������ֵ����������磺�Σ�����)����ǿ�ƺϳ�2������
	//��ֹ�������������������ģ��  �����ٿռ���������ʱ���йأ������������
	if( nSyl == 3 )
	{
		if(		(   g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine    ][RLAB_C_YM] == FINAL_CN_letter_w)
			 &&	(   g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_x
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_s
				 || g_pLabRTGlobal->m_LabRam[nStartLine + 1][RLAB_C_YM] == FINAL_CN_letter_w))
	
		{
			nSyl = 2;
		}
	}

	if( (nMaxLine - nStartLine) == 4)   //���ʣ4�����ڣ���sil������2+2�ϳ�  2012-10-08
		nSyl = 2;	


	for(i = nStartLine+1; i<= nMaxLine ; i++)
	{
		if( nLineCount == (nSyl + 1 ))
		{
			emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_LabPau, LAB_CN_LINE_LEN);
			nLineCount++;
			break;
			
		}
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount], g_pLabRTGlobal->m_LabRam[i-1], LAB_CN_LINE_LEN);
		nLineCount++;
	}


	if(nLineCount <= (nSyl + 1))
		emMemCpy(g_pLabRTGlobal->m_LabOneSeg[nLineCount++], g_LabPau, LAB_CN_LINE_LEN);

	g_hTTS->m_FirstSynthSegCount++;		//���κϳ��ı��ڼ����ֶμ����������磺������1�����ӵĵ�1��С�ֶ� = 1��
	return nLineCount;
}
#endif




//��ȡÿ��С�ֶε���֡�������ֶκϳɣ�
emInt16 GetSegmentLength(emInt16 nStartLineOfPau ,emInt16 nLineCountOfPau)
{
	emInt16 i, j, t, length=0;

	LOG_StackAddr(__FUNCTION__);

	if(g_bRearSynChn)		//���ĺϳ�
	{
		for(i = 0; i< nLineCountOfPau ; i++)
		{
			
			//��Ϊ������β��sil��ԭ���Ľ����߲�����ĳ����ǰ5֡��25ms����Ϊ���ƾ�β���ڵ�������ͻȻ��ֹ��û�й����㣩
			
			// �˴�Ϊ��β�����sil֡�����ɵ��ڣ���ע�ⲻ��̫��LSPģ�ͻ���־�β����������MGCģ�Ͳ��ᣩ
					
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == SIL_INITIAL_NO)
			{
				if(nStartLineOfPau + nLineCountOfPau - 2 == g_pLabRTGlobal->m_MaxLabLine )	//�Ǿ�βsil
				{
					//modified by hyl  2012-04-06
					for(t= 0 ;t< (LAST_SIL_MUTE/5) ;t++)
					{
						if( t > 5)		//hyl 2012-04-10  ��2�ĳ�5
							g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1�����������
						else
							g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2�����������������С
					}				
				}
				else													//�Ǿ���sil
				{
					//����sil�������  t = ?  ( �ӵڼ�֡�����0)  hyl  2012-04-05	
					for(t=0;t< ((FRONT_SIL_MUTE/5)) ;t++)				  //������1֡Ҫ���������m��n��w��ͷ�ľ���������gong4����
						g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1�������	

				}
			}


			for(j=0; j< g_hTTS->pInterParam->nState ; j++)
			{
				//���ԣ�Sil��ʱ���̶�����ͬ�ı�����Ӧ�����೤ʱ�䣬��ǰ��ģ��ȥ����  20110527
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == SIL_INITIAL_NO)	
				{
					if( nStartLineOfPau == 0 )
						g_pRearRTGlobal->m_PhDuration[i][j] = FRONT_SIL_MUTE/50;		//һ�仰�ľ���sil�ں�˵�ʱ��
					else
						g_pRearRTGlobal->m_PhDuration[i][j] = LAST_SIL_MUTE/50;			//һ�仰�ľ�βsil�ں�˵�ʱ��
				}

				//���ԣ�����Pau��ʱ��  20110526
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == PAU_INITIAL_NO)	
				{
					g_pRearRTGlobal->m_PhDuration[i][j] = (emInt32 )( ( g_pRearRTGlobal->m_PhDuration[i][j] + 2 ) / 3 );  //�豣֤ÿ��״̬������1֡
					
					if(  i!=0 && i!=(nLineCountOfPau-1))  //�Ҳ���ǰ��ӵ�Сpau
					{
						//pau�������  j >= ?  ( �ӵڼ�״̬�����0)	
						//modified by hyl  2012-04-06
						if( j >= 2 )			//hyl 2012-04-10  ��2�ĳ�2
						{
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1�����������
						}
						else
						{
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2�����������������С
						}
					}
				}			

				length += g_pRearRTGlobal->m_PhDuration[i][j];

			}
		}
	}
	else		//Ӣ�ĺϳ�
	{
		//������Ӣ�ĵģ��������ĵĲ���

		for(i = 0; i< nLineCountOfPau ; i++)
		{			
			if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silt)  //�Ǿ�βsil
			{
				for(t= 0 ;t< (LAST_SIL_MUTE/5) ;t++)
				{
					if( t > 5)		
						g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1�����������
					else
						g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2�����������������С
				}				
			}

			if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silh)  //�Ǿ���sil
			{
				for(t=0;t< ((FRONT_SIL_MUTE/5)) ;t++)				 
					g_pRearRTGlobal->m_VADflag[length+t] = 1;			//=1�������	
			}

			for(j=0; j< g_hTTS->pInterParam->nState ; j++)
			{
				if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silh )	
					g_pRearRTGlobal->m_PhDuration[i][j] = FRONT_SIL_MUTE/25;		//һ�仰�ľ���sil�ں�˵�ʱ��

				if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_silt)	
					g_pRearRTGlobal->m_PhDuration[i][j] = LAST_SIL_MUTE/25;			//һ�仰�ľ�βsil�ں�˵�ʱ��

#if HTRS_REAR_DEAL_SP
				if(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_sp)	
				{					
					if(  i!=0 && i!=(nLineCountOfPau-1))  //����ǰ��ӵ�Сpau
					{	
						g_pRearRTGlobal->m_PhDuration[i][j] *= 2;		//sp��ʱ������һ��
						if( j >= 2 )			
						{							
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 1;		//=1�����������
						}
						else
						{
							for(t = 0; t< g_pRearRTGlobal->m_PhDuration[i][j]; t++)
								g_pRearRTGlobal->m_VADflag[length+t] = 2;		//=2�����������������С
						}
					}
					else										 
						g_pRearRTGlobal->m_PhDuration[i][j] = 1;  //��ǰ��ӵ�Сpau,�豣֤ÿ��״̬������1֡

				}	
#endif

				length += g_pRearRTGlobal->m_PhDuration[i][j];

			}
		}
	}
	return length;
}

//��ȡÿ��С�ֶε�����֡�������ֶκϳɣ� 
emInt16 GetSegmentMsdLength( emInt16 PauLength)
{
	emInt16 t, length=0;

	LOG_StackAddr(__FUNCTION__);

	//�������ϳɶ�ѭ��
	for(t = 0; t< PauLength ; t++)
	{
		//��������֡�ۼ�
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED) 
		{
			length ++;

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_GONG4_GONG4

			//����if��Ϊ����gong4�����޸�MSD���ԣ���������֡����ǰһ֡
			if ( t>2 && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED && g_pRearRTCalc->m_MSDflag[t-2] == UNVOICED ) 
			{
				g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
				length++;				
			}
#endif
			}	// end of g_bRearSynChn
		}
	}
	return length;
}

