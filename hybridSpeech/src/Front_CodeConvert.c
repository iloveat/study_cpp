#include "emPCH.h"
//#include <iconv.h>          //linux����ת��
#include <memory.h>
#include <string.h>
#include "Front_CodeConvert.h"



//****************************************************************************************************
//����ת�������� ���ݱ������ͣ����ñ���ת���Ӻ���
//
//	���أ�				
//			�����룺emTTSErrID    ��=emTTS_ERR_OK����ʾ���α���ת���ɹ���  =emTTS_ERR_END_OF_INPUT����ʾ���α���ת���������˳�TTS��
//
//****************************************************************************************************
emTTSErrID emCall CodeConvertToGBK( )  //���б���ת����ȫ��ת��GBK����
{

#define		ASCII_JuHao			0x002e
#define		ASCII_MaoHao		0x003a
#define		GBK_MaoHao			0xa3ba
#define		BIG5_MaoHao			0xa147 
#define		UNICODE_MaoHao		0xff1a 


	emInt16	pEndPuncCodeSet[4][MAX_END_PUNC][2] = 
	{	
		//��㣺ASCII��-���룺pEndPuncCodeSet[0]	
	  {	{ASCII_JuHao,	PUNC_MUTE_JuHao},			//���		��ASCII����	�����ֽ�
		{0x003f,		PUNC_MUTE_WenHao},			//�ʺ�		��ASCII����	�����ֽ�
		{0x0021,		PUNC_MUTE_GanTanHao},		//��̾��	��ASCII����	�����ֽ�
		{0x000a,		PUNC_MUTE_HuanHangHao},		//���к�	��ASCII����	�����ֽ�
		{0x003b,		PUNC_MUTE_FenHao},			//�ֺ�		��ASCII����	�����ֽ�
		{0x002c,		PUNC_MUTE_DouHao},			//����		��ASCII����	�����ֽ�
		{ASCII_MaoHao,  PUNC_MUTE_MaoHao},			//ð��		��ASCII����	�����ֽ�
		{0x0000,		0},							//����
		{0x0000,		0},							//����
		{0x0000,		0}							//����	
	  },
	
		//��㣺GBK-���룺pEndPuncCodeSet[1]
	  {	{0xa1a3, PUNC_MUTE_JuHao },			//���		��GBK����	��˫�ֽ�
		{0xa3bf, PUNC_MUTE_WenHao },		//�ʺ�		��GBK����	��˫�ֽ�
		{0xa3a1, PUNC_MUTE_GanTanHao },		//��̾��	��GBK����	��˫�ֽ�
		{0xa1ad, PUNC_MUTE_ShengLueHao },	//ʡ�Ժ�	��GBK����	��˫�ֽ�
		{0xa3bb, PUNC_MUTE_FenHao },		//�ֺ�		��GBK����	��˫�ֽ�
		{0xa3ac, PUNC_MUTE_DouHao },		//����		��GBK����	��˫�ֽ�
		{GBK_MaoHao, PUNC_MUTE_MaoHao },	//ð��		��GBK����	��˫�ֽ�
		{0xa1a2, PUNC_MUTE_DunHao },		//�ٺ�		��GBK����	��˫�ֽ�
		{0xa38a, PUNC_MUTE_HuanHangHao},	//���к�	��GBK����	��˫�ֽ�		 
		{0x0000, 0}							//����
	  },
	
		//��㣺BIG5-���룺pEndPuncCodeSet[2]	
	  {	{0xa143, PUNC_MUTE_JuHao },			//���		��BIG5����	��˫�ֽ�
		{0xa148, PUNC_MUTE_WenHao },		//�ʺ�		��BIG5����	��˫�ֽ�
		{0xa149, PUNC_MUTE_GanTanHao },		//��̾��	��BIG5����	��˫�ֽ�
		{0xa14b, PUNC_MUTE_ShengLueHao },	//ʡ�Ժ�	��BIG5����	��˫�ֽ�
		{0xa146, PUNC_MUTE_FenHao },		//�ֺ�		��BIG5����	��˫�ֽ�
		{0xa141, PUNC_MUTE_DouHao },		//����		��BIG5����	��˫�ֽ�
		{BIG5_MaoHao, PUNC_MUTE_MaoHao },	//ð��		��BIG5����	��˫�ֽ�
		{0xa142, PUNC_MUTE_DunHao },		//�ٺ�		��BIG5����	��˫�ֽ�
		{0x0000, 0},						//����
		{0x0000, 0}							//����
	  },
	
		//��㣺UNICODE-���룺pEndPuncCodeSet[3]
	  {	{0x3002, PUNC_MUTE_JuHao },			//���		��UNICODE����	��˫�ֽ�
		{0xff1f, PUNC_MUTE_WenHao },		//�ʺ�		��UNICODE����	��˫�ֽ�
		{0xff01, PUNC_MUTE_GanTanHao },		//��̾��	��UNICODE����	��˫�ֽ�
		{0x2026, PUNC_MUTE_ShengLueHao },	//ʡ�Ժ�	��UNICODE����	��˫�ֽ�
		{0xff1b, PUNC_MUTE_FenHao },		//�ֺ�		��UNICODE����	��˫�ֽ�
		{0xff0c, PUNC_MUTE_DouHao },		//����		��UNICODE����	��˫�ֽ�
		{UNICODE_MaoHao, PUNC_MUTE_MaoHao },//ð��		��UNICODE����	��˫�ֽ�
		{0x3001, PUNC_MUTE_DunHao },		//�ٺ�		��UNICODE����	��˫�ֽ�
		{0x0000, 0},						//����
		{0x0000, 0}							//����
	  }
	};

	LOG_StackAddr(__FUNCTION__);
	
	g_hTTS->m_bConvertWWW = emFalse;

	switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
	{
	case emTTS_CODEPAGE_GBK :
		g_hTTS->m_nErrorID = ConvertGbkOrGB2312ToGbk(pEndPuncCodeSet[0][0] );
		break;
	case emTTS_CODEPAGE_GB2312 :
		g_hTTS->m_nErrorID = ConvertGbkOrGB2312ToGbk(pEndPuncCodeSet[0][0]);
		break;
	case emTTS_CODEPAGE_BIG5 :
		g_hTTS->m_nErrorID = ConvertBig5ToGbk(pEndPuncCodeSet[0][0]);
		break;		
	case emTTS_CODEPAGE_UTF16BE :
	case emTTS_CODEPAGE_UTF16LE :
		g_hTTS->m_nErrorID = ConvertUnicodeToGbk(pEndPuncCodeSet[0][0]);
		break;
	default: 
		g_hTTS->m_nErrorID = emTTS_ERR_END_OF_INPUT;
		break;
	}

	//ת����������������ת�����������е����������Ŀո�
	if( *(g_hTTS->m_pDataAfterConvert) != 0xff)		//���ǽ�������������ת�������ݣ�    �����жϣ�������Щ�������  ���磺[b1]����,
	{
		if( (g_hTTS->m_nErrorID == emTTS_ERR_OK) )
		{
			QuKongGe( );	
		}
	}

	return g_hTTS->m_nErrorID;
}




//����ֵ :  1:�������ֽڻ�˫�ֽ��Ƿ־��㣻 0:������
emBool emCall IsEndPuncCode(emInt16* pParaEndPuncCodeSet ,	//�ֶα��Ķ�ά���飨��1ά���������ֱ���ı�㣬��2ά��ÿ�����ı���ֵ��
							emInt nByteCount,				//1:����ǵ��ֽڣ� 2�������˫�ֽ�
							emByte nSumOfConverted)			//������ת���˶����ֽ�
{
	emInt8	i, nPuncCodeIndex = 1;
	emInt16 nCurMuteMs;  
	emUInt16 nMaoHaoCode, nWord = 0,nCurPunc;

	emByte cH_DataOfConvert;
	emByte cL_DataOfConvert;

	emByte cH_DataOfBasic;                     
	emByte cL_DataOfBasic;    

	emByte cH_Next;                     
	emByte cL_Next;  
	emBool bRseult;

	LOG_StackAddr(__FUNCTION__);

	if( g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE) 	//����UnicodeСͷ��ʽ
	{
		cH_DataOfBasic =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 1);                     
		cL_DataOfBasic =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData);  

		cH_Next        =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 3);       
		cL_Next        =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 2);       
	}
	else
	{
		cH_DataOfBasic =  *(( emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData);                     
		cL_DataOfBasic =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 1);          

		cH_Next        =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 2);   
		cL_Next        =  *( (emByte *)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData + 3);       
	}


	nWord = ( (emInt16)cH_DataOfBasic << 8 ) + ( (emInt16)cL_DataOfBasic & (emInt16)0x00ff );

	switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
	{
		case emTTS_CODEPAGE_GBK :			nPuncCodeIndex = 1;			break;
		case emTTS_CODEPAGE_GB2312 :		nPuncCodeIndex = 1;			break;
		case emTTS_CODEPAGE_BIG5 :			nPuncCodeIndex = 2;			break;		
		case emTTS_CODEPAGE_UTF16BE :	nPuncCodeIndex = 3;			break;
		case emTTS_CODEPAGE_UTF16LE :	nPuncCodeIndex = 3;			break;
		default:							nPuncCodeIndex = 1;			break;
	}
	if( nByteCount == 1 )		//ASCII�룺��������
	{
		if(    g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16BE
			|| g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE)				
		{
			if( cH_DataOfBasic == 0x00)
				cH_DataOfBasic = cL_DataOfBasic;
			else									//����������Unicode��ASCII������ַ�
			{
				g_hTTS->m_NextMuteMs = 0;
				return FALSE;
			}
		}

		i = 0;
		while(TRUE)
		{
			nCurPunc   = pParaEndPuncCodeSet[2*i];			//��һά�������ʽ������ά����
			nCurMuteMs = pParaEndPuncCodeSet[2*i + 1];		//��һά�������ʽ������ά����

			i++;

			if( nCurPunc == 0x0000 )		//������
				break;

			//����Ƚϵ��ֽ�
			if(  cH_DataOfBasic == (emByte)nCurPunc  && nSumOfConverted >= 2)				//��0ά�洢��ASCII����
			{
				//����ǰASCTT�ֽ�Ϊ����ð�ţ���֮ǰ1����Ϊ���֣����ܵ������ӽ������������磺�ȷ�Ϊ3:2����
				if( cH_DataOfBasic==((emByte)ASCII_MaoHao) )		
				{
					cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 2) ;
					cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 1) ;
					//if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //Ϊ����
					//	break;
					if( cH_DataOfConvert==0xa3 && 
						(cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) || 
						( cL_DataOfBasic>= 0x30 && cL_DataOfBasic<= 0x39 ))//Ϊ����
						break;		//sqb 20140804 modify
					if(  cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xa0 &&  nSumOfConverted >= 4)  //Ϊ�ո�
					{
						cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 4) ;
						cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 3) ;
						if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //Ϊ����
							break;
					}
					if ( cH_DataOfConvert==0xa3 && cL_DataOfBasic == 0x20 &&    //sqb 20140804 modify
						cH_Next >= 0x30 && cH_Next <= 0x39 )
						break;
				}

				//�жϵ�ǰ����ת����ASCII��ġ�.���Ƿ�Ӧ�÷־䣬bResultΪemTrue �־䣬emFalse ���־�
				if( cH_DataOfBasic== ((emByte)ASCII_JuHao)  )		//��ǰASCTT�ֽ�Ϊ����
				{					
					bRseult = IsDotSplitSen(cL_DataOfBasic, cH_Next, cL_Next, nSumOfConverted);
					if( bRseult == emFalse)
						break;
				}


				g_hTTS->m_NextMuteMs = nCurMuteMs;
				return TRUE;
			}	
		}
		g_hTTS->m_NextMuteMs = 0;
		return FALSE;
	}

	if( nByteCount == 2 )		//˫�ֽڣ���������
	{
		i = 0;
		switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
		{
			case emTTS_CODEPAGE_GBK :			nMaoHaoCode = GBK_MaoHao;			break;
			case emTTS_CODEPAGE_GB2312 :		nMaoHaoCode = GBK_MaoHao;			break;
			case emTTS_CODEPAGE_BIG5 :			nMaoHaoCode = BIG5_MaoHao;			break;		
			case emTTS_CODEPAGE_UTF16BE :	nMaoHaoCode = UNICODE_MaoHao;		break;
			case emTTS_CODEPAGE_UTF16LE :	nMaoHaoCode = UNICODE_MaoHao;		break;
			default:							nMaoHaoCode = GBK_MaoHao;			break;
		}
		while(TRUE)
		{
			nCurPunc   = pParaEndPuncCodeSet[nPuncCodeIndex*MAX_END_PUNC*2 + i*2];		//��һά�������ʽ������ά����  
			nCurMuteMs = pParaEndPuncCodeSet[nPuncCodeIndex*MAX_END_PUNC*2 + i*2 + 1];	//��һά�������ʽ������ά����  		

			if(  nWord == nCurPunc )
			{
				//����ǰ˫�ֽ�Ϊ��ð�ţ���֮ǰ1����Ϊ���֣����ܵ������ӽ������������磺ʱ��9:30�֣�
				if( nWord == nMaoHaoCode)
				{
					if( nSumOfConverted >= 2 )
					{
						cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 2) ;
						cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 1) ;
						if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //Ϊ����
						{
							break;
						}

						if(  cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xa0 &&  nSumOfConverted >= 4)  //Ϊ�ո�
						{
							cH_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 4) ;
							cL_DataOfConvert = *(g_hTTS->m_pDataAfterConvert + nSumOfConverted - 3) ;
							if( cH_DataOfConvert==0xa3 && cL_DataOfConvert>=0xb0 && cL_DataOfConvert<=0xb9 ) //Ϊ����
							{
								break;
							}
						}
					}
				}

				g_hTTS->m_NextMuteMs = nCurMuteMs;
				return TRUE;
			}
			i++;
			if( nCurPunc == 0x0000 )
			{
				break;
			}
			
		}
		g_hTTS->m_NextMuteMs = 0;
		return FALSE;
	}

	return TRUE;
}

/*********************************************************/
//����ת��:��һ�ֱ���תΪ��һ�ֱ���
#if 0
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
//����ת���Ӻ�����  GBK����GB2312 --> GBK
//
//���ݣ�	��ԭʼ�ı�����g_hTTS->m_pc_DataOfBasic�ж�ȡ�����ı����ݻ���g_hTTS->m_pDataAfterConvert
//			����������������֮һ���ͽ������α���ת����ͬʱg_hTTS->m_nCurIndexOfBasicDataǰ�Ƽ�¼�Ѿ�ת�����δ���
//				1. ���������������е�һ����������Ϊ������������  ����˫�ֽڱ��͵��ֽڱ�㣩������emTTS_ERR_OK
//				2. g_hTTS->m_pDataAfterConvert�Ѿ������������1�ֽ����������������ΪCORE_CONVERT_BUF_MAX_LEN��������emTTS_ERR_OK
//				3. g_hTTS->m_nCurIndexOfBasicDataָ���Ѿ�ָ����g_hTTS->m_pc_DataOfBasic��β������ȫ��ת������    ������emTTS_ERR_OK
//
//	���õ���ȫ�ֱ�����	g_hTTS->m_pc_DataOfBasic			���Ӳ����л�ȡ��ԭʼ�ı����ݵ�ָ�� //						
//						g_hTTS->m_nCurIndexOfBasicData	����ǰ�����Ļ����ı����ݵ�����λ��
//						g_hTTS->m_pDataAfterConvert		������ת������ı����ݵ�ָ��
//
//	���أ�				������emTTSErrID
//							=emTTS_ERR_OK		����ʾ���α���ת���ɹ���  
//							=emTTS_ERR_END_OF_INPUT		����ʾ���α���ת���������˳�TTS��������ѭ��ת���е����һ�Σ����������ı����ݵ�β��
//
//  �ı��ȫ�ֱ�����	g_hTTS->m_nCurIndexOfBasicData
//
//****************************************************************************************************
emTTSErrID emCall ConvertGbkOrGB2312ToGbk(emInt16* pParaEndPuncCodeSet )
{

	emPByte pDataOfBasic = (emPByte)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData; //����ת����ʼʱԭʼ�ı����ݵ�ָ��
	emPByte pDataAfterConvert = g_hTTS->m_pDataAfterConvert;                          //����ת������ı����ݵ�ָ��
	emByte i;                                                                //ȥ������ת���������ո��ַ�ʱ���õ�ѭ������
	emByte nSumOfConverted = 0;                                              //�Ѵ洢������ת�����ı����������ֽ���
	emBool bDouhao = 0 ;                                                      //��־������flag =1�����Ե��ֽڶ���������ת����flag =0���Ե��ֽڶ��Ž��б���ת�����˳����α���ת�� 
	emTTSErrID  nTypeOfemTTS_ERR;                                            //����ֵ
    //size_t rc = 0;
    //emByte input[255]={0} ;
    //char out[255]={0};

#if EM_ENG_PURE_LOG	
	FILE* pfEnExit;
#endif


	LOG_StackAddr(__FUNCTION__);
/*
    if(emTTS_CODEPAGE_UTF8==1){
        //memcpy(input,pDataOfBasic,g_hTTS->m_nNeedSynSize/2*3);
        //u2g(input,strlen(input),out,255);        //sqb 2017/6/5
        u2g(pDataOfBasic,strlen(pDataOfBasic),out,255);
        pDataOfBasic = out;
    }
*/

	//��ձ���ת������ı�������
	for( i = 0; i < (emByte)CORE_CONVERT_BUF_MAX_LEN; i++ )
	{
		g_hTTS->m_pDataAfterConvert[i]  = 0;
	}

	//ԭʼ�ı������ݣ����α���ת��ʧ�ܣ��˳���������������emTTS_ERR_END_OF_INPUT
	if( g_hTTS->m_nCurIndexOfBasicData >= g_hTTS->m_nNeedSynSize-1)
		return emTTS_ERR_END_OF_INPUT;

	//ԭʼ�ı������ݣ���ʼ����ת��
	do 
	{
		emByte cH_DataOfBasic =  *pDataOfBasic;                           //�ȶ�ȡ��ԭʼ�ı���������һ���ֽ�
		emByte cL_DataOfBasic =  *( pDataOfBasic + 1 );                   //���ȡ��ԭʼ�ı���������һ���ֽ�

        //memcpy(&temp,pDataOfBasic,2);
        //temp=temp<<4;
        //emByte cH_DataOfBasic =  *(pDataOfBasic+1);
        //emByte cL_DataOfBasic =  *pDataOfBasic;

		nSumOfConverted = pDataAfterConvert - g_hTTS->m_pDataAfterConvert ;


		if (( cH_DataOfBasic >= 0x20 && cH_DataOfBasic <= 0x7f) || ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F ))
		{
			//ASCII����߿����ַ�
			if ( IsEndPuncCode( pParaEndPuncCodeSet, 1,nSumOfConverted ) == TRUE )
			{
				//�������ֽڽ������
				if( cH_DataOfBasic == 0x2c )					
				{   //�������ֽڶ���

					bDouhao = QuDouHao( pDataOfBasic, nSumOfConverted, bDouhao);
					if( bDouhao )
					{   //���������������ö���
						pDataOfBasic++;
						g_hTTS->m_nCurIndexOfBasicData++;
						continue;	
					}	
				}
				//�������ֽڷֺš��ʺš�̾�š���źͲ���Ҫ�����Ķ��ż�0xa380ת��Ϊ��Ӧ��˫�ֽڱ�㣬�������α���ת��������emTTS_ERR_OK
				*pDataAfterConvert = 0xa3;
				*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;

				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				nTypeOfemTTS_ERR = emTTS_ERR_OK;
				pDataOfBasic++;
				break;		
			}
			else
			{
				if ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F )
				{
					//ԭʼ�ı�Ϊ�����ַ���ת��Ϊ˫�ֽڿո�
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;

					if ( cH_DataOfBasic == 0x0a )
					{//�������ֽڻ��з�ʱ��ת��Ϊ˫�ֽڿո񣬽������α���ת��������emTTS_ERR_OK  ��Ϊ������
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData++;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic++;
						break;		
					}

				}
				else
				{
					//ԭʼ�ı�ΪASCII�룬��0xA380ת�����ֽڣ�����Ԫ����0x24�Ǽ�0xA1C3��
					if( cH_DataOfBasic == 0x24 )
					{
						*pDataAfterConvert = 0xa1;
						*(pDataAfterConvert + 1) = 0xc3 + cH_DataOfBasic;
					}
					else 
					{
						*pDataAfterConvert = 0xa3;
						*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;						
					}		
				}
                
				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) ) //����������   2012-03-29
				{
					bDouhao = 0;
				}
				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				pDataOfBasic++;		

				//20140218����  ��Ӣ�ĳ����ڵ����ڱ��жϣ���������ĳ���ǰ����������ֻҪ�пո����ǰ������
				if( cH_DataOfBasic == 0x20 && (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) > (CORE_CONVERT_BUF_MAX_LEN - 20) )
				{
#if EM_ENG_PURE_LOG 
					pfEnExit = fopen("log/��־_�ڴ�_�쳣.log","a");
					fprintf(pfEnExit,"������ת��ʱ��--�ո񴦡��������־����󳤶ȣ��ڿո񴦲�֣�ѭ���ϳ�\n");
					fclose(pfEnExit);
#endif
					break;
				}

				continue;  
			}
		}	
		else 
		{
			//if( cH_DataOfBasic == 0xa3 && (cL_DataOfBasic == 0xac || cL_DataOfBasic == 0xbb || cL_DataOfBasic == 0xbf || cL_DataOfBasic == 0xa1) 
			if ( IsEndPuncCode( pParaEndPuncCodeSet, 2 ,nSumOfConverted ) == TRUE )
			{

				//����˫�ֽڽ������(�ֺš�̾�š��ʺ�)����˫�ֽڱ��洢��ת�����������������α���ת��������emTTS_ERR_OK
				*pDataAfterConvert = cH_DataOfBasic;
				*(pDataAfterConvert + 1) = cL_DataOfBasic;

				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData += 2;
				nTypeOfemTTS_ERR = emTTS_ERR_OK;
				pDataOfBasic += 2;
				break;		
			}
			else 
			{
				if( (cH_DataOfBasic >= 0xa1 && cH_DataOfBasic <= 0xa9 && cL_DataOfBasic >= 0x40 && cL_DataOfBasic <= 0xfe) //��������Χ��չ��
					|| ( ((cH_DataOfBasic >= 0x81 && cH_DataOfBasic <= 0xa0) || (cH_DataOfBasic >= 0xaa && cH_DataOfBasic <= 0xfe)) && cL_DataOfBasic >= 0x40 && cL_DataOfBasic <= 0xfe) ) //������
				{					
					*pDataAfterConvert = cH_DataOfBasic;
					*(pDataAfterConvert + 1) = cL_DataOfBasic;
				}
				else
				{
					//����ʶ������ݣ��Ѷ���������ֽ�һ��ת��Ϊ˫�ֽڵĿո�
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
				}

				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //����������  
				{
					bDouhao = 0;
				}
				g_hTTS->m_nCurIndexOfBasicData += 2;
				pDataAfterConvert += 2;
				pDataOfBasic += 2;
				continue;
			}
		}		

	}while(   (g_hTTS->m_nCurIndexOfBasicData < g_hTTS->m_nNeedSynSize)
		   && (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) < (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16) );


	if( (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) > (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16)  )
	{
		;
#if EM_ENG_PURE_LOG 
		pfEnExit = fopen("log/��־_�ڴ�_�쳣.log","a");
		fprintf(pfEnExit,"������ת��ʱ��--��󴦡��������־����󳤶ȣ�����󴦲�֣�ѭ���ϳ�\n");
		fclose(pfEnExit);
#endif
	}

	//��ת��������β������β�ַ�
	*pDataAfterConvert = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );		//�������0xff��Ϊ���Ժ���ж�
	*(pDataAfterConvert + 1) = (emByte)( (emInt16)END_WORD_OF_BUFFER );

	//���ñ���ת�����������	
	//���α���ת���ɹ�
	nTypeOfemTTS_ERR = emTTS_ERR_OK;




	//����ת���ɹ����߽������˳��ú�����������emTTS_ERR_OK������ɹ�����emTTS_ERR_END_OF_INPUT�����������
	return nTypeOfemTTS_ERR;
}
//****************************************************************************************************
//����ת���Ӻ�����  BIG5 --> GBK
//
//����˵������պ���ConvertGbkOrGB2312ToGbk��˵��
//*****************************************************************************************************
emTTSErrID  emCall ConvertBig5ToGbk( emInt16* pParaEndPuncCodeSet )
{
	emPByte pDataOfBasic = (emPByte)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData; //����ת����ʼʱԭʼ�ı����ݵ�ָ��
	emPByte pDataAfterConvert = g_hTTS->m_pDataAfterConvert;                          //����ת������ı����ݵ�ָ��	
	emByte  cSumOfInvalidCode = 0;                                           //��¼ת�������������ո��ַ��ĸ���
	emByte i;                                                            //ȥ������ת���������ո��ַ�ʱ���õ�ѭ������
	emByte nSumOfConverted = 0;                                              //�Ѵ洢������ת�����ı����������ֽ���
	emBool bDouhao = 0;                                                      //��־������flag =1�����Ե��ֽڶ���������ת����flag =0���Ե��ֽڶ��Ž��б���ת�����˳����α���ת�� 
	emByte nSumOfShuZi = 0;                                                  //ԭʼ�ı��������У����ź����������λ��
	emTTSErrID  nTypeOfemTTS_ERR;                                            //�����ķ���ֵ
	emInt16 nWord = 0;                                                       //���ڴ洢�������ı����ݶ�ȡ�������ֽ�
	emInt16 nOffset = 0;                                                     //�������Ǳ����ݶ�Ӧ��GBK������g_Res.offset_CodeBig5ToGbk�е���ƫ����

	LOG_StackAddr(__FUNCTION__);

	//��ձ���ת������ı�������
	for( i = 0; i < (emByte)CORE_CONVERT_BUF_MAX_LEN; i++ )
	{
		g_hTTS->m_pDataAfterConvert[i]  = 0;
	}

	//ԭʼ�ı������ݣ����α���ת��ʧ�ܣ��˳���������������emTTS_ERR_END_OF_INPUT
	if( g_hTTS->m_nCurIndexOfBasicData >= g_hTTS->m_nNeedSynSize)
		return emTTS_ERR_END_OF_INPUT;

	//ԭʼ�ı������ݣ����б���ת������
	do 
	{
		emByte cH_DataOfBasic =  *pDataOfBasic;                           //�ȶ�ȡ��ԭʼ�ı���������һ���ֽ�
		emByte cL_DataOfBasic =  *( pDataOfBasic + 1 );                   //���ȡ��ԭʼ�ı���������һ���ֽ�
		nSumOfConverted = pDataAfterConvert - g_hTTS->m_pDataAfterConvert ;



		//�����ı�������Ҫת��
		if (( cH_DataOfBasic >= 0x20 && cH_DataOfBasic <= 0x7f) || ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F ))
		{
			//�����ı�����Ϊ���ֽ�ASCII��(0x20��0x7f)���߿����ַ�(0x00��0x1f)
			if ( IsEndPuncCode( pParaEndPuncCodeSet, 1 ,nSumOfConverted ) == TRUE )
			{				
				//�������ֽڽ������
				if( cH_DataOfBasic == 0x2c )					
				{   //�������ֽڶ���

					bDouhao = QuDouHao( pDataOfBasic, nSumOfConverted, bDouhao);
					if( bDouhao )
					{   //���������������ö���
						pDataOfBasic++;
						g_hTTS->m_nCurIndexOfBasicData++;
						continue;	
					}	
				}
				//�������ֽڷֺš��ʺš�̾�š���źͲ���Ҫ�����Ķ��ţ���0xa380ת��Ϊ��Ӧ��˫�ֽڱ�㣬�������α���ת��������emTTS_ERR_OK
				*pDataAfterConvert = 0xa3;
				*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;

				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				nTypeOfemTTS_ERR = emTTS_ERR_OK;
				pDataOfBasic++;
				break;		
			}
			else
			{
				if ( cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x1F )
				{
					//�����ı�����Ϊ�����ַ�,ת��Ϊ˫�ֽڿո�0xa3a0
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					if ( cH_DataOfBasic == 0x0a )
					{//�������ֽڻ��з�ʱ��ת��Ϊ˫�ֽڿո񣬽������α���ת��������emTTS_ERR_OK  ��Ϊ������
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData++;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic++;
						break;		
					}
				}
				else
				{
					//�����ı�����ΪASCII�룬��0xA380ת�����ֽڣ�����Ԫ����0x24�Ǽ�0xA1C3��
					if( cH_DataOfBasic == 0x24 )
					{
						*pDataAfterConvert = 0xa1;
						*(pDataAfterConvert + 1) = 0xc3 + cH_DataOfBasic;
					}
					else 
					{
						*pDataAfterConvert = 0xa3;
						*(pDataAfterConvert + 1) = 0x80 + cH_DataOfBasic;						
					}		
				}

				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )   //����������
				{
					bDouhao = 0;
				}
				pDataAfterConvert += 2;
				g_hTTS->m_nCurIndexOfBasicData++;
				pDataOfBasic++;				
				continue;  
			}
		}	
		else 
		{   //�����ı�����Ϊ˫�ֽڷ��Ż���
			if( cH_DataOfBasic >= 0xa1 && cH_DataOfBasic <= 0xf9 )
			{   //�����ı����ݸ��ֽ���BIG5����Ч���뷶Χ��
				nWord = ( (emInt16)cH_DataOfBasic << 8 ) + ( (emInt16)cL_DataOfBasic & (emInt16)0x00ff );
				if ( IsEndPuncCode( pParaEndPuncCodeSet, 2,nSumOfConverted  ) == TRUE )
				{
					//�����ı�����Ϊ˫�ֽڽ�����㣬��˫�ֽڱ��洢��ת�����������������α���ת��������emTTS_ERR_OK
					nOffset = ( cH_DataOfBasic - (emByte) 0xa1 ) * 191 + cL_DataOfBasic - (emByte) 0x40;

					if( g_Res.offset_CodeBig5ToGbk == 0)
					{
						//��û�С�Big5תGBK��������0
						*(pDataAfterConvert) = 0;
						*(pDataAfterConvert+1) = 0;
					}
					else
					{
						fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_CodeBig5ToGbk + nOffset * 2, 0);						
						fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
						fFrontRead(pDataAfterConvert,1,1,g_hTTS->fResFrontMain);
					}

					pDataAfterConvert += 2;
					g_hTTS->m_nCurIndexOfBasicData += 2;
					nTypeOfemTTS_ERR = emTTS_ERR_OK;
					pDataOfBasic += 2;
					break;
				}
				else 
				{   //�����ı�������˫�ֽڷ��Ż��֣��Ҳ��ǽ������
					if( cL_DataOfBasic >= 0x40 && cL_DataOfBasic <= 0xfe )
					{   //�����ı����ݵ��ֽ���BIG5����Ч���뷶Χ��
						nOffset = ( cH_DataOfBasic - (emByte) 0xa1 ) * 191 + cL_DataOfBasic - (emByte) 0x40;

						if( g_Res.offset_CodeBig5ToGbk == 0)
						{
							//��û�С�Big5תGBK��������0
							*(pDataAfterConvert) = 0;
							*(pDataAfterConvert+1) = 0;
						}
						else
						{
							fFrontSeek( g_hTTS->fResFrontMain, g_Res.offset_CodeBig5ToGbk + nOffset * 2, 0);						
							fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
							fFrontRead(pDataAfterConvert,1,1,g_hTTS->fResFrontMain);	
						}
						
						if( *pDataAfterConvert == 0x00 &&  *(pDataAfterConvert + 1) == 0x00 )
						{
							g_hTTS->m_nCurIndexOfBasicData += 2;							
							pDataOfBasic += 2;
							continue;
						}
					}
					else
					{
						//����ʶ������ݣ��Ѷ���������ֽ�һ��ת��Ϊ˫�ֽڵĿո�
						*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
						*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					}

					if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) ) //����������   2012-03-29
					{
						bDouhao = 0;
					}
					g_hTTS->m_nCurIndexOfBasicData += 2;
					pDataAfterConvert += 2;
					pDataOfBasic += 2;
					continue;
				}
			}
			else
			{
				//����ʶ������ݣ��Ѷ���������ֽ�һ��ת��Ϊ˫�ֽڵĿո�
				*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
				*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;

				if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //����������   2012-03-29
				{
					bDouhao = 0;
				}

				g_hTTS->m_nCurIndexOfBasicData += 2;
				pDataAfterConvert += 2;
				pDataOfBasic += 2;
				continue;
			}				
		}

	}while(    (g_hTTS->m_nCurIndexOfBasicData < g_hTTS->m_nNeedSynSize)
		   &&  (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) < (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16) );

	//��ת��������β������β�ַ�
	*pDataAfterConvert = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
	*(pDataAfterConvert + 1) = (emByte)( (emInt16)END_WORD_OF_BUFFER );


	//���α���ת���ɹ�
	nTypeOfemTTS_ERR = emTTS_ERR_OK;



	//����ת���ɹ����߽������˳��ú�����������emTTS_ERR_OK������ɹ�����emTTS_ERR_END_OF_INPUT�����������
	return nTypeOfemTTS_ERR;
}

//****************************************************************************************************
//����ת����  UNICODE --> GBK
//
//���ݣ�	��ԭʼ�ı�����g_hTTS->m_pc_DataOfBasic�ж�ȡ�����ı����ݻ���g_hTTS->m_pDataAfterConvert
//			����������������֮һ���ͽ������α���ת����ͬʱg_hTTS->m_nCurIndexOfBasicDataǰ�Ƽ�¼�Ѿ�ת�����δ���
//				1. ���������������е�һ����������Ϊ������������  ����˫�ֽڱ��͵��ֽڱ�㣩������emTTS_ERR_OK
//				2. g_hTTS->m_pDataAfterConvert�Ѿ������������1�ֽ����������������ΪCORE_CONVERT_BUF_MAX_LEN��������emTTS_ERR_OK
//				3. g_hTTS->m_nCurIndexOfBasicDataָ���Ѿ�ָ����g_hTTS->m_pc_DataOfBasic��β������ȫ��ת������    ������emTTS_ERR_OK
//
//	���õ���ȫ�ֱ�����	g_hTTS->m_pc_DataOfBasic			���Ӳ����л�ȡ��ԭʼ�ı����ݵ�ָ�� //						
//						g_hTTS->m_nCurIndexOfBasicData	����ǰ�����Ļ����ı����ݵ�����λ��
//						g_hTTS->m_pDataAfterConvert		������ת������ı����ݵ�ָ��
//
//	���أ�				������emTTSErrID
//							=emTTS_ERR_OK		����ʾ���α���ת���ɹ���  
//							=emTTS_ERR_END_OF_INPUT		����ʾ���α���ת���������˳�TTS��������ѭ��ת���е����һ�Σ����������ı����ݵ�β��
//
//  �ı��ȫ�ֱ�����	g_hTTS->m_nCurIndexOfBasicData
//
//****************************************************************************************************
emTTSErrID emCall  ConvertUnicodeToGbk(emInt16* pParaEndPuncCodeSet )
{
	emPByte pDataOfBasic = (emPUInt8)g_hTTS->m_pc_DataOfBasic + g_hTTS->m_nCurIndexOfBasicData;//����ת����ʼʱԭʼ�ı����ݵ�ָ��
	emPByte pDataAfterConvert = g_hTTS->m_pDataAfterConvert;                          //����ת������ı����ݵ�ָ��
	emUInt8  cSumOfInvalidCode = 0;                                          //��¼ת�������������ո��ַ��ĸ���
	emUInt8 i;                                                           //ȥ������ת���������ո��ַ�ʱ���õ�ѭ������
	emByte nSumOfConverted = 0;                                              //�Ѵ洢������ת�����ı����������ֽ���
	emBool bDouhao = 0;                                                      //��־������flag =1�����Ե��ֽڶ���������ת����flag =0���Ե��ֽڶ��Ž��б���ת�����˳����α���ת�� 
	emTTSErrID  nTypeOfemTTS_ERR;                                            //����ֵ

	LOG_StackAddr(__FUNCTION__);

	for( i = 0; i < (emUInt8)CORE_CONVERT_BUF_MAX_LEN; i++ )
	{
		g_hTTS->m_pDataAfterConvert[i]  = 0;
	}

	//ԭʼ�ı������ݣ����α���ת��ʧ�ܣ��˳���������������emTTS_ERR_END_OF_INPUT
	if( g_hTTS->m_nCurIndexOfBasicData >= g_hTTS->m_nNeedSynSize)
		return emTTS_ERR_END_OF_INPUT;

	//ԭʼ�ı������ݣ���ʼ����ת��
	do 
	{
		emByte cH_DataOfBasic;
		emByte cL_DataOfBasic;
		if( g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE) 	//����UnicodeСͷ��ʽ
		{
			cL_DataOfBasic =  *pDataOfBasic;                       
			cH_DataOfBasic =  *( pDataOfBasic + 1 );  
		}
		else
		{
			cH_DataOfBasic =  *pDataOfBasic;                       
			cL_DataOfBasic =  *( pDataOfBasic + 1 );               
		}
		

		nSumOfConverted = pDataAfterConvert - g_hTTS->m_pDataAfterConvert ;


		if( cH_DataOfBasic == 0 && cL_DataOfBasic == 0 )		//ԭUnicode����Ϊ0x0000
		{
			//ת��GBK����0xa3a0(�ո�)
			*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
			*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;

			pDataAfterConvert += 2;
			g_hTTS->m_nCurIndexOfBasicData += 2;
			nTypeOfemTTS_ERR = emTTS_ERR_OK;
			pDataOfBasic += 2;
			continue;
		}



		if(  IsEndPuncCode( pParaEndPuncCodeSet, 1 ,nSumOfConverted ) == TRUE ||  IsEndPuncCode( pParaEndPuncCodeSet, 2,nSumOfConverted  ) == TRUE ) 
		{
			//�����㣬
			emUInt16 nWordOfTemp = 0 ;
			emUInt16 nQiShiHangIndex = 0 ;
			emUInt16 nHangIndex = 0 ;


			nWordOfTemp = ((emUInt16)cH_DataOfBasic<<8) + ((emUInt16)cL_DataOfBasic); 			
			
			if( nWordOfTemp == (emUInt)0x002c )
			{   //������Ƕ��ţ�0x002c��������Ƿ����������ö��ŵ������������㣬�������ö��ţ����򣬶ԸĶ��Ž��б���ת�����˳�����ת��
				bDouhao = QuDouHao( pDataOfBasic, nSumOfConverted, bDouhao);
				if( bDouhao )
				{   //���������������ö���
					g_hTTS->m_nCurIndexOfBasicData += 2;
					pDataOfBasic += 2;
					continue;	
				}	
			}

			//�������������ƫ���� = 0xX �C 0x00
            nQiShiHangIndex =  cH_DataOfBasic - (emByte)0x00;
            fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nQiShiHangIndex)*2, 0);

			fFrontRead(&nHangIndex,2,1,g_hTTS->fResFrontMain);
			
			//�����������ڵ�����ƫ���� =  ���������ʼ�к� + 0xY �C 0x00
			
			nHangIndex =  nHangIndex + cL_DataOfBasic - (emByte)0x00; ;//����ƫ����
			
			//��ѯ����תGBK��
			fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nHangIndex-1)*2, 0);
			fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
			fFrontRead(pDataAfterConvert,1,1,g_hTTS->fResFrontMain);

			pDataAfterConvert += 2;
			g_hTTS->m_nCurIndexOfBasicData += 2;
			nTypeOfemTTS_ERR = emTTS_ERR_OK;
			pDataOfBasic += 2;
			break;

		}
		else 
		{
              if ( (cH_DataOfBasic >= 0x00 && cH_DataOfBasic <= 0x3f) || (cH_DataOfBasic >= 0xfe && cH_DataOfBasic <= 0xff) )//����
			  {

				//��ѯ����תGBK��
				emUInt16 nWordOfTemp = 0 ;
				emUInt16 nQiShiHangIndex = 0 ;
				emUInt16 nHangIndex = 0 ;


				//�������������ƫ���� = 0xX �C 0x00
                nQiShiHangIndex =  cH_DataOfBasic - (emByte)0x00;
                fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nQiShiHangIndex)*2, 0);
				fFrontRead(&nHangIndex,2,1,g_hTTS->fResFrontMain);

				if ( nHangIndex == 0 ) //��Ϊ0�������ڲ���ʶ���Unicode��
				{//����һ���ֽ�һ��ת��GBK����0xa3a0(�ո�)
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					if (  cH_DataOfBasic == 0x00 && cL_DataOfBasic == 0x0a )
					{//Ϊ���з�ʱ �������������� 
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData += 2;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic += 2;
						break;
					}
				}
				else
				{
					//�����������ڵ�����ƫ���� =  ���������ʼ�к� + 0xY �C 0x00

					nHangIndex =  nHangIndex + cL_DataOfBasic - (emByte)0x00; ;//����ƫ����

					//��ѯ����תGBK��
					fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeFuhaoUnicodeToGbk + (nHangIndex-1)*2, 0);
					fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
					fFrontRead(pDataAfterConvert ,1,1,g_hTTS->fResFrontMain);
				}
				
				if( *pDataAfterConvert == 0x00 &&  *(pDataAfterConvert + 1) == 0x00 )
				{
					if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) ) //����������   2012-03-29
					{
						bDouhao = 0;
					}

					g_hTTS->m_nCurIndexOfBasicData += 2;			
					pDataOfBasic += 2;
					continue;
				}
			}
			else 
			{
				if( cH_DataOfBasic >= 0x4e && cH_DataOfBasic <= 0x9f )//������
				{
					//��ѯ����תGBK��
					emUInt16 nWordOfTemp = 0 ;
					emUInt16 nHangIndex = 0 ;

					nHangIndex = ((emUInt16)cH_DataOfBasic<<8) + ((emUInt16)cL_DataOfBasic); 
					nHangIndex = nHangIndex - (emUInt16)0x4e00 + 1;	//����ƫ����

					fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_CodeHanziUnicodeToGbk + (nHangIndex-1)*2, 0);
					fFrontRead(pDataAfterConvert + 1,1,1,g_hTTS->fResFrontMain);
					fFrontRead(pDataAfterConvert ,1,1,g_hTTS->fResFrontMain);
					
					if( *pDataAfterConvert == 0x00 &&  *(pDataAfterConvert + 1) == 0x00 )
					{   
						if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //����������   2012-03-29
						{
							bDouhao = 0;
						}
						g_hTTS->m_nCurIndexOfBasicData += 2;			
						pDataOfBasic += 2;
						continue;
					}
				}

				else//��ʶ��
				{
					//����һ���ֽ�һ��ת��GBK����0xa3a0(�ո�)
					*pDataAfterConvert = (emByte)(INVALID_CODE >> 8);
					*(pDataAfterConvert + 1) = (emByte)INVALID_CODE;
					if (  cH_DataOfBasic == 0x00 && cL_DataOfBasic == 0x0a )
					{//Ϊ���з�ʱ �������������� 
						pDataAfterConvert += 2;
						g_hTTS->m_nCurIndexOfBasicData += 2;
						nTypeOfemTTS_ERR = emTTS_ERR_OK;
						pDataOfBasic += 2;
						break;
					}
				}
			}

			if ( (*(pDataAfterConvert + 1) < 0xb0) || (*(pDataAfterConvert + 1) > 0xb9) )  //����������   2012-03-29
			{
				bDouhao = 0;
			}

			g_hTTS->m_nCurIndexOfBasicData += 2;
			pDataAfterConvert += 2;
			pDataOfBasic += 2;
			continue;
		}
	}while(   (g_hTTS->m_nCurIndexOfBasicData < g_hTTS->m_nNeedSynSize)
		   && (pDataAfterConvert - g_hTTS->m_pDataAfterConvert) < (CORE_CONVERT_BUF_MAX_LEN - emResSize_Int16));
	
	//��ת��������β������β�ַ�
	*pDataAfterConvert = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
	*(pDataAfterConvert + 1) = (emByte)( (emInt16)END_WORD_OF_BUFFER );


	//���α���ת���ɹ�
	nTypeOfemTTS_ERR = emTTS_ERR_OK;



	//����ת���ɹ����߽������˳��ú�����������emTTS_ERR_OK������ɹ�����emTTS_ERR_END_OF_INPUT�����������
	return nTypeOfemTTS_ERR;
}

//****************************************************************************************************
// ��Ƕ��Ŵ���    ���ǣ�ǧλ�ָ���
// ���ݣ�   ��ԭʼ�ı�����ָ��pDataOfBasicָ�������Ϊ��Ƕ���ʱ����Զ���ǰ������ݽ����жϡ�
//         ������������ʱ������������ֵΪ1���ڱ���ת�������в��Ըö��Ž���ת����ֱ�Ӵ����ź����һ�ַ���
//         1���ö���Ϊ����ת������ĵ�һ���ַ�
//         2��flag = 1���ö���ǰ��1������Ϊ���֣��Ҷ��ź�Ϊ3λ����
//         3��flag = 0���ö���ǰ��1������Ϊ���֣�����ǰ���4�����Ų�������Ҳ����"-"�����ź���Ϊ3λ����
//         ��������£�����������ֵΪ0.�ڱ���ת�������аѸö�����Ϊ�������Ŵ���
//    
//
//	���õ���ȫ�ֱ����� g_hTTS->m_pDataAfterConvert     ����ת������ı����ݵ�ָ��
//  ���������         pDataOfBasic           ԭʼ����ָ�룬����ǰָ�򶺺����ڵ�ַ
//                     nSumOfConverted        ����ת��������������Ѿ��洢�ı���ת�����ַ���
//                     flag                   ��־���������ڼ�¼ǰһ�������Ƿ�����
//
//	����ֵ��  �ö��Ų����㱻����������ʱ����������0�����򣬷���1
//****************************************************************************************************
emBool  emCall QuDouHao( emPByte pDataOfBasic, emByte nSumOfConverted, emBool flag)
{
	emByte nSumOfShuZi = 0;                                           //��ԭʼ�ı�������ָ�룩pDataOfBasic�У����ź����������λ��
	emByte i;                                                         //����ѭ��

	LOG_StackAddr(__FUNCTION__);

	//������ǰΪ1~3λ���֣����Ҷ��ź����3λ���֣��򽫸ö���ȥ��������ת�������򽫶���ת���󣬽������α���ת��
	switch(flag)
	{
	case 0://���α���ת����һ�δ����ţ���Ҫ��鶺��ǰ�Ƿ�Ϊ1~3λ�������ִ�
		//������ת������ĵ�һ���ַ����Ƕ��ţ��˳���������������ֵ1
		//������ת������ı����Ѵ洢���ַ������ڵ���4���������ת���ı���ָ������λ��ǰ�ĵ�4���ַ��Ƿ�Ϊ���֣�
		//��Ϊ���֣�������ȡ���ŵ��������˳�������������0�����������֣���������һ���ж�

		if(  (nSumOfConverted >= 8) && (CheckDataType( g_hTTS->m_pDataAfterConvert, nSumOfConverted - 8) == (emInt16)DATATYPE_SHUZI
			||  GetWord( g_hTTS->m_pDataAfterConvert, nSumOfConverted - 8) == (emInt16)0xa3ad  ) )		//"��"		//hhh 2014-07-09		[g2]010-82572-234,010-82572-2343
		{
			return 0;
		}	
		if( nSumOfConverted == 0 )
		{
			return 1;
		}
	case 1://��һ�δ������Ҷ���ǰ���4λ�������֣�����ǰһ�������ڱ���ת��ʱ�������������鶺��ǰ1λ�Ƿ�Ϊ����
	default:
		if( (nSumOfConverted >= 2) && (CheckDataType( g_hTTS->m_pDataAfterConvert, nSumOfConverted - 2) == (emInt16)DATATYPE_SHUZI)  )
		{   //����ǰ1λ������,�����ж϶��ź��Ƿ����3λ����	
			switch( g_hTTS->m_ControlSwitch.m_nCodePageType)
			{
			case emTTS_CODEPAGE_GBK :					
			case emTTS_CODEPAGE_GB2312 :
				for( i = 1; ;)
				{   //���㶺�ź����������λ��
					if( *( pDataOfBasic + i ) >= 0x30 && *( pDataOfBasic + i ) <= 0x39 )
					{   //���ֽ�����
						nSumOfShuZi ++;
						i++;								
					}
					else
					{
						if( *( pDataOfBasic + i ) == 0xa3 && ( *( pDataOfBasic + i + 1) >= 0xb0 && *( pDataOfBasic + i + 1) <= 0xb9 ) )
						{   //˫�ֽ�����
							nSumOfShuZi ++;
							i += 2;
						}
						else 
						{
							break;
						}							
					}
				}	
				break;
			case emTTS_CODEPAGE_BIG5 :
				for( i = 1; ;)
				{   //���㶺�ź����������λ��
					if( *( pDataOfBasic + i ) >= 0x30 && *( pDataOfBasic + i ) <= 0x39 )
					{   //���ֽ�����
						nSumOfShuZi ++;
						i++;								
					}
					else
					{
						if( *( pDataOfBasic + i ) == 0xa2 && ( *( pDataOfBasic + i + 1) >= 0xaf && *( pDataOfBasic + i + 1) <= 0xb8 ) )
						{   //˫�ֽ�����
							nSumOfShuZi ++;
							i += 2;
						}
						else 
						{
							break;
						}							
					}
				}	
				break;
			case emTTS_CODEPAGE_UTF16BE :
				for( i = 2; ;i += 2)
				{
					if( *( pDataOfBasic + i ) == 0x00 && ( *( pDataOfBasic + i + 1) >= 0x30 && *( pDataOfBasic + i + 1) <= 0x39 ) )
					{   //˫�ֽ�����
						nSumOfShuZi ++;						
					}
					else 
					{
						if( *( pDataOfBasic + i ) == 0xff && ( *( pDataOfBasic + i + 1) >= 0x10 && *( pDataOfBasic + i + 1) <= 0x19 ) )
						{   //˫�ֽ�����
							nSumOfShuZi ++;

						}
						else 
						{
							break;
						}	
					}
				}
				break;	
			case emTTS_CODEPAGE_UTF16LE :
				for( i = 2; ;i += 2)
				{
					if( *( pDataOfBasic + i + 1) == 0x00 && ( *( pDataOfBasic + i) >= 0x30 && *( pDataOfBasic + i) <= 0x39 ) )
					{   //˫�ֽ�����
						nSumOfShuZi ++;						
					}
					else 
					{
						if( *( pDataOfBasic + i + 1) == 0xff && ( *( pDataOfBasic + i) >= 0x10 && *( pDataOfBasic + i) <= 0x19 ) )
						{   //˫�ֽ�����
							nSumOfShuZi ++;
						}
						else 
						{
							break;
						}	
					}
				}
				break;	
			}			
			if( nSumOfShuZi == 3)
			{   //���ź����3λ���֣��˳�������������1
				return 1;
			}
			else 
			{   //���ź�����������ָ�����Ϊ3���˳�������������0
				return 0;
			}
		}
		else 
		{  //����ǰ1λ�������֣��˳�������������0
			return 0;
		}

	}

}

//****************************************************************************************************
// ȥ�ո���  
// ���ݣ�  ������˳����������ת�������������������Ŀո�
//         1. ���ȣ����������ո񴮣�ֻ����һ���ո��ַ���
//         2. Ȼ�����ո�ǰ����������Ͳ�ͬ�ࣨ��ǰ�����ֺ�����ĸ���⣩���������ո�
//	���õ���ȫ�ֱ����� g_hTTS->m_pDataAfterConvert     ����ת������ı����ݵ�ָ��
//  �ı��ȫ�ֱ�����   g_hTTS->m_pDataAfterConvert
//	����ֵ��  �޷���ֵ
//****************************************************************************************************

void emCall	QuKongGe( )
{
	emByte  cSumOfInvalidCode = 0;                                           //��¼ת�������������ո��ַ��ĸ���
	emByte i,j,k;                                                            //ȥ������ת���������ո��ַ�ʱ���õ�ѭ������
	emInt16	nPreType,nNextType;
	emBool	bTypeQu = emFalse;

	LOG_StackAddr(__FUNCTION__);

	//���������ո񴮣�ֻ����һ���ո�
	for( i = 0; g_hTTS->m_pDataAfterConvert[i] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[i + 1] != (emByte)END_WORD_OF_BUFFER; i += 2 )
	{

		//��¼�����ո���Ŀ  //GBK�ո�  ��  0xa1a1 �� 0xa3a0 
		if( ((g_hTTS->m_pDataAfterConvert[i] == (emByte)(INVALID_CODE >> 8)) && (g_hTTS->m_pDataAfterConvert[i + 1] == (emByte)INVALID_CODE) )
			|| ((g_hTTS->m_pDataAfterConvert[i] == (emByte)(0xa1a1 >> 8)) && (g_hTTS->m_pDataAfterConvert[i + 1] == (emByte)0xa1a1) ) )
		{
			cSumOfInvalidCode += 0x01;
			continue;
		}
		//���������ո�󣬱�����һ���ո�Ȼ�������ո����������ӵ������Ŀո�ĺ��棬�����ո���Ŀ����
		if( cSumOfInvalidCode >= (emInt8)2 || cSumOfInvalidCode*2 == i )
		{
			if( cSumOfInvalidCode*2 == i )		//���׵Ŀո�ȫ��ȥ��
				k = cSumOfInvalidCode * 2;			
			else								//�Ǿ��׵Ŀո�ֻ����1��
				k = (cSumOfInvalidCode - 1) * 2;

			for( j = i - k; g_hTTS->m_pDataAfterConvert[j + k] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[j + k + 1] != (emByte)END_WORD_OF_BUFFER; j += 2 )
			{
				g_hTTS->m_pDataAfterConvert[j] = g_hTTS->m_pDataAfterConvert[j + k];
				g_hTTS->m_pDataAfterConvert[j + 1] = g_hTTS->m_pDataAfterConvert[j + k + 1];	
			}
			g_hTTS->m_pDataAfterConvert[j ] = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
			g_hTTS->m_pDataAfterConvert[j + 1] = (emByte)( (emInt16)END_WORD_OF_BUFFER );
			i = i - k;
			cSumOfInvalidCode = 0;
		}
		else
		{
			cSumOfInvalidCode = 0;
		}
	}
	//�������ո�ͽ����ַ�֮�����������ݣ����������ո�ĵڶ����ո�����β�ַ�
	if( cSumOfInvalidCode >= (emInt8)2 )
	{
		k = (cSumOfInvalidCode - 1) * 2;
		g_hTTS->m_pDataAfterConvert[i - k ] = (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 );
		g_hTTS->m_pDataAfterConvert[i - k + 1] = (emByte)( (emInt16)END_WORD_OF_BUFFER );
	} 

	//���ո�ǰ����������Ͳ�ͬ�ࣨ����ǰ�����ֺ�����ĸ���⣬ǰ����ĸ�������ֳ��⣩��ǰ���Ǻ��֣��򽫿ո�ȥ��  //GBK�ո�  ��  0xa1a1 �� 0xa3a0 
	//���磺V60  V66 T720��50cm 50kg 50V��
	for( i = 2; g_hTTS->m_pDataAfterConvert[i + 2] != (emByte)(END_WORD_OF_BUFFER >> 8) || g_hTTS->m_pDataAfterConvert[i + 3] != (emByte)END_WORD_OF_BUFFER; i += 2 )
	{
		bTypeQu = emFalse;
		nPreType = CheckDataType( g_hTTS->m_pDataAfterConvert, i - 2);
		nNextType = CheckDataType( g_hTTS->m_pDataAfterConvert, i + 2);
		/*
		if( nPreType != nNextType   || (nPreType==DATATYPE_HANZI && nNextType==DATATYPE_HANZI))
		{
			if(    nPreType == DATATYPE_SHUZI   && nNextType == DATATYPE_YINGWEN		//ǰ�����ֺ�����ĸ
				|| nPreType == DATATYPE_YINGWEN && nNextType == DATATYPE_SHUZI )	//ǰ����ĸ��������
			{
				bTypeQu = emFalse;
			}
			else
			{
				bTypeQu = emTrue;
			}

		}
		*/
		

		// fhy 130105 ֻȥ���������ҵĿո�
		if( nPreType==DATATYPE_HANZI || nNextType==DATATYPE_HANZI)
			bTypeQu = emTrue;
		else
			bTypeQu = emFalse;

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



//����ת���� ��g_hTTS->m_pDataAfterConvert������ݽ���
void emCall		FanToJian()		
{
	emInt32 i,nRowNum;
	emUInt16 nJianCode;

	LOG_StackAddr(__FUNCTION__);

	i = 0;
	while( *(g_hTTS->m_pDataAfterConvert + 2 * i) != (emByte)( (emInt16)END_WORD_OF_BUFFER >> 8 ) )
	{
		nRowNum = (*(g_hTTS->m_pDataAfterConvert + 2 * i) - 0x81) * 191 +  *(g_hTTS->m_pDataAfterConvert + 2 * i + 1) - 0x40; //�����ƫ������

		fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_FanToJian + nRowNum * 2, 0);  //��λ
		fFrontRead(&nJianCode, 2, 1, g_hTTS->fResFrontMain);  //��ȡ
		if( nJianCode != 0)
		{
			*(g_hTTS->m_pDataAfterConvert + 2 * i)  = nJianCode>>8;
			*(g_hTTS->m_pDataAfterConvert + 2 * i +1 )  = nJianCode&0xff;
		}
		i++;
	}
}




emUInt8 emCall	CheckInfo()
{

	emInt16 nTotalLen,i;
	emUInt nReturn = 1;

	#define INFO_VERSION_LEN		40			//Ԥ�����Ű汾�ŵ��ֽ�

	if( emStrLenA(g_hTTS->m_pDataAfterConvert) == (COMPANY_INFO_NEED_COMPARE_LEN+2) )
	{
		emMemSet(g_hTTS->m_Info ,0, MAX_COMPANY_INFO_LEN);

		fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_CompanyInfo ,0);
		fFrontRead(&nTotalLen, 2, 1, g_hTTS->fResFrontMain);
		if( nTotalLen >= (MAX_COMPANY_INFO_LEN-INFO_VERSION_LEN))
			nTotalLen = MAX_COMPANY_INFO_LEN - INFO_VERSION_LEN;		
		fFrontRead(g_hTTS->m_Info, 1, nTotalLen, g_hTTS->fResFrontMain);

		for( i=0; i<MAX_COMPANY_INFO_LEN; i++)
		{
			if(  g_hTTS->m_Info[i] != 0)
				g_hTTS->m_Info[i] ^= 0x55;
			else
				break;
		}
		//��̶��ı�ƥ����
		if( !emMemCmp(g_hTTS->m_pDataAfterConvert, g_hTTS->m_Info, COMPANY_INFO_NEED_COMPARE_LEN) )
		{
			//���ϣ����Ű汾�ţ��Զ���ȡ�汾�ţ�
			emMemCpy(g_hTTS->m_Info+nTotalLen,"�汾�ţ�",8);
			emMemCpy(g_hTTS->m_Info+nTotalLen+8,EM_VERSION,sizeof(EM_VERSION));
			nTotalLen += (sizeof(EM_VERSION) + 8);

			g_hTTS->m_pc_DataOfBasic = g_hTTS->m_Info;
			nReturn = 2;
			g_hTTS->m_nNeedSynSize = nTotalLen ;
		}
	}

	return nReturn;

}


void emCall	 GetChaBoText()
{
	emInt16 i,nCount;
	emUInt8 nByte, nMod;
	emInt16 nOffset,nLen;

	emMemSet(g_hTTS->m_Info ,0, MAX_COMPANY_INFO_LEN);	

	fFrontSeek(g_hTTS->fResFrontMain,g_Res.offset_InsertPlayInfo,0);
	fFrontRead(&nCount, sizeof(emInt16), 1, g_hTTS->fResFrontMain);

	//ͨ���Ե�2�ֽ����������õ�Ҫ�岥���ǵڼ����岥��Ϣ
	nByte = *(g_hTTS->m_pDataAfterConvert+1);
	nMod = nByte - nByte/nCount*nCount;		//ȡ��

	fFrontSeek(g_hTTS->fResFrontMain, nMod * 4 ,1);
	fFrontRead(&nOffset, sizeof(emInt16), 1, g_hTTS->fResFrontMain);
	fFrontRead(&nLen, sizeof(emInt16), 1, g_hTTS->fResFrontMain);

	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_InsertPlayInfo+nOffset ,0);
	fFrontRead(g_hTTS->m_Info, nLen, 1, g_hTTS->fResFrontMain);

	for( i=0; i<nLen; i++)
		g_hTTS->m_Info[i] ^= 0x55;

	g_hTTS->m_Info[i] =   0xa1;
	g_hTTS->m_Info[i+1] = 0xa3;
	g_hTTS->m_Info[i+2] = 0xff;
	g_hTTS->m_Info[i+3] = 0xff;
	g_hTTS->m_Info[i+4] = 0x00;
	g_hTTS->m_Info[i+5] = 0x00;

	emMemCpy(g_hTTS->m_pDataAfterConvert, g_hTTS->m_Info,i+6);

}

//�жϵ�ǰ����ת����ASCII��ġ�.���Ƿ�Ӧ�÷־�
//����ֵ�� emFalse: ���־䣻  emTrue: �־䣻
emBool emCall IsDotSplitSen(emByte cL_DataOfBasic,
							emByte cH_Next, 
							emByte cL_Next, 
							emByte nSumOfConverted)
{
	emPByte cP1_DataOfConvert;	// ת��������
	emBool bResult = emTrue;
	emBool bUcode = emFalse;
	emInt16 i,j;
#define LEN 26
	emInt16 AbbrDot[LEN][4] = {
		{0xa3cd, 0xa3f2},	// Mr
		{0xa3cd, 0xa3f3},	// Ms
		{0xa3c4, 0xa3f2},	// Dr
		{0xa3ce, 0xa3ef},	// No
		{0xa3ca, 0xa3f2},	// Jr
		{0xa3d3, 0xa3f2},   //Sr	//gaogao 2014.7.5 modify
		{0xa3d3, 0xa3f4},	// St
		{0xa3c7, 0xa3e5, 0xa3ee},	// Gen
		{0xa3cd, 0xa3f2, 0xa3f3},	// Mrs
		{0xa3ed, 0xa3e7, 0xa3f2},	// mgr
		{0xa3c1, 0xa3f0, 0xa3f0},	// App
		{0xa3ca, 0xa3e1, 0xa3ee},	// Jan
		{0xa3c6, 0xa3e5, 0xa3e2},	// Feb
		{0xa3cd, 0xa3e1, 0xa3f2},	// Mar
		{0xa3c1, 0xa3f0, 0xa3f2},	// Apr
		{0xa3ca, 0xa3f5, 0xa3ee},	// Jun
		{0xa3ca, 0xa3f5, 0xa3ec},	// Jul
		{0xa3c1, 0xa3f5, 0xa3e7},	// Aug
		{0xa3d3, 0xa3e5, 0xa3f0},	// Sep
		{0xa3cf, 0xa3e3, 0xa3f4},	// Oct
		{0xa3ce, 0xaeef, 0xa3f6},	// Nov
		{0xa3c4, 0xa3e5, 0xa3e3},	// Dec
		{0xa3c1, 0xa3e3, 0xa3f4, 0xa3f6},	// Actv
		{0xa3c3, 0xa3ef, 0xa3ee, 0xa3f4},	// Cont
		{0xa3d0, 0xa3f2, 0xa3ef, 0xa3e6},	//Prof
	};


	LOG_StackAddr(__FUNCTION__);
	

	cP1_DataOfConvert = g_hTTS->m_pDataAfterConvert + nSumOfConverted - 2;
	
	if(g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16BE
		|| g_hTTS->m_ControlSwitch.m_nCodePageType == emTTS_CODEPAGE_UTF16LE)
		bUcode = emTrue;

	// ��.������Ϊ�ո��Tab��
	if((bUcode && cH_Next == 0x00 && (cL_Next == SPACE_CODE || cL_Next == TAB_CODE))
		|| (!bUcode && (cL_DataOfBasic == SPACE_CODE || cL_DataOfBasic == TAB_CODE)))
	{
		emInt16 t;

		//ǰΪ����������д�ģ����־䡣  ��Mr.  Dr.  Gen.  Mrs.  Prof. ��  ���磺Mr. Andrew    Dr. Mathew    Gen. Philips    Mrs. Hellen    Prof. Adams
		for(i=LEN-1,j=strlen(AbbrDot[0])-1; i>=0; i--,j=strlen(AbbrDot[0])-1)
		{
			while(!AbbrDot[i][j]) j--;
			if(j >= (short)(nSumOfConverted/2)) continue;
			t=0;
			while(j >= 0)
			{
				if(GetWord(cP1_DataOfConvert,t) == AbbrDot[i][j])
				{
					if(j==0)
					{
						if((nSumOfConverted/2 + t-1 >= 0) && (CheckDataType(cP1_DataOfConvert,t-2) == (emInt16)DATATYPE_YINGWEN))
							break;
						else
							return emFalse;
					}
					t -= 2;
					j--;
				}else
					break;
			}
		}

		if((CheckDataType(cP1_DataOfConvert,0) == (emInt16)DATATYPE_YINGWEN)	//gaogao 2014.7.2 modify 
			&& (CheckDataType(cP1_DataOfConvert,-2) != (emInt16)DATATYPE_YINGWEN) 
			&& (CheckDataType(cP1_DataOfConvert,-4) == (emInt16)DATATYPE_YINGWEN)
			&& (CheckDataType(cP1_DataOfConvert,-6) != (emInt16)DATATYPE_YINGWEN))
		{
			return emFalse;		//���־䣺 ǰ������Ϊ1����ĸ���������д�ĳ�������Ϊ2����ĸʱ�����־䡣 ���磺W.C. C.V. M.A. i.e. e.g. S.O.S.  1520 B.C. A.C.  3:00 a.m.    2:00 p.m.   3:00 a.m.   2:00 p.m.	
		}
		else
		{
			//���࣬�־�		���磺 a.  b.  c. you are there. you. are. there. I am here.  158.   Ph.D.	filename.txt	http://mail.163.com	my_name@example.com 3.5kg.  123.23.   ��.��kg.  ������.����.  it is 3.it is 4.
			//                         This is W.C. I come to the U.S.
			return emTrue;		
		}
	
	}
	else			
	{
		/*
		// �������޿ո��Tab�������
		// aisound�� 1.�����ŵ㲻�������ڵ������ַ����е����һ����ʱ�� �򲻷־�
		//			 2.�����ŵ�Ϊ�����ַ���������һ����ʱ�� ��������Ĳ���com��cn���򲻷־��ҵ����dot
		//			 3.����,�־䲻����
		if((CheckDataType(cP1_DataOfConvert,0) == (emInt16)DATATYPE_YINGWEN)	// ǰ������Ϊ1����ĸ��ʱ�����־䡣 ���磺W.C.   C.V.   M.A.	  i.e.   e.g.	  S.O.S.  1520 B.C.    A.C. 158.    3:00 a.m.    2:00 p.m.   3:00 a.m.   2:00 p.m.	��error���� Ph.D.	filename.txt	http://mail.163.com	my_name@example.com
			&& (CheckDataType(cP1_DataOfConvert,-2) != (emInt16)DATATYPE_YINGWEN))
		{
			bResult = emFalse;
		}
		else if(CheckDataType(cP1_DataOfConvert,0) == (emInt16)DATATYPE_SHUZI)	// ǰ��Ϊ����ʱ�����־䡣���磺3.5kg.  123.23.   ��.��kg.  ������.����.  ��error����it is 3.it is 4.
		{
			bResult = emFalse;
		}
		else if((g_hTTS->m_bConvertWWW == emTrue)								// ֮ǰ���ֹ�www���򲻷־䡣��ע�⣺�����Сд�����磺WWW.sohu.com.cn��wWw.tTs168.net.cn��WwW.263.org.cn��www.SOHU.gov.cn��
				|| ((nSumOfConverted/2 >= 3) &&
					(((GetWord(cP1_DataOfConvert,0) == (emInt16)0xa3f7) ||
					(GetWord(cP1_DataOfConvert,0) == (emInt16)0xa3d7))
					&& ((GetWord(cP1_DataOfConvert,-2) == (emInt16)0xa3f7) ||
					(GetWord(cP1_DataOfConvert,-2) == (emInt16)0xa3d7))
					&& ((GetWord(cP1_DataOfConvert,-4) == (emInt16)0xa3f7) ||
					(GetWord(cP1_DataOfConvert,-4) == (emInt16)0xa3d7)))))
		{
			if((nSumOfConverted/2 > 3) && (CheckDataType(cP1_DataOfConvert,-6) == (emInt16)DATATYPE_YINGWEN))	//awww.123.cn
			{
				bResult = emTrue;
			}
			else		//www.123.cn 1www.123.cn #www.123.cn
			{
				bResult = emFalse;
 				g_hTTS->m_bConvertWWW = emTrue;
			}
		}
		else																	//��������¶��־䡣���磺ab.ab�� ��ӭ.���١�you.are.pic. see.you��Tom is here.you are there. it is ok.23 is ok.	
			bResult = emTrue;

			*/

		bResult = emFalse;			//���治�ǿո��tab������һ�ɲ��־�

		return bResult;
	}
}
	
