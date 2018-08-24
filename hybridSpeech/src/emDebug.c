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
//  ���ܣ���ӡ�������ݵ�һ���ļ���  add by songkai
//  �������ļ���
//****************************************************************************************************
void emCall Print_To_Rhythm(emCharA *pTitle,				//�ļ���
							emCharA *mode,					//д�ļ��ķ�ʽ
							emInt32  isHaveBorder,			//�����������Ƿ��ѱ�ע�ˡ�PW����PPH���߽�
							emCharA *pHead)				//��ǰ�ӵľ�ͷ
{

	emCharA cText[4000];
	emCharA tempBuffer[50];  //��ʱ����
	emPInt8 p1 = NULL, p2 = NULL, retString = NULL;
	emInt32  nLen = 0;
	FILE *fp_pos = NULL;
	FILE *fp = fopen(pTitle, mode);
	

	fp_pos = fopen("..\\irf\\POSֵ��ѯ��.txt", "r");


	g_nP1TextByteIndex = 0 ;

	while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0)
	{
		if( isHaveBorder == TRUE )
		{
			if( (!emMemCmp( pHead, "�ִʣ�",6) || g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PW)  &&  g_nP1TextByteIndex!=0)
			{
				emMemCpy(cText+nLen, "|", 1);
				nLen += 1;
			}
			if( g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PPH)
			{
				if( g_nP1TextByteIndex!=0)
				{
					emMemCpy(cText+nLen, "��",2);
					nLen += 2;
				}
			}
		}

		emMemCpy(cText+nLen, &g_pText[g_nP1TextByteIndex], g_pTextInfo[g_nP1TextByteIndex/2].Len);
		nLen += (g_pTextInfo[g_nP1TextByteIndex/2].Len);

		fFrontSeek(fp_pos,0,0);
		while((retString=fgets(tempBuffer, 50, fp_pos)) != NULL)
		{
			p1 = strtok(tempBuffer, "\t");  //POS��ֵ
			p2 = strtok(NULL, "\n");        //POS�ı���
			if(p2 != NULL && g_pTextInfo[g_nP1TextByteIndex/2].Pos == atoi(p2))
			{
				
#if DEBUG_LOG_SWITCH_POS_IN_RHYTHM
				emMemCpy(cText+nLen, p1,emStrLenA(p1));
				nLen += emStrLenA(p1);
#endif
				break; //�ҵ�������
			}
		}
		if(retString == NULL)  //�ļ����ҽ�����Ȼû���ҵ���Ӧֵ
		{
			emMemCpy(cText+nLen, " ",1);
			nLen += 1;
		}
		if( isHaveBorder == 0)
		{
			if( emMemCmp( pHead, "ԭ�䣺",6))
			{				
				;
			}
		}


	
		g_nP1TextByteIndex += g_pTextInfo[g_nP1TextByteIndex/2].Len; 
	}


	if( !emMemCmp( pHead, "�ִʣ�",6))
	{
		emMemCpy(cText+nLen, "|",1);
		nLen += 1;
	}
	emMemCpy(cText+nLen, "\0", 1);
	if( emMemCmp( pHead, "ԭ�䣺",6))
	{	
		if( g_hTTS->bTextTailWrap == emTrue )
			fprintf(fp, "%s%s%s%s",pHead,  "��",cText,"����\n");
		else
			fprintf(fp, "%s%s%s%s",pHead,  "��",cText,"����");
	}
	else
	{
		fprintf(fp, "%s%s%s%s%s","\n",pHead,"��",cText,"��\n\n");
	}
	
	
	fclose(fp);
	fclose(fp_pos);
}
#endif



#if DEBUG_LOG_SWITCH_TIME

//****************************************************************************************************
//  ���ܣ�
//        ������ nPara�� 1��������ı�+��ʱ   2�� ������ı�	3�� ���������
//						11:��ʱ��ʼ			 12����ʱ����
//****************************************************************************************************
#if ARM_MTK_LOG_TIME

void myTimeTrace_For_Debug(emInt16 nPara,emByte *Text, emInt16 nCount)
{
	switch(nPara)
	{
		case 1:
			kal_get_time(&tick_2);									//mtkƽ̨tracert��ʽ��ӡ
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
			kal_prompt_trace(MOD_IVTTS,"��ʱ��ʼ");
			break;
		case 12:
			kal_prompt_trace(MOD_IVTTS, "��ʱ������");
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
			g_t2_time = GetTickCount();			//WinCE�棺��C������  �� ����ƽ̨��
#else
			#if ARM_LINUX_SWITCH
				gettimeofday(&tv,&tz);			//ARM Linuxƽ̨��
				g_t2_time =  (tv.tv_sec*1000)+(tv.tv_usec/1000);	
			#else				

				#if !ARM_WINCE_TIME	
					g_t2_time = timeGetTime();			//PC��   ����C������
				#else
					g_t2_time = GetTickCount();			//Wince ��
				#endif
			#endif
#endif

			g_t3_time=g_t2_time-g_t1_time;
			g_t1_time = g_t2_time;
			if( g_t3_time< 5000)
			{
				g_total_time += g_t3_time;
			}
			fprintf(g_fLogTime, "\n%25s��	%10d           %10d", Text, g_t3_time,g_total_time);		
			break;
		case 2:
			fprintf(g_fLogTime, "%s", Text);
			break;
		case 3:
			fprintf(g_fLogTime, "%d", nCount);
			break;
		case 11:
#if ARM_LINUX_SWITCH
			g_fLogTime=fopen("log/��־_��ʱ.txt","a");			//��ʱ��ʼ
#else
			g_fLogTime=fopen("log/��־_��ʱ.log","a");			//��ʱ��ʼ
#endif
			break;
		case 12:
			fclose(g_fLogTime);			//��ʱ����
			break;
		default:
			break;
	}
}
#endif
#endif


#if DEBUG_LOG_SWITCH_HANZI_LINK
//****************************************************************************************************
//  ���ܣ���ӡ�������������   add by songkai
//        ������ nPara�� 1���½��ļ�    2�� �ϳɺ��ֻ�������ʼ     3�� �ϳɹ�����ÿ��ģ������
//				 bIsOutputBorder:  0:������߽磻 1�����PW��PPH�߽磻  2�������PPH�߽�
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
		fp_output = fopen("log/��־_��������.log", "a");
	}
	else
	{
		fp_output = fopen("log/��־_��������.log", "w");
	}

	fprintf(fp_output, "%s%s%s\n\n", "********",Text, "************************");

	if( nPara == 3)
	{		
		if(g_pTextInfo[0].Len != 0)
		{
			g_nP1TextByteIndex = 0 ;
			fprintf(fp_output, "%s\t%s\t%s\t%s\t%s\t%s\t%s\n\n","�߽�" ,"����          ", "posֵ", "����", "����", "���", "ƴ��");
			while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0 && g_nP1TextByteIndex<=MAX_HANZI_COUNT_OF_LINK*2)
			{
				nPinYinCount=0, nCount=0;

				if( g_nP1TextByteIndex!=0 && bIsOutputBorder==1)
				{
					if( g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PW)
						fprintf(fp_output, "%s\n", "�У�");
				}

				if( g_nP1TextByteIndex!=0 && (bIsOutputBorder==1 || bIsOutputBorder==2))
				{
					if( g_pTextInfo[g_nP1TextByteIndex/2].BorderType == BORDER_PPH)
						fprintf(fp_output, "%s\n", "�УУ�");

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
				nPinYinCount = g_pTextInfo[g_nP1TextByteIndex/2].Len / 2;  //���ݺ��ָ���������м���ƴ��


				for(i= 0; i < posTableLog.nLineCount; i++)
				{
					if(posTableLog.pCode[i] != NULL && g_pTextInfo[g_nP1TextByteIndex/2].Pos == posTableLog.pCode[i])
					{
						fprintf(fp_output, "%s\t", posTableLog.pPos[i]);
						break; //�ҵ�������
					}
				}


				if( i == posTableLog.nLineCount)  //�ļ����ҽ�����Ȼû���ҵ���Ӧֵ
				{
					fprintf(fp_output, "%s\t", "��Pos");
				}

				fprintf(fp_output, "%d\t%d\t",(g_pTextInfo[g_nP1TextByteIndex/2].Len)/2, g_pTextInfo[g_nP1TextByteIndex/2].TextType);

				emMemSet(t_Text, 0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
				emMemCpy(t_Text, g_pTextInfo[g_nP1TextByteIndex/2].nBiaoDian,2);
				fprintf(fp_output, "%s\t", t_Text);

				if( 1 )   //��ƴ���Ļ�
				{			
					while(nPinYinCount)      //ѭ������ÿ��ƴ��
					{
						nPinYin = g_pTextPinYinCode[g_nP1TextByteIndex/2+nCount];
						if( nPinYin == 0)
						{
							fprintf(fp_output, "--\t");
						}
						else
						{

							if( (nPinYin & 0x8000) == 0x8000)  //�ж�pPinYinCode�����λ�Ƿ�Ϊ1
							{
								nPinYin = nPinYin & 0x7FFF;			//������λ��1,�õ�ƴ������	
							}

							i = ErFenSearchPinPinLog( nPinYin );
							fprintf(fp_output, "%s ", pinyinTableLog.pPinYin[i]);
						}
						
						nCount++;
						nPinYinCount--;
					}

				}
				else  //�����û�ж�ƴ���ֶθ�ֵ
				{
					fprintf(fp_output, "%s\t", "��ƴ��");
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
//  ���ܣ���ӡ�����ֲ����õ�ƴ����Ϣ
//        ������ nPara�� 1���½��ļ�    2�� �����Ϣ
//        ������ Text��  �����Ϣ�мӵ�ǰ׺
//****************************************************************************************************
void Print_Poly_Test(emInt16 nPara, emByte *Text)
{
	emCharA tempBuffer[50];  //��ʱ����
	FILE   *fp_output = NULL;
	emPInt8 p1 = NULL, p2 = NULL, retString = NULL;
	emUInt8 nPinYinCount, nCount;
	emInt16 nPinYin,i;

	if( nPara == 1)
	{
		fp_output = fopen("log/��־_�����ֲ���-����-temp.txt", "w");
	}
	else
	{
		fp_output = fopen("log/��־_�����ֲ���-����-temp.txt", "a");


		fprintf(fp_output, "%s\t%s\n", Text, g_pText);
		fprintf(fp_output, "%s\t", Text);
		
		if(g_pTextInfo[0].Len != 0)
		{
			g_nP1TextByteIndex = 0 ;
			while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0 && g_nP1TextByteIndex<=MAX_HANZI_COUNT_OF_LINK*2)
			{
				nPinYinCount=0, nCount=0;

				nPinYinCount = g_pTextInfo[g_nP1TextByteIndex/2].Len / 2;  //���ݺ��ָ���������м���ƴ��

				while(nPinYinCount)      //ѭ������ÿ��ƴ��
				{
					nPinYin = g_pTextPinYinCode[g_nP1TextByteIndex/2+nCount];
					if( (nPinYin & 0x8000) == 0x8000)  //�ж�pPinYinCode�����λ�Ƿ�Ϊ1
					{
						nPinYin = nPinYin & 0x7FFF;			//������λ��1,�õ�ƴ������	
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
//���ַ����������ƴ���ڡ�ƴ����ѯ�������ڵ�����
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
		{   //���ҵĴ��ڱ��е�
			cMin = cMiddle;
		}
		else 
		{   //���ҵ�С�ڱ��е�
			cMax = cMiddle;
		}
		cMiddle = ( cMin + cMax) / 2;
	}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );

	return pinyinTableLog.nLineCount;
}
#endif