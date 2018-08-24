#include "emPCH.h"


#include "emTTS_Common.h"

extern emBool global_bStopTTS;


//*****************************************	 ȫ�ֱ�������  ************************************************************************

emInt8				g_bIsUserAddHeap;					//�û��Ƿ��ṩ�˶�����ڴ������ʺ��ڴ�ռ主ԣ�Ŀͻ����ɽ����22�ž�����ȫ�����뵽�ڴ棩

struct emHTTS_G		*g_hTTS;							//emTTS��ȫ���������
PLabRTGlobal		g_pLabRTGlobal;						//LAB�ڴ�ȫ�ֱ���
struct tagIsHeap	*g_pHeap;							//�ѵ�ȫ��ָ��

struct TextItem     *g_pTextInfo;						//�ı�����������Ϣ
emByte				*g_pText;							//�ı���������
emInt16				*g_pTextPinYinCode;					//�ı�����ƴ�����벢ƴ������ �����1λ:ƴ�����ͣ�  -----  ƴ�����ͣ� 0��Ĭ�ϣ�1��ǿ��ƴ��
emUInt8				g_nCurTextByteIndex;				//�ı�������ǰ�ֵ�����
emUInt8				g_nLastTextByteIndex,  g_nNextTextByteIndex, g_nP1TextByteIndex, g_nP2TextByteIndex;  //�ı�����������Ϣ����

struct ForcePPHInfo	g_ForcePPH;							//ǿ��PPH���򣺳ɶԱ����ţ�   ���磺�����ˡ���¥�Ρ���ܿ���

emUInt32	       (*g_pOutputCallbackFun)( emPointer , emUInt16,emPByte,emSize);  //��Ƶ�ص�����ָ�룬sqb 32->16
emUInt32	       (*g_pProgressCallbackFun)( emPointer , emUInt32,emSize);  //��Ƶ�ص�����ָ��


#if EM_USER_VOICE_RESAMPLE
TReSampleData		g_objResample[1];					// �ز������� 
#endif


#if DEBUG_LOG_SWITCH_ERROR
FILE				*g_fLogError;						//������־�ļ�ָ��
#endif


//��Ƶ�������
emUInt32	emTTS_OutputVoice(emPointer pParameter, emUInt16 nCode, emPByte pcData, emSize nSize)
{
	emUInt32 nErrorID;
	emInt16 *nPcmOut;
	emSize nSizeOut;
	emSize nReadLen;

	#define RESAMPLE_COUNT_EACH_TIME		1024		//ÿ���ز����Ĳ��������:ע�⣺���ó�256ʱ���������� �� [m55]��������ʱ��20�ա���

#if DEBUG_LOG_SWITCH_TIME
	//myTimeTrace_For_Debug(1,(emByte *)"���--С�β�ǰ", 0);
#endif

	LOG_StackAddr(__FUNCTION__);

	nReadLen = nSize;

	g_hTTS->m_bIsStartBoFang = emTrue;

	//��ֹͣ������� 
	if( g_hTTS->m_bStop == emTrue || global_bStopTTS == emTrue)
	{
		return emTTS_ERR_EXIT;

	}


#if !EM_USER_VOICE_RESAMPLE			//������������ز���

	nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, pcData, nSize); 

	return nErrorID;


#else								//�ز������

	if( g_hTTS->m_bResample != emTrue)					//���������ⷢ���ˣ������ز���
	{
		nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, pcData, nSize); 

#if DEBUG_LOG_SWITCH_TIME
		//myTimeTrace_For_Debug(1,(emByte *)"���--С�β���", 0);
#endif

		return nErrorID;
	}
	else
	{
		nReadLen = nReadLen >> 1;

#if DEBUG_LOG_SWITCH_HEAP
		nPcmOut = (emPInt16)emHeap_AllocZero( RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16), "�ز��������������ģ��  ��");
#else
		nPcmOut = (emPInt16)emHeap_AllocZero( RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16));
#endif

		while ( nReadLen > RESAMPLE_COUNT_EACH_TIME )
		{

			nSizeOut = ReSample(g_objResample, pcData , nPcmOut, RESAMPLE_COUNT_EACH_TIME);

			g_hTTS->m_nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, (emPByte)nPcmOut, nSizeOut*2 ); 
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				return emTTS_ERR_EXIT;
			}
			nReadLen -= RESAMPLE_COUNT_EACH_TIME;
			pcData = (emPointer)((emPInt16)pcData + RESAMPLE_COUNT_EACH_TIME);
		}

		nSizeOut = ReSample(g_objResample, pcData, nPcmOut, nReadLen);

		g_hTTS->m_nErrorID =  ( *  g_pOutputCallbackFun)(pParameter,(emUInt16)nCode, (emPByte)nPcmOut, nSizeOut*2 ); 
		if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
		{
			return emTTS_ERR_EXIT;
		}
		
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free( nPcmOut, RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16), "�ز��������������ģ��  ��");
#else
		emHeap_Free( nPcmOut, RESAMPLE_COUNT_EACH_TIME * 2 * sizeof(emInt16));
#endif

#if DEBUG_LOG_SWITCH_TIME
		myTimeTrace_For_Debug(1,(emByte *)"���--С�β���", 0);
#endif

		return emTTS_ERR_OK;

	}

#endif		//end: EM_USER_VOICE_RESAMPLE
}



// ���ܣ�		�жϵ�ǰ������ʶ���ʲô�䣨2ѡ1�������ľ䣬Ӣ�ľ�
// �õ������ݣ�	1. g_hTTS->m_pDataAfterConvert����ת�����һ������ݣ�
//				2. ѡ����������g_hTTS->m_ControlSwitch.m_nLangType�������ֿ��Ʊ�ǣ�[g1][g2][g0]������
//				3. ǰһ����������g_hTTS->m_nPrevSenType
// ����ֵ��		SEN_TYPE_CN�����ľ䣩��SEN_TYPE_EN��Ӣ�ľ䣩����ʵʱ����g_hTTS->m_nPrevSenType��ֵ������¼��ǰ����������
emInt8	DecideSenType()
{
	emInt16 nIndex, nSumYW;
	emInt16 nDataType, nWordp, nWordn;
	emInt8 nSenType;
	
	// ���ľ��жϣ����ٺ���1�����֣�������һ�������ľ䣩
	// ���ӣ�������PK��UFO�ǲ�������� Tom��Hellen�Ǻ����ѡ�����ʹ�õ���Windows Mobileƽ̨��Windows Mobileƽ̨�ܺ��á�
	nIndex=(emInt16)0;
	while((nDataType = CheckDataType(g_hTTS->m_pDataAfterConvert,nIndex)) != (emInt16)DATATYPE_END)
	{
		if(nDataType == (emInt16)DATATYPE_HANZI)
		{
			g_hTTS->m_nPrevSenType = SEN_TYPE_CN;
			return SEN_TYPE_CN;
		}
		nIndex += 2;
	}


	/** �����־�ķ��ź�Ŀո��ѱ�ȥ����Ӱ��Ӣ�ľ���жϡ� **/

	// Ӣ�ľ��жϣ�����1��������Ӣ�Ĵ�������>1��������ǰ��û�����ֺͷ���"."������β�ķ���"."���㣩
	// ���ӣ�apple e.g. ok.hello. Adm. 5 is 5.ok. ok .	is a.Mrs. Hellen.��Mrs. Hellen.��Mrs�������еĻᱻ��Ϊ�ǲ�ʶ��䣩.
	//       HR&Adm.hi-fi.CD-ROM.E-mail.
	nIndex = (emInt16)0;
	while((nDataType = CheckDataType(g_hTTS->m_pDataAfterConvert,nIndex)) != (emInt16)DATATYPE_END)
	{
		if(nDataType == (emInt16)DATATYPE_YINGWEN)
		{
			nSumYW = GetStrSum(g_hTTS->m_pDataAfterConvert,nIndex,(emInt16)DATATYPE_YINGWEN);
			if(nSumYW > 1)		// ��1��������Ӣ�Ĵ�������>1��
			{
				nWordp = GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)(nIndex-2));			// ȡ������ĸ����ǰһ����
				nWordn = GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)(nIndex+2*nSumYW));	// ȡ������ĸ���ĺ�һ����
				if((((nWordp < (emInt16)0x3a3b0)		// ǰû�����ֺͷ���"."(0xa3ae)
					|| (nWordp > (emInt16)0xa3b9))
					&& (nWordp != (emInt16)0xa3ae))
					&&
					(((nWordn < (emInt16)0xa3b0)		// ��û�����ֺͷ���"."������β�ķ���"."���㣩
					|| (nWordn > (emInt16)0xa3b9))
					&& ((nWordn != (emInt16)0xa3ae) ||		// ��ĩ�ġ�.����0xa3ae��������������в����ԭ�򣩣�1)��ĩ�ġ�.����ֱ�Ӹ���������0xffff����2)��ĩ�ġ�.����һ���ո�0xa3a0�����ٸ���������0xffff��
						((GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)nIndex+2*nSumYW+2) == (emInt16)END_WORD_OF_BUFFER)
						|| 
						((GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)nIndex+2*nSumYW+2) == (emInt16)0xa3a0)
						&&
						(GetWord(g_hTTS->m_pDataAfterConvert,(emUInt32)nIndex+2*nSumYW+4) == (emInt16)END_WORD_OF_BUFFER))))))
				{
					g_hTTS->m_nPrevSenType = SEN_TYPE_EN;
					return SEN_TYPE_EN;
				}
			}
			nIndex = nIndex + 2*nSumYW;
		}
		else
			nIndex += 2;

	}


	// ��ʶ��䣨�ܿ��Ʊ��Ӱ�죩
	// ���ӣ�WWW.sohu.com.cn.3.5kg. mp3.
	//       a. 3.5kg. b. 120cm. .W.C. WWW.sohu.com ���� WWW.sohu.com.cn �� 3.5kg .
	switch (g_hTTS->m_ControlSwitch.m_nLangType)
	{
	case (emInt8)emTTS_LANG_CN:			// ѡ��������[g1]�������ľ䴦��
		nSenType = SEN_TYPE_CN; break;
	case (emInt8)emTTS_LANG_EN:			// ѡ��Ӣ������[g2]����Ӣ�ľ䴦��
		nSenType = SEN_TYPE_EN; break;
	case (emInt8)emTTS_LANG_AUTO:		// ��ѡ���Զ�ʶ������[g0]������ǰ1��Ķ�����ǰ1�������ľ䣬�����ľ䴦�����ƣ�
		nSenType = g_hTTS->m_nPrevSenType; break;
	default:
		 nSenType = SEN_TYPE_CN;
	}

	g_hTTS->m_nPrevSenType = nSenType;		// ��¼��ǰ����������

	return nSenType;
	
}



//����    ���жϲ�������Ч��ʾ��
//����ǰ�᣺1)����ʾ����ǣ�2)����Ӣ�Ĵ�����Ϊ4��6���� �����ı�������ȫ��Сд��ĸ
//����ֵ  ��Ϊ0����ʾ������Ч����ʾ����>0��ʾ����Ч��ʾ���ĳ�������ʾ���ѷ���
emInt8 JudgePrompt(emInt16 nSumOfYingWen,emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nErrorID;
	emInt16 nWord,nWord2;
	emByte  strNeedCmp[11]="";
	emInt16 nTemp; 
	emByte  buffSoundShuZi[4];
	emInt16 nSoundShuZi;

	LOG_StackAddr(__FUNCTION__);

	//���ж��Ƿ��������ͣ� msga, msgb,....
	if( nSumOfYingWen == 4 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  ( nSumOfYingWen - 1 )*2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID != emTTS_ERR_FAILED )
		{
			if( emMemCmp("����",strNeedCmp,6  ) == 0 )
			{
				//������ʾ��msg����
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 6);
				nTemp = PlayPromptPreRecordVoice( PROMPT_MSG, 0, nWord - '��', g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);
				if( nTemp != 0 )				
					return 8;	//����Ч��ʾ�����ѷ�����
			}
		}
	}

	//���ж��Ƿ��������ͣ� sounda, soundb,....
	if( nSumOfYingWen == 6 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  ( nSumOfYingWen - 1 )*2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID != emTTS_ERR_FAILED )
		{
			if( emMemCmp("�������" ,strNeedCmp,10 ) == 0 )
			{
				//������ʾ��sound����
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 10);
				if( nWord  > (emInt16)0xa3f3 )
					nTemp = PlayPromptPreRecordVoice( PROMPT_SOUND, 1, nWord - '��' - 1 , g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);
				else if( nWord  == (emInt16)0xa3f3 )		//ע�⣺ sounds������Ч��ʾ��  (0xa3f3: '��')
					nTemp = 0;
				else
					nTemp = PlayPromptPreRecordVoice( PROMPT_SOUND, 1, nWord - '��', g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);

				if( nTemp != 0 )
					return 12;		//����Ч��ʾ�����ѷ�����
			}
		}
	}

	//���ж��Ƿ��������ͣ� sound101, sound102, sound103,....
	if( nSumOfYingWen == 5 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  ( nSumOfYingWen )*2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID != emTTS_ERR_FAILED )
		{
			if( emMemCmp("�������", strNeedCmp,10   ) == 0 )
			{
				//��sounda�����3λ���֣���Ԥ¼������
				if(    CheckDataType(strDataBuf ,nCurIndexOfBuf+10)==DATATYPE_SHUZI
					&& GetShuZiSum(strDataBuf ,nCurIndexOfBuf+10)==3 )
				{
					buffSoundShuZi[0] = GetWord(strDataBuf,nCurIndexOfBuf+10)-0xa3b0;		//0xa3b0 = �ַ� ��0��
					buffSoundShuZi[1] = GetWord(strDataBuf,nCurIndexOfBuf+12)-0xa3b0;
					buffSoundShuZi[2] = GetWord(strDataBuf,nCurIndexOfBuf+14)-0xa3b0;
					buffSoundShuZi[3] = 0;

					nSoundShuZi  = buffSoundShuZi[0]*100+buffSoundShuZi[1]*10+buffSoundShuZi[2];		//hyl 2012-03-26 �޸ģ�������atoi����ʵ�֣�����ƽ̨��atoi����������⣩

					if( nSoundShuZi < 1000 && nSoundShuZi > 100 &&  (emInt16)(nSoundShuZi/100)*100 != nSoundShuZi )
					{
						//������ʾ��sound����
						nTemp = (emInt16)(nSoundShuZi/100);		//��100��ȡ��   (�����ʾ���Ķ������)
						nSoundShuZi -= nTemp*100 + 1;			//��100��ȡ���� (�����ʾ������µ���Ը���ƫ��)
						nTemp = PlayPromptPreRecordVoice( PROMPT_SOUND,nTemp, nSoundShuZi, g_hTTS->m_nCurPointOfPcmInfo, g_Res.offset_PromptIndex);
						
						if( nTemp != 0 )
							return 16;			//����Ч��ʾ�����ѷ�����					
					}
				}
			}
		}
	}

	return 0;	//������Ч��ʾ��
}


//����    ��ʶ����Ʊ�ǲ�����ȫ�ֱ���
//����ǰ�᣺��ǰ�����ǡ�[��
//����ֵ  ��Ϊ0����ʾ������Ч�Ŀ��Ʊ�ǣ�>0��ʾ����Ч���Ʊ�ǵĳ�����������ȫ�ֿ��Ʊ���
emInt8 JudgeCtrlMark(emInt16 nSumOfFuHao,emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nNWord,nNNWord,nWord,nSumOfShuZi,nWordTemp;

	if( CheckDataType( strDataBuf, nCurIndexOfBuf + 2 ) == DATATYPE_YINGWEN )		//��[��������ַ���: ��ĸ
	{ 
		nNWord  = GetWord( strDataBuf, nCurIndexOfBuf + 2 );
		nNNWord = GetWord( strDataBuf, nCurIndexOfBuf + 4 );

		//��1����ĸ����дתСд
		if(  nNWord >= (emInt16)0xa3c1 && nNWord <= (emInt16)0xa3da)		
			nNWord += 0x20;				

		if( nNWord == (emInt16)0xa3e4 && nNNWord == (emInt16)0xa3dd)  //�ж��Ƿ��ǣ�[d]
		{
			IsValidMarkAndPlayPrev();		//����Ч�Ŀ��Ʊ�ǣ��ڸı����֮ǰ��PCM�����е����ݽ�����������
			InitTextMarkG();				//���á����Ʊ�ǡ���ص�ȫ�ֲ���			
			return 6;						//����Ч���Ʊ��
		}

		//"[*"������ ���ֵ������
		if ( nNNWord >= (emInt16)0xa3b0 && nNNWord <= (emInt16)0xa3b9 )
		{	
			nSumOfShuZi = GetShuZiSum( strDataBuf, nCurIndexOfBuf + 4 );//�����������ļ�λ����
			nWord = GetWord( strDataBuf, nCurIndexOfBuf + 4 + 2 * nSumOfShuZi );
			if ( nWord == ( emInt16 ) 0xa3dd  )//�ж��Ƿ��ǡ�]�����Ʊ�ǣ���Ч�Ŀ��Ʊ��
			{
				nWordTemp = ChangeShuZiToLong( strDataBuf, nCurIndexOfBuf + 4 );

				if( nNWord == (emInt16)0xa3f0 )  //[p*]
					g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;						//����[p*]ͣ�ٲ��������ľ�βsil

				IsValidMarkAndPlayPrev();

				if( nNWord == (emInt16)0xa3f0 )  //[p*]
					g_hTTS->m_ControlSwitch.bIsPauseNoOutPutStartSil   = emTrue;					//����[p*]ͣ�ٲ��������ľ���sil
	
				switch (nNWord)
				{					
					case ( emInt16 ) 0xa3e2:	//[b	���ñ���Ƿ����
						if(nWordTemp==1)								
							g_hTTS->m_ControlSwitch.m_bPunctuation = emTTS_PUNCTUATION_READ ;
						else						
							g_hTTS->m_ControlSwitch.m_bPunctuation = emTTS_PUNCTUATION_NO_READ ;
						break;		

					case ( emInt16 ) 0xa3e5:	//[e]	������Чģʽ//[e*](*=0/1/2/3/4/5/6)
						if((nWordTemp>=0)&&(nWordTemp<=6))
							g_hTTS->m_ControlSwitch.m_nVoiceMode=(emInt8)nWordTemp;
						else
							g_hTTS->m_ControlSwitch.m_nVoiceMode=emTTS_USE_VoiceMode_CLOSE;///��Чģʽ �ر�
						break;

					case ( emInt16 ) 0xa3e6:	//[f	�����ı��ʶ���ʽ
						if(nWordTemp==0)
							g_hTTS->m_ControlSwitch.m_nSpeakStyle = emTTS_STYLE_WORD ;
						else
							g_hTTS->m_ControlSwitch.m_nSpeakStyle = emTTS_STYLE_NORMAL ;
						break;							

					case ( emInt16 ) 0xa3e9:	//[i	����ʶ��ƴ���������
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = emTTS_USE_PINYIN_OPEN;
						else
							g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy = emTTS_USE_PINYIN_CLOSE ;
						break;

					case ( emInt16 ) 0xa3ed:	//[m*]	ѡ������///[m*] (*=3��51��52��53��54��55)
						if(    nWordTemp == 3  || nWordTemp == 51 || nWordTemp == 52 
							|| nWordTemp == 53 || nWordTemp == 54 || nWordTemp == 55 ) 
						{
							g_hTTS->m_ControlSwitch.m_nRoleIndex=(emInt16)nWordTemp;
						}
						else
							g_hTTS->m_ControlSwitch.m_nRoleIndex=emTTS_USE_ROLE_XIAOLIN;

#if EM_8K_MODEL_PLAN	//8Kģ������		ֻ֧�ַ�����[m3] [m51] [m52]
						if( nWordTemp == 53 || nWordTemp == 54 || nWordTemp == 55 ) 
							g_hTTS->m_ControlSwitch.m_nRoleIndex=emTTS_USE_ROLE_XIAOLIN;
#endif
						break;

					case ( emInt16 ) 0xa3ee:	//"[n"	�������ִ������						
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_nReadDigitPolicy = emTTS_READDIGIT_AS_NUMBER ;
						else if(nWordTemp==2)
							g_hTTS->m_ControlSwitch.m_nReadDigitPolicy = emTTS_READDIGIT_AS_VALUE ;
						else
							g_hTTS->m_ControlSwitch.m_nReadDigitPolicy = emTTS_READDIGIT_AUTO ;	
						break;

					case ( emInt16 ) 0xa3f0 :	//[p*]	����ͣ��[p*]
						g_hTTS->m_CurMuteMs = nWordTemp;		
						break;

					case ( emInt16 ) 0xa3f2:	//[r]	���ϲ���						
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_JUSHOU_OPEN ;	//�Ժ�ÿ��ľ���ǿ�ƶ�������
						else if(nWordTemp==2)
							g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_AFTER_OPEN ;		//��������ľ���ǿ�ƶ�������
						else
							g_hTTS->m_ControlSwitch.m_bXingShi = emTTS_USE_XINGSHI_CLOSE;			//�Զ��ж�
						break;

					case ( emInt16 ) 0xa3f3 :	//"[s"	��������
						if(nWordTemp>emTTS_SPEED_MAX)
							nWordTemp=emTTS_SPEED_NORMAL;
						g_hTTS->m_ControlSwitch.m_nVoiceSpeed = (emInt8)nWordTemp ;
						break;

					case ( emInt16 ) 0xa3f4:	//"[t"	�������
						if(nWordTemp>emTTS_PITCH_MAX)
							nWordTemp=emTTS_PITCH_NORMAL;
						g_hTTS->m_ControlSwitch.m_nVoicePitch = (emInt8)nWordTemp ;
						break;

					case ( emInt16 ) 0xa3f6:	//"[v"	��������������ʾ������											
						if(nWordTemp>emTTS_VOLUME_MAX)
							nWordTemp=emTTS_VOLUME_NORMAL;
						g_hTTS->m_ControlSwitch.m_nVolumn = (emInt8)nWordTemp ;					
						break;

					case ( emInt16 ) 0xa3f8:	//"[x"	������ʾ���������						
						if(nWordTemp==0)
							g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy = emTTS_USE_PROMPTS_CLOSE ;
						else
							g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy = emTrue;//emTTS_USE_PROMPTS_OPEN ;
						break;			

					case ( emInt16 ) 0xa3f9:	//"[y"	���ú����С�1���Ķ���
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI ;
						else
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO ;
						break;

					case ( emInt16 ) 0xa3fa:	//"[z"	�������ɱ�ע�������
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_nManualRhythm=emTTS_USE_Manual_Rhythm_OPEN;
						else
							g_hTTS->m_ControlSwitch.m_nManualRhythm=emTTS_USE_Manual_Rhythm_CLOSE;
						break;
					case ( emInt16 ) 0xa3ef:	//"[o"	����Ӣ��0�ĺ������
						if(nWordTemp==1)
							g_hTTS->m_ControlSwitch.m_bZeroPolicy=emTTS_EN_0_OU;
						else
							g_hTTS->m_ControlSwitch.m_bZeroPolicy=emTTS_EN_0_ZERO;
						break;

					case ( emInt16 ) 0xa3e7:	//"[g"	��������
						if(nWordTemp==1)
						{
							g_hTTS->m_ControlSwitch.m_nLangType=emTTS_LANG_CN;
							g_hTTS->m_nPrevSenType = SEN_TYPE_CN;		
						}
						else if(nWordTemp==2)
						{
							g_hTTS->m_ControlSwitch.m_nLangType=emTTS_LANG_EN;
							g_hTTS->m_nPrevSenType = SEN_TYPE_EN;	
						}
						else
							g_hTTS->m_ControlSwitch.m_nLangType=emTTS_LANG_AUTO;
						break;
					default:
						return 0;	//������Ч�Ŀ��Ʊ�ǣ����Ͽ��Ʊ�Ǳ�ʾ�淶�������ڷ�Χ��
					
				}
				return (nSumOfShuZi*2 + 6);	//����Ч�Ŀ��Ʊ��
			}			
		}	
	}

	return 0;		//������Ч�Ŀ��Ʊ�ǣ������Ͽ��Ʊ�Ǳ�ʾ�淶
}