/* �����ļ� */
#ifndef	_EMTTS__EM_CONFIG__H_
#define _EMTTS__EM_CONFIG__H_


#define ZIMU_READ_AS_CN_MODEL					1			//���ľ��е�������ĸ������0������Ӣ�����⣻1�������������⣩


//																�ڴ�+Cache	��Դ		 ����+ǰ�˳���		ջ
//ARM7 MTK104���� �� 8Kģ�� + 8K��Cache + ��˳��� + ǰ���ļ�	40K+8K		1.5M		 0.3M+0.5M			4.5K
//ARM9 WinCE ���� ��16Kģ�� + 8K��Cache + ��˳��� + ǰ���ļ�	64K+8K		1.5M		 0.3M+1.2M			4.5K
//ARM9 Linux ���� ��16Kģ�� + ȫ���ļ�							64K			2.7M		 0.3M     			4.5K

// 8Kģ������  ��4.5Kջ��40K�ѣ���֧����Чģʽ(��С)�������ϳɵ��������Ϊ50��ֻ֧�ַ�����[m3] [m51] [m52]
//16Kģ������  ��4.5Kջ��64K�ѣ���֧����Чģʽ��      �����ϳɵ��������Ϊ80�����з����˶�֧��    

//====================================		�汾+����+��Ч+�û���Ϣ	  =================================================================

//ע�⣺�û���Ϣ������Դ�����ã�������������Դ��

#define EM_VERSION						"emTTS soft V3.40"		//�汾��
#define EM_TRIAL								0				//�Ƿ����ð� 	
#define EM_USER_SWITCH_VOICE_EFFECT				0				//�Ƿ�ʶ����Чģ��[e?]    //sqb ����

//====================================		����Դ��ʽ��ֻ�ܿ�1����	  ===================================================================
#define EM_RES_READ_FILE						0				//������ֱ�Ӷ��ļ���ΪӢ�ĺϳ���ʱ�ӵģ�
#define EM_RES_READ_CALLBACK					1				//�û��ص�����Դ������������Դ��ȡ�ص�,�ص�ʱ�����ֱ�Ӷ�д���ļ���д��
#define EM_RES_READ_ALL_LIB_CONST				0				//LIB������������Դ���˿����ر�����irf��Դ�ļ���ȫ���ӳ�������
#define EM_RES_READ_REAR_LIB_CONST				0				//LIB�������������Դ��������Դ���Ǵ�irf�лص���,��˴�lib��������
#define EM_RES_READ_DIRECT_ANKAI				0				//����ֱ�Ӷ���Դ��SYN6658����ƽ̨ר�ã�

//====================================		����+��Դ	  =====================================================================

//08Kģ������-����    ��40K�ѣ������ֺϳɣ�  �����ϳɵ��������Ϊ50������������(��֧��һ��һ�ٷ�񣬲�֧�ֽ�����)
//16Kģ������-����    ��64K�ѣ������ɴʺϳɣ������ϳɵ��������Ϊ80�����з����˶�֧��           

#define EM_8K_MODEL_PLAN						0				//���÷�����1��8Kģ�����ã�0��16Kģ�����ã�
//================================================================================================================================


//Ӣ�����Ƿ���ǰԤ��Lab���ظ�����  
//��ǰԤ���ĺô�����˺ϳ�Lab���ᳬ�����볬ʱ������˺ϳɣ���ʱǰ�˿ռ仹û�ͷţ��������ڴ濪������������Lab����ĸ������������ڴ濪����
//��ǰԤ���Ļ�����Ԥ������׼ȷ�������Ǵ����ֵģ������¸�һ��ϳɵı��ֳ�����ϳɣ�������ֶΣ�����������
#define	EM_EN_LAB_EXPECT				0			//Ŀǰ�������ó�0   ��û��ȫ�����Ԥ����������ȷ�ԣ�

//��Ӣ��ǰ�ˣ����� =====================================================================================================================================

#define EM_OPEN_HTRS_BIANYIN			0			//�Ƿ�����Ӣ��Htrs������ʶ�
#define EM_ENG_AS_SEN_SYNTH				0			//�Ƿ����������Ӻϳɣ�1�������Ӻϳ�  0�������ʷֶκϳɣ� ��ע�⣺���ó�1���ϳ����Ļ�����⣬��ʱ��Ӣ������ĸҲ�е��ҡ�

#define	HTRS_REAR_DEAL_SP				0			//��������Ӣ�Ŀ�-����-�ı�sp���ص����
#define HTRS_REAR_DEAL_ALL_VOL			0			//��������Ӣ�Ŀ�-����-��ȫ������  //���뿪���������ᳬ�磨���磺Other��
#define HTRS_REAR_DEAL_CHANGE_MSD		0			//��������Ӣ�Ŀ�-����-����ת���ı�MSD��־
#define DEBUG_LOG_ENF_MSD				0			//��������Ӣ�Ŀ�-Ӣ�ģ�MSD��־

//��-Ŀǰ���Խ��ȡ��0.7��ϵ����lsp��ֵ0.3����ѭ��2�飬��ѭ������Խ�࣬��Խ�൫ʧ��Ҳ��  ѭ������Խ�࣬lsp��ֵ��0.4������һ�㣩
#define HTRS_REAR_DEAL_RH_PARA			0.7			//��������Ӣ�Ŀ⣺���LSP�񻯿��أ���Χ��0.5--1�� ��=1������ �� =0.7����Ч��Ŀǰ�Ϻã� �ɵ��ڴ�ϵ���۲��񻯵�Ч����
#define HTRS_REAR_DEAL_RH_PARA_FIX		(179)		//��������Ӣ�Ŀ⣺���LSP�񻯿���-���㣨��HTRS_REAR_DEAL_RH_PARA_FIXͬʱ�ģ�������256����(0.5:128, 0.6:154, 0.7:179)
#define	HTRS_RH_PARA_MIN_DIFF			0.052		//��������Ӣ�Ŀ⣺���LSP���Ż���lsp��ֵ����Сֵ  = 0.3 * 3.14 / 18  ( = 0.3����0.4�� * 3.14 / Ƶ��ά��)  ��0.2����0.037��  0.25����0.043��  0.3����0.052��  0.35����0.061��  0.4����0.069��  0.5����0.087��
#define	HTRS_RH_PARA_MIN_DIFF_FIX		1704		//��������Ӣ�Ŀ⣺���LSP���Ż���lsp��ֵ����Сֵ-���㣨��HTRS_RH_PARA_MIN_DIFFͬʱ�ģ�������Q15������Ӧ��ϵ��0.037��1212��  0.043��1409��  0.052��1704��  0.061��1999��  0.069��2261��  0.087��2851����
#define HTRS_RH_PARA_TIMES				3			//��������Ӣ�Ŀ⣺���LSP���Ż���ѭ���񻯱���


#define EM_PE_HEAP_SIZE					(12288)		/* ǰ�˶Ѵ�С(8bitƽ̨) */
#define EM_USERDICT_RAM					0			/* �Ƿ��û��ʵ���ص��ڴ� */
#define PURE_CHANGE_RHYTHM				0			//���ã���Ӣ��ǰ�ˣ�L3����
#define PURE_CHANGE_ACCENT				0			//���ã���Ӣ��ǰ�ˣ�Accent����
#define EM_ENG_PURE_LOG					0			//��Ӣ��ǰ��-��־����



//#define HTRS_REAR_DEAL_SPEED_FACTOR		(0.0)		//��������Ӣ�Ŀ⣺���ٵ���		��=0�������ڣ� =0.1��������һ�㣩	

//#define	HTRS_OPEN_QUEST05				1			//��������Ӣ�Ŀ⣺�Ƿ������á�05�����⼯��ѵ��ģ��
//#define HTRS_CHANGE_PHONE				1			//��������Ӣ�Ŀ⣺�Ƿ����ò�������ת�������磺ax->ah������������

//#define	HTRS_HANDLE_ERR_DICT			1			//��������Ӣ�Ŀ⣺�������ĵ���
//#define	HTRS_HANDLE_PIE					1			//��������Ӣ�Ŀ⣺����Ʋ�����ʵ���ش��루���뿪�˿��أ������ڲ鿴��ش��룩

//#define HTRS_REAR_DEAL_FIX				0			//��������Ӣ�Ŀ⣺��������

#define HTRS_LINK_CN_EN					0			//��������Ӣ�Ŀ⣺��Ӣ�����Ӵ��Ż���1��ģ�����Ӵ���ǰ���ﾳ�� 0�����Ӵ���ǰ���ﾳ��ֱ���Ǿ��׾�β��  �Ż��ĺ�Щ�����磺����scansoft��˾��mew speech����holdס������ȥ������windows���̡�FEELING�ưɣ�ROYAL�赸������KEY������

//=====================================================================================================================================


//=========================================��������Ӣ�Ŀ�-������-С�����-2014-2-20=================================================
//#define HTRS_REAR_DEAL_GAIN             1         //��������Ӣ�Ŀ�:���������ܿ���
//#define HTRS_REAR_DEAL_DUR              1         //��������Ӣ�Ŀ�:ʱ�������ܿ���
//
//#define HTRS_REAR_DEAL_V                1         //v��ͷ��������,ֻ�������һ��״̬������
//#define HTRS_REAR_DEAL_M_N              0          //M N NG �׵�����
//#define HTRS_REAR_DEAL_IY_R             1           // year
//#define HTRS_REAR_DEAL_AH               1         //ahʱ����
//#define HTRS_REAR_DEAL_Z                1         // Z������
//#define HTRS_REAR_DEAL_S                1         // S + ������ʱ�����һ��״̬�޸� 
//#define HTRS_REAR_DEAL_HH				1         //hh����ʼ������,hhʱ������
//#define HTRS_REAR_DEAL_B                1         //��������Ӣ�Ŀ⣺������b����������
//#define HTRS_REAR_DEAL_K                1         //k������,���͵�һ״̬����
//#define HTRS_REAR_DEAL_P                1         //������p����������
//#define HTRS_REAR_DEAL_G                1         //������g����������
//#define HTRS_REAR_DEAL_D                1         //������d����������
//#define HTRS_REAR_DEAL_DH               1         // DH
//#define HTRS_REAR_DEAL_T                1         // t
//#define HTRS_REAR_DEAL_ZH               1         // ZH
//#define HTRS_REAR_DEAL_JH               1         // JH
//#define HTRS_REAR_DEAL_CH               1         // CH
//#define HTRS_REAR_DEAL_TH               1         //TH ,something
//#define HTRS_REAR_DEAL_Y                1         // Y��ͷ���ײ��ֲ���
//
//#define HTRS_REAR_DEAL_LF0_WIN		    0         //��Ƶƽ��



//==========================================================================================================================










//====================================		Cache����+�������ڴ�	  =====================================================================
//�����ӿ졶ǰ�ˡ�
#define EM_RES_CACHE							0				//�Ƿ�֧����ԴCache���������ã���8+4��*512�ֽڣ�

//====================================		�����������ݷ�ʽ	  ============================================================

//if(���á���ԴCache���򡶺��Lib��������)��	һ��Ҫ���ó�0�� 
//else if()���������ó�1����0�������˭���� 
#define EM_DECISION_READ_ALL					0				//�Ƿ񣺶������þ�������1�������������� 0����ζ�����Ҫ�У�															

//�˿���Ӱ�첻��
#define EM_READ_LEAF_FLOAT_ALL					0				//�Ƿ�һ���Զ�������floatҶ�ڵ㣨1������������ 0������float�Ķ���
//================================================================================================================================















//====================================		emApiPlatform.h ����	  =========================================================

//						
//====================================		ƽ̨����(��࿪1��)	  ===================================================================

//ע�⣺ ARM MTKƽ̨�Ŀ⺯�� strcmp(str1,str2)Ҳ������������ȴ����-1����ò���strcmp����, ��strncmp��memcmp�滻

#define ANKAI_TOTAL_SWITCH						0		//
#define ARM_LINUX_SWITCH						0		//ARM Linuxƽ̨  ���ܿ��� (֧�ּ�ʱ��־)
#define ARM_WINCE_TIME							0		//ARM WINCEƽ̨  ���ܿ��� (֧�ּ�ʱ��־)

//2�����ؿ�1���򶼲�����Ϊ�˶Աȵ��ԣ�
#define ARM_MTK_LOG_TO_MTK						0		//ARM MTKƽ̨	 ����־���أ���ӡ��PC��VSƽ̨��
#define ARM_MTK_LOG_TO_PC_VS					0		//ARM MTKƽ̨	 ����־���أ���ӡ��MTKƽ̨��

#define ARM_MTK_LOG_TIME						0		//ARM MTKƽ̨	 ����ʱ���أ������ DEBUG_LOG_SWITCH_TIME ��ʱ�����ز���Ч��


//====================================		�����ĺϳ�-��־������	  ==================================================================================

#define	DEBUG_LOG_SWITCH_CACHE					0		//�Ƿ�	�������־_��ԴCache��
#define	DEBUG_LOG_SWITCH_HANZI_LINK				0		//�Ƿ�	�������־_��������
#define	DEBUG_LOG_SWITCH_HEAP					0		//�Ƿ�	�������־_�ڴ�ѡ��͡���־_�ڴ�ջ��
#define	DEBUG_LOG_SWITCH_RHYTHM_RESULT			0		//�Ƿ�	�������־_���ɡ�--���ս��
#define	DEBUG_LOG_SWITCH_RHYTHM_MIDDLE			0		//�Ƿ�	�������־_���ɡ�--�м����
#define	DEBUG_LOG_SWITCH_POS_IN_RHYTHM			0		//�Ƿ�	��  ����־_���ɡ��У��Ƿ�POS��ϢҲ���
#define	DEBUG_LOG_SWITCH_LAB					0		//�Ƿ�	������ϳ�.lab��
#define	DEBUG_LOG_SWITCH_LAB_BUF				1		//�Ƿ�	���lab
#define	DEBUG_LOG_SWITCH_TIME					0		//�Ƿ�	�������־--��ʱ��
#define	DEBUG_LOG_SWITCH_ERROR					0		//�Ƿ�	�����������Ϣ��
#define	DEBUG_LOG_POLY_TEST						0		//�Ƿ�	�������־--�����ֲ���--����--temp��,������ֱ����
#define	DEBUG_LOG_SWITCH_PW_COMBINE				0		//�Ƿ�	�������־--���ɴ�ƴ�ӡ�
#define	DEBUG_LOG_SWITCH_IS_OUTPUT_POS			0		//�Ƿ�	��  ����־--���ɴ�ƴ�ӡ��У��Ƿ�POS��ϢҲ��� 

//========================================================================================================================================



//====================================		��Ӣ�ĺϳ�-��־������	  ==================================================================================



//========================================================================================================================================






























//==========================================  ����ע��  ==================================================================================

//ע�⣺�ڡ�emRepackApi.h��Ҳ�п��� ��DEBUG_LOG_SWITCH_WAV_FILE����DEBUG_SYS_SWITCH_DIRECT_PLAY����
//ע�⣺�ڡ�emPCH.h��Ҳ�п��� 

//�����ֲ��ԣ���DEBUG_LOG_POLY_TEST��־���أ�ע�͵���������򣻹ر�emRepackApi.h�е��������أ������emTTS.wav��+ ʵʱ�����ı�

/*����ƽ̨��ֲ���̣�
	1. emPCH.h�Ŀ��ش򿪣��ر�������־���أ��򿪰������ܿ���
	2. �ѡ�emTTS_HEAP_SIZE�����������ⷢ���˺��������ٺ����80������һ�ڵ㣬Ŀǰÿ2�����ڴ�47K��ÿ3�����ڴ�60K��������Ч�����20K�ڴ棻
*/

//�������ͣ� 1.����[p1000]��   2.���ͣ�٣�	3. �����βͣ��Ч�� GenPauseNoOutSil()

//=======================================================================================================================================



//******************************** ��̹淶(Ϊ�˿�ƽ̨) ***********************************************************
//����Դ��д�����������ݿ�ֱ�Ӷ�д���ļ���д��ROM��д
//					ֻ����fRearSeek��fRearRead��fFrontSeek��fFrontRead��ע�⣺�����з���ֵ��
//					������fseek��fread��fgetc��
//
//��ANSI�ڴ�����⡿�����أ�EM_ANSI_MEMORY��//�Ƿ�ʹ�� ANSI �ڴ������
//					������emMemSet,emMemCpy,emMemMove,emMemCmp �ֱ���棺memset,memcpy,memmove,memcmp
//
//��ANSI�ַ��������⡿�����أ�EM_ANSI_STRING��//�Ƿ�ʹ�� ANSI �ַ���������	
//					emTTS���ĳ����б�����emStrLenA �ֱ���棺strlen
//					emTTS���ĳ����У��޵��ã�strlen��wcslen��������1�Σ�emStrLenW��emStrLen������ε��ã�emStrLenA��
//
//��Unicode ��ʽ�����������أ�EM_UNICODE��//�Ƿ��� Unicode ��ʽ����
//					emTTS���ĳ����б�����emCharA�ֱ���棺char
//					emTTS���ĳ����У��޵��ã�char��������1�Σ�emCharW��emChar������ε��ã�emCharA��
//
//*****************************************************************************************************************



//====================================  �����̶����ĵĿ���  ==================================================================================

#define EM_SYS_SWITCH_FIX						0				//�Ƿ񣺰�������ϳɡ�--������LSPģ��ʱ���ܴ�


#if EM_ENG_AS_SEN_SYNTH
#define	EM_SYN_SEN_HEAD_SIL						1				//�Ƿ񣺺ϳ�ÿ��ľ���sil
#else
#define	EM_SYN_SEN_HEAD_SIL						0				//�Ƿ񣺺ϳ�ÿ��ľ���sil
#endif

#define OUTPUT_FIRST_SIL_IN_TEXT				0				//�Ƿ�����ϳ��ı��е�1��ľ���sil

#define	EM_PAU_TO_MUTE							1				//�Ƿ񣺾��е�pauͣ��ֱ����������������߽����˲�����Ŀǰ���򿪣��з��գ�	

// fhy 121225
#define SYN_MIX_LANG_LAB						1				//�Ƿ񣺺ϳɻ������lab

#define USER_HEAP_MIN_CONFIG					(40<<10)		// 8Kģ�����ã�LSPģ���ڴ���������MGCģ�ͻ���20K��
#define USER_HEAP_MAX_CONFIG					(64<<10)		//16Kģ�����ã�LSPģ���ڴ���������MGCģ�ͻ���20K��
#define USER_HEAP_VOICE_EFFECT					(20<<10)		//��Чģʽ���ڴ�������

#if EM_8K_MODEL_PLAN	//8Kģ������	ֻ֧��50������			
	#define MAX_FRAME_COUNT_OF_SYL				100
	#define MAX_HANZI_COUNT_OF_LINK				50
#else					//16Kģ������
	#define MAX_FRAME_COUNT_OF_SYL				150				//��ˣ��ֶκϳɵ�ÿ������ƽ����������֡�����迼������������ʱ�򣩣�Ŀǰ90�Ǻ��ʵģ���������+���㣩������Ϊ150  // [s0]���������٣���������������������������������[d]
	#define MAX_HANZI_COUNT_OF_LINK				100				//�����ϳɵ�������������ڻᱻ��֣���ÿ����10���֣��ڴ�ռ��Լ���ռ1K��
#endif

#define	EM_PW_SEGMENT_OPEN					1					//�Ƿ񣺰����ɴʷֶ� ��1�������ɴ�  0�����̶��֣�
#define	MAX_CITIAO_COUNT					20					//�ʵ�������������Ŀǰ���������Ǵʡ��͡�����13�����������籣������ɸĳ�16��������
#define SEG_ADD_PAU_TEN_STAGE_TEN_FRAME		1					//�Ƿ�1���ֶκϳ�ǰ���pau����10��״̬��10֡��0��ĿǰǰpauΪ6֡��pauΪ4֡��Ч��Ҳ�У��Բ�һ�㣩 ���رմ˿��ؿɣ������������������ڴ棩


#if EM_PW_SEGMENT_OPEN				//���ɴʷֶΣ��ʺϣ������ã����ܶ��������ȡ�ֶκ������д�
	#define EM_SYNTH_ONE_WORD_PW				0				//�Ƿ�����ϳɵ������ɴʣ�0�������� 1������
	#define FIRST_MIN_SYNTH_LAB_COUNT			3				//����������emTTS_SynthText�����״κϳɵ�����������Сֵ����������������������
	#define MAX_SYNTH_LAB_COUNT					5				//��ˣ������ɴʷֶκϳɡ�ÿ�Ρ��ϳɵ���������sil��PauҲ���������������ϳ�ʱ��β��ʱ�ӵ�Сpau  ��СΪ2����Ϊ3ʱ��ѣ����ռ��ٶ��������4��5���ܺ����������ɴʣ�
	#define	EM_STYLE_WORD_SYNTH_LAB_COUNT		2				//��ˣ�����һ��һ�ٷ����ÿ�κϳɵ�������������Ϊ2(1����+1Pau)��
#else								//�������ֶΣ��ʺϣ�С���ã����ܶ��������ȡ�ֶκ������д����磺�ĳ�1,1,1������ȡ�ֶκ������ܻ᷵��2����ɿռ������
	#define FIRST_SYNTH_LAB_COUNT				3				//��ˣ����ֶַκϳ�ʱ���״Ρ��ϳɵ���������sil��PauҲ���������������ϳ�ʱ��β��ʱ�ӵ�Сpau����СΪ1
	#define MAX_SYNTH_LAB_COUNT					3				//��ˣ����ֶַκϳ�ʱ��ÿ�Ρ��ϳɵ���������sil��PauҲ���������������ϳ�ʱ��β��ʱ�ӵ�Сpau�����״��⣩����СΪ1����������״κϳɵ�����
	#define	EM_STYLE_WORD_SYNTH_LAB_COUNT		3				//��ˣ�����һ��һ�ٷ����ÿ�κϳɵ�������������Ϊ2(1����+1Pau)��
#endif


// fhy 121225
#define NSTATE_ENG						5		//Ӣ��
#define NSTATE_CN						10		//����
//#define	NSTATE							NSTATE_CN		//��Ϊ10 ����Ӣ��

#if EM_ENG_AS_SEN_SYNTH
	#define MAX_SYNTH_ENG_LAB_COUNT		100		// �ϳ�һ�ε����Ӣ��lab��  ������ֶΡ�
#else
	#define MAX_SYNTH_ENG_LAB_COUNT		8		// �ϳ�һ�ε����Ӣ��lab��  �������ʷֶΡ�   ������С��8��LabNumOfWord()�����з��գ�1��������������8�����أ����������ʵ����س�������������LocateSegment_Eng()���ٴηֶΣ�
#endif


//ע�⣺�˿������ó�1������������δ���Ը�������MTK�ºϳ������ı���β�������������磺bbb��zzz��fff��dddddd��iiiiii����
#define EM_CAL_EXP								0				//�Ƿ�����expָ������Ķ��㻯��ÿ���ֶ��˲��ܽ�ʡ15ms���������ж���ı䣩


//=======================================================================================================================================



//====================================	ר����ԡ����֣�StrLsp18ά������261ģ+��ĸ303ģ�����⣺����һϵ�к��� ======================================================================================
	
//���ò�����
#define WL_REAR_DEAL_SPEED_FACTOR				(0.06)	//��������������ٵ���		��=0�������ڣ� =0.1��������һ�㣩			
#define WL_REAR_DEAL_GV_PARA					0		//�������������˻�ƵGV���� ����Χ��  0--1�� ��=0������GV �� =0.7��GVЧ��Ŀǰ�Ϻã� �ɵ��ڴ�ϵ���۲�GV��Ч����

#define WL_REAR_DEAL_RH_PARA					0.7		//��������������LSP�񻯿��أ���Χ��0.5--1�� ��=1������ �� =0.7����Ч��Ŀǰ�Ϻã� �ɵ��ڴ�ϵ���۲��񻯵�Ч����
#define WL_REAR_DEAL_RH_PARA_FIX				(179)	//��������������LSP�񻯿���-���㣨��WL_REAR_DEAL_RH_PARAͬʱ�ģ�������256����


	//��򿪵�    sqb 2016.12.5
#define		WL_REAR_SWITCH      (0)				//����

#define	WL_REAR_DEAL_LETTER						WL_REAR_SWITCH		//�����������Ӣ����ĸ
#define	WL_REAR_DEAL_VIRTUAL_QST				WL_REAR_SWITCH		//������������Ƿ񣺴������ⷢ���˵�һЩ����
#define WL_REAR_DEAL_CHAN_YIN					WL_REAR_SWITCH		//������������ı䲿�ֲ���  ze4����ĸen eng an ang ong�������ĸ
#define WL_REAR_DEAL_LIGHT_GAIN					WL_REAR_SWITCH		//������������ı䲿������������������Ч����
#define WL_REAR_DEAL_LIGHT_GAIN_TWO		    	WL_REAR_SWITCH		//���������������һЩ�������ٶ�� le5/me5/ne5/de5/men5	wchm
#define WL_REAR_DEAL_END_GAIN   		    	WL_REAR_SWITCH		//�����������for zi4/pian4 unlight	wchm
#define WL_REAR_DEAL_END_GAIN_TWO           	WL_REAR_SWITCH		//�����������for pin3 unlight	wchm
#define WL_REAR_DEAL_LIGHT_MGC					WL_REAR_SWITCH		//������������ı䲿��������Ƶ�ף�����Ч����
#define WL_REAR_DEAL_DUR_ZENME					WL_REAR_SWITCH		//�����������"��ô"
#define WL_REAR_DEAL_SYL_TOO_FAST				WL_REAR_SWITCH		//��������������ڹ���		????????
#define WL_REAR_DEAL_SYL_TOO_FAST_NEW			WL_REAR_SWITCH		//��������������ڹ��� wchm
#define	WL_REAR_DEAL_FOU3						WL_REAR_SWITCH		//�����������������		��4��5��״̬ǿ��Ϊ����
#define WL_REAR_DEAL_LOWER_F0					WL_REAR_SWITCH		//�������������Ƶ��		��β������ĸ4����Ƶ�͵ĵ��������˻��������Ŀ, naxy 1220
#define WL_REAR_DEAL_EN_ENG_QST					WL_REAR_SWITCH		//�������������ĸeng	����weng����en2��eng2���3����������ĸ  ���磺���ͣ����̣���������[i1]sen2mei3��[i1]wen2mei3��[i1]deng2mei3��[i1]feng2mei3
#define WL_REAR_DEAL_LITTER_QST					WL_REAR_SWITCH		//�����������С����	��Ҫ�ǣ�gong����֨֨����	����ĸǿ��Ϊȫ��������ch p t k��ĸ֨֨������
#define WL_REAR_DEAL_R_INITIAL					WL_REAR_SWITCH		//�������������ĸr
#define WL_REAR_DEAL_GONG4_GONG4				WL_REAR_SWITCH		//�����������gong4��
#define WL_REAR_DEAL_FIX						WL_REAR_SWITCH		//�������������������
#define	WL_REAR_DEAL_LAB_LONG_SEN				WL_REAR_SWITCH		//�����������Lab�н����������[[[[[[[[
#define	WL_REAR_DEAL_LAB_SHORT_SEN				WL_REAR_SWITCH		//�����������Lab�н���̾����⣺3�ֶ̾���lab��ģ���4�־�
#define	WL_REAR_DEAL_DCT_LIGHT					WL_REAR_SWITCH		//�����������DCTģ�壺�������ɶ���β���������־�����ϣ���������ĸ��
#define	WL_REAR_DEAL_DCT_PPH_TAIL				WL_REAR_SWITCH		//�����������DCTģ�壺�������ɶ���β	 ��������˫�̾䣬��������ĸ��
#define	WL_REAR_DEAL_DCT_SHORT_SEN				WL_REAR_SWITCH		//�����������DCTģ�壺���ĵ�˫��		  (��ĸ�Ĳ�����): ר�Ŵ���<���־�><���־�>�Ļ�Ƶ,ʱ��,������
#define WL_REAR_DEAL_DAO_LF0					WL_REAR_SWITCH		//�����������zhao3dao4�Ļ�Ƶ���			mdj
#define WL_REAR_DEAL_RU3_LF0					WL_REAR_SWITCH		//�����������ru3�Ļ�Ƶ���					mdj
#define WL_REAR_DEAL_LIANG_LF0					WL_REAR_SWITCH		//�����������liang3�Ļ�Ƶ���				mdj
#define WL_REAR_DEAL_DCT_PPH_TAIL_TRAIL			WL_REAR_SWITCH		//�����������DCTģ��+ԭʼ��Ƶ����ֵ���������ɶ���β	 ��������˫�̾䣬��������ĸ��	mdj
#define WL_REAR_DEAL_DCT_SEN_TAIL_LEN			WL_REAR_SWITCH		//�����������DCTģ��+������β����ʱ���������������ɶ���β�;�β	 ��������˫�̾䣬��������ĸ��	mdj
#define WL_REAR_DEAL_LOWER_F0_ALL				WL_REAR_SWITCH		//�������������Ƶ��		��β������ĸ4����Ƶ�͵ĵ���������WL_REAR_DEAL_LOWER_F0����û���޸ĵķ���β�Ļ�Ƶ��������	mdj
#define WL_REAR_DEAL_ZHUO_SM					WL_REAR_SWITCH		//���������������ĸ����ĸ����ʱ������
#define WL_REAR_DEAL_OU_CHAN_YIN				WL_REAR_SWITCH		//������������ı�����ĸou���� ���硰�ж�ŷ��				mdj
#define WL_REAR_DEAL_AI_CHAN_YIN				WL_REAR_SWITCH		//������������ı�����ĸai���� ���硰����������				mdj
#define WL_REAR_DEAL_THIRD_TONE_LF0				WL_REAR_SWITCH		//�������������������������Ӧ������Ļ�Ƶ��������			mdj
#define WL_REAR_DEAL_MA0_DCT					WL_REAR_SWITCH		//���������������mao1��βDCTģ���Ƶ����			mdj
#define WL_REAR_DEAL_LI_SPEC					WL_REAR_SWITCH		//���������������li��β��ʱ�����׵�����	ʱ�������ӣ��޸Ĳ���״̬��ʱ��
#define WL_REAR_DEAL_IANG_LF0					WL_REAR_SWITCH		//���������������iang4��Ƶ�յ�����			added by mdj 2010-03-15
#define WL_REAR_DEAL_AI3_DCT					WL_REAR_SWITCH		//���������������ai3��β��Ƶģ�������		added by mdj 2012-03-19
#define WL_REAR_DEAL_TONE3_LINKED_ZHUOSM        WL_REAR_SWITCH       //��������������������������ĸ������m/l/n��������Ƶ�͵����  -wangcm-2012-03-20
#define WL_REAR_DEAL_DCT_SHORT_SEN_LEN			WL_REAR_SWITCH	//<����>���Ķ̾䴦��ģʽ(��ĸ�Ĳ�����)��ר�Ŵ������־䣬���־�   added by mdj 2012-03-26
#define WL_REAR_DEAL_LIGHT_M_N                  WL_REAR_SWITCH       //�����������������β��������ĸ���õ����    -wangcm-20120401
#define WL_REAR_DEAL_VOL_TOO_LARGE             WL_REAR_SWITCH       //�������������������������������

	//Ŀǰ���򿪵�  ����Ҫ��  ���ܼ�
	#define	WL_REAR_DEAL_LIGHT_TONE					0		//���������������			��Ƶ��10,���ٿ�10%,������10%��Ч�������ԣ�һ�㲻�򿪣�
	#define	WL_REAR_DEAL_SHEN2_ME5					0		//�����������ר�Ŵ���ʲô��
	#define	WL_REAR_DEAL_DUR_XIEXIE					0		//�����������лл (����ʱ��������)


//========================================================================================================================================
#define  HT_REAR_DEAL_PAU_ENERGY					(1)				//������ͣ��������0
#define  HT_REAR_DEAL_DUR							(1)				//ͼ�������ʱ������


//====================================  ���������һ�㲻�� ==================================================================================

#define EM_INTERFACE_VQ							1		//������������Ƶ�ײ����Ƿ�������0����������1��������  (Ŀǰ��Ȼ����Ϊ1����ʵ���ϲ�������)
#define EM_INTERFACE_IS_STRAIGHT				0		//�������������Ƿ�ʹ��straight�㷨,����wolrd��ȡ����
#define EM_INTERFACE_IS_LSP						1		//������������ģ���Ƿ���LSPģ�ͣ�0��MGC��1��LSP��

//========================================================================================================================================




//====================================  �����̶����ĵĿ���  ==================================================================================

#define  CHABO_INDEX							8			//�ڼ���岥
#define  CHABO_EACH_COUNT						10			//ÿ����岥һ��

#define	EM_PROCESS_CALLBACK_OPEN				1			//�Ƿ񣺴򿪽��Ȼص�

#define EM_USER_DECODER_ADPCM					0			//�Ƿ�֧�� ADPCM ����	 ����Դ����ADPCM�������������򿪣�һ�㲻��
#define EM_USER_DECODER_G7231					1			//�Ƿ�֧�� G7231 ����  ����Դ����G7231��������������

#define PPH_AFTER_DEAL							1			//�Ƿ�PPH�߽绮�ֺ���

#define EM_USER_ENCODER_ADPCM					0			//�Ƿ�֧�� ADPCM ����  һ�㲻��
#define EM_USER_ENCODER_G7231					0			//�Ƿ�֧�� G7231 ����  һ�㲻�� 

#define EM_USER_VOICE_RESAMPLE					1			//�Ƿ�֧���ز���(���磺���ⷢ���˵���ɫ������)


#define UVP_DUR_MOD								0			//(���ܼ�Ч������)������ת��ģ�Ϳ��أ���״̬ʱ���������ڣ���UVP_FRAME_MOD���ػ��⣩
#define UVP_DUR_MOD_P							0			//(���ܼ�Ч������)��UVP_DUR_MOD�򿪵Ļ����ϵĶ������أ�����MSD�о����Ƿ�����״̬��1��

#define	EM_SYS_SWITCH_RHYTHM_NEW				1			//�Ƿ��á�����Ԥ��--�·�����  ע�⣺�Ϸ����ݲ�ͨ

#define EM_PARAM_MAX_SAMPLE						16000		//����ڲ�������
#define MAX_OUTPUT_PCM_SIZE						1200		//���Ĳ������PCM���ݵĻ������Ĵ�С�� ������2��ָ������  hyl 2012-10-15 �����豸�ĵ����������С�����ֵ

//��ע�⣺�����á���ʼ������22�ž������ڴ桱���ԣ������Ը��ƺ���ٶȣ���Ϊ��ģ��Ҷ�ڵ㻹�Ǻ�����
#define EM_DECISION_HEAP						0				//�Ƿ�֧�־������ڴ�(���뽫EM_DECISION_READ_ALL���ó�1���������þ�����)

#define USER_HEAP_MIN_CONFIG_ADD				(100<<10)		// 8Kģ�����ã�LSPģ���û��ܶ��ṩ���ڴ������ʺ��ڴ�ռ主ԣ�Ŀͻ������ڳ�ʼ��ʱ�����22�ž�����ȫ�����뵽�ڴ棩
#define USER_HEAP_MAX_CONFIG_ADD				(170<<10)		//16Kģ�����ã�LSPģ���û��ܶ��ṩ���ڴ������ʺ��ڴ�ռ主ԣ�Ŀͻ������ڳ�ʼ��ʱ�����22�ž�����ȫ�����뵽�ڴ棩


//========================================================================================================================================





//====================================  һ�㲻�õĿ���,ά����״  ==================================================================================

#define SYL_DURATION							0		//����ʱ��ģ�Ϳ��أ��ݹأ����Ϻã��ȶ��Ի�û���������ԣ�����ż�����е������������׾�β������ʱ����ӳ������ʵ�ʱ�����̣�		 changed by naxy19
#define	REAR_LSP_POSTFILTER						0		//��ˣ�LSP���˲�����   ĿǰЧ������
#define UVP_FRAME_MOD							0		//������ת��ģ�Ϳ��أ���֡��������UVP_DUR_MOD���ػ��⣩	(һ�㲻�ã���������)
#define EM_SYS_QING_ADD_ONE_STATUS				0		//�Ƿ񣺴򿪡�ǿ��ĳЩƴ��������1��״̬������ -- ���ƣ��������б𲻺á����⣬Ŀǰ������0.8Kȫ�ֿռ䣬���Ż�
														//�˿����Ժ��ã�����UVP_DUR_MOD��������������ת��


#define REAR_RH_ADAPTION						0		//��ˣ����Ƿ�����Ӧ����	ĿǰЧ������
#if REAR_RH_ADAPTION
	//��ˣ��൱�ں��LSP�񻯿��أ���Χ��0.5--1�� ��=1������ �� =0.7����Ч��Ŀǰ�Ϻã� �ɵ��ڴ�ϵ���۲��񻯵�Ч����
	#define EM_SYS_VAR_REAR_RH_PARA_L					0.8		//Ч���Ϻõ�ϵ����0.8
	#define EM_SYS_VAR_REAR_RH_PARA_M					0.7		//Ч���Ϻõ�ϵ����0.7
	#define EM_SYS_VAR_REAR_RH_PARA_H					0.8		//Ч���Ϻõ�ϵ����0.8
#endif
//========================================================================================================================================




//======================================== ��������ء� ==================================================================================

//�����Դ��ȡ��أ�
#define Table_NODE_LEN_INF			8		//�����Դ�洢�ģ�����ģ�;�����ÿ�е��ֽ���
//#define PDF_NODE_FLOAT_COUNT_DUR	20		//�����Դ�洢�ģ�״̬ʱ��ģ��ÿ��Ҷ�ڵ��float����
#define PDF_NODE_FLOAT_COUNT_SYD 	2		//�����Դ�洢�ģ�����ʱ��ģ��ÿ��Ҷ�ڵ��float���� ,		changed by naxy19
#define PDF_NODE_FLOAT_COUNT_UVP	1		//�����Դ�洢�ģ�������ת����ģ��ÿ��Ҷ�ڵ��float���� ,	changed by naxy19
#define PDF_NODE_FLOAT_COUNT_LF0	7		//�����Դ�洢�ģ���Ƶģ��ÿ��Ҷ�ڵ��float����
#define PDF_NODE_FLOAT_COUNT_BAP	6		//�����Դ�洢�ģ�bapģ��ÿ��Ҷ�ڵ��float����
#define PDF_NODE_FLOAT_COUNT_MGC	(NULL)	//�����Դ�洢�ģ�Ƶ��ģ��ÿ��Ҷ�ڵ��float���������ܺ궨�壬ȡ������Դ�ж�����׵�ά�����洢�ڣ�g_hTTS->pRearG->nMgcLeafPrmN��
//========================================================================================================================================








#endif	/* #define _EMTTS__EM_CONFIG__H_ */
