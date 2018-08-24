#include "emPCH.h"

#include "Front_ProcYingWen.h"


emInt16 emCall ProcessYingWen( emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nSumOfYingWen = 0 ,  nWord,nWord2;
	emByte  strNeedCmp[11]="";
	emInt16 nErrorID;
	emInt16 nSumOfShuZi = 0;

	emInt16 nTempOfSum,nSoundShuZi;

	emBool    bHaveOnlyPinYin;
	emByte   *pnShengMuNo;	    //ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
	emByte   *pnYunMuNo;		//ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
	emByte   *pnShengDiaoNo;    //ָ�봫�Σ���Чƴ�����������룬��ʼֵΪ0
	emInt16   nValueOfHandelPinYinReturn;
	emByte    PinYin[3] ;

	emByte    cHighByte = 0;                       //������ʱ��ű���еĸ��ֽ�
	emByte    cLowByte = 0;                        //������ʱ��ű���еĵ��ֽ�
	emInt16   nStartOfYinSe = 0;                  //ĳһƴ������Ӧ����ɫ��ʼ�к�
	emByte    cNumOfYinSe = 0;                    //ĳһƴ������Ӧ����ɫ����
	emInt16   nYinSeNum = 0;                      //ĳһƴ������Ӧ����ɫ���
	emInt16    nTemp; 
	emInt16 nPinYin;
	emInt8 nPromptLen;





	LOG_StackAddr(__FUNCTION__);

	pnShengMuNo = PinYin;
	pnYunMuNo = PinYin + 1;
	pnShengDiaoNo = PinYin + 2;



	nSumOfYingWen = GetYingWenSum( strDataBuf ,  nCurIndexOfBuf );


	//��Ϊʶ��ƴ���Ŀ��Ʊ��Ϊʶ��[i1]
	if ( g_hTTS->m_ControlSwitch.m_bShiBiePinYinPolicy == emTTS_USE_PINYIN_OPEN )
	{//������Ӣ�Ĵ��ĳ���С����6��  
      if ( nSumOfYingWen > 0 && nSumOfYingWen < 7 )
      {
        nTempOfSum = GetStrSum( strDataBuf,  nCurIndexOfBuf + 2*nSumOfYingWen , DATATYPE_SHUZI );
		if ( nTempOfSum = 1 )
		{//�Һ�����������ִ��ĳ��ȵ���1,��Ӣ�Ĵ�+���ִ������ݴ�����ʶ��ƴ������--����ͼ����������
			bHaveOnlyPinYin = emTrue;
			nValueOfHandelPinYinReturn=GetPinYinCode(strDataBuf,nCurIndexOfBuf,bHaveOnlyPinYin,pnShengMuNo,pnYunMuNo,pnShengDiaoNo );
			if ( nValueOfHandelPinYinReturn != 0 )
			{  //���Ǻϸ��ƴ����ʽ������ݴ˺������ص���ĸ��ĸ������Ϣ�����ַ���
			
				//��ĸ��ĸ�������Ϊ0ʱ����ʾ�Ǻ��֣���Ȼ�ں����������ߴ˺�����ƴ������ʱ��������
				if ( PinYin[0]==0 && PinYin[1]==0 && PinYin[2]==0  )
				{
					;		//��������ֱ�Ӷ���			
					
				}
				else	//�����ַ���
				{
					nPinYin = 0x8000 + (*pnYunMuNo)*256 + (*pnShengMuNo)*8 + (*pnShengDiaoNo); //��ֵΪǿ��ƴ��
					WriteToHanZiLink( (const emPByte)"�A", 0 ,POS_CODE_g, nPinYin );     //���ţ�ǿ��ƴ������   ����ƴ��ǿ�Ƹ�ֵΪ���֡��A�����������뵽���ֺϳ���ȥ
				}
                
				nCurIndexOfBuf += nValueOfHandelPinYinReturn;
				return nCurIndexOfBuf;		
			}
            //�����Ǻϸ�ƴ��������ͨӢ�ķ���
		}
      }
	}

    if ( nCurIndexOfBuf > 1 )
    {  //����(kg),���γߴ�(mm):400��280��860 (min2) �� ��G nter����
		nWord  = GetWord(strDataBuf,nCurIndexOfBuf - 2);
		nWord2 = GetWord(strDataBuf,nCurIndexOfBuf + nSumOfYingWen*2);		//hyl 2012-04-13
		if (   (nWord == (emInt16)0xa3a8 && nWord2 == (emInt16)0xA3A9 )		//�Ƿ��ţ�  ��	��
			|| (nWord == (emInt16)0xa3db && nWord2 == (emInt16)0xA3DD )		//�Ƿ��ţ�  ��	��
			|| (nWord == (emInt16)0xa3fb && nWord2 == (emInt16)0xA3FD )		//�Ƿ��ţ�  ��	��
			|| (nWord == (emInt16)0xa1be && nWord2 == (emInt16)0xA1BF ) )	//�Ƿ��ţ�  ��	��
		{
		  nTempOfSum = CheckDanWeiTable(strDataBuf, nCurIndexOfBuf);
          if ( nTempOfSum != 0 )//Ӣ�Ĵ���λ
		  {
			  SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ���ĸ��λ
			  ClearBuffer( g_hTTS->m_ShuZiBuffer,  11 );			//������ֻ����д洢������
			  nCurIndexOfBuf +=  nTempOfSum * 2 ;
			  return nCurIndexOfBuf;
		  }
		}
    }


	//���ж��Ƿ��������ͣ� 40.3��C   �� ����C������
	if( nSumOfYingWen == 1 )
	{
		nWord = GetWord(strDataBuf,nCurIndexOfBuf );
		if(nWord == (emInt16)0xa3e3 )
		{
			nWord = GetWord(strDataBuf,nCurIndexOfBuf-2 );
			if(nWord == (emInt16)0xa1e3)   
			{
				nCurIndexOfBuf +=  2 ;
				return nCurIndexOfBuf;
			}
		}
	}





	//���ж��Ƿ��������ͣ� qq �� ip
	if( nSumOfYingWen == 2 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf, nCurIndexOfBuf,  nSumOfYingWen*2
			, sizeof(strNeedCmp) ) ;
		if( nErrorID == emTTS_ERR_FAILED )
		{
			//����ͨӢ�ķ���
			nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf,  nCurIndexOfBuf, nSumOfYingWen );
			return nCurIndexOfBuf;
		}
		else
		{
			//���ǡ����qq
			if( emMemCmp("���",strNeedCmp,4   ) == 0 || emMemCmp("���" ,strNeedCmp,4 ) == 0)
			{
				g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
				//qq����ͨӢ�ķ���
				nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );

				return nCurIndexOfBuf;

			}


		}
	}	

	//���ж��ǵ�·����G������Sʡ����X�ص���Y�����Zר�ù�·��Q��·�����3λ����
	//������䣺��··�߱���������Ϊ�ǣ��������ǣ��������ǣ��������ǣ��������ǣ��������ǣ�������
	//			ʡ��Ϊ�ӣ��������ӣ��������ӣ��������ӣ��������ӣ��������ӣ�������
	//			�ء��硢ר�ù�·��������·Ϊ�أ��٣��ڣ��ѣ��������أ��٣��ڣ��ѣ�������
	//��ע�⣺���ݻ��г���    hyl  2012-03-30
	//			D121���г���K121���г���L121���г���N121���г���T121���г���X121���г���Y121���г���Z121���г���
	if( nSumOfYingWen == 1 )
	{

		nTempOfSum = GetStrSum( strDataBuf ,  nCurIndexOfBuf+2,  DATATYPE_SHUZI);
		if( nTempOfSum == 3)
		{
			nWord = GetWord(strDataBuf,nCurIndexOfBuf);
			if(    nWord == (emInt16)0xa3e7 || nWord == (emInt16)0xa3f3 )		//��ĸ�� G��  S
				//|| nWord == (emInt16)0xa3f8 || nWord == (emInt16)0xa3f9
				//|| nWord == (emInt16)0xa3fa || nWord == (emInt16)0xa3f1 )
			{
				nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
				g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
				ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+2);
				g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;		

				if( nWord == (emInt16)0xa3e7 )	//��110
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb9fa ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//����			
				}
				if( nWord == (emInt16)0xa3f3 )	//��110
				{
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcaa1 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//ʡ��				
				}

				//����Ϊ�˼��ݻ��г�������Ҫ��   hyl  2012-03-30
				//if( nWord == (emInt16)0xa3f8 )	//��110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcfd8 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//�ص�					
				//}
				//if( nWord == (emInt16)0xa3f9 )	//��110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcfe7 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb5c0 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//���				
				//}
				//if( nWord == (emInt16)0xa3fa )	//��110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd7a8 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xd3c3 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, 0xb9ab ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,6, 0xc2b7 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,8, END_WORD_OF_BUFFER );		//ר�ù�·				
				//}
				//if( nWord == (emInt16)0xa3f1 )	//��110
				//{
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb9ab ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xc2b7 ); 
				//	PutIntoWord( g_hTTS->m_ShuZiBuffer,4, END_WORD_OF_BUFFER );		//��·			
				//}

				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//����
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 11);

				nCurIndexOfBuf += 8;
				return nCurIndexOfBuf;
			}
	
		}		
	}	



	//���ж��Ƿ��������ͣ� www.
	if( nSumOfYingWen == 3 )
	{
		nErrorID = CpyPartStr( strNeedCmp, strDataBuf , nCurIndexOfBuf,  nSumOfYingWen *2 
			, sizeof(strNeedCmp) ) ;
		if( nErrorID == emTTS_ERR_FAILED )
		{
			//����ͨӢ�ķ���
			nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );
			return nCurIndexOfBuf;
		}
		else
		{
			//���ǡ����www 
			if( emMemCmp("������" ,strNeedCmp,6 ) == 0 )
			{				
				nWord = GetWord( strDataBuf , nCurIndexOfBuf + 6 );
				//�ж�www�����ǲ���"."
				if( nWord == ( emInt16 )0xa3ae )
				{
					g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;

					//��������www��
					//nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );

					//����������w��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc8fd); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,   2, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ���
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf+4, 1 );

					return nCurIndexOfBuf;

				}
			}
		}
	}


	//�Ƿ���Ҫ������ʾ��
	if( g_hTTS->m_ControlSwitch.m_bUsePromptsPolicy == emTrue )
	{
		if( nSumOfYingWen >=4 && nSumOfYingWen <= 6)
		{

			nPromptLen = JudgePrompt(nSumOfYingWen, strDataBuf, nCurIndexOfBuf);		//�жϲ�������Ч��ʾ��
			if( nPromptLen > 0 )
				return (nCurIndexOfBuf+nPromptLen);
		}
	}


	//���ж��Ƿ��������ͣ����磺 ISO-8859-1������GB2312-80��
	nWord = GetWord(strDataBuf,nCurIndexOfBuf+nSumOfYingWen*2 );
	if(nWord == (emInt16)0xa3ad )				//��ĸ�����Ƿ��ţ�  -
	{
		if( DATATYPE_SHUZI ==CheckDataType(strDataBuf,nCurIndexOfBuf+nSumOfYingWen*2 +2)) //����"-"��������
		{
			g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
			
			//����ͨӢ�ķ���
			nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );
			return nCurIndexOfBuf;

		}
	}



	//�����������κ�һ�����������ͨӢ�ķ���

	nCurIndexOfBuf  = PlayYingWenVoice( strDataBuf ,  nCurIndexOfBuf, nSumOfYingWen );  //������Ӣ�ĺ�ָ��ָ��Ӣ�Ĵ�֮����ַ�
	return nCurIndexOfBuf;
}

////***************************************************************************************************************
////���ܣ�Ӣ�ĺ������3λ���������ֶ��ɺ�����1�����ۣ�������1���ո�*/)������ͨӢ�ķ���
////�����������ǰӢ�Ĵ�emPByte pCurStr�����λ����emInt16  nCurIndex��Ӣ�Ĵ����� nLen
////���أ��޷���ֵ
////***************************************************************************************************************
//emInt16 emCall PlayYingWenVoice( emPByte pCurStr, emInt16  nCurIndex , emInt16 nLen )
//{
//
//	emInt16   nShuZiLen;
//
//	emInt16 countUsal;
//
//	LOG_StackAddr(__FUNCTION__);
//
//	//nLen = GetYingWenSum( pCurStr,  nCurIndex );
//	//nWord = GetWord( pCurStr, nCurIndex + 2 * nLen );
//
//	//if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9 )  //Ӣ�ĺ��һλ��0��9������
//	//{
//	//	nWord = GetWord( pCurStr, nCurIndex + 2 * ( nLen + 1 ) );
//	//	if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9 )  //Ӣ�ĺ�ڶ�λ��0��9������
//	//	{
//	//		nWord = GetWord( pCurStr, nCurIndex + 2 * ( nLen + 2 ) );
//	//		if( nWord >= (emInt16)0xa3b0 && nWord <= (emInt16)0xa3b9 )  //Ӣ�ĺ����λ��0��9������
//	//		{
//	//			g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
//	//			g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
//	//			//nCurIndex += 2 * nLen;
//	//			//return nCurIndex;
//	//		}
//	//	}
//	//}
//
//	nLen = GetStrSum( pCurStr,  nCurIndex ,DATATYPE_YINGWEN);
//	nShuZiLen = GetStrSum( pCurStr, nCurIndex + 2 * nLen, DATATYPE_SHUZI);		//Ӣ�Ĵ���Ϊ3λ����
//	if( nShuZiLen >= 2)
//	{
//		g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
//		g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
//	}
//
//
//
//	//����ͨӢ�ķ���-- -- 2011-02-15�޸�  Ϊʵ�ֹ��ܣ�Ӣ����ĸ�����������ѵ��
//	countUsal = WriteToHanZiLink(pCurStr, nCurIndex, POS_CODE_n , 0);		//���ţ���ĸ
//
//	g_hTTS->m_ControlSwitch.bIsReadDian = emTrue;
//
//	return countUsal;
//
//
//
//}


//***************************************************************************************************************
//���ܣ����ľ��д���Ͳ�����ĸ��(������Ӣ��ϲ���)
//�����������ǰӢ�Ĵ�emPByte pCurStr�����λ����emInt16  nCurIndex��Ӣ�Ĵ����� nLen
//���أ� ����������µ����ľ��е�����λ��
//***************************************************************************************************************
emInt16 emCall PlayYingWenVoice( emPByte pCurStr, emInt16  nCurIndex , emInt16 nLen )
{
	#define MAX_CN_CALL_EN_LEN	23		//���ľ��е���Ӣ��ģ����ַ�������󳤶�   ��󵥴ʳ���	  hhh		//deinstitutionalization 22�� internationalization 20��   �������ɺ��ԣ�Supercalifragilisticexpealidoshus	33��  Antidisestablishmentarianism	28��
	emByte  pEnText[MAX_CN_CALL_EN_LEN+2];
	emInt16 nShuZiLen, nWord,nNextZiMuLen,countUsal, nEnParaLen, nReturnVal,i;
	emBool  bUseEnModel = emTrue;		//Ĭ�ϣ�ʹ��Ӣ��ģ��

	LOG_StackAddr(__FUNCTION__);
	nShuZiLen = GetStrSum( pCurStr, nCurIndex + 2 * nLen, DATATYPE_SHUZI);		
	
	if( nShuZiLen > 0 )						//�������ĸ������Ӣ�Ĵ�������֣� ���磺ϲ��GB2312�� ϲ��BIG5��ϲ��MP3��
	{
		if( nShuZiLen >= 2)
		{
			g_hTTS->m_ControlSwitch.m_bIsPhoneNum =  emTrue;
			g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
		}
		bUseEnModel = emFalse;
	}
	else									//���ܰ����ʷ���
	{
		nEnParaLen = nLen;
		memset(pEnText, 0 , MAX_CN_CALL_EN_LEN);
		nWord = GetWord( pCurStr, nCurIndex + 2 * nLen );  

		 //��ĸ������ַ����ǣ���-����&�����ո�
		if( nWord != (emInt16)0xa3ad && nWord != (emInt16)0xa3a6 && nWord != (emInt16)0xa3a0) 
		{	
			if( nLen == 1)							//�������ĸ��������1����ĸ��  ���磺ϲ��4S�� ϲ��3G��
				bUseEnModel = emFalse;
			else
			{

#if ZIMU_READ_AS_CN_MODEL							//���ľ��е�������ĸ������������������
				if( nEnParaLen > MAX_CN_CALL_EN_LEN)
					nEnParaLen = MAX_CN_CALL_EN_LEN;	
				for(i =0; i< nEnParaLen; i++)
					pEnText[i] = *(pCurStr + nCurIndex + 2 * i + 1) - 0x80;		//���ַ���ת���ɵ��ֽ�
				pEnText[i] = 0;
				pEnText[i+1] = 0;
				nReturnVal = 0;			//sqb  Ŀǰֻ�����ĺϳ�
				//nReturnVal = EngToLabAndSynth(pEnText,0,emTrue,emTrue,emFalse,emFalse);	//Ӣ�ĵ��ı������ͺϳɣ����һ������ΪemFalse���ڲ�����ϳ����������ı�������
				if(nReturnVal == 0)					//�������ĸ��������Ӣ��ģ��ʶ��Lab��Ϊ�㣩
					bUseEnModel = emFalse;			//���磺������PK��  
				else
					bUseEnModel = emTrue;			//���磺��holdס����  
#endif
			}
		}
		else				//�����ʷ�������ĸ������ַ��ǣ���-����&�����ո񡱣�
		{
			if( nEnParaLen > MAX_CN_CALL_EN_LEN)
				nEnParaLen = MAX_CN_CALL_EN_LEN;
			else
			{
				nNextZiMuLen = GetStrSum( pCurStr, nCurIndex + 2 * nLen + 2, DATATYPE_YINGWEN);				//ÿ����ദ��2������
				if( (nEnParaLen + nNextZiMuLen + 1) < MAX_CN_CALL_EN_LEN )
					nEnParaLen += nNextZiMuLen+1;
			}

			//����-����&�����ո�ǰ��ĵ�����Ϊһ������   ���磺ϲ��HR&Adm.ϲ��hi-fi.ϲ��CD-ROM������ʹ�õ���Windows Mobileƽ̨��
			for(i =0; i< nEnParaLen; i++)
				pEnText[i] = *(pCurStr + nCurIndex + 2 * i + 1) - 0x80;		//���ַ���ת���ɵ��ֽ�
			pEnText[i] = 0;
			pEnText[i+1] = 0;
		}			
	}

	if( bUseEnModel == emFalse)  //�����������⣬�������ĸ����
	{
		countUsal = WriteToHanZiLink(pCurStr, nCurIndex, POS_CODE_n , 0);		//���ţ���ĸ
		g_hTTS->m_ControlSwitch.bIsReadDian = emTrue;
		return countUsal;
	}
	//else						//����Ӣ�����⣬�����ʷ���
	//{
	//	if( g_hTTS->m_HaveEnWordInCnType == 0)
	//		g_hTTS->m_HaveEnWordInCnType = 1;
	//	FirstHandleHanZiBuff(emFalse);
	//	EngToLabAndSynth(pEnText,0,emFalse,emFalse,emTrue,emTrue);	//Ӣ�ĵ��ı������ͺϳ�
	//	return (nCurIndex + 2 * nEnParaLen);
	//}	
}









