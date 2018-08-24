#include "emPCH.h"

#include "emCommon.h"
#include "emDecode.h"



//���ţ����� �������������ŵľ����������βЧ���ľ�����
//ע�⣺ǰ1��ľ�ŵľ���һ��Ҫ����1����˲�ǰ�ٲ���������1��ľ�ͷ���Ῠ
//������ nMuteMs�������ĺ�����
emTTSErrID emCall PlayMuteDelay(emInt32 nMuteMs)
{
	emUInt32  nReadLen,nMuteLen;
	emPointer pParameter;

	LOG_StackAddr(__FUNCTION__);

	if( nMuteMs> 0)
	{
		if( g_hTTS->m_bIsMuteNeedResample == emTrue)		//�����ڣ����ز����͵ľ��������磺2.���ͣ�٣�	3. �����βͣ��Ч�� GenPauseNoOutSil()����1.����[p1000]�������ز�����
		{
			//�������ⷢ�������������������
			switch( g_hTTS->m_ControlSwitch.m_nRoleIndex )
			{				
				case emTTS_USE_ROLE_Virtual_51:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_51;	break;
				case emTTS_USE_ROLE_Virtual_52:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_52;	break;
				case emTTS_USE_ROLE_Virtual_53:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_53;	break;
				case emTTS_USE_ROLE_Virtual_54:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_54;	break;
				case emTTS_USE_ROLE_Virtual_55:		nMuteMs /= RESAMPLE_SCALE_ROLE_Virtual_55;	break;
				default:	break;			
			}
		}


		pParameter = g_hTTS->pCBParam ;	

		nMuteLen =(emUInt32) ( ((float)nMuteMs/ 500)  * g_hTTS->pRearG->nModelSample );	//hyl 2012-03-23

		while(  nMuteLen > 0)
		{	
			if( nMuteLen > MAX_OUTPUT_PCM_SIZE )
			{
				nReadLen = MAX_OUTPUT_PCM_SIZE;
				nMuteLen -= nReadLen;					
			}
			else
			{
				nReadLen = ( (emInt32)( nMuteLen/2) )*2;
				nMuteLen = 0 ; 
			}
					
			emMemSet(g_hTTS->m_pPCMBuffer,0,MAX_OUTPUT_PCM_SIZE);
			g_hTTS->m_nErrorID =  emTTS_OutputVoice(pParameter,g_hTTS->pRearG->nModelSample, g_hTTS->m_pPCMBuffer,nReadLen);  //�ص�--������������HMM�ϳɵģ�
			
			if( g_hTTS->m_nErrorID == emTTS_ERR_EXIT)
			{
				return emTTS_ERR_EXIT;
			}

		}	

		g_hTTS->m_CurMuteMs = 0;	
		g_hTTS->m_bIsMuteNeedResample = emFalse;

#if DEBUG_LOG_SWITCH_TIME										//hyl 2012-04-10
			myTimeTrace_For_Debug(1,(emByte *)"���岥��--����", 0);
			myTimeTrace_For_Debug(2,(emByte *)"		������ʵ��ʱ����", 0);
			myTimeTrace_For_Debug(3, (emByte *)"" , nMuteMs);
#endif

	}

	return emTTS_ERR_OK;
}


//****************************************************************************************************
//  ���ܣ��ö��ַ������ݱ��в���2���ֽڵ�����										add by songkai
//	���������pCurStr ���������������ڵ��ַ�����nCurIndex ���������ַ���pCurStr �е�����λ��
//            nTable���������ı�����ڴ��е��׵�ַ��nSize�Ǳ����һ����ռ�ݵ��ֽ��� 
//            nStartNum��nTotalNum�ֱ��Ǳ����������������ʼ�кź�����������
//	���ز����������������������������ʼ�е�����ƫ���� (������nTotalNum����ʾû�ҵ�)
//****************************************************************************************************
emUInt16 ErFenSearchOne_FromRAM( emPByte pCurStr, emInt16 nCurIndex, emPByte nTable, emInt16 nStartNum, emUInt16 nTotalNum, emByte nSize )
{
	emUInt16 cMin,cMax,cMiddle, i = 0;
	emByte cHighByte,cLowByte;
	emByte cNextHighByte,cNextLowBte;

	LOG_StackAddr(__FUNCTION__);

	cMin = 0;
	cMax = nTotalNum -1;


	//���������Χ�ĵ�һ��
	cLowByte = *(nTable + i);		      //ȡ����һ���ֽ�
	cHighByte = *(nTable + i + 1);        //ȡ���ڶ����ֽ�
	cNextLowBte = *(nTable + i + 2);      //ȡ���������ֽ�      // add by liugang 2011-5-26
	cNextHighByte = *(nTable + i + 3);    //ȡ���������ֽ�      // add by liugang 2011-5-26
	if( cLowByte == *( pCurStr + nCurIndex + 1 ) &&  cHighByte == *( pCurStr + nCurIndex ) && cNextLowBte == 0 && cNextHighByte == 0 )
	{	
		return cMin;
	}
	if( cMax != 0 )
	{

		//��λ��ָ���Ƶ������������Χ�����һ��λ����λ�ã�����Ƿ��ڸ���
		cLowByte = *(nTable + ( nStartNum - 1 + cMax) * nSize);
		cHighByte = *(nTable + (nStartNum -1 + cMax) * nSize + 1);
		if( cLowByte == *( pCurStr + nCurIndex + 1 ) &&  cHighByte == *( pCurStr + nCurIndex )  && cNextLowBte == 0 && cNextHighByte == 0 )
		{		
			return cMax;
		}
	}
	if( cMax >= 2 ) //������Χ��������ʱ
	{
		cMiddle = ( cMin + cMax) / 2;
		do
		{		
			cLowByte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize);
			cHighByte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 1);
			cNextLowBte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 2);    // add by liugang 2011-5-26
			cNextHighByte = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 3);  // add by liugang 2011-5-26
			if( cLowByte == *( pCurStr + nCurIndex + 1 ) &&  cHighByte == *( pCurStr + nCurIndex ) && cNextLowBte == 0 && cNextHighByte == 0  )
			{		
				return cMiddle;
			}
			if( cHighByte < *( pCurStr + nCurIndex ) || ( cHighByte == *( pCurStr + nCurIndex ) && cLowByte < *( pCurStr + nCurIndex + 1 )) )
			{   //���ҵĺ��ֱ�����ڱ��к��ֱ���
				cMin = cMiddle;
			}
			else 
			{   //���ҵĺ��ֱ���С�ڱ��к��ֱ���
				cMax = cMiddle;
			}
			cMiddle = ( cMin + cMax) / 2;
		}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );
	}	
	return nTotalNum;
}


//****************************************************************************************************
//  ���ܣ��ö��ַ������ݱ��в���ĳ4���������ֽ�        add by songkai
//	���������pCurStr �������Һ������ڵ��ַ�����nCurIndex Ϊ��һ���������ַ���pCurStr �е�����λ��
//            nTable���������ı�����ڴ��е��׵�ַ��nSize�Ǳ����һ����ռ�ݵ��ֽ��� 
//            nStartNum��nTotalNum�ֱ��Ǳ����������������ʼ�кź�����������
//	���ز����������������������������ʼ�е�����ƫ����  (������nTotalNum����ʾû�ҵ�)
//****************************************************************************************************
emUInt16 ErFenSearchTwo_FromRAM( emPByte pCurStr, emInt16 nCurIndex, emPByte nTable, emInt16 nStartNum, emUInt16 nTotalNum, emByte nSize )
{
	emUInt16 cMin,cMax,cMiddle;
	emByte strCodeInTable[5],strCodeSearching[5];
	emByte i;

	LOG_StackAddr(__FUNCTION__);

	cMin = 0;
	cMax = nTotalNum -1;
	for( i = 0; i < 4; i++ )
	{
		strCodeSearching[i] = *( pCurStr + nCurIndex + i);
	}
	strCodeSearching[4] = '\0';

	strCodeInTable[4] = '\0';

	//���ļ�λ��ָ���Ƶ������������ʼ��λ�ã�����Ƿ��ڸ���
	strCodeInTable[1] = *(nTable + (nStartNum - 1) * nSize);
	strCodeInTable[0] = *(nTable + (nStartNum - 1) * nSize + 1);
	strCodeInTable[3] = *(nTable + (nStartNum - 1) * nSize + 2);
	strCodeInTable[2] = *(nTable + (nStartNum - 1) * nSize + 3);	

	if( emMemCmp( strCodeInTable, strCodeSearching,4 ) == 0 )
	{	
		return cMin;
	}
	if( cMax != 0 )                                                 //���ļ�λ��ָ���Ƶ������������ʼ��λ�ã�����Ƿ��ڸ���
	{

		strCodeInTable[1] = *(nTable + ( nStartNum - 1 + cMax) * nSize);
		strCodeInTable[0] = *(nTable + ( nStartNum - 1 + cMax) * nSize + 1);
		strCodeInTable[3] = *(nTable + ( nStartNum - 1 + cMax) * nSize + 2);
		strCodeInTable[2] = *(nTable + ( nStartNum - 1 + cMax) * nSize + 3);
		if( emMemCmp( strCodeInTable, strCodeSearching,4 ) == 0 )
		{		
			return cMax;
		}
	}
	if( cMax >= 2 )
	{
		cMiddle = ( cMin + cMax) / 2;
		do
		{			

			strCodeInTable[1] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize);
			strCodeInTable[0] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 1);
			strCodeInTable[3] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 2);
			strCodeInTable[2] = *(nTable + ( nStartNum - 1 + cMiddle) * nSize + 3);

			if( emMemCmp( strCodeInTable, strCodeSearching ,4) == 0 )
			{		
				return cMiddle;
			}
			if( emMemCmp( strCodeInTable, strCodeSearching ,4) < 0 )		//hyl 2012-03-28 SYN6658�Ŀ⺯������
			{   //���ҵĺ��ֱ�����ڱ��к��ֱ���
				cMin = cMiddle;
			}
			else 
			{   //���ҵĺ��ֱ���С�ڱ��к��ֱ���
				cMax = cMiddle;
			}
			cMiddle = ( cMin + cMax) / 2;
		}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );
	}	
	return nTotalNum;
}





//****************************************************************************************************
// �������������ı�����  (ÿ2���ֽ�ѭ����������)
//		������	emPByte  pStart,		// ���룺��ı������Ĵ洢�ռ����ʼ��ַ�� ͬʱΪ����� ͨ��ָ�봫��
//		������	emInt32 nLen, 			// ���룺��ı������ĴӴ洢�ռ����ʼ��ַ��ʼ���ֽڸ����� 
//		������	emInt32  nVolXiShu,		// ��������
//****************************************************************************************************

void emCall	ChangePcmVol(emInt8 nVoiceType, emPByte  pStart, emInt32 nLen, emInt32  nVol)
{

	 emInt32  nNormalFix;	//������ϵ��
	 emInt32  nPromptFix;	//��ʾ��ϵ��
	 emInt32  nVolXiShu;		//����ϵ��
	 emInt32 nLong;

	 emInt32  i, nMinVolumnValue, nMaxVolumnValue, nSum1;


	 LOG_StackAddr(__FUNCTION__);

	 switch( nVol )	//��������    
	 {	
		 //hyl  ������׼������������������������ģ�  2012-04-16
		case   5:	 nNormalFix =10;	nPromptFix = 5;		break;		//��׼����
		case   6:	 nNormalFix =12;	nPromptFix = 6;		break;
		case   7:	 nNormalFix =14;	nPromptFix = 7;		break;
		case   8:	 nNormalFix =16;	nPromptFix = 8;		break;
		case   9:	 nNormalFix =18;	nPromptFix = 9;		break;
		case  10:	 nNormalFix =20;	nPromptFix =10;		break;		//�������

		case   4:	 nNormalFix = 8;	nPromptFix = 4;		break;
		case   3:	 nNormalFix = 6;	nPromptFix = 3;		break;
		case   2:	 nNormalFix = 4;	nPromptFix = 2;		break;
		case   1:	 nNormalFix = 2;	nPromptFix = 1;		break;
		case   0:	 nNormalFix = 0;	nPromptFix = 0;		break;		//����		

		default	:	 nNormalFix =15;	nPromptFix = 7;		break;


		//case   5:	 nNormalFix =10;	nPromptFix = 5;		break;		//��׼����
		//case   6:	 nNormalFix =13;	nPromptFix = 7;		break;
		//case   7:	 nNormalFix =16;	nPromptFix = 8;		break;
		//case   8:	 nNormalFix =18;	nPromptFix = 9;		break;
		//case   9:	 nNormalFix =20;	nPromptFix =10;		break;
		//case  10:	 nNormalFix =22;	nPromptFix =11;		break;		//�������
	 }

	 if( nVoiceType == VOICE_TYPE_PROMPT)		//�������ͣ���ʾ��
	 {
		 nVolXiShu = nPromptFix;
	 }
	 else
	 {
		 nVolXiShu = nNormalFix;
	 }
	
	 nMinVolumnValue =  -32768 + 500;
	 nMaxVolumnValue =  32768 - 500;


	 //��������ϵ�����ı�����
	 for( i=0; i< nLen ; i=i+2 )			
	 {
		 nSum1 = *((emInt16 *)(pStart + i));

		 nLong = ( nSum1 * nVolXiShu/10);	

		 //�������������
		 if( nLong > nMaxVolumnValue)
			 nLong = nMaxVolumnValue;
		 if( nLong < nMinVolumnValue )
			 nLong = nMinVolumnValue;

		 *((emInt16 *)(pStart + i) ) = (emInt16)nLong;	
	 }

}



//****************************************************************************************************
//�������ܣ� ����Ч�Ŀ��Ʊ�ǣ��ڸı����֮ǰ��PCM�����е����ݽ�����������
//****************************************************************************************************
void emCall IsValidMarkAndPlayPrev()
{

	LOG_StackAddr(__FUNCTION__);

	FirstHandleHanZiBuff(emTrue);	//���Ʊ�Ǵ�������ǿ��ƴ���⣩�������ֻ������������ȴ���	
	//EnTextSynTrigger();		//��Ч���Ʊ��ǰ������Ӣ�ĺϳ�   sqb

}


//****************************************************************************************************
//  
//  ���ܣ�������ʾ����Ԥ¼��
//			����ֵ�� 0��������Ч��ʾ��		1������Ч��ʾ�����ҷ�����
//****************************************************************************************************

emInt16  emCall  PlayPromptPreRecordVoice(
	emInt8 nPromptType,						//���������  PROMPT_MSG �� PROMPT_SOUND
	emInt8 nMoreType,						//���Ķ������ ���磺PROMPT_SOUND�Ķ�������� 1��sound1**  2��sound2**  �ȵ�
	emInt16 nPromptNo,						//�ڼ�����
	struct promptInfo emPtr nCurIndex,		//������Ϣ�����ȣ�ƫ�ƣ���������
	emInt32 nIndexTableOffset)				//����������ƫ���������£�����������ݱ��һ�ˣ�
{
	#define  PROMPT_INDEX_HEAD_LEN  30				//��ʾ������ͷռ���ٸ�˫�ֽڣ�������ʾ����λ����ֵ��	
	#define	 PROMPT_FEATURE_LEN		24				//��ʾ����λ����ֵ������
	#define	 PROMPT_INDEX_LEN		(4<<10)		//��ʾ���������ֵĴ�С


	emInt16 head[PROMPT_INDEX_HEAD_LEN];
	emInt16 nDeodeType, nSample, nChannel, nBit;
	emInt16 nIndexOffset;
	emInt32 nPromptOffset,nPromptLen;

	emUInt16 msgStruct[1][2];		//��ά�ĵ�1����������ƫ�ƣ�  ��ά�ĵ�2�������ܸ����� 
	emUInt16 soundStruct[9][2];		//��ά�ĵ�1����������ƫ�ƣ�  ��ά�ĵ�2�������ܸ����� 

	emInt32 nTableOffset;



	LOG_StackAddr(__FUNCTION__);

	if( nIndexTableOffset == 0 )
	{
		//û�С���ʾ����Դ��
		return 0;
	}

	nTableOffset = nIndexTableOffset + PROMPT_INDEX_LEN;	//�̶���ʾ�����ݲ��ֵĿ�ʼλ�ã���ʾ���������ֵĳ��ȹ̶���

	PlayMuteDelay(g_hTTS->m_CurMuteMs);		//�Ȳ��ž���

	FirstHandleHanZiBuff(emTrue);	//Ӣ�ģ���ʾ������ͨӢ�ģ�ƴ�������������ܽ��ͳɺ��ֵ�Ӣ���⣬�����ֻ������������ȴ���
	//EnTextSynTrigger();		//������ʾ��ǰ������Ӣ�ĺϳ�   sqb

#if DEBUG_LOG_SWITCH_TIME										//hyl 2012-04-12
			myTimeTrace_For_Debug(1,(emByte *)"����ʾ����--��ʼ", 0);
#endif

	fFrontSeek(g_hTTS->fResFrontMain, nIndexTableOffset+PROMPT_FEATURE_LEN  ,0);

	//��ȡ��ʾ���������60��ͷ�ֽڣ�������ʾ����λ����ֵ��
	fFrontRead(head, 2, PROMPT_INDEX_HEAD_LEN, g_hTTS->fResFrontMain);


	nDeodeType   = head[6];
	nSample     = head[7];
	nChannel    = head[8];
	nBit        = head[9];

	emMemCpy(msgStruct,&head[10], 4);
	emMemCpy(soundStruct, &head[12], 36);

	if( nPromptType == PROMPT_MSG)
	{
		if( nPromptNo >= msgStruct[0][1] )  //�������ܸ���
		{
			//������Ч��ʾ��			
			return 0;
		}

		nIndexOffset = 8 *  nPromptNo ;		
	}
	else if( nPromptType == PROMPT_SOUND)
	{
		if( nPromptNo >= soundStruct[nMoreType-1][1] )  //�������ܸ���
		{
			//������Ч��ʾ��
			return 0;
		}

		nIndexOffset = 8 * ( soundStruct[nMoreType-1][0] + nPromptNo);
	}
	
	//��ȡ��ǰ��ʾ���ģ���ʼƫ�ƣ��ܳ���
	fFrontSeek(g_hTTS->fResFrontMain, nIndexOffset , 1);
	fFrontRead(&nPromptOffset, sizeof(emInt32), 1, g_hTTS->fResFrontMain);
	fFrontRead(&nPromptLen, sizeof(emInt32), 1, g_hTTS->fResFrontMain);

	nCurIndex->nPromptDecodeType = nDeodeType;	
	nCurIndex -> nPromptLen      =  nPromptLen;
	nCurIndex -> nPromptOffset   = nPromptOffset; 

	nCurIndex++;
	g_hTTS->m_nCurPointOfPcmInfo = nCurIndex;

	switch( nDeodeType)
	{
		case PROMPT_DECODE_ADPCM:
#if EM_USER_DECODER_ADPCM
			DecodePromptPreRecordAdpcmAndPlay(nSample, nTableOffset);
#endif

			break;
		case PROMPT_DECODE_G7231:
			DecodePromptPreRecordG7231AndPlay(nSample, nTableOffset);	

			break;
		case PROMPT_DECODE_PCM:
			DecodePromptPreRecordPcmAndPlay(nSample, nTableOffset);		
			break;
		default:
			break;
	}

	g_hTTS->m_nCurPointOfPcmInfo = g_hTTS->m_structPromptInfo;
	g_hTTS->m_nHanZiCount_In_HanZiLink = 0;  //��������

	g_nLastTextByteIndex = 0;

	ClearTextItem();	//������������

#if DEBUG_LOG_SWITCH_TIME										//hyl 2012-04-12
			myTimeTrace_For_Debug(1,(emByte *)"����ʾ����--����", 0);
			myTimeTrace_For_Debug(2,(emByte *)"		��ʾ����ʵ��ʱ����", 0);
			myTimeTrace_For_Debug(3, (emByte *)"" , g_hTTS->m_structPromptInfo[0].nPromptLen*1000/1600);
#endif

	PlayMuteDelay(100);		//ǿ�Ʋ���100ms�ľ���  hyl 2012-04-12  ��ֹ������  ���磺���������绰��soundj�������ܲ�ȥ�аְ��������ӵ绰ѽ��

	return  1 ;
}






//****************************************************************************************************
//�������ܣ���ȡԴ�ַ���strSource�е�ĳ���ַ����������临�Ƶ�Ŀ���ַ�������strDest��
//          ��emByte *strSource�ĵ�nStart����ʼ���ƣ���ȡnLen����emByte *strDestt��
//����Ĳ�����emByte *strDest, emByte *strSource, emInt16 nStart,  emInt16 nLen ,  emInt16 nSizeOfDest
//�ı�Ĳ�������emByte *strSource�ĵ�nStart����ʼ���ƣ���ȡnLen����emByte *strDestt��
//����Ĳ����� �����ַ����ɹ�emTTS_ERR_OK�����ַ���ʧ��emTTS_ERR_FAILED
//****************************************************************************************************
emTTSErrID  emCall CpyPartStr( emByte *strDest, emByte *strSource, emInt16 nStart,  emInt16 nLen ,  emInt16 nSizeOfDest)
{
	emInt16  i;

	LOG_StackAddr(__FUNCTION__);

	if( emStrLenA( strSource) < ( nStart + nLen ) )
	{
		return emTTS_ERR_FAILED;
	}

	if( nSizeOfDest <  ( nLen ) )
	{
		return emTTS_ERR_FAILED;
	}


	for( i = 0; i <  nLen ; i++ )
	{
		strDest[i] = strSource[ nStart + i ];

	}
	//strDest[i] = '\0';

	return emTTS_ERR_OK;
}

//****************************************************************************************************
//  ���ܣ���ȡ��ǰ�ַ����Ƿ��ǡ��ɶԱ��������͡�  ��pCurStr��ĿǰnCurIndexλ�ô���ָ�ַ�
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ��� ����
//****************************************************************************************************
emBool emCall  IsShuangBiaoDian( emPByte pCurStr, emInt16  nCurIndex)
{
	emInt16 nIndex = nCurIndex;
	emInt16	nWord = 0;	
	emInt16	nDataType = 0;

	LOG_StackAddr(__FUNCTION__);

	nWord = *(pCurStr+nIndex);		
	nWord =  nWord << 8;		//��ȡ��ǰ˫�ֽڵĸ��ֽ�

	nWord = nWord + (*(pCurStr+(++nIndex)) & (emInt16)0x00ff);		//��ȡ�ĵ�ǰ˫�ֽ��ַ�

	if ( ( nWord >= (emInt16)0xa140 && nWord < (emInt16)0xa3b0)	)		//�Ƿ���
	{	 
		
		if(	   nWord == (emInt16)0xA3A2	// "
			|| nWord == (emInt16)0xA1AE	// ��
			|| nWord == (emInt16)0xA1AF	// ��
			|| nWord == (emInt16)0xA1B0	// ��
			|| nWord == (emInt16)0xA1B1	// ��
			//|| nWord == (emInt16)0xA3A8	// ��			deleted by hyl  2012-03-30  ��֮ǰ�ѵ��� GenPauseNoOutSil();	//�����βͣ��Ч��		
			//|| nWord == (emInt16)0xA3A9	// ��
			//|| nWord == (emInt16)0xA1B2	// ��
			//|| nWord == (emInt16)0xA1B3	// ��
			|| nWord == (emInt16)0xA1B4	// ��
			|| nWord == (emInt16)0xA1B5	// ��
			|| nWord == (emInt16)0xA1B6	// ��
			|| nWord == (emInt16)0xA1B7	// ��
			|| nWord == (emInt16)0xA1BE	// ��
			|| nWord == (emInt16)0xA1BF	// ��
			|| nWord == (emInt16)0xA3FB	// ��
			|| nWord == (emInt16)0xA3FD	// ��
			//|| nWord == (emInt16)0xA3DB	// ��		���Ʊ��Ҳ���������ܲ���
			//|| nWord == (emInt16)0xA3DD	// ��
		   )
		{
			return emTrue;
		}
	}
	else
	{
		return emFalse;
	}
	return nDataType;

}

//****************************************************************************************************
//  ���ܣ���ȡ��ǰ�ַ����������ͼ�pCurStr��ĿǰnCurIndexλ�ô���ָ�ַ�����������
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ�16λ���ε���������nDataType  �������ͻ������ͻ�Ӣ�����ͻ���������
//ע�⣺�жϵ���GBK��˫�ֽ��ַ����ͣ�ʶ��Ӣ�Ĵ�Сд
//****************************************************************************************************
emInt16 emCall  CheckDataType( emPByte pCurStr, emInt16  nCurIndex)
{	

	emInt16 nIndex = nCurIndex;
	emInt16	nWord = 0;	
	emInt16	nDataType = 0;

	LOG_StackAddr(__FUNCTION__);

	nWord = *(pCurStr+nIndex);		
	nWord =  nWord << 8;		//��ȡ��ǰ˫�ֽڵĸ��ֽ�

	nWord = nWord + (*(pCurStr+(++nIndex)) & (emInt16)0x00ff);		//��ȡ�ĵ�ǰ˫�ֽ��ַ�

	if ( nWord == (emInt16)END_WORD_OF_BUFFER )
	{	 
		nDataType = (emInt16)DATATYPE_END;
	}
	else if( ( nWord >= (emInt16)0x8140 && nWord <= (emInt16)0xa0fe ) 
		|| ( nWord >=(emInt16)0xaa40 && nWord <= (emInt16)0xfefe ) )
	{
		nDataType = (emInt16)DATATYPE_HANZI;
	}
	else if( ( nWord >= (emInt16)0xa3c1 && nWord <= (emInt16)0xa3da ) 
		|| ( nWord >= (emInt16)0xa3e1 && nWord <= (emInt16)0xa3fa ) )
	{
		nDataType = (emInt16)DATATYPE_YINGWEN;	//����Сд
	}
	else if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9)
	{
		nDataType = (emInt16)DATATYPE_SHUZI;
	}
	else if (  nWord >= (emInt16)0xa140 && nWord <= (emInt16)0xa9ff )	//����������ֺ���ĸ��֮ǰ��ʶ��
	{
		nDataType = (emInt16)DATATYPE_FUHAO;
	}
	else 
	{
		nDataType = (emInt16)DATATYPE_UNKNOWN;
	}

	return nDataType;
}



//****************************************************************************************************
//  ���ܣ���BUFFER�е��������
//	���������emPByte pCurStr, emInt16 n , nΪ���buffer�е����ݵĳ���
//	���أ��޷���ֵ
//****************************************************************************************************
void ClearBuffer( emPByte pCurStr, emInt16 n )
{

	LOG_StackAddr(__FUNCTION__);

	for( ; n >= 0 ; n--)
		*( pCurStr + n ) = 0;
}

//****************************************************************************************************
//  ���ܣ��õ�ָ���ַ��������
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex	
//	���أ�16λ���ε���emInt16	nWord
//****************************************************************************************************
emInt16 emCall  GetWord( emPByte pCurStr, emUInt32  nCurIndex)
{	
	emUInt32 nIndex = nCurIndex;
	emInt16	nWord = 0;	

	LOG_StackAddr(__FUNCTION__);

    if(nCurIndex>256)
        return nWord;

	nWord = *(pCurStr+nIndex);		
	nWord =  nWord << 8;		//��ȡ��ǰ˫�ֽڵĸ��ֽ�
	nWord = nWord + (*(pCurStr+(++nIndex)) & (emInt16)0x00ff);		//��ȡ�ĵ�ǰ˫�ֽ��ַ�
	return nWord;
}

//****************************************************************************************************
//  ���ܣ���һ���ֵ����ݴ�ŵ�ָ���ַ���ڴ��У�
//        ����ַΪ��pCurStr+nCurIndex��,��ָ�򴦵���������ΪemInt16  nWord	
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex ,��Ҫ�����������emInt16  nWord	
//	���أ��޷���ֵ
//****************************************************************************************************
void PutIntoWord( emPByte pCurStr, emInt16  nCurIndex,emInt16  nWord )
{	
	emInt16  nWordtemp = nWord ;
	emInt16  nIndex = nCurIndex;

	LOG_StackAddr(__FUNCTION__);

	(*(pCurStr+nIndex)) = (emByte)(nWordtemp >> 8);			//��Ÿ��ֽ�
	nIndex += 1;
	(*(pCurStr+nIndex)) = (emByte)nWord;		//��ŵ��ֽ�	
}

//****************************************************************************************************
//  ���ܣ������������ĳ���
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex ����������emInt16	nDataType
//	���أ��������ĸ���nSumOfStr
//****************************************************************************************************
emInt16 emCall  GetStrSum( emPByte pCurStr, emInt16  nCurIndex, emInt16	nDataType )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfStr = 0 ;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == nDataType )
		{
			nSumOfStr++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == nDataType );

	return nSumOfStr;
}
//****************************************************************************************************
//  ���ܣ�������ת����ĺ��ֲ��ţ�������g_hTTS->m_ShuZiBuffer[60]�ڲ�����
//	�������������g_hTTS->m_ShuZiBuffer[60],emInt16 nCurIndexSZBuffer
//	���أ��޷���ֵ  
//****************************************************************************************************
emInt16 emCall SoundSZBuffer( emPByte pCurStr, emInt16 nCurIndexSZBuffer, emInt16 nPos )
{
	emInt16  nNextIndex;

	LOG_StackAddr(__FUNCTION__);

	nNextIndex = WriteToHanZiLink( g_hTTS->m_ShuZiBuffer,nCurIndexSZBuffer , nPos, 0);

	return nNextIndex;

}



//****************************************************************************************************
//  ���ܣ��������ִ�pCurStr��nCurIndexSZBuffer��ʼ�ģ����1���ڵ�д�뵽����������ȥ
//        ���� nPos = POS_CODE_kong					�� ��ʾ�����ִ���ԭʼ���ִ�  
//             nPos != POS_CODE_kong				�� ��ʾ�����ִ���ת�������ĺ��ִ�   �����磺���֣�����ת�������ģ�����Щ�Ǵ��˴��Թ�����
//             nPos != POS_CODE_kong �� nPinYin��=0 �� ��ʾ�����ִ���1�����֣���ǿ��ƴ��  �����磺 ��xiao3����
//		  ���أ� д�꺺�������ָ���nNextCurIndexSZBuffer
//****************************************************************************************************
emInt16 emCall WriteToHanZiLink(emPByte pCurStr, emInt16 nCurIndexSZBuffer,emInt16 nPos, emInt16 nPinYin)
{
	
	emInt16 nLen, nHanZiCount,nTextType;

	emInt16 nNextCurIndexSZBuffer;

	emByte   *pnShengMuNo=0;	    //ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
	emByte   *pnYunMuNo=0;		//ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
	emByte   *pnShengDiaoNo=0;    //ָ�봫�Σ���Чƴ�����������룬��ʼֵΪ0
	emInt16   nValueOfHandelPinYinReturn;
	emByte    PinYin[3] ;

	

	LOG_StackAddr(__FUNCTION__);

	pnShengMuNo = PinYin;
	pnYunMuNo = PinYin + 1;
	pnShengDiaoNo = PinYin + 2;

	nNextCurIndexSZBuffer = nCurIndexSZBuffer;

	

	nHanZiCount = GetHanZiSum( pCurStr , nCurIndexSZBuffer );  //��ȡ�������ִ��ĺ��ָ���

	nTextType = TextCsItem_TextType_HANZI;

	//������ͨӢ����ĸ��Ҳ���ɺ��ִ���  -- 2011-02-15�޸�  Ϊʵ�ֹ��ܣ�Ӣ����ĸ�����������ѵ��
	if( nHanZiCount == 0)
	{
		nHanZiCount = GetYingWenSum( pCurStr , nCurIndexSZBuffer );  //��ȡ����Ӣ�Ĵ��ĺ��ָ���

		if( nHanZiCount >= 6)				//��Ӣ����ĸ������С�ڵ���5����һ���Զ��ꣻ ���ڵ���6����ÿ�ζ�4�� ��
		{
			nHanZiCount = 4;
		}
		nTextType = TextCsItem_TextType_ZIMU;

	}

	//�ж��Ƿ��ǡ�һ��һ�١�������� ���ǣ������������������ֻ����(MAX_HANZI_COUNT_OF_LINK/2)
	//������Ϊһ��һ�ٷ����ÿ���ֺ�Ҫǿ�м�PAU����ֹm_LabRam[MAX_HANZI_COUNT_OF_LINK + 20][]Խ��
	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD)
	{
		
		if( (g_hTTS->m_nHanZiCount_In_HanZiLink + nHanZiCount)> (MAX_HANZI_COUNT_OF_LINK/2)  && nPos == POS_CODE_kong )
		{
			nHanZiCount = (MAX_HANZI_COUNT_OF_LINK/2) - g_hTTS->m_nHanZiCount_In_HanZiLink;	//��ֻ��ȡ���ֺ��ִ�
		}
	}
	else
	{
		//���账��ĺ��ִ�����Դ��ԭʼ���ִ� ���人�ָ������Ϻ���������ĺ��ָ�������MAX_HANZI_COUNT_OF_LINK
		if( (g_hTTS->m_nHanZiCount_In_HanZiLink + nHanZiCount)> MAX_HANZI_COUNT_OF_LINK  && nPos == POS_CODE_kong )
		{
			nHanZiCount = MAX_HANZI_COUNT_OF_LINK - g_hTTS->m_nHanZiCount_In_HanZiLink;	//��ֻ��ȡ���ֺ��ִ�
			g_hTTS->m_FirstSynthSegCount = 1;		//2012-04-16   ��ֹ����  ���磺[s10]Ӣ����ĸ�����ͨ3G�����ڡ��֡�һ������������Լҵķ�ǰ������UFO����һ����������PK��ʱ����ҵӦ��ǰ��600�׽ӽ�Ŀ�ĵ��ϵس���վ��ʻ2.5������ǰ����ʻ���ϵ���Ϣ·��ӭ�������°�ʿ��ʮ��·������Ʊ�����������ܶ���475.25Ԫ����500Ԫ����24.75Ԫ����������绰���Խ�������������Ϣ����Ϣ����13905511861[d]

		}
	}

	if( nHanZiCount > 0 )
	{
		g_hTTS->m_nHanZiCount_In_HanZiLink += nHanZiCount;

		g_hTTS->m_nCurIndexOfAllHanZiBuffer += nHanZiCount;

		nLen = nHanZiCount * 2; 
		nNextCurIndexSZBuffer = nLen + nCurIndexSZBuffer;	


		//���캺���������1�ڵ�  -- ����
		g_pTextInfo[g_nLastTextByteIndex/2].Pos = nPos;
		g_pTextInfo[g_nLastTextByteIndex/2].Len = nLen;
		g_pTextInfo[g_nLastTextByteIndex/2].TextType = nTextType;
		if( !emMemCmp( pCurStr + nCurIndexSZBuffer+nLen,"��",2))
		{
			emMemCpy(g_pTextInfo[g_nLastTextByteIndex/2].nBiaoDian , "��",2);
		}
		emMemCpy( &g_pText[g_nLastTextByteIndex], pCurStr + nCurIndexSZBuffer ,nLen); //�������ִ�
		if( nPinYin != 0 )											//�ж����������  ��hang2�г�
		{
			emMemCpy(&g_pTextPinYinCode[g_nLastTextByteIndex/2], &nPinYin,2);		
		}
		else
		{
			//�жϷ����Ƿ�"[="  �����Ǻϸ��ǿ��ƴ����Ӧһ��д�뵽����buff �� ���ж��"[="�ϸ��ǿ��ƴ�����Ե�һ��Ϊ��Ч�����ඪ��
			while((GetWord(  pCurStr,nCurIndexSZBuffer+nLen ) == (emInt16)0xa3db)&&(GetWord(  pCurStr,nCurIndexSZBuffer+nLen + 2 ) == (emInt16)0xa3bd))
			{
				(*pnShengMuNo)		= 255;		//��ʼ�� hyl 2012-03-27
				(*pnYunMuNo)		= 255;
				(*pnShengDiaoNo)	= 255;

				nValueOfHandelPinYinReturn=GetPinYinCode(pCurStr,nCurIndexSZBuffer+nLen,emFalse,pnShengMuNo,pnYunMuNo,pnShengDiaoNo );
				if (   nValueOfHandelPinYinReturn != 0									//�ϸ��ƴ����Ϣ
					&& CheckDataType(pCurStr, nCurIndexSZBuffer) !=DATATYPE_YINGWEN)	//�ұ��ֲ���Ӣ����ĸ
				{
					nNextCurIndexSZBuffer += nValueOfHandelPinYinReturn;
					if( (*pnYunMuNo)<100 && (*pnShengMuNo)<100 && (*pnShengDiaoNo)<100)
					{
						//��Чƴ��
						g_pTextPinYinCode[(g_nLastTextByteIndex+g_pTextInfo[g_nLastTextByteIndex/2].Len)/2 -1 ] = 0x8000 + (*pnYunMuNo)*256 + (*pnShengMuNo)*8 + (*pnShengDiaoNo); //��ֵΪǿ��ƴ��
					}
					else
					{
						//��Чƴ��
						g_pTextPinYinCode[(g_nLastTextByteIndex+g_pTextInfo[g_nLastTextByteIndex/2].Len)/2 -1 ] = 0;
					}
					//������ǿ��ƴ�������Ƿ��к��֣��еĻ����������ĺ��ִ�һ����
					nCurIndexSZBuffer = nNextCurIndexSZBuffer;
					nHanZiCount = GetHanZiSum( pCurStr , nNextCurIndexSZBuffer );  //��ȡ�������ִ��ĺ��ָ���
					if( nHanZiCount > 0 )
					{
						g_hTTS->m_nHanZiCount_In_HanZiLink += nHanZiCount;		
						g_hTTS->m_nCurIndexOfAllHanZiBuffer += nHanZiCount;
						nLen = nHanZiCount * 2; 
						nNextCurIndexSZBuffer = nLen + nCurIndexSZBuffer;	
						//����������ı��ڵ�����ֵ������Ӧ�ĸı�						
						emMemCpy( &g_pText[g_nLastTextByteIndex+g_pTextInfo[g_nLastTextByteIndex/2].Len], pCurStr + nCurIndexSZBuffer ,nLen); //�������ִ�
						g_pTextInfo[g_nLastTextByteIndex/2].Len += nLen;
					}
				}
				else
				{
					break;
				}
			}	
		}
		

		g_hTTS->m_nCurIndexOfConvertData = nNextCurIndexSZBuffer;	

		if( nPos == POS_CODE_kong)       //��Դ��ԭʼ���ִ���������Դ��ת�����ĺ��ִ�
		{
			//�ж�g_Text���ִ����Ƿ���Ҫ�ú��������ֶ��ɺ��룻����Ļ���ֱ���ں����︳ֵ
			
			JustHanZiLetDigitToPhone();
		}

		g_nCurTextByteIndex = g_nLastTextByteIndex;

		g_nLastTextByteIndex += g_pTextInfo[g_nLastTextByteIndex/2].Len;



	}

	//�ж��Ƿ��ǡ�һ��һ�١�������� ���ǣ������������������ֻ����(MAX_HANZI_COUNT_OF_LINK/2)
	//������Ϊһ��һ�ٷ����ÿ���ֺ�Ҫǿ�м�PAU����ֹm_LabRam[MAX_HANZI_COUNT_OF_LINK + 20][]Խ��
	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD)
	{
		if (g_hTTS->m_nHanZiCount_In_HanZiLink >= (MAX_HANZI_COUNT_OF_LINK/2) ) 
		{
			g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;	//����[p*]ͣ�ٲ��������ľ�βsil
			FirstHandleHanZiBuff(emTrue);	
		}
	}
	else
	{
		if (g_hTTS->m_nHanZiCount_In_HanZiLink >= MAX_HANZI_COUNT_OF_LINK ) //�����������еĺ��ָ�������MAX_HANZI_COUNT_OF_LINK
		{
			g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;	//����[p*]ͣ�ٲ��������ľ�βsil
			FirstHandleHanZiBuff(emTrue);	//���������еĺ��ָ�������50���������������ֻ������������ȴ���
			g_hTTS->m_FirstSynthSegCount = 1;		//2012-04-16   ��ֹ����  ���磺[s10]Ӣ����ĸ�����ͨ3G�����ڡ��֡�һ������������Լҵķ�ǰ������UFO����һ����������PK��ʱ����ҵӦ��ǰ��600�׽ӽ�Ŀ�ĵ��ϵس���վ��ʻ2.5������ǰ����ʻ���ϵ���Ϣ·��ӭ�������°�ʿ��ʮ��·������Ʊ�����������ܶ���475.25Ԫ����500Ԫ����24.75Ԫ����������绰���Խ�������������Ϣ����Ϣ����13905511861[d]

		}
	}

	return nNextCurIndexSZBuffer;	
}


//****************************************************************************************************
//�������ܣ� �����������
//****************************************************************************************************
void emCall ClearTextItem()
{
	emInt8 i;

	LOG_StackAddr(__FUNCTION__);

	for(i=0;i<MAX_HANZI_COUNT_OF_LINK_NEED_HEAP;i++)
	{
		g_pTextInfo[i].BorderType = 0;
		g_pTextInfo[i].Len = 0;
		g_pTextInfo[i].Pos = 0;
		g_pTextInfo[i].TextType = 0;
		g_pTextInfo[i].nBiaoDian[0] = 0;
		g_pTextInfo[i].nBiaoDian[1] = 0;

	}
	emMemSet(g_pText,			  0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);				
	emMemSet(g_pTextPinYinCode, 0 , MAX_HANZI_COUNT_OF_LINK_NEED_HEAP*2);	
	g_nLastTextByteIndex=0; 
	g_nCurTextByteIndex=0; 
	g_nNextTextByteIndex=0; 
	g_nP1TextByteIndex=0; 
	g_nP2TextByteIndex=0; 

	emMemSet( g_hTTS->m_ManualRhythmIndex, 0, MAX_MANUAL_PPH_COUNT_IN_SEN);
	g_hTTS->m_CurManualRhythmIndex = 0;

	g_ForcePPH.nStartIndex	= 0;
	g_ForcePPH.nSylLen	= 0;
	g_ForcePPH.nCount	= 0;
	g_ForcePPH.bIsStartCount= 0;
}

//****************************************************************************************************
//  ���ܣ��ж�g_Text���ִ����Ƿ���Ҫ�ú��������ֶ��ɺ��룻����Ļ���ֱ���ں����︳ֵ
//****************************************************************************************************

void emCall JustHanZiLetDigitToPhone() 
{
	emInt16 nLastHanZiNum,nSumOfHanZi,nSumOfShuZi;
	emByte i;                                   //ѭ������
	emInt16 nDataType = 0;                      //��¼�ַ�������
	emInt16 nDataTypeAfterHanZi = 0;  
	emInt16 nWord = 0;                          //��ʱ���һ�����֣����������ţ�

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetShuZiSum( g_hTTS->m_pDataAfterConvert,g_hTTS->m_nCurIndexOfConvertData );
	nDataTypeAfterHanZi = CheckDataType( g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData);

	nSumOfHanZi = emStrLenA(g_pText)/2;

	//���ִ�������ϣ��ж��Ƿ���Ҫ�������������ֶ��ɺ��롱���ش�	
	nLastHanZiNum = ( nSumOfHanZi >= (emInt16) KEYWORD_SEARCH_COUNT ) ? (emInt16) KEYWORD_SEARCH_COUNT : nSumOfHanZi;


	//��֮ǰ�����뿪���Ǵ򿪵ģ��ҽ������ĺ��ִ���Ϊ��ת������ ������ ��������������뿪�ؼ�����
	if( g_hTTS->m_ControlSwitch.m_bIsPhoneNum == emTrue )
	{		
		nWord = GetWord( g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData -2);
		if( nWord == ( emInt16 ) 0xd7aa ||  nWord == ( emInt16 ) 0xbbf2 ||  nWord == ( emInt16 ) 0xb5bd ||  nWord == ( emInt16 ) 0xbaf4)
		{  
			if(CheckDataType( g_hTTS->m_pDataAfterConvert, g_hTTS->m_nCurIndexOfConvertData -4) == DATATYPE_SHUZI)
			{
				g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
				g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
				return;
			}
		}

	}

	g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emFalse;
	
	//���ú��ִ���5���������Ƿ�������л�������;������ͨ�����ֻ������绰��������һ���ʡ�
	//���������Ҹú��ִ�����ֵĵ�һ�����ֲ��ǡ�Ԫ���������������ֶ��ɺ��롱���ش򿪣�����1������ɡ��ۡ�
		if(    (nSumOfShuZi >= 3  && nDataTypeAfterHanZi == DATATYPE_SHUZI)
		|| nDataTypeAfterHanZi != DATATYPE_SHUZI)		//Ҫô���ֺ��3λ�������֣�Ҫô���Ĳ�������
	{		
		for( i = 0; i <= ( nLastHanZiNum - 2 ); i++ )
		{ //���ú��ִ���6���������Ƿ�������ݱ�Table_HanZiLetDigitToPhone�еĵ������                                �������ִ������ִ�֮ǰ��N�������г���--���봦��Ԫ��
			if ( CheckHanZiLetDigitToPhoneTable( g_pText,  (nSumOfHanZi-nLastHanZiNum + i)*2 , 4 ) == emTrue)
			{//�ҵ��ȴʣ��жϺ��ִ���ĵ�һ�������Ƿ�Ϊ��Ԫ��
				i = g_hTTS->m_nCurIndexOfConvertData-2;  
				do
				{	
					i += 2;
					nDataType = CheckDataType( g_hTTS->m_pDataAfterConvert,i);
					
				}while( nDataType != DATATYPE_HANZI && nDataType != DATATYPE_END );
				nWord = GetWord(g_hTTS->m_pDataAfterConvert, i);
				if( nWord == ( emInt16 ) 0xd4aa )
				{   //���ִ����һ�������ǡ�Ԫ�������������������������ֶ��ɺ��롱���ش򿪣�����1������ɡ��ۡ����˳�����
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emFalse;
				}
				else 
				{
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
					g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
				}
			}
		}   
	}

	//���ú��ִ���������������Ƿ���������ݱ�Table_HanZiLetDigitToPhone���Һ������3λ��3λ��������--                  �������ִʽ��������ִ�֮ǰ��
	//�����������������������ֶ��ɺ��롱���ش򿪣�����1������ɡ��ۡ�
	if(    nLastHanZiNum >= 2
		&& nDataTypeAfterHanZi == DATATYPE_SHUZI
		&& nSumOfShuZi >= 3)
	{   //�ñ�������Table_HanZiLetDigitToPhone�в��Һ��ִ���������֡��ñ��15�У�ÿ��ռ4���ֽ�
		if ( CheckHanZiLetDigitToPhoneTable(  g_pText, (nSumOfHanZi-2 )*2, 2 ) == emTrue)
		{//���������������������ֶ��ɺ��롱���ش򿪣�����1������ɡ��ۡ����˳�����
			g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
			g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
		}
	}

	//���ú��ִ���6���������Ƿ�������ݱ�Table_HanZiLetDigitToPhone�е�����һ��,   �������ִ������ִ�֮ǰ��N�������г��֡�
	//���У������������ֶ��ɺ��롱���ش򿪣�����1������ɡ��ۡ�	
	if( nLastHanZiNum >= 2 )
	{
		//Ҫô���ֺ��3λ�������֣�Ҫô���Ĳ�������
		if(    (nSumOfShuZi >= 3  && nDataTypeAfterHanZi == DATATYPE_SHUZI)
			|| nDataTypeAfterHanZi != DATATYPE_SHUZI)
		{
			//�����������ĺ��ִ�ĩβ���ָ�������1ʱ�����ñ���������Ƿ�������ݱ�Table_HanZiLetDigitToPhone�е�����һ�����ִ�
			for( i = 0; i <= ( nLastHanZiNum - 2 ); i++ )
			{
				if ( CheckHanZiLetDigitToPhoneTable( g_pText,(nSumOfHanZi-nLastHanZiNum + i)*2 , 1 ) == emTrue )
				{   

					//���������������������ֶ��ɺ��롱���ش򿪣�����1������ɡ��ۡ����˳�����			
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;		
					g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;	
				}
			} 
		}
	}
}


//****************************************************************************************************
//  ���ܣ��жϴ˽ڵ�ĺ��ִ����Ƿ���Ҫ�ú��������ֶ��ɺ���
//	���������gbk��������gbk���� emInt16 nLuoMaShuZi 
//	���أ����ֵ�gbk���� emInt16 nHanZiOfShuZi 
//****************************************************************************************************
emInt16  emCall  ChangeShuZiToHanZi( emInt16 nLuoMaShuZi )
{
	emInt16 nHanZiOfShuZi =0;

	LOG_StackAddr(__FUNCTION__);

	if( nLuoMaShuZi > (emInt16)0xa3b9 || nLuoMaShuZi < (emInt16)0xa3b0 )
	{   
		return  emTTS_ERR_EXIT; //ת�������ݲ����������˳�
	}
	else 
	{
		switch( nLuoMaShuZi )
		{ 
		case (emInt16)0xa3b9: nHanZiOfShuZi =  (emInt16)0xbec5;break;//9ת��
		case (emInt16)0xa3b8: nHanZiOfShuZi =  (emInt16)0xb0cb;break;//8ת��
		case (emInt16)0xa3b7: nHanZiOfShuZi =  (emInt16)0xc6df;break;//7ת��
		case (emInt16)0xa3b6: nHanZiOfShuZi =  (emInt16)0xc1f9;break;//6ת��
		case (emInt16)0xa3b5: nHanZiOfShuZi =  (emInt16)0xcee5;break;//5ת��
		case (emInt16)0xa3b4: nHanZiOfShuZi =  (emInt16)0xcbc4;break;//4ת��
		case (emInt16)0xa3b3: nHanZiOfShuZi =  (emInt16)0xc8fd;break;//3ת��
		case (emInt16)0xa3b2: nHanZiOfShuZi =  (emInt16)0xb6fe;break;//2ת��
		case (emInt16)0xa3b1: 
			{
				if ( g_hTTS->m_ControlSwitch.m_bYaoPolicy == emTTS_CHNUM1_READ_YAO)
				{
					nHanZiOfShuZi =  (emInt16)0xe7db;break;//1ת��
				}
				nHanZiOfShuZi =  (emInt16)0xd2bb;break;//1תһ
			}
		case (emInt16)0xa3b0: nHanZiOfShuZi =  (emInt16)0xc1e3;break;//0ת��
		}
	}
	return nHanZiOfShuZi;
}
//****************************************************************************************************
//  ���ܣ������ִ�ת��Ϊ������ 
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex�������ִ�����ʼλ��
//	���أ�emInt32ֵ   
//****************************************************************************************************
emInt32 emCall  ChangeShuZiToInt( emPByte pCurStr, emInt16  nCurIndex )
{
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfShuZi = 0;
	emInt32 nReturnValue = 0 ;
	emInt16 nWordTemp = 0 ;

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	nReturnValue = GetWord( pCurStr,nIndex );
	nReturnValue = nReturnValue - 0xa3b0;
	if ( nSumOfShuZi <= 5 )
	{  
		while ( nSumOfShuZi )
		{   
			nSumOfShuZi -= 1;
			if ( nSumOfShuZi == 0)
			{
				break;
			}
			else
			{
				nIndex += 2;
				nWordTemp = GetWord( pCurStr,nIndex );
				nReturnValue = nReturnValue  * 10 + (nWordTemp - 0xa3b0) ;
			}
		} 
	}
	return nReturnValue;
}

//****************************************************************************************************
//  ���ܣ������ִ�ת��Ϊ�������� 
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex�������ִ�����ʼλ��
//	���أ�emInt32ֵ    
//****************************************************************************************************
emInt32 emCall  ChangeShuZiToLong( emPByte pCurStr, emInt16  nCurIndex )
{
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfShuZi = 0;
	emInt16 nWordTemp = 0 ;
	emInt32 nResult = 0;

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	nWordTemp = GetWord( pCurStr,nIndex ) - 0xa3b0;

	nResult = nWordTemp;
	  
	while ( nSumOfShuZi )
	{   
		nSumOfShuZi -= 1;
		if ( nSumOfShuZi == 0)
		{
			break;
		}
		else
		{
			nIndex += 2;
			nWordTemp = GetWord( pCurStr,nIndex )  - 0xa3b0;
			nResult = nResult  * 10 + nWordTemp ;
		}
	} 

	return nResult;
}
//****************************************************************************************************
//  ���ܣ������ִ������뷢��
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����������ִ��ж��ɺ���ĸ��� nSumOfReadHaoMa
//  �ı��ȫ�ֲ�����g_hTTS->m_ShuZiBuffer
//****************************************************************************************************
emInt16 emCall  ReadDigitHaoMa( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadHaoMa = 0 ;
	emInt16 ntempCount = 0;
	emInt16	nDataType = 0;
	emInt16 nLuoMaShuZi;
	emInt16 nHanZiOfShuZi;
	emInt16 nCurIndexSZBuffer = 0;
	emInt16 nLeaveShuZiLen;  
	emInt16 nTotalShuZiLen;  

	LOG_StackAddr(__FUNCTION__);


	g_hTTS->m_ControlSwitch.m_bYiBianYinPolicy = emTTS_USE_YIBIANYIN_CLOSE;		//����һ���ı���ȫ�ֱ���ֵ�رգ�����һ�������������3������Ȼ����� 

	nLeaveShuZiLen = GetShuZiSum( pCurStr, nIndex)*2;  
	nTotalShuZiLen = nLeaveShuZiLen;

	do
	{
		nDataType = CheckDataType(pCurStr, nIndex);
		if( nDataType != DATATYPE_SHUZI ) 
		{   
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, END_WORD_OF_BUFFER );
		}

		else 
		{
			nLuoMaShuZi = GetWord( pCurStr,nIndex );
			nHanZiOfShuZi = ChangeShuZiToHanZi( nLuoMaShuZi );
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, nHanZiOfShuZi );
			nCurIndexSZBuffer += 2;
			nLeaveShuZiLen -= 2;
			nIndex += 2;
			ntempCount += 1;  			

			//����ÿ4λ��1�Σ��Ӻ�����  ���磺13910008888 ����ɣ� 139  1000  8888�� ����λ��С�ڵ���5λʱ���ڷֿ������磺�绰��10086������101����
			//����ǰͷֻʣ1λʱ����1λ�ͺ���4λһ��������磺QQ��568790234��
			if( ((emInt32 )(nLeaveShuZiLen/8)*8) == (nLeaveShuZiLen)  && nTotalShuZiLen>=10 && (nTotalShuZiLen-nLeaveShuZiLen)>=4 && nLeaveShuZiLen>0)  //hyl 1220    
			{				
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, END_WORD_OF_BUFFER );
				nCurIndexSZBuffer = 0;
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, POS_CODE_m  );			//���ţ�����
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������

				GenPauseNoOutSil();	//�����βͣ��Ч��		
				
			}
			if( ntempCount != 29 )		//�ж��Ƿ񳬳����ֻ�������洢��60 ��g_hTTS->m_ShuZiBuffer[60]��30������
			{
				continue;
			}
			else 
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, END_WORD_OF_BUFFER );
			}
		}

		nCurIndexSZBuffer = 0;
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, nCurIndexSZBuffer, POS_CODE_m  );			//���ţ�����
		ClearBuffer( g_hTTS->m_ShuZiBuffer, (ntempCount+1) * 2 );			//������ֻ����д洢������
		nSumOfReadHaoMa += ntempCount;
		ntempCount = 0;
		nDataType = CheckDataType( pCurStr, nIndex );
	}while( nDataType == DATATYPE_SHUZI );


	g_hTTS->m_ControlSwitch.m_bYiBianYinPolicy = emTTS_USE_YIBIANYIN_OPEN; //��ԭ���뺯��ǰ�ġ�һ���ı���ȫ�ֱ���ֵ

	return nSumOfReadHaoMa;
}

//****************************************************************************************************
//  ���ܣ������ִ���С������ֵ����
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����������ִ��ж���С������ֵ�ĸ��� nSumOfReadShuZhi
//  ע�⣺��û��С�������ֵ���������Ҳ����ȷ����
//****************************************************************************************************
emInt16 emCall  ReadDigitShuZhiAddDian( emPByte pCurStr, emInt16  nCurIndex )
{
	 emInt16 nSumOfReadShuZhi, nTemp, nSumOfReadHaoMa, nTotalSum;

	 LOG_StackAddr(__FUNCTION__);

	 nSumOfReadShuZhi = ReadDigitShuZhi( pCurStr, nCurIndex );
	 if( GetWord( pCurStr,nCurIndex+nSumOfReadShuZhi*2 ) == (emInt16) 0xa3ae)		//������С����
	 {
		 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb5e3 ); //��
		 PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
		 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);	//���ţ���
		 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
		 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
		 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
		 nSumOfReadHaoMa = ReadDigitHaoMa(pCurStr,nCurIndex+nSumOfReadShuZhi*2+2);
		 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
		 nTotalSum =  nSumOfReadShuZhi+1+nSumOfReadHaoMa;
		 return nTotalSum;

	 }
	 else
	 {
		 nTotalSum =  nSumOfReadShuZhi;
		 return nTotalSum;
	 }
}

//****************************************************************************************************
//  ���ܣ������ִ�����ֵ����
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����������ִ��ж�����ֵ�ĸ��� nSumOfReadShuZhi
//****************************************************************************************************
emInt16 emCall  ReadDigitShuZhi( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadShuZhi = 0 ;
	emInt16 nSumOfTemp = 0 ;
	emInt16 nCountTemp = 0;
	//emInt16	nTemp_gValue ;
	emInt16 i;
	emInt16 nSoundSZOffset;

	LOG_StackAddr(__FUNCTION__);

	
	ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������

	nSumOfReadShuZhi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	for ( i = 0; i < nSumOfReadShuZhi; i++ )
	{
		if ( GetWord( pCurStr, nIndex + 2*i ) == (emInt16)0xa3b0 )
		{
			nCountTemp += 1; //���ִ�ǰ0�ĸ���
			continue;
		}
		break;
	}
	//���ִ�ȫΪ0��ֻ��һ�����㡱����
	if ( nCountTemp == nSumOfReadShuZhi )
	{
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ� ��ֵ
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������
	}

	nIndex += nCountTemp * 2;
	nSumOfTemp = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );
	while( (nSumOfTemp <= 16) && (nSumOfTemp > 12) )
	{
		g_hTTS->m_bIsErToLiang = emTrue;
		nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex, nSumOfTemp - 12);
		nIndex += (nSumOfTemp - 12) * 2 ;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset, 0xcdf2 ); //��λ����
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset + 2, END_WORD_OF_BUFFER );
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ� ����
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������
		nSumOfTemp = 12;
	}
	while( (nSumOfTemp <= 12) && (nSumOfTemp > 8) )
	{
		g_hTTS->m_bIsErToLiang = emTrue;
		nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex, nSumOfTemp - 8);
		nIndex += (nSumOfTemp - 8) * 2 ;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset, 0xd2da ); //��λ���ڡ�
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset + 2, END_WORD_OF_BUFFER );
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���š��ڡ�
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������
		nSumOfTemp = 8;
	}
	while( (nSumOfTemp <= 8) && (nSumOfTemp > 4) )
	{   
		for ( i = 0,nCountTemp = 0; i < 4; i ++ )
		{
			if ( GetWord( pCurStr, nIndex + 2*i ) == (emInt16)0xa3b0 )
			{
				nCountTemp += 1; //���ִ�0�ĸ���
				continue;
			}
			break;
		}
		if ( nCountTemp != 4 ) //���ִ���0�ĸ�����Ϊ4ʱ
		{
			g_hTTS->m_bIsErToLiang = emTrue;
			nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex , nSumOfTemp - 4);
			nIndex += (nSumOfTemp - 4) * 2 ;
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset, 0xcdf2 ); //��λ����
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nSoundSZOffset + 2, END_WORD_OF_BUFFER );
			SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );			//���ţ� ����
			ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������
			nSumOfTemp = 4;
		}
		else 
		{
			if ( GetWord( pCurStr, nIndex + 2*4 ) != (emInt16)0xa3b0 )
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ� ��ֵ
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������
			}
			nIndex += (nSumOfTemp - 4) * 2 ;
			nSumOfTemp = 4;
		}
	}
	//��1-4λ�����ִ�
	if(  CheckDataType( pCurStr,nIndex+2*nSumOfTemp ) == DATATYPE_HANZI   )
	{
		g_hTTS->m_bIsErToLiang = emTrue;		
	}
	nSoundSZOffset = ReadDigitShuZhiOfFour( pCurStr, nIndex, nSumOfTemp);

	SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ� ��ֵ
	ClearBuffer( g_hTTS->m_ShuZiBuffer, 20);			//������ֻ����д洢������


	return nSumOfReadShuZhi;
}
//****************************************************************************************************
//  ���ܣ���������4�����ֵ����ִ�����ֵ����
//			����1����ǰ�ַ���emPByte pCurStr��
//			����2�����λ����emInt16  nCurIndex
//			����3����ȡ���ֵĸ���emInt16 nSumOfRead
//	���أ��޷���ֵ
//****************************************************************************************************
emInt16 emCall  ReadDigitShuZhiOfFour( emPByte pCurStr, emInt16  nCurIndex, emInt16 nSumOfRead)
{
	emInt16 nIndex = nCurIndex;
	emInt16 nIndexOfBuf = 0;
	emInt16 nCopyYaoPolicy = 0 ;
	emInt16 nWordTemp = 0,nNextWord ,nNextNextWord,nPrevWord;
	emInt16 nWord = 0 ;
	emBool  bIsHaveLiang = emFalse;		//�Ƿ��Ѿ���1��������ת���ˡ�����

	LOG_StackAddr(__FUNCTION__);

	nCopyYaoPolicy = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
	g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;

	//������ת���ɡ������Ĺ���
	//����1.������ֵ�в��п���ת�������в�ת
	//����2.��4λһ���У��׸������ǡ��������ҳ�����ǧλ���λ����ת�ɡ�������
	//����3.��4λһ���У�һ����ֻ�и�λ�ġ�������ǧλ��λʮλ��û�����������������ٻ���4λ��������Ǹ����ĺ��ֻ��ڡ�2:30���У������������ҲӦת�ɡ�����
	switch ( nSumOfRead )
	{
	case 4: //ʣ��λ��
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );
		//������ת���ɡ������ж�
		if( nWordTemp == (emInt16)0xb6fe && bIsHaveLiang == emFalse)		//���ڡ�����
		{				
			nWordTemp = (emInt16)0xc1bd;		//ת���ɡ�����
		}
		bIsHaveLiang = emTrue;
		nIndex += 2;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
		nIndexOfBuf += 2;
		if ( nWordTemp != (emInt16)0xc1e3 )
		{
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xc7a7 );//ǧ
			nIndexOfBuf += 2;
			SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );		
			ClearBuffer( g_hTTS->m_ShuZiBuffer, 10);	
			nIndexOfBuf = 0;
		}
	case 3: //ʣ��λ��
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );
		//������ת���ɡ������ж�
		if( nWordTemp == (emInt16)0xb6fe && bIsHaveLiang == emFalse)		//���ڡ�����
		{					
			nWordTemp = (emInt16)0xc1bd;		//ת���ɡ�����
		}
		bIsHaveLiang = emTrue;
		nIndex += 2;
		if( nSumOfRead > 3 )
		{
			nWord =  GetWord((emPByte)g_hTTS->m_ShuZiBuffer, nIndexOfBuf-2) ;
			if ( nWordTemp == (emInt16)0xc1e3 && nWord == (emInt16)0xc1e3 )
			{
			}
			else
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
				nIndexOfBuf += 2;
				if ( nWordTemp != (emInt16)0xc1e3)
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xb0d9 );//��
					nIndexOfBuf += 2;
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );		
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 10);	
					nIndexOfBuf = 0;
				}
			}
		}
		else 
		{
			PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
			nIndexOfBuf += 2;
			if ( nWordTemp != (emInt16)0xc1e3)
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xb0d9 );//��
				nIndexOfBuf += 2;
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );		
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 10);	
				nIndexOfBuf = 0;
			}
		}
	case 2: //ʣ��λ��
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );		
		nIndex += 2;
		if( nSumOfRead > 2 )
		{
			nWord = GetWord((emPByte)g_hTTS->m_ShuZiBuffer, nIndexOfBuf-2) ;
			if ( nWordTemp == (emInt16)0xc1e3 && nWord == (emInt16)0xc1e3 )
			{
			}
			else
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
				nIndexOfBuf += 2;
				if ( nWordTemp != (emInt16)0xc1e3)
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xcaae );//ʮ
					nIndexOfBuf += 2;
				}
			}
		}
		else 
		{
			if( nWordTemp == (emInt16)0xd2bb )//��λ��,ʮλ��Ϊһʱ��һ������
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xcaae );//ʮ
				nIndexOfBuf += 2;
			}
			else 
			{   
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
				nIndexOfBuf += 2;
				if( nWordTemp != (emInt16)0xc1e3 )
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, (emInt16)0xcaae );//ʮ
					nIndexOfBuf += 2;
				}
			}
		}
	case 1://ʣһλ��
		nWordTemp = ChangeShuZiToHanZi( GetWord(pCurStr, nIndex) );
		//������ת���ɡ������ж�
		if( nWordTemp == (emInt16)0xb6fe && bIsHaveLiang == emFalse && g_hTTS->m_bIsErToLiang == emTrue && nSumOfRead==1 )		//���ڡ�����
		{	
			//����3��  ������ת���ɡ������Ĺ���
			//���ӣ� ֱ�ӻظ�1��2���ɡ�2���ˡ�2�䷿��2���ơ�2���š�2�ѵ���2Ͱˮ����2���ˡ���2�䷿����2���ơ���2���š���2�ѵ�����2Ͱˮ��
			//ע�⣺���ֲ����ǣ���,��,��  �� ǰ�ֲ����ǣ���
			nPrevWord = GetWord(pCurStr, nIndex-2);		//ǰ��	
			nNextWord = GetWord(pCurStr, nIndex+2);		//����
			nNextNextWord = GetWord(pCurStr, nIndex+4);	//�����
			if(    CheckDataType(pCurStr, nIndex+2) == DATATYPE_SHUZI	//�����ǣ�   ����
				|| nNextWord == (emInt16)0xa3ba							//�����ǣ�   ��
				|| (    nPrevWord != (emInt16)0xb5da							//ǰ�ֲ��ǣ� ��
				     && (   nNextWord == (emInt16)0xb8f6						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xbce4						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc9c8						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xb0d1						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xb4ce						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc3d7						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc3eb						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc4ea						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc8cb						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xccec						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xccf5						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xcebb						//�����ǣ�   λ
						 || nNextWord == (emInt16)0xbdef						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xbfc3						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xbfc5						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc1a3						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xc3fb						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xd2b3						//�����ǣ�   ҳ
						 || nNextWord == (emInt16)0xd6bb						//�����ǣ�   ֻ
						 || nNextWord == (emInt16)0xd6dc						//�����ǣ�   ��
						 || nNextWord == (emInt16)0xcdb0						//�����ǣ�   Ͱ		
						 || (nNextWord == (emInt16)0xb7d6 && nNextNextWord == (emInt16)0xd6d3) 		//�����ǣ�   ����
						 || (nNextWord == (emInt16)0xb9ab && nNextNextWord == (emInt16)0xc0ef) 		//�����ǣ�   ����
						 || (nNextWord == (emInt16)0xc7a7 && nNextNextWord == (emInt16)0xbfcb) 		//�����ǣ�   ǧ��
						 || (nNextWord == (emInt16)0xc7a7 && nNextNextWord == (emInt16)0xc3d7) 		//�����ǣ�   ǧ��
						 || (nNextWord == (emInt16)0xd0a1 && nNextNextWord == (emInt16)0xcab1) 		//�����ǣ�   Сʱ
						 || (nNextWord == (emInt16)0xb9ab && nNextNextWord == (emInt16)0xbdef)))) 	//�����ǣ�   ����
						 	
			{
				nWordTemp = (emInt16)0xc1bd;		//ת���ɡ�����
			}
		}
		bIsHaveLiang = emTrue;
		nIndex += 2;
		if( nSumOfRead > 1 )
		{   
			nWord = GetWord((emPByte)g_hTTS->m_ShuZiBuffer, nIndexOfBuf-2);
			if ( nWordTemp == (emInt16)0xc1e3 ) //&& nWord == (emInt16)0xc1e3 )
			{   
				if ( nWord == (emInt16)0xc1e3 )
				{
					nIndexOfBuf -= 2;
					PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, END_WORD_OF_BUFFER );
					break;
				}
				PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, END_WORD_OF_BUFFER );
				break;
			}
		}

		PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, nWordTemp );
		nIndexOfBuf += 2;
		PutIntoWord( g_hTTS->m_ShuZiBuffer, nIndexOfBuf, END_WORD_OF_BUFFER );
		break;

	default: break;
	}

	g_hTTS->m_ControlSwitch.m_bYaoPolicy = nCopyYaoPolicy;

	g_hTTS->m_bIsErToLiang = emFalse;

	return nIndexOfBuf;
}

//****************************************************************************************************
//  ���ܣ������������ִ��ĳ��ȣ�2���ֽ�Ϊһ������
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����ִ��ĺ��ָ���nSumOfHanZi
//****************************************************************************************************
emInt16 emCall  GetHanZiSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfHanZi = 0 ;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_HANZI )
		{
			nSumOfHanZi++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_HANZI );

	return nSumOfHanZi;
}

//****************************************************************************************************
//  ���ܣ���������Ӣ�Ĵ��ĳ��ȣ�2���ֽ�Ϊһ��Ӣ��
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ�Ӣ�Ĵ���Ӣ�ĸ���nSumOfYingWen
//****************************************************************************************************
emInt16 emCall  GetYingWenSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfYingWen = 0;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_YINGWEN )
		{
			nSumOfYingWen++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_YINGWEN );

	return nSumOfYingWen;
}

//****************************************************************************************************
//  ���ܣ������������Ŵ��ĳ��ȣ�2���ֽ�Ϊһ������
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����Ŵ��ķ��Ÿ���nSumOfFuHao
//****************************************************************************************************
emInt16 emCall  GetFuHaoSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfFuHao = 0 ;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_FUHAO )
		{
			nSumOfFuHao++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_FUHAO );

	return nSumOfFuHao;
}

//****************************************************************************************************
//  ���ܣ������������ִ��ĳ��ȣ�2���ֽ�Ϊһ������
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex	
//	���أ����ִ������ָ���nSumOfShuZi
//****************************************************************************************************
emInt16 emCall  GetShuZiSum( emPByte pCurStr, emInt16  nCurIndex)
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfShuZi = 0;

	LOG_StackAddr(__FUNCTION__);

	do
	{
		g_hTTS->m_nDataType = CheckDataType(pCurStr, nIndex);
		if ( g_hTTS->m_nDataType == DATATYPE_SHUZI )
		{
			nSumOfShuZi++ ;
			nIndex += 2 ;
		}
		//else break;
	}while( g_hTTS->m_nDataType == DATATYPE_SHUZI );

	return nSumOfShuZi;
}

//****************************************************************************************************
//  ���ܣ����������ַ����Ƿ�����Ч��ƴ������������Ч��ƴ�����롣   ��ϵͳ
//	����������μ����º���
//	���أ� ���Ǻϸ��ƴ����ʽ������ֵΪ������ƴ�������������ĳ��ȣ������ǣ�����ֵΪ0��
//  
//****************************************************************************************************
emInt16  emCall  GetPinYinCode( 
							  emPByte  pCurStr, 		//��ǰ�����buffer����ʼλ��
							  emInt16  nCurIndex,  	    //��ǰ����buffer�ĵڼ����ַ�
							  emBool   bHaveOnlyPinYin, //[=cha1]: ��ֵfalse�� cha1����ֵtrue
							  emByte   *pnShengMuNo,	    //ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
							  emByte   *pnYunMuNo,		//ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
							  emByte   *pnShengDiaoNo   //ָ�봫�Σ���Чƴ�����������룬��ʼֵΪ0
							  )
{
	emInt16 nIndex        = nCurIndex;
	emInt16 nlenOfYingWen = 0;
	emInt16 nlenOfShuZi   = 0;
	emInt16 nlen          = 0;
	emInt16 nWordTempofSZ   = 0;
	emInt16 nWordTempofYW   = 0;
	emInt16 nPianYiHangShu  = 0;
	emInt16 nQiShiHangShu   = 0;
	emInt16 nSumOfHangShu   = 0;
	emInt16 i = 0,j = 0;
	emInt16 nWord           = 0;
	emByte cHighByte = 0;                       //������ʱ��ű���еĸ��ֽ�
	emByte cLowByte = 0;                        //������ʱ��ű���еĵ��ֽ�

	LOG_StackAddr(__FUNCTION__);


	if ( bHaveOnlyPinYin == emFalse) // ���磺[=cha1]
	{
		nIndex += 4;
	}

	nlenOfYingWen = GetStrSum( pCurStr, nIndex, DATATYPE_YINGWEN );
	if ( nlenOfYingWen == 0 || nlenOfYingWen > 6 )
	{    
		return nlen;
	}

	nlenOfShuZi   = GetStrSum( pCurStr, nIndex + nlenOfYingWen*2 , DATATYPE_SHUZI );
	nWordTempofSZ = ChangeShuZiToInt( pCurStr, nIndex + nlenOfYingWen*2 );
	if ( nlenOfShuZi != 1 || (nlenOfShuZi == 1 && nWordTempofSZ == 0) || nWordTempofSZ > 5 )
	{    
		return nlen;
	}

	if ( bHaveOnlyPinYin == emFalse) // [=cha1]
	{   //�ж����ִ�����ַ��Ƿ�Ϊ��]��
		if ( GetWord(pCurStr, nIndex + nlenOfYingWen*2 + nlenOfShuZi*2 ) != (emInt16)0xa3dd ) 
		{
			return nlen;
		}
	}

	nWordTempofYW = GetWord(pCurStr, nIndex );
	nPianYiHangShu = ( nWordTempofYW - (emInt16)0xa3e1 ) * 6 + nlenOfYingWen;

	//���ݡ�ƫ�����������ҡ�ƴ����ѯ���������ҵ�����ʼ�������͡�����������
	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_PinYinInquireIndex + (nPianYiHangShu-1)* 4 ,0);
	fFrontRead(&nQiShiHangShu,2,1,g_hTTS->fResFrontMain);	//��ʼ����	
	fFrontRead(&nSumOfHangShu,2,1,g_hTTS->fResFrontMain);	//��������

	//�ٸ��ݡ���ʼ�������͡������������ӡ�ƴ����ѯ��������ƴ��
	nlen = nIndex ;

	for ( i = 0 ; i < nSumOfHangShu ; i++ )
	{ 
		nIndex = nlen;
		fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_PinYinInquire + (nQiShiHangShu-1 + i) * 8 ,0);
		do 
		{   
			j = 0;

			fFrontRead(&cLowByte,1,1,g_hTTS->fResFrontMain);
			fFrontRead(&cHighByte,1,1,g_hTTS->fResFrontMain);

			while( j < 2 ) 
		 {
			 nWordTempofYW = GetWord(pCurStr, nIndex ); 
			 if ( nWordTempofYW - cHighByte  == (emInt16)0xa380 )
			 {
				 nIndex += 2;
				 cHighByte = cLowByte;
				 j++;
				 continue;
			 }
			 break;
		 }
		} while ( j == 2 && (nIndex - nlen) <= 2*nlenOfYingWen );


		if ( (nIndex - nlen) == 2*nlenOfYingWen )//�ҵ���ƥ���ƴ���ˣ�����ĸ����ĸ��������Ŵ��ݸ�����
		{  
			fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_PinYinInquire + (nQiShiHangShu-1 + i) * 8 + 6,0);


			fFrontRead(pnYunMuNo,1,1,g_hTTS->fResFrontMain);
			fFrontRead(pnShengMuNo,1,1,g_hTTS->fResFrontMain);


			*pnShengDiaoNo = (emByte)nWordTempofSZ;
			nlen = 0;
			if ( bHaveOnlyPinYin == emFalse )
		 {
			 nlen =  2*2 + 2 ;
		 }
			nlen += nlenOfYingWen*2 + nlenOfShuZi*2 ;
			return nlen;
		}

	}

	//û��������ƴ������ƴ����cha1��������0ֵ
	if ( bHaveOnlyPinYin == emTrue )
	{
		nlen = 0;
		return nlen;
	}
	//��bHaveOnlyPinYin=false�� ����ƴ����[=cha1] ��������ֵΪ������ƴ�������������ĳ���
	nlen = 2*2 + nlenOfYingWen*2 + nlenOfShuZi*2 + 2 ;
	return nlen;
}

//****************************************************************************************************
//  ���ܣ���Ӣ�ĵ�λ�� Table_Char1JustAfterNumToHanZi��Table_Char2JustAfterNumToHanZi
//        �ж����ִ����Ӣ���Ƿ��ڴ˱���
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex,��ǰΪӢ�Ĵ�����ʼ��ַ
//	����ֵ����Ӣ�ķ��ŵ�λ�ڱ���ʱ����ֵΪӢ�ķ��ŵ�λ�ĸ���,���򷵻�0��Ӣ�ķ��ŵ�λ
//  �ı������������ֵ��Ϊ0ʱ����ı���ȫ�ֱ���g_hTTS->m_ShuZiBuffer�����ݣ����򲻸ı�
//****************************************************************************************************
emInt16  emCall  CheckDanWeiTable( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nSumOfYWDW = 0 ;
	emInt16 nWordOne = 0 ;
	emInt16 nWordTwo = 0 ;
	emInt16 nWordThree = 0 ;
	emInt16 nWordTemp = 0 ;
	emInt16 i ;
	emInt16 StartLine;
	emInt16	LineCount;
	emByte cMin,cMax,cMiddle;
	emBool bFlag = emFalse;

	LOG_StackAddr(__FUNCTION__);

	cMin = 0;


	nSumOfYWDW = GetStrSum( pCurStr, nCurIndex, DATATYPE_YINGWEN );
	if ( nSumOfYWDW > 3 )
	{
		nSumOfYWDW = 0;
		return  nSumOfYWDW;
	}

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_EnglishLiangCiToHanZi+ 4*(nSumOfYWDW - 1), 0);
	fFrontRead(&StartLine,2,1,g_hTTS->fResFrontMain);
	fFrontRead(&LineCount,2,1,g_hTTS->fResFrontMain);

	cMax = LineCount -1;

	if ( cMax >= 2 ) //������Χ��������ʱ
	{
		cMiddle = ( cMin + cMax) / 2;
		do
		{		
			fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_EnglishLiangCiToHanZi + ( StartLine - 1 + cMiddle) * 14, 0 );
			fFrontRead(&nWordOne,2,1,g_hTTS->fResFrontMain);
			switch ( nSumOfYWDW )
			{
			case 3:
				fFrontRead(&nWordTwo,2,1,g_hTTS->fResFrontMain);
				fFrontRead(&nWordThree,2,1,g_hTTS->fResFrontMain);
				break;
			case 2:
				fFrontRead(&nWordTwo,2,1,g_hTTS->fResFrontMain);
				break;
			}

			if( nWordOne == GetWord(pCurStr, nCurIndex) )
			{	
				if ( nWordTwo == 0)
				{//���ҵ�
					break;
					//return cMiddle;
				}

				if ( nWordTwo == GetWord(pCurStr, nCurIndex+ 2) )
				{
					if ( nWordThree == 0 )
					{//���ҵ�

						break;
						//return cMiddle;
					}
					if ( nWordThree == GetWord(pCurStr, nCurIndex+ 4) )
					{//���ҵ�
						break;
						//return cMiddle; 
					}
					//δ�ҵ�
					nSumOfYWDW = 0;
					return nSumOfYWDW;
				}

				if ( nWordTwo < GetWord(pCurStr, nCurIndex+ 2) )
				{ //���ҵĵڶ���Ӣ�ĵı�����ڱ��еڶ���Ӣ�ĵı���
					cMin = cMiddle;
				}

				else
				{//���ҵĵڶ���Ӣ�ĵı���С�ڱ��еڶ���Ӣ�ĵı���
					cMax = cMiddle;
				}
			}

			if( nWordOne < GetWord(pCurStr, nCurIndex) )
			{   //���ҵĵ�һ��Ӣ�ĵı�����ڱ��е�һ��Ӣ�ĵı���
				cMin = cMiddle;
			}

			if( nWordOne > GetWord(pCurStr, nCurIndex) )
			{   //���ҵĵ�һ��Ӣ�ĵı���С�ڱ��е�һ��Ӣ�ĵı���
				cMax = cMiddle;
			}

			cMiddle = ( cMin + cMax) / 2;
		}while( ( cMiddle != cMin ) && ( cMiddle !=cMax ) );
	}	

	for( i = 0; i < 2 ; i++ )
	{
		fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_EnglishLiangCiToHanZi + ( StartLine - 1 + cMiddle) * 14, 0 );
		fFrontRead(&nWordOne,2,1,g_hTTS->fResFrontMain);
		fFrontRead(&nWordTwo,2,1,g_hTTS->fResFrontMain);
		fFrontRead(&nWordThree,2,1,g_hTTS->fResFrontMain);

		if ( (nWordOne == GetWord(pCurStr, nCurIndex)) && (nWordTwo==0 || (nWordTwo==GetWord(pCurStr, nCurIndex+2))) && (nWordThree==0 || (nWordThree==GetWord(pCurStr, nCurIndex+4))) )
		{   //�ҵ��󣬽�Ӣ�Ķ�Ӧ�ĺ�����Ϣд��g_hTTS->m_ShuZiBuffer
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //����1�ı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //����2�ı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //����3�ı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //����4�ı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, nWordTemp );
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 8, END_WORD_OF_BUFFER  ); //ShuZiBuffer�Ľ�β��
			return  nSumOfYWDW;
		}

		cMiddle =  cMax ;
	}

	nSumOfYWDW = 0;
	return  nSumOfYWDW;
} 

//****************************************************************************************************
//  ���ܣ��顶�������ʱ� 
//        �жϺ����Ƿ��ڴ˱���
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex,��ǰΪ���ִ�����ʼ��ַ
//	����ֵ�����������ʵĵ�һ�����ڱ���ʱ����ֵΪ1,���򷵻�0
//****************************************************************************************************
emInt16  emCall  CheckHanZiLiangCiTable( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nWordOne = 0 ;
	emInt16 nWordTwo = 0 ;
	emInt16 nWordTemp;
	emInt16 i = 0 ;
	emInt16 bFlag =  0;

	LOG_StackAddr(__FUNCTION__);

	nWordOne = GetWord(pCurStr, nCurIndex);
	nWordTwo = GetWord(pCurStr, nCurIndex + 2);

	for ( i = 0; i < 80 ; i ++ )
	{   
		fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_HanZiLiangCi + i*4, 0);
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);
		if ( nWordTemp == nWordOne )
		{
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);			
			if (nWordTemp == nWordTwo )
			{
					bFlag = 1;
					return bFlag;
			}
			if ( nWordTemp == (emInt16) 0 )
			{
				bFlag = 1;
				return bFlag;
			}
		}
	}

	return  bFlag;
} 

//****************************************************************************************************
//  ���ܣ��� �������������ֶ��ɺ����,���� �������ִ������ִ�֮ǰ��N�������г��֡����������ִʽ��������ִ�֮ǰ���� �������ִʽ��������ִ�֮��
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex,  
// nTypeTable������Ҫ��ѯ���������е���һ���,
//			nTypeTable=1��	��ѯ�������ִ������ִ�֮ǰ��N�������г��֡���				--���ִ���������
//			nTypeTable=2��	��ѯ�������ִʽ��������ִ�֮ǰ����							--���ִ���������
//			nTypeTable=3��	��ѯ�������ִʽ��������ִ�֮�󡷱�							--���ִ���������
//			nTypeTable=4��	��ѯ�������ִ������ִ�֮ǰ��N�������г���--���봦��Ԫ��		--���ִ���������
//	���أ�    ���Ҫ���ҵĺ��ִ��ڱ��У��򷵻�emTrue,���򷵻�emFalse
//****************************************************************************************************
emInt16  emCall  CheckHanZiLetDigitToPhoneTable( emPByte pCurStr, emInt16  nCurIndex ,emByte nTypeTable )
{   
	emInt16 nWordOne = 0 ;
	emInt16 nWordTwo = 0 ;
	emInt16 nWordTempOne = 0 ;
	emInt16 nWordTempTwo = 0 ;
	emInt16 i,j=0 ;
	emInt16 nStartLine ;//��ʼ����
	emInt16 nSumOfLine ;//��������

	LOG_StackAddr(__FUNCTION__);

	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_HanZiLetDigitToPhone + (nTypeTable - 1) * 4 ,0);
	fFrontRead(&nStartLine,2,1,g_hTTS->fResFrontMain);
	fFrontRead(&nSumOfLine,2,1,g_hTTS->fResFrontMain);


	nWordOne = GetWord( pCurStr, nCurIndex );
	nWordTwo = GetWord( pCurStr, nCurIndex + 2 );
	for ( i = 0; i < nSumOfLine; i++ )  //30Ϊ��Table_HanZiJustAfterNumOpenPhone������
	{
		fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_HanZiLetDigitToPhone + (nStartLine - 1)*4 + i * 4 ,0);
		fFrontRead(&nWordTempOne,2,1,g_hTTS->fResFrontMain);
		if ( nWordTempOne == nWordOne )  
		{
			fFrontRead(&nWordTempTwo,2,1,g_hTTS->fResFrontMain);			
			if ( nWordTempTwo == nWordTwo || nWordTempTwo==0x0000)	//�ڱ����ҵ������������
			{    
				if ( (nWordTempOne == (emInt16)0xc0b4) && (nWordTempTwo == (emInt16)0xd7d4) )
				{//�ǹؼ��֡����ԡ�
					for ( j = 0;  (GetWord(pCurStr, nCurIndex+2*j)!=((emInt16)0xa3ac)) && (CheckDataType(pCurStr,nCurIndex+2*j) != (emInt16)DATATYPE_SHUZI) && j<=7; j++ );
					if ( GetWord(pCurStr, nCurIndex+2*j)==((emInt16)0xa3ac) )//�˳�ѭ����ԭ������Ϊ������
					{//����Ƕ��ŵĻ�������ֵΪ�棬�����봦��
						return emTrue ;
					}
					if ( CheckDataType(pCurStr,nCurIndex+2*j) == (emInt16)DATATYPE_SHUZI )//�˳�ѭ����ԭ������Ϊû�ж��ŵ������������������
					{//����ǵĻ����ж����ִ��ĸ���
						if ( GetStrSum( pCurStr, nCurIndex+2*j , DATATYPE_SHUZI ) < 7 )
						{//����ֵΪ�٣�����ֵ����
							return emFalse;
						}
					}
				}
				return emTrue ;
			}
		}
	}

	return emFalse;
}



//�����βͣ��Ч��		
void emCall GenPauseNoOutSil()	
{
	emBool isDanZi = emFalse;

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	emBool b1;
	b1 = g_hTTS->bTextTailWrap;
	g_hTTS->bTextTailWrap = emFalse;
#endif

	LOG_StackAddr(__FUNCTION__);

	//����ͣ�ٵ�Ч����û��������ǰ�����ݻ�����������ͣ��
	g_hTTS->m_ControlSwitch.bIsPauseNoOutPutEndSil   = emTrue;	//����[p*]ͣ�ٲ��������ľ�βsil

	if( emStrLenA(g_pText) == 2 )		
		isDanZi = emTrue;


	FirstHandleHanZiBuff(emTrue);	//�����ֻ������������ȴ���

	//hyl 2012-04-10
	//�Ȳ���100ms��������ֹ������  ���磺����: �������������ˣ����ˣ���23�����Ի���29���ڳ�����Լ7Сʱ�Ļ�����̺��Ļ
	//ģ���βsil��100ms��������������ٺŵ�350ms��ͬ���ظ��ֻҪ�ٺŵĲ�����������ͣ�١���Ҳ���Ῠ
	//���磺  �ͷ�̩��--�¹������ͷ�̩�����¹�����
	PlayMuteDelay(100);			

	//���ǵ��֣���ͣ������200ms������SYN6658�Ῠ    hyl 2012-03-31
	//���磺�������Ϊ��39.40����1����ǿ�ҵĴ�ҵԸ�����뷨��
	if( isDanZi == emTrue)		
		PlayMuteDelay(200);


#if EM_SYN_SEN_HEAD_SIL
	g_hTTS->m_CurMuteMs = 100 ;							//���ܵ��ڣ�����оƬ�Ῠ��  ���磺�ͷ�̩��--�¹���������֮��	
#else
	g_hTTS->m_CurMuteMs = 150 ;							//���ܵ��ڣ�����оƬ�Ῠ��  ���磺�ͷ�̩��--�¹���������֮��	
#endif

	
	






	g_hTTS->m_bIsMuteNeedResample = emTrue;

		

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	g_hTTS->bTextTailWrap = b1;
#endif

}


//****************************************************************************************************
//  ���ܣ��Ƿ�ٷֺ�����
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ��ٷֺ�����
//			���أ�0���ǰٷֺ�ǧ�ֺ�����
//			���أ�1���ޡ�-���ٷֺ�����	��50%
//			���أ�2���С�-���ٷֺ�����	��30-50%
//			���أ�3���ޡ�-��ǧ�ֺ�����	��50��
//			���أ�4���С�-��ǧ�ֺ�����	��30-50��
//****************************************************************************************************
emInt16 emCall CheckIsTypeBaiFenHao( emPByte pCurStr, emInt16  nCurIndex )
{
	emBool bIsHaveZhi = emFalse;
	emInt16 nSum1;
	emInt16 nWord;
	emInt16 nIndex = nCurIndex;

	LOG_StackAddr(__FUNCTION__);
	
	nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
	nIndex += nSum1*2;
	nWord = GetWord( pCurStr,nIndex);
	if( nWord == (emInt16)0xa3ae)										//���ţ� ��.��
	{
		nIndex +=  2;
		nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
		nIndex += nSum1*2;
		nWord = GetWord( pCurStr,nIndex);
	}
	if( nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa )          //���ţ���-�� �� ������
	{
		bIsHaveZhi = emTrue;
		nIndex +=  2;
		nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
		nIndex += nSum1*2 ;
		nWord = GetWord( pCurStr,nIndex);
		if( nWord == (emInt16)0xa3ae)										//���ţ� ��.��
		{
			nIndex +=  2;
			nSum1 = GetStrSum( pCurStr,nIndex,DATATYPE_SHUZI);
			nIndex += nSum1*2 ;
			nWord = GetWord( pCurStr,nIndex);
		}
	}
	if ( (nWord == (emInt16) 0xa3a5) || (nWord == (emInt16) 0xa987)  )//���ֺ��аٷֺ�%,��
	{
		if(  bIsHaveZhi == emTrue)
			return  2;
		else
			return  1;
	}
	if ( (nWord == (emInt16) 0xa1eb) )//���ֺ���ǧ�ֺš�
	{
		if(  bIsHaveZhi == emTrue)
			return  4;
		else
			return  3;
	}

	return 0;
}
