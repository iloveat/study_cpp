#include <math.h>
//#include <mathcalls.h>					//sqb  2017/6/1
#include "emPCH.h"
#include "Rear_Common.h"
#include "emMath.h"
//#include "emVioceChange.h"



//��ĸ����
#define		INITIAL_OTHER				0		//������ĸ
#define		INITIAL_ZERO_ZHUO			1		//������ĸ

//λ�����ͣ������ּ����¾�ģ�壩
#define		POS_LEN2_PPH_HEAD			0		//����PPH������
#define		POS_LEN2_PPH_TAIL			1		//����PPH����β
#define		POS_LEN2_OTHER				2		//����

//λ�����ͣ�������ģ�壩
#define		POS_NO_LIGHT_SEN_TAIL		0		//��β							��ֻͳ��4�ֻ����ϵ����ɶ��
#define		POS_NO_LIGHT_PPH_TAIL		1		//�����ɶ���β	����������β��  ��ֻͳ��4�ֻ����ϵ����ɶ��
#define		POS_NO_LIGHT_OTHER			2		//����

//λ�����ͣ���������ģ�壩 �������֣���β�;�β��
#define		POS_LIGHT_PPH_TAIL			0		//��β�� ����
#define		POS_LIGHT_NO_PPH_TAIL		1		//����β������



// ������ʹ�õ�ȫ�ֱ���
float					g_gv[2];									//���ڱ����ƵGV����
float					*g_Matrix;									//Ƶ�׷����������
PRearRTGlobal			g_pRearRTGlobal;							//���ȫ��ʱ��
PRearRTCalcFilter		g_pRearRTCalcFilter;						//��˽����˲�ʱ��
PRearRTCalc				g_pRearRTCalc;								//��˽���ʱ��
float syl_uvp[MAX_SYNTH_LAB_COUNT+2];								//��������ص�������ת������߽��, changed by naxy19
emByte				*g_tree;										//������ʱ����һ�ž�����������
emByte				*g_AllTree;										//����22������
emInt32				g_AllTreeOffset[32];							//����22��������ƫ��������(1��:����ת��������1��:ʱ������10��:Ƶ������10��:��Ƶ��)
float *g_lf0param;													//���ڱ����Ƶ����������
float *g_bapparam;												//bap����
PRearOffsetStr	g_pRearOffset;								//���С��Դƫ��ָ�� fhy 121225
emBool			g_bRearSynChn;								//�ϳ����ı�־

//��DCT����
emInt32  BackDct( float *x, emInt32  m, float *c, emInt32  n )
{
	emInt32  i, j;

	LOG_StackAddr(__FUNCTION__);

	for(i=0;i<m;i++) 
	{
		x[i] = 0.0;
		for(j=1;j<n;j++) {
			//���棺x[i] += c[j] * cos(PI*j*(i+0.5)/m);  ����ʹ��cos������
			//x[i] += c[j] * emFloat_ShiftDown( F_cos( (emUInt16) (PI*j*(i+0.5)/m)<<16 ) ,15 );  //���У��滻����β��������
			x[i] += c[j] * cos(PI*j*(i+0.5)/m);
		}
		x[i] += c[0] * 0.5;

		x[i] = x[i]*(539.207247-52.1242348)+52.1242348 ;	//ȫ�����һ��

		x[i] = x[i]+g_hTTS->pRearG->f0_mean;  //2012-03-27�� �����л�Ƶģ����ϻ�Ƶ�����Ŀ���  �������Ƶ���ƶԵ�˫�ֵȲ������ã�
	}
	return 0;
}

#if HTRS_REAR_DEAL_CHANGE_MSD     //С���޸�

//�жϣ����غ�������
emInt8 GetPhoneType(emInt8 pp)
{
	emInt8 nType = 3;

	if(    pp==EngPP_l  || pp==EngPP_r  || pp==EngPP_w  || pp==EngPP_y  || pp==EngPP_m  || pp==EngPP_n  || pp==EngPP_ng || pp==EngPP_aa
		|| pp==EngPP_ae || pp==EngPP_ah || pp==EngPP_ao || pp==EngPP_aw || pp==EngPP_ay || pp==EngPP_eh || pp==EngPP_er || pp==EngPP_ey 
		|| pp==EngPP_ih || pp==EngPP_iy || pp==EngPP_ow || pp==EngPP_oy || pp==EngPP_uh || pp==EngPP_uw || pp==EngPP_ax)
		nType = 1;

	if(   pp==EngPP_th  || pp==EngPP_f  || pp==EngPP_s  || pp==EngPP_sh  || pp==EngPP_ch )
		nType = 2;

	return nType;
}
#endif


//���Ԥ����  �������ⷢ���˵�
void RearPrevHandle()		
{
	float   fVirtualResampleScale = 1.0;
	float   fVirtualDurScale = 0;
	float   fVirtualF0Scale = 0;
	emByte	nFloatType;

	LOG_StackAddr(__FUNCTION__);

	//g_hTTS->m_ControlSwitch.m_nRoleIndex = emTTS_USE_ROLE_Virtual_54;
	//���õ�ǰ�����˵�offset�� ���ⷢ���˻������ã��ز���ϵ����ʱ������ϵ������Ƶ����ϵ��
	switch( g_hTTS->m_ControlSwitch.m_nRoleIndex )
	{
		case emTTS_USE_ROLE_XIAOLIN:	//С��
			fVirtualResampleScale = 1.0;					//1.0
			fVirtualDurScale = DUR_SCALE_ROLE_Virtual_52;						//0
				fVirtualF0Scale = 0.0;
				g_hTTS->m_bResample				= emFalse;			//��
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;	
				break;
		case emTTS_USE_ROLE_Virtual_51:	
				fVirtualResampleScale = RESAMPLE_SCALE_ROLE_Virtual_51;
				fVirtualDurScale = DUR_SCALE_ROLE_Virtual_51;
				fVirtualF0Scale = F0_SCALE_ROLE_Virtual_51;
				g_hTTS->m_bResample = emTrue;
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;	
				break;
		case emTTS_USE_ROLE_Virtual_52:	
				fVirtualResampleScale = RESAMPLE_SCALE_ROLE_Virtual_52;
				fVirtualDurScale = DUR_SCALE_ROLE_Virtual_52;
				fVirtualF0Scale = F0_SCALE_ROLE_Virtual_52;
				g_hTTS->m_bResample = emTrue;
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;
				break;
		case emTTS_USE_ROLE_Virtual_53:	
				fVirtualResampleScale = RESAMPLE_SCALE_ROLE_Virtual_53;
				fVirtualDurScale = DUR_SCALE_ROLE_Virtual_53;
				fVirtualF0Scale = F0_SCALE_ROLE_Virtual_53;
				g_hTTS->m_bResample = emTrue;
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;
				break;
		case emTTS_USE_ROLE_Virtual_54:	
				fVirtualResampleScale = RESAMPLE_SCALE_ROLE_Virtual_54;
				fVirtualDurScale = DUR_SCALE_ROLE_Virtual_54;
				fVirtualF0Scale = F0_SCALE_ROLE_Virtual_54;
				g_hTTS->m_bResample = emTrue;
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;
				break;
		case emTTS_USE_ROLE_Virtual_55:
				fVirtualResampleScale = RESAMPLE_SCALE_ROLE_Virtual_55;
				fVirtualDurScale = DUR_SCALE_ROLE_Virtual_55;
				fVirtualF0Scale = F0_SCALE_ROLE_Virtual_55;
				g_hTTS->m_bResample = emTrue;
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;
				break;		
		default:								
				g_Res.offset_RearVoiceLib_Cur = g_Res.offset_RearVoiceLib03;	
				break;
	}

#if EM_OPEN_HTRS_BIANYIN					//������Ӣ��Htrs������ʶ�
	if( !g_bRearSynChn )		//���ǣ�Ӣ��
	{
		if( g_hTTS->m_ControlSwitch.m_nRoleIndex == emTTS_USE_ROLE_Virtual_53)	//ѡ�������ǣ���������HtrsӢ�ĵ�ԭ������
		{
			fVirtualResampleScale = 1.0;
			fVirtualDurScale      = 0.0;
			fVirtualF0Scale       = 0.0;
			g_hTTS->m_bResample = emFalse;
		}
		else																	//ѡ�����������ˣ���HtrsӢ�ĵ�����������֮����
		{
			fVirtualResampleScale += HTRS013_ENG_RESAMPLE_BASE;
			fVirtualDurScale      += HTRS013_ENG_DUR_BASE;
			fVirtualF0Scale       += HTRS013_ENG_F0_BASE;
			g_hTTS->m_bResample = emTrue;
		}
	}
#endif



#if EM_USER_VOICE_RESAMPLE
	if( g_hTTS->m_bResample == emTrue)
	{
		//�������ⷢ���˵��ز���ϵ���������ز����ĳ�ʼ��
		ReSample_Init(g_objResample, g_hTTS->pRearG->nModelSample, (emUInt16 )(g_hTTS->pRearG->nModelSample * fVirtualResampleScale));				
	}
#endif




	//�������ٵ�������    sqb
	if( g_bRearSynChn)
		g_hTTS->pRearG->Speech_speed = (float)(g_hTTS->m_ControlSwitch.m_nVoiceSpeed-emTTS_SPEED_NORMAL)*0.04 + 1 - WL_REAR_DEAL_SPEED_FACTOR + fVirtualDurScale;
	//else
		//g_hTTS->pRearG->Speech_speed = (float)(g_hTTS->m_ControlSwitch.m_nVoiceSpeed-emTTS_SPEED_NORMAL)*0.04 + 1 - HTRS_REAR_DEAL_SPEED_FACTOR + fVirtualDurScale;

	if( g_bRearSynChn)	{
#if WL_REAR_DEAL_VIRTUAL_QST		//�������ⷢ���˵�һЩ����	

	//Ŀǰ���ⷢ����[m55]�ȵ���������ʱ������Ϊ0.47���ϳɻ������������ڴ治������
	//��ʱ��һ��������Լ������Լ����0.60���0.57Ҳ��������
	if( g_hTTS->pRearG->Speech_speed < 0.60)
	{
		g_hTTS->pRearG->Speech_speed = 0.60;	
	}

	if(    g_hTTS->m_ControlSwitch.m_nRoleIndex == emTTS_USE_ROLE_Virtual_54
		|| g_hTTS->m_ControlSwitch.m_nRoleIndex == emTTS_USE_ROLE_Virtual_55)
	{
		//��ֹ���������ⷢ�����������ʱ��оƬ�ٶȸ����ϣ���������  hyl  2012-04-10
		//���磺[m55][s10]��һĦ��������ʮ����ǧ����΢�����ӡ��Ҳ�֪����������ʲô��[m3][d]
		if( g_hTTS->pRearG->Speech_speed > 0.84 )
			g_hTTS->pRearG->Speech_speed = 0.84;
	}
#endif

#if WL_REAR_DEAL_DCT_SHORT_SEN					//<����>���Ķ̾䴦��ģʽ(��ĸ�Ĳ�����)��ר�Ŵ������־䣬���־�
	if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] == 1 )		//Ϊ���־�
	{
		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//ԭʼ�����ǡ����֡���
		{
			//��Ӣ����ĸ�Ĳ�����
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] <= TONE_CN_sil_pau )	//���ֲ�����ĸ	
			{
				//���־�ģʽ: �������־�ʱ��				
				switch( g_pLabRTGlobal->m_LabRam[1][RLAB_C_SM] )	//��ĸ	
				{
					case INITIAL_CN_b:		g_hTTS->pRearG->Speech_speed *= 0.65;	break;		//������ĸ�ǣ�b  	
					case INITIAL_CN_d:		g_hTTS->pRearG->Speech_speed *= 0.60;	break;		//������ĸ�ǣ�d	
					default:	g_hTTS->pRearG->Speech_speed *= 0.85;	break;
				}	
				if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] == TONE_CN_light )	//����Ϊ����
					g_hTTS->pRearG->Speech_speed *= 1.2;
				
			}
		}
	}
#endif
	}	// end of g_bRearSynChn

	
	//���û�Ƶ��������
	if((g_hTTS->m_ControlSwitch.m_nVoicePitch-emTTS_PITCH_NORMAL)>=0)
		g_hTTS->pRearG->f0_mean = 64 * ((float)(g_hTTS->m_ControlSwitch.m_nVoicePitch-emTTS_PITCH_NORMAL) / emTTS_PITCH_NORMAL );
	else
		g_hTTS->pRearG->f0_mean = 64  * ((float)(g_hTTS->m_ControlSwitch.m_nVoicePitch-emTTS_PITCH_NORMAL) / emTTS_PITCH_NORMAL );		

	g_hTTS->pRearG->f0_mean -=  100 * fVirtualF0Scale;

	if( g_bRearSynChn) {
#if WL_REAR_DEAL_VIRTUAL_QST		//�������ⷢ���˵�һЩ����	

	//Ŀǰ���ⷢ����[m54]�ȵ�������[t0]ʱ������Ϊ-124����������ʧ�棨��Ƶ���ͣ���
	//��ʱ��һ��������Լ��  ��Ŀǰ��ϵ�����100�����
	if( g_hTTS->pRearG->f0_mean < -90)			//���磺[m54][t0]�������ڷ񶨼�����Ϊ�ȴ�ͳ����[m3][d]
		g_hTTS->pRearG->f0_mean = -90;	
#endif
	}	// end of g_bRearSynChn


}

#if WL_REAR_DEAL_DCT_LIGHT			//�����������DCTģ�壺�������ɶ���β���������־�����ϣ���������ĸ��
void ChangeLightDctF0(   emInt32  nCurStartTime,				//�ֶκϳ���: ���ֿ�ʼ��֡��
						   emInt32  nSylTotalT,				//�ֶκϳ���: ���ֵ���ʱ��
						   emInt32  m1)	
{
	emInt16  n, t;
	emInt16 nSylStartT = 0, nSylEndT= 0;
	emInt16  nIndex, nUStart;
	emInt32  nToneMe,nTonePrev ,nToneNext,nEffectTone, nInitialType, nPosType ;
	float  lf0Float[150];
	emBool bModifyDct;


	////////////////////////  ������ƵDCTģ��˵��   //////////////////////////////////
	//
	//	�е����� = λ�����ͣ�*10  + 	��ĸ����*5  + ǰ����

	//	λ�����ͣ�2�� �� ��ĸ���ͣ�2�� �� ǰ����5�֣�

	//	λ�����ͣ�ȡֵ��Χ��    0�� ��β��β		1�� ����β��β
	//	��ĸ���ͣ�ȡֵ��Χ��	0�� ������ĸ		1�� ������ĸ	

	//	ǰ��
	//		0:��ʾ��������	1:��ʾ1����	2:��ʾ2����	3:��ʾ3����	4:��ʾ4��
	////////////////////////  ������ƵDCTģ��˵��   //////////////////////////////////
	

								
	float fTwoDctValue[20][7]={											//���	����	λ������	��ĸ����	ǰ��	���ڸ���
 
	//�����֣���β�;�β ��Ŀǰ��10��û�ã�����β��β��//modified by wangcm 2012-03-01
	{0.503730,0.040799,0.011756,0.00905,0.000793,0.004522,0.001254},			//1,����,0,0,0,66, //modified by wangcm 2012-03-16  ԭ����0.42373,0.050799,0.011756,0.00905,0.000793,0.004522,0.001254
	{0.533730,0.040799,0.011756,0.00905,0.000793,0.004522,0.001254},			//2,����,0,0,1,460, //hyl 2012-04-11
	{0.510243,0.048419,0.011539,0.005121,-0.001734,0.002402,-0.000913},			//3,����,0,0,2,394,
	{0.471737,0.000018,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //4,����,0,0,3,299,
	{0.475627,0.040475,0.014229,0.009790,0.000981,0.003331,0.000462},		    //5,����,0,0,4,606, //modified by wangcm 2012-03-17 ԭ����0.435627,0.032475,0.014229,0.00979,0.000981,0.003331,0.000462
	{0.485627,0.024799,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //6,����,0,1,0,48,
	{0.665696,0.084379,0.024148,-0.009103,-0.005961,0.002093,-0.000957},		//7,����,0,1,1,218,
	{0.556611,0.017921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//8,����,0,1,2,213, //modified by wangcm 2012-03-20
	{0.465627,0.023813,0.013717,0.006116,0.002598,0.001848,0.000532},	     	//9,����,0,1,3,153,
	{0.46267,0.037912,0.011972,0.000746,-0.001285,0.004285,-0.000675},			//10,����,0,1,4,241,
	{0.612275,0.025849,0.010634,0.005307,0.002356,0.001626,-0.000046},			//11,����,1,0,0,150,
	{0.758347,0.063361,0.020645,0.008887,0.002477,0.002559,0.000512},			//12,����,1,0,1,1074,
	{0.794895,0.024799,0.000293,0.002896,-0.000406,0.000886,-0.000628},			//13,����,1,0,2,927,
	{0.602648,-0.006668,0.002722,0.003425,0.001179,0.001597,0.000195},			//14,����,1,0,3,859,
	{0.597253,0.023813,0.013717,0.006116,0.002598,0.001848,0.000532},			//15,����,1,0,4,1603,
	{0.793275,0.034155,0.006766,-0.002332,-0.000913,0.003127,-0.000245},		//16,����,1,1,0,20,
	{0.897771,0.087555,-0.005318,-0.006805,-0.002271,0.002733,-0.001203},		//17,����,1,1,1,332,
	{0.822973,-0.012072,-0.030094,-0.006535,-0.005222,0.000692,-0.002446},		//18,����,1,1,2,304,
	{0.630488,-0.038981,0.000324,-0.000515,-0.002221,0.00174,-0.000446},		//19,����,1,1,3,258,
	{0.682047,0.050011,0.013739,-0.000625,0.00085,0.002978,0.000981}};			//20,����,1,1,4,377,


	//����������ڣ�ĳЩ������β��������ģ���滻
	float fPPHTailSpecialDctValue[20][7]={										//				   ǰ��

	{0.413730,0.055799,0.011756,0.00905,0.000793,0.004522,0.001254},			//0, ����, zhe5		0(����)
	{0.541498,0.078405,0.01005,0.010376,0.003475,0.005092,0.001742},			//1, ����, zhe5		1
	{0.450243,0.063419,0.011539,0.005121,-0.001734,0.002402,-0.000913},			//2, ����, zhe5		2
	{0.461737,0.019018,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //3, ����, zhe5		3
	{0.465627,0.049475,0.014229,0.009790,0.000981,0.003331,0.000462},		    //4, ����, zhe5		4

	{0.483627,0.019799,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //5, ����, men5		0(����)
	{0.663696,0.079379,0.024148,-0.009103,-0.005961,0.002093,-0.000957},		//6, ����, men5		1
	{0.554611,0.012921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//7, ����, men5		2
	{0.504611,0.012921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//8, ����, men5		3
	{0.46067, 0.032912,0.011972,0.000746,-0.001285,0.004285,-0.000675},			//9, ����, men5		4

	{0.465627,0.014799,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //10, ����, ma5		0(����)
	{0.645696,0.072379,0.024148,-0.009103,-0.005961,0.002093,-0.000957},		//11, ����, ma5		1
	{0.521611,0.007921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//12, ����, ma5		2
	{0.536611,0.007921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},     	//13, ����, ma5		3
	{0.44267, 0.027912,0.011972,0.000746,-0.001285,0.004285,-0.000675}};		//14, ����, ma5		4


	LOG_StackAddr(__FUNCTION__);

	nUStart = nCurStartTime;
	for( t = nCurStartTime; t< (nCurStartTime+nSylTotalT-5); t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == UNVOICED)
		{
			nUStart = t;				//������������һ�����ڵ�һС��������Ƶ
			break;
		}
	}

		
	//�ҵ������ڵ�������Ƶ����ʼ֡���ͽ���֡��
	nSylStartT = nUStart;
	nSylEndT = (nCurStartTime+nSylTotalT -1);
	for( t = nUStart; t< nCurStartTime+nSylTotalT; t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED)
			nSylStartT = t;

		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t+1] == UNVOICED)
			nSylEndT = t;
	}	


	//����Ѱ�ҵ�DCT��Ƶģ�������
	emMemSet(lf0Float,0,sizeof(lf0Float));
	if(    g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 1	 					//���־� �� ����
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_SM] != INITIAL_CN_letter				//ǰ���ڲ�ΪӢ����ĸ
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_SM] != INITIAL_CN_letter)				//�����ڲ�ΪӢ����ĸ
	{
		//�����Ƶģ�������
		nToneMe =   g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE];
		nTonePrev = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE];
		nToneNext = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_TONE];
		if( nToneMe == TONE_CN_light )		
			nToneMe = 0;		//����ת��
		if( nTonePrev == TONE_CN_light )	
			nTonePrev = 0;		//����ת��
		if( nToneNext == TONE_CN_light )	
			nToneNext = 0;		//����ת��

		nInitialType = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM];
		if( nInitialType == 3 || nInitialType == 13 || nInitialType == 14 || nInitialType == 15 || nInitialType == 18 )
			nInitialType = INITIAL_ZERO_ZHUO;		//������ĸ�� a,o,e,w,y��ͷ��,13:l  14:m  15:n  18:r 
		else
			nInitialType = INITIAL_OTHER;			//������ĸ

		//�����֣���β�;�β
		//��ֵ��λ������
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0)		
			nPosType = POS_LIGHT_PPH_TAIL;		//���ɶ���β
		else
			nPosType = POS_LIGHT_NO_PPH_TAIL;		//�����ɶ���β

		nEffectTone = nTonePrev;

		//��Ӱ������Ϊ��sil����pau����������ĸ����
		if( nEffectTone == TONE_CN_sil_pau || nEffectTone == TONE_CN_letter)	//hyl  2012-03-29
		{
			nEffectTone = TONE_CN_1;  //ǿ��Ϊ1����
		}


		nIndex = nPosType*10  + nInitialType*5 + nEffectTone;

		//����de5��zi5������������β���Ǿ�β�����򽫻�Ƶ�½���������Щ  hyl 2012-04-17
		//���磺�Ϸ�ǰ��������3679�κ���9��30�־�Ҫ����ˡ����������������Ҳ�����а������ɡ�
		if( nIndex == 1 && g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_SEN] !=0 )
		{
			fTwoDctValue[nIndex][0] += 0.07;
			fTwoDctValue[nIndex][1] += 0.034;
		}

		bModifyDct = emFalse;

		//zhe5 modified by wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM] == INITIAL_CN_zh  &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] == FINAL_CN_e     &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)
		{
			bModifyDct = emTrue;
			nIndex += 0;					// fPPHTailSpecialDctValue�����е�	 ����ֵ	nIndex = 0 + ǰ����	
		}

		//men5 modified by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM] == INITIAL_CN_m  &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] == FINAL_CN_en     &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)
		{
			bModifyDct = emTrue;
			nIndex += 0;					// fPPHTailSpecialDctValue�����е�	 ����ֵ	nIndex = 5 + ǰ����				
		}

		//ma5 modified by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM] == INITIAL_CN_m  &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] == FINAL_CN_a     &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)
		{
			bModifyDct = emTrue;
			nIndex += 5;					// fPPHTailSpecialDctValue�����е�	 ����ֵ	nIndex = 10 + ǰ����				
		}

		if( bModifyDct == emTrue )
		{
			//����������ڣ�ĳЩ������β��������ģ���滻
			if( fPPHTailSpecialDctValue[nIndex][0] != 0.0)
				BackDct( lf0Float, nSylEndT-nSylStartT+1, &fPPHTailSpecialDctValue[nIndex][0], 7 );	//��DCT		
		}
		else
		{
			//һ������ģ��
			if( fTwoDctValue[nIndex][0] != 0.0)
				BackDct( lf0Float, nSylEndT-nSylStartT+1, &fTwoDctValue[nIndex][0], 7 );			//��DCT		
		}
	}

	//��ʼ�������ڵ�f0�滻��DCTģ��
	if( lf0Float[0] != 0.0 )
	{
		for( n = nSylStartT; n < nSylEndT; n++)
			g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length] = log(lf0Float[n-nSylStartT]);
	}
	return ;
}
#endif


#if WL_REAR_DEAL_DCT_PPH_TAIL			//������������������ɶ���βDCTģ�壨������˫�̾䣬��������ĸ��
void ChangePphTailDctF0(   emInt32  nCurStartTime,			//�ֶκϳ���: ���ֿ�ʼ��֡��
						   emInt32  nSylTotalT,				//�ֶκϳ���: ���ֵ���ʱ��
						   emInt32  m1)	
{
	emInt16  n, t;
	emInt16 nSylStartT = 0, nSylEndT= 0;
	emInt16  nIndex, nUStart;
	emInt32  nToneMe,nTonePrev ,nToneNext,nEffectTone, nInitialType, nPosType ;
	float  lf0Float[150],err_f0_DCT[150],err_f0_tmp;
							



#if WL_REAR_DEAL_MA0_DCT	|| 	WL_REAR_DEAL_AI3_DCT	//���������������mao1��βDCTģ���Ƶ����			mdj modified by mdj 2012-02-20
   emInt8 maolabChange = 0;
   emInt8 ai3labChange = 0;
#endif

   LOG_StackAddr(__FUNCTION__);

		nUStart = nCurStartTime;
	for( t = nCurStartTime; t< (nCurStartTime+nSylTotalT-5); t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == UNVOICED)
		{
			nUStart = t;				//������������һ�����ڵ�һС��������Ƶ
			break;
		}
	}

		
	//�ҵ������ڵ�������Ƶ����ʼ֡���ͽ���֡��
	nSylStartT = nUStart;
	nSylEndT = (nCurStartTime+nSylTotalT -1);
	for( t = nUStart; t< nCurStartTime+nSylTotalT; t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED)
			nSylStartT = t;

		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t+1] == UNVOICED)
			nSylEndT = t;
	}	


	//����Ѱ�ҵ�DCT��Ƶģ�������
	emMemSet(lf0Float,0,sizeof(lf0Float));
	if(    g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)					//�ǵ�˫��
		//hyl 2012-04-17 �����ų���ĸ  ���磺���⡱����xie4��Aisound4��
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_SM] != INITIAL_CN_letter			//ǰ���ڲ�ΪӢ����ĸ
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_SM] != INITIAL_CN_letter)			//�����ڲ�ΪӢ����ĸ
	{

#if WL_REAR_DEAL_MA0_DCT				//���������������mao1��βDCTģ���Ƶ����			mdj
		if(g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM]==INITIAL_CN_m
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM]==FINAL_CN_ao
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE]==TONE_CN_1)					// ��ǰ������mao3
//		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE]==TONE_CN_light				// ǰ����Ϊ���� modified by mdj 2012-03-16
		{
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE]=TONE_CN_1;					// ǰ������Ϊһ��
			maolabChange = 1;
		}
#endif

#if WL_REAR_DEAL_AI3_DCT			//���������������ai3��β��Ƶģ�������		added by mdj 2012-03-19

		if((g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero ||		// ��ǰ����������ĸ
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal)		// ��ǰ�����Ǳ���ĸm��n modified by mdj 2012-02-20
		&&	g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM]==FINAL_CN_ai
		&&	g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE]==TONE_CN_3
		&&	g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE]==TONE_CN_2)
		{
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE] = TONE_CN_3;					// ǰ������Ϊ3��
			ai3labChange = 1;
		}


#endif

		//�����Ƶģ�������
		nToneMe =   g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE];
		nTonePrev = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE];
		nToneNext = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_TONE];
		if( nToneMe == TONE_CN_light )		
			nToneMe = 0;		//����ת��
		if( nTonePrev == TONE_CN_light )	
			nTonePrev = 0;		//����ת��
		if( nToneNext == TONE_CN_light )	
			nToneNext = 0;		//����ת��

		nInitialType = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM];
		if( nInitialType == 3 || nInitialType == 13 || nInitialType == 14 || nInitialType == 15 || nInitialType == 18 )
			nInitialType = INITIAL_ZERO_ZHUO;		//������ĸ�� a,o,e,w,y��ͷ��,13:l  14:m  15:n  18:r 
		else
			nInitialType = INITIAL_OTHER;		//������ĸ

		//��ֵ��λ������
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0)		//���ɶ���β
		{
			if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_SEN] == 0)
				nPosType = POS_NO_LIGHT_SEN_TAIL;		//��β							��ֻͳ��4�ֻ����ϵ����ɶ��
			else
				nPosType = POS_NO_LIGHT_PPH_TAIL;		//�����ɶ���β	����������β��  ��ֻͳ��4�ֻ����ϵ����ɶ��
		}
		else
			nPosType = POS_NO_LIGHT_OTHER;

#if WL_REAR_DEAL_OU_CHAN_YIN				//������������ı�����ĸou���� ���硰�ж�ŷ��				mdj

		if(g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero		// ��ǰ����������ĸ
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM]==FINAL_CN_ou
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE]==TONE_CN_3)					// ��ǰ����ou3
			nPosType = POS_NO_LIGHT_PPH_TAIL;

#endif


		nEffectTone = nTonePrev;

		//��Ӱ������Ϊ��sil����pau����������ĸ����
		if( nEffectTone == TONE_CN_sil_pau || nEffectTone == TONE_CN_letter)	//hyl  2012-03-29
		{
			nEffectTone = TONE_CN_1;  //ǿ��Ϊ1����
		}


		nIndex = nToneMe*20 + nPosType*10  + nInitialType*5 + nEffectTone;
		
		if( fTwoDctValuePphTail[nIndex][0] != 0.0)
			BackDct( lf0Float, nSylEndT-nSylStartT+1, (float *)(&fTwoDctValuePphTail[nIndex][0]), 7 );	//��DCT
	}


#if WL_REAR_DEAL_AI3_DCT			//���������������ai3��β��Ƶģ�������ָ�lab		added by mdj 2012-03-20
		if(ai3labChange == 1)
		{
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE] = TONE_CN_2;					// ǰ������Ϊ3��
			ai3labChange = 0;
		}

#endif
//��ʼ�������ڵ�f0�滻��DCTģ��
	emMemSet(err_f0_DCT,0,sizeof(err_f0_DCT));
	err_f0_tmp = 0;
	if( lf0Float[0] != 0.0 )
	{ 
#if WL_REAR_DEAL_DCT_PPH_TAIL_TRAIL
		for( n = nSylStartT; n < nSylEndT; n++)
		{
			lf0Float[n-nSylStartT] = 0.5*lf0Float[n-nSylStartT]+err_f0_tmp + 0.5*exp(g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length]);
			g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length] = log(lf0Float[n-nSylStartT]);
		}

#else
		for( n = nSylStartT; n < nSylEndT; n++)
			g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length] = log(lf0Float[n-nSylStartT]);
#endif
	}


	return ;
}
#endif


#if WL_REAR_DEAL_DCT_SHORT_SEN			//<����>���Ķ̾䴦��ģʽ(��ĸ�Ĳ�����)��ר�Ŵ������־䣬���־�

//Ϊ�滻��Ƶģ��
/*********************************/
/* ��DCT����DCTϵ���ָ���Ƶ��������  */
/* �����б�                    */
/*   float *x �����Ƶ������������   */
/*   emInt32  m    �����Ƶ����x�ĳ���    */
/*   float *c ����DCT����        */
/*   emInt32  n    ����DCT����c�ĳ��� */
/*********************************/




//�̾�ģʽ: �������־�����־������ͻ�Ƶ����DCTģ�壩
//���أ����ֵ�֡��
void ChangeShortSenZiGainF0(emInt32  nCurStartTime,			//�ֶκϳ���: ���ֿ�ʼ��֡��
						   emInt32  nSylTotalT,				//�ֶκϳ���: ���ֵ���ʱ��
						   emInt32  m1)						//�ֶκϳ���: ���εĵڼ�����
{
	emInt16  n, t;
	emInt16 nSylStartT = 0, nSylEndT= 0;
	emInt16  nIndex, nUStart;
	float fGain;		//������ֵ
	float fGainChange;	//�����ı�ϵ��
	emInt32  nToneMe,nTonePrev ,nToneNext,nEffectTone, nInitialType, nPosType ;
	float  lf0Float[150];



	////////////////////////  ���־䣺��ƵDCTģ��˵��  //////////////////////////////////
	//
	float fOneDctValue[5][7]={
		{1.032649,  -0.009009,  -0.000393,  -0.003199,  -0.004161,  -0.001517,  -0.002263},	//һ����dctģ��
		{0.719289,  -0.064659,  0.029642,   0.001623,   -0.003843,  0.002884,   -0.002150},	//������dctģ��
		{0.545271,  0.043375,   0.022054,   0.007160,   -0.004063,  0.002792,   -0.001307},	//������dctģ��
		{0.808059,  0.121666,   -0.018365,  -0.006492,  -0.003713,  -0.000954,  -0.003166},	//������dctģ��
		{0.561895,  0.099480,   0.026527,   0.006876,   0.001738,   0.004419,   0.000465}};	//������dctģ��	
																					



#define		TWOZI_SEN_STD_GAIN			(0.008)		//������������

	  LOG_StackAddr(__FUNCTION__);

	  nUStart = nCurStartTime;
	for( t = nCurStartTime; t< (nCurStartTime+nSylTotalT-5); t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == UNVOICED)
		{
			nUStart = t;				//������������һ�����ڵ�һС��������Ƶ
			break;
		}
	}

		
	//�ҵ������ڵ�������Ƶ����ʼ֡���ͽ���֡��
	nSylStartT = nUStart;
	nSylEndT = (nCurStartTime+nSylTotalT -1);
	for( t = nUStart; t< nCurStartTime+nSylTotalT; t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED)
			nSylStartT = t;

		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t+1] == UNVOICED)
			nSylEndT = t;
	}	



	//************************************�ݹ������ı�*******************************
	//  ������һ�Σ�����  ����������
	//    ����һ�Σ�deng3 �ᱬ��
	//*******************************************************************************
	////ͳ�Ʊ����ڵ�������ֵ
	//fGain = 0.0;
	//nLen = (nSylEndT-nSylStartT+1)/4;
	//for( t = nSylStartT+nLen; t<= nSylEndT-nLen; t++)	//ֻͳ�Ʊ����������ε��м��1/2������
	//	fGain += exp(g_pRearRTCalcFilter->m_CmpFeature[t][0]);

	//fGain /= (nSylEndT-nSylStartT-2*nLen+1);	

	////���������ı�ϵ��
	//fGainChange = pow(TWOZI_SEN_STD_GAIN / fGain, 0.4) ;

	//if ( fGainChange> 1.4 || fGainChange < 0.91 )	//ֻ�ı䷧ֵ��������ڵ�����
	//{
	//	if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light )		//����
	//	{
	//		fGainChange *= 0.90 ;
	//	}

	//	//�ı�����
	//	for (t = nUStart; t<= nSylEndT; t++)
	//		g_pRearRTCalcFilter->m_CmpFeature[t][0] += log(fGainChange);
	//}	
	//************************************�ݹ������ı�*******************************





	//����Ѱ�ҵ�DCT��Ƶģ�������
	emMemSet(lf0Float,0,sizeof(lf0Float));
	if(    g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] == 2	)				//���־�
		//hyl 2012-04-17 �����ų���ĸ  ���磺G4�����۰ĸ��١�G6-��Ӫ���١�
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_SM] != INITIAL_CN_letter			//ǰ���ڲ�ΪӢ����ĸ
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_SM] != INITIAL_CN_letter)			//�����ڲ�ΪӢ����ĸ
	{
		//�����Ƶģ�������
		nToneMe =   g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE];
		nTonePrev = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE];
		nToneNext = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_TONE];
		if( nToneMe == TONE_CN_light )		
			nToneMe = 0;		//����ת��
		if( nTonePrev == TONE_CN_light )	
			nTonePrev = 0;		//����ת��
		if( nToneNext == TONE_CN_light )	
			nToneNext = 0;		//����ת��

		nInitialType = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM];
		if( nInitialType == 3 || nInitialType == 13 || nInitialType == 14 || nInitialType == 15 || nInitialType == 18 )
			nInitialType = INITIAL_ZERO_ZHUO;		//������ĸ�� a,o,e,w,y��ͷ��,13:l  14:m  15:n  18:r 
		else
			nInitialType = INITIAL_OTHER;		//������ĸ

		//��ֵ��λ������
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_NUM_IN_C_SEN] == 2 )
		{
			if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_FWD_POS_IN_SEN] == 0 )
			{
				nPosType = POS_LEN2_PPH_HEAD;
				nEffectTone = nToneNext;
			}
			if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_SEN] == 0 )
			{
				nPosType = POS_LEN2_PPH_TAIL;
				nEffectTone = nTonePrev;
			}
		}
		else
		{
			nPosType = POS_LEN2_OTHER;
			nEffectTone = 0;
		}		

		//��Ӱ������Ϊ��sil����pau����������ĸ����
		if( nEffectTone == TONE_CN_sil_pau || nEffectTone == TONE_CN_letter)			//hyl  2012-03-29
		{
			nEffectTone = TONE_CN_1;  //ǿ��Ϊ1����
		}


		nIndex = nToneMe*20 + nPosType*10  + nInitialType*5 + nEffectTone;
		
		if( fTwoDctValueShortSen[nIndex][0] != 0.0)
			BackDct( lf0Float, nSylEndT-nSylStartT+1,(float *)( &fTwoDctValueShortSen[nIndex][0]), 7 );	//��DCT
	}
	else															//���־�
	{
		if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] == TONE_CN_light)		//��1����������
			nIndex = 4;							
		else
			nIndex = g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] - 1 ;


		if( fOneDctValue[nIndex][0] != 0.0)
			BackDct( lf0Float, nSylEndT-nSylStartT+1, &fOneDctValue[nIndex][0], 7 );	//��DCT
	}

	//��ʼ�������ڵ�f0�滻��DCTģ��
	if( lf0Float[0] != 0.0 )
	{
		for( n = nSylStartT; n < (nSylEndT+1); n++)
			g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length] = log(lf0Float[n-nSylStartT]);
	}


	return ;

}
#endif

// ********************  ����ʱ��ģ�� start , changed by naxy19 **********************************
#if SYL_DURATION


// ��������ʱ��Լ�������ٵ���: ���Խ�������׾�β������ʱ����ӳ������ʵ�ʱ������
//����̫�죺���ɷ�ָ�����ݵĶ��ŵ�10086����μӻ��
float ModifySylSpeed(emInt32 Offset,emInt16 pOffsetValue[10], float sydur[2], float remain)
{  
	float mean[10], vari[10], temp1, temp2;
	float rho, w;
	emInt32  lOffset;
	emInt8 i;

	LOG_StackAddr(__FUNCTION__);

	lOffset = Offset;
	fRearSeek(g_hTTS->fResCurRearMain, lOffset ,0);


	if( g_hTTS->pRearG->nModelFloatLen == 2 )			
	{
		fread_float2(mean,g_hTTS->pRearG->nModelFloatLen,g_hTTS->pInterParam->nState,g_hTTS->fResCurRearMain);
		fread_float2(vari,g_hTTS->pRearG->nModelFloatLen,g_hTTS->pInterParam->nState,g_hTTS->fResCurRearMain);
	}
	else
	{
		fRearRead(mean,sizeof(float),g_hTTS->pInterParam->nState,g_hTTS->fResCurRearMain);
		fRearRead(vari,sizeof(float),g_hTTS->pInterParam->nState,g_hTTS->fResCurRearMain);
	}
	
	temp1 = 0.0;
	temp2 = 0.0;

	//w����ʾȨ�أ�Ŀǰ��5���ʡ� //
	//�ο�����ѯ���ߡ��еġ��ɡ���
	//w=1:rho=0.0029��w=3:rho=0.0068��w=5:rho=0.0094��w=30:rho=0.0174��w=100:rho=0.0198��w=1000:rho=0.0200��
	//w��ֵԽ�پ�����ʱ��Խ�ӽ�ԭ���ģ�ֵԽ�󣬾�����ʱ��Խ�̡�
	//���磺ԭ������ʱ����15.15�� w=0.1: 15.12 ��w=1 : 14.88��w=5 : 14.43��w=100 : 13.92
	w = 5;		
	for ( i=0; i<g_hTTS->pInterParam->nState; i++ )
	{
		mean[i] /= g_hTTS->pRearG->Speech_speed;
		temp1 += mean[i];
		temp2 += vari[i];
	}
	
	rho = w * (sydur[0] - temp1) / (sydur[1] + w*temp2);
	for ( i=0; i<g_hTTS->pInterParam->nState; i++ )
	{
		temp1 = mean[i] + rho * vari[i] + remain;

		temp1 /= 0.945;	//Ϊ���þ�����ʱ����ԭ���ı���һ��  ����w=5����

		pOffsetValue[i] =(emInt16) (temp1 + 0.5); //ȡ����
		if (pOffsetValue[i]<1)
			pOffsetValue[i] = 1;
		remain = temp1 - pOffsetValue[i];
	}
	return remain;
}

#endif

// ********************  ����ʱ��ģ�� end **********************************




//ģ�;������ľ���--����������
emInt32 RearDecision( emInt8 i)
{
	emByte  ch[Table_NODE_LEN_INF];
	emInt16	nNode;
	emInt16	outInt,nTempValue,nCompareX;

	LOG_StackAddr(__FUNCTION__);

	nNode = 0;

	while(TRUE)
	{
		emMemCpy(ch, &g_tree[nNode*Table_NODE_LEN_INF], Table_NODE_LEN_INF);
		//ch[0]�ĵ�4��bit��������lab�Ƚϵ�lab�е�������,��8��bit��������lab�ж�Ӧ�бȽϵķ�ʽ
		//ch[1]��������lab�Ƚϵ�lab�е���ʼ����
		outInt=ch[2] | ch[3] << 8;//����lab�ж�Ӧ�бȽϵ�����ֵ

		if ( (ch[0] & (emInt8)0x10) == (emInt8)0x10 ) //����lab�Ƚϵ�lab�е�������Ϊ2
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1] << 8 | g_pLabRTGlobal->m_LabOneSeg[i][ch[1]];
		else //����lab�Ƚϵ�lab�е�������Ϊ1
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1];

		if ( (((ch[0] &1)==0) && (nCompareX==outInt)) //ǡ�õ����Ҿ���ֵΪ��  
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (ch[1] < (emInt8)42)) //С�ڵ����Ҿ���ֵΪ��
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (nCompareX>0) && (ch[1] > (emInt8)41)) )
		{
			nTempValue = ch[6] | ch[7] << 8;	// �ش�Ϊ���ǡ�
		} else
			nTempValue = ch[4] | ch[5] << 8;	// �ش�Ϊ����

		if ( (nTempValue & 0x8000) == 0x8000 )
		{
			nTempValue &= 0x7FFF;//Ӧת��ı���������pdf�ж�Ӧ�ĵڼ���Ҷ�ڵ�			
			break;
		}
		else // ���λΪ0��Ӧת��ı���������������
			nNode = nTempValue;
	}
	return nTempValue;
}



//ģ�;������ľ���--��������Ҫ��
emInt32 RearDecision_Line( emInt8 i, emInt32  nTreeOffset)
{
	emByte  ch[Table_NODE_LEN_INF];
	emInt16	nNode;
	emInt16	outInt,nTempValue,nCompareX;

	LOG_StackAddr(__FUNCTION__);

	nNode = 0;

	while(TRUE)
	{
		fRearSeek(g_hTTS->fResCurRearMain, nTreeOffset+nNode*Table_NODE_LEN_INF ,0);	
		fRearRead(ch,Table_NODE_LEN_INF,1,g_hTTS->fResCurRearMain);

		//ch[0]�ĵ�4��bit��������lab�Ƚϵ�lab�е�������,��8��bit��������lab�ж�Ӧ�бȽϵķ�ʽ
		//ch[1]��������lab�Ƚϵ�lab�е���ʼ����
		outInt=ch[2] | ch[3] << 8;//����lab�ж�Ӧ�бȽϵ�����ֵ

		if ( (ch[0] & (emInt8)0x10) == (emInt8)0x10 ) //����lab�Ƚϵ�lab�е�������Ϊ2
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1] << 8 | g_pLabRTGlobal->m_LabOneSeg[i][ch[1]];
		else //����lab�Ƚϵ�lab�е�������Ϊ1
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1];		//nCompareXΪǰ��lab�з�������ֵ  sqb  2016.11.2

	if(g_bRearSynChn)			//����--20131225-���ɴ˶�������
	{
		if ( (((ch[0] &1)==0) && (nCompareX==outInt)) //ǡ�õ����Ҿ���ֵΪ��  
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (ch[1] < (emInt8)42)) //С�ڵ����Ҿ���ֵΪ��
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (nCompareX>0) && (ch[1] > (emInt8)41)) )
		{
			nTempValue = ch[6] | ch[7] << 8;	// �ش�Ϊ���ǡ�
		} else
			nTempValue = ch[4] | ch[5] << 8;	// �ش�Ϊ����
	}
	else						//Ӣ��
	{
		if (   (  ( (ch[0] &1)==0 ) && nCompareX==outInt )		//ǡ�õ���,�Ҿ���ֵΪ��  
		     ||(  ( (ch[0] &1)==1 ) && nCompareX<=outInt ) )	//С�ڵ���,�Ҿ���ֵΪ��
		{
			nTempValue = ch[6] | ch[7] << 8;	// �ش�Ϊ���ǡ�
		} else
			nTempValue = ch[4] | ch[5] << 8;	// �ش�Ϊ����
	}

		if ( (nTempValue & 0x8000) == 0x8000 )
		{
			nTempValue &= 0x7FFF;//Ӧת��ı���������pdf�ж�Ӧ�ĵڼ���Ҷ�ڵ�			
			break;
		}
		else // ���λΪ0��Ӧת��ı���������������
			nNode = nTempValue;
	}
	return nTempValue;
}




// ********************  ����ȫ���ǣ���˾��߽���Ĺ������� ��������Ƕ��㣩 *************************************


#if	WL_REAR_DEAL_R_INITIAL

//����ri,re��������������ri,re������ص�ǰ��������ƽ��  naxy 1205
void ModifyRGain(emInt16 nStartLineOfPau, emInt16 nLineCountOfPau )
{
	emInt16 n, s, t;
	emByte nTemp1,nTemp2 ;
	emInt16 dur;
	float step;
	emBool bIsSylRiReRuNeed = emFalse;
	emBool bIsNextSylRiReRuNeed = emFalse;

	#define	R_START_GAIN	(3.5)

	LOG_StackAddr(__FUNCTION__);

	dur = 0;
	
	//���ִ�����β�����pau����
	for (n = 0; n < (nLineCountOfPau-1); n++) 
	{
		bIsSylRiReRuNeed = emFalse;
		bIsNextSylRiReRuNeed = emFalse;

		//������Ϊri,re�����ô�����
		nTemp1 = g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM];		
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r 
			&& (nTemp1==FINAL_CN_iii || nTemp1==FINAL_CN_e  ) )			//������Ϊri,re
		{
			bIsSylRiReRuNeed = emTrue;
		}

		//�����ڲ���sil��pau����һ����Ϊri,re�����ô�����
		nTemp1 = g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM];		
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil)					//�����ڲ���sil��pau
			if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_SM] == INITIAL_CN_r 
				&& (nTemp1==FINAL_CN_iii || nTemp1==FINAL_CN_e  ))						//��һ����Ϊri,re
			{
				bIsNextSylRiReRuNeed = emTrue;
			}

		//��״̬ѭ������
		for(s = 0; s < g_hTTS->pInterParam->nState; s++)
		{
			//������Ϊri,re�Ĵ���
			if( bIsSylRiReRuNeed == emTrue )
			{
				//r��ĸ��������������
				if(g_pRearRTCalc->m_MSDflag[dur] == UNVOICED)
				{
					for(t=0; t<g_pRearRTGlobal->m_PhDuration[n][s]; t++)
						g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (R_START_GAIN-1.5);	//r��ĸ��������������
				}
				//r��ĸ��������1��״̬����ƽ��
				else
				{
					step = (R_START_GAIN-s+1.5)/(g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1]-1);
					for(t=0; t<g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1]; t++)		//r��ĸ��������1��״̬����ƽ��
						g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (R_START_GAIN-s+1.5-t*step);	
					break;
				}
			}
			
			//�����ڲ���sil��pau����һ����Ϊri,re�Ĵ���
			if( bIsNextSylRiReRuNeed == emTrue )
			{
				//�������2��״̬������ƽ��
				if( s == (g_hTTS->pInterParam->nState-2) )				
				{
					if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil)
					{
						nTemp2 = g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1];
						step = (R_START_GAIN-1)/nTemp2;

						//r��ĸ��ǰ1���ڵ��������2��״̬����ƽ��
						for(t=0; t<nTemp2; t++)	
							g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (t+1)*step;
					}
				}
			}

			dur += g_pRearRTGlobal->m_PhDuration[n][s];
		}	

		// ���break���ˣ�2012-02-03	//��������2��r��ĸ��  ���ȵġ����ȡ����ա�
		for(; s < g_hTTS->pInterParam->nState; s++)
		{
			// ��������r��ĸ�����
			if( bIsNextSylRiReRuNeed == emTrue )
			{
				//�������2��״̬������ƽ��
				if( s == (g_hTTS->pInterParam->nState-2) )	
				{
					if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil)
					{
						nTemp2 = g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1];
						step = (R_START_GAIN-1)/nTemp2;

						for(t=0; t<nTemp2; t++)	
							g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (t+1)*step;
					}
				}
			}
			// ʱ�������ۼ�
			dur += g_pRearRTGlobal->m_PhDuration[n][s];
		}
	}
}

#endif



#if WL_REAR_DEAL_VOL_TOO_LARGE   //�Գ����������е���   2012-04-18

void ChangeTooLargeVol(emInt16 nLineCountOfPau)
{
	emInt16 i,j;

  //#define GAIN_FROM_100_TO_95			(-0.051293295)				//=log(0.95);
	#define GAIN_FROM_100_TO_90			(-0.10536052)				//=log(0.90);
  //#define GAIN_FROM_100_TO_85			(-0.16251893)				//=log(0.85);
	#define GAIN_FROM_100_TO_80			(-0.22314355)				//=log(0.80);
	#define GAIN_FROM_100_TO_75			(-0.28768209)				//=log(0.75);	
  //#define GAIN_FROM_100_TO_70			(-0.35667494)				//=log(0.70);

	LOG_StackAddr(__FUNCTION__);

	for( i =0;i<nLineCountOfPau;i++ )
	{		
		if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0				//���׻���β
		    || g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] == 0 )
		{
			if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_a 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ua 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ao 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ang 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_eng 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ang 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uai
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uan
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uang
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ueng 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ong
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_o
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_y
				)
			{
				//���׻���β��fou , hou
				if(    (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_f && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou )
					|| (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_h && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou ))
				{
					//���磺[v10]ҥ�Եı���һ���й�ʽ���ʺ�[d]
					//���磺[v10]�񶨡�����Ͳ����������Ƿ�[d]
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_75;	
				}
				//��β��song2,song3,sou2��sou3��zou2��zou3
				else if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0
					     && (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_2 ||  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3)
						 &&	(   (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_s && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ong )
						     || (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_s && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou  )
						     || (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou  )))
				{
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_75;	

				}
				//��β
				else if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0)				
				{
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_80;	
				}
				//ʣ��
				else																				
				{
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_90;	
				}
			}					
		}
		else			//�����׷���β
		{
			//fou��wang, zao3
			if(    (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_f && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou )
				|| (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uang)
				|| (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ao && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3))
			{
				//���磺[v10]��ʵ�ֶԸ��־���ʵʩ�����ź������ڱ���[d]
				//���磺[v10]�ٶȲ��ۿ�������Դ��ڰ�ȫ���֮�ڡ����Ƿ����������[d]
				for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_75;	
			}
			else if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_a 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ao 
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_o
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_y
				)
			{
				for(j=0;j<g_hTTS->pInterParam->nState;j++ )
					g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_90;	
			}				
		}		
	}
}

#endif

/******************** ���ھ�����ص��Ӻ����� ************************/
/*                                                                  */
/* ���������������ڻ�ȡʱ�����ס���Ƶ��Ҷ�ڵ�                       */
/* ʱ�������߽���������ٵ�����ȡ������ֱ�Ӵ���g_pRearRTGlobal->m_PhDuration         */
/* �׺ͻ�Ƶ�ľ��߽������g_pRearRTGlobal->m_MgcLeafNode��g_pRearRTGlobal->m_Lf0LeafNode         */
/* ���أ�ʱ����������ֵ  ************************************************************************/
float GetLeafNode(emInt16 nStartLineOfPau , emInt16 nLineCountOfPau, float remain)
{
	// main decision function
	emInt32 nLeafNo;
	emInt32 loffsetOfRes;
	emInt32 Next_TREE_OFFSET;
	float ratio;

	emBool  bTemp;
	emInt16 nCurSylTotalFrame, n1, n2;

	emInt16 length;
	emInt16 curNodeSum;
	emInt8  j,i,k;
	emInt8	lightToneChange[MAX_SYNTH_LAB_COUNT+2], zelabChange, wolabChange, enlabChange, zhuolabChange, foulabChange;   //added by wangcm 2012-03-08
	emInt8	durChange,delabchange;					//added by mdj 2012-03-26 used for de5 chanyin and flag for light duration justified
	emInt8  ce5labChange, holdValueFour;       //---wangcm-2012-03-16
	emInt8	senlabChange, laolabChange;		//added by mdj 2012-03-17
	emInt8  mnlabchange;                       //---wangcm-2012-04-01
	emInt8  holdValueOne, holdValueTwo, holdValueThree;
	emInt32 nAllocSize;
	emInt16 m1;			//����ͳ����β������������ʱ��	

	// added by wangcm 2012-03-22
	emByte	g_LabZhe5[LAB_CN_LINE_LEN]={0x17, 0x09, 0x05, 0x01, 0x01, 0x06, 0x01, 0x01, 0x06, 0x01, 
		0x06, 0x02, 0x03, 0x07, 0x05, 0x01, 0x02, 0x07, 0x05, 0x01, 
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00};
	emByte	g_LabZhe5bak[LAB_CN_LINE_LEN];

	//����ʱ�����߽��
#if SYL_DURATION //changed by naxy19
	float syl_dur[MAX_SYNTH_LAB_COUNT+2][2];
#endif
#if WL_REAR_DEAL_DAO_LF0 || WL_REAR_DEAL_RU3_LF0
	emInt8	daolabChange = 0;
	emInt8	rulabChange = 0;
	emInt16 sylPospph = 0;
#endif

#if WL_REAR_DEAL_OU_CHAN_YIN || WL_REAR_DEAL_AI_CHAN_YIN 
	emInt8 oulabChange = 0;
	emInt8 ailabChange = 0;
	emInt8 aiplaceChange = 0;
	emInt8 aimethodChange = 0;

#endif

#if WL_REAR_DEAL_MA0_DCT				//���������������mao1��βDCTģ���Ƶ����			mdj
   emInt8 maolabChange = 0;
   emInt8 maoRtone = 0;					//added by mdj 2012-03-21
   emInt8 yanlabChange = 0;
#endif

#if WL_REAR_DEAL_IANG_LF0							//���������������iang4��Ƶ�յ�����			added by mdj 2012-03-15
   emInt8 ianglabChange = 0;
#endif

   LOG_StackAddr(__FUNCTION__);

	if(g_bRearSynChn)
	{
#if SYL_DURATION   /* ����ʱ��ģ�Ϳ�ʼ, changed by naxy19 */
	Next_TREE_OFFSET = g_Res.offset_RearVoiceLib_Cur + g_hTTS->pRearG->offset.sydMod;
	fRearSeek(g_hTTS->fResCurRearMain, Next_TREE_OFFSET ,0);
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);		// ����ʱ������Ҷ�ڵ����
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

#if EM_DECISION_READ_ALL	

#if DEBUG_LOG_SWITCH_HEAP
	g_tree = (emCharA *)emHeap_AllocZero(nAllocSize , "����ʱ��ģ����Ҷ�ڵ㣺�����ģ��  ��");
#else
	g_tree = (emCharA *)emHeap_AllocZero(nAllocSize );
#endif

	fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load syllable duration Tree
#endif

	for( i = 1;i<nLineCountOfPau-1;i++ ) 
	{
#if EM_DECISION_READ_ALL
		nLeafNo = RearDecision(i);
#else
		nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
		loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				PDF_NODE_FLOAT_COUNT_SYD * g_hTTS->pRearG->nModelFloatLen * (nLeafNo-1); //���ݱ���е�ƫ����
		fRearSeek(g_hTTS->fResCurRearMain, loffsetOfRes, 0);
		if( g_hTTS->pRearG->nModelFloatLen == 2 )
		{
			fread_float2(&syl_dur[i][0], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);			
			fread_float2(&syl_dur[i][1], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);
		}
		else
		{
			fRearRead(&syl_dur[i][0], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);			
			fRearRead(&syl_dur[i][1], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);
		}
	}

#if EM_DECISION_READ_ALL	
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_tree , nAllocSize , "����ʱ��ģ����Ҷ�ڵ㣺�����ģ��  ��");
#else
	emHeap_Free(g_tree , nAllocSize );
#endif
#endif



#endif            /* ����ʱ��ģ�ͽ��� */


	/*************************************************************************************************/
	/******************************* ������ת����ģ�Ϳ�ʼ ********************************************/
	/*************************************************************************************************/

	/*
	Next_TREE_OFFSET = g_pRearOffset->uvpMod;
	fRearSeek(g_hTTS->fResCurRearMain,Next_TREE_OFFSET ,0);	
	// ��ȡ������ת�����������Ҷ�ڵ����
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

	if( g_bIsUserAddHeap == 0 )
	{	
#if EM_DECISION_READ_ALL	
			// �����ڴ����ʱ���������� 
	
#if DEBUG_LOG_SWITCH_HEAP
			g_tree = (emByte *)emHeap_AllocZero(nAllocSize , "����ת����ģ�͵�Ҷ�ڵ㣺�����ģ��  ��");
#else
			g_tree = (emByte *)emHeap_AllocZero(nAllocSize );
#endif
			// ��ȡ������ת���������
			fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load syllable uvp Tree
#endif  
	}
	else
	{
		g_tree = g_AllTree + g_AllTreeOffset[0];

	}

	


	// ����ÿ���ֵ�ת����
	for( i = 1;i<nLineCountOfPau-1;i++ ) 
	{
#if EM_DECISION_READ_ALL
		nLeafNo = RearDecision(i);
#else
		nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
		loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				PDF_NODE_FLOAT_COUNT_UVP * g_hTTS->pRearG->nModelFloatLen * (nLeafNo-1); //���ݱ���е�ƫ����
		fRearSeek(g_hTTS->fResCurRearMain, loffsetOfRes, 0);
		if( g_hTTS->pRearG->nModelFloatLen == 2 )		
			fread_float2(&syl_uvp[i], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);	
		else
			fRearRead(&syl_uvp[i], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);
	}

	if( g_bIsUserAddHeap == 0 )
	{	

#if EM_DECISION_READ_ALL	
		// �ͷ���ʱ������������ڴ� 
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_tree , nAllocSize , "����ת����ģ�͵�Ҷ�ڵ㣺�����ģ��  ��");
#else
		emHeap_Free(g_tree , nAllocSize );
#endif
#endif  
	}

	*/								//sqb    2016.12.2
	/*************************************************************************************************/
	/******************************* ������ת����ģ�ͽ��� ********************************************/
	/*************************************************************************************************/

	}

	if(g_bRearSynChn)	{
#if WL_REAR_DEAL_LIGHT_MGC				//������������ı䲿��������Ƶ�ף�����Ч����

	for( i =0;i<nLineCountOfPau;i++ )
	{
		//�����������������ĳ�4������Ƶ�׾�������ٸĻ���
		lightToneChange[i] = 0;
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light	)	// ��ǰ����������
			//g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] ==0 &&				// ��ǰ��������β
		{
			if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_o	//��ĸ��o			����
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ie	//��ĸ��ie			лл
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ve	//��ĸ��ve			[i1]xue2xue5
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ua	//��ĸ��ua			[i1]gua4gua5 [i1]kua4kua5
				)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_4;					//�����ĳ�4��
				lightToneChange[i] = 1;
			}
		}
	}

#endif
	}	
	

	// ��ȡ״̬ʱ����������Ҷ�ڵ����
	fRearSeek(g_hTTS->fResCurRearMain,g_pRearOffset->durMod,0); //changed by naxy17
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain); // ʱ������Ҷ�ڵ����


	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

	if( g_bIsUserAddHeap == 0 )
	{
#if EM_DECISION_READ_ALL
		/* �����ڴ����ʱ���������� */
#if DEBUG_LOG_SWITCH_HEAP
		g_tree = (emByte *)emHeap_AllocZero(nAllocSize , "ʱ����1����Ҷ�ڵ㣺�����ģ��  ��");
#else
		g_tree = (emByte *)emHeap_AllocZero(nAllocSize );
#endif	
		// ��ȡ״̬ʱ��������
		fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load Duration Tree
#endif
	}
	else
	{
		g_tree = g_AllTree + g_AllTreeOffset[1];
	}

	Next_TREE_OFFSET = g_pRearOffset->durMod; //changed by naxy17

	
	//ÿ�ηֶκϳɣ�ǰ��ӵ�pau��ÿ��״̬��֡��
	//ÿ��״̬Ϊ1֡��2֡��wav��һ���ģ���ÿ��״̬Ϊ4֡��wav�Ͳ�һ���ˣ�
	for(j=0;j<g_hTTS->pInterParam->nState;j++ )
	{

#if SEG_ADD_PAU_TEN_STAGE_TEN_FRAME
		g_pRearRTGlobal->m_PhDuration[0][j] = SEG_TEMP_PAU_STATE_FRAME;
		g_pRearRTGlobal->m_PhDuration[nLineCountOfPau-1][j] = SEG_TEMP_PAU_STATE_FRAME;
#else
		if( j < (10-SEG_TEMP_PREV_PAU_TOTAL_FRAME) )
			g_pRearRTGlobal->m_PhDuration[0][j] = 0;
		else
			g_pRearRTGlobal->m_PhDuration[0][j] = 1;

		if( j < (10-SEG_TEMP_NEXT_PAU_TOTAL_FRAME))
			g_pRearRTGlobal->m_PhDuration[nLineCountOfPau-1][j] = 0;
		else
			g_pRearRTGlobal->m_PhDuration[nLineCountOfPau-1][j] = 1;
#endif
	}
	length = 0;
	durChange = 0;						//added by mdj 2012-03-26 used for duration change flag
		
	for( i = 1;i<nLineCountOfPau-1;i++ )
	{

		if(g_bRearSynChn)	{
#if WL_REAR_DEAL_RU3_LF0			//�����������ru3�Ļ�Ƶ��� modified by mdj

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_r 					// ��ǰ��������ĸ��r
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_u					// ��ǰ��������ĸ��u
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3					// ��ǰ������ru3
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW] != 0				// ��ǰ����PPHβ
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] != 0)			// ��ǰ����PPHͷ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW] = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PW] = 1;
			}
#endif
		}	// end of g_bRearSynChn

		//ʱ�����ߣ�
#if EM_DECISION_READ_ALL
		nLeafNo = RearDecision(i);
#else
		nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
		loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) 
			+ 2 * g_hTTS->pInterParam->nState * g_hTTS->pRearG->nModelFloatLen *(nLeafNo-1); //���ݱ���е�ƫ����
		


		if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DCT_SHORT_SEN					//<����>���Ķ̾䴦��ģʽ(��ĸ�Ĳ�����)��ר�Ŵ������־䣬���־�


		//���־��ʱ���ݲ�����--��ɾ
		//if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//ԭʼ�����ǡ����֡���
		//{
		//	//��Ӣ����ĸ�Ĳ�����
		//	if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] <= TONE_CN_sil_pau && g_pLabRTGlobal->m_LabRam[2][RLAB_C_TONE] <= TONE_CN_sil_pau)	//��1�ֺ͵�2�ֶ�������ĸ	
		//	{

		//		//���־�ģʽ: �������־�ʱ��
		//		if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN]==2  && nStartLineOfPau >= 2)		//Ϊ���־䣬�Ҵ����˵�2���� ��ע�⣺���־�ĵ�һ���ֲ�������
		//		{
		//			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light )	//����Ϊ����
		//				g_hTTS->pRearG->Speech_speed *= 1.2;
		//		}

		//	}
		//}

#endif

#if WL_REAR_DEAL_LIGHT_TONE			//�Ƕ̾�����������ģʽ��
		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//ԭʼ�����ǡ����֡���
		{
			//�Ƕ̾���: ��������ģʽ��  �䳤>2
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2 && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light)	
				g_hTTS->pRearG->Speech_speed *= 1.1;
		}
#endif
		}	// end of g_bRearSynChn

		//״̬ʱ��ȡ������
#if SYL_DURATION
		if(g_bRearSynChn)
		{
		//��Ϊsil��pau���ڣ��򲻸�������ʱ��ģ�͵���
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_sil 
			||  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_pau)	
		{
			remain = ModifySpeed(loffsetOfRes,g_pRearRTGlobal->m_PhDuration[i],remain);
		}
		else
			remain = ModifySylSpeed(loffsetOfRes,g_pRearRTGlobal->m_PhDuration[i],syl_dur[i], remain);
		//remain = 0;
		}
#else
		remain = ModifySpeed(loffsetOfRes,g_pRearRTGlobal->m_PhDuration[i],remain);
#endif


		if(g_bRearSynChn)	{
#if HT_REAR_DEAL_DUR
			for(j=0;j<g_hTTS->pInterParam->nState;j++ )
			{
				g_pRearRTGlobal->m_PhDuration[i][j] = g_pRearRTGlobal->m_PhDuration[i][j]*7/5;  //����1.4��
			}
#endif

#if WL_REAR_DEAL_LETTER	

		//�ı�ʱ��		
		if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+i][RLAB_C_YM]  == FINAL_CN_letter_c 		//��1��ĸ��C�Ļ�
			&&(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_v
			|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_u
			|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_j
			|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_letter_z) )	
		{
			g_pRearRTGlobal->m_PhDuration[i][8] /= 3;
			g_pRearRTGlobal->m_PhDuration[i][9] /= 3;
		}

#endif

#if WL_REAR_DEAL_LETTER			

		//��ȫ����ĸ��ʱ������һЩ			hyl  2012-04-19  ����
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_letter )	 //������ĸ
		{
			for(j=0;j<g_hTTS->pInterParam->nState;j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] > 12 )
					g_pRearRTGlobal->m_PhDuration[i][j] = g_pRearRTGlobal->m_PhDuration[i][j]*11/12;  //����1/12				
			}
		}

#endif

#if WL_REAR_DEAL_EN_ENG_QST
		//���en��eng�����⴦��(����weng����en2��eng2���3����������ĸ)  + ����iang
		//���ӣ����ͣ����������͡����̣�������̺�ϲ�����̡���
		//���ӣ����������������������ܰ�������[=hen2]�á�����[=hen2]û��
		//���ӣ�[i1]sen2mei3��[i1]wen2mei3��[i1]deng2mei3��[i1]feng2mei3
		//���ӣ�����Ϊ�����Ϲ���ɻ�Ʊ��õ��ϴ���Żݣ�

		bTemp = emFalse;
		if(   g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero && 
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ueng     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0) //�����ڣ�weng�޵�����
		{
			bTemp = emTrue;
		}
		if(   g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l && 
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_iang     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0) //�����ڣ�weng�޵�����
		{
			bTemp = emTrue;
		}

		//�����㣬�򽫵�5��6״̬��ʱ����Ϊ1֡�����ٵ�֡Ų����4��7״̬
		if( bTemp == emTrue )	
		{
			g_pRearRTGlobal->m_PhDuration[i][4] += g_pRearRTGlobal->m_PhDuration[i][5] - 1;
			g_pRearRTGlobal->m_PhDuration[i][5]  =                                       1;
			g_pRearRTGlobal->m_PhDuration[i][7] += g_pRearRTGlobal->m_PhDuration[i][6] - 1;
			g_pRearRTGlobal->m_PhDuration[i][6]  =                                       1;
		}

		//bTemp = emFalse;
 	//	if(    (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_en)								//�����ڣ�en
		//	&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_2 							//�����ڣ�����
		//	&& (   g_pLabRTGlobal->m_LabRam[i+nStartLineOfPau][RLAB_C_SM] == INITIAL_CN_m) 			//�����ڣ�m��ĸ
		//	&& g_pLabRTGlobal->m_LabRam[i+nStartLineOfPau][RLAB_C_TONE] == TONE_CN_3 )				//�����ڣ�����
		//{
		//	bTemp = emTrue;
		//}

		////�����㣬�ı�֡��
		//if( bTemp == emTrue )	
		//{
		//	g_pRearRTGlobal->m_PhDuration[i][7]  =  1;
		//	g_pRearRTGlobal->m_PhDuration[i][8]  =  1;
		//	g_pRearRTGlobal->m_PhDuration[i][9]  =  3;
		//}

#endif


#if WL_REAR_DEAL_DUR_XIEXIE
		// naxy, 2012-01-13, ie,ve
		if( (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ie ||		// ��ĸ��ie
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ve)&&		// ��ĸ��ve
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light)		// ����
		{
			g_pRearRTGlobal->m_PhDuration[i][7] = 1;
		}
#endif

#if	WL_REAR_DEAL_DUR_ZENME
		// naxy, 2012-01-18, zen me
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z &&		// ��ǰ������ĸ��z
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en &&			// ��ǰ������ĸ��en
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE] == TONE_CN_light)		// ��һ��������
		{
			g_pRearRTGlobal->m_PhDuration[i][6] = 1;
			g_pRearRTGlobal->m_PhDuration[i][7] = 1;
			g_pRearRTGlobal->m_PhDuration[i][8] = 1;
			g_pRearRTGlobal->m_PhDuration[i][9] = 1;
		}
#endif

#if WL_REAR_DEAL_R_INITIAL
		if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_r &&		// ��ĸ��r
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_u)			// ��ĸ��u
		{
			g_pRearRTGlobal->m_PhDuration[i][0] += 2;
			g_pRearRTGlobal->m_PhDuration[i][1] = 1;
			g_pRearRTGlobal->m_PhDuration[i][2] = 1;
			g_pRearRTGlobal->m_PhDuration[i][3] += 2;
		}

		if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_r &&		// ��ĸ��r
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_iii )		// ��ĸ��i
		{
			g_pRearRTGlobal->m_PhDuration[i][0] = 2;
			g_pRearRTGlobal->m_PhDuration[i][1] = 2;
			g_pRearRTGlobal->m_PhDuration[i][2] = 2;
			g_pRearRTGlobal->m_PhDuration[i][3] += 2;
		}

#endif

#if WL_REAR_DEAL_ZHUO_SM
		if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l )
		{
			g_pRearRTGlobal->m_PhDuration[i][0] = (emInt16)(4/g_hTTS->pRearG->Speech_speed); // modified, naxy120317
			g_pRearRTGlobal->m_PhDuration[i][1] = (emInt16)(5/g_hTTS->pRearG->Speech_speed);
		}
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_nasal &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_light)
		{
			g_pRearRTGlobal->m_PhDuration[i][1] = (emInt16)(3/g_hTTS->pRearG->Speech_speed);
		}
#endif
		}	//end of g_bRearSynChn

		nCurSylTotalFrame = 0;
		for(j=0;j<g_hTTS->pInterParam->nState;j++ )
		{
			nCurSylTotalFrame += g_pRearRTGlobal->m_PhDuration[i][j];
		}


		if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DCT_SEN_TAIL_LEN					//�ı��β������������ʱ��

		#define  SYL_MIN_FRAME_FOR_TAIL_WL			50	//����������������֡��С�ڴ���������ı䣨��g_hTTS->pRearG->Speech_speed=0.9��
		#define  SYL_CHANGE_MIN_FRAME_FOR_SEN_WL	52	//���������¸ı���SEN������С֡������g_hTTS->pRearG->Speech_speed=0.9��
		#define  SYL_CHANGE_MIN_FRAME_FOR_PPH_WL	50	//���������¸ı���PPH������С֡������g_hTTS->pRearG->Speech_speed=0.9��

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//ԭʼ�����ǡ����֡���
			&& g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD )		//���ǣ�һ��һ��Ч��
		{
			//�������ɶ���βDCTģ��			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)			//�䳤>2
			{
				if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0	&&		//��β��SEN��syl�ķ���λ��=0
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_letter  &&		//��ΪӢ����ĸ
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_light)			//��Ϊ����
				{	
					if(nCurSylTotalFrame < (SYL_MIN_FRAME_FOR_TAIL_WL/g_hTTS->pRearG->Speech_speed))
					{

						n1 = (SYL_CHANGE_MIN_FRAME_FOR_SEN_WL/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;

						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
//						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i)
						{
							n1=0;
						}
						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d		//������di4��ʱ�� modified by mdj 2012-03-17
						&& (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i || 
							g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_u	)	//������du4��ʱ�� modified by mdj 2012-03-26
						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_4)
							n1=0;

						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero		//������ai4��ʱ�� modified by mdj 2012-03-22
						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ai
						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_4)
							n1=0;
							
						if(n1 > 15)     //modified back by mdj 2012-03-19
							n1 = 15;
						if( n1 <= 5)
						{
							for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-n1;j--)
								g_pRearRTGlobal->m_PhDuration[i][j]++;
						}
						else 
						{
							if(n1 <= 10)
							{
								for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-5;j--)
								{
									g_pRearRTGlobal->m_PhDuration[i][j]++;
								}
								for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-(n1-5);j--)
								{
									g_pRearRTGlobal->m_PhDuration[i][j]++;
								}

							}
							else
							{
								for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-5;j--)
								{
									g_pRearRTGlobal->m_PhDuration[i][j]+=2;
								}
								for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-(n1-10);j--)
								{
									g_pRearRTGlobal->m_PhDuration[i][j]++;
								}
							}
						}
					}
				}
				else if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0	&&	//���ɶ���β��PPH��syl�ķ���λ��=0
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_letter  &&		//��ΪӢ����ĸ
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_light)			//��Ϊ����
				{	
					if(nCurSylTotalFrame < (SYL_MIN_FRAME_FOR_TAIL_WL/g_hTTS->pRearG->Speech_speed) )
					{
						n1 = (SYL_CHANGE_MIN_FRAME_FOR_PPH_WL/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;
						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
//						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i)
						{
							n1=0;
						}
						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d		//������di4��ʱ�� modified by mdj 2012-03-17
						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i
						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_4)
							n1=0;
						if(n1 > 10)				//modified back by mdj 2012-03-19
							n1 = 10;
						if( n1 <= 5)
						{
							for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-n1;j--)
								g_pRearRTGlobal->m_PhDuration[i][j]++;
						}
						else 
						{
							for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-5;j--)
							{
								g_pRearRTGlobal->m_PhDuration[i][j]++;
							}
							for(j=g_hTTS->pInterParam->nState-1;j>=g_hTTS->pInterParam->nState-(n1-5);j--)
							{
								g_pRearRTGlobal->m_PhDuration[i][j]++;
							}

						}
					}
				}
			}
		}

#endif

#if WL_REAR_DEAL_SYL_TOO_FAST				//���֣��ı�������ڹ�������  

		//���磺�й��Ʋø��Ե�Ī�������������ض��з��֡��ܲ��쵼�ɹ��ö�
		//���������ٹ�������
		#define  SYL_MIN_FRAME_FOR_WL			30	//����������������֡��С�ڴ���������ı䣨��g_hTTS->pRearG->Speech_speed=0.9��
		#define  SYL_CHANGE_MIN_FRAME_FOR_WL	33	//���������¸ı���������С֡������g_hTTS->pRearG->Speech_speed=0.9��
		#define  SYL_LIGHT_NEED_CUT_FOR_WL		5	//������������������ٵķ�ֵ	

		//����1��4��
		if( nCurSylTotalFrame<=(SYL_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed)  
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]<=TONE_CN_4)  
		{
			n1 = (SYL_CHANGE_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;
			if( n1>g_hTTS->pInterParam->nState)
			{
				n1 = g_hTTS->pInterParam->nState;			//�������10֡				
			}
			nCurSylTotalFrame += n1;
			n1 = g_hTTS->pInterParam->nState - n1;			//�ӵڼ���״̬��ʼÿ��״̬����1֡
			for(j=n1 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j]++;			
		}

		//��������
		if( nCurSylTotalFrame<=((SYL_MIN_FRAME_FOR_WL-SYL_LIGHT_NEED_CUT_FOR_WL)/g_hTTS->pRearG->Speech_speed)  
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light)  
		{
			n1 = ((SYL_CHANGE_MIN_FRAME_FOR_WL-5)/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;
			if( n1>g_hTTS->pInterParam->nState)
			{
				n1 = g_hTTS->pInterParam->nState;			//�������10֡				
			}
			nCurSylTotalFrame += n1;
			n1 = g_hTTS->pInterParam->nState - n1;			//�ӵڼ���״̬��ʼÿ��״̬����1֡
			for(j=n1 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j]++;			
		}

#endif

#if WL_REAR_DEAL_DCT_SHORT_SEN
		// hyl  2012-04-17   �������bu�������õ�����
		if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_b		//��ĸ��b
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_u		//��ĸ��u
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_NUM_IN_C_SEN] == 1 )	//���־�
		{
			
			g_pRearRTGlobal->m_PhDuration[i][0] = 2;
			g_pRearRTGlobal->m_PhDuration[i][1] = 3;
		}
#endif


#if WL_REAR_DEAL_SYL_TOO_FAST_NEW
		//for de2����β�����  wangcm 2012-03-17
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_2 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			/*for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;*/
			//mei3de2
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_3  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_ei &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_m)
			{
				for(j=8 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i-1][j] += (emInt16)(3/g_hTTS->pRearG->Speech_speed);
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}
		//de3 modified end wangcm 2012-03-17

		//mei4 wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_4  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ei &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE] != TONE_CN_light)	//�Һ��������������� hyl 2012-04-23 ���磺����ε���������
		{
			for(j=0 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;

			//deleted by hyl 2012-04-23
			/*if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] != 1)
			{
				for(j=1 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;

			}*/
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//mei4 end wangcm 2012-03-17

		//for mei3wei4����β�����  wangcm 2012-03-17
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ei &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m )
		{
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE]==TONE_CN_4 &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 1 &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM] == FINAL_CN_uei    &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero)
			{
				for(j=8 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i][j] += (emInt16)(6/g_hTTS->pRearG->Speech_speed);
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}

		//mei3wei4 modified end wangcm 2012-03-17

		//for gui3zi5 ����β�����

		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3  &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uei &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_g)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE]==TONE_CN_light &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 1 &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM] == FINAL_CN_ii      &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_z)
			{
				for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i][j] += 2;
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}

		//for zi5 ����β�����
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ii      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//zi5 end

		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_b     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3        &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM]==INITIAL_CN_g       &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM] == FINAL_CN_uei)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_4  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_iii &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_r)
			{
				//�ӵ�1��״̬��ʼÿ��״̬����1֡
				for(j=5 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i][j] += 1;
				for(j=6 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i-1][j] += 1;
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}

		//gui3zi5 modified end

		//for wo3guo2 
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_2       &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uo    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_g)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_3  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_uo &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_zero)
			{
				//�ӵ�1��״̬��ʼÿ��״̬����1֡
				for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i][j] += 1;
				for(j=3 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i-1][j] += 1;
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}
		//wo3guo5 modified end

		//for wo3gmen2 
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uo &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE]==TONE_CN_light    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] != 1 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM] == FINAL_CN_en     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_m)
			{
				//�ӵ�1��״̬��ʼÿ��״̬����1֡
				for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i][j] += 1;
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}
		//men5 modified wangcm 2012-03-16
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m)
		{
			for(j=1 ; j<g_hTTS->pInterParam->nState ; j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2 )
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//wo3men5 modified end  wangcm 2012-03-16

		//for me5 ��β��ʱ��  wangcm 2012-03-16
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//end me5 wangcm 2012-03-16
		//added by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ia      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(j = 2; j < g_hTTS->pInterParam->nState; j++)
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;	//added by wangcm 2012-03-01 
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			
		}
		//end  wangcm 2012-03-20


		//for lai2le5 and le5 modified by wangcm 2012-03-16
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
		{
			//�ӵ�1��״̬��ʼÿ��״̬����1֡
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_2  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_ai &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_l)
			{
				//�ӵ�1��״̬��ʼÿ��״̬����1֡
				for(j=2 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i-1][j] += 1;
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}
		//modified by wangcm 2012-03-16
		//la5 �ײ������ѵ�4��5��״̬��1  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_a    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
		{
			for(j=4 ; j<6; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] = 1;
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//la5  wangcm 2012-03-17
		//di4di5 ���������di5ʱ����
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d)
		{
			for(j=4 ; j<g_hTTS->pInterParam->nState; j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//di4di5 ���������di5ʱ����
		//wa2wa5  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ua    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero)
		{
			for(j=3 ; j<g_hTTS->pInterParam->nState; j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//wa2wa5  wangcm 2012-03-17
		//zhe5   wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light   &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zh)
		{
			for(j=6 ; j<g_hTTS->pInterParam->nState; j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
			}
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]==TONE_CN_4)
			{
				for(j=3 ; j<6; j++ )
				{
					if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
						g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
				}
			}
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]==TONE_CN_2)
			{
				for(j=1 ; j<3; j++ )
				{
					if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
						g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
				}
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag

		}
		//zhe5  wangcm 2012-03-20
		//shen2me5 shen2�� me5�ײ������ѵ�4��5��״̬��1  wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m)
		{
			for(j=2 ; j<5; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] = 1;
			for(j=5 ; j<g_hTTS->pInterParam->nState; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;
			for(j=0 ; j<3; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;
			if( g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_2  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_en    &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_sh)
			{
				for(j=7 ; j<g_hTTS->pInterParam->nState; j++ )
				g_pRearRTGlobal->m_PhDuration[i-1][j] += 1;
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//shen2me5  wangcm 2012-03-17

		//nong2+l/m/n  wangcm 2012-03-21
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_2  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ong    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_n   &&
		   (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_m  ||
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_n  ||
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_l  ))
		{
				for(j=6 ; j<g_hTTS->pInterParam->nState; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] += 1;

				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//end of nong2+l/m/n  wangcm 2012-03-21

#endif

#if WL_REAR_DEAL_LIGHT_M_N
		//m/n ��������ĸ�������磺��������wangcm 2012-04-01
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&

		   (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m  ||
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_n  ))
		{
				
				g_pRearRTGlobal->m_PhDuration[i][0] = 4;
				g_pRearRTGlobal->m_PhDuration[i][1] = 5;

				durChange = 1;				
		}
//		   wangcm 2012-04-01
#endif
		}	// end of g_bRearSynChn


		 // ��ֹ����״̬ʱ�����ڵ����naxy120319
		for(j=0 ; j<g_hTTS->pInterParam->nState ; j++ )
			if(g_pRearRTGlobal->m_PhDuration[i][j] < 1)
				g_pRearRTGlobal->m_PhDuration[i][j] = 1;
	}


		
	/*
	//���������ʱ������������С
	if(length>MAX_FRAME_COUNT_OF_SEG) 
	{
		ratio = (float)MAX_FRAME_COUNT_OF_SEG;
		ratio /= length;
		for( i = 1;i<nLineCountOfPau-1;i++ )
			for(j=0;j<g_hTTS->pInterParam->nState;j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] = (emInt16)(g_pRearRTGlobal->m_PhDuration[i][j]*ratio);
	}*/

	if( g_bIsUserAddHeap == 0 )
	{
#if EM_DECISION_READ_ALL
		/* �ͷ���ʱ������������ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_tree , nAllocSize , "ʱ����1����Ҷ�ڵ㣺�����ģ��  ��");
#else
		emHeap_Free(g_tree , nAllocSize );
#endif
#endif
	}

	//ָ����Դ�е�Ƶ��ģ��
	Next_TREE_OFFSET = g_pRearOffset->mgcMod;

	zelabChange = 0;
	enlabChange = 0;
	zhuolabChange = 0;   // added by wangcm 2012-03-08
	wolabChange = 0;
	ce5labChange = 0;     //---wangcm-2012-03-16
	foulabChange = 0;
	holdValueOne = 0;
	holdValueTwo = 0;
	holdValueThree = 0;
	holdValueFour  = 0;
	senlabChange = 0;		//added by mdj 2012-03-17
	laolabChange = 0;		//added by mdj 2012-03-17
	delabchange = 0;    //added by mdj 2012-03-27
	mnlabchange = 0;      //---wangcm-2012-04-01
	
	//Ϊ�ӿ�����ٶȣ���״̬ѭ������
	for(j=0;j<g_hTTS->pInterParam->nState;j++ )
	{
		//��ȡƵ�׵�j������Ҷ�ڵ�����������ڴ�
		fRearSeek(g_hTTS->fResCurRearMain,Next_TREE_OFFSET ,0); //changed by naxy17
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain); // Ƶ�׵�j������Ҷ�ڵ����

		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

		if( g_bIsUserAddHeap == 0 )
		{

#if EM_DECISION_READ_ALL	
#if DEBUG_LOG_SWITCH_HEAP
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize  , "Ƶ�׵�n����Ҷ�ڵ㣺�����ģ��  ��");
#else
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize );
#endif		
			//��ȡƵ�׵�j����
			fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load Spectrum Tree
#endif
		}
		else
		{
			g_tree = g_AllTree + g_AllTreeOffset[2+j];
		}

		for( i =0;i<nLineCountOfPau;i++ )
		{

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_CHAN_YIN		//������������ı䲿�ֲ���  ze4����ĸen eng an ang ong�������ĸ

			//����ze4�ϳɲ�������   //�ơ�ƽƽ���ơ�ƽ�ơ����ơ�
			//if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z &&	// ��ĸz
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// ��ĸe
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_4  &&	// ����
			//	j>4)
			//{
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_asprt_affricate;     // �ı���ĸ������ʽ��־λ
			//	zelabChange = 1;
			//}


			//������ĸen eng an ang ong�������ĸ���õ�����
			//���ӣ� ����������������80��Ľ������Ե���һЩ80����������ˡ�
			//���ӣ� ������������[=eng1]����������[=an1]����������[=ang1]����������[=kong1]��������
			if(	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM_REAR] == YM_CN_REAR_nasal &&		// ��β������ʽ��
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] != FINAL_CN_ian &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] != FINAL_CN_an &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] != FINAL_CN_uan &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero		)		// ������������ĸ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_s;     // �ı��һ������ĸΪ������ĸ
				enlabChange = 1;
			}
			if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM_REAR] == YM_CN_REAR_nasal   &&		// ��β������ʽ��
				(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_m      ||
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_n      ||
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_l      ||
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_r))				    // ������������ĸ
			{
				holdValueOne = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM];
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_t;                       // �ı��һ������ĸΪ������ĸ

				holdValueTwo = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD];
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD] = SM_CN_METHOD_asprt_stop;     // �ı��һ������ĸΪ������ĸ

				holdValueThree = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE];
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] = SM_CN_PLACE_dentalveolar;

				zhuolabChange = 1;
			}
			//���uo3����������ĸ�����---wangcm-2012-03-17
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uo  &&		// ��ĸuo
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3 &&		// ����
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero)	//����ĸ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_s;     // �ı���ĸ������ʽ��־λ
				wolabChange = 1;
			}
			//end  ---wangcm-2012-03-17
			
			//fou3��β��ʱ��
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_f &&	// ��ĸz
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou  &&		// ��ĸe
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3  &&
				j>2)	//����ĸ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] = INITIAL_CN_s;     // �ı���ĸ������ʽ��־λ
				foulabChange = 1;
			}


			//ze5/zhe5/ce5/che5/se5/she5/he5 �״�������  ---wangcm-2012-03-16
			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_fricative       ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_affricate       ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_asprt_affricate)&&   // 
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// ��ĸe
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0)	
			{

				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// ��ĸe			hyl  2012-04-07
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zh)		// ��ĸzh
				{
					emMemCpy(g_LabZhe5bak, g_pLabRTGlobal->m_LabOneSeg[i], LAB_CN_LINE_LEN);
					emMemCpy(g_pLabRTGlobal->m_LabOneSeg[i], g_LabZhe5, LAB_CN_LINE_LEN);	
				}
				else
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_4;     // �ı�����				

				ce5labChange = 1;
			}
			//end----ze5/zhe5/ce5/che5/se5/she5/he5 �״�������  ---wangcm-2012-03-16


			//sen1,cen1����β��ʱ��						added by mdj  2012-03-17
			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_s ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_c)				// ��ĸs��z
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en  
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_1				// ��ĸen
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] == SM_CN_PLACE_labial	// ��������ĸΪb��p
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] != INITIAL_CN_m)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] = SM_CN_PLACE_labiodental;
				senlabChange = 1;
			}

			//l/n/m/rao3���������ĸ��ʱ��				added by mdj	2012-03-17
			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_lateral ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_nasal	 ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_r)				// ��ĸm,n,l,r
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ao  
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3				// ��ĸao3
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD] == SM_CN_METHOD_zero)	// ������Ϊ����ĸ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_g;
				laolabChange = 1;
			}
			
			//de5���������ĸ��ʱ��						added by mdj	2012-03-26
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  		// 
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light 		// de5
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM_FRONT] == YM_CN_FRONT_open)	//������Ϊ������ĸ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_t;
				delabchange = 1;
			}


#endif


#if WL_REAR_DEAL_LIGHT_M_N    //-wangcm-20120401

			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m
			||	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_n) 
			&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] == 0
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light)	//������Ϊ������ĸ
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_4;
				mnlabchange = 1;
			} 


#endif						//-wangcm-20120401

#if WL_REAR_DEAL_LIANG_LF0

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_l					// ��ǰ�����Ǳ�����ĸ��l
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_iang					// ��ǰ��������ĸ��iang
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3					// ��ǰ������liang3
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE] != TONE_CN_4							// ǰһ���ڲ�������
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] != 0)			// ��ǰ����PPHͷ
			{
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE] = TONE_CN_4;
			}
#endif

#if WL_REAR_DEAL_OU_CHAN_YIN				//������������ı�����ĸou���� ���硰�ж�ŷ��				mdj
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0				// ��ǰ��������β
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] != 0
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero		// ��ǰ����������ĸ
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_ou
			&& j > 1)
			{
//				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] = 1;
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_1)
				{
					oulabChange = 1;
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_stop;
					if(j == 5)
					{
						g_pRearRTGlobal->m_PhDuration[i][j-1] += (g_pRearRTGlobal->m_PhDuration[i][j]-2);
						g_pRearRTGlobal->m_PhDuration[i][j] = 1;
						g_pRearRTGlobal->m_PhDuration[i][j+1] += 1;
					}
				}
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3)
				{
					oulabChange = 2;
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]=TONE_CN_1;
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_stop;
					if(j == 5)
					{
						g_pRearRTGlobal->m_PhDuration[i][j-1] += (g_pRearRTGlobal->m_PhDuration[i][j]-2);
						g_pRearRTGlobal->m_PhDuration[i][j] = 1;
						g_pRearRTGlobal->m_PhDuration[i][j+1] += 1;
					}
				}
			}
#endif

#if WL_REAR_DEAL_AI_CHAN_YIN				//������������ı�����ĸai4���� ���硰�ϰ���

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0			// ��ǰ��������β
			&& (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero ||		// ��ǰ����������ĸ
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal)		// ��ǰ������m��n modified by mdj 2012-03-19
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_ai)
			{
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero
				&& j > 3)
				{
	//				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] = INITIAL_CN_d;
					aiplaceChange = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE];
					aimethodChange = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD];
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_stop;
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = SM_CN_PLACE_dentalveolar;
					ailabChange = 1;
				}
				else if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal			// ��ǰ��ĸ��m��n modified by mdj 2012-03-20
					&& j > 4)
				{
					aiplaceChange = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE];
					aimethodChange = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD];
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_stop;
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = SM_CN_PLACE_dentalveolar;
					ailabChange = 1;
				}
			}
#endif

#if WL_REAR_DEAL_LI_SPEC			//���������������li��β��ʱ�����׵�����
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0)	// ��ǰ��������β
			{
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3)
					if((g_pRearRTGlobal->m_PhDuration[i][j]>12) && (j < 7) && (j > 3))		// modifed by mdj 2012-03-19
					{
						g_pRearRTGlobal->m_PhDuration[i][j-3] += 2;
						g_pRearRTGlobal->m_PhDuration[i][j-2] += 2;
						g_pRearRTGlobal->m_PhDuration[i][j-1] += 2;
						g_pRearRTGlobal->m_PhDuration[i][j+1] += 2;
						g_pRearRTGlobal->m_PhDuration[i][j+2] += 2;
						g_pRearRTGlobal->m_PhDuration[i][j+3] += 2;
						g_pRearRTGlobal->m_PhDuration[i][j] = g_pRearRTGlobal->m_PhDuration[i][j] - 12;
					}

				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_4)
					if(j==8)
					{
						g_pRearRTGlobal->m_PhDuration[i][j-1] += 1;
						g_pRearRTGlobal->m_PhDuration[i][j+1] += (g_pRearRTGlobal->m_PhDuration[i][j]-2);
						g_pRearRTGlobal->m_PhDuration[i][j] = 1;
					}
			}
//				continue;
#endif
			}	// end of g_bRearSynChn
//			else if (!g_bRearSynChn)  //-Ӣ��ǿ�Ƹ���
//			{
//#if HTRS_REAR_DEAL_M_N
//				if (  
//					 g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C02_L_pp] != EngPP_silh && 
//					 g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_silt  )					
//				   g_pLabRTGlobal->m_LabOneSeg[i][ELAB_W06_C_pp] = 0;
//#endif
//
//			}
			     
			

			if(  (i!=0 && i!=nLineCountOfPau-1) || (g_hTTS->m_bIsSynFirstAddPau == emFalse && i == 0 )   )				
			{
				//���ߵ�i���ֵ�Ƶ��Ҷ�ڵ�
#if EM_DECISION_READ_ALL
				nLeafNo = RearDecision(i);
#else
				nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
				if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //Ƶ������ά0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) +							//����Ҷ�ڵ����ƫ�� 
						(g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen+2) * (nLeafNo-1);		
				else
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) +							//����Ҷ�ڵ����ƫ�� 
						g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen * (nLeafNo-1);	

				fRearSeek(g_hTTS->fResCurRearMain,loffsetOfRes ,0);
	
				//��ȡ���ߵ�Ҷ�ڵ�
				if( g_hTTS->pRearG->nModelFloatLen == 2 )			
				{
					//Ƶ������ά0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
					if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 )  
					{
						fRearRead(g_pRearRTGlobal->m_MgcLeafNode[i][j], sizeof(float),1, g_hTTS->fResCurRearMain); 

						fread_float2(g_pRearRTGlobal->m_MgcLeafNode[i][j]+1, g_hTTS->pRearG->nModelFloatLen,		
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width - 1, g_hTTS->fResCurRearMain);
					}
					else
					{
						fread_float2(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_hTTS->pRearG->nModelFloatLen,		
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width, g_hTTS->fResCurRearMain); //emInt16 g_pRearRTGlobal->m_MgcLeafNode[12][10] �洢ÿ�����߽����ֵ���뱾����
					}					
				}
				else
				{
					fRearRead(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_hTTS->pRearG->nModelFloatLen,		
						g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width, g_hTTS->fResCurRearMain); //emInt16 g_pRearRTGlobal->m_MgcLeafNode[12][10] �洢ÿ�����߽����ֵ���뱾����
				}

				if(g_bRearSynChn)
				{
				////le5&me5&ne5 men5�Ĳ�������  naxingyu 2012-03-19 
				//if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_n  || 
				//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m  ||
				//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l )&&
				//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e    &&
				//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light &&
				//	j==6)
				//{
				//	emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-2], g_pRearRTGlobal->m_MgcLeafNode[i][j], 
				//		g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
				//	emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-3], g_pRearRTGlobal->m_MgcLeafNode[i][j-4], 
				//		g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
				//}
				//if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m  &&
				//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en    &&
				//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light &&
				//	j==6)
				//{
				//	emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-2], g_pRearRTGlobal->m_MgcLeafNode[i][j], 
				//		g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
				//	emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-3], g_pRearRTGlobal->m_MgcLeafNode[i][j-4], 
				//		g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
				//}
				//// naxingyu 2012-03-19
				}//
				else    //��Ƶ��
				{
#if HTRS_REAR_DEAL_HH     //hh��ͷ�����⣬hearth��here��hello��hold,with hundred 
					if ( g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_hh && j==3 
						 && GetPhoneType(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C02_L_pp]) != 1)
					{   						
						
						 if (g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_y ||
							 g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_iy|| 
							 g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_ih )						
						 {					
							emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-2], g_pRearRTGlobal->m_MgcLeafNode[i][j], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
							emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-1], g_pRearRTGlobal->m_MgcLeafNode[i][j], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));	
						 }						 
						 else 
						 {							
							 emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-2], g_pRearRTGlobal->m_MgcLeafNode[i][j-1], 
								 g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));	
							 emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_pRearRTGlobal->m_MgcLeafNode[i][j-1], 
								 g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));	
						 }
					}					
#endif
#if HTRS_REAR_DEAL_G 
					if ( g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_g &&
						(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_silt||
						GetPhoneType(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp]) ==1) &&
						GetPhoneType(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C02_L_pp]) ==1  
						&&  j==4)
					{

						emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-1], g_pRearRTGlobal->m_MgcLeafNode[i][j-4], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
					}
					if ( g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_g  &&
						(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_hh ) &&						
						j==4)
					{						
						emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_pRearRTGlobal->m_MgcLeafNode[i][j-2], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
						emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-1], g_pRearRTGlobal->m_MgcLeafNode[i][j-2], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
					}

#endif
#if HTRS_REAR_DEAL_IY_R
					if ( g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_iy &&
						g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp] == EngPP_r && 
						g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C05_RR_pp] != EngPP_silt &&j==4)
					{
						emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_pRearRTGlobal->m_MgcLeafNode[i][j-1], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
					}

#endif
#if HTRS_REAR_DEAL_S
					if ( g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C01_C_pp] == EngPP_s && 
						 GetPhoneType(g_pLabRTGlobal->m_LabOneSeg[i][ELAB_C03_R_pp]) == 1  && j == 4)
					{
						emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_pRearRTGlobal->m_MgcLeafNode[i][j-2], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
						emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j-1], g_pRearRTGlobal->m_MgcLeafNode[i][j-2], 
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
					}
					
#endif
				}

		}
			else			//ÿ���ֶ�ǰ��ӵ�pau��������ߣ�������ÿ���ĵ�1�����ֵ�pau��֮��������滻����ʱ����Ƶ�ף���Ƶ����һ���ģ� 2012-09-19
			{
				//���뿽��������ֱ�ӽ���ָ�븳ֵ�����磺g_pRearRTGlobal->m_MgcLeafNode[i][j] = g_pRearRTGlobal->m_MgcLeafNode[0][j];��
				emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_pRearRTGlobal->m_MgcLeafNode[0][j], 
						g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
			}

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_CHAN_YIN		//������������ı䲿�ֲ���  ze4����ĸen eng an ang�������ĸ
			// ze
			//if(zelabChange == 1)
			//{
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_affricate;
			//	zelabChange = 0;
			//}
			// en
			if(enlabChange == 1)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_zero;
				enlabChange = 0;
			}	
			if(zhuolabChange == 1)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = holdValueOne;            // �ı��һ������ĸΪ������ĸ
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD] = holdValueTwo;     // �ı��һ������ĸΪ������ĸ
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] = holdValueThree;

				zhuolabChange = 0;
			}
			if(wolabChange == 1)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_zero;
				wolabChange = 0;
			}
			if(foulabChange == 1)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] = INITIAL_CN_f;
				foulabChange = 0;
			}
			//---wangcm-2012-03-16
			if(ce5labChange == 1)
			{
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// ��ĸe			hyl 2012-04-07
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zh)		// ��ĸzh
				{
					emMemCpy(g_pLabRTGlobal->m_LabOneSeg[i], g_LabZhe5bak, LAB_CN_LINE_LEN);
				}
				else
				{
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_light;     // �ı�����
				}
				ce5labChange = 0;
			}
			if(senlabChange == 1)			//added by mdj 2012-03-17
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] == SM_CN_PLACE_labial;
				senlabChange = 0;
			}
			if(laolabChange == 1)			//added by mdj 2012-03-17
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_zero;
				laolabChange = 0;
			}
			if(delabchange == 1)			//added by mdj 2012-03-26
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_zero;
				delabchange = 0;
			}					
			
			if(mnlabchange == 1)			//---wangcm-2012-04-01
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_light;
				mnlabchange = 0;
			}
			
#endif

			
#if WL_REAR_DEAL_OU_CHAN_YIN				//������������ı�����ĸai4���� ���硰����������

			if(oulabChange == 2)					// ��ǰ������ou3
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = SM_CN_PLACE_zero;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_zero;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_3;
				oulabChange = 0;
			}
			if(oulabChange == 1)					// ��ǰ������ou1
			{
				oulabChange = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = SM_CN_PLACE_zero;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_zero;

			}
#endif

#if WL_REAR_DEAL_AI_CHAN_YIN				//������������ı�����ĸai4���� ���硰����������

			if(ailabChange == 1)					// ��ǰ������ai��modified by mdj 2012-03-19
			{
				ailabChange = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = aimethodChange;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = aiplaceChange;
//				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_zero;
			}
#endif
			}	// end of g_bRearSynChn

		}

		if( g_bIsUserAddHeap == 0 )
		{
#if EM_DECISION_READ_ALL	
			/* �ͷ���ʱ�������ڴ� */
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_tree , nAllocSize , "Ƶ�׵�n����Ҷ�ڵ㣺�����ģ��  ��");
#else
			emHeap_Free(g_tree , nAllocSize );
#endif
#endif
		}

		//ָ����һ�þ�����
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //Ƶ������ά0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				(g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen + 2) * curNodeSum;		
		else
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen * curNodeSum;		
	}

	if(g_bRearSynChn)	{
#if WL_REAR_DEAL_LIGHT_MGC					//������������ı䲿��������Ƶ�ף�����Ч����
	for( i =0;i<nLineCountOfPau;i++ )
	{
		//Ƶ���Ѿ����꣬������������������4���Ļ���
		if( lightToneChange[i] == 1)								// �����Ĺ�
		{
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_light;			// �Ļ�����			
		}
#if WL_REAR_DEAL_DCT_SHORT_SEN_LEN					//<����>���Ķ̾䴦��ʱ��(��ĸ�Ĳ�����)��ר�Ŵ������־�  mdj 2012-03-23

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//ԭʼ�����ǡ����֡��� 
			&& 	durChange == 0)	//�ǡ����֡�
		{
				//���־�ģʽ: �������־�ʱ��
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_NUM_IN_C_SEN] == 2  
			&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_PW_NUM_IN_C_SEN] == 1 
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0		//Ϊ���־䣬�Ҵ����˵�2���� ��ע�⣺���־�ĵ�һ���ֲ�������
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light			//����Ϊ����
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE] < TONE_CN_sil_pau)			//��1��Ҳ������ĸ
			{
				//��Ӣ����ĸ�Ĳ�����
				for(j=0;j<5;j++)
				{
					if( g_pRearRTGlobal->m_PhDuration[i][j]>=2)			//hyl  2012-04-07  ����ϳɡ���ʰ���Ȼ��
						g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
				}
				for(j=5;j<10;j++)
				{
					if( g_pRearRTGlobal->m_PhDuration[i][j]>=4)			//hyl  2012-04-07
						g_pRearRTGlobal->m_PhDuration[i][j] -= 2;
				}

			}
		}
#endif
	}

#endif


#if WL_REAR_DEAL_VOL_TOO_LARGE   //Ƶ���Ѿ����꣬�Գ����������е���   2012-04-18

	ChangeTooLargeVol(nLineCountOfPau);

#endif
	}	// end of g_bRearSynChn


	//Ϊ�ӿ�����ٶȣ���״̬ѭ�����߻�Ƶ
	for(j=0;j<g_hTTS->pInterParam->nState;j++ )
	{
		//��ȡ��Ƶ��j������Ҷ�ڵ�����������ڴ�
		fRearSeek(g_hTTS->fResCurRearMain,Next_TREE_OFFSET ,0);
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain); // ��Ƶ��j������Ҷ�ڵ����

		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

		if( g_bIsUserAddHeap == 0 )
		{
#if EM_DECISION_READ_ALL	
#if DEBUG_LOG_SWITCH_HEAP
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize  , "��Ƶ��n����Ҷ�ڵ㣺�����ģ��  ��");
#else
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize );
#endif	
			//��ȡ��Ƶ��j����
			fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load LogF0 Tree
#endif
		}
		else
		{
			g_tree = g_AllTree + g_AllTreeOffset[12+j];
		}

		for( i =0;i<nLineCountOfPau;i++ )
		{
			//���ߵ�i���ֵĻ�ƵҶ�ڵ�
			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DAO_LF0					//�����������zhao3dao4�Ļ�Ƶ��� ���磺��û��һ������������˳����ͬʱ�ҵ����Ҹ�����

		//�������ҵ���zhao3dao4����dao4���ֻ�Ƶ��������� modified by mdj 2012-2-28
			if( g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD					//���ǣ�һ��һ��Ч��
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_d 				// ��ǰ��������ĸ��d
			    &&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_ao                  // ��ǰ��������ĸ��ao
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_4					// ��ǰ������dao4
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM]==INITIAL_CN_zh 				// ��ǰ��������ĸ��zh
			    &&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_YM]==FINAL_CN_ao                  // ��ǰ��������ĸ��ao
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]==TONE_CN_3					// ǰһ������zhao3
			    &&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW]==0 )				// ��ǰ��������β
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM] = INITIAL_CN_ch;					
				daolabChange = 1;
			}

#endif

#if WL_REAR_DEAL_MA0_DCT				//���������������mao1��βDCTģ���Ƶ����			mdj modifed on 2012-03-16

				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW]==0				// ��ǰ��������β
				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_m
				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_ao
				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_1)					// ��ǰ������mao1
//				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]==TONE_CN_light)				// ǰ����Ϊ����
				{
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]=TONE_CN_1;					// ǰ������Ϊһ��
//					maolabChange = 1;
				}
#endif

#if WL_REAR_DEAL_IANG_LF0							//���������������iang4��Ƶ�յ�����			added by mdj 2012-03-15

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_iang
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_4					// ��ǰ��ĸ��iang4
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM]==INITIAL_CN_zh					// ǰ����Ϊzhao
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_YM]==FINAL_CN_ao					// ǰ����Ϊzhao
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW]!=0 )				// ��ǰ��������β
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM]=INITIAL_CN_h;
				ianglabChange = 1;
			}

#endif
			}	// end of g_bRearSynChn

			if(  (i!=0 && i!=nLineCountOfPau-1) || (g_hTTS->m_bIsSynFirstAddPau == emFalse && i == 0 )   )
			{
#if EM_DECISION_READ_ALL
				nLeafNo = RearDecision(i);
#else
				nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif

				if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //��Ƶ0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1)		
								+ (PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen+2)*(nLeafNo-1); //���ݱ���е�ƫ����
				else
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1)		
								+ PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen*(nLeafNo-1); //���ݱ���е�ƫ����	

				g_pRearRTGlobal->m_Lf0LeafNode[i][j] = loffsetOfRes;	 //emInt32 g_pRearRTGlobal->m_Lf0LeafNode[12][10] �洢ÿ�����߽����resource�е�ƫ��
			}
			else			//ÿ���ֶ�ǰ��ӵ�pau��������ߣ�������ÿ���ĵ�1�����ֵ�pau��֮��������滻����ʱ����Ƶ�ף���Ƶ����һ���ģ� 2012-09-19
			{
					g_pRearRTGlobal->m_Lf0LeafNode[i][j] = g_pRearRTGlobal->m_Lf0LeafNode[0][j];
			}

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DAO_LF0					//�����������dao4�Ļ�Ƶ���

			if(daolabChange == 1) //��Ƶ������ɣ�������zhao3dao4������ǰһ������ĸ�Ļ���
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM] = INITIAL_CN_zh;
				daolabChange = 0;
			}
#endif

#if WL_REAR_DEAL_MA0_DCT				//���������������mao1��βDCTģ���Ƶ����			mdj
			if(maolabChange == 1)				// ǰ����Ϊ����
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]=TONE_CN_light;					// ǰ������Ϊ����
				maolabChange = 0;
			}
#endif

#if WL_REAR_DEAL_IANG_LF0							//���������������iang4��Ƶ�յ�����			added by mdj 2010-03-15
			if(ianglabChange == 1)				// ��ǰ��������β
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM]=INITIAL_CN_zh;
				ianglabChange = 0;
			}

#endif
			}	// end of g_bRearSynChn
		}

		if( g_bIsUserAddHeap == 0 )
		{
#if EM_DECISION_READ_ALL
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_tree , nAllocSize , "��Ƶ��n����Ҷ�ڵ㣺�����ģ��  ��");
#else
			emHeap_Free(g_tree , nAllocSize );
#endif
#endif
		}

		//ָ����һ�þ�����
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //��Ƶ0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) 
				+ (PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen+2) *curNodeSum;	
		else
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) 
				+ PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen *curNodeSum;

	}


	/******************************����bap����*********************/
	//sqb 2017-4-21    ע��bap��ʱûдһ�ζ�������������ڿ��޸�����
			
	//Ϊ�ӿ�����ٶȣ���״̬ѭ�����߻�Ƶ
	for (j = 0; j < g_hTTS->pInterParam->nState; j++)
	{
		//��ȡbap��j������Ҷ�ڵ�����������ڴ�
		fRearSeek(g_hTTS->fResCurRearMain, Next_TREE_OFFSET, 0);
		fRearRead(&curNodeSum, 2, 1, g_hTTS->fResCurRearMain); // bap��j������Ҷ�ڵ����

		nAllocSize = Table_NODE_LEN_INF*(curNodeSum - 1);
		g_tree = g_AllTree + g_AllTreeOffset[12 + j];
		
		for (i = 0; i < nLineCountOfPau; i++)
		{
			//���ߵ�i���ֵ�bapҶ�ڵ�
			if ((i != 0 && i != nLineCountOfPau - 1) || (g_hTTS->m_bIsSynFirstAddPau == emFalse && i == 0))
			{
				nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET + 2);


				if (g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum - 1)
					+ (PDF_NODE_FLOAT_COUNT_BAP * g_hTTS->pRearG->nModelFloatLen + 2)*(nLeafNo - 1); //���ݱ���е�ƫ����
				else
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum - 1)
					+ PDF_NODE_FLOAT_COUNT_BAP * g_hTTS->pRearG->nModelFloatLen*(nLeafNo - 1); //���ݱ���е�ƫ����	

				g_pRearRTGlobal->m_BapLeafNode[i][j] = loffsetOfRes;	 //emInt32 g_pRearRTGlobal->m_Lf0LeafNode[12][10] �洢ÿ�����߽����resource�е�ƫ��
			}
			else			//ÿ���ֶ�ǰ��ӵ�pau��������ߣ�������ÿ���ĵ�1�����ֵ�pau��֮��������滻����ʱ����Ƶ�ף���Ƶ����һ���ģ� 2012-09-19
			{
				g_pRearRTGlobal->m_BapLeafNode[i][j] = g_pRearRTGlobal->m_BapLeafNode[0][j];
			}
		}

		//ָ����һ�þ�����
		if (g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //��Ƶ0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum - 1)
			+ (PDF_NODE_FLOAT_COUNT_BAP * g_hTTS->pRearG->nModelFloatLen + 2) *curNodeSum;
		else
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum - 1)
			+ PDF_NODE_FLOAT_COUNT_BAP * g_hTTS->pRearG->nModelFloatLen *curNodeSum;

	}

	/****************************************************/


	g_hTTS->m_bIsSynFirstAddPau = emTrue;
	return remain;
}



/******************** ���ھ�����ص��Ӻ���������Ӻ��� ************************/
/*�Ӻ������ܣ� ��ʱ���ľ��߽���������ٵ�����ȡ������ֱ�Ӵ���pOffsetValue[10] */
/*�����������ָ�����ļ���FILE *infile ,emInt32 Offset��ȡ���ݵ�����pOffsetValue[10]*/
/*          �����ٵ���ֵemInt8 Speech_speed������һ���ڲ���ֵfloat remain*/
/*����ֵ�����ڲ���ֵfloat remain                                         */
float ModifySpeed(emInt32 Offset,emInt16 pOffsetValue[10],float remain)
{  
	float outFloat;
	emInt32  lOffset;
	emInt8 i;

	LOG_StackAddr(__FUNCTION__);

	lOffset = Offset;
	fRearSeek(g_hTTS->fResCurRearMain, lOffset ,0);

	for ( i=0; i<g_hTTS->pInterParam->nState; i++ )
	{
		if( g_hTTS->pRearG->nModelFloatLen == 2 )		
			fread_float2(&outFloat, g_hTTS->pRearG->nModelFloatLen ,1,g_hTTS->fResCurRearMain);	
		else
			fRearRead(&outFloat, g_hTTS->pRearG->nModelFloatLen ,1,g_hTTS->fResCurRearMain);	


		if(g_hTTS->pRearG->Speech_speed != 1.0)
		{
			outFloat /= g_hTTS->pRearG->Speech_speed;
		}
		pOffsetValue[i] =(emInt16) (outFloat + remain + 0.5); //ȡ����
		if (pOffsetValue[i]<1)
			pOffsetValue[i] = 1;
		remain += outFloat - pOffsetValue[i];
	}
	return remain;
}

/*                                                                  */
/*                                                                  */
/******************* ������ص��Ӻ������������ *********************/




/******************* ���ھ������������Ӻ����� *********************/


#if  1

//�����׾�ֵWUM����
//�Ż���ģ��ٶ����30%��

emInt16 Calc_wum (emInt16 m ,emInt16 length, emInt16 nStartLineOfPau, emInt16 nLineCountOfPau, float *wu)
{
	emInt16 n, s, it, t, t_j; // t for frames
	emInt16 i, j, k;     // for dynamic window loops
	emInt16 width, win_l_width, win_r_width;
	float mean;
	emInt16 m_width, nCurState;
	emInt16 nTotalState;
	emInt16 nFrame1, nFrame2;

	LOG_StackAddr(__FUNCTION__);

	nTotalState = nLineCountOfPau*g_hTTS->pInterParam->nState;

	width = g_hTTS->pInterParam->width;
	m_width = m*width;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	n=-1;
	t=0;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	g_pRearRTCalc->m_FrameParam[1][0] = g_pRearRTGlobal->m_MgcLeafNode[0][0][m_width];			//->	energy
	g_pRearRTCalc->m_FrameParam[1][1] = g_pRearRTGlobal->m_MgcLeafNode[0][0][m_width+1];			//->	delta energy
	g_pRearRTCalc->m_FrameParam[1][2] = g_pRearRTGlobal->m_MgcLeafNode[0][0][m_width+2];			//->	delta^2 energy
	nFrame1 = 0;
	for (s=0; s< nTotalState; s++ )
	{	
		if(s%g_hTTS->pInterParam->nState == 0)
			n++;

		nCurState = s-n*g_hTTS->pInterParam->nState;
		if (s != nTotalState-1)
		{
			if( (s+1)%g_hTTS->pInterParam->nState == 0 )					
			{
				g_pRearRTCalc->m_FrameParam[2][0] = g_pRearRTGlobal->m_MgcLeafNode[n+1][0][m*width];			//->	energy
				g_pRearRTCalc->m_FrameParam[2][1] = g_pRearRTGlobal->m_MgcLeafNode[n+1][0][m*width+1];			//->	delta energy
				g_pRearRTCalc->m_FrameParam[2][2] = g_pRearRTGlobal->m_MgcLeafNode[n+1][0][m*width+2];			//->	delta^2 energy
			}
			else
			{
				g_pRearRTCalc->m_FrameParam[2][0] = g_pRearRTGlobal->m_MgcLeafNode[n][nCurState+1][m_width];			//->	energy
				g_pRearRTCalc->m_FrameParam[2][1] = g_pRearRTGlobal->m_MgcLeafNode[n][nCurState+1][m_width+1];			//->	delta energy
				g_pRearRTCalc->m_FrameParam[2][2] = g_pRearRTGlobal->m_MgcLeafNode[n][nCurState+1][m_width+2];			//->	delta^2 energy
			}
		}
		nFrame2 = t+g_pRearRTGlobal->m_PhDuration[n][nCurState];
		
		for(it=0; it<g_pRearRTGlobal->m_PhDuration[n][nCurState] ; it++)
		{
			g_pRearRTCalc->m_wum[t] = 0.0;

			for (i = 0; i < width; i++)
			{				
				for (j = win_l_width; j <= win_r_width; j++)
				{
					t_j = t + j;

					if ((t_j >= 0) && (t_j < length) && (g_hTTS->pInterParam->win_coefficient[i][-j+1] != 0)) 
					{
						if( t_j< nFrame1)
						{
							if(i != 1)
								g_pRearRTCalc->m_wum[t] += wu[2*i+j] * g_pRearRTCalc->m_FrameParam[0][i];
							else
								g_pRearRTCalc->m_wum[t] += wu[i+(j==1?j:0)] * g_pRearRTCalc->m_FrameParam[0][i];
						}
						else if( t_j >= nFrame2 )
						{
							if(i != 1)
								g_pRearRTCalc->m_wum[t] += wu[2*i+j] * g_pRearRTCalc->m_FrameParam[2][i];
							else
								g_pRearRTCalc->m_wum[t] += wu[i+(j==1?j:0)] * g_pRearRTCalc->m_FrameParam[2][i];
						}
						else
						{
							if(i != 1)
								g_pRearRTCalc->m_wum[t] += wu[2*i+j] * g_pRearRTCalc->m_FrameParam[1][i];
							else
								g_pRearRTCalc->m_wum[t] += wu[i+(j==1?j:0)] * g_pRearRTCalc->m_FrameParam[1][i];
						}						
					}
				}
			} // for i
			t++;
		} // for it
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[0], g_pRearRTCalc->m_FrameParam[1], 4*sizeof(float));
			emMemCpy(g_pRearRTCalc->m_FrameParam[1], g_pRearRTCalc->m_FrameParam[2], 4*sizeof(float));
			nFrame1 = nFrame2;
		}
	} // for s
	return t-1;
} // end of Calc_wum

#else

//�����׾�ֵWUM����
//�Ż�ǰ��
emInt16 Calc_wum (emInt16 m ,emInt16 length, emInt16 nStartLineOfPau, emInt16 nLineCountOfPau, float *wu)
{
	emInt16 n, s, it, t, t_j; // t for frames
	emInt16 i, j, k;     // for dynamic window loops
	emInt16 width, win_l_width, win_r_width;
	float mean;
	emInt16 m_width, nCurState;
	emInt16 nTotalState;

	LOG_StackAddr(__FUNCTION__);

	nTotalState = nLineCountOfPau*g_hTTS->pInterParam->nState;

	
	width = g_hTTS->pInterParam->width;
	m_width = m*width;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	n=-1;
	t=0;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	g_pRearRTCalc->m_FrameParam[1][0] = g_pRearRTGlobal->m_MgcLeafNode[0][0][m_width];			//->	energy
	g_pRearRTCalc->m_FrameParam[1][1] = g_pRearRTGlobal->m_MgcLeafNode[0][0][m_width+1];			//->	delta energy
	g_pRearRTCalc->m_FrameParam[1][2] = g_pRearRTGlobal->m_MgcLeafNode[0][0][m_width+2];			//->	delta^2 energy
	g_pRearRTCalc->m_FrameParam[1][3] = 0.0;
	for (s=0; s< nTotalState; s++ )
	{	
		if(s%g_hTTS->pInterParam->nState == 0)
			n++;

		nCurState = s-n*g_hTTS->pInterParam->nState;
		if (s != nTotalState-1)
		{
			g_pRearRTCalc->m_FrameParam[2][0] = g_pRearRTGlobal->m_MgcLeafNode[n][nCurState+1][m_width];			//->	energy
			g_pRearRTCalc->m_FrameParam[2][1] = g_pRearRTGlobal->m_MgcLeafNode[n][nCurState+1][m_width+1];			//->	delta energy
			g_pRearRTCalc->m_FrameParam[2][2] = g_pRearRTGlobal->m_MgcLeafNode[n][nCurState+1][m_width+2];			//->	delta^2 energy
		}
		g_pRearRTCalc->m_FrameParam[2][3] = t+g_pRearRTGlobal->m_PhDuration[n][nCurState];
		for(it=0; it<g_pRearRTGlobal->m_PhDuration[n][nCurState] ; it++)
		{
			g_pRearRTCalc->m_wum[t] = 0.0;

			for (i = 0; i < width; i++)
			{
				for (j = win_l_width; j <= win_r_width; j++)
				{
					t_j = t + j;

					if ((t_j >= 0) && (t_j < length) && (g_hTTS->pInterParam->win_coefficient[i][-j+1] != 0.0)) 
					{
						if( t_j< g_pRearRTCalc->m_FrameParam[1][width])
							mean = g_pRearRTCalc->m_FrameParam[0][i];
						else if( t_j >= g_pRearRTCalc->m_FrameParam[2][width])
							mean = g_pRearRTCalc->m_FrameParam[2][i];
						else
							mean = g_pRearRTCalc->m_FrameParam[1][i];
						if(i != 1)
							g_pRearRTCalc->m_wum[t] += wu[2*i+j] * mean;
						else
							g_pRearRTCalc->m_wum[t] += wu[i+(j==1?j:0)] * mean;
					}
				}
			} // for i
			t++;
		} // for it
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[0], g_pRearRTCalc->m_FrameParam[1], 4*sizeof(float));
			emMemCpy(g_pRearRTCalc->m_FrameParam[1], g_pRearRTCalc->m_FrameParam[2], 4*sizeof(float));
		}
	} // for s
	return t-1;
} // end of Calc_wum

#endif


#if 1

//����MSD��WUW��WUM����
//�Ż���ģ��ٶ����30%��
emInt16 Calc_wuw_wum_msd(emInt16 m ,emInt16 length, emInt16 total_frame, emInt16 nStartLineOfPau, emInt16 nLineCountOfPau)
{
	emInt16 n, s, it, t, mt,t_j; // t for frames, mt for msd frames
	emInt16 i, j, k;     // for dynamic window loops
	emInt16 width, win_l_width, win_r_width;
	emInt16 frame;
	emByte  not_bound;
	float wu, mean, ivar;
	emInt16 nTotalState;
	emInt16 nFrame1, nFrame2;

	LOG_StackAddr(__FUNCTION__);

	nTotalState = nLineCountOfPau*g_hTTS->pInterParam->nState;



	width = g_hTTS->pInterParam->width;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	mt=0;
	n=-1;
	t=0;
	emMemCpy(g_pRearRTCalc->m_FrameParam[1],g_lf0param,6*sizeof(float));
	nFrame1 = 0;
	for (s=0; s< nTotalState; s++ )
	{
		if(s%g_hTTS->pInterParam->nState == 0)
			n++;
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[2],&g_lf0param[(s+1)*7],6*sizeof(float));
		}
		nFrame2 = t+g_pRearRTGlobal->m_PhDuration[n][s-n*g_hTTS->pInterParam->nState];
		for(it=0; it<g_pRearRTGlobal->m_PhDuration[n][s-n*g_hTTS->pInterParam->nState] ; it++)
		{
			if( g_pRearRTCalc->m_MSDflag[t] == VOICED) 
			{
				/* initialize */
				g_pRearRTCalc->m_wum[mt] = 0.0;
				g_pRearRTCalc->m_ldl[mt][0] = 0.0;
				g_pRearRTCalc->m_ldl[mt][1] = 0.0;
				g_pRearRTCalc->m_ldl[mt][2] = 0.0;
					/* calc WUW & WUM */
				for (i = 0; i < width; i++)
				{
					for (j = win_l_width; j <= win_r_width; j++)
					{
						t_j = t+j;
						if ((mt + j >= 0) && (mt + j < length) && (g_hTTS->pInterParam->win_coefficient[i][-j+1] != 0)) 
						{
							if( t_j < nFrame1)
							{
								mean = g_pRearRTCalc->m_FrameParam[0][i];
								ivar = g_pRearRTCalc->m_FrameParam[0][i+width];
							}
							else if( t_j >= nFrame2)
							{
								mean = g_pRearRTCalc->m_FrameParam[2][i];
								ivar = g_pRearRTCalc->m_FrameParam[2][i+width];
							}
							else
							{
								mean = g_pRearRTCalc->m_FrameParam[1][i];
								ivar = g_pRearRTCalc->m_FrameParam[1][i+width];
							}
							/* check current frame is MSD boundary or not */
							not_bound = TRUE;
							for (frame= t_j+win_l_width ; frame<= t_j+win_r_width ; frame++)
								if(frame<0 || frame>= total_frame || !g_pRearRTCalc->m_MSDflag[frame])
								{
									not_bound = FALSE;
									break;
								}
							if(g_hTTS->pRearG->f0_mean != 0.0 && i==0)
							{
								mean = log(exp(mean)+g_hTTS->pRearG->f0_mean);
								if(mean < 2.3)
									mean = (float)2.3;
							}
							if(!(not_bound || i == 0))
								ivar = 0.0;
							wu = g_hTTS->pInterParam->win_coefficient[i][-j+1] * ivar / 2;
							g_pRearRTCalc->m_wum[mt] += wu * mean;
							for (k = 0; (k < width) && (mt + k < length); k++)
								if ((k - j <= win_r_width) && (g_hTTS->pInterParam->win_coefficient[i][k - j+1] != 0))
									g_pRearRTCalc->m_ldl[mt][k] += wu * g_hTTS->pInterParam->win_coefficient[i][k - j+1] / 2;
						}
					}
				} // for i
				mt++;
			} // if msd
			t++;
		} // for it
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[0], g_pRearRTCalc->m_FrameParam[1], 7*sizeof(float));
			emMemCpy(g_pRearRTCalc->m_FrameParam[1], g_pRearRTCalc->m_FrameParam[2], 7*sizeof(float));
			nFrame1 = nFrame2;
		}
	} // for s
	
	return mt-1;
}

//bap
emInt16 Calc_wuw_wum_bap(emInt16 m, emInt16 length, emInt16 total_frame, emInt16 nStartLineOfPau, emInt16 nLineCountOfPau)
{
	emInt16 n, s, it, t, mt,t_j; // t for frames, mt for msd frames
	emInt16 i, j, k;     // for dynamic window loops
	emInt16 width, win_l_width, win_r_width;
	emInt16 frame;
	emByte  not_bound;
	float wu, mean, ivar;
	emInt16 nTotalState;
	emInt16 nFrame1, nFrame2;

	LOG_StackAddr(__FUNCTION__);

	nTotalState = nLineCountOfPau*g_hTTS->pInterParam->nState;

	width = g_hTTS->pInterParam->width;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	mt=0;
	n=-1;
	t=0;
	emMemCpy(g_pRearRTCalc->m_FrameParam[1],g_bapparam,6*sizeof(float));
	nFrame1 = 0;
	for (s=0; s< nTotalState; s++ )
	{
		if(s%g_hTTS->pInterParam->nState == 0)
			n++;
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[2], &g_bapparam[(s + 1) * 7], 6 * sizeof(float));
		}
		nFrame2 = t+g_pRearRTGlobal->m_PhDuration[n][s-n*g_hTTS->pInterParam->nState];
		for(it=0; it<g_pRearRTGlobal->m_PhDuration[n][s-n*g_hTTS->pInterParam->nState] ; it++)
		{
			/*	if( g_pRearRTCalc->m_MSDflag[t] == VOICED)
				{*/
				/* initialize */
				g_pRearRTCalc->m_wum[mt] = 0.0;
				g_pRearRTCalc->m_ldl[mt][0] = 0.0;
				g_pRearRTCalc->m_ldl[mt][1] = 0.0;
				g_pRearRTCalc->m_ldl[mt][2] = 0.0;
				/* calc WUW & WUM */
				for (i = 0; i < width; i++)
				{
					for (j = win_l_width; j <= win_r_width; j++)
					{
						t_j = t+j;
						if ((mt + j >= 0) && (mt + j < length) && (g_hTTS->pInterParam->win_coefficient[i][-j+1] != 0)) 
						{
							if( t_j < nFrame1)
							{
								mean = g_pRearRTCalc->m_FrameParam[0][i];
								ivar = g_pRearRTCalc->m_FrameParam[0][i+width];
							}
							else if( t_j >= nFrame2)
							{
								mean = g_pRearRTCalc->m_FrameParam[2][i];
								ivar = g_pRearRTCalc->m_FrameParam[2][i+width];
							}
							else
							{
								mean = g_pRearRTCalc->m_FrameParam[1][i];
								ivar = g_pRearRTCalc->m_FrameParam[1][i+width];
							}
							/* check current frame is MSD boundary or not */
							/*not_bound = TRUE;
							for (frame= t_j+win_l_width ; frame<= t_j+win_r_width ; frame++)
							if(frame<0 || frame>= total_frame || !g_pRearRTCalc->m_MSDflag[frame])
							{
							not_bound = FALSE;
							break;
							}
							if(g_hTTS->pRearG->f0_mean != 0.0 && i==0)
							{
							mean = log(exp(mean)+g_hTTS->pRearG->f0_mean);
							if(mean < 2.3)
							mean = (float)2.3;
							}
							if(!(not_bound || i == 0))
							ivar = 0.0;*/
							wu = g_hTTS->pInterParam->win_coefficient[i][-j+1] * ivar / 2;
							g_pRearRTCalc->m_wum[mt] += wu * mean;
							for (k = 0; (k < width) && (mt + k < length); k++)
								if ((k - j <= win_r_width) && (g_hTTS->pInterParam->win_coefficient[i][k - j+1] != 0))
									g_pRearRTCalc->m_ldl[mt][k] += wu * g_hTTS->pInterParam->win_coefficient[i][k - j+1] / 2;
						}
					}
				} // for i
				mt++;
		//	} // if msd
			t++;
		} // for it
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[0], g_pRearRTCalc->m_FrameParam[1], 7*sizeof(float));
			emMemCpy(g_pRearRTCalc->m_FrameParam[1], g_pRearRTCalc->m_FrameParam[2], 7*sizeof(float));
			nFrame1 = nFrame2;
		}
	} // for s

	return mt-1;
}

#else

//����MSD��WUW��WUM����
//�Ż�ǰ��
emInt16 Calc_wuw_wum_msd(emInt16 m ,emInt16 length, emInt16 total_frame, emInt16 nStartLineOfPau, emInt16 nLineCountOfPau)
{
	emInt16 n, s, it, t, mt,t_j; // t for frames, mt for msd frames
	emInt16 i, j, k;     // for dynamic window loops
	emInt16 width, win_l_width, win_r_width;
	emInt16 frame;
	emByte  not_bound;
	float wu, mean, ivar;
	emInt16 nTotalState;

	LOG_StackAddr(__FUNCTION__);

	nTotalState = nLineCountOfPau*g_hTTS->pInterParam->nState;



	width = g_hTTS->pInterParam->width;
	win_r_width = width >> 1;
	win_l_width = -win_r_width;
	mt=0;
	n=-1;
	t=0;
	emMemCpy(g_pRearRTCalc->m_FrameParam[1],g_lf0param,6*sizeof(float));
	g_pRearRTCalc->m_FrameParam[1][6] = 0.0;
	for (s=0; s< nTotalState; s++ )
	{
		if(s%g_hTTS->pInterParam->nState == 0)
			n++;
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[2],&g_lf0param[(s+1)*7],6*sizeof(float));
		}
		g_pRearRTCalc->m_FrameParam[2][6] = t+g_pRearRTGlobal->m_PhDuration[n][s-n*g_hTTS->pInterParam->nState];
		for(it=0; it<g_pRearRTGlobal->m_PhDuration[n][s-n*g_hTTS->pInterParam->nState] ; it++)
		{
			if( g_pRearRTCalc->m_MSDflag[t] == VOICED) 
			{
				/* initialize */
				g_pRearRTCalc->m_wum[mt] = 0.0;
				g_pRearRTCalc->m_ldl[mt][0] = 0.0;
				g_pRearRTCalc->m_ldl[mt][1] = 0.0;
				g_pRearRTCalc->m_ldl[mt][2] = 0.0;
					/* calc WUW & WUM */
				for (i = 0; i < width; i++)
				{
					for (j = win_l_width; j <= win_r_width; j++)
					{
						t_j = t+j;
						if ((mt + j >= 0) && (mt + j < length) && (g_hTTS->pInterParam->win_coefficient[i][-j+1] != 0.0)) 
						{
							if( t_j < g_pRearRTCalc->m_FrameParam[1][6])
							{
								mean = g_pRearRTCalc->m_FrameParam[0][i];
								ivar = g_pRearRTCalc->m_FrameParam[0][i+width];
							}
							else if( t_j >= g_pRearRTCalc->m_FrameParam[2][6])
							{
								mean = g_pRearRTCalc->m_FrameParam[2][i];
								ivar = g_pRearRTCalc->m_FrameParam[2][i+width];
							}
							else
							{
								mean = g_pRearRTCalc->m_FrameParam[1][i];
								ivar = g_pRearRTCalc->m_FrameParam[1][i+width];
							}
							/* check current frame is MSD boundary or not */
							not_bound = TRUE;
							for (frame= t_j+win_l_width ; frame<= t_j+win_r_width ; frame++)
								if(frame<0 || frame>= total_frame || !g_pRearRTCalc->m_MSDflag[frame])
								{
									not_bound = FALSE;
									break;
								}
							if(g_hTTS->pRearG->f0_mean != 0.0 && i==0)
							{
								mean = log(exp(mean)+g_hTTS->pRearG->f0_mean);
								if(mean < 2.3)
									mean = (float)2.3;
							}
							if(!(not_bound || i == 0))
								ivar = 0.0;
							wu = g_hTTS->pInterParam->win_coefficient[i][-j+1] * ivar;
							g_pRearRTCalc->m_wum[mt] += wu * mean;
							for (k = 0; (k < width) && (mt + k < length); k++)
								if ((k - j <= win_r_width) && (g_hTTS->pInterParam->win_coefficient[i][k - j+1] != 0.0))
									g_pRearRTCalc->m_ldl[mt][k] += wu * g_hTTS->pInterParam->win_coefficient[i][k - j+1];
						}
					}
				} // for i
				mt++;
			} // if msd
			t++;
		} // for it
		if (s != nTotalState-1)
		{
			emMemCpy(g_pRearRTCalc->m_FrameParam[0], g_pRearRTCalc->m_FrameParam[1], 7*sizeof(float));
			emMemCpy(g_pRearRTCalc->m_FrameParam[1], g_pRearRTCalc->m_FrameParam[2], 7*sizeof(float));
		}
	} // for s
	
	return mt-1;
}

#endif




/* ����msdֵ���ÿ֡��msd��־ */
void ModifyMSD(emInt16 nStartLineOfPau, emInt16 nLineCountOfPau)
{
	//define
	emInt16 labFrameLength;
	emInt16 labVoicedStartList[20][2];
	emInt16 labVoicedStartNum;
	emInt16 lastMsdFlag;
	emInt16 bestVoicedStart;
	emInt16 minErr,minErrId;
	emInt16 result;
	emInt16 n, s,s1, it, yt, t, i,n1,n2;
	float *msd;
	emInt16 msd_change, uvtrans;
	emInt16 Len_unvoiced, Len_voiced, time_shift, vstart;
	float ftemp, remain, ratio;
	emBool bIsFirstVoice;
	float nMsdFazhi;
#if HTRS_REAR_DEAL_CHANGE_MSD	
	int nStartFrame,state,frame,CurrDur,j;
	emInt8 CurPhone,PrePhone,NextPhone;     
	float PrevMSD,CurrMSD,NextMSD;
#endif
#if DEBUG_LOG_ENF_MSD
	emBool msdFlag2[10000];
	FILE *msd_output;
#endif
	


#if	 EM_SYS_QING_ADD_ONE_STATUS
	emByte  nPinCode[4];
	emUInt16 nLine;	

	emPInt32 pModifyOnePinYinMSD;
	emInt16	 nModifyOnePinYinMSDLine;
#endif


	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	msd = (float *) emHeap_AllocZero(g_hTTS->pInterParam->nState * sizeof(float), "���MSD�������ģ��  ��");
#else
	msd = (float *) emHeap_AllocZero(g_hTTS->pInterParam->nState * sizeof(float));
#endif


if( ! g_bRearSynChn)	//Ӣ�Ĵ���
{
	nMsdFazhi = 0.398;		//ר��ΪӢ���趨����ȡg_hTTS->pInterParam->msd_threshold��0.4��;

#if !HTRS_REAR_DEAL_CHANGE_MSD				//�����У�MSDֵ�ĺ���

	t = 0;
	for (n = 0; n < nLineCountOfPau; n++) 
	{
		for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
		{	
			result = (n*g_hTTS->pInterParam->nState+s)*7;
			msd[s] = g_lf0param[result+6];

			if ( msd[s] > nMsdFazhi )												// ����״̬
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)		// ״̬��������֡�ã�������־
					g_pRearRTCalc->m_MSDflag[t++] = VOICED;
			else																	// ����״̬
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)		// ״̬��������֡�ã�������־
					g_pRearRTCalc->m_MSDflag[t++] = UNVOICED;
		} 
	} 
#else										//���У�MSDֵ�ĺ���
#endif
}//Ӣ�Ĵ������
else					//���Ĵ���
{
	// ��ʼ��ʱ�����������ѭ������
	t=0;
	for (n = 0; n < nLineCountOfPau; n++) {
		labFrameLength=0;
		labVoicedStartNum=0;
		lastMsdFlag=FALSE;
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_letter)		//���ĺ��֣�����ĸ
		{
			// ����״̬MSDֵѰ�����е��嵽��ת����
			for (s = 0; s < g_hTTS->pInterParam->nState; s++) {
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd[s] = g_lf0param[result+6];
				if (msd[s] > g_hTTS->pInterParam->msd_threshold)
				{
					if (lastMsdFlag==FALSE)
					{
						labVoicedStartList[labVoicedStartNum][0]=labFrameLength;
						labVoicedStartList[labVoicedStartNum][1]= n*g_hTTS->pInterParam->nState+s;
						labVoicedStartNum++;
					}
					lastMsdFlag=TRUE;
				}
				else
					lastMsdFlag=FALSE;
				labFrameLength += g_pRearRTGlobal->m_PhDuration[n][s];
			}
			//sqb 2016/12/15
			if ((g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_pau)
				|| (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_nil))
			{
				for (s = 0; s < g_hTTS->pInterParam->nState; s++)
					msd[s] *= -1;
			}

			/*
			
			// Ѱ�����к�ѡת��������ο�ת���������ת����,sil��pau������
			if((g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_pau) 
				&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]!= FINAL_CN_nil)) 
			{
				// ���߳����Ĳο�ת����
				bestVoicedStart = (emInt16)(labFrameLength * syl_uvp[n]);
				minErr=10000;
				minErrId=-1;
				// Ѱ�������С��ת���㣬labVoicedStartListΪ��ѡת�����б�
				for (i=0;i<labVoicedStartNum;i++)
				{
					if (minErr>emAbs(labVoicedStartList[i][0]-bestVoicedStart))
					{
						minErr=emAbs(labVoicedStartList[i][0]-bestVoicedStart);
						minErrId=i;
					}
				}

				//������ǣ�����ĸ��������������R�����ܵ�������ĸ���ҵõ�������ת����Ϊ0�������������1����ת���㣨һ���б������2������ת���㣬1��ǰ1���ڲ�����������1��������������
				//���磺 ��ͷ�Լ���̨���йص�֪ʶ
				if (labVoicedStartList[minErrId][0] == 0)
				{
					if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_zero  &&		//��Ϊ����ĸ
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_l &&			//��Ϊl
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_m &&			//��Ϊm
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_n &&			//��Ϊn
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_r )				//��Ϊr
					{
						minErr=10000;
						minErrId=-1;
						for (i=1;i<labVoicedStartNum;i++)
						{
							if (minErr>emAbs(labVoicedStartList[i][0]-bestVoicedStart))
							{
								minErr=emAbs(labVoicedStartList[i][0]-bestVoicedStart);
								minErrId=i;
							}
						}
					}
				}

					// �޸�״̬MSDֵ��ʹ����״̬MSDС��0������״̬MSD����1������ǿ���ֶ�
				if (minErrId>=0)
				{
					msd_change = 0;

#if	WL_REAR_DEAL_LETTER		//���֣���ĸ�ĵ�������
					// naxy 1219 ���ǰһ����������Ӣ����ĸ���򲻱ر�����һ������״̬
					if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_f ||		//��ĸ��
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_h ||		//��ĸ��
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_s ||		//��ĸ��
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_x )		//��ĸ��
					{
						msd_change = 1;
					}
#endif

					// ����״̬������-1��
					for (i = n*g_hTTS->pInterParam->nState;
						i<labVoicedStartList[minErrId][1];
						i++) 
					{	
#if WL_REAR_DEAL_GONG4_GONG4
						//���ƺϳ�ʱ��Щ�����������ء���Ծʽ����
						//ÿ���ֵĵ�1��״̬���������ߵĽ�������ݡ�������ת���㡷ǿ�Ƹı䡣�����gong4�������⣺�������β����gong4�������⣩
						if(msd[i-n*g_hTTS->pInterParam->nState]<g_hTTS->pInterParam->msd_threshold)
							msd_change = 1;
						if(msd_change == 1)
							msd[i-n*g_hTTS->pInterParam->nState] *= -1;
#else
						//ԭ��û���Ƶ�
						msd[i-n*g_hTTS->pInterParam->nState] *= -1;
#endif
					}

					// ����״̬������1��
					for (i = (n+1)*g_hTTS->pInterParam->nState-1;
						i>=labVoicedStartList[minErrId][1];	i--)
						msd[i-n*g_hTTS->pInterParam->nState] += 1.0;
				}
			} 
			// �����sil��pau��ȫ����Ϊ����״̬
			else 
			{
				for(s = 0; s < g_hTTS->pInterParam->nState; s++)
					msd[s] *= -1;
			}
			*/
		} 
		// ��ĸ�Ĵ���
		else 
		{	

#if	WL_REAR_DEAL_LETTER		//���֣���ĸ�ĵ�������

			//Ӣ����ĸ�Ĵ��� naxy 1219
			for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd[s] = g_lf0param[result+6];
			}

			//ǿ��Ϊ������+����
			//���ӣ���f�͡���h�͡���k�͡���s�͡�
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_f ||		
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_h ||		
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_s ||		
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_x )		
			{
				for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
				{		//�ҵ�������ʼ
					if(msd[s]>g_hTTS->pInterParam->msd_threshold)
						break;
				}
				for (; s < g_hTTS->pInterParam->nState; s++) 
				{			//�ҵ����Ǳ���ĵ�һ������֡
					if(msd[s]<g_hTTS->pInterParam->msd_threshold)
						break;
				}
				result = s;
				
				for (s = result+1; s < g_hTTS->pInterParam->nState; s++) 
				{	
					if(s >= 8 )  //20120904 �Ӵ˾� ������8Kģ�ͻ��д�X��H����
					{
						//ǿ�Ƹ��ĺ�����е�״̬������msdֵ����
						if(msd[s]>g_hTTS->pInterParam->msd_threshold)
						{
							emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[n][s], g_pRearRTGlobal->m_MgcLeafNode[n][result],
								g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width * sizeof(float));
							msd[s] = 0.1;
						}
					}
				}

			} 			
			else if (  g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_a ||	
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_e ||	
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_i ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_l ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_m ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_n ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_o ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_r ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_u ||		
					   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_y )		
			{
				for (s = 0; s < g_hTTS->pInterParam->nState; s++)
					msd[s] = 0.9;										//ǿ��Ϊȫ����

				//�ı���������ȥ�����⣺(ֻ����1��ĸ��C�й�) ���磺AC��EC��IC��NC��UC�� AAC��EEC��IIC��NNC��UUC��
				if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM]  == FINAL_CN_letter_c )		//��1��ĸ��C�Ļ�
				{
					g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 0.7;
					g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 0.7;
				}
			}
			else if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_b ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_c ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_d ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_g ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_j ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_k ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_p ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_q ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_t ||						
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_w ||		
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_z )		
			{

#if EM_8K_MODEL_PLAN	//8Kģ������  ��ֹw,dȫ�ǵ����
				//��ֹȫ�ǵ����  hyl 2012-10-12	
				//���磺Power Downģʽ��Www��WinCE��wa��wb��wc��wd��w�ˡ�w�ġ�wΪ��
				//���磺LA40B530P7R��wb��wd��
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_d || 
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_w )	
				{
					msd[0] = 0.1;
				}
#else					//16Kģ������
				//��ֹȫ�ǵ����  hyl  2012-04-20   ����ĸb��d��������
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_b ||	
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_d ) 
				{
					msd[0] = 0.1;
				}
#endif
				

				for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
				{		//�ҵ�������ʼ
					if(msd[s]<g_hTTS->pInterParam->msd_threshold)
						break;
				}
				for (; s < g_hTTS->pInterParam->nState; s++) 
				{			//�ҵ�������ǵĵ�һ������֡
					if(msd[s]>g_hTTS->pInterParam->msd_threshold)
						break;
				}
				result = s;
				
				for (s = result+1; s < g_hTTS->pInterParam->nState; s++) 
				{	//ǿ�Ƹ��ĺ�����е�״̬Ϊ����
					if(msd[s]<g_hTTS->pInterParam->msd_threshold)
						msd[s] = 0.9;
				}			

				//�ı���������ȥ�����⣺(ֻ����1��ĸ��C�й�) ���磺BC��DC��GC��JC��KC��PC��TC��ZC�� BBC��DDC��GGC��JJC��KKC��PPC��TTC��ZZC��
				if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM]  == FINAL_CN_letter_c )		//��1��ĸ��C�Ļ�
				{
					g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 0.7;
					g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 0.7;
				}
				
			} 
			else if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_v )		//v
			{
				for (s = 2; s < g_hTTS->pInterParam->nState; s++) 
					msd[s] = 0.9;

				//�ı���������ȥ�����⣺(ֻ����1��ĸ��C�й�) ���磺VC�� VVC��
				if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM]  == FINAL_CN_letter_c )		//��1��ĸ��C�Ļ�
				{
					g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 0.7;
					g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 0.7;
				}
			}

			//����֨֨�����ı�K��ĸ�ĵ�3��״̬��ʱ�����ָ�ǰ���1״̬
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_k )		//k
			{
				g_pRearRTGlobal->m_PhDuration[n][1] += g_pRearRTGlobal->m_PhDuration[n][2] -1;
				g_pRearRTGlobal->m_PhDuration[n][2] = 1;
			}
#endif

		}

//���ƣ��������б𲻺á����⣺ �򿪡�ǿ��ĳЩƴ��������1��״̬������
#if	 EM_SYS_QING_ADD_ONE_STATUS

		if(g_hTTS->pRearG->stage == MODEL_MGC)	//MGCģ��
		{
			pModifyOnePinYinMSD     = g_ModifyOnePinYinMSD_StrMgc;
			nModifyOnePinYinMSDLine = ModifyOnePinYinMSD_Line_StrMgc;
		}
		else							//LSPģ��
		{
			//����ģ�ͣ� ��207-2ģ�ͣ�StrLsp������Ƶ��StrMgc���򿪡�ǿ��ĳЩƴ��������1��״̬�����أ�
			pModifyOnePinYinMSD     = g_ModifyOnePinYinMSD_StrLsp_MgcLf0;
			nModifyOnePinYinMSDLine = ModifyOnePinYinMSD_Line_StrLsp_MgcLf0;

			//����ģ�ͣ� ��207-1ģ�ͣ�StrLsp
			//pModifyOnePinYinMSD     = g_ModifyOnePinYinMSD_StrLsp;
			//nModifyOnePinYinMSDLine = ModifyOnePinYinMSD_Line_StrLsp;
		}	
		
		nPinCode[1] = g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]<<3 | g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE];
		nPinCode[0] = g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM];
		nPinCode[2] = 0;
		nPinCode[3] = 0;
		nLine = ErFenSearchOne_FromRAM( nPinCode, 0, pModifyOnePinYinMSD, 0, nModifyOnePinYinMSDLine, 4 );
		if( nLine < nModifyOnePinYinMSDLine)
		{
			
			for (s = 0; s < g_hTTS->pInterParam->nState-1; s++)
			{
				//��ǰ1״̬����������1״̬���������򽫺�1״̬ǿ�Ƴ�������Ȼ��break
				if(msd[s] < g_hTTS->pInterParam->msd_threshold && msd[s+1] > g_hTTS->pInterParam->msd_threshold)
				{
					msd[s+1] = 0.1;
					break;
				}

				//���ru4 ri4�Ĵ���
				if( s==1 && msd[s] > g_hTTS->pInterParam->msd_threshold && msd[s+1] > g_hTTS->pInterParam->msd_threshold )
				{
					msd[s] = 0.1;
					break;
				}
			}

		}
#endif

		// ��״̬MSDֵ���еĺ���
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_letter &&		//�ų�sil��pau����ĸ
			(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_pau) 
			&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]!=FINAL_CN_nil)) 
		{
			// �ҵ�����ת��״̬���Ա�����
			for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
			{
				if (msd[s]<g_hTTS->pInterParam->msd_threshold)
					break;
			}
			if(s == g_hTTS->pInterParam->nState) // ȫ����
				msd_change = 0;
			else
				msd_change = s;

#if UVP_DUR_MOD  // mofidy voicing label using duration shifting, naxy 2011-11-30
		
			if(syl_uvp[n]>0) 
			{
				Len_unvoiced = 0;
				Len_voiced = 0;
				
				for (s = msd_change; s < g_hTTS->pInterParam->nState; s++) 
				{
					if (msd[s]>g_hTTS->pInterParam->msd_threshold)
						Len_voiced += g_pRearRTGlobal->m_PhDuration[n][s];
					else
						Len_unvoiced += g_pRearRTGlobal->m_PhDuration[n][s];
				}
				yt = Len_voiced + Len_unvoiced;
				//mofidy duration of unvoiced part
				//2012-12-31
				remain = 0.0;
				ratio =  (yt * syl_uvp[n]) / Len_unvoiced;
				if(syl_uvp[n]>0.5 && ratio>1.5)
					ratio = 1.0;
				Len_unvoiced = 0;
			
				for (s = msd_change; s < g_hTTS->pInterParam->nState; s++) 
				{
					if(msd[s]>g_hTTS->pInterParam->msd_threshold)
						break;
					ftemp = g_pRearRTGlobal->m_PhDuration[n][s] * ratio;
					g_pRearRTGlobal->m_PhDuration[n][s] = (emInt16) (ftemp + remain + 0.5);
					if(g_pRearRTGlobal->m_PhDuration[n][s]<1)
						g_pRearRTGlobal->m_PhDuration[n][s] = 1;
					Len_unvoiced += g_pRearRTGlobal->m_PhDuration[n][s];
					remain += ftemp - g_pRearRTGlobal->m_PhDuration[n][s];
				}
				ratio =  (float)(yt - Len_unvoiced) / Len_voiced;
				Len_voiced = 0;
				s1 = s;
				for (; s < g_hTTS->pInterParam->nState; s++) 
				{
					ftemp = g_pRearRTGlobal->m_PhDuration[n][s] * ratio;
					g_pRearRTGlobal->m_PhDuration[n][s] = (emInt16) (ftemp + remain + 0.5);
					if(g_pRearRTGlobal->m_PhDuration[n][s]<1)
						g_pRearRTGlobal->m_PhDuration[n][s] = 1;
					Len_voiced += g_pRearRTGlobal->m_PhDuration[n][s];
					remain += ftemp - g_pRearRTGlobal->m_PhDuration[n][s];
				}
			}

#endif


#if UVP_DUR_MOD_P //����MSD�о�����MSDֵ��0.4��0.5֮���������ʼ�߽�״̬�о�Ϊ����״̬��naxy 2011-12-02
			// ʱ���ϳ���״̬��msd�����ڶ����о����޸ģ�naxy, 2011-12-30 �޸�
			for(s=1; s < g_hTTS->pInterParam->nState; s++) 
			{
				if(msd[s-1] < g_hTTS->pInterParam->msd_threshold && msd[s] > g_hTTS->pInterParam->msd_threshold
					&& msd[s] < (g_hTTS->pInterParam->msd_threshold+1.1))
				{
					//������������1��״̬С��4֡���ҵ�2״̬��msdֵ��0.4��0.5֮�䣬
					//�ҷ�����ʽΪ��g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] :3	asprt_stop������	p,t,k  8	stop����	b,d,g
					//��ǿ�Ƶ�2״̬Ϊ����
					if(s==1						//&& g_pRearRTGlobal->m_PhDuration[n][s-1] < 4 
						&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_asprt_stop 
						|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD]==SM_CN_METHOD_stop)  )							
					{
						break;
					}
					else if(g_pRearRTGlobal->m_PhDuration[n][s] > 1)
					{
						break;
					}
					else
					{
						msd[s] = 0.1;
						break;
					}
				}
			}

#endif


#if WL_REAR_DEAL_GONG4_GONG4
			// �޸�������ĸ���ڵ�2��3״̬��MSDֵ��ǿ��������ĸΪһ��״̬��Ϊ����gong4��naxy 2011-12-30 
			if(    (    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_b		//��ĸ��b
				     || g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_d)	//��ĸ��d
				&&	g_pRearRTGlobal->m_PhDuration[n][0] > 2 )
			{
				msd[1] = 1.9;
				msd[2] = 1.9;

				//��������
				//for (s=0; s < g_hTTS->pInterParam->nState; s++)
				//	if(msd[s] < g_hTTS->pInterParam->msd_threshold)
				//	{
				//		n1 = g_pRearRTGlobal->m_PhDuration[n][s] - 2;
				//		if(n1>0)
				//		{
				//			g_pRearRTGlobal->m_PhDuration[n][s] = 2;
				//			for (s++; s < g_hTTS->pInterParam->nState; s++)
				//			{
				//				if(n1==0)
				//					break;
				//				g_pRearRTGlobal->m_PhDuration[n][s]++;
				//				n1--;
				//			}
				//		}
				//	}
			}

#endif



#if	WL_REAR_DEAL_R_INITIAL
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r)			//��ĸΪ��r
				{
					//��ĸΪ��i  e  
					if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_iii || g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_e  )	
					{
						msd[0] = 0.1;		//	ǿ�Ʊ�״̬Ϊ������״̬
						msd[1] = 0.1;		//	ǿ�Ʊ�״̬Ϊ������״̬
						msd[2] = 0.1;		//	ǿ�Ʊ�״̬Ϊ������״̬
					}
					else			//��r��ĸ������:ri, re, ru
					{
						//hyl 2012-04-12  ���磺[f0]ÿ���������Ķ�������ʵ�������ӡ���[d]  �ˡ��̡��ϡ��ԡ��⡣
						if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH] == 0  //������ʱ(�Ҳ��ǣ�ren)��ǿ�Ƶ�1��״̬�������������ף��򲻸ı�
							&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]!=FINAL_CN_en)
							msd[0] = 0.1;			
					}
				}

								
#endif


#if	WL_REAR_DEAL_LITTER_QST
			// naxy1208��12��8�ճ��ֵ��������
			for(s=msd_change; s < g_hTTS->pInterParam->nState; s++) 
				if (msd[s]>g_hTTS->pInterParam->msd_threshold)
					break;
			vstart = s;
			
			// ch, t��p��k��ĸ����ת��С�������⣺ �������ĵ�1��״̬��ʱ������1֡��������֡��ƽ�ָ�����״̬
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_ch ||  
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_k  ||  
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_p  ||  
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_t )   
			{
				time_shift = g_pRearRTGlobal->m_PhDuration[n][vstart] - 2; 
				g_pRearRTGlobal->m_PhDuration[n][vstart] = 2;
				while(time_shift>0)		
				{
					for(s=vstart-1;s>=0;s--)
					{
						g_pRearRTGlobal->m_PhDuration[n][s]++;
						time_shift--;
						if(time_shift==0)
							break;
					}
				}

			}

			// qiang ����֨֨���ĺ���2012-01-18
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_q &&  
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_iang)	// iang
				msd[vstart] = 0.1;

			// qie2 ����֨֨���ĺ���naxy120315
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_q &&
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_ie &&
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_2)
				msd[vstart] = 0.1;

			/*
			//���²��Բ�Ҫ�����ٳ���
			// zh��ĸ����ת��С��������
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zh )  
			{
				if(vstart==2)
					msd[vstart] = 0.1;
			}
			*/

			
			//���²����ݲ�Ҫ���������ܸ���   --��Ҫɾ
			// i��u��e��a��ĸ���������м�״̬����ͻ��С������  //̬�Ⱥܺá��������ˡ����İְ֡��ܿ�ͳһ�ˡ�
			//if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_a ||  // a
			//   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e ||  // e
			//   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_i ||  // i
			//   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_u )   // u
			//{
			//	for(s=vstart+1;s<g_hTTS->pInterParam->nState-2; s++)
			//		if(g_pRearRTGlobal->m_MgcLeafNode[n][s][0]<(g_pRearRTGlobal->m_MgcLeafNode[n][s-1][0]+g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0])/2)
			//			g_pRearRTGlobal->m_MgcLeafNode[n][s][0]=(g_pRearRTGlobal->m_MgcLeafNode[n][s-1][0]+g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0])/2;
			//}
#endif


#if	WL_REAR_DEAL_LITTER_QST

			// ����l����ȫ�������ڣ�naxy, 2011-12-30 �޸�
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]== INITIAL_CN_zero ||			//Ϊ����ĸ��y, w��a��er: ǿ������״̬Ϊ����
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_lateral)			//������l
			{
				msd[0] = 1.9;	
				msd[1] = 1.9;	
			}

			/*
			//���²��Բ�Ҫ��֨֨���ܸ���һ�㣬�����Ȳִ٣�����Щ��仵
			// ����ĸ����С������
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_zero)	//���顣���İ��̡�ѧϰ�Ļ������ǵ����硣�����˶����ܶ࿹��������ͼ�������ƽ�����Ҫ�ֵ��ϡ�
			{
				time_shift = g_pRearRTGlobal->m_PhDuration[n][0] + g_pRearRTGlobal->m_PhDuration[n][1] + g_pRearRTGlobal->m_PhDuration[n][2] - 3; 
				g_pRearRTGlobal->m_PhDuration[n][0] = 1;
				g_pRearRTGlobal->m_PhDuration[n][1] = 1;
				g_pRearRTGlobal->m_PhDuration[n][2] = 1;
				while(time_shift>0)
				{
					for(s=3;s<g_hTTS->pInterParam->nState;s++)
					{
						g_pRearRTGlobal->m_PhDuration[n][s]++;
						time_shift--;
						if(time_shift==0)
							break;
					}
				}
			}*/
#endif


//#if	WL_REAR_DEAL_FOU3						//����������������ֵ�4��5��״̬ǿ��Ϊ����  2012-04-18
//			//��ǰ����Ϊ ��fou3��
//			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_f 
//				&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_ou 
//				&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_3)
//			{
//				msd[3] = 1.9;	
//				msd[4] = 1.9;	
//			}
//#endif


		}


#if WL_REAR_DEAL_SYL_TOO_FAST				//���֣��ı�������ڹ�������  

		//***************  �����ݲ��򿪣��ݲ��� ����ɾ *********************
		
		//���磺��ӭ���١���������ô����ġ����ִ�ҵ���Ҳ���ʲô�������ơ�����Ϊ��ð���ò�����ڼ���Ϣ��
		//���������ٹ�������
		//#define  V_MIN_FRAME_FOR_WL			15	//����������������֡��С�ڴ���������ı䣨��g_hTTS->pRearG->Speech_speed=0.9��
		//#define  V_CHANGE_MIN_FRAME_FOR_WL	18	//���������¸ı���������С֡������g_hTTS->pRearG->Speech_speed=0.9��
		//if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]<=TONE_CN_4)		//����С��4���ų�������sil��pau����ĸ
		//{
		//	Len_voiced = 0;
		//	Len_unvoiced = 0;
		//	for(s=0;s<g_hTTS->pInterParam->nState;s++)
		//		if (msd[s]<g_hTTS->pInterParam->msd_threshold)
		//			break;
		//	if(s==g_hTTS->pInterParam->nState)
		//		vstart = 0;
		//	else
		//	{
		//		for(;s<g_hTTS->pInterParam->nState;s++)
		//			if (msd[s]>g_hTTS->pInterParam->msd_threshold)
		//				break;
		//		vstart = s;
		//	}
		//	for(s=0;s<g_hTTS->pInterParam->nState;s++)
		//	{
		//		if( s>= vstart)
		//			Len_voiced += g_pRearRTGlobal->m_PhDuration[n][s];
		//		else
		//			Len_unvoiced += g_pRearRTGlobal->m_PhDuration[n][s];
		//	}

		//	if( Len_voiced<=(V_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed)  )  
		//	{
		//		n1 = (V_CHANGE_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed) - Len_voiced;	//Ҫ������֡��

		//		if( (Len_unvoiced - n1) <= vstart)		//������Ҫ��������ÿ��״̬1֡
		//		{
		//			break;
		//		}
		//		else
		//		{
		//			n2 = n1;
		//			while( n1>0 )
		//			{
		//				for(s=vstart ; s<g_hTTS->pInterParam->nState ; s++ )
		//				{
		//					g_pRearRTGlobal->m_PhDuration[n][s]++;	
		//					n1--;
		//					if(n1==0)
		//						break;
		//				}
		//			}

		//			while( n2>0 )
		//			{
		//				for(s=0 ; s<vstart ; s++ )
		//				{
		//					g_pRearRTGlobal->m_PhDuration[n][s]--;	
		//					n2--;
		//					if(n2==0)
		//						break;
		//				}
		//			}
		//		}
		//	}
		//}
#endif

#if WL_REAR_DEAL_LITTER_QST

		//����֨֨���������׵�shi4��shi1��msd[4]ǿ��Ϊ����  hyl 2012-04-20
		//���磺 [z1][v10]ʵʩ��ʷ�ǡ�ʧ���л����Ƿ���ע�⳵��#�Ƿ�򿪡���ע�⳵��#ʵʩ�򿪡���ע�⳵��#ʧ��򿪡���ע�⳵��#ʷ�Ǵ򿪡��л��ϰ����л��ϰ�#����112ǰ#�Ӳ���;���š�
		if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_sh 
			&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_iii
			&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_1 || g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_4)
			&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH] == 0
			)
		{
			if(  g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_SEN] != 0 )
			{
				msd[4] = 0.1;
				msd[5] = 0.1;
			}
			else
				msd[4] = 0.1;
			
		}

		//����֨֨������qiao��msd[3]ǿ��Ϊ����  hyl 2012-04-23
		//���磺 [v10]���á����š����ɡ����̡�
		if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_q 
			&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_iao
			)
		{		
			msd[3] = 0.1;			
		}

#endif

#if WL_REAR_DEAL_GONG4_GONG4 && EM_8K_MODEL_PLAN	  //8Kģ�����ã�gong4 gong4���Ĵ���  2012-10-12 hyl
		//���磺��ӭ�ۿ������ϳ�ϵͳ����ʾ��ӭ�ۿ������ϳ�ϵͳ����ʾ
		//���磺�����˰˺š������̴�5���������ع��ӽ�����ˮ������Ϯ��
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] != FINAL_CN_nil )  //ǰ���ڲ���sil��pau
		{
			if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_h 
				|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_s )
			{
				msd[0] = 1.9;	
			}
		}
#endif

		// ����g_lf0param�����е�MSDֵ�Ա���һ��
		for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
		{
			result = (n*g_hTTS->pInterParam->nState+s)*7;
			g_lf0param[result+6] = msd[s];
		}

		// ���õ�ǰ����ÿһ֡��������״̬������֡���Ǳ�־���к���
		bIsFirstVoice = emFalse;		//�Ƿ����ڵĵ�1������״̬
		for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
		{
			// ����״̬�Ĵ���
			if (msd[s]>g_hTTS->pInterParam->msd_threshold)
			{

				// Ϊ���gong4����������֡��ǰ�����Դ��������֡MSD��ֵ֮ǰ
#if	WL_REAR_DEAL_GONG4_GONG4			
				
				//����gong4��������Ŀ�����ڣ�������ǰ��2֡��naxy 1219
				//Ŀǰ���ˣ�shi  suo  sui  zhe  j  x
				//
				//Ŀǰ�Ҳ���һЩ�������ֺ����������Ĳ�ͬ�����ġ�
				//���ݹ۲�����shi��suo�������ڵ�gong4��Ӱ��ϴ󣬸��ݴ˷������ڿ��Ը��ơ�
				//�����������ڸ���Ŀǰ�Ĳ��Խ��������û�б�Ҫ���е�������Ϊ������������������ɵ�Ƶ����������
				//��һ�������Ժ󣬿���������������ĵ�������ǰ�Ƶ�֡��Ҳ���Ըı䡣
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_sh			
					&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_iii		
					&& syl_uvp[n]>0.6 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED && s>0 
					&& g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}



				// �����µ����ڲ��ԣ�naxy, 2011-12-30 �޸�
				// suo, sui
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_s			
					&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_uo 
					|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]  == FINAL_CN_uei)		//uo,ui
					&& syl_uvp[n]>0.44								// ��ֵԽС�޸ĵķ�ΧԽ��ԭ�汾��0.45��2012-01-18 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}
				// zhe
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zh
					&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e)		//e
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_j 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					//g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_x 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					//g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}

				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_h 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}

#endif
				// ����״̬��������֡����λ
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)
				{
					g_pRearRTCalc->m_MSDflag[t] = VOICED;
					t++;
				}
				bIsFirstVoice = emTrue ;
			}
			// ����״̬�Ĵ���
			else
			{
				// ����״̬��������֡����λ
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)
				{
					g_pRearRTCalc->m_MSDflag[t] = UNVOICED;
					t++;
				}
			}
		} // for s��״̬ѭ��
	} // for n���ֶ���ѭ��
}//���Ĵ������


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(msd, g_hTTS->pInterParam->nState * sizeof(float), "���MSD�������ģ��  ��");
#else
	emHeap_Free(msd, g_hTTS->pInterParam->nState * sizeof(float));
#endif

} // end of ModifyMSD




/* WUW��LDL�ֽ� */
void LDL_facorication( emInt16 length, float *wuw)
{
   emInt16 t, i, j,  t_j;

	LOG_StackAddr(__FUNCTION__);

   for (t = 0; t < length; t++) 
   {
      if (t>0 && t< length-1)
	     g_pRearRTCalc->m_ldl[t][0] = wuw[1];
	  else
		 g_pRearRTCalc->m_ldl[t][0] = wuw[0];
      for (i = 1; (i < g_hTTS->pInterParam->width) && (t >= i); i++)
         g_pRearRTCalc->m_ldl[t][0] -= g_pRearRTCalc->m_ldl[t - i][i] * g_pRearRTCalc->m_ldl[t - i][i] * g_pRearRTCalc->m_ldl[t - i][0];

      for (i = 1; i < g_hTTS->pInterParam->width; i++) 
	  {
		 if(t<length-2) 
		 {
			g_pRearRTCalc->m_ldl[t][i] = wuw[i+1];
			for (j = 1; (i + j < g_hTTS->pInterParam->width) && (t >= j); j++)
			{
				t_j = t-j;
				g_pRearRTCalc->m_ldl[t][i] -= g_pRearRTCalc->m_ldl[t_j][j] * g_pRearRTCalc->m_ldl[t_j][i + j] * g_pRearRTCalc->m_ldl[t_j][0];
			}
            g_pRearRTCalc->m_ldl[t][i] /= g_pRearRTCalc->m_ldl[t][0];
		 } 
		 else if((t == length-2) && (i == 1)) 
		 {
		    g_pRearRTCalc->m_ldl[t][i] = wuw[i+1];
			for (j = 1; (i + j < g_hTTS->pInterParam->width) && (t >= j); j++)
			{
				t_j = t-j;
				g_pRearRTCalc->m_ldl[t][i] -= g_pRearRTCalc->m_ldl[t_j][j] * g_pRearRTCalc->m_ldl[t_j][i + j] * g_pRearRTCalc->m_ldl[t_j][0];
			}
			g_pRearRTCalc->m_ldl[t][i] /= g_pRearRTCalc->m_ldl[t][0];
		 } 
		 else
			g_pRearRTCalc->m_ldl[t][i] = 0.0;
      } // for i
   } // for t
} // end of LDL_facorication

/* WUW��LDL�ֽ� */
void LDL_facorication_msd( emInt16 length )
{
   emInt16 t, i, j, t_i, t_j;

	LOG_StackAddr(__FUNCTION__);

   for (t = 0; t < length; t++) 
   {
      for (i = 1; (i < g_hTTS->pInterParam->width) && (t >= i); i++)
	  {
			t_i = t-i;
			g_pRearRTCalc->m_ldl[t][0] -= g_pRearRTCalc->m_ldl[t_i][i] * g_pRearRTCalc->m_ldl[t_i][i] * g_pRearRTCalc->m_ldl[t_i][0];
	  }

      for (i = 1; i < g_hTTS->pInterParam->width; i++) 
	  {
         for (j = 1; (i + j < g_hTTS->pInterParam->width) && (t >= j); j++)
		 {
				t_j = t-j;
				g_pRearRTCalc->m_ldl[t][i] -= g_pRearRTCalc->m_ldl[t_j][j] * g_pRearRTCalc->m_ldl[t_j][i + j] * g_pRearRTCalc->m_ldl[t_j][0];
		 }
         g_pRearRTCalc->m_ldl[t][i] /= g_pRearRTCalc->m_ldl[t][0];
      } // for i
   } // for t
} // end of LDL_facorication

/* WUW��LDL�ֽ� */
void LDL_facorication_bap(emInt16 length)
{
	emInt16 t, i, j, t_i, t_j;

	LOG_StackAddr(__FUNCTION__);

	for (t = 0; t < length; t++)
	{
		for (i = 1; (i < g_hTTS->pInterParam->width) && (t >= i); i++)
		{
			t_i = t - i;
			g_pRearRTCalc->m_ldl[t][0] -= g_pRearRTCalc->m_ldl[t_i][i] * g_pRearRTCalc->m_ldl[t_i][i] * g_pRearRTCalc->m_ldl[t_i][0];
		}

		for (i = 1; i < g_hTTS->pInterParam->width; i++)
		{
			for (j = 1; (i + j < g_hTTS->pInterParam->width) && (t >= j); j++)
			{
				t_j = t - j;
				g_pRearRTCalc->m_ldl[t][i] -= g_pRearRTCalc->m_ldl[t_j][j] * g_pRearRTCalc->m_ldl[t_j][i + j] * g_pRearRTCalc->m_ldl[t_j][0];
			}
			g_pRearRTCalc->m_ldl[t][i] /= g_pRearRTCalc->m_ldl[t][0];
		} // for i
	} // for t
} // end of LDL_facorication


/* ǰ���� */
void Forward_substitution( emInt16 m, emInt16 length )
{
	emInt16 t, t_1, t_2;

	LOG_StackAddr(__FUNCTION__);

	t = 0;
	g_pRearRTCalcFilter->m_CmpFeature[t][m] = g_pRearRTCalc->m_wum[t];
    t++;
	t_1 = t-1;
	g_pRearRTCalcFilter->m_CmpFeature[t][m] = g_pRearRTCalc->m_wum[t]
												- g_pRearRTCalc->m_ldl[t_1][1] * g_pRearRTCalcFilter->m_CmpFeature[t_1][m];
    t++;
	do
	{
		t_1 = t-1;
		t_2 = t-2;
		g_pRearRTCalcFilter->m_CmpFeature[t][m] = g_pRearRTCalc->m_wum[t]
													- g_pRearRTCalc->m_ldl[t_1][1] * g_pRearRTCalcFilter->m_CmpFeature[t_1][m]
													- g_pRearRTCalc->m_ldl[t_2][2] * g_pRearRTCalcFilter->m_CmpFeature[t_2][m];
		g_pRearRTCalcFilter->m_CmpFeature[t][20]=0;
        t++;
	}while ( length - t > 0);// for t
} // end of Forward_substitution


/* ���������� */
void Backward_substitution( emInt16 m, emInt16 length )
{
   emInt16 t, length_1,  length_2;

	LOG_StackAddr(__FUNCTION__);

   length_1 = length-1;
   length_2 = length-2;
   g_pRearRTCalcFilter->m_CmpFeature[length_1][m] /= g_pRearRTCalc->m_ldl[length_1][0];
   g_pRearRTCalcFilter->m_CmpFeature[length_2][m] = g_pRearRTCalcFilter->m_CmpFeature[length_2][m] / g_pRearRTCalc->m_ldl[length_2][0] -
								g_pRearRTCalc->m_ldl[length_2][1] * g_pRearRTCalcFilter->m_CmpFeature[length_1][m];
   for (t = length - 3; t >= 0; t--) 
   {
      g_pRearRTCalcFilter->m_CmpFeature[t][m] = g_pRearRTCalcFilter->m_CmpFeature[t][m] / g_pRearRTCalc->m_ldl[t][0] -
							g_pRearRTCalc->m_ldl[t][1] * g_pRearRTCalcFilter->m_CmpFeature[t + 1][m] - 
							g_pRearRTCalc->m_ldl[t][2] * g_pRearRTCalcFilter->m_CmpFeature[t + 2][m];
   } // for t
} // end of Backward_substitution

/* GV �������ɣ�����ʼ���� */
void GV_paramgen(emInt16 m, emInt16 length, float weight)
{
	emInt16 t;
	float mean, vari, ratio;

	LOG_StackAddr(__FUNCTION__);

	mean = 0;
	for(t= 0; t< length ; t++)
		mean += g_pRearRTCalcFilter->m_CmpFeature[t][m];
	mean /= length;

	vari = 0;
	for(t= 0; t< length ; t++)
		vari += (g_pRearRTCalcFilter->m_CmpFeature[t][m] - mean) * (g_pRearRTCalcFilter->m_CmpFeature[t][m] - mean);
	vari /= length;

	ratio = sqrt(g_gv[0] / vari);
	for(t= 0; t< length ; t++)
		g_pRearRTCalcFilter->m_CmpFeature[t][m] += weight * ( ratio * (g_pRearRTCalcFilter->m_CmpFeature[t][m] - mean) + mean - g_pRearRTCalcFilter->m_CmpFeature[t][m]);
}

/* ��������֡��Ƶ��־λ */
emInt16 AddUnvoiced(emInt16 m, emInt16 MSDlength, emInt16 total_length)
{
	emInt16 t, mt;

	LOG_StackAddr(__FUNCTION__);

	for(t=total_length-1, mt=MSDlength-1;t>=0;t--)
	{
		if(g_pRearRTCalc->m_MSDflag[t]==VOICED)
		{
			g_pRearRTCalcFilter->m_CmpFeature[t][m] = g_pRearRTCalcFilter->m_CmpFeature[mt][m];
			mt--;
		}
		else
			g_pRearRTCalcFilter->m_CmpFeature[t][m] = 0;
	}
	return mt+1;
}
/****************** ������������Ӻ������������ ********************/




/******************** ���ڻ�ȡ���ݵ��Ӻ����� ************************/
/* ���ݾ��߽����ȡLF0����                                          */
void GetLf0Param(emInt16 nLineCountOfPau)
{
	emInt16 n,s,result;      
	LOG_StackAddr(__FUNCTION__);
	for(n=0;n<nLineCountOfPau;n++)
		for(s=0;s<g_hTTS->pInterParam->nState;s++)
		{
			fRearSeek(g_hTTS->fResCurRearMain,g_pRearRTGlobal->m_Lf0LeafNode[n][s],0);
			result = (n*g_hTTS->pInterParam->nState+s)*7;
			
			if( g_hTTS->pRearG->nModelFloatLen == 2 )	
			{
				if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 )	//��Ƶ0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
				{
					fRearRead(&g_lf0param[result],sizeof(float),1,g_hTTS->fResCurRearMain);
					fread_float2(&g_lf0param[result+1],g_hTTS->pRearG->nModelFloatLen,PDF_NODE_FLOAT_COUNT_LF0-1,g_hTTS->fResCurRearMain);	
				}
				else
				{
					fread_float2(&g_lf0param[result],g_hTTS->pRearG->nModelFloatLen,PDF_NODE_FLOAT_COUNT_LF0,g_hTTS->fResCurRearMain);	
				}				
			}
			else
				fRearRead(&g_lf0param[result],g_hTTS->pRearG->nModelFloatLen,PDF_NODE_FLOAT_COUNT_LF0,g_hTTS->fResCurRearMain);							
		}	
}

// sqb  2017-4-25
/*���ݾ��߽����ȡbap*/     
void GetBapParam(emInt16 nLineCountOfPau)
{
	emInt16 n, s, result;
	LOG_StackAddr(__FUNCTION__);
	for (n = 0; n < nLineCountOfPau; n++)
		for (s = 0; s < g_hTTS->pInterParam->nState; s++)
		{
			fRearSeek(g_hTTS->fResCurRearMain, g_pRearRTGlobal->m_BapLeafNode[n][s], 0);
			result = (n*g_hTTS->pInterParam->nState + s) * 7;

			if (g_hTTS->pRearG->nModelFloatLen == 2)
			{
				if (g_hTTS->pRearG->nModelFloatLeafAdd == 2)	//��Ƶ0�׾�ֵ��4�ֽڶ������ఴ2�ֽڶ�
				{
					fRearRead(&g_bapparam[result], sizeof(float), 1, g_hTTS->fResCurRearMain);
					fread_float2(&g_bapparam[result + 1], g_hTTS->pRearG->nModelFloatLen, PDF_NODE_FLOAT_COUNT_BAP - 1, g_hTTS->fResCurRearMain);
				}
				else
				{
					fread_float2(&g_bapparam[result], g_hTTS->pRearG->nModelFloatLen, PDF_NODE_FLOAT_COUNT_BAP, g_hTTS->fResCurRearMain);
				}
			}
			else
				fRearRead(&g_bapparam[result], g_hTTS->pRearG->nModelFloatLen, PDF_NODE_FLOAT_COUNT_BAP, g_hTTS->fResCurRearMain);
		}
}
/*                                                                  */
/******************* ��ȡ���ݵ��Ӻ������������ *********************/


/* �ı����״̬�����������ں������ */
void ModifyStateGain( emInt16 nLineCountOfPau ) // naxy 1219
{
	emInt16 n, s;
	emInt16 result;
	float msd, gain;
	emInt16 temp,t;
	emInt8 Curpp,Prepp,Nextpp;
    
	LOG_StackAddr(__FUNCTION__);

   
if( !g_bRearSynChn )		//Ӣ�Ĵ���     sqb 2017/1/6
{

}   //end !g_bRearSynChn
else			//���Ĵ���
{
	for (n = 1; n < (nLineCountOfPau-1); n++)
	{

//���������   sqb   2017-5-26
#if HT_REAR_DEAL_PAU_ENERGY
		if (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_pau
			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_sil)
		{
			for (s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] /= 10;
		}

#endif
//****************************************************************************************//
//���β���Ҫ������8Kģ�ͳ������磺X��H����
//#if WL_REAR_DEAL_LETTER				//Ӣ����ĸ
//		if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_s		//��ĸ��	
//			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_f		//��ĸ��
//			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_h		//��ĸ��	
//			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_x )		//��ĸ��
//		{
//			g_pRearRTGlobal->m_MgcLeafNode[n][9][0] = -11;	//����10��״̬��������ֵΪ0
//		}
//#endif


#if WL_REAR_DEAL_DUR_XIEXIE
		// naxy, 2012-01-13, ie,ve
		if( (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ie ||		// ��ĸ��ie
			 g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ve )&&		// ��ĸ��ve
			 g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light)		// ����
		{
			g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 1.0;	
			//g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 1.0;	
		}
#endif


#if WL_REAR_DEAL_LIGHT_GAIN		//������������ı䲿������������������Ч����
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light)				// ����
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	
			
		}
#endif

#if WL_REAR_DEAL_LIGHT_GAIN_TWO  //����һЩ�������ٶ�� le5/me5/ne5/de5/men5
		//ne5 modified by wangcm 2012-03-19
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
			 g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_n )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.08;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//le5 modified by wangcm 2012-03-16
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����			
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
		    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_l)
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
			{
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_2)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.28;	//added by wangcm 2012-03-01 // 0.2->0.1
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_3)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.15;	//added by wangcm 2012-03-01 // 0.2->0.1
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_1)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.30;	//added by wangcm 2012-03-01 // 0.2->0.1
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_4)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.15;	//added by wangcm 2012-03-01 // 0.2->0.1
			}
			
		}
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����			
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
		    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_l)
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
			{
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_2)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.0;	//added by wangcm 2012-03-01 // 0.2->0.1
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_3)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	//added by wangcm 2012-03-01 // 0.2->0.1
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_1)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.3;	//added by wangcm 2012-03-01 // 0.2->0.1
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_4)
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.3;	//added by wangcm 2012-03-01 // 0.2->0.1
			}
			
		}
		//le5 end
		//de5 modified by wangcm 2012-03-16
		//if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
		//	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
		//	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
		//	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_d )
		//	                                                            //  added by wangcm 2012-03-01				
		//{
		//	for(s = 0; s < g_hTTS->pInterParam->nState; s++)
		//		g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.00; 	//added by wangcm 2012-03-01 // 0.2->0.1
		//	
		//}
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_4 &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_d )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.1; 	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//de5 end 

		//me5 added by wangcm 2012-03-16
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_m )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.40;	//added by wangcm 2012-03-01 // 0.2->0.1			
		}
		//end  wangcm 2012-03-16
		//ya5  added by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ia      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zero )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.17;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//end  wangcm 2012-03-20
		//men5  wangcm 2012-03-16
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_en      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_m )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.09;	//added by wangcm 2012-03-01 
			for(s = 5; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.02;	//added by wangcm 2012-03-20 
			for(s = 8; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.01;	//added by wangcm 2012-03-20
			
		}
		//men5 end  wangcm 2012-03-16
		//la5
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_a      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_l )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.1;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//la5 end
		//wa5
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ua      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zero )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//wa5 end
		//zi5 wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ�����Ǿ�β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&              // 4��
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ii      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_z )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.02;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//zi5 end wangcm 2012-03-17
		//ba5  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_a      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_b )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//ba5 end wangcm 2012-03-17
		//zhe5  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// ��ǰ��������β
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // ����
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zh )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.19;	//added by wangcm 2012-03-01 // 0.2->0.1
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_3)
			{
				for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.05;
			}
			//if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_2)
			//{
			//	for(s = 0; s < g_hTTS->pInterParam->nState; s++)
			//	g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.05;
			//}
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_4)
			{
				for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.03;
			}
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_TONE] == TONE_CN_4)
			{
				for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.02;
			}
		}
		//zhe5  wangcm 2012-03-17
#endif

#if WL_REAR_DEAL_END_GAIN  // for zi4/pian4 unlight 
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0   &&	
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_4       &&
			((g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ii     &&
			  g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_z) ||
			 (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ian   &&
			  g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_p )
			)
		  )
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.1;	//added by wangcm 2012-03-01 
		}

#endif

#if WL_REAR_DEAL_END_GAIN_TWO  // for pin3 unlight 
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0  &&	
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_3      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_in      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_p )
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.1;	//added by wangcm 2012-03-01 
		}

#endif

#if WL_REAR_DEAL_GONG4_GONG4	
		//���󣺻���gong4�������α���Ϊ������ʼ������̫���ұ仯��
		//�����������ʼ������ƽ����С
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_asprt_stop ||	//������	p,t,k
		   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_stop)			//����	b,d,g
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd = g_lf0param[result+6];
				if( msd<g_hTTS->pInterParam->msd_threshold )
					break;
			}
			for(s++; s < g_hTTS->pInterParam->nState; s++)
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd = g_lf0param[result+6];
				if( msd>g_hTTS->pInterParam->msd_threshold )
				{
					gain = exp(g_pRearRTGlobal->m_MgcLeafNode[n][s-1][0]+11) + exp(g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0]+11);
					gain /= 2;
					gain -= exp(g_pRearRTGlobal->m_MgcLeafNode[n][s][0]+11);
					if(gain<0)
					{
						g_pRearRTGlobal->m_MgcLeafNode[n][s][0] = 
						 log(exp(g_pRearRTGlobal->m_MgcLeafNode[n][s][0]+11) + gain)-11;
						break;
					}
				}
			}
		} 
		
		// qie2 ��ĸ��gong�� naxy120315
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_q &&	//q
		   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ie  && //ie
		   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_2)
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd = g_lf0param[result+6];
				if( msd<g_hTTS->pInterParam->msd_threshold )
					break;
			}
			for(s++; s < g_hTTS->pInterParam->nState; s++)
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd = g_lf0param[result+6];
				if( msd>g_hTTS->pInterParam->msd_threshold )
				{
					gain = exp(g_pRearRTGlobal->m_MgcLeafNode[n][s][0]+11) + exp(g_pRearRTGlobal->m_MgcLeafNode[n][s+2][0]+11);
					gain /= 2;
					gain -= exp(g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0]+11);
					if(gain<0)
					{
						g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0] = 
						 log(exp(g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0]+11) + gain)-11;
						break;
					}
				}
			}
		} 


		//���²�Ҫ
		//���󣺻���gong4�������α���Ϊ����ĸʱ��ƫ��
		//�����������ĸ��ʱ�����̣���ƽ������ĸ����ĸ�ε�����
		//if (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_nasal)		//����	m,n
		//{
		//	result = 0;
		//	for(s = 0; s < 3; s++)
		//	{
		//		if(g_pRearRTGlobal->m_PhDuration[n][s] > 3)
		//		{
		//			result += g_pRearRTGlobal->m_PhDuration[n][s] - 3;
		//			g_pRearRTGlobal->m_PhDuration[n][s] = 3;
		//		}
		//	}
		//	gain = exp(g_pRearRTGlobal->m_MgcLeafNode[n][s-1][0]+11) + exp(g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0]+11);
		//	gain /= 2;
		//	gain -= exp(g_pRearRTGlobal->m_MgcLeafNode[n][s][0]+11);
		//	if(gain<0)
		//	{
		//		g_pRearRTGlobal->m_MgcLeafNode[n][s][0] = 
		//				 log(exp(g_pRearRTGlobal->m_MgcLeafNode[n][s][0]+11) + gain)-11;
		//	}
		//	while(result>0)
		//	{
		//		for(s = 5; s < g_hTTS->pInterParam->nState; s++)
		//		{
		//			g_pRearRTGlobal->m_PhDuration[n][s] += 2;
		//			result -= 2;
		//			if(result<=0)
		//				break;
		//		}
		//	}
		//}

#endif


#if WL_REAR_DEAL_SHEN2_ME5

		//������������
		if( //g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_sh &&  // sh
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_en &&	// en
			//g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_TONE] == TONE_CN_light)					// ��һ����������
			(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_lateral ||		// ��һ�����Ǳ�����ĸ��l
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_nasal  ||		// ��һ�����Ǳ�����ĸ��m��n
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_zero) )			// ��һ����������ĸ
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd = g_lf0param[result+6];
				if( msd<g_hTTS->pInterParam->msd_threshold )
					break;
			}
			if(s == g_hTTS->pInterParam->nState)
				s = 6;
			for(s++; s < g_hTTS->pInterParam->nState; s++)
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd = g_lf0param[result+6];
				if( msd>g_hTTS->pInterParam->msd_threshold )
				{
					g_pRearRTGlobal->m_MgcLeafNode[n][s][0] = g_pRearRTGlobal->m_MgcLeafNode[n][s+2][0];
					g_pRearRTGlobal->m_MgcLeafNode[n][s+1][0] = g_pRearRTGlobal->m_MgcLeafNode[n][s+3][0];
					g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 1.0;
					g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 1.0;
					break;
				}
			}
		}
#endif

	}
}
}


//���ļ��ж�ȡ���2�ֽڵ�float��
emInt32  fread_float2(void * pDstBuf, emInt32 nElementSize,emInt32 nCount, FILE * pFile)
{

#if EM_READ_LEAF_FLOAT_ALL	

	//һ���Զ�������floatҶ�ڵ�
	emCharA pReadBuff[200];
	emInt16 i,i2,i3;

	LOG_StackAddr(__FUNCTION__);

	fRearRead(pReadBuff, nElementSize*nCount, 1, pFile);

	for(i=0; i<nCount; i++)
	{
		i2 = i * sizeof(float);
		i3 = i * 2;

		*((emCharA *)pDstBuf+i2+2)   = pReadBuff[i3];
		*((emCharA *)pDstBuf+i2+3)   = pReadBuff[i3+1];

		if( pReadBuff[i2] == 0 && pReadBuff[i2+1] == 0 )
		{
			*((emCharA *)pDstBuf+i2)   = 0;
			*((emCharA *)pDstBuf+i2+1) = 0;
		}
		else
		{
			*((emCharA *)pDstBuf+i2)   = 0x7f;
			*((emCharA *)pDstBuf+i2+1) = 0x7f;
		}
	}
	return (nElementSize*nCount);
	
#else

	//����float�Ķ�(��ζ���Դ)
	emInt16 i,tt;

	LOG_StackAddr(__FUNCTION__);

	emMemSet(pDstBuf,0x7f, nElementSize * nCount * 2);
	for(i=0; i<nCount; i++)
	{
		tt = i*sizeof(float);
		fRearRead((emCharA *)pDstBuf+tt+2, 2, 1, pFile);

		if( *((emCharA *)pDstBuf+tt+2) == 0 && *((emCharA *)pDstBuf+tt+3) == 0 )
		{
			*((emCharA *)pDstBuf+tt  ) = 0;
			*((emCharA *)pDstBuf+tt+1) = 0;
		}
	}
	return (nElementSize*nCount);

#endif

}

