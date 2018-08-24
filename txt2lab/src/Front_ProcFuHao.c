#include "emPCH.h"

#include "Front_ProcFuHao.h"



emInt16 emCall ProcessFuHao( emPByte strDataBuf, emInt16 nCurIndexOfBuf)
{
	emInt16 nSumOfFuHao = 0;
	emInt16 nSumOfShuZi = 0;
	emInt16	nSumOfHanZi = 0;
	emInt16	nSumOfYingWen = 0;
	emInt16	nCurIndexSZBuffer = 0;
	emInt16	nWord ,nNextWord;
	emInt16	nWord1,nWord2,nWord3;
	emInt16 nSumOfReadHaoMa = 0;
	emInt16 nDataType,nDataType1,nDataType2;
	emInt16 nTemp = 0 ;
	emInt32 nWordTemp = 0;///���Ʊ��[��ĸ+����]
	emInt16 i,j;
	

	emBool    bHaveOnlyPinYin;
	emByte   *pnShengMuNo;	    //ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
	emByte   *pnYunMuNo;		//ָ�봫�Σ���Чƴ������ĸ���룬��ʼֵΪ0
	emByte   *pnShengDiaoNo;    //ָ�봫�Σ���Чƴ�����������룬��ʼֵΪ0
	emInt16   nValueOfHandelPinYinReturn;
	emByte    PinYin[3] ;

	LOG_StackAddr(__FUNCTION__);

	pnShengMuNo = PinYin;
	pnYunMuNo = PinYin + 1;
	pnShengDiaoNo = PinYin + 2;



	nSumOfFuHao = GetFuHaoSum( strDataBuf,  nCurIndexOfBuf );
	//�жϷ����Ƿ�Ϊ�����������š��������ɡ��ȡ�������Ϊ�������ţ��Ƿ���ɱȷֿ��ء��򿪣���ǰ�������ִ�
	if ( (GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa3ba)||(GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa1c3) )
	{
		if ( g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen == 1) //�����ȡ����ش�
		{
			if ( (nCurIndexOfBuf>1) && (CheckDataType( strDataBuf, nCurIndexOfBuf-2) == DATATYPE_SHUZI) && (CheckDataType( strDataBuf, nCurIndexOfBuf+2) == DATATYPE_SHUZI) )
			{  //�������ǰ���Ϊ��ֵʱ���Ŵ���ɱȷָ�ʽ
				PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb1c8 ); //��
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );	//���ţ� ��
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
				nCurIndexOfBuf += 2;
				return nCurIndexOfBuf;
			}
		}
	}

	//�жϷ����Ƿ�"[="��  ��������������ǵ�����"[=]"�� ǰ��û�и�����
	if((GetWord( strDataBuf,nCurIndexOfBuf ) == (emInt16)0xa3db)&&(GetWord( strDataBuf,nCurIndexOfBuf + 2 ) == (emInt16)0xa3bd))
	{
		bHaveOnlyPinYin = emFalse;
		nValueOfHandelPinYinReturn=GetPinYinCode(strDataBuf,nCurIndexOfBuf,bHaveOnlyPinYin,pnShengMuNo,pnYunMuNo,pnShengDiaoNo );
		if ( nValueOfHandelPinYinReturn != 0 )//�ϸ��ƴ����Ϣ
		{
			nCurIndexOfBuf += nValueOfHandelPinYinReturn;
			return nCurIndexOfBuf;
		}
	}

	nWord = GetWord( strDataBuf, nCurIndexOfBuf );

	//ʶ�𣺿��Ʊ��		
	if ( nWord == (emInt16)0xa3db  )		//����Ϊ"["�������
	{
		emInt8 nCtrlMarkLen;
		//ʶ����Ʊ�ǲ�����ȫ�ֱ���
		nCtrlMarkLen = JudgeCtrlMark(nSumOfFuHao,strDataBuf,nCurIndexOfBuf);		
		if( nCtrlMarkLen > 0 )							//��Ч���Ʊ��
			return (nCurIndexOfBuf+nCtrlMarkLen);
	}


	//������//��ֱ��ȥ�� ������    http://www.tts168.com.cn
	if ( ( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 0 ) ) == (emInt16)0xa3af 
		&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 ) ) == (emInt16)0xa3af )
	{
		nCurIndexOfBuf += 4 ;
		return nCurIndexOfBuf;
	}


	//���һ�����������ж�
	if ( CheckDataType( strDataBuf, nCurIndexOfBuf + 2 ) != (emInt16)DATATYPE_FUHAO			//��1�����Ƿ���
		||  GetWord( strDataBuf, nCurIndexOfBuf + 2 ) == (emInt16)0xa3ad)					//���ߣ���1���Ƿ��š�-��  20120118  hyl
	{
		nWord = GetWord( strDataBuf, nCurIndexOfBuf ) ;

		if( nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa )          //������š�-��   ------6��3��д ���š�����
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2);//��ȡ�������"-"ǰһ���ַ�
			nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf - 2);//��ȡ������š�-"ǰ��һ���ַ�������

			if( nDataType == (emInt16)DATATYPE_SHUZI)//ǰһ���ַ�����Ϊ����
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�
				nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�������
				if(nDataType == (emInt16)DATATYPE_SHUZI)//������š�-����һ���ַ�����ҲΪ����
				{
					if( g_hTTS->m_ControlSwitch.m_bIsHandleToBiFen == 1 )//�ȷֿ��ش򿪶��ɱ�
					{
						//���ɱ�
						PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb1c8 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);	//���ţ� ��
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						nCurIndexOfBuf += 2;
						return nCurIndexOfBuf;
					}
					else
					{
						//������
						PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);	//���ţ� ��
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						nCurIndexOfBuf += 2;
						return nCurIndexOfBuf;
					}
				}
				if(nDataType == (emInt16)DATATYPE_YINGWEN)//������š�-����һ���ַ�������Ӣ��
				{
					//���ɸ�
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8dc ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//���ţ� �� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}

			}

			if( nDataType == (emInt16)DATATYPE_YINGWEN )//ǰһ���ַ�����ΪӢ��
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�
				nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�������
				if( nDataType == (emInt16)DATATYPE_YINGWEN )//��һ���ַ�����ҲΪӢ��
				{
					//���ɸ�
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8dc ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//���ţ� ��
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}
				if( nDataType == (emInt16)DATATYPE_SHUZI)//��һ���ַ�����Ϊ����
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 ) //���ֺ���ǡ���ǡ����ʶ�������
					{
						//������������϶Ȳ����ţ�s-12�棩
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
						return nCurIndexOfBuf;

					}
					if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
					{
						nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
						if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) )//С�����������ֵĺ����ǡ�ͬʱ�ո��ǰ�治�ǡ�
						{
							//������������϶Ȳ����ţ����磺s-12.8�棩
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );//���ţ�����
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
							nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ							
							nCurIndexOfBuf += nSumOfShuZi*2 + 2;
							return nCurIndexOfBuf;
						}
					}
					//���ɸ�
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8dc ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);	//���ţ� �� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}
			}

			if( nDataType == (emInt16)DATATYPE_FUHAO)//ǰһ���ַ�����Ϊ����
			{
				if( (nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2)) == (emInt16)0xa1e6 )//�������'-'ǰһ������ǡ���ǡ�
				{
					nWord = GetWord( strDataBuf,nCurIndexOfBuf + 2);
					nDataType = CheckDataType( strDataBuf,nCurIndexOfBuf + 2 );
					if( nDataType == (emInt16)DATATYPE_SHUZI )//��һ���ַ�����Ϊ����
					{
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
						if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 ) //���ֺ���ǡ���ǡ����
						{
							//��������12��-12�棩
							PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //��
							PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);	//���ţ� ��	
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
							ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
						if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) )//С�����������ֵĺ����ǡ�
							{
								//��������12��-12.9�棩
								PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //��
								PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);		//���ţ� ��
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);

								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ; //��-����������ֶ�����ֵ					
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi +  1 ) ;
								return nCurIndexOfBuf;
							}
						}
					}
				}
				if( nWord != (emInt16)0xa1eb && nWord != (emInt16)0xa987 && nWord != (emInt16)0xa3a5  )
				{
					nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�
					nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�������
					if( nDataType == (emInt16)DATATYPE_SHUZI )//��һ���ַ�����Ϊ����
					{
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
						if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 
							&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6 ) //���ֺ���ǡ���ǡ����ʶ�������
						{
							//������������϶Ȳ����ţ���-12�棩
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
							ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
						if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) 
								&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6)//С�����������ֵĺ����ǡ�
							{
								//������������϶Ȳ����ţ������-12.8�棩
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
								PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������

								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
								return nCurIndexOfBuf;
							}
						}
						//���ֺ��治�ǡ�Ͷ��ɸ�
						PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8ba ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_v);	//���ţ� ��������
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
						nCurIndexOfBuf += 2;
						return nCurIndexOfBuf;
					}
				}

			}


			if( nDataType == (emInt16)DATATYPE_UNKNOWN)//ǰһ���ַ�����Ϊδ֪ ������ӵ�5���������
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�
				nDataType = CheckDataType( strDataBuf, nCurIndexOfBuf + 2);//��ȡ������š�-����һ���ַ�������
				if( nDataType == (emInt16)DATATYPE_SHUZI )//��һ���ַ�����Ϊ����
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6 
						&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6 ) //���ֺ���ǡ���ǡ����ʶ�������
					{
						//������������϶Ȳ����ţ���-12�棩
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
						return nCurIndexOfBuf;
					}

					//hyl 2012-10-28 ������������
					//���䣺-1�档1�档�Ͻ�����¶Ƚ���-1~-8�档�Ͻ�����¶Ƚ���-1~8�档-1��-8�档-1��8�档-1~-8�档-1~8�档�Ͻ�����¶Ƚ���-1 ~ -8�档�Ͻ�����¶Ƚ���-1 ~ 8�档-1 �� -8�档-1 �� 8�档-1 ~ -8�档-1 ~ 8�档1 �� -8�档1 �� 8�档1��-8�档1��8�档1 ~ -8�档1 ~ 8�档1~-8�档1~8�档
					nTemp = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);
					nWord = GetWord( strDataBuf,  nTemp);
					nNextWord = GetWord( strDataBuf, nTemp+2 );
					if(    nWord == (emInt16)0xa1ab  //���ţ���
						|| nWord == (emInt16)0xa3fe  //���ţ�~
						|| (nWord == (emInt16)0xa3a0 &&  (nNextWord == (emInt16)0xa1ab || nNextWord == (emInt16)0xa3fe) ))//���ţ��ո�  �� �ո�~
					{
						for( j=2; j<16;j=j+2)
						{
							nWord = GetWord( strDataBuf, nTemp+j );
							if( nWord == (emInt16)0xa1e6 ) //���ţ���	
								break;
						}

						if( j != 16 )
						{
							//������������϶Ȳ�����
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
							ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
					}


					if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
					{
						nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
						if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) 
							&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2 ) ) != (emInt16)0xa1e6)//С�����������ֵĺ����ǡ�
						{
							//������������϶Ȳ����ţ������-12.8�棩
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
							PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
							PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
							SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
							ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������

							nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
							nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
							return nCurIndexOfBuf;
						}
					}


					//���ֺ��治�ǡ�Ͷ��ɸ�
					PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xb8ba ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_v);	//���ţ� �������� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
					nCurIndexOfBuf += 2;
					return nCurIndexOfBuf;
				}
			}

			//������š�-���������ϸ������
			nWord = GetWord( strDataBuf,nCurIndexOfBuf + 2);
			nDataType = CheckDataType( strDataBuf,nCurIndexOfBuf + 2 );
			if( nDataType == (emInt16)DATATYPE_SHUZI )//��һ���ַ�����Ϊ����
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
				if(( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1) ) ) == (emInt16)0xa1e6  ) //���ֺ���ǡ���ǡ����
				{
					//������������϶Ȳ����ţ���-12�棩
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
					ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
					return nCurIndexOfBuf;
				}

				//hyl 2012-10-28 ������������
				//���䣺-1�档1�档�Ͻ�����¶Ƚ���-1~-8�档�Ͻ�����¶Ƚ���-1~8�档-1��-8�档-1��8�档-1~-8�档-1~8�档�Ͻ�����¶Ƚ���-1 ~ -8�档�Ͻ�����¶Ƚ���-1 ~ 8�档-1 �� -8�档-1 �� 8�档-1 ~ -8�档-1 ~ 8�档1 �� -8�档1 �� 8�档1��-8�档1��8�档1 ~ -8�档1 ~ 8�档1~-8�档1~8�档
				nTemp = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);
				nWord = GetWord( strDataBuf,  nTemp);
				nNextWord = GetWord( strDataBuf, nTemp+2 );
				if(    nWord == (emInt16)0xa1ab  //���ţ���
					|| nWord == (emInt16)0xa3fe  //���ţ�~
					|| (nWord == (emInt16)0xa3a0 &&  (nNextWord == (emInt16)0xa1ab || nNextWord == (emInt16)0xa3fe) ))//���ţ��ո�  �� �ո�~
				{
					for( j=2; j<16;j=j+2)
					{
						nWord = GetWord( strDataBuf, nTemp+j );
						if( nWord == (emInt16)0xa1e6 ) //���ţ���	
							break;
					}

					if( j != 16 )
					{
						//������������϶Ȳ�����
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1 ) ;
						return nCurIndexOfBuf;
					}
				}


				if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1))) == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
				{
					nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 4,(emInt16)DATATYPE_SHUZI );
					if( nSumOfReadHaoMa > 0 && ((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 2 ))) == (emInt16)0xa1e6) )//С�����������ֵĺ����ǡ�
					{
						//������������϶Ȳ����ţ������-12.8�棩
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_m );//���ţ�����
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������

						nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi  + 1 ) ;
						return nCurIndexOfBuf;
					}
				}
				//������
				PutIntoWord( g_hTTS->m_ShuZiBuffer,0, 0xd6c1 ); //��
				PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, END_WORD_OF_BUFFER );
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );	//���ţ� ��
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);
				nCurIndexOfBuf += 2;
				return nCurIndexOfBuf;
			}

		}

	

		if ( nWord == (emInt16)0xa1ab || nWord == (emInt16)0xa3fe)//�������"�� " {����~���ϻ��ߣ��Ĵ���Ҳ������}����С��6��1��
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2) ;
			if( nWord == (emInt16)0xa3a0 )//�������" �� "ǰ���ǿո�
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4);
				if( nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//�ո�ǰ�治������
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0 )//����������������
					{
						//����� ��  �����ţ�����  ��8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;

					}
				}
				else//�ո�ǰ��������
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0 ) 
					{
						//����� ��  �����ţ�����4 ��8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;
					}
				}
			}

			else//�������"��"ǰ�治�ǿո�
			{
				if (nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//������ǰ��һλ��������
				{
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0 )//"��"����������
					{
						//����� ��  �����ţ����� %��8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;

					}

					//���ӣ�����0�桫-6��			//hyl  20120108
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
					if ( nSumOfShuZi > 0  && GetWord(strDataBuf, nCurIndexOfBuf + 2)==(emInt16)0xa3ad)  //���š�-��
					{
						//����� ��  �����ţ����� %��8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;

					}
				}
				else
				{
					if( GetWord( strDataBuf, nCurIndexOfBuf+2 ) == (emInt16)0xa3ad)		//��һ�����ǡ�-�������� ��-1��-8�档��   //hyl 2012-10-28
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
					else
						nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
					
					if ( nSumOfShuZi > 0 ) 
					{
						//����� ��  �����ţ�����4��8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 ;
						return nCurIndexOfBuf;
					}
				}
			}
		}


		//��"��"���жϡ����������ǲ�������
		if ( nWord == (emInt16)0xa3a4 )
		{
			nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
			if ( nSumOfShuZi > 0 )
			{

				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc8cb ); //��
				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , (emInt16)0xc3f1 ); //��
				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  4 , (emInt16)0xb1d2 ); //��
				PutIntoWord( g_hTTS->m_ShuZiBuffer,  6 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);//���ţ������
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 7 );//������ֻ����д洢������
				nCurIndexOfBuf += 2 ;

				nWord = GetWord( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 );
				if ( nWord == (emInt16)0xa3ae )//���ֺ��滹��С����(���磤20.54)
				{
					nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf );
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd4aa ); //Ԫ
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);//���ţ�Ԫ
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3);//������ֻ����д洢������
					nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi ) ;						
				}
				else
				{
					//�����Ԫ+��ֵ������(���磤20)
					ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf  ); //�ȶ���ֵ
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd4aa ); //Ԫ
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_n );//���ţ�Ԫ
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					nCurIndexOfBuf = nCurIndexOfBuf + 2 *  nSumOfShuZi ;
				}
				nWord     = GetWord( strDataBuf, nCurIndexOfBuf);
				if ( nWord == (emInt16)0xd4aa  )	//����ֵ����ӡ�Ԫ��������
				{
					nCurIndexOfBuf += 2;
				}
				return nCurIndexOfBuf;
			}
		}

		//�ǡ��硱���顱���ꡱ���жϺ����ǲ�������
		if ( nWord == (emInt16)0xa1e7 || nWord == (emInt16)0xa1e9 || nWord == (emInt16)0xa1ea  )
		{
			if( nWord == (emInt16)0xa1e7 )
			{
				nWord1 = (emInt16)0xc3c0 ;		//=��
				nWord2 = (emInt16)0xd4aa ;		//=Ԫ
			}
			if( nWord == (emInt16)0xa1e9 )
			{
				nWord1 = (emInt16)0xc5b7 ;		//=ŷ
				nWord2 = (emInt16)0xd4aa ;		//=Ԫ
			}
			if( nWord == (emInt16)0xa1ea  )
			{
				nWord1 = (emInt16)0xd3a2 ;		//=Ӣ
				nWord2 = (emInt16)0xb0f7 ;		//=��
			}

			nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
			if ( nSumOfShuZi > 0 )
			{
				nWord = GetWord( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 2);
				if ( nWord == (emInt16)0xa3ae )//���ֺ��滹��С����(�����20.54)
				{
					nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf, nCurIndexOfBuf + 2 );
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , nWord1); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , nWord2 ); 
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  4 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_n);//���ţ���Ԫ ŷԪ Ӣ��
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
					nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);						
					
				}
				else
				{
						//�������Ԫ+��ֵ������(�����20)
						ReadDigitShuZhi( strDataBuf, nCurIndexOfBuf + 2 ); //�ȶ���ֵ
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , nWord1); 
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , nWord2); 
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_n );//���ţ���Ԫ ŷԪ Ӣ��
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 1);
				}
				nWord     = GetWord( strDataBuf, nCurIndexOfBuf);
				nNextWord = GetWord( strDataBuf, nCurIndexOfBuf+2);
				if ( nWord == nWord1  && nNextWord == nWord2 )	//����ֵ����ӡ���Ԫ����ŷԪ�� ��Ӣ����������
				{
					nCurIndexOfBuf += 4;
				}
				return nCurIndexOfBuf;
			}
		}


		//---------------------------------------------С�����6��12��start

		if ( nWord == (emInt16)0xa3af ) //�������"/"
		{
			nSumOfHanZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_HANZI );
			if ( nSumOfHanZi == 1)//��/"������������ִ�����Ϊһ
			{
				//�ж�ǰ5�������Ƿ�������
				nTemp = 5;
				if( (nCurIndexOfBuf/2) < nTemp)
				{
					nTemp = nCurIndexOfBuf/2;
				}
				for(i = 0; i< nTemp; i++)
				{
					nWord3 = GetWord( strDataBuf, nCurIndexOfBuf-2-2*i);
					if( nWord3 >= (emInt16)0xa3b0 && nWord3 <= (emInt16)0xa3b9 )  //������
					{
						//��ǰ5�����������֣���/�����ɡ�ÿ��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc3bf );//ÿ
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_r);//���ţ� ÿ�����ӣ�
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						nCurIndexOfBuf = nCurIndexOfBuf + 2;
						return nCurIndexOfBuf;
					}
				}
			}

			if ( nSumOfHanZi >= 2 ) 
			{//��X>=2ʱ�����ҡ��������ʱ�
				if ( CheckHanZiLiangCiTable( strDataBuf, nCurIndexOfBuf + 2 ) != 0 )
				{ //����ֵ��Ϊ�㣬���ڡ��������ʱ���
					//����ÿ
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc3bf );//ÿ
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_r);//���ţ� ÿ�����ӣ�
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					nCurIndexOfBuf = nCurIndexOfBuf + 2;
					return nCurIndexOfBuf;
				}
			}

			nSumOfYingWen = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_YINGWEN );
			if ( nSumOfYingWen <= 3 )//��/"���������Ӣ�Ĵ�����< = 3�� ���ҡ�Ӣ������ת���ֱ�
			{   
				if ( CheckDanWeiTable( strDataBuf, nCurIndexOfBuf + 2 ) != 0 )
				{
					//����ÿ
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc3bf );//ÿ
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_r);//���ţ� ÿ�����ӣ�
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
					CheckDanWeiTable( strDataBuf, nCurIndexOfBuf + 2 );
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_q);//���ţ���ĸ����
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 20 );//������ֻ����д洢������
					nCurIndexOfBuf = nCurIndexOfBuf + 2 + nSumOfYingWen * 2;
					return nCurIndexOfBuf;
				}
			}
		}
		//---------------------------------------------С�����6��12��end



		if ( nWord == (emInt16)0xa3ab ) //��"+"
		{
			//Ĭ�϶��ɡ��ӡ�������ά��ԭ״
			nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 2, (emInt16)DATATYPE_SHUZI );
			if ( nSumOfShuZi >= 13 )
			{
				nCurIndexOfBuf = nCurIndexOfBuf + 2;//"+"������
				ReadDigitHaoMa( strDataBuf, nCurIndexOfBuf );//���ú�������+����������ִ������봦���
				nCurIndexOfBuf = nCurIndexOfBuf + 2 * nSumOfShuZi;
				return nCurIndexOfBuf;
			}
		}

		if( nWord == (emInt16)0xa3aa ) //�ǡ�*��                {Ĭ�϶����� ��ǰ�������ֵ�ʱ����ɳˡ���С�� 6��1������}
		{
			if( CheckDataType(strDataBuf, nCurIndexOfBuf - 2) == (emInt16)DATATYPE_SHUZI && CheckDataType(strDataBuf,nCurIndexOfBuf + 2) == (emInt16)DATATYPE_SHUZI)
			{
				PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xb3cb );//��
				PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
				SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_v);//���ţ����壩 �ˣ��ģ�
				ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
				nCurIndexOfBuf = nCurIndexOfBuf + 2;
				return nCurIndexOfBuf;
			}

		}
	}


	if( nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa && (nWord = GetWord (strDataBuf,nCurIndexOfBuf + 2)) == (emInt16)0xa3a0)//��"-"���ߡ��������ҡ�-�����ߡ�����������һ���ո� �����ӡ���������С��6��1�գ�
	{
		nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2) ;
		if( nWord == (emInt16)0xa3a0 )//�������"-"ǰ���ǿո�
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4);
			if( nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//����"-"ǰ��Ŀո�ǰ�治������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 )//�ո����������

				{
					nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2) );
					if( nWord  == (emInt16)0xa1e6 
						&& ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4)) != (emInt16)0xa1e6))//����С�������ֺ����ǡ�ͬʱ����"-"ǰ��Ŀո�ǰ�治�ǡ�
					{
						//������������϶Ȳ����ţ����� - 12��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 4 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2 ) ;
						return nCurIndexOfBuf;
					}
					else
					{
						if( nWord == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 6,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && 
								((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 3 ))) == (emInt16)0xa1e6) 
								&& ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4)) != (emInt16)0xa1e6))//С�����������ֵĺ����ǡ�ͬʱ�ո��ǰ�治�ǡ�
							{
								//������������϶Ȳ����ţ�����-12.8�棩
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
								PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������

								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 4) ;//��-����������ֶ�����ֵ
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2) ;
								return nCurIndexOfBuf;
							}
						}
						//����� ��  �����ţ����� 4 - 8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
						return nCurIndexOfBuf;
					}
				}
			}
			else//�ո�ǰ��������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 ) 
				{
					//����� ��  �����ţ����� 4- 8��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}

		else//�������"-"ǰ�治�ǿո�
		{
			if (nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//��-��ǰ��һλ��������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				//nWord = GetWord(strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2));
				if ( nSumOfShuZi > 0)// && nWord == (emInt16)0xa1e6 )
				{
					nWord = GetWord(strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2));
					if( nWord == (emInt16)0xa1e6 && ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2)) != (emInt16)0xa1e6))//����С�������ֺ����ǡ�ͬʱ��-��ǰ�治�ǡ�
					{
						//������������϶Ȳ����ţ�����- 12��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
						ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 4 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2 ) ;
						return nCurIndexOfBuf;
					}
					else
					{
						if((nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2))) == (emInt16)0xa3ae )//���ֺ����е㣨��С�������֣�
						{
							nSumOfReadHaoMa = GetStrSum( strDataBuf, nCurIndexOfBuf + nSumOfShuZi * 2 + 6,(emInt16)DATATYPE_SHUZI );
							if( nSumOfReadHaoMa > 0 && 
								((nWord = GetWord ( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + nSumOfReadHaoMa + 3 ))) == (emInt16)0xa1e6)
								&& ((nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2)) != (emInt16)0xa1e6))//С�����������ֵĺ����ǡ�ͬʱ�������-ǰ�治�ǡ�
							{
								//������������϶Ȳ����ţ�����- 12.8��
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xc1e3 ); //��		
								PutIntoWord( g_hTTS->m_ShuZiBuffer ,   2, (emInt16)0xcfc2 ); //��
								PutIntoWord( g_hTTS->m_ShuZiBuffer,  4, END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
								SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_m);//���ţ�����
								ClearBuffer( g_hTTS->m_ShuZiBuffer, 5 );//������ֻ����д洢������
								
								nSumOfShuZi = ReadDigitShuZhiAddDian( strDataBuf , nCurIndexOfBuf + 4 ) ;//��-����������ֶ�����ֵ
								nCurIndexOfBuf = nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2 ) ;
								return nCurIndexOfBuf;
							}
						}
						//����� ��  �����ţ����� d- 8��
						PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
						PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
						SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
						ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
						//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
						nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
						return nCurIndexOfBuf;
					}
				}
			}
			else//��-��ǰ��һλ������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 ) 
				{
					//����� ��  �����ţ�����4- 8��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0,POS_CODE_p );//���ţ��� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��-����������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}

	}

	if( nWord ==(emInt16)0xa1ab || nWord == (emInt16)0xa3fe )//������š����� ��~����������ǿո����� {���ӣ��ϻ��ߵĴ�����Ҳ������}С�š���6��1��
	{
		nWord = GetWord( strDataBuf, nCurIndexOfBuf - 2) ;
		if( nWord == (emInt16)0xa3a0 )//�������" ��"ǰ���ǿո�
		{
			nWord = GetWord( strDataBuf, nCurIndexOfBuf - 4);
			if( nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//����"��"ǰ��Ŀո�ǰ�治������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 )//�ո����������
					//&& ( nWord = GetWord( strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2) ) ) == (emInt16)0xa1e6 )//����"-"����Ŀո�ǰ�治������
				{
					//����� ��  �����ţ����� 4 �� 8��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
			else//�ո�ǰ��������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				if ( nSumOfShuZi > 0 ) 
				{
					//����� ��  �����ţ����� 4�� 8��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}
		else if( GetWord( strDataBuf, nCurIndexOfBuf + 2) == (emInt16)0xa3a0 )//�������" ��"�����ǿո�
		{
			if (nWord < (emInt16)0xa3b0 || nWord > (emInt16)0xa3b9)//������ǰ��һλ��������
			{
				nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );
				//nWord = GetWord(strDataBuf, nCurIndexOfBuf + 2 * ( nSumOfShuZi + 2));
				if ( nSumOfShuZi > 0)// && nWord == (emInt16)0xa1e6 )
				{
					//����� ��  �����ţ����� d�� 8��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
			else//������ǰ��һλ������
			{
				if( GetWord( strDataBuf, nCurIndexOfBuf+4 ) == (emInt16)0xa3ad)		//����һ�����ǡ�-�������� ��-1 �� -8�档��   //hyl 2012-10-28
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 6, (emInt16)DATATYPE_SHUZI );
				else
					nSumOfShuZi = GetStrSum( strDataBuf, nCurIndexOfBuf + 4, (emInt16)DATATYPE_SHUZI );


				if ( nSumOfShuZi > 0 ) 
				{
					//����� ��  �����ţ�����4�� 8��
					PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xd6c1 ); //��
					PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
					SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
					ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
					//ReadDigitShuZhi( strDataBuf , nCurIndexOfBuf + 2 ) ;//��������������ֶ�����ֵ
					nCurIndexOfBuf = nCurIndexOfBuf + 4 ;
					return nCurIndexOfBuf;
				}
			}
		}

	}


	//��"+" ,  
	if ( nWord == (emInt16)0xa3ab ) 
	{
		nDataType1 = CheckDataType( strDataBuf, nCurIndexOfBuf-2);
		nDataType2 = CheckDataType( strDataBuf, nCurIndexOfBuf+2);
		
		//��ǰ���������Ͳ�һ�£��򲻶�
		if(    nDataType1 != nDataType2)
		{
			nCurIndexOfBuf = nCurIndexOfBuf + 2;//"+"������
			return nCurIndexOfBuf;
		}
	}

	//��.����Ҫ����
	if( nWord == (emInt16)0xa3ae && g_hTTS->m_ControlSwitch.bIsReadDian == emTrue)
	{
		PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xb5e3); //��
		PutIntoWord( g_hTTS->m_ShuZiBuffer,  2 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//���ţ��� 
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 3 );//������ֻ����д洢������
		nCurIndexOfBuf += 2 ;
		return nCurIndexOfBuf;
	}



	//����������š�-���򡰡���������ǰ����Ķ��Ǻ��֣��������βͣ��Ч��
	//���ӣ�	Ҳǡ˵�����й����ڵ���������ȷ�ġ�����Ȼ�Ѿ����˶��꣡ 
	//���ӣ�	Ҳǡ˵�����й����ڵ���������ȷ��----��Ȼ�Ѿ����˶��꣡ 
	//���ӣ�	Ҳǡ˵�����й����ڵ���������ȷ�ġ���Ȼ�Ѿ����˶��꣡ 
	//���ӣ�	Ҳǡ˵�����й����ڵ���������ȷ��-��Ȼ�Ѿ����˶��꣡
	//���ӣ�	G1���������٣�����-��������
	nWord = GetWord(strDataBuf,nCurIndexOfBuf);
	if(    (nWord == (emInt16)0xa3ad || nWord == (emInt16)0xa1aa) )			//�ǡ�-�����ߡ�����
		//&& CheckDataType(strDataBuf,nCurIndexOfBuf - 2) == DATATYPE_HANZI)  //�ҷ���֮ǰ�Ǻ���  deleted by hyl  2012-03-30
	{
		nTemp = 0;
		for(;;)
		{
			nTemp += 2;
			if( GetWord(strDataBuf,nCurIndexOfBuf+nTemp) != nWord)
				break;			
		}
		if( CheckDataType(strDataBuf,nCurIndexOfBuf+nTemp) == DATATYPE_HANZI)	//�������������֮���Ǻ���
		{
			GenPauseNoOutSil();	//�����βͣ��Ч��		
			nCurIndexOfBuf += nTemp ;
			return nCurIndexOfBuf;
		}
	}

	//��.��������֣�Ҫ�Ͼ�
	//���䣺					1���˿����á�1.�˿����á�
	if(    nWord == (emInt16)0xa3ae  			//�ǡ�.��
		&& CheckDataType(strDataBuf,nCurIndexOfBuf+2) == DATATYPE_HANZI)  //�ҷ���֮���Ǻ���
	{
		GenPauseNoOutSil();	//�����βͣ��Ч��		
		nCurIndexOfBuf += 2 ;
		return nCurIndexOfBuf;
	}

	//�ǡ�����   ���磺���䣺tanxh@tts.net					//hyl  2012-04-09
	if(    nWord == (emInt16)0xa3c0 ) 			//�ǡ�����
	{
		GenPauseNoOutSil();	//�����βͣ��Ч��		

		PutIntoWord( g_hTTS->m_ShuZiBuffer ,  0 , (emInt16)0xb0ae); //��
		PutIntoWord( g_hTTS->m_ShuZiBuffer ,  2 , (emInt16)0xccd8); //��
		PutIntoWord( g_hTTS->m_ShuZiBuffer,   4 , END_WORD_OF_BUFFER );//ShuZiBuffer�Ľ�β��
		SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,POS_CODE_p);//����
		ClearBuffer( g_hTTS->m_ShuZiBuffer, 6 );//������ֻ����д洢������
		nCurIndexOfBuf += 2 ;

		GenPauseNoOutSil();	//�����βͣ��Ч��		

		return nCurIndexOfBuf;
	}



	//ʣ�µ�	
	nCurIndexOfBuf = CheckGBKFuhaoToHanziDic( strDataBuf, nCurIndexOfBuf );//��GBK���Ŷ��ɺ��ֱ�
	return nCurIndexOfBuf;
}




//****************************************************************************************************
//  ���ܣ���GBK���Ŷ��ɺ��ֱ�----GBKFuhaoToHanziDic
//	�����������ǰ�ַ���emStrA pCurStr�����λ����emInt16  nCurIndex,��ǰΪ���Ŵ��ĵ�ַ
//	����ֵ������nCurIndex
//****************************************************************************************************



emInt16  emCall  CheckGBKFuhaoToHanziDic( emPByte pCurStr, emInt16  nCurIndex )
{   
	emInt16 nWordTemp = 0 ;
	emInt16 nHangIndex = 0 ;
	emInt16 nTemp = 0 ;
	emInt16 nWord1;
    emByte  nHighOfWord;
	emByte  nLowOfWord;
	emUInt8 nPos;

	LOG_StackAddr(__FUNCTION__);


	nHighOfWord  = *( pCurStr + nCurIndex) ;
	nLowOfWord  = *( pCurStr + nCurIndex + 1) ;

	//�жϡ����ɱ�ע���ԡ�
	if( g_hTTS->m_ControlSwitch.m_nManualRhythm == emTTS_USE_Manual_Rhythm_OPEN)
	{
		//�ֶ���ע�����ɶ��#����	 ��¼�ֶ���ע�ĺ��ֵ�����λ�õ�g_hTTS->m_ManualRhythmIndex������ȥ
		//ע�⣺��#���𵽷�pau�����ã��ִ�ʱֻ��ÿ��pau���зִ�
		if ( nHighOfWord == 0xa3 && nLowOfWord == 0xa3  )	//��#��=0xa3a3
		{
			//�������ǡ�#��
			g_hTTS->m_ManualRhythmIndex[g_hTTS->m_CurManualRhythmIndex] = (emInt8)g_hTTS->m_nHanZiCount_In_HanZiLink;
			g_hTTS->m_CurManualRhythmIndex++;

			nCurIndex += 2;
			return  nCurIndex;
		}

		//�ֶ���ע�����ɴʡ�*����	 �ݲ�����ֱ�ӽ���*��ȥ��
		if ( nHighOfWord == 0xa3 && nLowOfWord == 0xaa  )	//��*��=0xa3aa
		{
			//�������ǡ�*��
			nCurIndex += 2;
			return  nCurIndex;
		}
	}
   
	if ( nHighOfWord >= 0xa1 && nHighOfWord <= 0xa3 && nLowOfWord >= 0xa0 && nLowOfWord <= 0xff )
	{//������0xa1a0��0xa3ff��
	  nHangIndex = (nHighOfWord -(emByte)0xA1)*96 + (nLowOfWord-(emByte)0xA0) + 1 ;	//����ƫ����
	}

	else if ( nHighOfWord >= 0xa8 && nHighOfWord <= 0xa9 && nLowOfWord >= 0x40 && nLowOfWord <= 0x9f )
	{//������0xa840��0xa99f��
		nHangIndex =(emInt16) ((nHighOfWord -(emByte)0xA8)*96 + (nLowOfWord-(emByte)0x40)) + (emInt16)288 + 1;	//����ƫ����
	}

	else
	{
		//���������������� ��Ĭ�ϲ�����
			nCurIndex += 2;
			return  nCurIndex;
	}
     

	nTemp = (emInt16)(g_Res.offset_GbkFuhaoToHanzi + nHangIndex);
	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_GbkFuhaoToHanzi + (nHangIndex-1)*10 , 0);


	fFrontRead(&nWordTemp,1,1,g_hTTS->fResFrontMain);
	fFrontRead(&nPos,1,1,g_hTTS->fResFrontMain);

	if ( nWordTemp == 0 )//����ת����
	{
		if ( GetWord(pCurStr , nCurIndex) == (emInt16)INVALID_CODE )
		{
			//GenPauseNoOutSil();	//�����βͣ��Ч��	hyl 1220
		}

		nCurIndex += 2;
		return  nCurIndex;
	}

	if ( nWordTemp == 1 )//�Ƿ���Ҫת����==1
	{
		if (g_hTTS->m_ControlSwitch.m_bPunctuation == emTTS_PUNCTUATION_READ)//�Ҷ���㿪�ش�
		{
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain); //ת�ɵĵ�1�����ֵı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain); //ת�ɵĵ�2�����ֵı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //ת�ɵĵ�3�����ֵı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, nWordTemp );
			fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //ת�ɵĵ�4�����ֵı���
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, nWordTemp );
			PutIntoWord( g_hTTS->m_ShuZiBuffer, 8, END_WORD_OF_BUFFER  ); //ShuZiBuffer�Ľ�β��
			SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,nPos);			//���ţ�����ת�ɵĺ��� 
			ClearBuffer( g_hTTS->m_ShuZiBuffer,  9 );            //������ֻ����д洢������				
		}
		else				//�Ҷ���㿪�عرգ��������ʱ��
		{
			nWord1 =  GetWord( pCurStr, nCurIndex );				
		}
		nCurIndex += 2;
		return  nCurIndex;
	}

	if ( nWordTemp == 2 )  //�Ƿ���Ҫת����==2ʱ����Ҫ������ת�ɺ���
	{
		emInt16  nNextIndexSZBuffer;
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //ת�ɵĵ�1�����ֵı���
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 0, nWordTemp );
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //ת�ɵĵ�2�����ֵı���
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 2, nWordTemp );
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //ת�ɵĵ�3�����ֵı���
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 4, nWordTemp );
		fFrontRead(&nWordTemp,2,1,g_hTTS->fResFrontMain);  //ת�ɵĵ�4�����ֵı���
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 6, nWordTemp );
		PutIntoWord( g_hTTS->m_ShuZiBuffer, 8, END_WORD_OF_BUFFER  ); //ShuZiBuffer�Ľ�β��
		nNextIndexSZBuffer = SoundSZBuffer( g_hTTS->m_ShuZiBuffer, 0 ,nPos);			//���ţ�����ת�ɵĺ���
		if( GetWord( (emPByte)g_hTTS->m_ShuZiBuffer, nNextIndexSZBuffer) == (emInt16)0xa1a3 )
		{
			//��ת����ĺ��ֺ�����ǡ����������ʾ����������Щ���ͣ��� �������ŢƢ٢ڢ����
			GenPauseNoOutSil();	//�����βͣ��Ч��		
		}
		ClearBuffer( g_hTTS->m_ShuZiBuffer,  9 );           //������ֻ����д洢������
		nCurIndex += 2;
		return  nCurIndex;			
	}
	return  nCurIndex;	

}








