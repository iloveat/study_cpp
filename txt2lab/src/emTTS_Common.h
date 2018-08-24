#ifndef	_EMTTS__EM_TTS_COMMON__H_
#define _EMTTS__EM_TTS_COMMON__H_

#include "emConfig.h"
#include "emHeap.h"
#include "emDebug.h"


#ifdef __cplusplus
extern "C" {
#endif


#define strtok(L,M)		(emPByte)strtok((char *)L,(const char*)M)			//MTKƽ̨����ģ��������Ͳ�ƥ��

//----------------------------------------ARM MTKƽ̨�Ĵ�ӡ--------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------

//���磺
//kal_prompt_trace(MOD_IVTTS, "ģ��MTKƽ̨��ʼ��ӡ��־");			//MTK��־���ش�ʱ������Ч
//kal_sleep_task(1);
#if ARM_MTK_LOG_TO_PC_VS

	#define kal_prompt_trace(a,b,c);			{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c); fclose(g_fMTK);}

	#define kal_prompt_trace2(a,b);				{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b); fclose(g_fMTK);}
	#define kal_prompt_trace3(a,b,c);			{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c); fclose(g_fMTK);}
	#define kal_prompt_trace4(a,b,c,d);			{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c,d); fclose(g_fMTK);}
	#define kal_prompt_trace5(a,b,c,d,e);		{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c,d,e); fclose(g_fMTK);}
	#define kal_prompt_trace6(a,b,c,d,e,f);		{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c,d,e,f); fclose(g_fMTK);}
	#define kal_prompt_trace7(a,b,c,d,e,f,g);	{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c,d,e,f,g); fclose(g_fMTK);}
	#define kal_prompt_trace8(a,b,c,d,e,f,g,h);	{g_fMTK = fopen("log/��־_MTK.log","a"); fprintf(g_fMTK,b,c,d,e,f,g,h); fclose(g_fMTK);}
	
	#define kal_sleep_task(a)			((void)0)
#else
	#if ARM_MTK_LOG_TO_MTK
		#define kal_prompt_trace2(a,b);				kal_prompt_trace(a,b);	
		#define kal_prompt_trace3(a,b,c);			kal_prompt_trace(a,b,c);
		#define kal_prompt_trace4(a,b,c,d);			kal_prompt_trace(a,b,c,d);	
		#define kal_prompt_trace5(a,b,c,d,e);		kal_prompt_trace(a,b,c,d,e);
		#define kal_prompt_trace6(a,b,c,d,e,f);		kal_prompt_trace(a,b,c,d,e,f);
		#define kal_prompt_trace7(a,b,c,d,e,f,g);	kal_prompt_trace(a,b,c,d,e,f,g);
		#define kal_prompt_trace8(a,b,c,d,e,f,g,h);	kal_prompt_trace(a,b,c,d,e,f,g,h);
	#else
		#if !ARM_MTK_LOG_TIME

			#define kal_prompt_trace(a,b,c);			((void)0)

			#define kal_prompt_trace2(a,b);				((void)0)
			#define kal_prompt_trace3(a,b,c);			((void)0)
			#define kal_prompt_trace4(a,b,c,d);			((void)0)
			#define kal_prompt_trace5(a,b,c,d,e);		((void)0)
			#define kal_prompt_trace6(a,b,c,d,e,f);		((void)0)
			#define kal_prompt_trace7(a,b,c,d,e,f,g);	((void)0)
			#define kal_prompt_trace8(a,b,c,d,e,f,g,h);	((void)0)

			#define kal_sleep_task(a)					((void)0)
		#endif
	#endif
#endif

//-----------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------

typedef emUInt32	       (*  OutputCallbackFun)( emPointer , emUInt16,emPByte,emSize); 
typedef emUInt32	       (*  ProgressCallbackFun)( emPointer , emUInt32,emSize); 


//�߽�ֵ����
#define EM_SBYTE_MAX	(+127)
#define EM_MAX_INT16	(+32767)
#define EM_INT_MAX		(+8388607L)
#define EM_MAX_INT32	(+2147483647L)

#define EM_SBYTE_MIN	(-EM_SBYTE_MAX - 1)
#define EM_MIN_INT16	(-EM_MAX_INT16 - 1)
#define EM_INT_MIN		(-EM_INT_MAX - 1)
#define EM_MIN_INT32	(-EM_MAX_INT32 - 1)

#define EM_BYTE_MAX		(0xffU)
#define EM_USHORT_MAX	(0xffffU)
#define EM_UINT_MAX		(0xffffffUL)
#define EM_ULONG_MAX	(0xffffffffUL)


//��Ч����  sqb
#define emTTS_USE_VoiceMode_CLOSE		    0			//�رգ���Чģʽ���رգ�
//#define emTTS_USE_VoiceMode_FarAndNear	    1			//��Ч����Զ����
//#define emTTS_USE_VoiceMode_Echo		    2			//��Ч������
//#define emTTS_USE_VoiceMode_Robot		    3			//��Ч��������
//#define emTTS_USE_VoiceMode_Chorus		    4			//��Ч���ϳ�
//#define emTTS_USE_VoiceMode_UnderWater	    5			//��Ч��ˮ��
#define emTTS_USE_VoiceMode_Reverb		    6			//��Ч������


#define emTTS_USE_YIBIANYIN_CLOSE			0			//һ�ı���: �ر� 
#define emTTS_USE_YIBIANYIN_OPEN			1			//һ�ı���: ��

// fhy 121225
#define SYNTH_CN						0
#define SYNTH_ENG						1
#define SYN_MIX_LANG					2

//*****************************************	 Define��������  ************************************************************************

#define  REAR_FRAME_LENGTH						80			//��ˣ�֡��



//���ⷢ���ˣ��ز���ϵ��  ��1Ϊ��������    ��ֵԽС������Խ��
#define RESAMPLE_SCALE_ROLE_Virtual_51			(1.18)		//������
#define RESAMPLE_SCALE_ROLE_Virtual_52			(1.32)		//������
#define RESAMPLE_SCALE_ROLE_Virtual_53			(1.07)		//������
#define RESAMPLE_SCALE_ROLE_Virtual_54			(0.80)		//������
#define RESAMPLE_SCALE_ROLE_Virtual_55			(0.80)		//������	

//���ⷢ���ˣ���Ƶϵ��    ��0Ϊ��������    ��ֵ����һ�����ز���ϵ���ķ���һ�������෴������Ч���ȽϹ֣��磺54������Ѽ��
#define F0_SCALE_ROLE_Virtual_51				(0.2)		//������	
#define F0_SCALE_ROLE_Virtual_52				(0.4)		//������
#define F0_SCALE_ROLE_Virtual_53				(0.10)		//������
#define F0_SCALE_ROLE_Virtual_54				(0.60)		//������	-- ֻ����һ�ֻ�Ƶ����ͬ
#define F0_SCALE_ROLE_Virtual_55				(-0.25)		//������	

//���ⷢ���ˣ�ʱ��ϵ��    ��0Ϊ��������    ��ֵԽ������Խ��
#define DUR_SCALE_ROLE_Virtual_51				(0.16)		//������	
#define DUR_SCALE_ROLE_Virtual_52				(0.29)		//������
#define DUR_SCALE_ROLE_Virtual_53				(0.064)		//������	
#define DUR_SCALE_ROLE_Virtual_54				(-0.18)		//������	
#define DUR_SCALE_ROLE_Virtual_55				(-0.18)		//������	


//���ⷢ���ˣ����HtrsӢ�Ŀ����  
//Re-0.09999996��F0-0.12��DUR-0.090000011 ��Ŀǰ���Ե���Ѵ���
//Re-0.10��F0-0.12��DUR-0.09���е�����
//Re-0.05��F0-0.06��DUR-0.05��������
//Re-0.10��F0-0.25��DUR-0.18��������
//Re-0.15��F0-0.19��DUR-0.13��������
//Re-0.20��F0-0.25��DUR-0.18����������̫��С��
#define HTRS013_ENG_RESAMPLE_BASE				(-0.09999996)		//�ı����ֵ���ز���ϵ��
#define HTRS013_ENG_F0_BASE						(-0.12)				//�ı����ֵ����Ƶϵ��
#define HTRS013_ENG_DUR_BASE					(-0.090000011)		//�ı����ֵ��ʱ��ϵ��



//-------------------------------------------------------------------------------------------------------------------------

#define MAX_COMPANY_INFO_LEN					300			//�û���Ϣ�����ֽ���������ǰ���ʶ���	
#define	COMPANY_INFO_NEED_COMPARE_LEN			22

#define MAX_HANZI_COUNT_OF_LINK_NEED_HEAP		(MAX_HANZI_COUNT_OF_LINK+10)	//���ٿռ�ʱ��࿪��10����������MAX_HANZI_COUNT_OF_LINK�����ֵ�ĩβ������һ�����ֻ���ĸ��һ�㲻�ֿ�
#define CORE_CONVERT_BUF_MAX_LEN				(MAX_HANZI_COUNT_OF_LINK*2+2)	//����ת����ı���洢��������ֽڸ���   //��END_WORD_OF_BUFFER��ʾ����

#define END_WORD_OF_BUFFER			  			0xFFFF		//�����õ�buffer�Ľ����жϷ�
#define INVALID_CODE							0xA3A0		//�ո񣺱���ת������Ч����(�ɶ�����)��ת��ͳһ���루�ɸģ�
#define SPACE_CODE								0x20		//�ո�
#define TAB_CODE								0x09		//Tab����

#define KEYWORD_SEARCH_COUNT		 			6			//��ʾ���������ִ���󼸸���������Ѱ�������ߡ����ȹؼ��֣��򿪡��������������

#define  MaxLine_Of_OnePos_Of_TableBigram		126			//������Ԫ�ķ����Ĺ̶�Pos1ת�Ƶ�126����ͬ��Pos2��ֵһ����ȡ�뵽�ڴ�

#define		MAX_MANUAL_PPH_COUNT_IN_SEN			50			//һ�仰�У��ֶ���������PPH��������


#define  RES_OFFSET_ERROR				0


//�����ţ�����ʱ�����壨��λ�����룩
#if EM_SYN_SEN_HEAD_SIL
	#define	FRONT_SIL_MUTE					50				//���ϳɾ���sil��һ�仰��ǰsil�ں�˵�ʱ�������Ϊ50ms
#else
	#define	FRONT_SIL_MUTE					0				//�����ϳɾ���sil��
#endif

#if EM_USER_SWITCH_VOICE_EFFECT
	#define	LAST_SIL_MUTE					250			//һ�仰�ĺ�sil�ں�˵�ʱ�������Ϊ250ms��Ϊ����Ч�Ļ���ã���������Ҳ���У��ٺŵ�ͣ�ٻ�Ϊ������
#else
	#define	LAST_SIL_MUTE					50			
#endif

#define	PUNC_MUTE_JuHao					(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//���
#define	PUNC_MUTE_WenHao				(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//�ʺ�
#define	PUNC_MUTE_GanTanHao				(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//��̾��
#define	PUNC_MUTE_ShengLueHao			(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//ʡ�Ժ�
#define	PUNC_MUTE_HuanHangHao			(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//���к�
#define	PUNC_MUTE_FenHao				(500 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//�ֺ�
#define	PUNC_MUTE_DouHao				(450 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//����
#define	PUNC_MUTE_MaoHao				(350 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//ð��
#define	PUNC_MUTE_DunHao				(350 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//�ٺ�

//�ı���������
#define DATATYPE_HANZI 				 	0x10			// �������ͺ궨�壺 ����
#define DATATYPE_FUHAO 				 	0x11			// �������ͺ궨�壺 ����
#define DATATYPE_SHUZI 				 	0x12			// �������ͺ궨�壺 ����
#define DATATYPE_YINGWEN 			 	0x13			// �������ͺ궨�壺 Ӣ��
#define DATATYPE_END 				 	0x14			// �������ͺ궨�壺 ��β��
#define DATATYPE_UNKNOWN 			 	0x15			// �������ͺ궨�壺 ��ʶ�������

//�ı���������
#define SEN_TYPE_EN						0x10			//Ӣ�ľ�
#define SEN_TYPE_CN						0x11			//���ľ�
#define SEN_TYPE_OTHER					0x12			//��ʶ���

//�ı�����
#define  TextCsItem_TextType_HANZI		1
#define  TextCsItem_TextType_ZIMU		3

//��ʾ������
#define PROMPT_SOUND					1
#define PROMPT_MSG						2

//PPH�߽�����
#define  BORDER_NONE					0
#define  BORDER_LW						1
#define  BORDER_PW						2
#define  BORDER_PPH						3


#define  MODEL_MGC						0
#define  MODEL_LSP						1


//���ģ����ԣ���� ********************************************************************************************
//								�����ʵĸ���	����			����
#define		POS_CODE_kong	0		//	0					
#define		POS_CODE_a		1		//	3584	���ݴ�			һ�������ҡ�һ�¡��Ͻ������˵�
#define		POS_CODE_ad		2		//	745		���δ�			������������˼�������ѡ����ȡ��ƻ�
#define		POS_CODE_Ag		3		//	441		���ݴ�����		�����͡��䡢������������
#define		POS_CODE_an		4		//	300		���δ�			ֱ�ӡ���ʵ��ì�ܡ���ò������
#define		POS_CODE_b		5		//	1869	�����			Ϸ��ʽ�����ס����ġ���������ƽ
#define		POS_CODE_Bg		6		//	9		���������		�����ء��ȡ��Ρ���
#define		POS_CODE_c		7		//	253		����			�ȵ��������š���ʹ���̶����������������ҡ���
#define		POS_CODE_d		8		//	1500	����			���ġ��ݲݡ�ݸ����Ī�ǡ����⡢��Ȼ
#define		POS_CODE_dc		9		//	35						���󡢼��������⡢ʮ�֡�����
#define		POS_CODE_df		10		//	14						���ء����������á���Ҫ����û��
#define		POS_CODE_Dg		11		//	55		��������		�䡢�ϡ����������ɡ��塢ٿ
#define		POS_CODE_ds		12		//	4		�����Ը���		�������Ĵ����������洦
#define		POS_CODE_dt		13		//	20		ʱ���Ը���		һֱ���Ծɡ�������ż��������
#define		POS_CODE_dv		14		//	1						����
#define		POS_CODE_dw		15		//	7						�͡�����Ҳ���⡢��������ֻ
#define		POS_CODE_e		16		//	67		̾��			ѽ�������š��غ����ϡ��Ǻ�
#define		POS_CODE_eg		17		//	1						ŵ
#define		POS_CODE_f		18		//	332		��λ��			���ܡ���ࡢ���桢��ͷ����
#define		POS_CODE_ft		19		//	1						��
#define		POS_CODE_g		20		//	17871	����			�ɡ��ޡ������ס���衢������
#define		POS_CODE_h		21		//	16		ǰ�ӳɷ�		�Ρ������ϡ���������������
#define		POS_CODE_i		22		//	7748	����			���׷ǹʡ�ǿ�����ѡ��������衢������ʱ��Լ���׳�
#define		POS_CODE_ia		23		//	192		���ݴ��Գ���	��Ӱ���롢����������Ĺ�������Ȼ�Եá��㷲���
#define		POS_CODE_ib		24		//	87						��ǰ������ɽ�������޶���ż���Զ��׼�����������
#define		POS_CODE_ic		25		//	67						��ȥ�������������������߰��㡢������֡����쾡��
#define		POS_CODE_id		26		//	116						�����ҹ�������������ɴ˼��ˡ�����ʵ�⡢������Ҫ
#define		POS_CODE_im		27		//	51						һ����ǰ���򲻵��ѡ����������������뻯�������ܼ�
#define		POS_CODE_in		28		//	537						���Ķ��ˡ�ƥ��֮�¡�ʮ�꺮������������������֮��
#define		POS_CODE_iv		29		//	2998	�����Գ���		�������¡����������������������������Ż����Ѫ
#define		POS_CODE_j		30		//	4631	�������		һ�����ơ���ί��һ��һ̨��һ�󡢴��
#define		POS_CODE_jb		31		//	432						���⡢�й�ί����̫������������������������ó
#define		POS_CODE_jd		32		//	1						ͬ��
#define		POS_CODE_jm		33		//	3						������վ����˾�
#define		POS_CODE_jn		34		//	439						һ��ȫ�ᡢһ�����ơ�һ���������ᡢ����һ��
#define		POS_CODE_jv		35		//	15						͵©˰�����ܡ����ĺ������°ࡢ����������ƶ����
#define		POS_CODE_k		36		//	43		��ӳɷ�		����ͧ���ۡ��ۡ��ߡ���
#define		POS_CODE_l		37		//	5958	ϰ����			��һ�ǡ�����֮�ơ�������ࡢ����������һһ�о�
#define		POS_CODE_la		38		//	69						���˹��ʡ���·���ߡ������Ͽڡ�ģ�����塢��ͷ��
#define		POS_CODE_lb		39		//	29						��ƶ�¸������������������𼧡�ǿ��������ʵ��
#define		POS_CODE_lc		40		//	6						�ϲ�£�졢������ζ�����ֱ��š�������Բ��ϸ����΢
#define		POS_CODE_ld		41		//	62						��ʧʱ���������������������ӽ��Ժ󡢶�ȥ����
#define		POS_CODE_lm		42		//	16						׼ȷ�����긴һ�ꡢ������ǰ�������˾������Ա�
#define		POS_CODE_ln		43		//	629		������ϰ����	ר��ѧУ��˿��֮·�������ս���в��׼���������ֵ
#define		POS_CODE_lv		44		//	1146	������ϰ����	һһ�о١�һһ��Ӧ���ϵ��Ρ��¿๦��������ת
#define		POS_CODE_m		45		//	538		����			һ��һ��ǧ��Ƭ�̡��١��ٰ�ʮ
#define		POS_CODE_mg		46		//	12						������������î
#define		POS_CODE_mq		47		//	42						һ����������ꡢһ�����ٱ����״�
#define		POS_CODE_n		48		//	67377	����			
#define		POS_CODE_na		49		//	7						���͹�������ᡢ�������鰸�������ơ����ڡ�����
#define		POS_CODE_nf		50		//	1						����
#define		POS_CODE_Ng		51		//	1661	������			���ġ��ʡ��ѡ�յ���桢��
#define		POS_CODE_nh		52		//	1						��
#define		POS_CODE_nr		53		//	6162	����			���ظ�������硢��������ë���𼪶�
#define		POS_CODE_nrf	54		//	949		�����е���		�ԡ���������������Ұ������������
#define		POS_CODE_nrg	55		//	4591	�����е���		����̩�������桢�󶫡�������ȫ
#define		POS_CODE_ns		56		//	4177	����			��³ľ�롢��ͤ���Ż�ɽ����Ȫ����Զ
#define		POS_CODE_nt		57		//	534		������			Ų���ӡ����ӡ��ݿ˶ӡ��ݰ��ضӡ�Ħ����
#define		POS_CODE_nx		58		//	5						�@뎡��˵�ڡ���������
#define		POS_CODE_nz		59		//	7152	����ר��		Ħ�ޡ�Ħ�޵��������ɡ������塢��ɳ
#define		POS_CODE_nzg	60		//	1						����
#define		POS_CODE_o		61		//	257		������			ͻͻ���������䡢��¡¡����
#define		POS_CODE_p		62		//	127		���			����͸�������ա����ڡ����
#define		POS_CODE_pt		63		//	1						��
#define		POS_CODE_q		64		//	738		����			�ӡ�����Ǯ������������
#define		POS_CODE_qb		65		//	3						Щ���㡢���
#define		POS_CODE_qc		66		//	50						Ƭ���ꡢ�衢縡��ơ���
#define		POS_CODE_qd		67		//	104						Ӣ�硢���ȡ��ꡢ�ɡ���
#define		POS_CODE_qe		68		//	98						���������ᡢ��������
#define		POS_CODE_Qg		69		//	19		����������		Ҷ��ƺ���ס������ġ������
#define		POS_CODE_qj		70		//	29						�ԡ�����������Ѯ
#define		POS_CODE_ql		71		//	1						��
#define		POS_CODE_qr		72		//	22						�ס�̳������������
#define		POS_CODE_qt		73		//	23		ʱ���й�����	�㡢�롢�ء��󡢹��ꡢ��
#define		POS_CODE_qv		74		//	37						�ѡ��ܡ��ء�Ȧ�������֡���
#define		POS_CODE_qz		75		//	12						�ɡ��ۡ���֡���
#define		POS_CODE_r		76		//	516		����			���䡢һ�С����ߡ�����Ϊʲô����
#define		POS_CODE_Rg		77		//	10		����������		�������������衢������
#define		POS_CODE_rr		78		//	50		�˳ƴ���		�˴ˡ������ҡ����ˡ����ˡ�ĳ
#define		POS_CODE_ry		79		//	28		���ʴ���		�ɡ�˭���ǡ�ʲô���Ρ�����
#define		POS_CODE_ryw	80		//	10						Ϊʲô��Ϊ�Ρ�զ����Ρ���ô
#define		POS_CODE_s		81		//	412		������			���ϡ�У�ڡ�У�ſڡ����ϡ�����
#define		POS_CODE_t		82		//	519		ʱ���			���ꡢ���硢���¡��˼ʡ�ÿ��һ
#define		POS_CODE_Tg		83		//	51		ʱ����			�񡢺����塢�֡�����
#define		POS_CODE_tt		84		//	45						ǰ����ǰ�塢ǰ�ˡ��ϡ��϶�
#define		POS_CODE_u		85		//	45		����			�ȡ��ߡ����⡢�㡢����˵��
#define		POS_CODE_ud		86		//	1		���ʵ�			��
#define		POS_CODE_ue		87		//	1		���ʵ�			��
#define		POS_CODE_ug		88		//	6						�ӡ��ۡ�����������
#define		POS_CODE_ui		89		//	1		���ʵ�			��
#define		POS_CODE_ul		90		//	1		���ʶ�			��
#define		POS_CODE_uo		91		//	1		���ʹ�			��
#define		POS_CODE_us		92		//	1		������			��
#define		POS_CODE_uz		93		//	1		������			��
#define		POS_CODE_v		94		//	20838	����			˵�������������������𡢱�̮
#define		POS_CODE_vb		95		//	1						����
#define		POS_CODE_vd		96		//	94		������			��ͷ��ƽ�С����š�ǿ�ơ�ѭ����Ľ��
#define		POS_CODE_Vg		97		//	904		������			ο����������Ϸ��ս
#define		POS_CODE_vi		98		//	5290	�����ﶯ��		ս�͡����ʡ��Ƶۡ���λ����Ϣ
#define		POS_CODE_vl		99		//	39		�����Թ�����	���ǡ����ڡ��㡢ͳ�ơ��Գ�
#define		POS_CODE_vn		100		//	8861	������			�������𵴡�ͦ�١�����������
#define		POS_CODE_vq		101		//	28						����������ȥ����������ȥ������
#define		POS_CODE_vt		102		//	7		���ﶯ��		���顢ѡ�١���������������
#define		POS_CODE_vu		103		//	41						���⡢�ᡢֵ�á�ֻ�ܡ�����
#define		POS_CODE_vv		104		//	1						����
#define		POS_CODE_vx		105		//	6		��ʽ����		�С����衢���С����ԡ�����
#define		POS_CODE_w		106		//	0		���			
#define		POS_CODE_wd		107		//	2		����			��
#define		POS_CODE_wf		108		//	2		�ֺ�			��
#define		POS_CODE_wj		109		//	3		���			��
#define		POS_CODE_wky	110		//	8		������			ȫ�ǣ���
#define		POS_CODE_wkz	111		//	8		������			ȫ�ǣ���
#define		POS_CODE_wm		112		//	2		ð��			��
#define		POS_CODE_wp		113		//	0					
#define		POS_CODE_ws		114		//	1		ʡ�Ժ�			��
#define		POS_CODE_wt		115		//	2		̾��			��
#define		POS_CODE_wu		116		//	1		�ٺ�			��
#define		POS_CODE_ww		117		//	2		�ʺ�			��
#define		POS_CODE_wy		118		//	4		����			
#define		POS_CODE_wyy	119		//	3		������			ȫ�ǣ���
#define		POS_CODE_wyz	120		//	3		������			ȫ�ǣ���
#define		POS_CODE_x		121		//	126		��������			�ס��¶������ɡ�������
#define		POS_CODE_y		122		//	49		������			���ɡ�ô��֮����Ҳ����
#define		POS_CODE_yg		123		//	4						�֡�����������
#define		POS_CODE_z		124		//	1733	״̬��			���ۡ���Ȼ���ϳˡ���������и
#define		POS_CODE_zg		125		//	2						��
#define		POS_CODE_START	126		//	0		��������Ԥ��	Pos-2=START
#define		POS_CODE_END	127		//	0		��������Ԥ��	Pos+2=END



//����ֵ����ǰ׺
#define Q0
#define Q1
#define Q2
#define Q3
#define Q4
#define Q5
#define Q6
#define Q7
#define Q8
#define Q9
#define Q10
#define Q11
#define Q12
#define Q13
#define Q14
#define Q15
#define Q16
#define Q17
#define Q18
#define Q19
#define Q20
#define Q21
#define Q22
#define Q23
#define Q24
#define Q25
#define Q26
#define Q27
#define Q28
#define Q29
#define Q30
#define Q31
#define Q32
#define Q33
#define Q34
#define Q35
#define Q36
#define Q37
#define Q38
#define Q39
#define Q40
#define Q41
#define Q42
#define Q43
#define Q44
#define Q45
#define Q46
#define Q47
#define Q48
#define Q49
#define Q50
#define Q51
#define Q52
#define Q53
#define Q54
#define Q55
#define Q56
#define Q57
#define Q58
#define Q59
#define Q60
#define Q61
#define Q62



//*****************************************	 �ṹ�嶨��  ************************************************************************

/* RESAMPLE   �ṹ���� */
typedef struct _tagReSampleData
{
    emInt16 PrePcm[9];		// �洢����ǰ9��PCMԭʼ������ORDER=4 PrePcmLen = 2*ORDER+1 
	emInt16 gain;			// ����
	emInt16 pi_freq;		// ��СƵ��
	emUInt16 des_int;		// Ŀ��������������������֮��������������
	emUInt16 ori_int;		// ԭʼ������������������֮��������������
	emUInt16 d_last;		// Ŀ��������������λ��	
} ;

typedef struct _tagReSampleData TReSampleData, emPtr PReSampleData;

#define MAX_CN_LAB_LINE_NUM		((emUInt16)(MAX_HANZI_COUNT_OF_LINK*1.4))				//�������Lab��		(���1.4����׼����sil��pau)

#define MIN_EN_LAB_LINE_NUM		(MAX_SYNTH_ENG_LAB_COUNT)

#if EM_ENG_AS_SEN_SYNTH
	#define MAX_EN_LAB_LINE_NUM		(400)												//�����Ӻϳɣ�Ӣ�����Lab��
#else
	#define MAX_EN_LAB_LINE_NUM		(MAX_PHONE_INWORD + MAX_SYNTH_ENG_LAB_COUNT + 10)	//�����ʺϳɣ�Ӣ�����Lab��		//���ӣ�[g2]bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
#endif

#define LAB_CN_LINE_LEN       				63									//���ģ�Lab�������
#define LAB_EN_LINE_LEN       				99									//Ӣ�ģ�Lab�������


struct tagLabRTGlobal
{
	emByte emPtr m_LabRam[ MAX_CN_LAB_LINE_NUM ];											//HMM�ϳ�ǰ��Lab��Ϣ������ȡ���ģ�Ӣ��������������
	emByte		m_LabOneSeg[MAX_SYNTH_ENG_LAB_COUNT+2][LAB_EN_LINE_LEN];					//�洢һ���ϳɵ�Ԫ��Lab������ȡ���ģ�Ӣ��������������
	emUInt16	m_MaxLabLine;																//HMM�ϳ�ǰ�����lab��������������		//zz 2014-6-26	һ���е�����lab�����ܳ���256  ���磺100��w

};
typedef struct tagLabRTGlobal TLabRTGlobal, emPtr PLabRTGlobal;

struct promptInfo
{
	emInt16		nPromptDecodeType;
	emInt32		nPromptLen;
	emInt32		nPromptOffset;	
};

struct TextItem
{
	emUInt8     Pos;						//���� 
	emUInt8     Len;						//�ʳ� 
	emUInt8     TextType;					//��������   1.���֣�����㣩 3. ��ĸ
	emUInt8     BorderType	;				//�߽����� 0����  1��LW   2��PW  3��PPH  
	emByte		nBiaoDian[2];				//���ʺ�����ı�㣨������;��1.�������⴫���ȶ����ֵ��б� 2. �ٺ�ð�ŵ��б�
};


struct ForcePPHInfo
{
	emUInt8		nStartIndex;		//ǿ��PPH�ģ��ɶԱ���ǰ����λ��
	emUInt8		nSylLen;			//ǿ��PPH�ģ��ɶԱ���ǰ����֮������ڸ���
	emUInt8		nCount;				//ǿ��PPH�ģ����������������ڸ����ģ�
	emUInt8		bIsStartCount;		//�Ƿ񣺿�ʼ�ɶԱ���ǰ����֮������ڸ����ļ���
};

struct RearOffsetStr
{
	emInt32	uvpMod;			//ƫ��������������ת����ģ��, changed by naxy19
	emInt32 sydMod;			//ƫ����������ʱ��ģ��, changed by naxy19
	emInt32	durMod;			//ƫ������ʱ��ģ��
	emInt32	mgcWuw;			//ƫ��������Ƶ�������, changed by naxy17
	emInt32	mgcMod;			//ƫ������Ƶ��ģ��
	emInt32	lf0Mod;			//ƫ��������Ƶģ��
	emInt32	bapMod;			//ƫ������bapģ��
	emInt32	gv;				//ƫ������GV����
};

typedef struct RearOffsetStr emPtr PRearOffsetStr;

typedef struct _emRearG
{
	emInt16		frame_length;		/* ֡��, default = 80 */
	float		Speech_speed;		/* ���� */
	emUInt16	nModelSample;		/* ������ */
	emInt8		nModelFloatLen;		/* �������ģ�͵�floatҶ�ڵ���ռ���ֽ�����һ��Ϊ2��4*/
	emInt8		nModelFloatLeafAdd; /* �������ģ�͵�Ҷ�ڵ㳤����ӣ� ��floatռ2�ֽ�ʱ������ֵ=2�������⴦����Ƶģ��0�׾�ֵռ4�ֽ�,Ƶ��ģ������ά0�׾�ֵռ4�ֽ�*/
	float		f0_mean;			/* ��Ƶ�������� */
	float		alpha;				/* default = 0.42 */
	float		gamma;				/* default = 0 */
	float		beta;				/* beta = 0.4 */	

	emBool		bIsVQ;				//Ƶ��ģ���Ƿ�ʸ������   -- �������
	emBool		stage;				//Ƶ������		0��MGC  1��LSP	  -- �������
	emBool      bIsStraight;		//�õ��Ƿ���Straightѵ������

	emInt16		nMgcLeafPrmN;		//�׵�ҳ�ڵ�Ĳ�����Ŀ

	struct RearOffsetStr	offset_cn, offset_eng;		//�����Դƫ����,cn-����ģ�ͣ�eng-Ӣ��ģ��

}emRearG;

typedef struct _emInterParam
{
	emInt16		nState;					/* ״̬��, default = 10 */
	emInt16		length;					/* ÿ���ϳ�С�ε�֡��, max = 720 */
	emInt16		width;					/* ��̬�����, default = 3*/
	emInt16		static_length;			/* ��̬����ά��, default = 25*/
	float		msd_threshold;			/* msd��ֵ, default = 0.4 */
	emInt16		win_coefficient[3][3];	/* ��ϵ��, default = [0 1 0; -0.5 0 0.5; 1 -2 1]   ȫ����2���������� */
}emInterParam;


struct tagControlSwitch
{
	emInt16				m_nCodePageType ;					//�����ı�����ҳ����

	emInt16				m_nRoleIndex;						//������				[m*]
	emInt8				m_bUsePromptsPolicy	;				//��ʾ����Ԥ¼���������[x*] 
	emInt8				m_nReadDigitPolicy 	;				//���ִ������			[n*]
	emInt8				m_bYaoPolicy ;						//��һ���Ķ�����		[y*] 
	emInt8				m_nSpeakStyle;						//�������				[f*]
	emInt8				m_nVoiceSpeed ;						//�����ٶ�				[s*]
	emInt8				m_nVoicePitch;						//�������				[t*] 
	emInt8				m_nVolumn; 							//������С				[v*]
	emInt8				m_bXingShi 	;						//���ϲ���				[r*] 
	emInt8				m_bShiBiePinYinPolicy;	            //ʶ��ƴ��				[i*] 
	emInt8				m_nManualRhythm ;					//���ɱ�ע				[z*] 
	emInt8				m_bPunctuation;						//��㴦�����			[b*]		//���Ƕ��е�

	emInt8				m_nVoiceMode;						//��Чģʽ				[e*]

	emInt8				m_nLangType ;						//����					[g*] 
	emInt8				m_bZeroPolicy	 ;					//Ӣ��0�Ķ���			[o*] 
	emInt8				m_bTimePolicy	 ;					//Ӣ��ʱ��Ķ���		[c*]		//Zz


	emInt8				m_bYiBianYinPolicy;					//һ�ı������ԣ�0��������ֹ�� 1�������򿪣�
	emInt8				m_bIsPhoneNum ;						//������ܳ��ֵ����ִ�������


	emInt8				m_bIsHandleToBiFen ;	            //�Ƿ���ɱȷ֣�����ʱΪ1������Ϊ0

	emInt8				bIsPauseNoOutPutEndSil;				//����[p*]ͣ�ٲ��������ľ�βsil
	emInt8				bIsPauseNoOutPutStartSil;			//����[p*]ͣ�ٲ��������ľ���sil

	emInt8				bIsReadDian;						//��.���Ƿ����
};

typedef struct tagControlSwitch TControlSwitch, emPtr PControlSwitch;

struct emHTTS_G
{
	emPointer			pCBParam;							//�û��ص�����  �����Ƶ�����ļ���ָ��

	emPointer			fResFrontMain;						//ָ��ǰ����Դ�ĵ�ָ��
	emPointer			fResCurRearMain;					//ָ������Դ�ĵ�ָ��
	emCBReadRes			m_cbReadRes;						//��ȡ��Դ�Ļص�����ָ��

	emRearG				*pRearG;
	emInterParam		*pInterParam;
	emBool				m_FirstSynthSegCount;				//���κϳ��ı��ڼ����ֶμ����������磺������1�����ӵĵ�1��С�ֶ� = 1��
	emBool				m_bIsSynFirstAddPau;				//�ϳɾ�ĺ�ˣ��Ƿ��Ѻϳ����׸��ֶ�ǰ��ӵ���ʱpau
	emBool				m_bIsSynFirstTxtZi;					//�Ƿ��ǽ���ϳɵĺϳ�����


	//emBool				m_bRunning;							//״̬���Ƿ��ںϳ�
    emBool				m_bStop	;							//״̬���Ƿ�ֹͣ�ϳ�


	TControlSwitch		m_ControlSwitch;					//���ƿ��أ����ı���ǵȿ��ƿ��أ�

	emByte				m_ShuZiBuffer[60] ;					//���Ҫ�����ĺ��֣��������֣�����@�ȣ�		//ֻҪ�����Ǻ��ֱ������

	emInt16				m_nDataType;						//�ִ��е�1���ֵ��ַ�����
	emTTSErrID			m_nErrorID;							//�������ͣ�ֻ����ʱ�ã���ʱ����������ܻᱻ���������ı�

	emBool				m_bIsMuteNeedResample;				//�����Ƿ����ز���
	emUInt32			m_CurMuteMs;						//�洢��ǰ�����ĺ��������������ʱ��
	emUInt32			m_NextMuteMs;						//�洢�´ξ����ĺ��������������ʱ��
	emBool				m_bIsStartBoFang;					//�Ƿ��Ѿ�����


	emInt16				m_nCurIndexOfAllHanZiBuffer; 		//��ǰ������HanZiBuffer������λ��
	emInt16				m_nCurIndexOfEnBuffer; 				//��ǰ������Ӣ��Buffer������λ��

	emUInt16			m_nNeedSynSize;						//��Ҫ�ϳɵ��ı��ֽ���

	emCPointer			m_pc_DataOfBasic ;					//�Ӳ����л�ȡ��ԭʼ�ı����ݵ�ָ��      //������������ʶ�����
	emUInt32			m_nCurIndexOfBasicData; 			//��ǰ�����Ļ����ı����ݵ�����λ��

	emPByte				m_pDataAfterConvert;				//����ת������ı����ݵ�ָ�룬����ֽڸ���Ϊ��CORE_CONVERT_BUF_MAX_LEN����END_WORD_OF_BUFFER��ʾ����
	emInt16				m_nCurIndexOfConvertData; 			//��ǰ�����ı���ת������ı����ݵ�����λ��
	emBool				m_bConvertWWW;						//��ǰת���ı������Ƿ��ѳ���WWW.

	emInt8				m_nPrevSenType;						//ǰ����������

	emInt16				m_nHanZiCount_In_HanZiLink; 		//���������еĺ��ָ���

	emBool				m_bIsErToLiang;						//�Ƿ񡰶������ɡ�����

	emBool				m_bResample;						//����������Ƿ���Ҫ�ز���

	emPByte				m_pPCMBuffer;
	emInt32				m_pPCMBufferSize;

	emPByte				m_Info;

	emInt16				m_ChaBoCount;						//�岥����

	struct promptInfo	m_structPromptInfo[1];	
	struct promptInfo	*m_nCurPointOfPcmInfo; 				//��ǰ������ָ��PcmInfo��ָ��


	emInt8				m_ManualRhythmIndex[MAX_MANUAL_PPH_COUNT_IN_SEN];		//һ�仰�У��ֶ���������PPH����������
	emInt8				m_CurManualRhythmIndex;									//һ�仰�У��ֶ��������ɵ�ǰPPH������

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	emBool				bTextTailWrap;					//���н�β�ǻ��з���Ϊ��־��ӡ������
#endif

	emBool              m_HaveEnWordInCnType;			//�����ľ����Ƿ���Ӣ�ĵ������ϳɵ�״̬�� 0����Ӣ�ĵ���  1����Ӣ�ĵ������ϳɾ��׶Σ� 2����Ӣ�ĵ������ϳɾ��еļ����Σ� 3����Ӣ�ĵ������ϳɾ�β��

	emUInt8				m_nParaStartLine;				//���ƴӵڼ������ؿ�ʼ�ϳ�
	emUInt8				m_nEnSynCount;					//Ӣ����ѭ�����ú�˺ϳɵļ���

	float				m_TimeRemain;					//��ź��ʱ����������ֵ

};



emExtern emInt8					g_bIsUserAddHeap;					
emExtern struct emHTTS_G		*g_hTTS;								
emExtern PLabRTGlobal			g_pLabRTGlobal;						
emExtern struct tagIsHeap		*g_pHeap;							
	
emExtern struct TextItem		*g_pTextInfo;						
emExtern emByte					*g_pText;							
emExtern emInt16				*g_pTextPinYinCode;					
emExtern emUInt8				g_nLastTextByteIndex, g_nCurTextByteIndex, g_nNextTextByteIndex, g_nP1TextByteIndex, g_nP2TextByteIndex;  

emExtern struct ForcePPHInfo	g_ForcePPH;		

emExtern emUInt32				(*g_pOutputCallbackFun)( emPointer , emUInt16,emPByte,emSize);		//��Ƶ����ص�����
emExtern emUInt32				(*g_pProgressCallbackFun)( emPointer , emUInt32 ,emSize);			//���Ȼص�����


#if DEBUG_LOG_SWITCH_ERROR
emExtern FILE					*g_fLogError;						//������־�ļ�ָ��
#endif

#if EM_USER_VOICE_RESAMPLE
	emExtern TReSampleData		g_objResample[1];					// �ز������� 
#endif

emExtern emUInt32				emTTS_OutputVoice( emPointer , emUInt16,emPByte,emSize);		//��Ƶ���ͳһ����
emExtern emTTSErrID  emCall FirstHandleHanZiBuff(emBool bIsSylTailSil);

emExtern void emCall	InitTextMarkG()	;
emExtern emInt8 JudgePrompt(emInt16 nSumOfYingWen,emPByte strDataBuf, emInt16 nCurIndexOfBuf);
emExtern emInt8 JudgeCtrlMark(emInt16 nSumOfFuHao,emPByte strDataBuf, emInt16 nCurIndexOfBuf);


emExtern void emCall HandleCnSen();

emInt8	DecideSenType();

emExtern emUInt32  g_total_time,g_t1_time;


#if DEBUG_LOG_SWITCH_HANZI_LINK|DEBUG_LOG_POLY_TEST
	struct	pinyinTable
	{
		emInt32 		nLineCount;
		emByte	pPinYin[3000][8];
		emInt32 		pCode[3000];
	};

	struct pinyinTable     pinyinTableLog;	

	struct	posTable
	{
		emInt32 		nLineCount;
		emByte	pPos[200][6];
		emInt32 		pCode[200];
	};

	struct posTable     posTableLog;	

#endif


#ifdef __cplusplus
}
#endif

#endif	/* #define _EMTTS__EM_TTS_COMMON__H_ */




