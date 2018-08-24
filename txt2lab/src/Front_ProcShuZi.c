#include "emPCH.h"

#include "Front_ProcShuZi.h"


//****************************************************************************************************
//�������ܣ����������Ĵ����ִ���ͨ���ж���ǰ����ַ����������ִ�ת���ɺ����ҽ�����Ӧ����
//��������������ִ�����ǰ����������ַ�
//������������������ִ�������غ�����ƫ����emInt16 nCurIndexOfBuf
//****************************************************************************************************


emInt16 emCall ProcessShuZi( emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16	nDataType = 0, nDataType1;
    emInt16 nSumOfShuZi = 0;
	emInt16 nSumOfReadHaoMa = 0 ;
	emInt16 nSumOfReadShuZhi = 0 ;
	emInt16 nCurIndex = nCurIndexOfBuf ;
	emInt16 nWord = 0 ;
	emInt16 nWordTemp = 0 ,nWordTemp2 = 0 ,nWordPrev,nWordNext;
	emInt16 nSumOfReadDate = 0 ;
	emInt16 nSumOfReadTime = 0 ;
	emInt16 nSumTemp = 0 , nSumTemp1 = 0,nSumTemp2 = 0;
	emInt16 nTemp = 0, nTemp1 = 0 ;
	emInt16 nTempLong=0;
	emInt16 i,nBaiFenHaoType;

	LOG_StackAddr(__FUNCTION__);

	nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf, DATATYPE_SHUZI );
   
	if( nSumOfShuZi == 0 )
	{
		return emNull;
	}

    //��ʼ���ֵĴ���
	//first.��������Ϊ����ʱ�����ְ����뷢��
	if( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AS_NUMBER )
    {
	   nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
	   nCurIndexOfBuf += nSumOfReadHaoMa * 2;
	   //���ִ����Ƿ�Ϊ��-�������ֻ������ǣ�139-1234-5678
	   if( (GetWord( strDataBuf,nCurIndexOfBuf) == (emInt16)0xa3ad) || (GetWord( strDataBuf,nCurIndexOfBuf) == (emInt16)0xa1aa) )
	   {//start
		   if( DATATYPE_SHUZI == CheckDataType( strDataBuf, nCurIndexOfBuf + 2) ) 
		   { //start--1   
			   GenPauseNoOutSil();	//�����βͣ��Ч��		
			   nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf,nCurIndexOfBuf + 2);
			   if( (GetWord( strDataBuf,nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa3ad) ||(GetWord( strDataBuf,nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa1aa) )
			   {//start--2
				   i = nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ;
				   if( DATATYPE_SHUZI == CheckDataType( strDataBuf, i + 2) )
				   {
					   GenPauseNoOutSil();	//�����βͣ��Ч��		
					   nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, i + 2);
					   nCurIndexOfBuf =  i + 2+ nSumOfReadHaoMa*2;
					   return nCurIndexOfBuf;  //end of XXXX-XXXXXXXX-XXXXX
				   }
			   }//end--2
			   nCurIndexOfBuf =  nCurIndexOfBuf + 2 + nSumOfReadHaoMa*2 ;
			   return nCurIndexOfBuf; //end of XXXX-XXXXXXX
		   }//end--1
	   }//end
	   return nCurIndexOfBuf; //end of XXXXX
    }//end of first,goto exit
	//third:��������Ϊ�Զ��ж�(Ĭ��) �� ��������Ϊ����ֵ��
	else		//if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO  || g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AS_VALUE  )
	{  
		if ( nSumOfShuZi > 16) //3.1������ִ��ĳ��ȴ���16λ�Ļ�
		{
			if( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AS_VALUE )
			{
				//��������Ϊ����ֵ���� ������������ִ�����16λ�Ļ�����ô��ȡǰ���16λ������ֵ��������ֶ���
				nCurIndex += 32;
				for ( i = 0; i < (nSumOfShuZi-16); i++ )
				{
					PutIntoWord( strDataBuf, nCurIndex, (emInt16)DATATYPE_UNKNOWN );
					nCurIndex += 2;
				}
				nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
				nCurIndexOfBuf += nSumOfReadShuZhi * 2;
				return nCurIndexOfBuf; //end of ReadShuZhi
			}
			else
			{
				//��������Ϊ�Զ��жϣ� 3.1������ִ��ĳ��ȴ���16λ�Ļ��������ְ������
				nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
				nCurIndexOfBuf += nSumOfReadHaoMa * 2;
				return nCurIndexOfBuf; //end of XXXXXXXXXXXXXXXXXReadHaoMa
			}

		}//end of third,goto exit
		else			 //3.2���ִ�С��16λʱ��start----
	    {

			 //�жϣ��Ƿ�ٷֺ�����
			 //50%��50.4%��30-50%��30.4-50.4%��30-50.4%��30.4-50%��30%-50%��
			 //50����50.4����30-50����30.4-50.4����30-50.4����30.4-50����30��-50����
			 //50�롣50.4�롣30-50�롣30.4-50.4�롣30-50.4�롣30.4-50�롣30��-50�롣
			 nBaiFenHaoType = CheckIsTypeBaiFenHao(strDataBuf, nCurIndex);
			 if( nBaiFenHaoType>0 )
			 {
				 if( nBaiFenHaoType == 1 || nBaiFenHaoType == 2)		//�ٷֺ�
				 {
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb0d9 ); //��
				 }
				 else													//ǧ�ֺ�
				 {
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xc7a7 ); //ǧ
				 }
				 PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xb7d6 ); //��
				 PutIntoWord( g_hTTS->m_ShuZiBuffer,4, 0xd6ae ); //֮
				 PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, END_WORD_OF_BUFFER );
				 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );	//���ţ��ٷ�֮ �� ǧ��֮
				 ClearBuffer( g_hTTS->m_ShuZiBuffer, 7);

				 if( nBaiFenHaoType == 2 || nBaiFenHaoType == 4 )		// �С�-����  30%-50%
				 {
					 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2 + 2;
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //��
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,2, END_WORD_OF_BUFFER );
					 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );	//���ţ���
					 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2 + 2;
				 }
				 else													// �ޡ�-����  30%
				 {
					 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2 + 2;
				 }
				 return nCurIndexOfBuf;
			 }


             nCurIndex += nSumOfShuZi * 2; 
			 nWord = GetWord( strDataBuf,nCurIndex);


			 //�жϣ����ִ����Ƿ�ӡ��̡�
			 if ( nWord == (emInt16)0xa6cc )
			 {//���̡�֮���������Ϊ1��Ӣ�Ĵ�������ɣ���ֵ+��΢��+���ĵ�λ
				 if ( GetStrSum( strDataBuf, nCurIndex + 2, DATATYPE_YINGWEN ) == 1 )
				 {
                     if ( CheckDanWeiTable( strDataBuf, nCurIndex + 2) != 0 )
                     {
						 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//����ֵ
						 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xcea2 ); //΢
						 PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_a);	//���ţ�΢���ף�
						 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						 CheckDanWeiTable(strDataBuf, nCurIndex + 2);//��Ӣ�ĵ�λת����
						 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ���ĸ��λ
						 ClearBuffer( g_hTTS->m_ShuZiBuffer,  11 );			//������ֻ����д洢������
						 nCurIndexOfBuf += nSumOfReadShuZhi * 2 + 4;
						 return nCurIndexOfBuf; //end of ReadData
                     }
				 }
			 }

			// �жϣ����ִ����1���ַ�X�Ƿ���( -  /  \   . )�� 
			 if( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae)) 
			 {//���������жϿ�ʼ
                 nSumOfReadDate = CheckDateFormat( strDataBuf, nCurIndexOfBuf ); //���ش����˼�������
                if ( nSumOfReadDate != 0 )		//�������ͼ��ֵ���ز�Ϊ��ʱ����ж�Ϊ�Ǻϸ����������
                 {
			         	 nCurIndexOfBuf += nSumOfReadDate;
						 return nCurIndexOfBuf; //end of ReadData
                  } //end of XXXX-XX-XX or XXXX/XX/XX or XXXX\XX\XX or XXXX.XX.XX or others
				//�ж��������������ͣ���ת��С�������͡�����
			    if ( nWord == (emInt16)0xa3ae )
				 {//С������жϿ�ʼ 
					nDataType = CheckDataType( strDataBuf, nCurIndex + 2); 
						if ( nDataType == DATATYPE_SHUZI )			
					     {//С����ĺ��������� start   
							nSumTemp = GetStrSum( strDataBuf, nCurIndex + 2, DATATYPE_SHUZI );
							nWordTemp = GetWord( strDataBuf,nCurIndex + 2 + nSumTemp*2 );
							
							if ( CheckDanWeiTable(strDataBuf, nCurIndex + 2 + nSumTemp*2) != 0 )//���ֺ���Ӣ�Ĵ���λ
							{ 
								nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//����ֵ
								PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb5e3 ); //��
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);	//���ţ���
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
								nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+nSumOfReadShuZhi*2+2);//������
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
								nSumTemp = CheckDanWeiTable(strDataBuf, nCurIndex + 2 + nSumTemp*2);//��Ӣ�ĵ�λת����
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ���ĸ��λ
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  11 );			//������ֻ����д洢������
								nCurIndexOfBuf +=  nSumOfReadShuZhi*2+2+nSumOfReadHaoMa*2 + nSumTemp*2;
								return nCurIndexOfBuf; //end of ReadShuZi To С����ֵ+Ӣ�ĵ�λת��ĺ���
							}//end of XX.XXkg or XX.XXg or others

							//ip��ַ�����ͣ�  IP: 192.168.73.129
							if ( (nWordTemp == (emInt16) 0xa3ae) )//С�����������ֺ���С����
							{
								 while ( (nWordTemp == (emInt16) 0xa3ae) )
								 {																
									 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);									 
									 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb5e3 ); //��
									 PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);	//���ţ���
									 ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
									 nCurIndexOfBuf +=  2+nSumOfReadHaoMa*2;
									 nSumTemp = GetStrSum( strDataBuf, nCurIndexOfBuf, DATATYPE_SHUZI );
									 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp);
								 }

								 if ( CheckDataType( strDataBuf, nCurIndexOfBuf) == DATATYPE_SHUZI )	
								 {
									 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);									 
									 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;

								 }
								 
								 return nCurIndexOfBuf; 
							}


							 //ʣ�µ��ǣ�С�������ͣ�234.11 
							 nSumOfReadShuZhi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
							 nCurIndexOfBuf +=  nSumOfReadShuZhi*2;
							 if ( (GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa3ba)||(GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa1c3) )
							 { //С�����Ƿ��з��š�:��
								 g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen = 1;  //�����ţ��Ƿ���ɱȷֿ��ء���
							 }
							 return nCurIndexOfBuf; //end of ReadShuZi To XX.XX	


					     }//С����ĺ��������� end
				 }//С������жϽ���				
			 }//���������жϽ���


			 //�жϣ����ֺ������":",ʱ�������жϿ�ʼ
			 if ( (nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3) ) 
			 {
				 if ( g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen == 1)
				 {//��ʱ���ִ����ȶ��ɱȷָ�ʽ������ʱ�䣬����ʱ�����ͼ��,�˳����ִ��Ĵ������
					 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//���ִ�������ֵ
					 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
					 return nCurIndexOfBuf; 
				 }//end of ReadShuZi To ShuZhi,it will go to ProcFuHao,Read ":" To "��"


				 //���ٽ���7���������С��ȡ����ٽ���1���������С��ԡ�������ɱȷָ�ʽ���Ҵ򿪱ȷֿ���
				 for(i = 0;i < 7; i++)
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf-2-i*2);
					 if(    nWordTemp == (emInt16)0xb1c8   //���ڡ��ȡ� 
						 || (i == 0 && nWordTemp == (emInt16)0xd2d4 ))			//����ǰ�����ǣ����ԡ�  ���磺����ھ���Ӱ��22��21��ʤ��

					 {
						 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );//���ִ�������ֵ
						 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
						 g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen = 1;
						 return nCurIndexOfBuf; 
					 }
				 }


				 //������ȷֿ���û�򿪣����ٽ���ǰ��ĺ��ִ��в����С��ȡ��֣���ʱʱ���ʽ������Ȩ��
				 nSumOfReadTime = CheckTimeFormat( strDataBuf, nCurIndexOfBuf ); //���ش����˼������ּ��������
				 if ( nSumOfReadTime != 0 )		//�Ǻϸ��ʱ������
				 {
					 nCurIndexOfBuf += nSumOfReadTime * 2;
					 return nCurIndexOfBuf; //end of ReadShuZi To ʱ��XX:XX:XX
				 } //end of XX:XX:XX
				 //����ʱ���ʽʱ�����Ƕ��ɡ��ȡ�
				 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
				 g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen = 1;  //�������ţ��Ƿ���ɱȷֿ��ء���
				 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
				 return nCurIndexOfBuf; 
			 }
	

			//�жϣ����ִ��Ƿ�Ϊ��3G�� ����"4S"��ʽ
            if( ((nWord == (emInt16)0xa3e7) || (nWord == (emInt16) 0xa3f3)) && (GetStrSum(strDataBuf, nCurIndexOfBuf+2, DATATYPE_YINGWEN)==1) )
              {
			    nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
				if ( ((nWordTemp == (emInt16)0xa3b3) && (nWord == (emInt16)0xa3e7)) || ((nWordTemp == (emInt16)0xa3b4) && (nWord == (emInt16)0xa3f3)))//3G or 4S
				{
					nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
					nCurIndexOfBuf += nSumOfReadShuZhi*2;
					return nCurIndexOfBuf; //end of ReadShuZi  
				}
              }
		
		     //�жϣ����ִ��������Ӣ�Ĵ�,�����Ƿ�Ӣ�ĵ�λ���������ݷ��ص�ֵ��λ��ָ����ǰ������ת����
			 if( DATATYPE_YINGWEN == CheckDataType( strDataBuf, nCurIndex ) ) 
			 {   
				 nSumTemp = GetStrSum( strDataBuf, nCurIndex, DATATYPE_YINGWEN );

				 //�����Ǵ˸�ʽ�����ִ�+Ӣ�Ĵ�+���ִ�  ���磺BR50V3��BR50V��U6BC50L6L6gL2Z1K1Z3BC60L7
				 if(    DATATYPE_SHUZI != CheckDataType( strDataBuf, nCurIndex+nSumTemp*2 ) 
					 && DATATYPE_YINGWEN != CheckDataType( strDataBuf, nCurIndex -nSumOfShuZi*2-2 ))  
				 {				 
					 //���ò�ѯ���ֺ��Ӣ���Ƿ���Ӣ�ķ��ŵ�λ�����Ӻ���
					 nSumTemp = CheckDanWeiTable( strDataBuf, nCurIndex );
					 //if( nSumTemp == 1 || nSumTemp == 2 )
					 if( nSumTemp != 0 )
					  { 
						  nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
						  CheckDanWeiTable( strDataBuf, nCurIndex );
						  SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//���ţ���ĸ��λ
						  ClearBuffer( g_hTTS->m_ShuZiBuffer,  21 );			//������ֻ����д洢������
						  nCurIndexOfBuf +=  nSumOfReadShuZhi*2 + nSumTemp * 2;
						  return nCurIndexOfBuf; //end of ReadShuZi To ������ֵ+Ӣ�ĵ�λת��ĺ���
					  }//end of XXkg,XXg or others
				 }
			 }
			//�жϣ����ִ�����ַ�X�Ƿ���(�£��գ�ʱ���֣���)��
			 if( nWord == (emInt16)0xd4c2 || nWord == (emInt16)0xc8d5 || nWord == (emInt16)0xcab1 || nWord == (emInt16)0xb7d6 || nWord == (emInt16)0xc3eb)
			 {
			    nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
				if( nWordTemp == (emInt16)0xa3b0 )//���ִ�����0��ͷ,����ֵ����;����������"��0��ͷ�����ִ��������˺���"  
				{
					nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );     
					nCurIndexOfBuf +=  nSumOfReadShuZhi*2;
					return nCurIndexOfBuf; //end of ReadShuZi To ��ֵ + �µ�
				}//end of 0X�� or 0X�� or 0Xʱ or 0X�� or 0X��
			 }

			 if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )
			 {
				//�жϣ����ִ�����ַ�X�Ƿ���(�� ��  · ��)��	
				 if( nWord == (emInt16)0xc4ea || nWord == (emInt16)0xd5db || nWord == (emInt16)0xc2b7 || nWord == (emInt16)0xbac5)
				 {   
					 nSumOfReadHaoMa = 0;
					 nTemp = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
					 switch ( nWord )
					 {
					 //���ꡱ������ + ���ꡱ (��1��һ�����ܶ�����)
					 case (emInt16)0xc4ea:  
								  if(    (nSumOfShuZi==4 && nWordTemp<(emInt16)0xa3b3)		//���ꡱǰ�����ִ�����Ϊ4λ�ҵ�1λС��3
									  || (nSumOfShuZi==2 && nWordTemp==(emInt16)0xa3b0) //���ꡱǰ�����ִ���2λ����0��ͷ
									  || ( nTemp>=(emInt16)91 && nTemp<=(emInt16)99 ))	//���ꡱǰ�����ִ���Χ��91��99֮��
								   {
									 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
									 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
									 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
									 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
									 break;//end of XXXX�� or 0X�� or 91�� 99��
								   }
								  if ( ((GetWord( strDataBuf, nCurIndex + 2 + 2 )) == (emInt16)0xd4c2) || ((GetWord( strDataBuf, nCurIndex + 2 + 2 + 2)) == (emInt16)0xd4c2) )
								  {//���㡰�ꡱ֮��ĵ�2���3���ַ��ǡ��¡� XX��X�� XX��XX��
									  nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
									  g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
									  nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
									  g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
									  break;//end of XX��X�� XX��XX��
								  }
								  break;
								  
					//���ۡ������� + ���ۡ�
					 case (emInt16)0xd5db:  
									 nWordTemp = GetWord( strDataBuf,nCurIndex+2 );		//���ۡ��������
									 if( nWordTemp == (emInt16)0xbfdb )					//�ǡ��ۡ���  
									 {
										 //���磺 ÿ100Ԫ��30�ۿ�ȯ��
										 nSumOfReadHaoMa = 0;		//������
									 }
									 else
									 {
										 if( GetStrSum(strDataBuf,nCurIndexOfBuf,DATATYPE_SHUZI) == 1)		
										 {	 //1λ���֣�����ֵ��		 ���磺1����2��
											 nSumOfReadHaoMa = ReadDigitShuZhi(strDataBuf,nCurIndexOfBuf);										 
										 }
										 else
										 {	 //����1λ���֣��������  ���磺95��  15��  91��
											 nTemp1 = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
											 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;			//��1�����ɡ�һ��
									 		 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
											 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp1;							//��ԭ
										 }
									 }
									 break;//end of XX��
					//��·�������� + ��·��(ע�⣺���ֱ���Ϊ3λ������)(��1��������)			  
					 case (emInt16)0xc2b7:  if ( nSumOfShuZi >= 3 )
									{										 
										 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);

									}break;//end of XXX·
						 //���š������� + ���š�(ע�⣺���ֱ���Ϊ3λ������)(��1��������)			  
					 case (emInt16)0xbac5:  if ( nSumOfShuZi >= 3 )
											{
												nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
											}break;//end of XXX��
					 default: break;
					 }
					 if ( nSumOfReadHaoMa != 0 )
					 {
						 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
						 return nCurIndexOfBuf; //end of ReadShuZi To ���� + �� /�� / ·/��
					 }
				 }

				 //�жϣ����ִ���ĺ����Ƿ���(���ݱ�HanZiLetDigitToPhoneTable)��
				 if( DATATYPE_HANZI==(CheckDataType(strDataBuf, nCurIndex)) ) 
				 {
					 //����Ƿ��ڱ�Table_HanZiLetDigitToPhoneTable��
					 if ( CheckHanZiLetDigitToPhoneTable( strDataBuf,  nCurIndex, 3 ) == emTrue)
					 {//���ִ������봦�� (ע�⣺���ֱ���Ϊ3λ������)(��1��������)(���磺110������1105����)
						if ( nSumOfShuZi >= 3 )
						{
							nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
							nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							return nCurIndexOfBuf; //end of ReadShuZi To ���� + ���� /���� ��
						}
					 }
				 }


				 //�жϣ����ִ����3�����������Ƿ�����ˡ��ҡ�   //122�����ҡ�915�ҡ�412���ҡ�
				 if( GetWord(strDataBuf, nCurIndex) == (emInt16)0xcad2 
					 || GetWord(strDataBuf, nCurIndex+2) == (emInt16)0xcad2 
					 || GetWord(strDataBuf, nCurIndex+4) == (emInt16)0xcad2) 
				 {					 
					if ( nSumOfShuZi >= 3 )
					{
						nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );
						nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
						return nCurIndexOfBuf; //end of ReadShuZi To ���� + ���� /���� ��
					}
				 }

			 } //endof �� if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )


			 //�жϣ����ִ�����ַ�X�Ƿ���(0xa1a4)��  9��11�ֲ�Ϯ��   ����12��9�¼�	
			 if( nWord == (emInt16)0xa1a4 )
			 {   
				 if ( DATATYPE_SHUZI == CheckDataType( strDataBuf, nCurIndex + 2))
				 {
					 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
					 nCurIndexOfBuf +=  2*nSumOfReadHaoMa + 2;
					 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
					 nCurIndexOfBuf +=  2*nSumOfReadHaoMa ;
					 return nCurIndexOfBuf;
				 }
                 				
			 }

			 if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )
			 {
		 
				 //��: 1)���ֺ��Ƿ��� �� ��  �� ��2)����ǰ�Ƿ��š������ҽ������������ֺ��ǿո� ����ɺ��� ���Ҵ򿪶����뿪��   
				 //���磺(86)8471 7972����86��84717972��Ī�Ҵ�122�������绰��Ī�Ҵ�"122"�����绰����12Сʱ���򣩡���86 22��52131310��
				 nSumTemp1 = GetStrSum( strDataBuf, nCurIndexOfBuf,DATATYPE_SHUZI );
				 nWordPrev = GetWord( strDataBuf,nCurIndexOfBuf-2 );
				 nWordNext = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp1 );
				 if( nSumTemp1 > 1)		//�������ָ���>1
				 {
					 if(   ((nWordPrev == (emInt16)0xa1b0 || nWordPrev == (emInt16)0xa3a0 ) && nWordNext == (emInt16)0xa1b1) 	//���Ŷԣ��� �� �� ǰ�����ǿո�Ҳ��
						|| ((nWordPrev == (emInt16)0xa3a2 || nWordPrev == (emInt16)0xa3a0 ) && nWordNext == (emInt16)0xa3a2) 	//���Ŷԣ� "�� �� ǰ�����ǿո�Ҳ��
						|| ((nWordPrev == (emInt16)0xa3a8 || nWordPrev == (emInt16)0xa3a0 ) && nWordNext == (emInt16)0xa3a9) 	//���Ŷԣ� ()  �� ���� �� ǰ�����ǿո�Ҳ��
						|| (nWordPrev == (emInt16)0xa3a8 && CheckDataType( strDataBuf,nCurIndexOfBuf)==DATATYPE_SHUZI && nWordNext==(emInt16)0xa3a0)		//���� (8				
						)							
					 {
						 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
						 nCurIndexOfBuf +=  nSumTemp1*2;
						 if( nWordPrev != (emInt16)0xa3a8 )
							 nCurIndexOfBuf += 2;
						 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
						 GenPauseNoOutSil();	//�����βͣ��Ч��
						 return nCurIndexOfBuf; 			 
					 }
				 }


				 //�����ֺ��Ƿ��� - �������ִ�����>=3 ��������-���治�ܸ���ĸ�ͺ��֣�������ɺ��� ���Ҵ򿪶����뿪��   
				 //���磺139-1111-8888�� ��⣺30-50%���ȷ�Ϊ��12-15 ��(86-25)8471 7972�� 60G-70G��60-70g��60-70�ˡ�
				 nSumTemp1 = GetStrSum( strDataBuf, nCurIndexOfBuf,DATATYPE_SHUZI );
				 nWordNext = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp1 );
				 if(   nWordNext == (emInt16)0xa3ad	)	 //���� -
				 {

					 nSumTemp2 = GetShuZiSum( strDataBuf,nCurIndexOfBuf+2*nSumTemp1+2 );
					 nDataType1 = CheckDataType( strDataBuf,nCurIndexOfBuf+2*nSumTemp1+2 +2*nSumTemp2);
					 if( nDataType1 != DATATYPE_YINGWEN &&  nDataType1 != DATATYPE_HANZI && nSumTemp2>=3)
					 {
						 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
						 nCurIndexOfBuf +=  nSumTemp1*2+2;
						 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
						 GenPauseNoOutSil();	//�����βͣ��Ч��
						 return nCurIndexOfBuf; 
					 }				 			 
				 }			


				 //���������ִ��ĸ���Ϊ10λ��ǰ3λΪ400��800����ʶ��ɺ��룬�Ҵ򿪶����뿪�ء�
				 //���磺4008101666�� 8008100010��
				 if( nSumOfShuZi == 10 ) //����Ϊ10
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
					if( nWordTemp == (emInt16)0xa3b4 || nWordTemp == (emInt16)0xa3b8 )  //��1λ�ǡ�4����8��
					{
						nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+2 );
						if ( nWordTemp == (emInt16)0xa3b0  )  //��2λ�ǡ�0��
						{
							nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+4 );
							if ( nWordTemp == (emInt16)0xa3b0  )  //��3λ�ǡ�0��
							{
#if 1						 

								//���������벻һ����ͣ�ٶ�����  4008101666 ->  400-810-1666
								 #define FIRST_STOP_WEI	  3
								 #define SECEND_STOP_WEI  6
								 nSumOfReadHaoMa = GetStrSum(strDataBuf,nCurIndexOfBuf,DATATYPE_SHUZI);
								 *(strDataBuf + nCurIndexOfBuf + FIRST_STOP_WEI*2 + 1) += 10;
								 ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
								 GenPauseNoOutSil();	//�����βͣ��Ч��		
								 *(strDataBuf + nCurIndexOfBuf + FIRST_STOP_WEI*2 + 1) -= 10;
								 *(strDataBuf + nCurIndexOfBuf + SECEND_STOP_WEI*2 + 1) += 10;
								 ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+FIRST_STOP_WEI*2);
								 GenPauseNoOutSil();	//�����βͣ��Ч��		
								 *(strDataBuf + nCurIndexOfBuf + SECEND_STOP_WEI*2 + 1) -= 10;
								 ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf+SECEND_STOP_WEI*2);
								 GenPauseNoOutSil();	//�����βͣ��Ч��	
								 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
								 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
								 return nCurIndexOfBuf; 

#else						
								 //����������һ����ͣ�ٶ�����  4008101666 ->  40-0810-1666
								 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
								 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
								 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
								 return nCurIndexOfBuf; 
#endif
							}
						}
					}
				 }
			 
	            
				 //�жϣ��Ƿ��ԡ�0����ͷ
				 //if(((GetWord(strDataBuf,nCurIndexOfBuf)) == (emInt16)0xa3b0) && (nSumOfShuZi >= 3))
				 if(((GetWord(strDataBuf,nCurIndexOfBuf)) == (emInt16)0xa3b0) && (nSumOfShuZi >= 2))  //hyl 20111228  ���ӣ�E07
				 {
					 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
				 }


				 //�����ֺ��Ƿ��� / ���Ҷ����뿪���Ѵ򿪣�����ɺ��� ���Ҵ򿪶����뿪��  
				 //���磺086/0551/5331800��
				 nSumTemp1 = GetStrSum( strDataBuf, nCurIndexOfBuf,DATATYPE_SHUZI );
				 nWordNext = GetWord( strDataBuf,nCurIndexOfBuf+2*nSumTemp1 );
				 if(  (nWordNext == (emInt16)0xa3af  && g_hTTS->m_ControlSwitch.m_bIsPhoneNum == emTrue) )	 //���� ��
				 {
					 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
					 nCurIndexOfBuf +=  nSumTemp1*2+2;
					 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
					 GenPauseNoOutSil();	//�����βͣ��Ч��
					 return nCurIndexOfBuf; 			 
				 }			


				 //�жϣ����������ֶ��ɺ��롱ȫ�ֿ����Ƿ��		
				 if( g_hTTS->m_ControlSwitch.m_bIsPhoneNum == emTrue )
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndex );

					 //Ϊ����ȷʶ������************************
					 //			 ���磺��������¥�ͷ�������ȡ3000Ԫ����볡ȯһ�š����ͷ�3000��
					 //			 ���磺����3000��������3000Ԫ������3000��
					 //			 ���磺�ֻ�3000�����ֻ�3000Ԫ���ֻ�3000��
					 //			 ���磺����绰3000�Ρ�����绰3000���ӡ�����绰3000�롣����绰3000��
					 //			 ���磺��ѯ3000�ˡ���ѯ95588�� ��ͨ���д�������200��
					 //			 ���磺ad400��  D121���г���Z121���г� 
					 //			 ���磺����绰95588��95578������绰95588��95578������绰95588��95578��
					 //			 ���磺����120������ҵ��йع�Ա���������С���������148��רҵָ��ίԱ��ίԱ��

					 //�����ִ�����>=3,�����ִ������������ʣ�����ɺ���
					 if (    nSumOfShuZi >= 3 
						  && nWordTemp != (emInt16)0xd4aa			//Ԫ						  
						  && nWordTemp != (emInt16)0xccf5			//��
						  && nWordTemp != (emInt16)0xb8f6			//��
						  && nWordTemp != (emInt16)0xc8cb			//��
						  //&& nWordTemp != (emInt16)0xb4ce			//��		���磺D121���г���Z121���г�  hyl 2012-03-30
						  && nWordTemp != (emInt16)0xb7d6			//��
						  && nWordTemp != (emInt16)0xc3d7			//��
						  && nWordTemp != (emInt16)0xb6e0			//��
						  && nWordTemp != (emInt16)0xc3fb			//��
						  && nWordTemp != (emInt16)0xcdf2			//�� Ԫ	
						  && nWordTemp != (emInt16)0xd2da			//�� Ԫ		���磺2011��������ҵ���о�����Ϊ10412��Ԫ  hyl 2012-04-09
						  && nWordTemp != (emInt16)0xc3eb)			//��
					 {
						 //���ִ����Ƿ�Ϊ��-�������ֻ������ǣ�139-1234-5678�� �绰��010-62986600 ��
						 nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf ,nCurIndexOfBuf );
						 if( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa1aa) )
						 {//start
							   if( DATATYPE_SHUZI == CheckDataType( strDataBuf, nCurIndex + 2) ) 
							   { //start--1   

									GenPauseNoOutSil();	//�����βͣ��Ч��

									nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf,nCurIndex + 2);
									if( (GetWord( strDataBuf,nCurIndex + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa3ad) ||(GetWord( strDataBuf,nCurIndex + 2 + nSumOfReadHaoMa*2 ) == (emInt16)0xa1aa) )
									{//start--2
										  i = nCurIndex + 2 + nSumOfReadHaoMa*2 ;
										  if( DATATYPE_SHUZI == CheckDataType( strDataBuf, i + 2) )
										  {
											  GenPauseNoOutSil();	//�����βͣ��Ч��

											  nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf, i + 2);
											  nCurIndexOfBuf =  i + 2+ nSumOfReadHaoMa*2;
											  return nCurIndexOfBuf;  //end of XXXX-XXXXXXXX-XXXXX
										  }
									}//end--2

									nCurIndexOfBuf =  nCurIndex + 2 + nSumOfReadHaoMa*2 ;
									return nCurIndexOfBuf; //end of XXXX-XXXXXXX
							   }//end--1
						 }//end
					 
						 nCurIndexOfBuf = nCurIndex;
						 return nCurIndexOfBuf; //end of XXXXX
					 }
					 
					 if( nSumOfShuZi < 3 )		//�����ӵ�		���磺ad40�� X86ϵ�С��绰55966569��12Сʱ����
					 {
						 if( CheckDataType( strDataBuf ,nCurIndexOfBuf-2)== DATATYPE_YINGWEN)  //hyl  2012-03-30
						 {
							 nSumOfReadHaoMa = ReadDigitHaoMa( strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf += nSumOfReadHaoMa*2 ;
							 return nCurIndexOfBuf; 
						 }
					 }
				 }
			 


				 //���������ִ��ĸ���Ϊ11λ��ֻҪǰ2λΪ��13��15��18���Զ�ʶ����ֻ�����
				 //�ֻ�ǰ3Ϊ�Ŷ�ͳ�ƣ�130,131,132,133,134,135,136,137,138,139,150,151,152,153,155,156,157,158,159,180,185,186,187,188,189
				 if( nSumOfShuZi == 11 ) //����Ϊ11
				 {
					 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf );
					if( nWordTemp == (emInt16)0xa3b1 )  //��1λ�ǡ�1��
					{
						nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf+2 );
						if ( nWordTemp == (emInt16)0xa3b3 || nWordTemp == (emInt16)0xa3b5 || nWordTemp == (emInt16)0xa3b8  )  //��2λ�ǡ�3����5����8��
						{
							 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
							 return nCurIndexOfBuf; 
						}
					}
				 }



				 //�����ִ���7λ�����ϣ���յ��ǣ�7λ��8λ�����Һ���ĵ�1���ֲ��Ǻ��֣�ע�⣺�����뺺��֮��Ŀո���֮ǰ��ȥ��������ʶ��ɵ绰���롣
				 if( nSumOfShuZi == 7  || nSumOfShuZi == 8  )	//hyl 2012-03-29
				 {
					 //�����ֲ��Ǻ��֣����Ǻ��֡�ת���͡��򡱣���պõ���8λ������ɺ���
					if( DATATYPE_HANZI != CheckDataType( strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2 )   
						|| GetWord( strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2 ) ==  ( emInt16 ) 0xd7aa		
						|| GetWord( strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2 ) ==  ( emInt16 ) 0xbbf2  
						|| nSumOfShuZi == 8 )
					{
							 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
							 return nCurIndexOfBuf; 
					}
				 }

				 //����1������Ϊ8��������ǰ����Ϊ����+���ҷ���+ǰ�������֣�����ɺ���ʹ򿪶����뿪��
				 //���磺 +852 2185 6460 ��   +852-28220122�� +86 21 61224911��+86(0)21 6495 1616��+86+755+83667282��2+8�� 34 + 86 = 120��
				 nWordPrev = GetWord( strDataBuf,nCurIndexOfBuf-2);
				 nWordTemp = GetWord( strDataBuf,nCurIndexOfBuf);
				 if(   nWordPrev == (emInt16)0xa3ab &&  nWordTemp == (emInt16)0xa3b8   //  Ϊ��+8
					 && CheckDataType( strDataBuf,nCurIndexOfBuf-4)!=DATATYPE_SHUZI)
				 {
					 nSumTemp1 = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);					 
					 nCurIndexOfBuf +=  nSumTemp1*2;
					 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;
					 GenPauseNoOutSil();	//�����βͣ��Ч��
					 return nCurIndexOfBuf; 			 
				 }
			 } //endof �� if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )


			 // ��/"�����⴦��
			 if ( nWord == (emInt16)0xa3af )//��б�ܵ��жϿ�ʼ�����Ǽ���֮��������磨4/5: ���֮�ģ� 
			  {
				 nDataType = CheckDataType( strDataBuf, nCurIndex + 2); 
				 if( nDataType == DATATYPE_SHUZI  										//  ����������(������1λ)
					 && CheckDataType( strDataBuf, nCurIndex - 4 ) != DATATYPE_SHUZI)	//	ǰ��ֻ����1λ����
				 {
					 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndex + 2 );
					 nSumTemp = nSumOfReadShuZhi;
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb7d6 ); //��
					 PutIntoWord( g_hTTS->m_ShuZiBuffer,2, 0xd6ae ); //֮
					 PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, END_WORD_OF_BUFFER );
					 SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);	//���ţ���֮
					 ClearBuffer( g_hTTS->m_ShuZiBuffer, 5);
					 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
					 nCurIndexOfBuf += nSumOfReadShuZhi*2+2+nSumTemp*2;
					 return nCurIndexOfBuf; //end of ReadShuZi To X/X���ɷ���
				 }//end of X/X
			  }

		
			if ( g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == emTTS_READDIGIT_AUTO )
			{
				 //�жϣ��������ִ��ĳ���Ϊ4λ��
				 //��  ���ִ��������һ���ܶ����ۣ�
				 if ( nSumOfShuZi == 4 )
				 {
					 //modified by hyl 2012-04-09
					 //���磺��ʡ1955-1956�겶��171ֻ�����ֵ����ʱ����1600-2200Сʱ֮�䡣���ֵ����ʱ����1955-2200Сʱ֮��
					 nTemp = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 if (    ( nTemp >= 1900 && nTemp <= 2050)					//���ַ�Χ��1900��-2050��
						  && (   (nWord == (emInt16)0xa3ad)						//���ţ���-��������	��~��	
						      || (nWord == (emInt16)0xa1ab) 
							  || (nWord == (emInt16)0xa3fe)))	
					 {
						 if( CheckDataType(strDataBuf,nCurIndexOfBuf+10) == DATATYPE_SHUZI)  
						 {
							 nTemp = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf+10 );
							 if( nTemp >= 1900 && nTemp <= 2050)							 //���ź���������ַ�Χ��1900��-2050��
							 {
								 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
								 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
								 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
								 return nCurIndexOfBuf; 
							 }
						 }
					 }
				 }



				 //������������5λ��ʶ��ɺ���   ��  10010��10060��10086��12315��17909��17911��17950��17951
				 if ( nSumOfShuZi == 5 )
				 {
					 nTempLong = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 if (    nTempLong == 10010 ||  nTempLong == 10060 || nTempLong == 10086 || nTempLong == 12315
						  || nTempLong == 17909 ||  nTempLong == 17911 || nTempLong == 17950 || nTempLong == 17951) 
					 {
						 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
						 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
						 return nCurIndexOfBuf; 
					 }
				 }


				 //������������3λ��ʶ��ɺ���   ��  110��114��119��120��122		added by hyl 2012-03-30
				 //���磺  Ī�Ҵ�122�����绰�����绯����,��110��119��120�� 122 �ȽӴ�������ϵͳ��
				 //���磺  110��114��119��120��122��
				 //���磺  ����110���ˡ�֧��122Ԫ��
				 if ( nSumOfShuZi == 3 )
				 {
					 nTempLong = ChangeShuZiToInt( strDataBuf,nCurIndexOfBuf );
					 if (    nTempLong == 110 ||  nTempLong == 114 || nTempLong == 119 
						  || nTempLong == 120 ||  nTempLong == 122 ) 
					 {
						 //�������ִ����Ǻ��ֺ���ĸ����ǰ�滹�к��֣�������ɺ���
						 if(    nCurIndexOfBuf >= 2 
							 && CheckDataType( strDataBuf,nCurIndexOfBuf-2) == DATATYPE_HANZI		
							 && CheckDataType( strDataBuf,nCurIndexOfBuf+6) != DATATYPE_HANZI	
							 && CheckDataType( strDataBuf,nCurIndexOfBuf+6) != DATATYPE_YINGWEN)	
						 {
							 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
							 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
							 g_hTTS->m_ControlSwitch.m_bIsPhoneNum = emTrue;	//�򿪶����뿪��
							 return nCurIndexOfBuf; 
						 }
					 }
				 }

			 }

			 //�����ִ���2λ������0��β���Һ�����֡��󡱣�����ɺ���
			 //���ӣ� 80��Ľ�������Ե���һЩ80����������ˡ��ǵ��͵ġ�80��
			 if(    nSumOfShuZi == 2												//�պ�2λ����
				 && GetWord(strDataBuf,nCurIndexOfBuf+2)== (emInt16)0xa3b0			//��0��β
				 && GetWord(strDataBuf,nCurIndexOfBuf+4)== (emInt16)0xbaf3			//������֡���
				 && (    GetWord(strDataBuf,nCurIndexOfBuf)==(emInt16)0xa3b0		//��1λ����=0����>=6
				      || GetWord(strDataBuf,nCurIndexOfBuf)>=(emInt16)0xa3b6))	
			 {
				 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
				 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
				 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
				 return nCurIndexOfBuf; 
				 
			 }

			 //��������������ֶ��ɺ���  ���磺ah.12530.com��hhh@263.net��  hyl 2012-04-09
			 if(    nWord  == (emInt16) 0xa3ae 		//���ֺ�����š�.��
				 && CheckDataType(strDataBuf,nCurIndexOfBuf+nSumOfShuZi*2+2) == DATATYPE_YINGWEN )	//���š�.�����Ӣ����ĸ
			 {
				 nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YAO;
				 nSumOfReadHaoMa = ReadDigitHaoMa(strDataBuf,nCurIndexOfBuf);
				 g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
				 nCurIndexOfBuf +=  nSumOfReadHaoMa*2;
				 return nCurIndexOfBuf; 
			 }


			 //ʣ�µ�ȫ������ֵ����
			 nSumOfReadShuZhi = ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf );
			 nCurIndexOfBuf += nSumOfReadShuZhi * 2;
			 return nCurIndexOfBuf;//end of Read ShuZi To ��ֵ

		}// end of (nSumOfShuZi <= 16)
   }//end of (g_hTTS->m_ControlSwitch.m_nReadDigitPolicy == ....)
} //end of ProcessShuZi 

//****************************************************************************************************
//  ���ܣ�������ִ��Ƿ�Ϊ�ϸ���������ͣ�������򲥷����ڣ���������򷵻�ֵΪ��
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����������ִ��ж������ڵ����ָ������ַ����� nSumOfReadDate
//****************************************************************************************************
emInt16  emCall  CheckDateFormat( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadDate = 0 ;
	emInt16 nDataType = 0 ;
	emInt16 nSumOfShuZi = 0;
	emInt16 nSumOfTemp = 0;
	emInt16 nWord = 0 ;
	emInt16 nWordTemp = 0 ;
	emInt16 nTemp = 0 ;
	emInt16 nSum = 0 ;
    emInt16 nFuHaoTemp = 0 ;

	LOG_StackAddr(__FUNCTION__);

	nDataType = CheckDataType( pCurStr, nIndex );
	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );

	if( nDataType != DATATYPE_SHUZI )
	{
		return nSumOfReadDate;
	}

	if ( nSumOfShuZi == 4 ) //�Ƿ�Ϊ��λ����
	{
		nWordTemp = ChangeShuZiToInt( pCurStr, nCurIndex );
			if ( nWordTemp >= 1801 && nWordTemp <= 2099)
			{
				nIndex = nCurIndex + nSumOfShuZi * 2;
				nWord = GetWord( pCurStr,nIndex );
				if ( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae) )
				{  
					if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
					{
						nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
						nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
						if ( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 13 )  //��������YYYYMM
						{   
							nFuHaoTemp = nWord;//����ʱ����ţ������ж��Ƿ�������ʱ�����һ��
							nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
							ReadDigitHaoMa( pCurStr, nCurIndex );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc4ea );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ���
							ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
							ReadDigitShuZhi( pCurStr, nIndex + 2 );
							g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4c2 );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ���
							ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
							nIndex = nIndex + 2 + nSumOfTemp * 2;
							nSumOfReadDate = nIndex - nCurIndex ;
							nWord = GetWord( pCurStr, nIndex );
							if ( nWord == nFuHaoTemp ) //ʱ������Ƿ���ǰ���һ�� ���ָ�ʽΪXXXX-XX-XX ����ʽΪXXXX-XX/XX��XXXX-XX.XX��XXXX-XX\XX�ǲ��ϸ��ʱ���ʽ
							{
								nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
								nWordTemp = ChangeShuZiToInt( pCurStr, nIndex + 2 );
								if( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 32 )
								{
									ReadDigitShuZhi( pCurStr, nIndex + 2 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc8d5 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//���ţ����ա� 
									ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
									nIndex = nIndex + 2 + nSumOfTemp * 2;
									nSumOfReadDate = nIndex - nCurIndex ;
									nWord = GetWord( pCurStr, nIndex );
									if( nWord  == (emInt16)0xc8d5 )//YYYYMMDD������ա��������ظ�����
									{
										nSumOfReadDate = nIndex + 2 - nCurIndex  ;
									}
								}
							}
							else if( nWord == (emInt16)0xd4c2 )//YYYYMM������¡��������ظ�����
							{
								nSumOfReadDate = nIndex + 2 - nCurIndex ;
							}
						}
					}

				}
			}
	}

	 if ( nSumOfShuZi == 2 ) //�Ƿ�Ϊ��λ����
	{   
		nIndex = nCurIndex + nSumOfShuZi * 2;
			nWord = GetWord( pCurStr,nIndex );
			if ( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae) )
			{
				if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
				{
					nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
					nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
					if ( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 13 )  //��������YYMM
					{   
                        nFuHaoTemp = nWord;//����ʱ����ţ������ж��Ƿ�������ʱ�����һ��
	            		nIndex = nIndex + 2 + nSumOfTemp * 2;
						nWord = GetWord( pCurStr, nIndex );
						if ( nWord == nFuHaoTemp ) //ʱ������Ƿ���ǰ���һ�� ���ָ�ʽΪXX-XX-XX ����ʽΪXX-XX/XX��XX-XX.XX��XX-XX\XX�ǲ��ϸ��ʱ���ʽ
						{
							nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
							nWordTemp = ChangeShuZiToInt( pCurStr, nIndex + 2 );
							if( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 32 ) //��������YYMMDD
							{   
								nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								ReadDigitHaoMa( pCurStr, nCurIndex );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc4ea );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//���ţ���
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								ReadDigitShuZhi( pCurStr, nCurIndex + nSumOfShuZi * 2 + 2  );
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp ;
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4c2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ���
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								ReadDigitShuZhi( pCurStr, nIndex + 2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc8d5 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_q );			//���ţ����ա� 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								nIndex = nIndex + 2 + nSumOfTemp * 2;
								nSumOfReadDate = nIndex - nCurIndex ;
								nWord = GetWord( pCurStr, nIndex );
								if( nWord  == (emInt16)0xc8d5 )//YYMMDD������ա��������ظ�����
								{
									nSumOfReadDate = nIndex + 2 - nCurIndex  ;
								}
							}
						}
					}
				}

			}
	}

	
	if ( (nSumOfShuZi == 1) || (nSumOfShuZi == 2) )//����ǰ
	{
		nWordTemp = ChangeShuZiToInt( pCurStr, nCurIndex );
		if ( nWordTemp > 0 && nWordTemp < 13 )
		{
			nIndex = nCurIndex + nSumOfShuZi * 2;
			nWord = GetWord( pCurStr,nIndex );
			if ( (nWord == (emInt16)0xa3ad) || (nWord == (emInt16)0xa3af) || (nWord == (emInt16)0xa3dc) || (nWord == (emInt16)0xa3ae) )
			{   
				nFuHaoTemp = nWord;//����ʱ����ţ������ж��Ƿ�������ʱ�����һ��
				if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
				{
					nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
					nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
					if ( (nSumOfTemp == 2 || nSumOfTemp == 1) && nWordTemp >0 && nWordTemp < 32 )  
					{
						nIndex = nIndex + 2 + nSumOfTemp * 2;
						nWord = GetWord( pCurStr, nIndex );
						if ( nWord == nFuHaoTemp ) //ʱ������Ƿ���ǰ���һ�� ���ָ�ʽΪXX-XX-XXXX ����ʽΪXX-XX/XXXX��XX-XX.XXXX��XX-XX\XXXX�ǲ��ϸ��ʱ���ʽ
						{
							nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
							nWordTemp = ChangeShuZiToInt( pCurStr, nIndex + 2 );
							if( nSumOfTemp == 4  && nWordTemp > 1800 && nWordTemp < 2100 )
							{
								nTemp = g_hTTS->m_ControlSwitch.m_bYaoPolicy;
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = emTTS_CHNUM1_READ_YI;
								ReadDigitHaoMa( pCurStr, nIndex + 2 );
								g_hTTS->m_ControlSwitch.m_bYaoPolicy = nTemp;
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc4ea );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ����ꡱ 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								ReadDigitShuZhi( pCurStr, nCurIndex );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4c2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ����¡� 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								ReadDigitShuZhi( pCurStr, nCurIndex + nSumOfShuZi * 2 + 2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc8d5 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ����ա� ��
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								nIndex = nIndex + 2 + nSumOfTemp * 2;
								nSumOfReadDate = nIndex - nCurIndex ;
								if( GetWord( pCurStr,nIndex ) == (emInt16)0xc8d5 )//MMDDYYYY����꣬�����ظ�����
								{
									nSumOfReadDate = nIndex + 2 - nCurIndex  ;
								}
							}
						}
					}
				}
			}
		}
	}
	return nSumOfReadDate;
}

//****************************************************************************************************
//  ���ܣ�������ִ��Ƿ�Ϊ�ϸ��ʱ�����ͣ�������򲥷�ʱ�䣬��������򷵻�ֵΪ��
//	�����������ǰ�ַ���emPByte pCurStr�����λ����emInt16  nCurIndex
//	���أ����������ִ��ж���ʱ������ּ�������صĸ��� nSumOfReadTime
//****************************************************************************************************
emInt16  emCall  CheckTimeFormat( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nIndex = nCurIndex;
	emInt16 nSumOfReadTime = 0 ;
	emInt16 nDataType = 0 ;
	emInt16 nSumOfShuZi = 0;
	emInt16 nSumOfTemp = 0;
	emInt16 nWord = 0 ;
	emInt16 nWordTemp = 0;
	emInt16 nTemp = 0 ;
	emInt16 nSum = 0 ;
	emInt16 nNeedAddLen = 0;
	emInt16 nShi;

	LOG_StackAddr(__FUNCTION__);

	nDataType = CheckDataType( pCurStr, nIndex );
	nSumOfShuZi = GetStrSum( pCurStr, nIndex, DATATYPE_SHUZI );

	if( nDataType != DATATYPE_SHUZI )
	{
		return nSumOfReadTime;
	}

	if ( nSumOfShuZi == 2 || nSumOfShuZi == 1 ) 
	{
		nWordTemp = ChangeShuZiToInt( pCurStr, nIndex );
		nShi = nWordTemp;
		if ( nWordTemp < 25 )
		{
			nIndex += nSumOfShuZi * 2;
			nWord = GetWord( pCurStr,nIndex );
			if ( (nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3))
			{
				if ( CheckDataType( pCurStr, nIndex + 2 ) == DATATYPE_SHUZI )
				{
					nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
					nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
					if ( (nSumOfTemp == 2) && (nWordTemp < 60) )  
					{
						nIndex +=  2 + nSumOfTemp * 2;
                        nWord = GetWord( pCurStr,nIndex );
						if ( ((nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3)) &&((GetStrSum(pCurStr,nIndex + 2,DATATYPE_SHUZI)!=2) || ((GetStrSum(pCurStr,nIndex + 2,DATATYPE_SHUZI)==2)&&(ChangeShuZiToInt(pCurStr,nIndex + 2)>59)))) //���Ǻϸ�ʱ���ʽ
						{ 
                          return nSumOfReadTime;
						}  
							//�Ǻϸ��ʱ���ʽ

							//��pm����ʽ 
							if( GetWord( pCurStr,nIndex ) == (emInt16)0xa3f0 &&  GetWord( pCurStr,nIndex+2 ) == (emInt16)0xa3ed  )  
							{
								if(  nShi<=1)			//����
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd6d0 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xcee7 );
									nNeedAddLen = 4;
								}
								if( nShi>1 && nShi<=6)	//����
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xcfc2 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xcee7 );
									nNeedAddLen = 4;
								}
								if( nShi>6 )			//����
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xcded );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xc9cf );
									nNeedAddLen = 4;
								}

								
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_t);			//���ţ����磬���磬����
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  5 );			//������ֻ����д洢������
								
							}
							//��am����ʽ 
							if( GetWord( pCurStr,nIndex ) == (emInt16)0xa3e1 &&  GetWord( pCurStr,nIndex+2 ) == (emInt16)0xa3ed  )	
							{
								if(  nShi<=4)			//�賿
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e8 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xb3bf );
									nNeedAddLen = 4;
								}
								if( nShi>4 && nShi<=7)	//����
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xd4e7 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xc9cf );
									nNeedAddLen = 4;
								}
								if( nShi>7 )			//����
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc9cf );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xcee7 );
									nNeedAddLen = 4;
								}
								
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_t);			//���ţ��賿�����ϣ�����
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  5 );			//������ֻ����д洢������
								
							}

							g_hTTS->m_bIsErToLiang = emTrue;
							ReadDigitShuZhi( pCurStr, nCurIndex );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xb5e3 );
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);				//���ţ����㡱 
							ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������

							if( nWordTemp != 0 )			//���ǡ�3:00:14�����ָ�ʽ
							{
								if ( (nSumOfTemp == 2)&&(nWordTemp < 10) )
								{
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ����㡱 ���֣�
									ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
								}
								ReadDigitShuZhi( pCurStr, nIndex - nSumOfTemp * 2 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xb7d6 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ����֡� 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
							}
							else							//���ǡ�3:00:14�����ָ�ʽ�� ���ɡ���֡�
							{
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, 0xb7d6 );
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ�����֡� 
								ClearBuffer( g_hTTS->m_ShuZiBuffer,  5 );			//������ֻ����д洢������
							}

							//�Ƿ�ΪA:BB+�ֵĸ�ʽ�����������ķֲ�����
							if ( nWord == (emInt16)0xb7d6 ) //��
							{
							 nIndex += 2;
							}
                             nSumOfReadTime = nIndex - nCurIndex + nNeedAddLen;

        					if ( (nWord == (emInt16)0xa3ba)||(nWord == (emInt16)0xa1c3) )
							{
								nSumOfTemp = GetStrSum( pCurStr, nIndex + 2, DATATYPE_SHUZI );
								nWordTemp = ChangeShuZiToInt( pCurStr,  nIndex + 2 );
								if ((nSumOfTemp == 2)  && (nWordTemp < 60) ) //��������A:BB:CC or AA:BB:CC
								{
									if ( (nSumOfTemp == 2)&&(nWordTemp < 10) && (nWordTemp != 0))
									{
										PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc1e3 );
										PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
										SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);			//���ţ����㡱���룩 
										ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������
									}
									
									ReadDigitShuZhi( pCurStr, nIndex + 2 );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, 0xc3eb );
									PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
									SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);			//���ţ����롱 
									ClearBuffer( g_hTTS->m_ShuZiBuffer,  3 );			//������ֻ����д洢������

									nIndex +=  2 + nSumOfTemp * 2;
									//�Ƿ�����a:BB:CC��ĸ�ʽ�����������벻����
									if ((GetWord(pCurStr,nIndex))== (emInt16)0xc3eb)
									{
									 nIndex += 2;
									}
									nSumOfReadTime = nIndex - nCurIndex ;
								}
							}
						}
				}
			}
		}
	}
	nSumOfReadTime /= 2;
	return nSumOfReadTime;
}


