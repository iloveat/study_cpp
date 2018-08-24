//emTTS�ں�ͷ�ļ�

#ifndef	_EMTTS__EM_API_KERNEL__H_
#define _EMTTS__EM_API_KERNEL__H_


#include "emApiDefine.h"

#ifdef  __cplusplus
extern "C" {
#endif


	typedef emUInt16 emTTSErrID;
	typedef emPointer emHTTS;


	//���ӿں�����ء�*************************************************************************

	//����һ��ʵ��
	emTTSErrID emCall emTTS_Create(
		emHTTS emPtr	phTTS,							//�����ʵ����� 
		emPointer		pHeap,							//���룺ʵ�����õ��ڴ��ָ��
		emSize			nHeapSize,						//���룺ʵ�����õ��ڴ�Ѵ�С
		emPointer		pCBParam,						//���룺�û��ص�ָ��
		emPResPackDesc	pResPackDesc,					//���룺��Դ����������
		emSize			nResPackCount);					//���룺��Դ������


	//��������ʵ��
	emTTSErrID emCall emTTS_Destroy(
		emHTTS			hTTS);							//���룺ʵ�����

	//��ȡʵ���Ĳ���
	emTTSErrID emCall emTTS_GetParam(
		emHTTS			hTTS,							//���룺ʵ�����
		emUInt32		nParamID,						//���룺����ID 
		emPUInt32		pnParamValue);					//���������ֵ 

	//����ʵ���Ĳ���
	emTTSErrID emCall emTTS_SetParam(
		emHTTS			hTTS,							//���룺ʵ�����
		emUInt32		nParamID,						//���룺����ID
		emUInt32		nParamValue);					//���룺����ֵ

	//����ʵ����TTS�ϳɣ������е�emTTS_SynthText�����������˳�
	emTTSErrID emCall emTTS_Exit(
		emHTTS			hTTS);							//���룺ʵ�����

	//�ڵ�ǰʵ���кϳ�һ���ı�����ǰ�̻߳ᱻ������ֱ���ϳɽ���
	emTTSErrID emCall emTTS_SynthText(
		emHTTS			hTTS,							//���룺ʵ�����
		emCPointer		pcData,							//���룺���ϳ��ı�������bufferָ��
		emSize			nSize);						//���룺���ϳ��ı������ݴ�С

	//��ȡemTTS�ں˵İ汾
	emTTSErrID emCall emTTS_GetVersion(
		emPByte verStr);								//������汾���ַ���




	//�����صģ������룺������******************************************************************

#define emTTS_ERR_OK					0x0000						//�ɹ� 
#define emTTS_ERR_FAILED				0xFFFF						//ʧ�� 
#define emTTS_ERR_END_OF_INPUT			0x0001						//���������� 
#define emTTS_ERR_EXIT					0x0002						//�˳�TTS 

#define emTTS_ERR_BASE					0x8000						//������������ 
#define emTTS_ERR_INSUFFICIENT_HEAP		(emTTS_ERR_BASE + 1)		//�ѿռ䲻��  
#define emTTS_ERR_RESOURCE				(emTTS_ERR_BASE + 2)		//��Դ���� 




	//��ʵ���������룺������*********************************************************************


#define emTTS_PARAM_OUTPUT_CALLBACK		0x00000101			//����ص����� 
#define emTTS_PARAM_PROGRESS_CALLBACK	0x00000102			//�������֪ͨ�ص��������� 

#define emTTS_PARAM_INPUT_CODEPAGE		0x00000201			//�����ı�����ҳ���� 

#define emTTS_PARAM_USE_PROMPTS			0x00000301			//[x*]����ʾ������ 
#define emTTS_PARAM_READ_DIGIT			0x00000302			//[n*]�����ֶ�������  
#define emTTS_PARAM_CHINESE_NUMBER_1	0x00000303			//[y*]�����ĺ��롰1���Ķ������� 
#define emTTS_PARAM_SPEAK_STYLE			0x00000304			//[f*]������������ 
#define emTTS_PARAM_VOICE_SPEED			0x00000305			//[s*]�����ٲ��� 
#define emTTS_PARAM_VOLUME				0x00000306			//[v*]���������� 
#define emTTS_PARAM_VOICE_PITCH			0x00000308			//[t*]��������� 
#define emTTS_PARAM_PUNCTUATION			0x00000307			//[b*]���������� 

#define emTTS_PARAM_ROLE				0x00000309			//[m*]�������˲���				
#define emTTS_PARAM_ManualRhythm		0x0000030a			//[z*]�����ɲ��Բ��� 
#define emTTS_PARAM_PINYIN				0x0000030b			//[i*]��ʶ��ƴ������ 
#define emTTS_PARAM_XINGSHI				0x0000030c			//[r*]�����ϲ��� 

#define emTTS_PARAM_LANG				0x0000030d			//[g*]������
#define	emTTS_PARAM_ZERO				0x0000030e			//[o*]��Ӣ��0�Ķ���






	// ������ҳ��ء�*****************************************************************************
#define emTTS_CODEPAGE_UTF8             1                     //�ı�����ҳ��UTF-8
#define emTTS_CODEPAGE_GBK				936						//�ı�����ҳ��GBK (Ĭ��) 
#define emTTS_CODEPAGE_GB2312			937						//�ı�����ҳ��GB2312 
#define emTTS_CODEPAGE_BIG5				950						//�ı�����ҳ��Big5 
#define emTTS_CODEPAGE_UTF16LE			1200					//�ı�����ҳ��UTF-16 little-endian 
#define emTTS_CODEPAGE_UTF16BE			1201					//�ı�����ҳ��UTF-16 big-endian 
#define emTTS_CODEPAGE_UNICODE			emTTS_CODEPAGE_UTF16
#if EM_BIG_ENDIAN
#define emTTS_CODEPAGE_UTF16		emTTS_CODEPAGE_UTF16BE
#else
#define emTTS_CODEPAGE_UTF16		emTTS_CODEPAGE_UTF16LE
#endif




	// �����Ʊ�ǣ�������������*******************************************************************

#define emTTS_READDIGIT_AUTO				0			//���ֶ������Զ���Ĭ�ϣ� 
#define emTTS_READDIGIT_AS_NUMBER			1			//���ֶ����������� 
#define emTTS_READDIGIT_AS_VALUE			2			//���ֶ���������ֵ 

#define emTTS_CHNUM1_READ_YAO				0			//���롰1�������ɡ��ۡ� (Ĭ��) 
#define emTTS_CHNUM1_READ_YI				1			//���롰1�������ɡ�һ�� 

#define emTTS_STYLE_WORD					0			//�������word-by-word��� 
#define emTTS_STYLE_NORMAL					1			//�������������Ȼ��� (Ĭ��) 

#define emTTS_PITCH_MIN						0			//��������ֵ 
#define emTTS_PITCH_NORMAL					5			//���������ֵ5��Ĭ�ϣ� 
#define emTTS_PITCH_MAX						10			//��������ֵ 

#define emTTS_SPEED_MIN						0			//���٣�����ֵ 
#define emTTS_SPEED_NORMAL					5			//���٣�����ֵ5��Ĭ�ϣ� 
#define emTTS_SPEED_MAX						10			//���٣����ֵ 

#define emTTS_VOLUME_MIN					0			//��������Сֵ 
#define emTTS_VOLUME_NORMAL					5			//����������ֵ10��Ĭ�ϣ� 
#define emTTS_VOLUME_MAX					10			//���������ֵ 

#define emTTS_PUNCTUATION_NO_READ			0			//����㣺����(Ĭ��) 
#define emTTS_PUNCTUATION_READ				1			//����㣺��  

#define emTTS_USE_PROMPTS_CLOSE				0			//��ʾ��: ��ʶ�� 
#define emTTS_USE_PROMPTS_OPEN				1			//��ʾ��: ʶ��Ĭ�ϣ� 

#define emTTS_USE_XINGSHI_CLOSE				0			//���Ͽ���: �ر� ��Ĭ�ϣ�
#define emTTS_USE_XINGSHI_JUSHOU_OPEN		1			//���Ͽ���: �Ժ�ÿ��ľ���ǿ�ƶ������� ����
#define emTTS_USE_XINGSHI_AFTER_OPEN		2			//���Ͽ���: ��������ľ���ǿ�ƶ������� ����

#define emTTS_USE_PINYIN_CLOSE				0			//ʶ����ƴ������: �رգ�Ĭ�ϣ�
#define emTTS_USE_PINYIN_OPEN				1			//ʶ����ƴ������: ��

#define	emTTS_USE_ROLE_XIAOLIN				3			//�����ˣ�����    ��Ů������Ĭ�ϣ�
#define	emTTS_USE_ROLE_Virtual_51			51			//�����ˣ���С��  ��������
#define	emTTS_USE_ROLE_Virtual_52			52			//�����ˣ���Сǿ  ��������
#define	emTTS_USE_ROLE_Virtual_53			53			//�����ˣ�������  ��Ů����
#define	emTTS_USE_ROLE_Virtual_54			54			//�����ˣ�����Ѽ  ��Ч������
#define	emTTS_USE_ROLE_Virtual_55			55			//�����ˣ�С����  ��Ůͯ����

#define emTTS_USE_Manual_Rhythm_CLOSE		0			//�ر����ɱ�ʶ ������ԣ�Ĭ�ϣ�
#define emTTS_USE_Manual_Rhythm_OPEN		1			//�������ɱ�ʶ �������

#define emTTS_LANG_AUTO						0			//���֣��Զ�ʶ��Ĭ�ϣ�
#define emTTS_LANG_CN						1			//���֣�����
#define emTTS_LANG_EN						2			//���֣�Ӣ��

#define emTTS_EN_0_OU						0			//Ӣ��0�Ķ��������ɡ�ou��
#define emTTS_EN_0_ZERO						1			//Ӣ��0�Ķ��������ɡ�zero����Ĭ�ϣ�

	//Zz	20130313
#define emTTS_EN_TIME_NORM					0			//Ӣ��ʱ���������ͨ	����ֵ���ζ���Ĭ�ϣ�
#define emTTS_EN_TIME_SPEC					1			//Ӣ��ʱ��������ر�	��������ʽ��




	// ��PCM���ݲ����ʣ�������������*****************************************************************

#define emTTS_CODE_PCM_8K					8000		//PCM���ݣ�8K ��16bit����ͨ�� 
#define emTTS_CODE_PCM_11K					11025		//PCM���ݣ�11K��16bit����ͨ��
#define emTTS_CODE_PCM_16K					16000		//PCM���ݣ�16K��16bit����ͨ��



#ifdef __cplusplus
}
#endif

#endif	//#define _EMTTS__EM_API_KERNEL__H_
