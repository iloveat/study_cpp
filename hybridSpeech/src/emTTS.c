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


//ǿ��ƴ��������3�� �� 1. ����[=hang2]�г�   2.��hang2�г�   3. ������ ��δ��¼���еĶ����գ�

//****************************************************************************************************
//�������ܣ� �����ֻ������������ȴ���  ����ʹû�к��֣�Ҳ�ܲ���Pcm����������
//     �������¼�������ܴ����������ֻ������������ȴ���  
// 			1�� //���Ʊ�Ǵ�������ǿ��ƴ���⣩
// 		    2�� //Ӣ�ģ���ʾ������ͨӢ�ģ�ƴ�������������ܽ��ͳɺ��ֵ�Ӣ���⣬
// 		    3�� //��β������
// 		    4�� //���������еĺ��ֳ�����MAX_HANZI_COUNT_OF_LINK
//****************************************************************************************************
emTTSErrID  emCall FirstHandleHanZiBuff(emBool bIsSylTailSil)		//�Ƿ�ϳɾ�βsil
{
	emUInt32 t1,nLen;
	emInt16 i,k;
	emByte emPtr pLabRam;


	LOG_StackAddr(__FUNCTION__);

	if( g_hTTS->m_nErrorID != emTTS_ERR_EXIT)  //��ֹ�ز��Ż�ֹͣҪ�ȴ��ܳ�ʱ��
		g_hTTS->m_nErrorID =  emTTS_ERR_OK;	
	else
		return g_hTTS->m_nErrorID;

	if( g_hTTS->m_nCurIndexOfAllHanZiBuffer>0)
	{

		//���á�RearSynth���������PlayMuteDelayһ��ʹ��
		//��ֹ8Kģ�Ͳ��ž�βʱ����
		//ͨ�����ž��еľ��������㲥���豸���1��buff�����ݣ�����һ�������ľ�β��һ���������ų�ȥ)
		//���磺���Դ�10086��һ�¡����ɷ�ָ���Ķ��Ųμӻ��
		//���磺���Դ�10086�ʡ����ɷ�ָ���Ķ��Ųμӻ��
		if( g_hTTS->m_CurMuteMs >= 100 )
		{
			t1 = g_hTTS->m_CurMuteMs - 100;			
			PlayMuteDelay(100);			//�Ȳ��ž���	
			g_hTTS->m_CurMuteMs = t1;
		}



#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 2, "**************************************************************************************************************************",0);
#endif

#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  ��׼��-ģ��  ",0);
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"\n\n                   ��׼��", 0);
#endif

		if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue )			//�յ��˳�����
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			return g_hTTS->m_nErrorID;
		}

		//�ͷţ�PCM����Ļ�����
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE, "PCM������壺�����ڣ����١�");
#else
		emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE);
#endif		

        WordSeg();		//�ִ�ģ��
        //WordSeg_third();

		//������PCM����Ļ�����
#if DEBUG_LOG_SWITCH_HEAP
		g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE, "PCM������壺����ڣ�������");
#else
		g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE);
#endif
		g_hTTS->m_pPCMBufferSize = MAX_OUTPUT_PCM_SIZE;



#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  �ִ�-ģ��  ",0);
#endif


        ToPinYin();		//����ת��ģ��
        //ToPinYin_third();


/*
        for(int i = 0; i < 45; i++)
        {
            printf("%d ", g_pTextPinYinCode[i]);
        }
        printf("\n");
*/

		if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue )			//�յ��˳�����
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			return g_hTTS->m_nErrorID;
		}

#if DEBUG_LOG_POLY_TEST
		Print_Poly_Test(2, "����");
#endif


#if	EM_SYS_SWITCH_RHYTHM_NEW

		Rhythm_New();			//����--�·���


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

        //��������:�����ﾳ�����ܶ���ߵĻ����˴���
        //����[���ܶ�]������ǰ�������
        //g_pTextInfo[4].BorderType = 3;
        //g_pTextInfo[5].BorderType = 3;
        //g_pTextInfo[7].BorderType = 1;
        /////////////////////////////////////////////////////////20180104,zoucheng,test,end


#else
		Rhythm_Old();				//����--�Ϸ���
#endif

		
#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  ����Ԥ��-ģ��  ",1);
#endif


#if PPH_AFTER_DEAL					//PPH�߽绮�ֺ���

		if( g_hTTS->m_ControlSwitch.m_nManualRhythm != emTTS_USE_Manual_Rhythm_OPEN )		//û�д��ֶ����ɱ�ע[z1]
		{
			//ǿ�Ƶ���PPH��1.���ݡ��ɶԱ��ǿ��PPH��Ϣ��    ���磺�����ˡ���¥�Ρ���ܿ���
			//ǿ�Ƶ���PPH��2.���ֶ��������					���磺/����/����մմ��ϲ������/�ڲ�֪��������/��Ⱦ��������/��(��1�����Ͻ����)
			Rhythm_ForcePPH_Basic();	
		}
#endif


#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	Print_To_Rhythm("log/��־_����.log", "a", 1,"��PH��");
#endif


#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  ǿ��PPH����  ",2);
#endif



		//�ں��������У������ɴ��ж���ʺϲ���һ���ڵ㣨pos�����������壩��ȥ��PW�ڵ㣬ÿ�����ֽڵ㼴Ϊ1��������PW	
		MergeWordsOfPW();	

#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 3, "  �ϳ����ɴʺ�  ",2);
#endif



		//������һ����������3�������ʣ�
		ChangeTone();

#if DEBUG_LOG_SWITCH_HANZI_LINK
		Print_To_Txt( 3, "  �����  ",2);
#endif

#if DEBUG_LOG_POLY_TEST
		Print_Poly_Test(2, "���");
#endif

		if (g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue )			//�յ��˳�����
		{
			g_hTTS->m_nErrorID = emTTS_ERR_EXIT;
			return g_hTTS->m_nErrorID;
		}

//#if HTRS_LINK_CN_EN
//		if( g_hTTS->m_HaveEnWordInCnType > 0 )		//���ľ�����Ӣ�ĵ���
//		{
//			nLen = emStrLenA(g_pText)/2;
//
//			if( g_hTTS->m_HaveEnWordInCnType > 1)			//���ϳɣ� �м�� �� ��β�Σ�
//			{
//				//���ǡ������ڣ������˹�����
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
//				g_pTextInfo[0].BorderType = BORDER_PPH;		//���ó����ɶ���
//				g_pTextInfo[0].nBiaoDian[0] = 0;
//				g_pTextInfo[0].nBiaoDian[1] = 0;
//				g_pTextPinYinCode[0] = 10396;
//				nLen++;
//			}
//
//			if( g_hTTS->m_HaveEnWordInCnType < 3)			//���ϳɣ� ���׶� �� �м�� ��
//			{
//				//���ǡ������ڣ���β�˹�����			
//				g_pTextInfo[nLen].Pos = 99;
//				g_pTextInfo[nLen].Len = 2;
//				g_pTextInfo[nLen].TextType = 1;
//				g_pTextInfo[nLen].BorderType = BORDER_PPH;		//���ó����ɶ���
//				g_pTextInfo[nLen].nBiaoDian[0] = 0;
//				g_pTextInfo[nLen].nBiaoDian[1] = 0;
//				g_pTextPinYinCode[nLen] = 10396;
//			}
//		}
//#endif

#if DEBUG_LOG_SWITCH_HEAP
		g_pLabRTGlobal = emHeap_AllocZero(sizeof(struct tagLabRTGlobal) , "g_pLabRTGlobal��LAB�ڴ�   ��");	
		pLabRam = (emByte emPtr)emHeap_AllocZero(MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN, "�ڲ�m_LabRam��LAB�ڴ�   ��");	

#else
		g_pLabRTGlobal = emHeap_AllocZero(sizeof(struct tagLabRTGlobal) );	
		pLabRam = (emByte emPtr)emHeap_AllocZero(MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN);	
#endif
		for( k=0; k<MAX_CN_LAB_LINE_NUM; k++)
			g_pLabRTGlobal->m_LabRam[k] = pLabRam + k * LAB_CN_LINE_LEN;			//��ֵָ���ַ


		ToLab();		//����תLabģ��

#if HTRS_LINK_CN_EN
		if( g_hTTS->m_HaveEnWordInCnType > 0 )		//���ľ�����Ӣ�ĵ���
		{
			if( g_hTTS->m_HaveEnWordInCnType > 1)			//���ϳɣ� �м�� �� ��β�Σ�
			{
				//���ǡ������ڣ������˹�ȥ��
				for(i=1; i<g_pLabRTGlobal->m_MaxLabLine; i++)
					emMemCpy(g_pLabRTGlobal->m_LabRam[i],g_pLabRTGlobal->m_LabRam[i+1],LAB_CN_LINE_LEN);
				g_pLabRTGlobal->m_MaxLabLine--;
			}
			
			if( g_hTTS->m_HaveEnWordInCnType < 3)			//���ϳɣ� ���׶� �� �м�� ��
			{
				//���ǡ������ڣ���β�˹�ȥ��
				i = g_pLabRTGlobal->m_MaxLabLine-2;
				emMemCpy(g_pLabRTGlobal->m_LabRam[i],g_pLabRTGlobal->m_LabRam[i+1],LAB_CN_LINE_LEN);	//�Ѿ�βsilǰ��
				g_pLabRTGlobal->m_MaxLabLine--;

				//��βsil������ϳ�
				g_pLabRTGlobal->m_MaxLabLine--;
			}

			g_hTTS->m_HaveEnWordInCnType = 2;
		}
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"�ϲ�+����+תLab", 0);
#endif

#if EM_SYN_SEN_HEAD_SIL
		g_hTTS->m_nParaStartLine = 0;			//�ӵ�0�����ؿ�ʼ�ϳɣ����ϳɾ���sil
#else
		g_hTTS->m_nParaStartLine = 1;			//�ӵ�1�����ؿ�ʼ�ϳɣ������ϳɾ���sil
#endif	

		//��ˣ������ϳ�
        g_hTTS->m_bResample = emFalse;			//������ǰ�����óɲ�Ҫ�ز������ú���Լ�ʶ���ز���
		if( g_pLabRTGlobal->m_MaxLabLine > 2)	//��ֹ�ϳɵĶ��ǲ������ĺ���  ���磺������
		{
			if( bIsSylTailSil == emFalse)
				g_pLabRTGlobal->m_MaxLabLine--;

			g_hTTS->m_TimeRemain = 0.0;
			RearSynth(SYNTH_CN);				
		}
		g_hTTS->m_bResample = emFalse;			//�����˳��������óɲ�Ҫ�ز����������ʾ������Ӱ��


#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pLabRam, MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN, "�ڲ�m_LabRam��LAB�ڴ�   ��");	
		emHeap_Free(g_pLabRTGlobal, sizeof(struct tagLabRTGlobal) , "g_pLabRTGlobal��LAB�ڴ�   ��");		
#else
		emHeap_Free(pLabRam, MAX_CN_LAB_LINE_NUM*LAB_CN_LINE_LEN);	
		emHeap_Free(g_pLabRTGlobal, sizeof(struct tagLabRTGlobal));	
#endif

	
#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"���--���", 0);
#endif

		ClearTextItem();	//������������
	}

	g_hTTS->m_nCurIndexOfAllHanZiBuffer = 0;
	g_hTTS->m_nCurPointOfPcmInfo = g_hTTS->m_structPromptInfo;

	g_hTTS->m_nHanZiCount_In_HanZiLink = 0;  //��������

	g_nLastTextByteIndex = 0;


	return g_hTTS->m_nErrorID;

}




//****************************************************************************************************
// ���á����Ʊ�ǡ���ص�ȫ�ֲ���
//
//****************************************************************************************************
void emCall	InitTextMarkG()					
{

	g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy	= emFalse	;						//��ʾ���������
	g_hTTS->m_ControlSwitch.m_nReadDigitPolicy	= emTTS_READDIGIT_AUTO 	;			//���ִ������
	g_hTTS->m_ControlSwitch.m_bYaoPolicy		= emTTS_CHNUM1_READ_YAO;			//���ġ�һ���Ķ�����Ĭ��ʱ���� ��
	g_hTTS->m_ControlSwitch.m_nSpeakStyle		= emTTS_STYLE_NORMAL;				//������� word-by-word ���� ��Ȼ��ʽ
	g_hTTS->m_ControlSwitch.m_nVoiceSpeed		= emTTS_SPEED_NORMAL;				//�����ٶ�
	g_hTTS->m_ControlSwitch.m_nVoicePitch		= emTTS_PITCH_NORMAL;				//��Ƶ
	g_hTTS->m_ControlSwitch.m_nVolumn			= emTTS_VOLUME_NORMAL; 				//������С
	g_hTTS->m_ControlSwitch.m_bPunctuation		= emTTS_PUNCTUATION_NO_READ;		//��㴦�����
	g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = emTTS_USE_PINYIN_CLOSE;         //ʶ��ƴ�����عر�
	g_hTTS->m_ControlSwitch.m_bXingShi          = emTTS_USE_XINGSHI_CLOSE;			//�����Ͽ��عر�
	g_hTTS->m_ControlSwitch.m_nManualRhythm     = emTTS_USE_Manual_Rhythm_CLOSE;	//���ɱ�ע����
	g_hTTS->m_ControlSwitch.m_nVoiceMode = emTTS_USE_VoiceMode_CLOSE;		//��Чģʽ�ر�
	g_hTTS->m_ControlSwitch.m_nLangType = emTTS_LANG_CN;					//���֣��Զ�ѡ��
	g_hTTS->m_ControlSwitch.m_bZeroPolicy       = emTTS_EN_0_ZERO;					//Ӣ��0��Ĭ�϶��ɣ�zero



	g_hTTS->m_ControlSwitch.m_bIsPhoneNum		= emNull;							//���ֲ��������
	g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen   = 0;                               //���ֲ����ȷֶ�

	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emFalse;						//����[p*]ͣ�ٲ��������ľ�βsil
	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil = emFalse;						//����[p*]ͣ�ٲ��������ľ���sil

	g_hTTS->m_FirstSynthSegCount = 1;												//���κϳ��ı��ڼ����ֶμ����������磺������1�����ӵĵ�1��С�ֶ� = 1��


}



//****************************************************************************************************
// //��������ȫ�ֲ���
//
//****************************************************************************************************
void emCall	InitOtherG()	
{

	g_hTTS->m_ControlSwitch.m_nCodePageType		= emTTS_CODEPAGE_GBK ;					//�����ı�����ҳ����
	g_hTTS->m_ControlSwitch.m_nRoleIndex = emTTS_USE_ROLE_XIAOLIN;				//����Ĭ�Ϸ�����

	g_hTTS->m_bResample = emFalse;														//��ʼ���������ز���

	g_hTTS->m_ChaBoCount						= 0;
	g_hTTS->m_bIsErToLiang						= emFalse;

	g_hTTS->m_nPrevSenType						= SEN_TYPE_CN;							//��ʼ�����ã�ǰ����������Ϊ�����ľ�

}


//���Unicode��Сͷ�洢��ʽ�����ߵ��ֽڵ�ת��
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
//UNICODE��תΪGB2312��
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
//GB2312��תΪUNICODE��
int g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
    return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}

#endif



//****************************************************************************************************
// ʵ�ֺ����������ϳɣ� emTTS_SynthText
//
// �ڵ�ǰʵ���кϳ�һ���ı�����ǰ�̻߳ᱻ������ֱ���ϳɽ���
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
    emByte uginput[1024]={0} ;		//utf8 1024��Լ��300������
    emByte ugout[1024]={0};


#if DEBUG_LOG_SWITCH_HANZI_LINK|DEBUG_LOG_POLY_TEST

	//���� POS�����ѯ�� �� ƴ�������ѯ��
	emCharA tempBuffer[50];  //��ʱ����
	FILE *fp_pinyin = NULL, *fp_pos = NULL;
	emInt32  tt1,tt2;

	fp_pinyin = fopen("..\\irf\\ƴ�������ѯ��.txt", "r");	
	pinyinTableLog.nLineCount = 0;
	while(fgets(tempBuffer, 50, fp_pinyin) != NULL)			//sqb	ÿ�ζ�ȡ�ļ���һ�У�50��һ�е�����ַ���
	{
		tt1 = strchr(tempBuffer,0x09)-tempBuffer;				//sqb 0x09��ʾ\t  0x0a ��ʾ\n  
		tt2 = strchr(tempBuffer,0x0a)-tempBuffer;				//����\t,�ҵ�������Ϣ���ٶ�λ��tempbuffer����һ��
		emMemCpy(pinyinTableLog.pPinYin[pinyinTableLog.nLineCount],tempBuffer,tt1);	//pinyin��ֵ
		pinyinTableLog.pPinYin[pinyinTableLog.nLineCount][tt1] = 0;
		pinyinTableLog.pCode[pinyinTableLog.nLineCount] = atoi(tempBuffer+tt1+1);	//pinyin�ı���

		pinyinTableLog.nLineCount++;
	}
	fclose(fp_pinyin);

		

	fp_pos = fopen("..\\irf\\POSֵ��ѯ��.txt", "r");	
	posTableLog.nLineCount = 0;
	while(fgets(tempBuffer, 50, fp_pos) != NULL)
	{
		tt1 = strchr(tempBuffer,0x09)-tempBuffer;  
		tt2 = strchr(tempBuffer,0x0a)-tempBuffer;             
		emMemCpy(posTableLog.pPos[posTableLog.nLineCount],tempBuffer,tt1);			//pos��ֵ
		posTableLog.pPos[posTableLog.nLineCount][tt1] = 0;
		posTableLog.pCode[posTableLog.nLineCount] = atoi(tempBuffer+tt1+1);			//pos�ı���

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
	myTimeTrace_For_Debug(11,(emByte *)"��ʱ��ʼ", 0);
	myTimeTrace_For_Debug(2,(emByte *)"\n\n\n ************************         С��(����)         �ܼ�(����)", 0);
	myTimeTrace_For_Debug(1,(emByte *)"��SynthTextǰ******", 0);
#endif


	g_BaseStack = 0;

	LOG_StackAddr(__FUNCTION__);				//��ӡջ����Ϣ��������

	g_hTTS->m_bStop = emFalse;
	global_bStopTTS = emFalse;

	
#if DEBUG_LOG_SWITCH_HANZI_LINK
	Print_To_Txt( 1, "�½��ļ�",0);
#endif

#if DEBUG_LOG_POLY_TEST
	Print_Poly_Test(1, "�½��ļ�");
#endif

	if ( pcData == emNull )
		return emTTS_ERR_BASE;

	if ( nSize == 0  )
		return emTTS_ERR_OK;

    //��utf8תgbk��һ��ʼ  sqb 2017/6/13
    if(emTTS_CODEPAGE_UTF8==1){
        emUInt16 size = emStrLenA((emStrA) pcData) * sizeof(emCharA);
        emMemCpy(uginput,pcData,size);
        u2g(uginput,strlen(uginput),ugout,1024);
        emMemSet(pcData,0,size);
        emMemCpy(pcData,ugout,size);
    }

	if ( nSize == (emSize)-1 )					//sqb ����Ĭ�ϸ�-1ֵ�����˴���if
	{
		emTTS_GetParam(hTTS, emTTS_PARAM_INPUT_CODEPAGE, &nCodeType);
        if(emTTS_CODEPAGE_UTF8==1)
        {
            g_hTTS->m_nNeedSynSize = emStrLenA((emStrA) pcData) * sizeof(char);
        }
        else if( nCodeType ==emTTS_CODEPAGE_UTF16LE  || nCodeType == emTTS_CODEPAGE_UTF16BE )
		{
			nSize = nSize;		//��ʱ�ӵģ����û�����

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

	g_hTTS->m_FirstSynthSegCount = 1;					//���κϳ��ı��ڼ����ֶμ����������磺������1�����ӵĵ�1��С�ֶ� = 1��


	g_hTTS->m_CurMuteMs = 0;		//��ʼ��
	g_hTTS->m_bIsMuteNeedResample = emFalse;
	g_hTTS->m_NextMuteMs = 0;		//��ʼ��


	g_hTTS->m_bIsStartBoFang = emFalse;
	g_hTTS->m_bIsSynFirstTxtZi = emTrue;


	
	//ѭ�����б���ת������ȫ��ת��GBK���룬�����������ϳ�
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
		if(    g_hTTS->m_ChaBoCount == CHABO_INDEX		//���˲岥�ĵڼ���
			&& nIsCheckInfo != 2)						//�����ڲ��ſͻ���Ϣ
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

		g_hTTS->m_FirstSynthSegCount = 1;		//��ֹ����  ���磺[s10]Ӣ����ĸ�����ͨ3G�����ڡ��֡�һ������������Լҵķ�ǰ������UFO����һ����������PK��ʱ����ҵӦ��ǰ��600�׽ӽ�Ŀ�ĵ��ϵس���վ��ʻ2.5������ǰ����ʻ���ϵ���Ϣ·��ӭ�������°�ʿ��ʮ��·������Ʊ�����������ܶ���475.25Ԫ����500Ԫ����24.75Ԫ����������绰���Խ�������������Ϣ����Ϣ����13905511861[d]

		if( nErrorID == emTTS_ERR_END_OF_INPUT)
		{
			break;		//����ת������������ѭ��,�˳�TTS
		}

		nProgressLen = g_hTTS->m_nCurIndexOfBasicData - nProgressBegin ;


#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
		//����ת������ı����һ��word�Ƿ��з�
		for( i =0; i< 1000 ;i=i+2)
		{
			if( GetWord(g_hTTS->m_pDataAfterConvert,i) == (emInt16)0xffff) 
			{
				if(    GetWord(g_hTTS->m_pDataAfterConvert,i-2) == (emInt16)0xa38a
					|| GetWord(g_hTTS->m_pc_DataOfBasic,g_hTTS->m_nCurIndexOfBasicData-2) == (emInt16)0x0d0a 
					|| GetWord(g_hTTS->m_pc_DataOfBasic,g_hTTS->m_nCurIndexOfBasicData) == (emInt16)0x0d0a 
					|| GetWord(g_hTTS->m_pc_DataOfBasic,g_hTTS->m_nCurIndexOfBasicData+2) == (emInt16)0x0d0a )

				{
					//�ǻ��з����ڡ�������־����Ҳ����
					g_hTTS->bTextTailWrap = emTrue;
				}
				else
				{
					//�ǻ��з����ڡ�������־���в�����
					g_hTTS->bTextTailWrap = emFalse;
				}
				break;
			}

		}
#endif


#if	EM_PROCESS_CALLBACK_OPEN    //���򿪣����Ȼص�����

		if ( g_pProgressCallbackFun )
		{
			//�ϳɽ��Ȼص�
			( *  g_pProgressCallbackFun)(NULL, nProgressBegin, nProgressLen);
		}

#endif

		g_hTTS->m_ControlSwitch.bIsReadDian = emFalse;

		if( g_Res.offset_FanToJian != 0 )
		{
			//���С���ת�����Դ������з���ת��
			FanToJian();		//��g_hTTS->m_pDataAfterConvert������ݽ���
		}


		if( nIsCheckInfo == 0 )			//  2012-04-23
			nIsCheckInfo = CheckInfo();



		g_hTTS->m_nCurIndexOfConvertData = 0;
		g_hTTS->m_nCurPointOfPcmInfo = g_hTTS->m_structPromptInfo;

		if ( nErrorID == emTTS_ERR_FAILED )		//ʧ�ܣ� �쳣 �� ���ı�����������ϳ�
		{
			break;		//����ѭ��
		}

		g_hTTS->m_ControlSwitch.m_bYiBianYinPolicy = emTTS_USE_YIBIANYIN_OPEN;

		g_hTTS->m_nCurIndexOfAllHanZiBuffer = 0;
		g_hTTS->m_nHanZiCount_In_HanZiLink = 0;		



		//�������[g2]a,b,c.  [g1]a,b,c.
		if( GetWord( g_hTTS->m_pDataAfterConvert, 0 )  == (emInt16)0xa3db  )			//����Ϊ��"["
			JudgeCtrlMark(GetFuHaoSum( g_hTTS->m_pDataAfterConvert, 0), g_hTTS->m_pDataAfterConvert,0);		//ʶ����Ʊ�ǲ�����ȫ�ֱ���

		//sqb  ȥ��Ӣ�ľ䣬Ŀǰֻ��������
		//�ж��ǰ������ľ䡱���ǰ���Ӣ�ľ䡱����		
		g_hTTS->m_HaveEnWordInCnType = 0;
		if( DecideSenType() ==  SEN_TYPE_EN)			//����Ӣ�ľ�
		{
			//HandleEnSen();			//sqb
		}
		else											//�������ľ�
		{
			HandleCnSen();

			if( g_hTTS->m_nCurIndexOfAllHanZiBuffer>0)
				nErrorID = FirstHandleHanZiBuff(emTrue);	//��β�����������ֻ������������ȴ���
			else
				if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
					break;			
		}	


		if ( nErrorID == emTTS_ERR_EXIT )		//ȫ���ı�ת������, �˳�TTS	
		{
			break;		//����ѭ��,�˳�TTS
		}


	}; //end while


#if	EM_PROCESS_CALLBACK_OPEN    //���򿪣����Ȼص�����
	if ( g_pProgressCallbackFun )
	{
		//�ϳɽ��Ȼص�
		( *  g_pProgressCallbackFun)(NULL, nProgressBegin+nProgressLen, 0); 
	}
#endif


#if DEBUG_LOG_SWITCH_TIME
	myTimeTrace_For_Debug(12,(emByte *)"��ʱ����", 0);
#endif


	return emTTS_ERR_OK;
}

//****************************************************************************************************
// ʵ�ֺ���������ʵ���� emTTS_Create 
//	����ÿ�κϳ�2�ֵ�nHeapSize�Ĵ�СĿǰΪ��42K������Ч����62K������Ч��
//****************************************************************************************************
emTTSErrID emCall emTTS_Create(
	emHTTS emPtr	phTTS,				// [out] ʵ�����
	emPointer		pHeap,				// [in] ʵ����
	emSize			nHeapSize,			// [in] ʵ���Ѵ�С
	emPointer		pCBParam,			// [in] �û��ص�����  �����Ƶ�����ļ���ָ��
	emPResPackDesc	pResPackDesc,		// [in] ��Դ����������
	emSize			nResPackCount )		// [in] ��Դ������
{

	emInt32 nMinHeap;
	emInt8 bIsLessMin = emFalse;
	emInt8 nFloatType;
	emInt32	*p;
	emInt8	i;


#if EM_8K_MODEL_PLAN							//8Kģ�����ã��ڴ�ռ����
	nMinHeap = USER_HEAP_MIN_CONFIG;
#else											//16Kģ������
	nMinHeap = USER_HEAP_MAX_CONFIG;
#endif

#if EM_USER_SWITCH_VOICE_EFFECT		//����Ч
	nMinHeap += USER_HEAP_VOICE_EFFECT;
#endif

	if( nHeapSize < nMinHeap)
	{
#if DEBUG_LOG_SWITCH_ERROR
		g_fLogError = fopen("log/��־_����.log","a");	
		fprintf(g_fLogError, "%s%d%s%d\n","���󣺿��ٵĶ��ڴ治��!	",nHeapSize," < ", nMinHeap);
		fclose(g_fLogError);
#endif
		return emTTS_ERR_INSUFFICIENT_HEAP;
	}

	g_bIsUserAddHeap = 0;
#if EM_DECISION_HEAP				//�Ƿ�֧�־������ڴ�
	#if EM_8K_MODEL_PLAN			//8Kģ������		
		nMinHeap += USER_HEAP_MIN_CONFIG_ADD;
		if( nHeapSize >= nMinHeap )
			g_bIsUserAddHeap = 1;				
	#else							//16Kģ������
		nMinHeap += USER_HEAP_MAX_CONFIG_ADD;
		if( nHeapSize >= nMinHeap )
			g_bIsUserAddHeap = 1;				
	#endif
#endif
	
	//������Դ�� 
	emResPack_Initialize(&g_ResPack, pResPackDesc);	
	emResCacheLog_Begin();			//��ʼ��Դ Cache Log 

	
	///////////////////////////////////////////////
	g_pHeap = (emPHeap)emGridPtr(pHeap);
	/* ��ʼ���� */
	emHeap_InitH(g_pHeap, nHeapSize - (EM_PTR_GRID - 1));
	///////////////////////////////////////////////


#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS = (struct emHTTS_G *)emHeap_AllocZero( sizeof(struct emHTTS_G), "ȫ��g_hTTS������ڣ�������");
#else
	g_hTTS = (struct emHTTS_G *)emHeap_AllocZero( sizeof(struct emHTTS_G));
#endif



	InitTextMarkG();			//���á����Ʊ�ǡ���ص�ȫ�ֲ���
	InitOtherG();				//��������ȫ�ֲ���



#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->m_pDataAfterConvert = (emPByte)emHeap_AllocZero( CORE_CONVERT_BUF_MAX_LEN, "ת����ı�׼�ı�������ڣ�������");
#else
	g_hTTS->m_pDataAfterConvert = (emPByte)emHeap_AllocZero( CORE_CONVERT_BUF_MAX_LEN);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_pTextInfo = (struct TextItem *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem ), "��������Ϣ������ڣ�������");
#else
	g_pTextInfo = (struct TextItem *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem ));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	g_pText     = (emByte *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "���������֣�����ڣ�������");
#else
	g_pText     = (emByte *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	g_pTextPinYinCode  = (emInt16 *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "������ƴ��������ڣ�������");
#else
	g_pTextPinYinCode  = (emInt16 *)emHeap_AllocZero( MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif



	// 
#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->pRearG = (emRearG *) emHeap_AllocZero(sizeof(emRearG), "���pRearG������ڣ�������");
#else
	g_hTTS->pRearG = (emRearG *) emHeap_AllocZero(sizeof(emRearG));
#endif


/************************************* Ŀǰ��emConfig.h�еõ��Ľ������    *********************************************/

	g_hTTS->pRearG->bIsVQ = EM_INTERFACE_VQ;
	g_hTTS->pRearG->bIsStraight = EM_INTERFACE_IS_STRAIGHT;
	g_hTTS->pRearG->stage = EM_INTERFACE_IS_LSP;

/************************************* Ŀǰ�ӽ������Ĳ������Ժ�ֱ�ӳ�ʼ�� *********************************************/

	
	g_hTTS->pRearG->beta = 0.4;
	g_hTTS->pRearG->Speech_speed = 1.0;
	g_hTTS->pRearG->f0_mean = 0.0;




	


#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->pInterParam = (emInterParam *) emHeap_AllocZero(sizeof(emInterParam), "���pInterParam������ڣ�������");
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

	
#if EM_RES_READ_REAR_LIB_CONST				//LIB�������������Դ��������Դ���Ǵ�irf�ж���
	g_hTTS->fResFrontMain       = pResPackDesc->pCBParam;
	if( pResPackDesc->pCBParam == emNull )
		return emTTS_ERR_RESOURCE;

	g_hTTS->fResCurRearMain     = (emPointer)ResourceConstData;
#else
	#if EM_RES_READ_ALL_LIB_CONST			//LIB������������Դ���˿����ر�����irf��Դ�ļ���ȫ���ӳ�������
		g_hTTS->fResFrontMain   = (emPointer)ResourceConstData;
		g_hTTS->fResCurRearMain = (emPointer)ResourceConstData;
	#else									//irf��Դ�ļ���(ȫ��)
		g_hTTS->fResFrontMain   = pResPackDesc->pCBParam;
		g_hTTS->fResCurRearMain = pResPackDesc->pCBParam;
		if( pResPackDesc->pCBParam == emNull )
			return emTTS_ERR_RESOURCE;
	#endif
#endif  

	g_hTTS->m_cbReadRes = pResPackDesc->pfnRead;

		GetTotalResOffset();		//��ȡ������Դ������ƫ����

	g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;
	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_RearVoiceLib_Cur  ,0);
	fFrontRead(&g_hTTS->pRearG->nModelSample,2,1,g_hTTS->fResFrontMain);								//�ڴ������ģ�Ͳ����ʣ���ʾ��ģ����Ҫ     2012-2-6


	/************************************* ����Դ�ж���Ĳ��� *********************************************/

	fRearSeek(g_hTTS->fResCurRearMain,g_Res.offset_RearVoiceLib_Cur  ,FSEEK_START);					//����ģ�ͺ����Դ��
	fRearRead(&g_hTTS->pRearG->nModelSample,2,1,g_hTTS->fResCurRearMain);							//ģ�Ͳ�����
	fRearRead(&g_hTTS->pInterParam->static_length, sizeof(emInt16), 1, g_hTTS->fResCurRearMain);	//Ƶ�׵�ά��
	fRearRead(&g_hTTS->pRearG->offset_cn, sizeof(g_hTTS->pRearG->offset_cn),1,g_hTTS->fResCurRearMain);	//��˵�ÿ��С��Դƫ��
	
	//���С��Դƫ�ƣ����ƫ��-->����ƫ��
	for( p = &g_hTTS->pRearG->offset_cn, i=0; i < sizeof(g_hTTS->pRearG->offset_cn)/sizeof(emInt32); i++ )
	{
		*(p++) += g_Res.offset_RearVoiceLib_Cur;
	}
	
	//����ģ��floatҶ�ڵ�ռ���ֽ���������:  ֵ=2��	��ռ�ֽ�ȫ��Ϊ2�ֽ�
	//										ֵ=3��	��Ƶģ��0�׾�ֵռ4�ֽ�,Ƶ��ģ������ά0�׾�ֵռ4�ֽ�,����ռ2���ֽ�
	//										ֵ=4��	��ռ�ֽ�ȫ��Ϊ4�ֽ�
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
			

	g_hTTS->pRearG->frame_length = g_hTTS->pRearG->nModelSample/200;						//֡��
	g_hTTS->pRearG->nMgcLeafPrmN = g_hTTS->pInterParam->static_length*g_hTTS->pInterParam->width;


	if( g_bIsUserAddHeap == 1)				//�û����˶�����ڴ棨�ʺ��ڴ�ռ主ԣ�Ŀͻ����ɽ����22�ž�����ȫ�����뵽�ڴ棩
	{
		//���غ�˵ľ�����
		LoadRearAllDecision();
	}

#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->m_Info = emHeap_AllocZero( MAX_COMPANY_INFO_LEN, "��Ϣ������ڣ�������");
#else
	g_hTTS->m_Info = emHeap_AllocZero( MAX_COMPANY_INFO_LEN);
#endif

	//������PCM����Ļ�����
#if DEBUG_LOG_SWITCH_HEAP
	g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE, "PCM������壺����ڣ�������");
#else
	g_hTTS->m_pPCMBuffer = emHeap_AllocZero(MAX_OUTPUT_PCM_SIZE);
#endif
	g_hTTS->m_pPCMBufferSize = MAX_OUTPUT_PCM_SIZE;

	return emTTS_ERR_OK;
}



//****************************************************************************************************
// �ͷű�������ʵ��
//
//****************************************************************************************************
emTTSErrID emCall emTTS_Destroy(
	emHTTS			hTTS )				/* [in] handle to an instance */
{




	//�ͷţ�PCM����Ļ�����
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE, "PCM������壺�����ڣ����١�");
#else
	emHeap_Free(g_hTTS->m_pPCMBuffer, MAX_OUTPUT_PCM_SIZE);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->m_Info, MAX_COMPANY_INFO_LEN,  "��Ϣ�������ڣ����١�");
#else
	emHeap_Free(g_hTTS->m_Info, MAX_COMPANY_INFO_LEN);
#endif

	if( g_bIsUserAddHeap == 1)				//�û����˶�����ڴ棨�ʺ��ڴ�ռ主ԣ�Ŀͻ����ɽ����22�ž�����ȫ�����뵽�ڴ棩
	{
		//�ͷź�˵ľ�����
		FreeRearAllDecision();
	}



#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->pInterParam, sizeof(emInterParam), "���pInterParam�������ڣ����١�");
#else
	emHeap_Free(g_hTTS->pInterParam, sizeof(emInterParam));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS->pRearG, sizeof(emRearG), "���pRearG�������ڣ����١�");
#else
	emHeap_Free(g_hTTS->pRearG, sizeof(emRearG));
#endif



#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_pTextPinYinCode, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "������ƴ���������ڣ����١�");
#else
	emHeap_Free( g_pTextPinYinCode, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_pText, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2, "���������֣������ڣ����١�");
#else
	emHeap_Free( g_pText, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_pTextInfo, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem), "��������Ϣ�������ڣ����١�");
#else
	emHeap_Free( g_pTextInfo, MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*sizeof(struct TextItem));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( g_hTTS->m_pDataAfterConvert, CORE_CONVERT_BUF_MAX_LEN, "ת����ı�׼�ı��������ڣ����١�");
#else
	emHeap_Free( g_hTTS->m_pDataAfterConvert, CORE_CONVERT_BUF_MAX_LEN);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_hTTS,  sizeof(struct emHTTS_G), "ȫ��g_hTTS�������ڣ����١�");
#else
	emHeap_Free(g_hTTS,  sizeof(struct emHTTS_G));
#endif


	emHeap_UninitH(g_pHeap);

	//������Դ Cache Log 
	emResCacheLog_End();

	return emTTS_ERR_OK;
}


//****************************************************************************************************
// ��ȡʵ���Ĳ���
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
// ����ʵ���Ĳ���
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
// ����ʵ����TTS�ϳɣ������е�emTTS_Run������������
//
//****************************************************************************************************
emTTSErrID emCall emTTS_Exit(
	emHTTS			hTTS )				/* [in] handle to an instance */
{



	g_hTTS->m_bStop = emTrue;	
	//�˳��ϳ�
	return emTTS_ERR_OK;

}


//�������ľ�
void emCall HandleCnSen()
{
	emInt16  t1,nDataTypePrev,nDataTypeNext;
	emInt16 nCurWord;

	ClearTextItem();	//������������

		//fhy 130105 ȥ�ո�
	{
		int i,j;
		emBool	bTypeQu;
		for( i = 2; g_hTTS->m_pDataAfterConvert[i + 2] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[i + 3] != (emByte)END_WORD_OF_BUFFER; i += 2 )
		{
			bTypeQu = emFalse;
			nDataTypePrev = CheckDataType( g_hTTS->m_pDataAfterConvert, i - 2);
			nDataTypeNext = CheckDataType( g_hTTS->m_pDataAfterConvert, i + 2);

			if( nDataTypePrev != nDataTypeNext)			//ǰ���ı����Ͳ�һ��
			{
				if(    nDataTypePrev == DATATYPE_SHUZI   && nDataTypeNext == DATATYPE_YINGWEN		//ǰ�����ֺ�����ĸ
					|| nDataTypePrev == DATATYPE_YINGWEN && nDataTypeNext == DATATYPE_SHUZI )	//ǰ����ĸ��������
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

	//��Ҫ������н��д�����ĸ��дתСд
	g_hTTS->m_nCurIndexOfConvertData = 0;
	while( global_bStopTTS == emFalse )
	{
		if(    *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData    ) == 0xff
			&& *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) >= 0xff )
		{
			break;			//��������
		}

		if(    *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData    ) == 0xa3
			&& *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) >= 0xc1 
			&& *(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) <= 0xda )
		{
			*(g_hTTS->m_pDataAfterConvert + g_hTTS->m_nCurIndexOfConvertData + 1) += 0x20;
		}
		g_hTTS->m_nCurIndexOfConvertData += 2;				
	}
	

	//��ʼ�Ա���ת���������ݽ���ѭ�������ϳ�
	g_hTTS->m_nCurIndexOfConvertData = 0;
	while( global_bStopTTS == emFalse )
	{

		//��ȡg_hTTS->m_pDataAfterConvert��Ŀǰg_hTTS->m_nCurIndexOfConvertDataλ�ô���ָ�ַ�����������
		g_hTTS->m_nDataType = CheckDataType(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);

		nDataTypePrev  = CheckDataType(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData-2);
		nDataTypeNext  = CheckDataType(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData+2);

		nCurWord = GetWord(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);  //��ǰ����

		if( g_hTTS->m_nDataType == DATATYPE_END )
		{
			if( g_hTTS->m_HaveEnWordInCnType > 0 )
				g_hTTS->m_HaveEnWordInCnType = 3;
			break;			//�����������������ϳɽ���
		}

		switch( g_hTTS->m_nDataType)
		{
			case DATATYPE_HANZI :

				//add by  2012-03-30  
				if(	  (     nCurWord != (emInt16)0xbacd			//���֣���
						 && nCurWord != (emInt16)0xd3eb)		//���֣���
					|| nDataTypePrev != DATATYPE_SHUZI
					|| nDataTypeNext != DATATYPE_SHUZI)
				{
					g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen   = 0;     //���ֲ����ȷֶ�

					//����:  �ȷ�Ϊ21��18��21��14��21��14��
				}

				//����д�뵽��������� ���ֵĴ�������FirstHandleHanZiBuff() ���ɡ����Ʊ�ǡ��͡�Ӣ�ġ��͡���β������
				g_hTTS->m_nCurIndexOfConvertData = WriteToHanZiLink(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData, POS_CODE_kong ,0);
				if( g_ForcePPH.bIsStartCount == 1)
					g_ForcePPH.nCount += g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
				break;
			case DATATYPE_FUHAO :
				t1 = g_hTTS->m_nCurIndexOfConvertData;

				

				//�����ַ��ǣ��ո� �� ��\�����ұ��ַ�ǰ�����ֻ���ĸ���������ֻ���ĸ���������βͣ��Ч��    20111227

				//���磺����Power Downģʽ�� �� 50cm 50kg 50V��
				if (   (    nCurWord == (emInt16)INVALID_CODE)		//�ո�
					&& (    nDataTypePrev == DATATYPE_YINGWEN)						    
					&& (    nDataTypeNext == DATATYPE_YINGWEN
						 || nDataTypeNext == DATATYPE_SHUZI))
				{
					GenPauseNoOutSil();	//�����βͣ��Ч��		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}

				//���磺V60/V66/T720/V300/V303/V500/V600/A768/A768I/  �� �𳵵��ٶ���622km/h�� 
				else if (   nCurWord == (emInt16)0xa3af 			//б��
					&& nDataTypePrev == DATATYPE_SHUZI
					&& nDataTypeNext == DATATYPE_YINGWEN)
				{
					GenPauseNoOutSil();	//�����βͣ��Ч��		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}


				//�����ַ������·��ţ��ұ��ַ�ǰ���ַ����Ǻ��֣��������βͣ��Ч��    2012-03-30
				//���磺����Ѷ(�������٣���С��)��������ġ���������
				//���磺����(m3/h)
				else if(   (   nCurWord == (emInt16)0xA3A8			//���ţ� ��
							|| nCurWord == (emInt16)0xA1B2)			//���ţ� ��
						&& (   nDataTypePrev == DATATYPE_HANZI
							|| nDataTypePrev == DATATYPE_SHUZI
							|| nDataTypePrev == DATATYPE_YINGWEN))								
				{
					GenPauseNoOutSil();	//�����βͣ��Ч��		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}
				else if(   (   nCurWord == (emInt16)0xA3A9			//���ţ� ��
							|| nCurWord == (emInt16)0xA1B3	)		//���ţ� ��							   
					   && (    nDataTypeNext == DATATYPE_HANZI
							|| nDataTypeNext == DATATYPE_SHUZI
							|| nDataTypeNext == DATATYPE_YINGWEN))									
				{
					GenPauseNoOutSil();	//�����βͣ��Ч��		
					g_hTTS->m_nCurIndexOfConvertData += 2;
				}

				else
				{
					g_hTTS->m_nCurIndexOfConvertData = ProcessFuHao(g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);
				}
				
				//����ǣ��ɶԱ�����  
				if( IsShuangBiaoDian(g_hTTS->m_pDataAfterConvert, t1) )
				{
					//��ʼ��ǿ��PPH�ĸ�ֵ�ͼ���
					if( g_ForcePPH.nCount != 0)	 //���ǵ�1�μ���
					{
						//���磺�����ˡ���¥�Ρ���ܿ���
						g_ForcePPH.nSylLen = g_ForcePPH.nCount;
						g_ForcePPH.nCount = 0;
						g_ForcePPH.bIsStartCount = 0;

						
						if( g_ForcePPH.nStartIndex == 0 &&							//�ɶԱ����� �ھ��ӿ�ͷ
							(g_ForcePPH.nSylLen == 1 || g_ForcePPH.nSylLen == 2) &&	//�ɶԱ����� �ں����ַ�������2��
							GetWord(g_hTTS->m_pDataAfterConvert,g_hTTS->m_nCurIndexOfConvertData-2) == (emInt16)0xa3a9) //�ǳɶԷ��� ����
						{
							//���磺��1����ǿ�ҵĴ�ҵԸ�����뷨����11����ǿ�ҵĴ�ҵԸ�����뷨����һ����ǿ�ҵĴ�ҵԸ�����뷨��
							//���磺��Ӧ�����������һ��������ʵʵ�ֵص������ˡ�
							GenPauseNoOutSil();	//�����βͣ��Ч��		
						}
					}
					else						//�ǵ�1�μ���
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
				g_hTTS->m_nCurIndexOfConvertData += 2;		//λ����ǰ�ƶ�2���ֽ�
				break;
			default:  ;
		};
	}
}


