#include <math.h>
//#include <mathcalls.h>					//sqb  2017/6/1
#include "emPCH.h"
#include "Rear_Common.h"
#include "emMath.h"
//#include "emVioceChange.h"



//声母类型
#define		INITIAL_OTHER				0		//其它声母
#define		INITIAL_ZERO_ZHUO			1		//零浊声母

//位置类型：（两字及以下句模板）
#define		POS_LEN2_PPH_HEAD			0		//两字PPH的韵首
#define		POS_LEN2_PPH_TAIL			1		//两字PPH的韵尾
#define		POS_LEN2_OTHER				2		//其它

//位置类型：（轻声模板）
#define		POS_NO_LIGHT_SEN_TAIL		0		//句尾							（只统计4字或以上的韵律短语）
#define		POS_NO_LIGHT_PPH_TAIL		1		//纯韵律短语尾	（不包含句尾）  （只统计4字或以上的韵律短语）
#define		POS_NO_LIGHT_OTHER			2		//其它

//位置类型：（非轻声模板） （不区分：韵尾和句尾）
#define		POS_LIGHT_PPH_TAIL			0		//韵尾： 轻声
#define		POS_LIGHT_NO_PPH_TAIL		1		//非韵尾：轻声



// 定义后端使用的全局变量
float					g_gv[2];									//用于保存基频GV参数
float					*g_Matrix;									//频谱方差矩阵数据
PRearRTGlobal			g_pRearRTGlobal;							//后端全局时库
PRearRTCalcFilter		g_pRearRTCalcFilter;						//后端解算滤波时库
PRearRTCalc				g_pRearRTCalc;								//后端解算时库
float syl_uvp[MAX_SYNTH_LAB_COUNT+2];								//上下文相关的清浊音转换点决策结果, changed by naxy19
emByte				*g_tree;										//用于临时保存一颗决策树的数组
emByte				*g_AllTree;										//保存22决策树
emInt32				g_AllTreeOffset[32];							//保存22决策树的偏移量数组(1颗:清浊转换点树；1颗:时长树；10颗:频谱树；10颗:基频树)
float *g_lf0param;													//用于保存基频参数的数组
float *g_bapparam;												//bap参数
PRearOffsetStr	g_pRearOffset;								//后端小资源偏移指针 fhy 121225
emBool			g_bRearSynChn;								//合成中文标志

//逆DCT函数
emInt32  BackDct( float *x, emInt32  m, float *c, emInt32  n )
{
	emInt32  i, j;

	LOG_StackAddr(__FUNCTION__);

	for(i=0;i<m;i++) 
	{
		x[i] = 0.0;
		for(j=1;j<n;j++) {
			//代替：x[i] += c[j] * cos(PI*j*(i+0.5)/m);  （不使用cos函数）
			//x[i] += c[j] * emFloat_ShiftDown( F_cos( (emUInt16) (PI*j*(i+0.5)/m)<<16 ) ,15 );  //不行，替换后韵尾发音不好
			x[i] += c[j] * cos(PI*j*(i+0.5)/m);
		}
		x[i] += c[0] * 0.5;

		x[i] = x[i]*(539.207247-52.1242348)+52.1242348 ;	//全局逆归一化

		x[i] = x[i]+g_hTTS->pRearG->f0_mean;  //2012-03-27， 对所有基频模板加上基频调整的控制  （否则基频控制对单双字等不起作用）
	}
	return 0;
}

#if HTRS_REAR_DEAL_CHANGE_MSD     //小沈修改

//判断：音素后处理类型
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


//后端预处理  设置虚拟发音人等
void RearPrevHandle()		
{
	float   fVirtualResampleScale = 1.0;
	float   fVirtualDurScale = 0;
	float   fVirtualF0Scale = 0;
	emByte	nFloatType;

	LOG_StackAddr(__FUNCTION__);

	//g_hTTS->m_ControlSwitch.m_nRoleIndex = emTTS_USE_ROLE_Virtual_54;
	//设置当前发音人的offset； 虚拟发音人还需设置：重采样系数，时长调节系数，基频调节系数
	switch( g_hTTS->m_ControlSwitch.m_nRoleIndex )
	{
		case emTTS_USE_ROLE_XIAOLIN:	//小林
			fVirtualResampleScale = 1.0;					//1.0
			fVirtualDurScale = DUR_SCALE_ROLE_Virtual_52;						//0
				fVirtualF0Scale = 0.0;
				g_hTTS->m_bResample				= emFalse;			//否
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

#if EM_OPEN_HTRS_BIANYIN					//开启：英文Htrs库变音朗读
	if( !g_bRearSynChn )		//若是：英文
	{
		if( g_hTTS->m_ControlSwitch.m_nRoleIndex == emTTS_USE_ROLE_Virtual_53)	//选择发音人是：田蓓蓓，则按Htrs英文的原音发音
		{
			fVirtualResampleScale = 1.0;
			fVirtualDurScale      = 0.0;
			fVirtualF0Scale       = 0.0;
			g_hTTS->m_bResample = emFalse;
		}
		else																	//选择其它发音人，则Htrs英文的音提亮变音之后发音
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
		//根据虚拟发音人的重采样系数：进行重采样的初始化
		ReSample_Init(g_objResample, g_hTTS->pRearG->nModelSample, (emUInt16 )(g_hTTS->pRearG->nModelSample * fVirtualResampleScale));				
	}
#endif




	//设置语速调节因子    sqb
	if( g_bRearSynChn)
		g_hTTS->pRearG->Speech_speed = (float)(g_hTTS->m_ControlSwitch.m_nVoiceSpeed-emTTS_SPEED_NORMAL)*0.04 + 1 - WL_REAR_DEAL_SPEED_FACTOR + fVirtualDurScale;
	//else
		//g_hTTS->pRearG->Speech_speed = (float)(g_hTTS->m_ControlSwitch.m_nVoiceSpeed-emTTS_SPEED_NORMAL)*0.04 + 1 - HTRS_REAR_DEAL_SPEED_FACTOR + fVirtualDurScale;

	if( g_bRearSynChn)	{
#if WL_REAR_DEAL_VIRTUAL_QST		//处理虚拟发音人的一些问题	

	//目前虚拟发音人[m55]等的最慢语速时本参数为0.47，合成会死机（估计内存不够），
	//暂时加一个这样的约束（若约束由0.60变成0.57也会死机）
	if( g_hTTS->pRearG->Speech_speed < 0.60)
	{
		g_hTTS->pRearG->Speech_speed = 0.60;	
	}

	if(    g_hTTS->m_ControlSwitch.m_nRoleIndex == emTTS_USE_ROLE_Virtual_54
		|| g_hTTS->m_ControlSwitch.m_nRoleIndex == emTTS_USE_ROLE_Virtual_55)
	{
		//防止这两个虚拟发音人语速最快时，芯片速度跟不上，声音发卡  hyl  2012-04-10
		//例如：[m55][s10]即一摩尔等于六十万两千艾个微观粒子。我不知道把它称作什么。[m3][d]
		if( g_hTTS->pRearG->Speech_speed > 0.84 )
			g_hTTS->pRearG->Speech_speed = 0.84;
	}
#endif

#if WL_REAR_DEAL_DCT_SHORT_SEN					//<王林>中文短句处理模式(字母的不处理)：专门处理：单字句，两字句
	if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] == 1 )		//为单字句
	{
		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//原始音库是《王林》的
		{
			//含英文字母的不处理
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] <= TONE_CN_sil_pau )	//本字不是字母	
			{
				//单字句模式: 调整单字句时长				
				switch( g_pLabRTGlobal->m_LabRam[1][RLAB_C_SM] )	//声母	
				{
					case INITIAL_CN_b:		g_hTTS->pRearG->Speech_speed *= 0.65;	break;		//单字声母是：b  	
					case INITIAL_CN_d:		g_hTTS->pRearG->Speech_speed *= 0.60;	break;		//单字声母是：d	
					default:	g_hTTS->pRearG->Speech_speed *= 0.85;	break;
				}	
				if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] == TONE_CN_light )	//声调为轻声
					g_hTTS->pRearG->Speech_speed *= 1.2;
				
			}
		}
	}
#endif
	}	// end of g_bRearSynChn

	
	//设置基频调节因子
	if((g_hTTS->m_ControlSwitch.m_nVoicePitch-emTTS_PITCH_NORMAL)>=0)
		g_hTTS->pRearG->f0_mean = 64 * ((float)(g_hTTS->m_ControlSwitch.m_nVoicePitch-emTTS_PITCH_NORMAL) / emTTS_PITCH_NORMAL );
	else
		g_hTTS->pRearG->f0_mean = 64  * ((float)(g_hTTS->m_ControlSwitch.m_nVoicePitch-emTTS_PITCH_NORMAL) / emTTS_PITCH_NORMAL );		

	g_hTTS->pRearG->f0_mean -=  100 * fVirtualF0Scale;

	if( g_bRearSynChn) {
#if WL_REAR_DEAL_VIRTUAL_QST		//处理虚拟发音人的一些问题	

	//目前虚拟发音人[m54]等的最低语调[t0]时本参数为-124，声音严重失真（基频过低），
	//暂时加一个这样的约束  （目前此系数变成100会出错）
	if( g_hTTS->pRearG->f0_mean < -90)			//例如：[m54][t0]并不等于否定见义勇为等传统美德[m3][d]
		g_hTTS->pRearG->f0_mean = -90;	
#endif
	}	// end of g_bRearSynChn


}

#if WL_REAR_DEAL_DCT_LIGHT			//王林音库后处理：DCT模板：中文韵律短语尾轻声（两字句或以上，不处理字母）
void ChangeLightDctF0(   emInt32  nCurStartTime,				//分段合成中: 本字开始的帧数
						   emInt32  nSylTotalT,				//分段合成中: 本字的总时长
						   emInt32  m1)	
{
	emInt16  n, t;
	emInt16 nSylStartT = 0, nSylEndT= 0;
	emInt16  nIndex, nUStart;
	emInt32  nToneMe,nTonePrev ,nToneNext,nEffectTone, nInitialType, nPosType ;
	float  lf0Float[150];
	emBool bModifyDct;


	////////////////////////  轻声基频DCT模板说明   //////////////////////////////////
	//
	//	行的索引 = 位置类型：*10  + 	声母类型*5  + 前调：

	//	位置类型：2种 ； 声母类型：2种 ； 前调：5种；

	//	位置类型：取值范围：    0： 韵尾句尾		1： 非韵尾句尾
	//	声母类型：取值范围：	0： 其它声母		1： 零浊声母	

	//	前调
	//		0:表示轻声调；	1:表示1调；	2:表示2调；	3:表示3调；	4:表示4调
	////////////////////////  轻声基频DCT模板说明   //////////////////////////////////
	

								
	float fTwoDctValue[20][7]={											//序号	本调	位置类型	声母类型	前调	音节个数
 
	//不区分：韵尾和句尾 ：目前后10个没用（非韵尾句尾）//modified by wangcm 2012-03-01
	{0.503730,0.040799,0.011756,0.00905,0.000793,0.004522,0.001254},			//1,轻声,0,0,0,66, //modified by wangcm 2012-03-16  原来：0.42373,0.050799,0.011756,0.00905,0.000793,0.004522,0.001254
	{0.533730,0.040799,0.011756,0.00905,0.000793,0.004522,0.001254},			//2,轻声,0,0,1,460, //hyl 2012-04-11
	{0.510243,0.048419,0.011539,0.005121,-0.001734,0.002402,-0.000913},			//3,轻声,0,0,2,394,
	{0.471737,0.000018,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //4,轻声,0,0,3,299,
	{0.475627,0.040475,0.014229,0.009790,0.000981,0.003331,0.000462},		    //5,轻声,0,0,4,606, //modified by wangcm 2012-03-17 原来：0.435627,0.032475,0.014229,0.00979,0.000981,0.003331,0.000462
	{0.485627,0.024799,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //6,轻声,0,1,0,48,
	{0.665696,0.084379,0.024148,-0.009103,-0.005961,0.002093,-0.000957},		//7,轻声,0,1,1,218,
	{0.556611,0.017921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//8,轻声,0,1,2,213, //modified by wangcm 2012-03-20
	{0.465627,0.023813,0.013717,0.006116,0.002598,0.001848,0.000532},	     	//9,轻声,0,1,3,153,
	{0.46267,0.037912,0.011972,0.000746,-0.001285,0.004285,-0.000675},			//10,轻声,0,1,4,241,
	{0.612275,0.025849,0.010634,0.005307,0.002356,0.001626,-0.000046},			//11,轻声,1,0,0,150,
	{0.758347,0.063361,0.020645,0.008887,0.002477,0.002559,0.000512},			//12,轻声,1,0,1,1074,
	{0.794895,0.024799,0.000293,0.002896,-0.000406,0.000886,-0.000628},			//13,轻声,1,0,2,927,
	{0.602648,-0.006668,0.002722,0.003425,0.001179,0.001597,0.000195},			//14,轻声,1,0,3,859,
	{0.597253,0.023813,0.013717,0.006116,0.002598,0.001848,0.000532},			//15,轻声,1,0,4,1603,
	{0.793275,0.034155,0.006766,-0.002332,-0.000913,0.003127,-0.000245},		//16,轻声,1,1,0,20,
	{0.897771,0.087555,-0.005318,-0.006805,-0.002271,0.002733,-0.001203},		//17,轻声,1,1,1,332,
	{0.822973,-0.012072,-0.030094,-0.006535,-0.005222,0.000692,-0.002446},		//18,轻声,1,1,2,304,
	{0.630488,-0.038981,0.000324,-0.000515,-0.002221,0.00174,-0.000446},		//19,轻声,1,1,3,258,
	{0.682047,0.050011,0.013739,-0.000625,0.00085,0.002978,0.000981}};			//20,轻声,1,1,4,377,


	//针对特殊音节（某些轻声韵尾），进行模板替换
	float fPPHTailSpecialDctValue[20][7]={										//				   前调

	{0.413730,0.055799,0.011756,0.00905,0.000793,0.004522,0.001254},			//0, 轻声, zhe5		0(轻声)
	{0.541498,0.078405,0.01005,0.010376,0.003475,0.005092,0.001742},			//1, 轻声, zhe5		1
	{0.450243,0.063419,0.011539,0.005121,-0.001734,0.002402,-0.000913},			//2, 轻声, zhe5		2
	{0.461737,0.019018,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //3, 轻声, zhe5		3
	{0.465627,0.049475,0.014229,0.009790,0.000981,0.003331,0.000462},		    //4, 轻声, zhe5		4

	{0.483627,0.019799,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //5, 轻声, men5		0(轻声)
	{0.663696,0.079379,0.024148,-0.009103,-0.005961,0.002093,-0.000957},		//6, 轻声, men5		1
	{0.554611,0.012921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//7, 轻声, men5		2
	{0.504611,0.012921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//8, 轻声, men5		3
	{0.46067, 0.032912,0.011972,0.000746,-0.001285,0.004285,-0.000675},			//9, 轻声, men5		4

	{0.465627,0.014799,0.000293,0.002896,-0.000406,0.000886,-0.000628},		    //10, 轻声, ma5		0(轻声)
	{0.645696,0.072379,0.024148,-0.009103,-0.005961,0.002093,-0.000957},		//11, 轻声, ma5		1
	{0.521611,0.007921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},		//12, 轻声, ma5		2
	{0.536611,0.007921,-0.008522,-0.00683,-0.005033,0.000443,-0.002879},     	//13, 轻声, ma5		3
	{0.44267, 0.027912,0.011972,0.000746,-0.001285,0.004285,-0.000675}};		//14, 轻声, ma5		4


	LOG_StackAddr(__FUNCTION__);

	nUStart = nCurStartTime;
	for( t = nCurStartTime; t< (nCurStartTime+nSylTotalT-5); t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == UNVOICED)
		{
			nUStart = t;				//需跨过的属于上一个音节的一小段浊音基频
			break;
		}
	}

		
	//找到本音节的浊音基频的起始帧数和结束帧数
	nSylStartT = nUStart;
	nSylEndT = (nCurStartTime+nSylTotalT -1);
	for( t = nUStart; t< nCurStartTime+nSylTotalT; t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED)
			nSylStartT = t;

		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t+1] == UNVOICED)
			nSylEndT = t;
	}	


	//计算寻找的DCT基频模板的索引
	emMemSet(lf0Float,0,sizeof(lf0Float));
	if(    g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 1	 					//两字句 或 以上
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_SM] != INITIAL_CN_letter				//前音节不为英文字母
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_SM] != INITIAL_CN_letter)				//后音节不为英文字母
	{
		//计算基频模板的索引
		nToneMe =   g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE];
		nTonePrev = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE];
		nToneNext = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_TONE];
		if( nToneMe == TONE_CN_light )		
			nToneMe = 0;		//轻声转换
		if( nTonePrev == TONE_CN_light )	
			nTonePrev = 0;		//轻声转换
		if( nToneNext == TONE_CN_light )	
			nToneNext = 0;		//轻声转换

		nInitialType = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM];
		if( nInitialType == 3 || nInitialType == 13 || nInitialType == 14 || nInitialType == 15 || nInitialType == 18 )
			nInitialType = INITIAL_ZERO_ZHUO;		//零浊声母： a,o,e,w,y开头的,13:l  14:m  15:n  18:r 
		else
			nInitialType = INITIAL_OTHER;			//其它声母

		//不区分：韵尾和句尾
		//赋值：位置类型
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0)		
			nPosType = POS_LIGHT_PPH_TAIL;		//韵律短语尾
		else
			nPosType = POS_LIGHT_NO_PPH_TAIL;		//非韵律短语尾

		nEffectTone = nTonePrev;

		//若影响声调为：sil声和pau声和虚拟字母声调
		if( nEffectTone == TONE_CN_sil_pau || nEffectTone == TONE_CN_letter)	//hyl  2012-03-29
		{
			nEffectTone = TONE_CN_1;  //强制为1声调
		}


		nIndex = nPosType*10  + nInitialType*5 + nEffectTone;

		//若是de5，zi5等音，且是韵尾（非句尾），则将基频下降趋势明显些  hyl 2012-04-17
		//例如：合肥前往北京的3679次航班9点30分就要起飞了。甚至连年轻的妻子也不让有半点的自由。
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
			nIndex += 0;					// fPPHTailSpecialDctValue数组中的	 绝对值	nIndex = 0 + 前声调	
		}

		//men5 modified by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM] == INITIAL_CN_m  &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] == FINAL_CN_en     &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)
		{
			bModifyDct = emTrue;
			nIndex += 0;					// fPPHTailSpecialDctValue数组中的	 绝对值	nIndex = 5 + 前声调				
		}

		//ma5 modified by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM] == INITIAL_CN_m  &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM] == FINAL_CN_a     &&
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light)
		{
			bModifyDct = emTrue;
			nIndex += 5;					// fPPHTailSpecialDctValue数组中的	 绝对值	nIndex = 10 + 前声调				
		}

		if( bModifyDct == emTrue )
		{
			//针对特殊音节（某些轻声韵尾），进行模板替换
			if( fPPHTailSpecialDctValue[nIndex][0] != 0.0)
				BackDct( lf0Float, nSylEndT-nSylStartT+1, &fPPHTailSpecialDctValue[nIndex][0], 7 );	//逆DCT		
		}
		else
		{
			//一般正常模板
			if( fTwoDctValue[nIndex][0] != 0.0)
				BackDct( lf0Float, nSylEndT-nSylStartT+1, &fTwoDctValue[nIndex][0], 7 );			//逆DCT		
		}
	}

	//开始将本音节的f0替换成DCT模板
	if( lf0Float[0] != 0.0 )
	{
		for( n = nSylStartT; n < nSylEndT; n++)
			g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length] = log(lf0Float[n-nSylStartT]);
	}
	return ;
}
#endif


#if WL_REAR_DEAL_DCT_PPH_TAIL			//王林音库后处理：中文韵律短语尾DCT模板（不含单双短句，不处理字母）
void ChangePphTailDctF0(   emInt32  nCurStartTime,			//分段合成中: 本字开始的帧数
						   emInt32  nSylTotalT,				//分段合成中: 本字的总时长
						   emInt32  m1)	
{
	emInt16  n, t;
	emInt16 nSylStartT = 0, nSylEndT= 0;
	emInt16  nIndex, nUStart;
	emInt32  nToneMe,nTonePrev ,nToneNext,nEffectTone, nInitialType, nPosType ;
	float  lf0Float[150],err_f0_DCT[150],err_f0_tmp;
							



#if WL_REAR_DEAL_MA0_DCT	|| 	WL_REAR_DEAL_AI3_DCT	//王林音库后处理：处理mao1韵尾DCT模板基频不好			mdj modified by mdj 2012-02-20
   emInt8 maolabChange = 0;
   emInt8 ai3labChange = 0;
#endif

   LOG_StackAddr(__FUNCTION__);

		nUStart = nCurStartTime;
	for( t = nCurStartTime; t< (nCurStartTime+nSylTotalT-5); t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == UNVOICED)
		{
			nUStart = t;				//需跨过的属于上一个音节的一小段浊音基频
			break;
		}
	}

		
	//找到本音节的浊音基频的起始帧数和结束帧数
	nSylStartT = nUStart;
	nSylEndT = (nCurStartTime+nSylTotalT -1);
	for( t = nUStart; t< nCurStartTime+nSylTotalT; t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED)
			nSylStartT = t;

		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t+1] == UNVOICED)
			nSylEndT = t;
	}	


	//计算寻找的DCT基频模板的索引
	emMemSet(lf0Float,0,sizeof(lf0Float));
	if(    g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)					//非单双句
		//hyl 2012-04-17 不能排除字母  例如：“解”读作xie4。Aisound4。
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_SM] != INITIAL_CN_letter			//前音节不为英文字母
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_SM] != INITIAL_CN_letter)			//后音节不为英文字母
	{

#if WL_REAR_DEAL_MA0_DCT				//王林音库后处理：处理mao1韵尾DCT模板基频不好			mdj
		if(g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM]==INITIAL_CN_m
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM]==FINAL_CN_ao
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE]==TONE_CN_1)					// 当前音节是mao3
//		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE]==TONE_CN_light				// 前音节为轻声 modified by mdj 2012-03-16
		{
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE]=TONE_CN_1;					// 前音节设为一声
			maolabChange = 1;
		}
#endif

#if WL_REAR_DEAL_AI3_DCT			//王林音库后处理：处理ai3韵尾基频模板的问题		added by mdj 2012-03-19

		if((g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero ||		// 当前音节是零声母
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal)		// 当前音节是鼻声母m，n modified by mdj 2012-02-20
		&&	g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM]==FINAL_CN_ai
		&&	g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE]==TONE_CN_3
		&&	g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE]==TONE_CN_2)
		{
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE] = TONE_CN_3;					// 前音节设为3声
			ai3labChange = 1;
		}


#endif

		//计算基频模板的索引
		nToneMe =   g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE];
		nTonePrev = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE];
		nToneNext = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_TONE];
		if( nToneMe == TONE_CN_light )		
			nToneMe = 0;		//轻声转换
		if( nTonePrev == TONE_CN_light )	
			nTonePrev = 0;		//轻声转换
		if( nToneNext == TONE_CN_light )	
			nToneNext = 0;		//轻声转换

		nInitialType = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM];
		if( nInitialType == 3 || nInitialType == 13 || nInitialType == 14 || nInitialType == 15 || nInitialType == 18 )
			nInitialType = INITIAL_ZERO_ZHUO;		//零浊声母： a,o,e,w,y开头的,13:l  14:m  15:n  18:r 
		else
			nInitialType = INITIAL_OTHER;		//其它声母

		//赋值：位置类型
		if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_PPH] == 0)		//韵律短语尾
		{
			if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_SYL_BWD_POS_IN_SEN] == 0)
				nPosType = POS_NO_LIGHT_SEN_TAIL;		//句尾							（只统计4字或以上的韵律短语）
			else
				nPosType = POS_NO_LIGHT_PPH_TAIL;		//纯韵律短语尾	（不包含句尾）  （只统计4字或以上的韵律短语）
		}
		else
			nPosType = POS_NO_LIGHT_OTHER;

#if WL_REAR_DEAL_OU_CHAN_YIN				//王林音库后处理：改变零声母ou颤音 例如“中东欧”				mdj

		if(g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero		// 当前音节是零声母
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_YM]==FINAL_CN_ou
		&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE]==TONE_CN_3)					// 当前音节ou3
			nPosType = POS_NO_LIGHT_PPH_TAIL;

#endif


		nEffectTone = nTonePrev;

		//若影响声调为：sil声和pau声和虚拟字母声调
		if( nEffectTone == TONE_CN_sil_pau || nEffectTone == TONE_CN_letter)	//hyl  2012-03-29
		{
			nEffectTone = TONE_CN_1;  //强制为1声调
		}


		nIndex = nToneMe*20 + nPosType*10  + nInitialType*5 + nEffectTone;
		
		if( fTwoDctValuePphTail[nIndex][0] != 0.0)
			BackDct( lf0Float, nSylEndT-nSylStartT+1, (float *)(&fTwoDctValuePphTail[nIndex][0]), 7 );	//逆DCT
	}


#if WL_REAR_DEAL_AI3_DCT			//王林音库后处理：处理ai3韵尾基频模板的问题恢复lab		added by mdj 2012-03-20
		if(ai3labChange == 1)
		{
			g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE] = TONE_CN_2;					// 前音节设为3声
			ai3labChange = 0;
		}

#endif
//开始将本音节的f0替换成DCT模板
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


#if WL_REAR_DEAL_DCT_SHORT_SEN			//<王林>中文短句处理模式(字母的不处理)：专门处理：单字句，两字句

//为替换基频模板
/*********************************/
/* 逆DCT，由DCT系数恢复基频数据序列  */
/* 参数列表：                    */
/*   float *x 输出基频数据序列数组   */
/*   emInt32  m    输出基频数组x的长度    */
/*   float *c 输入DCT数组        */
/*   emInt32  n    输入DCT数组c的长度 */
/*********************************/




//短句模式: 调整单字句和两字句能量和基频（用DCT模板）
//返回：本字的帧数
void ChangeShortSenZiGainF0(emInt32  nCurStartTime,			//分段合成中: 本字开始的帧数
						   emInt32  nSylTotalT,				//分段合成中: 本字的总时长
						   emInt32  m1)						//分段合成中: 本段的第几个字
{
	emInt16  n, t;
	emInt16 nSylStartT = 0, nSylEndT= 0;
	emInt16  nIndex, nUStart;
	float fGain;		//能量均值
	float fGainChange;	//能量改变系数
	emInt32  nToneMe,nTonePrev ,nToneNext,nEffectTone, nInitialType, nPosType ;
	float  lf0Float[150];



	////////////////////////  单字句：基频DCT模板说明  //////////////////////////////////
	//
	float fOneDctValue[5][7]={
		{1.032649,  -0.009009,  -0.000393,  -0.003199,  -0.004161,  -0.001517,  -0.002263},	//一声：dct模板
		{0.719289,  -0.064659,  0.029642,   0.001623,   -0.003843,  0.002884,   -0.002150},	//二声：dct模板
		{0.545271,  0.043375,   0.022054,   0.007160,   -0.004063,  0.002792,   -0.001307},	//三声：dct模板
		{0.808059,  0.121666,   -0.018365,  -0.006492,  -0.003713,  -0.000954,  -0.003166},	//四声：dct模板
		{0.561895,  0.099480,   0.026527,   0.006876,   0.001738,   0.004419,   0.000465}};	//五声：dct模板	
																					



#define		TWOZI_SEN_STD_GAIN			(0.008)		//基础能量参数

	  LOG_StackAddr(__FUNCTION__);

	  nUStart = nCurStartTime;
	for( t = nCurStartTime; t< (nCurStartTime+nSylTotalT-5); t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == UNVOICED)
		{
			nUStart = t;				//需跨过的属于上一个音节的一小段浊音基频
			break;
		}
	}

		
	//找到本音节的浊音基频的起始帧数和结束帧数
	nSylStartT = nUStart;
	nSylEndT = (nCurStartTime+nSylTotalT -1);
	for( t = nUStart; t< nCurStartTime+nSylTotalT; t++)
	{
		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t-1] == UNVOICED)
			nSylStartT = t;

		if( g_pRearRTCalc->m_MSDflag[t] == VOICED && g_pRearRTCalc->m_MSDflag[t+1] == UNVOICED)
			nSylEndT = t;
	}	



	//************************************暂关能量改变*******************************
	//  不加这一段：打鱼  音量不均衡
	//    加这一段：deng3 会爆音
	//*******************************************************************************
	////统计本音节的能量均值
	//fGain = 0.0;
	//nLen = (nSylEndT-nSylStartT+1)/4;
	//for( t = nSylStartT+nLen; t<= nSylEndT-nLen; t++)	//只统计本音节浊音段的中间的1/2的能量
	//	fGain += exp(g_pRearRTCalcFilter->m_CmpFeature[t][0]);

	//fGain /= (nSylEndT-nSylStartT-2*nLen+1);	

	////计算能量改变系数
	//fGainChange = pow(TWOZI_SEN_STD_GAIN / fGain, 0.4) ;

	//if ( fGainChange> 1.4 || fGainChange < 0.91 )	//只改变阀值以外的音节的音量
	//{
	//	if( g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE] == TONE_CN_light )		//轻声
	//	{
	//		fGainChange *= 0.90 ;
	//	}

	//	//改变能量
	//	for (t = nUStart; t<= nSylEndT; t++)
	//		g_pRearRTCalcFilter->m_CmpFeature[t][0] += log(fGainChange);
	//}	
	//************************************暂关能量改变*******************************





	//计算寻找的DCT基频模板的索引
	emMemSet(lf0Float,0,sizeof(lf0Float));
	if(    g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] == 2	)				//两字句
		//hyl 2012-04-17 不能排除字母  例如：G4—京港澳高速。G6-霍营高速。
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_SM] != INITIAL_CN_letter			//前音节不为英文字母
		//&& g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_SM] != INITIAL_CN_letter)			//后音节不为英文字母
	{
		//计算基频模板的索引
		nToneMe =   g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_TONE];
		nTonePrev = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_L_TONE];
		nToneNext = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_R_TONE];
		if( nToneMe == TONE_CN_light )		
			nToneMe = 0;		//轻声转换
		if( nTonePrev == TONE_CN_light )	
			nTonePrev = 0;		//轻声转换
		if( nToneNext == TONE_CN_light )	
			nToneNext = 0;		//轻声转换

		nInitialType = g_pLabRTGlobal->m_LabOneSeg[m1][RLAB_C_SM];
		if( nInitialType == 3 || nInitialType == 13 || nInitialType == 14 || nInitialType == 15 || nInitialType == 18 )
			nInitialType = INITIAL_ZERO_ZHUO;		//零浊声母： a,o,e,w,y开头的,13:l  14:m  15:n  18:r 
		else
			nInitialType = INITIAL_OTHER;		//其它声母

		//赋值：位置类型
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

		//若影响声调为：sil声和pau声和虚拟字母声调
		if( nEffectTone == TONE_CN_sil_pau || nEffectTone == TONE_CN_letter)			//hyl  2012-03-29
		{
			nEffectTone = TONE_CN_1;  //强制为1声调
		}


		nIndex = nToneMe*20 + nPosType*10  + nInitialType*5 + nEffectTone;
		
		if( fTwoDctValueShortSen[nIndex][0] != 0.0)
			BackDct( lf0Float, nSylEndT-nSylStartT+1,(float *)( &fTwoDctValueShortSen[nIndex][0]), 7 );	//逆DCT
	}
	else															//单字句
	{
		if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] == TONE_CN_light)		//第1个字是轻声
			nIndex = 4;							
		else
			nIndex = g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] - 1 ;


		if( fOneDctValue[nIndex][0] != 0.0)
			BackDct( lf0Float, nSylEndT-nSylStartT+1, &fOneDctValue[nIndex][0], 7 );	//逆DCT
	}

	//开始将本音节的f0替换成DCT模板
	if( lf0Float[0] != 0.0 )
	{
		for( n = nSylStartT; n < (nSylEndT+1); n++)
			g_pRearRTCalcFilter->m_CmpFeature[n][g_hTTS->pInterParam->static_length] = log(lf0Float[n-nSylStartT]);
	}


	return ;

}
#endif

// ********************  音节时长模型 start , changed by naxy19 **********************************
#if SYL_DURATION


// 带有音节时长约束的语速调整: 测试结果：句首句尾的音节时长会加长，助词的时长会变短
//个别太快：即可发指定内容的短信到10086申请参加活动。
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

	//w：表示权重：目前以5合适。 //
	//参考“咨询热线”中的“咨”：
	//w=1:rho=0.0029；w=3:rho=0.0068；w=5:rho=0.0094；w=30:rho=0.0174；w=100:rho=0.0198；w=1000:rho=0.0200；
	//w的值越少句子总时长越接近原来的，值越大，句子总时长越短。
	//例如：原来的总时长：15.15； w=0.1: 15.12 ；w=1 : 14.88；w=5 : 14.43；w=100 : 13.92
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

		temp1 /= 0.945;	//为了让句子总时长和原来的保持一致  对于w=5适用

		pOffsetValue[i] =(emInt16) (temp1 + 0.5); //取整数
		if (pOffsetValue[i]<1)
			pOffsetValue[i] = 1;
		remain = temp1 - pOffsetValue[i];
	}
	return remain;
}

#endif

// ********************  音节时长模型 end **********************************




//模型决策树的决策--读入整棵树
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
		//ch[0]的第4个bit代表需与lab比较的lab中的总列数,第8个bit代表需与lab中对应列比较的方式
		//ch[1]代表需与lab比较的lab中的起始列数
		outInt=ch[2] | ch[3] << 8;//需与lab中对应列比较的问题值

		if ( (ch[0] & (emInt8)0x10) == (emInt8)0x10 ) //需与lab比较的lab中的总列数为2
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1] << 8 | g_pLabRTGlobal->m_LabOneSeg[i][ch[1]];
		else //需与lab比较的lab中的总列数为1
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1];

		if ( (((ch[0] &1)==0) && (nCompareX==outInt)) //恰好等于且决策值为真  
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (ch[1] < (emInt8)42)) //小于等于且决策值为真
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (nCompareX>0) && (ch[1] > (emInt8)41)) )
		{
			nTempValue = ch[6] | ch[7] << 8;	// 回答为“是”
		} else
			nTempValue = ch[4] | ch[5] << 8;	// 回答为“否”

		if ( (nTempValue & 0x8000) == 0x8000 )
		{
			nTempValue &= 0x7FFF;//应转向的本决策树中pdf中对应的第几个叶节点			
			break;
		}
		else // 最高位为0：应转向的本决策树索引行数
			nNode = nTempValue;
	}
	return nTempValue;
}



//模型决策树的决策--仅读入需要行
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

		//ch[0]的第4个bit代表需与lab比较的lab中的总列数,第8个bit代表需与lab中对应列比较的方式
		//ch[1]代表需与lab比较的lab中的起始列数
		outInt=ch[2] | ch[3] << 8;//需与lab中对应列比较的问题值

		if ( (ch[0] & (emInt8)0x10) == (emInt8)0x10 ) //需与lab比较的lab中的总列数为2
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1] << 8 | g_pLabRTGlobal->m_LabOneSeg[i][ch[1]];
		else //需与lab比较的lab中的总列数为1
			nCompareX = g_pLabRTGlobal->m_LabOneSeg[i][ch[1]-1];		//nCompareX为前端lab中分析出的值  sqb  2016.11.2

	if(g_bRearSynChn)			//中文--20131225-怀疑此段有问题
	{
		if ( (((ch[0] &1)==0) && (nCompareX==outInt)) //恰好等于且决策值为真  
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (ch[1] < (emInt8)42)) //小于等于且决策值为真
		   ||(((ch[0] &1)==1) && (nCompareX<=outInt) && (nCompareX>0) && (ch[1] > (emInt8)41)) )
		{
			nTempValue = ch[6] | ch[7] << 8;	// 回答为“是”
		} else
			nTempValue = ch[4] | ch[5] << 8;	// 回答为“否”
	}
	else						//英文
	{
		if (   (  ( (ch[0] &1)==0 ) && nCompareX==outInt )		//恰好等于,且决策值为真  
		     ||(  ( (ch[0] &1)==1 ) && nCompareX<=outInt ) )	//小于等于,且决策值为真
		{
			nTempValue = ch[6] | ch[7] << 8;	// 回答为“是”
		} else
			nTempValue = ch[4] | ch[5] << 8;	// 回答为“否”
	}

		if ( (nTempValue & 0x8000) == 0x8000 )
		{
			nTempValue &= 0x7FFF;//应转向的本决策树中pdf中对应的第几个叶节点			
			break;
		}
		else // 最高位为0：应转向的本决策树索引行数
			nNode = nTempValue;
	}
	return nTempValue;
}




// ********************  下面全部是：后端决策解算的公共函数 （定点与非定点） *************************************


#if	WL_REAR_DEAL_R_INITIAL

//减少ri,re清音能量，并对ri,re清音相关的前后能量做平滑  naxy 1205
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
	
	//按字处理，首尾插入的pau除外
	for (n = 0; n < (nLineCountOfPau-1); n++) 
	{
		bIsSylRiReRuNeed = emFalse;
		bIsNextSylRiReRuNeed = emFalse;

		//本音节为ri,re，设置处理开关
		nTemp1 = g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM];		
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r 
			&& (nTemp1==FINAL_CN_iii || nTemp1==FINAL_CN_e  ) )			//本音节为ri,re
		{
			bIsSylRiReRuNeed = emTrue;
		}

		//本音节不是sil和pau，下一音节为ri,re，设置处理开关
		nTemp1 = g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM];		
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil)					//本音节不是sil和pau
			if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_SM] == INITIAL_CN_r 
				&& (nTemp1==FINAL_CN_iii || nTemp1==FINAL_CN_e  ))						//下一音节为ri,re
			{
				bIsNextSylRiReRuNeed = emTrue;
			}

		//按状态循环处理
		for(s = 0; s < g_hTTS->pInterParam->nState; s++)
		{
			//本音节为ri,re的处理
			if( bIsSylRiReRuNeed == emTrue )
			{
				//r声母的清音能量减少
				if(g_pRearRTCalc->m_MSDflag[dur] == UNVOICED)
				{
					for(t=0; t<g_pRearRTGlobal->m_PhDuration[n][s]; t++)
						g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (R_START_GAIN-1.5);	//r声母的清音能量减少
				}
				//r声母的浊音第1个状态能量平滑
				else
				{
					step = (R_START_GAIN-s+1.5)/(g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1]-1);
					for(t=0; t<g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1]; t++)		//r声母的浊音第1个状态能量平滑
						g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (R_START_GAIN-s+1.5-t*step);	
					break;
				}
			}
			
			//本音节不是sil和pau，下一音节为ri,re的处理
			if( bIsNextSylRiReRuNeed == emTrue )
			{
				//处理最后2个状态，能量平滑
				if( s == (g_hTTS->pInterParam->nState-2) )				
				{
					if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] != FINAL_CN_nil)
					{
						nTemp2 = g_pRearRTGlobal->m_PhDuration[n][s] + g_pRearRTGlobal->m_PhDuration[n][s+1];
						step = (R_START_GAIN-1)/nTemp2;

						//r声母的前1音节的浊音最后2个状态能量平滑
						for(t=0; t<nTemp2; t++)	
							g_pRearRTCalcFilter->m_CmpFeature[dur+t][0] -= (t+1)*step;
					}
				}
			}

			dur += g_pRearRTGlobal->m_PhDuration[n][s];
		}	

		// 如果break至此，2012-02-03	//处理连续2个r声母：  热热的。热热。日日。
		for(; s < g_hTTS->pInterParam->nState; s++)
		{
			// 处理连续r声母的情况
			if( bIsNextSylRiReRuNeed == emTrue )
			{
				//处理最后2个状态，能量平滑
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
			// 时长仍需累加
			dur += g_pRearRTGlobal->m_PhDuration[n][s];
		}
	}
}

#endif



#if WL_REAR_DEAL_VOL_TOO_LARGE   //对超大能量进行调整   2012-04-18

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
		if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0				//韵首或韵尾
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
				//韵首或韵尾：fou , hou
				if(    (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_f && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou )
					|| (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_h && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou ))
				{
					//例如：[v10]谣言的背后。一句中国式的问候。[d]
					//例如：[v10]否定。否则就不来。来否。是否[d]
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_75;	
				}
				//韵尾：song2,song3,sou2，sou3，zou2，zou3
				else if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0
					     && (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_2 ||  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3)
						 &&	(   (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_s && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ong )
						     || (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_s && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou  )
						     || (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou  )))
				{
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_75;	

				}
				//韵尾
				else if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0)				
				{
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_80;	
				}
				//剩余
				else																				
				{
					for(j=0;j<g_hTTS->pInterParam->nState;j++ )
						g_pRearRTGlobal->m_MgcLeafNode[i][j][0] += GAIN_FROM_100_TO_90;	
				}
			}					
		}
		else			//非韵首非韵尾
		{
			//fou，wang, zao3
			if(    (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_f && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou )
				|| (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uang)
				|| (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ao && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3))
			{
				//例如：[v10]能实现对各种警情实施网络信号区域内报警[d]
				//例如：[v10]速度不论快慢与否都仍处于安全轨道之内。您是否继续导航。[d]
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

/******************** 用于决策相关的子函数集 ************************/
/*                                                                  */
/* 决策主函数，用于获取时长、谱、基频的叶节点                       */
/* 时长将决策结果进行语速调整，取整数后直接存入g_pRearRTGlobal->m_PhDuration         */
/* 谱和基频的决策结果存入g_pRearRTGlobal->m_MgcLeafNode和g_pRearRTGlobal->m_Lf0LeafNode         */
/* 返回：时长规整的余值  ************************************************************************/
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
	emInt16 m1;			//用于统计韵尾非轻声音节总时长	

	// added by wangcm 2012-03-22
	emByte	g_LabZhe5[LAB_CN_LINE_LEN]={0x17, 0x09, 0x05, 0x01, 0x01, 0x06, 0x01, 0x01, 0x06, 0x01, 
		0x06, 0x02, 0x03, 0x07, 0x05, 0x01, 0x02, 0x07, 0x05, 0x01, 
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00};
	emByte	g_LabZhe5bak[LAB_CN_LINE_LEN];

	//音节时长决策结果
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

#if WL_REAR_DEAL_MA0_DCT				//王林音库后处理：处理mao1韵尾DCT模板基频不好			mdj
   emInt8 maolabChange = 0;
   emInt8 maoRtone = 0;					//added by mdj 2012-03-21
   emInt8 yanlabChange = 0;
#endif

#if WL_REAR_DEAL_IANG_LF0							//王林音库后处理：处理iang4基频拐的问题			added by mdj 2012-03-15
   emInt8 ianglabChange = 0;
#endif

   LOG_StackAddr(__FUNCTION__);

	if(g_bRearSynChn)
	{
#if SYL_DURATION   /* 音节时长模型开始, changed by naxy19 */
	Next_TREE_OFFSET = g_Res.offset_RearVoiceLib_Cur + g_hTTS->pRearG->offset.sydMod;
	fRearSeek(g_hTTS->fResCurRearMain, Next_TREE_OFFSET ,0);
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);		// 音节时长树的叶节点个数
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

#if EM_DECISION_READ_ALL	

#if DEBUG_LOG_SWITCH_HEAP
	g_tree = (emCharA *)emHeap_AllocZero(nAllocSize , "音节时长模型树叶节点：《后端模块  》");
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
				PDF_NODE_FLOAT_COUNT_SYD * g_hTTS->pRearG->nModelFloatLen * (nLeafNo-1); //数据表格中的偏移量
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
	emHeap_Free(g_tree , nAllocSize , "音节时长模型树叶节点：《后端模块  》");
#else
	emHeap_Free(g_tree , nAllocSize );
#endif
#endif



#endif            /* 音节时长模型结束 */


	/*************************************************************************************************/
	/******************************* 清浊音转换点模型开始 ********************************************/
	/*************************************************************************************************/

	/*
	Next_TREE_OFFSET = g_pRearOffset->uvpMod;
	fRearSeek(g_hTTS->fResCurRearMain,Next_TREE_OFFSET ,0);	
	// 读取清浊音转换点决策树的叶节点个数
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain);
	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

	if( g_bIsUserAddHeap == 0 )
	{	
#if EM_DECISION_READ_ALL	
			// 分配内存给临时决策树数组 
	
#if DEBUG_LOG_SWITCH_HEAP
			g_tree = (emByte *)emHeap_AllocZero(nAllocSize , "清浊转换点模型的叶节点：《后端模块  》");
#else
			g_tree = (emByte *)emHeap_AllocZero(nAllocSize );
#endif
			// 读取清浊音转换点决策树
			fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load syllable uvp Tree
#endif  
	}
	else
	{
		g_tree = g_AllTree + g_AllTreeOffset[0];

	}

	


	// 决策每个字的转换点
	for( i = 1;i<nLineCountOfPau-1;i++ ) 
	{
#if EM_DECISION_READ_ALL
		nLeafNo = RearDecision(i);
#else
		nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
		loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				PDF_NODE_FLOAT_COUNT_UVP * g_hTTS->pRearG->nModelFloatLen * (nLeafNo-1); //数据表格中的偏移量
		fRearSeek(g_hTTS->fResCurRearMain, loffsetOfRes, 0);
		if( g_hTTS->pRearG->nModelFloatLen == 2 )		
			fread_float2(&syl_uvp[i], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);	
		else
			fRearRead(&syl_uvp[i], g_hTTS->pRearG->nModelFloatLen, 1, g_hTTS->fResCurRearMain);
	}

	if( g_bIsUserAddHeap == 0 )
	{	

#if EM_DECISION_READ_ALL	
		// 释放临时决策树数组的内存 
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_tree , nAllocSize , "清浊转换点模型的叶节点：《后端模块  》");
#else
		emHeap_Free(g_tree , nAllocSize );
#endif
#endif  
	}

	*/								//sqb    2016.12.2
	/*************************************************************************************************/
	/******************************* 清浊音转换点模型结束 ********************************************/
	/*************************************************************************************************/

	}

	if(g_bRearSynChn)	{
#if WL_REAR_DEAL_LIGHT_MGC				//王林音库后处理：改变部分轻声的频谱（改善效果）

	for( i =0;i<nLineCountOfPau;i++ )
	{
		//将满足条件的轻声改成4声，待频谱决策完后，再改回来
		lightToneChange[i] = 0;
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light	)	// 当前音节是轻声
			//g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] ==0 &&				// 当前音节是韵尾
		{
			if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_o	//韵母：o			婆婆
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ie	//韵母：ie			谢谢
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ve	//韵母：ve			[i1]xue2xue5
				|| g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ua	//韵母：ua			[i1]gua4gua5 [i1]kua4kua5
				)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_4;					//声调改成4声
				lightToneChange[i] = 1;
			}
		}
	}

#endif
	}	
	

	// 读取状态时长决策树的叶节点个数
	fRearSeek(g_hTTS->fResCurRearMain,g_pRearOffset->durMod,0); //changed by naxy17
	fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain); // 时长树的叶节点个数


	nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

	if( g_bIsUserAddHeap == 0 )
	{
#if EM_DECISION_READ_ALL
		/* 分配内存给临时决策树数组 */
#if DEBUG_LOG_SWITCH_HEAP
		g_tree = (emByte *)emHeap_AllocZero(nAllocSize , "时长第1颗树叶节点：《后端模块  》");
#else
		g_tree = (emByte *)emHeap_AllocZero(nAllocSize );
#endif	
		// 读取状态时长决策树
		fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load Duration Tree
#endif
	}
	else
	{
		g_tree = g_AllTree + g_AllTreeOffset[1];
	}

	Next_TREE_OFFSET = g_pRearOffset->durMod; //changed by naxy17

	
	//每次分段合成：前后加的pau的每个状态的帧数
	//每个状态为1帧和2帧的wav是一样的，但每个状态为4帧的wav就不一样了，
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
#if WL_REAR_DEAL_RU3_LF0			//王林音库后处理：ru3的基频变调 modified by mdj

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_r 					// 当前音节是声母，r
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_u					// 当前音节是韵母，u
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3					// 当前音节是ru3
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW] != 0				// 当前不是PPH尾
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] != 0)			// 当前不是PPH头
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW] = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PW] = 1;
			}
#endif
		}	// end of g_bRearSynChn

		//时长决策：
#if EM_DECISION_READ_ALL
		nLeafNo = RearDecision(i);
#else
		nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
		loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) 
			+ 2 * g_hTTS->pInterParam->nState * g_hTTS->pRearG->nModelFloatLen *(nLeafNo-1); //数据表格中的偏移量
		


		if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DCT_SHORT_SEN					//<王林>中文短句处理模式(字母的不处理)：专门处理：单字句，两字句


		//两字句的时长暂不处理--不删
		//if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//原始音库是《王林》的
		//{
		//	//含英文字母的不处理
		//	if( g_pLabRTGlobal->m_LabRam[1][RLAB_C_TONE] <= TONE_CN_sil_pau && g_pLabRTGlobal->m_LabRam[2][RLAB_C_TONE] <= TONE_CN_sil_pau)	//第1字和第2字都不是字母	
		//	{

		//		//两字句模式: 调整两字句时长
		//		if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN]==2  && nStartLineOfPau >= 2)		//为两字句，且处理到了第2个字 （注意：两字句的第一个字不做处理）
		//		{
		//			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light )	//声调为轻声
		//				g_hTTS->pRearG->Speech_speed *= 1.2;
		//		}

		//	}
		//}

#endif

#if WL_REAR_DEAL_LIGHT_TONE			//非短句中轻声处理模式：
		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 )	//原始音库是《王林》的
		{
			//非短句中: 轻声处理模式：  句长>2
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2 && g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light)	
				g_hTTS->pRearG->Speech_speed *= 1.1;
		}
#endif
		}	// end of g_bRearSynChn

		//状态时长取整调节
#if SYL_DURATION
		if(g_bRearSynChn)
		{
		//若为sil和pau音节，则不根据音节时长模型调整
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
				g_pRearRTGlobal->m_PhDuration[i][j] = g_pRearRTGlobal->m_PhDuration[i][j]*7/5;  //增大1.4倍
			}
#endif

#if WL_REAR_DEAL_LETTER	

		//改变时长		
		if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+i][RLAB_C_YM]  == FINAL_CN_letter_c 		//后1字母是C的话
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

		//将全部字母的时长缩短一些			hyl  2012-04-19  防卡
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_letter )	 //若是字母
		{
			for(j=0;j<g_hTTS->pInterParam->nState;j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] > 12 )
					g_pRearRTGlobal->m_PhDuration[i][j] = g_pRearRTGlobal->m_PhDuration[i][j]*11/12;  //缩短1/12				
			}
		}

#endif

#if WL_REAR_DEAL_EN_ENG_QST
		//后端en和eng的特殊处理：(所有weng，另en2和eng2后跟3声的零浊声母)  + 加上iang
		//例子：入瓮，消费者入瓮。老翁，这个老翁很喜欢抽烟。。
		//例子：她很美丽。他很美。他很矮。他很[=hen2]妹。他很[=hen2]没。
		//例子：[i1]sen2mei3。[i1]wen2mei3。[i1]deng2mei3。[i1]feng2mei3
		//例子：本以为从网上购买飞机票会得到较大的优惠，

		bTemp = emFalse;
		if(   g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero && 
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ueng     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0) //本音节：weng无调音节
		{
			bTemp = emTrue;
		}
		if(   g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l && 
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_iang     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0) //本音节：weng无调音节
		{
			bTemp = emTrue;
		}

		//若满足，则将第5第6状态的时长缩为1帧，减少的帧挪到第4第7状态
		if( bTemp == emTrue )	
		{
			g_pRearRTGlobal->m_PhDuration[i][4] += g_pRearRTGlobal->m_PhDuration[i][5] - 1;
			g_pRearRTGlobal->m_PhDuration[i][5]  =                                       1;
			g_pRearRTGlobal->m_PhDuration[i][7] += g_pRearRTGlobal->m_PhDuration[i][6] - 1;
			g_pRearRTGlobal->m_PhDuration[i][6]  =                                       1;
		}

		//bTemp = emFalse;
 	//	if(    (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_en)								//本音节：en
		//	&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_2 							//本音节：二声
		//	&& (   g_pLabRTGlobal->m_LabRam[i+nStartLineOfPau][RLAB_C_SM] == INITIAL_CN_m) 			//下音节：m声母
		//	&& g_pLabRTGlobal->m_LabRam[i+nStartLineOfPau][RLAB_C_TONE] == TONE_CN_3 )				//下音节：三声
		//{
		//	bTemp = emTrue;
		//}

		////若满足，改变帧长
		//if( bTemp == emTrue )	
		//{
		//	g_pRearRTGlobal->m_PhDuration[i][7]  =  1;
		//	g_pRearRTGlobal->m_PhDuration[i][8]  =  1;
		//	g_pRearRTGlobal->m_PhDuration[i][9]  =  3;
		//}

#endif


#if WL_REAR_DEAL_DUR_XIEXIE
		// naxy, 2012-01-13, ie,ve
		if( (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ie ||		// 韵母：ie
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ve)&&		// 韵母：ve
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light)		// 轻声
		{
			g_pRearRTGlobal->m_PhDuration[i][7] = 1;
		}
#endif

#if	WL_REAR_DEAL_DUR_ZENME
		// naxy, 2012-01-18, zen me
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z &&		// 当前音节声母：z
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en &&			// 当前音节韵母：en
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE] == TONE_CN_light)		// 后一音节轻声
		{
			g_pRearRTGlobal->m_PhDuration[i][6] = 1;
			g_pRearRTGlobal->m_PhDuration[i][7] = 1;
			g_pRearRTGlobal->m_PhDuration[i][8] = 1;
			g_pRearRTGlobal->m_PhDuration[i][9] = 1;
		}
#endif

#if WL_REAR_DEAL_R_INITIAL
		if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_r &&		// 声母：r
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_u)			// 韵母：u
		{
			g_pRearRTGlobal->m_PhDuration[i][0] += 2;
			g_pRearRTGlobal->m_PhDuration[i][1] = 1;
			g_pRearRTGlobal->m_PhDuration[i][2] = 1;
			g_pRearRTGlobal->m_PhDuration[i][3] += 2;
		}

		if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_r &&		// 声母：r
			 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_iii )		// 韵母：i
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
#if WL_REAR_DEAL_DCT_SEN_TAIL_LEN					//改变句尾非轻声的音节时长

		#define  SYL_MIN_FRAME_FOR_TAIL_WL			50	//正常语速下若音节帧数小于此数，则需改变（即g_hTTS->pRearG->Speech_speed=0.9）
		#define  SYL_CHANGE_MIN_FRAME_FOR_SEN_WL	52	//正常语速下改变后的SEN音节最小帧数（即g_hTTS->pRearG->Speech_speed=0.9）
		#define  SYL_CHANGE_MIN_FRAME_FOR_PPH_WL	50	//正常语速下改变后的PPH音节最小帧数（即g_hTTS->pRearG->Speech_speed=0.9）

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//原始音库是《王林》的
			&& g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD )		//不是：一字一顿效果
		{
			//中文韵律短语尾DCT模板			
			if( g_pLabRTGlobal->m_LabRam[1][RLAB_SYL_NUM_IN_C_SEN] > 2)			//句长>2
			{
				if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0	&&		//句尾：SEN中syl的反序位置=0
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_letter  &&		//不为英文字母
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_light)			//不为轻声
				{	
					if(nCurSylTotalFrame < (SYL_MIN_FRAME_FOR_TAIL_WL/g_hTTS->pRearG->Speech_speed))
					{

						n1 = (SYL_CHANGE_MIN_FRAME_FOR_SEN_WL/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;

						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
//						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i)
						{
							n1=0;
						}
						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d		//不处理di4的时长 modified by mdj 2012-03-17
						&& (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i || 
							g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_u	)	//不处理du4的时长 modified by mdj 2012-03-26
						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_4)
							n1=0;

						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zero		//不处理ai4的时长 modified by mdj 2012-03-22
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
				else if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0	&&	//韵律短语尾：PPH中syl的反序位置=0
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_letter  &&		//不为英文字母
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] != TONE_CN_light)			//不为轻声
				{	
					if(nCurSylTotalFrame < (SYL_MIN_FRAME_FOR_TAIL_WL/g_hTTS->pRearG->Speech_speed) )
					{
						n1 = (SYL_CHANGE_MIN_FRAME_FOR_PPH_WL/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;
						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
//						&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i)
						{
							n1=0;
						}
						if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d		//不处理di4的时长 modified by mdj 2012-03-17
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

#if WL_REAR_DEAL_SYL_TOO_FAST				//王林：改变个别音节过快的情况  

		//例如：有关制裁更显得莫名其妙。在世界各地都有发现。总部领导成功访俄。
		//必须与语速关联起来
		#define  SYL_MIN_FRAME_FOR_WL			30	//正常语速下若音节帧数小于此数，则需改变（即g_hTTS->pRearG->Speech_speed=0.9）
		#define  SYL_CHANGE_MIN_FRAME_FOR_WL	33	//正常语速下改变后的音节最小帧数（即g_hTTS->pRearG->Speech_speed=0.9）
		#define  SYL_LIGHT_NEED_CUT_FOR_WL		5	//轻声比正常声调需减少的阀值	

		//处理：1至4声
		if( nCurSylTotalFrame<=(SYL_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed)  
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]<=TONE_CN_4)  
		{
			n1 = (SYL_CHANGE_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;
			if( n1>g_hTTS->pInterParam->nState)
			{
				n1 = g_hTTS->pInterParam->nState;			//最多增加10帧				
			}
			nCurSylTotalFrame += n1;
			n1 = g_hTTS->pInterParam->nState - n1;			//从第几个状态开始每个状态增加1帧
			for(j=n1 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j]++;			
		}

		//处理：轻声
		if( nCurSylTotalFrame<=((SYL_MIN_FRAME_FOR_WL-SYL_LIGHT_NEED_CUT_FOR_WL)/g_hTTS->pRearG->Speech_speed)  
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light)  
		{
			n1 = ((SYL_CHANGE_MIN_FRAME_FOR_WL-5)/g_hTTS->pRearG->Speech_speed) - nCurSylTotalFrame;
			if( n1>g_hTTS->pInterParam->nState)
			{
				n1 = g_hTTS->pInterParam->nState;			//最多增加10帧				
			}
			nCurSylTotalFrame += n1;
			n1 = g_hTTS->pInterParam->nState - n1;			//从第几个状态开始每个状态增加1帧
			for(j=n1 ; j<g_hTTS->pInterParam->nState ; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j]++;			
		}

#endif

#if WL_REAR_DEAL_DCT_SHORT_SEN
		// hyl  2012-04-17   解决单字bu发音不好的问题
		if(    g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_b		//声母：b
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_u		//韵母：u
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_NUM_IN_C_SEN] == 1 )	//单字句
		{
			
			g_pRearRTGlobal->m_PhDuration[i][0] = 2;
			g_pRearRTGlobal->m_PhDuration[i][1] = 3;
		}
#endif


#if WL_REAR_DEAL_SYL_TOO_FAST_NEW
		//for de2在韵尾的情况  wangcm 2012-03-17
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_2 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d)
		{
			//从第1个状态开始每个状态增加1帧
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
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE] != TONE_CN_light)	//且后字音调不是轻声 hyl 2012-04-23 例如：解放涛的妹妹来了
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

		//for mei3wei4在韵尾的情况  wangcm 2012-03-17
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

		//for gui3zi5 在韵尾的情况

		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3  &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uei &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_g)
		{
			//从第1个状态开始每个状态增加1帧
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

		//for zi5 在韵尾的情况
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ii      &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z)
		{
			//从第1个状态开始每个状态增加1帧
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
			//从第1个状态开始每个状态增加1帧
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_4  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_iii &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_r)
			{
				//从第1个状态开始每个状态增加1帧
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
			//从第1个状态开始每个状态增加1帧
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_3  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_uo &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_zero)
			{
				//从第1个状态开始每个状态增加1帧
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
			//从第1个状态开始每个状态增加1帧
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_TONE]==TONE_CN_light    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] != 1 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM] == FINAL_CN_en     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_m)
			{
				//从第1个状态开始每个状态增加1帧
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

		//for me5 韵尾的时候  wangcm 2012-03-16
		if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e     &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m)
		{
			//从第1个状态开始每个状态增加1帧
			for(j=4 ; j<g_hTTS->pInterParam->nState ; j++ )
			{
				if( g_pRearRTGlobal->m_PhDuration[i][j] >= 2)
					g_pRearRTGlobal->m_PhDuration[i][j] -= 1;
			}
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//end me5 wangcm 2012-03-16
		//added by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
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
			//从第1个状态开始每个状态增加1帧
			if(g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_TONE]==TONE_CN_2  &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_YM] == FINAL_CN_ai &&
				g_pLabRTGlobal->m_LabOneSeg[i-1][RLAB_C_SM] == INITIAL_CN_l)
			{
				//从第1个状态开始每个状态增加1帧
				for(j=2 ; j<g_hTTS->pInterParam->nState ; j++ )
					g_pRearRTGlobal->m_PhDuration[i-1][j] += 1;
				durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
			}
		}
		//modified by wangcm 2012-03-16
		//la5 谱颤音，把第4、5个状态置1  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_light  &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_a    &&
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l)
		{
			for(j=4 ; j<6; j++ )
				g_pRearRTGlobal->m_PhDuration[i][j] = 1;
			durChange = 1;						//added by mdj 2012-03-26 used for duration change flag
		}
		//la5  wangcm 2012-03-17
		//di4di5 后面的轻声di5时长长
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
		//di4di5 后面的轻声di5时长长
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
		//shen2me5 shen2快 me5谱颤音，把第4、5个状态置1  wangcm 2012-03-20
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
		//m/n 轻声浊声母的问题如：妈妈囔囔wangcm 2012-04-01
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


		 // 防止出现状态时长等于的情况naxy120319
		for(j=0 ; j<g_hTTS->pInterParam->nState ; j++ )
			if(g_pRearRTGlobal->m_PhDuration[i][j] < 1)
				g_pRearRTGlobal->m_PhDuration[i][j] = 1;
	}


		
	/*
	//若超过最大时长，则整体缩小
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
		/* 释放临时决策树数组的内存 */
#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(g_tree , nAllocSize , "时长第1颗树叶节点：《后端模块  》");
#else
		emHeap_Free(g_tree , nAllocSize );
#endif
#endif
	}

	//指向资源中的频谱模型
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
	
	//为加快决策速度，按状态循环决策
	for(j=0;j<g_hTTS->pInterParam->nState;j++ )
	{
		//读取频谱第j颗树的叶节点个数，分配内存
		fRearSeek(g_hTTS->fResCurRearMain,Next_TREE_OFFSET ,0); //changed by naxy17
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain); // 频谱第j颗树的叶节点个数

		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

		if( g_bIsUserAddHeap == 0 )
		{

#if EM_DECISION_READ_ALL	
#if DEBUG_LOG_SWITCH_HEAP
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize  , "频谱第n颗树叶节点：《后端模块  》");
#else
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize );
#endif		
			//读取频谱第j颗树
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
#if WL_REAR_DEAL_CHAN_YIN		//王林音库后处理：改变部分颤音  ze4，韵母en eng an ang ong后跟零声母

			//改善ze4合成不好问题   //仄。平平仄仄。平仄。仄仄。
			//if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_z &&	// 声母z
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// 韵母e
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_4  &&	// 四声
			//	j>4)
			//{
			//	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_asprt_affricate;     // 改变声母发音方式标志位
			//	zelabChange = 1;
			//}


			//改善韵母en eng an ang ong后跟零声母不好的问题
			//例子： 恩恩爱爱。恩恩。80后的金恩恩面对的是一些80后的政治老人。
			//例子： 恩恩爱爱。恩[=eng1]恩爱爱。恩[=an1]恩爱爱。恩[=ang1]恩爱爱。恩[=kong1]恩爱爱。
			if(	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM_REAR] == YM_CN_REAR_nasal &&		// 韵尾发音方式：
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] != FINAL_CN_ian &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] != FINAL_CN_an &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] != FINAL_CN_uan &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero		)		// 后音节是零声母
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_s;     // 改变后一音节声母为非零声母
				enlabChange = 1;
			}
			if(  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM_REAR] == YM_CN_REAR_nasal   &&		// 韵尾发音方式：
				(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_m      ||
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_n      ||
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_l      ||
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_r))				    // 后音节是浊声母
			{
				holdValueOne = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM];
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_t;                       // 改变后一音节声母为非浊声母

				holdValueTwo = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD];
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD] = SM_CN_METHOD_asprt_stop;     // 改变后一音节声母为非浊声母

				holdValueThree = g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE];
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] = SM_CN_PLACE_dentalveolar;

				zhuolabChange = 1;
			}
			//解决uo3后面是零声母的情况---wangcm-2012-03-17
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_uo  &&		// 韵母uo
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3 &&		// 三声
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero)	//零声母
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_s;     // 改变声母发音方式标志位
				wolabChange = 1;
			}
			//end  ---wangcm-2012-03-17
			
			//fou3韵尾的时候
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_f &&	// 声母z
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ou  &&		// 韵母e
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3  &&
				j>2)	//零声母
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] = INITIAL_CN_s;     // 改变声母发音方式标志位
				foulabChange = 1;
			}


			//ze5/zhe5/ce5/che5/se5/she5/he5 谱错误的情况  ---wangcm-2012-03-16
			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_fricative       ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_affricate       ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_asprt_affricate)&&   // 
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// 韵母e
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light &&
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0)	
			{

				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// 韵母e			hyl  2012-04-07
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zh)		// 声母zh
				{
					emMemCpy(g_LabZhe5bak, g_pLabRTGlobal->m_LabOneSeg[i], LAB_CN_LINE_LEN);
					emMemCpy(g_pLabRTGlobal->m_LabOneSeg[i], g_LabZhe5, LAB_CN_LINE_LEN);	
				}
				else
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_4;     // 改变音调				

				ce5labChange = 1;
			}
			//end----ze5/zhe5/ce5/che5/se5/she5/he5 谱错误的情况  ---wangcm-2012-03-16


			//sen1,cen1非韵尾的时候						added by mdj  2012-03-17
			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_s ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_c)				// 声母s、z
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_en  
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_1				// 韵母en
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] == SM_CN_PLACE_labial	// 后音节声母为b、p
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] != INITIAL_CN_m)
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_PLACE] = SM_CN_PLACE_labiodental;
				senlabChange = 1;
			}

			//l/n/m/rao3后面跟零声母的时候				added by mdj	2012-03-17
			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_lateral ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] == SM_CN_METHOD_nasal	 ||
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_r)				// 声母m,n,l,r
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_ao  
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_3				// 韵母ao3
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD] == SM_CN_METHOD_zero)	// 后音节为零声母
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_g;
				laolabChange = 1;
			}
			
			//de5后面跟零声母的时候						added by mdj	2012-03-26
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_d
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  		// 
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light 		// de5
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] == INITIAL_CN_zero
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_YM_FRONT] == YM_CN_FRONT_open)	//后音节为开零声母
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = INITIAL_CN_t;
				delabchange = 1;
			}


#endif


#if WL_REAR_DEAL_LIGHT_M_N    //-wangcm-20120401

			if((g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_m
			||	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_n) 
			&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] == 0
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light)	//后音节为开零声母
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_4;
				mnlabchange = 1;
			} 


#endif						//-wangcm-20120401

#if WL_REAR_DEAL_LIANG_LF0

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_l					// 当前音节是边音声母，l
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_iang					// 当前音节是韵母，iang
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_3					// 当前音节是liang3
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE] != TONE_CN_4							// 前一音节不是四声
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] != 0)			// 当前不是PPH头
			{
				 g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE] = TONE_CN_4;
			}
#endif

#if WL_REAR_DEAL_OU_CHAN_YIN				//王林音库后处理：改变零声母ou颤音 例如“中东欧”				mdj
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0				// 当前音节是韵尾
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_FWD_POS_IN_PPH] != 0
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero		// 当前音节是零声母
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

#if WL_REAR_DEAL_AI_CHAN_YIN				//王林音库后处理：改变零声母ai4颤音 例如“障碍”

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0			// 当前音节是韵尾
			&& (g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_zero ||		// 当前音节是零声母
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal)		// 或当前音节是m，n modified by mdj 2012-03-19
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
				else if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD]==SM_CN_METHOD_nasal			// 或当前声母是m，n modified by mdj 2012-03-20
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

#if WL_REAR_DEAL_LI_SPEC			//王林音库后处理：处理li韵尾的时长和谱的问题
			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_l
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_i
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] == 0)	// 当前音节是韵尾
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
//			else if (!g_bRearSynChn)  //-英文强制改谱
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
				//决策第i个字的频谱叶节点
#if EM_DECISION_READ_ALL
				nLeafNo = RearDecision(i);
#else
				nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET+2);
#endif
				if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //频谱能量维0阶均值按4字节读，其余按2字节读
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) +							//计算叶节点的总偏移 
						(g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen+2) * (nLeafNo-1);		
				else
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1) +							//计算叶节点的总偏移 
						g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen * (nLeafNo-1);	

				fRearSeek(g_hTTS->fResCurRearMain,loffsetOfRes ,0);
	
				//读取决策的叶节点
				if( g_hTTS->pRearG->nModelFloatLen == 2 )			
				{
					//频谱能量维0阶均值按4字节读，其余按2字节读
					if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 )  
					{
						fRearRead(g_pRearRTGlobal->m_MgcLeafNode[i][j], sizeof(float),1, g_hTTS->fResCurRearMain); 

						fread_float2(g_pRearRTGlobal->m_MgcLeafNode[i][j]+1, g_hTTS->pRearG->nModelFloatLen,		
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width - 1, g_hTTS->fResCurRearMain);
					}
					else
					{
						fread_float2(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_hTTS->pRearG->nModelFloatLen,		
							g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width, g_hTTS->fResCurRearMain); //emInt16 g_pRearRTGlobal->m_MgcLeafNode[12][10] 存储每个决策结果均值的码本索引
					}					
				}
				else
				{
					fRearRead(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_hTTS->pRearG->nModelFloatLen,		
						g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width, g_hTTS->fResCurRearMain); //emInt16 g_pRearRTGlobal->m_MgcLeafNode[12][10] 存储每个决策结果均值的码本索引
				}

				if(g_bRearSynChn)
				{
				////le5&me5&ne5 men5的颤的问题  naxingyu 2012-03-19 
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
				else    //改频谱
				{
#if HTRS_REAR_DEAL_HH     //hh开头的问题，hearth，here，hello，hold,with hundred 
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
			else			//每个分段前后加的pau不参与决策（仅决策每个的第1个出现的pau，之后的用其替换）（时长，频谱，基频都是一样的） 2012-09-19
			{
				//必须拷贝，不能直接进行指针赋值（例如：g_pRearRTGlobal->m_MgcLeafNode[i][j] = g_pRearRTGlobal->m_MgcLeafNode[0][j];）
				emMemCpy(g_pRearRTGlobal->m_MgcLeafNode[i][j], g_pRearRTGlobal->m_MgcLeafNode[0][j], 
						g_hTTS->pInterParam->static_length * g_hTTS->pInterParam->width*sizeof(float));
			}

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_CHAN_YIN		//王林音库后处理：改变部分颤音  ze4，韵母en eng an ang后跟零声母
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
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM] = holdValueOne;            // 改变后一音节声母为非浊声母
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_R_SM_METHOD] = holdValueTwo;     // 改变后一音节声母为非浊声母
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
				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM] == FINAL_CN_e  &&		// 韵母e			hyl 2012-04-07
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM] == INITIAL_CN_zh)		// 声母zh
				{
					emMemCpy(g_pLabRTGlobal->m_LabOneSeg[i], g_LabZhe5bak, LAB_CN_LINE_LEN);
				}
				else
				{
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_light;     // 改变音调
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

			
#if WL_REAR_DEAL_OU_CHAN_YIN				//王林音库后处理：改变零声母ai4颤音 例如“恩恩爱爱”

			if(oulabChange == 2)					// 当前音节是ou3
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = SM_CN_PLACE_zero;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_zero;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_3;
				oulabChange = 0;
			}
			if(oulabChange == 1)					// 当前音节是ou1
			{
				oulabChange = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PPH] = 0;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_PLACE] = SM_CN_PLACE_zero;
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM_METHOD] = SM_CN_METHOD_zero;

			}
#endif

#if WL_REAR_DEAL_AI_CHAN_YIN				//王林音库后处理：改变零声母ai4颤音 例如“恩恩爱爱”

			if(ailabChange == 1)					// 当前音节是ai，modified by mdj 2012-03-19
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
			/* 释放临时决策树内存 */
#if DEBUG_LOG_SWITCH_HEAP
			emHeap_Free(g_tree , nAllocSize , "频谱第n颗树叶节点：《后端模块  》");
#else
			emHeap_Free(g_tree , nAllocSize );
#endif
#endif
		}

		//指向下一棵决策树
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //频谱能量维0阶均值按4字节读，其余按2字节读
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				(g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen + 2) * curNodeSum;		
		else
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) + 
				g_hTTS->pRearG->nMgcLeafPrmN * g_hTTS->pRearG->nModelFloatLen * curNodeSum;		
	}

	if(g_bRearSynChn)	{
#if WL_REAR_DEAL_LIGHT_MGC					//王林音库后处理：改变部分轻声的频谱（改善效果）
	for( i =0;i<nLineCountOfPau;i++ )
	{
		//频谱已决策完，将满足条件的轻声由4声改回来
		if( lightToneChange[i] == 1)								// 轻声改过
		{
			g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] = TONE_CN_light;			// 改回轻声			
		}
#if WL_REAR_DEAL_DCT_SHORT_SEN_LEN					//<王林>中文短句处理时长(字母的不处理)：专门处理：两字句  mdj 2012-03-23

		if( g_Res.offset_RearVoiceLib_Cur  == g_Res.offset_RearVoiceLib03 	//原始音库是《王林》的 
			&& 	durChange == 0)	//是《王林》
		{
				//两字句模式: 调整两字句时长
			if( g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_NUM_IN_C_SEN] == 2  
			&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_PW_NUM_IN_C_SEN] == 1 
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_SEN] == 0		//为两字句，且处理到了第2个字 （注意：两字句的第一个字不做处理）
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE] == TONE_CN_light			//声调为轻声
			&&	g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE] < TONE_CN_sil_pau)			//第1字也不是字母
			{
				//含英文字母的不处理
				for(j=0;j<5;j++)
				{
					if( g_pRearRTGlobal->m_PhDuration[i][j]>=2)			//hyl  2012-04-07  否则合成“收拾”等会错
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


#if WL_REAR_DEAL_VOL_TOO_LARGE   //频谱已决策完，对超大能量进行调整   2012-04-18

	ChangeTooLargeVol(nLineCountOfPau);

#endif
	}	// end of g_bRearSynChn


	//为加快决策速度，按状态循环决策基频
	for(j=0;j<g_hTTS->pInterParam->nState;j++ )
	{
		//读取基频第j颗树的叶节点个数，分配内存
		fRearSeek(g_hTTS->fResCurRearMain,Next_TREE_OFFSET ,0);
		fRearRead(&curNodeSum,2,1,g_hTTS->fResCurRearMain); // 基频第j颗树的叶节点个数

		nAllocSize = Table_NODE_LEN_INF*(curNodeSum-1);

		if( g_bIsUserAddHeap == 0 )
		{
#if EM_DECISION_READ_ALL	
#if DEBUG_LOG_SWITCH_HEAP
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize  , "基频第n颗树叶节点：《后端模块  》");
#else
			g_tree = (emByte *)emHeap_AllocZero( nAllocSize );
#endif	
			//读取基频第j颗树
			fRearRead(g_tree,Table_NODE_LEN_INF*(curNodeSum-1),1,g_hTTS->fResCurRearMain); // Load LogF0 Tree
#endif
		}
		else
		{
			g_tree = g_AllTree + g_AllTreeOffset[12+j];
		}

		for( i =0;i<nLineCountOfPau;i++ )
		{
			//决策第i个字的基频叶节点
			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DAO_LF0					//王林音库后处理：zhao3dao4的基频变调 例如：但没有一个像我这样能顺利地同时找到两家歌舞厅

		//处理在找到（zhao3dao4）中dao4出现基频变调的问题 modified by mdj 2012-2-28
			if( g_hTTS->m_ControlSwitch.m_nSpeakStyle != emTTS_STYLE_WORD					//不是：一字一顿效果
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_d 				// 当前音节是声母，d
			    &&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_ao                  // 当前音节是韵母，ao
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_4					// 当前音节是dao4
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM]==INITIAL_CN_zh 				// 当前音节是声母，zh
			    &&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_YM]==FINAL_CN_ao                  // 当前音节是韵母，ao
				&&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]==TONE_CN_3					// 前一音节是zhao3
			    &&  g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW]==0 )				// 当前音节是韵尾
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM] = INITIAL_CN_ch;					
				daolabChange = 1;
			}

#endif

#if WL_REAR_DEAL_MA0_DCT				//王林音库后处理：处理mao1韵尾DCT模板基频不好			mdj modifed on 2012-03-16

				if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW]==0				// 当前音节是韵尾
				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_SM]==INITIAL_CN_m
				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_ao
				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_1)					// 当前音节是mao1
//				&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]==TONE_CN_light)				// 前音节为轻声
				{
					g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]=TONE_CN_1;					// 前音节设为一声
//					maolabChange = 1;
				}
#endif

#if WL_REAR_DEAL_IANG_LF0							//王林音库后处理：处理iang4基频拐的问题			added by mdj 2012-03-15

			if(g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_YM]==FINAL_CN_iang
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_C_TONE]==TONE_CN_4					// 当前韵母是iang4
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM]==INITIAL_CN_zh					// 前音节为zhao
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_YM]==FINAL_CN_ao					// 前音节为zhao
			&& g_pLabRTGlobal->m_LabOneSeg[i][RLAB_SYL_BWD_POS_IN_PW]!=0 )				// 当前音节是韵尾
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

				if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //基频0阶均值按4字节读，其余按2字节读
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1)		
								+ (PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen+2)*(nLeafNo-1); //数据表格中的偏移量
				else
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum-1)		
								+ PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen*(nLeafNo-1); //数据表格中的偏移量	

				g_pRearRTGlobal->m_Lf0LeafNode[i][j] = loffsetOfRes;	 //emInt32 g_pRearRTGlobal->m_Lf0LeafNode[12][10] 存储每个决策结果在resource中的偏移
			}
			else			//每个分段前后加的pau不参与决策（仅决策每个的第1个出现的pau，之后的用其替换）（时长，频谱，基频都是一样的） 2012-09-19
			{
					g_pRearRTGlobal->m_Lf0LeafNode[i][j] = g_pRearRTGlobal->m_Lf0LeafNode[0][j];
			}

			if(g_bRearSynChn)	{
#if WL_REAR_DEAL_DAO_LF0					//王林音库后处理：dao4的基频变调

			if(daolabChange == 1) //基频决策完成，将符合zhao3dao4条件的前一音节声母改回来
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_SM] = INITIAL_CN_zh;
				daolabChange = 0;
			}
#endif

#if WL_REAR_DEAL_MA0_DCT				//王林音库后处理：处理mao1韵尾DCT模板基频不好			mdj
			if(maolabChange == 1)				// 前音节为轻声
			{
				g_pLabRTGlobal->m_LabOneSeg[i][RLAB_L_TONE]=TONE_CN_light;					// 前音节设为三声
				maolabChange = 0;
			}
#endif

#if WL_REAR_DEAL_IANG_LF0							//王林音库后处理：处理iang4基频拐的问题			added by mdj 2010-03-15
			if(ianglabChange == 1)				// 当前音节是韵尾
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
			emHeap_Free(g_tree , nAllocSize , "基频第n颗树叶节点：《后端模块  》");
#else
			emHeap_Free(g_tree , nAllocSize );
#endif
#endif
		}

		//指向下一棵决策树
		if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //基频0阶均值按4字节读，其余按2字节读
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) 
				+ (PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen+2) *curNodeSum;	
		else
			Next_TREE_OFFSET += 2 + Table_NODE_LEN_INF*(curNodeSum-1) 
				+ PDF_NODE_FLOAT_COUNT_LF0 * g_hTTS->pRearG->nModelFloatLen *curNodeSum;

	}


	/******************************增加bap决策*********************/
	//sqb 2017-4-21    注：bap暂时没写一次读入决策树，后期可修改增加
			
	//为加快决策速度，按状态循环决策基频
	for (j = 0; j < g_hTTS->pInterParam->nState; j++)
	{
		//读取bap第j颗树的叶节点个数，分配内存
		fRearSeek(g_hTTS->fResCurRearMain, Next_TREE_OFFSET, 0);
		fRearRead(&curNodeSum, 2, 1, g_hTTS->fResCurRearMain); // bap第j颗树的叶节点个数

		nAllocSize = Table_NODE_LEN_INF*(curNodeSum - 1);
		g_tree = g_AllTree + g_AllTreeOffset[12 + j];
		
		for (i = 0; i < nLineCountOfPau; i++)
		{
			//决策第i个字的bap叶节点
			if ((i != 0 && i != nLineCountOfPau - 1) || (g_hTTS->m_bIsSynFirstAddPau == emFalse && i == 0))
			{
				nLeafNo = RearDecision_Line(i, Next_TREE_OFFSET + 2);


				if (g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //0阶均值按4字节读，其余按2字节读
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum - 1)
					+ (PDF_NODE_FLOAT_COUNT_BAP * g_hTTS->pRearG->nModelFloatLen + 2)*(nLeafNo - 1); //数据表格中的偏移量
				else
					loffsetOfRes = Next_TREE_OFFSET + 2 + Table_NODE_LEN_INF*(curNodeSum - 1)
					+ PDF_NODE_FLOAT_COUNT_BAP * g_hTTS->pRearG->nModelFloatLen*(nLeafNo - 1); //数据表格中的偏移量	

				g_pRearRTGlobal->m_BapLeafNode[i][j] = loffsetOfRes;	 //emInt32 g_pRearRTGlobal->m_Lf0LeafNode[12][10] 存储每个决策结果在resource中的偏移
			}
			else			//每个分段前后加的pau不参与决策（仅决策每个的第1个出现的pau，之后的用其替换）（时长，频谱，基频都是一样的） 2012-09-19
			{
				g_pRearRTGlobal->m_BapLeafNode[i][j] = g_pRearRTGlobal->m_BapLeafNode[0][j];
			}
		}

		//指向下一棵决策树
		if (g_hTTS->pRearG->nModelFloatLeafAdd == 2 && g_hTTS->pRearG->nModelFloatLen == 2)  //基频0阶均值按4字节读，其余按2字节读
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



/******************** 用于决策相关的子函数的相关子函数 ************************/
/*子函数功能： 将时长的决策结果进行语速调整，取整数后直接存入pOffsetValue[10] */
/*输入参数：从指定的文件（FILE *infile ,emInt32 Offset）取数据到数组pOffsetValue[10]*/
/*          及语速调整值emInt8 Speech_speed，及上一音节残留值float remain*/
/*返回值：音节残留值float remain                                         */
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
		pOffsetValue[i] =(emInt16) (outFloat + remain + 0.5); //取整数
		if (pOffsetValue[i]<1)
			pOffsetValue[i] = 1;
		remain += outFloat - pOffsetValue[i];
	}
	return remain;
}

/*                                                                  */
/*                                                                  */
/******************* 决策相关的子函数集定义完毕 *********************/




/******************* 用于矩阵求解运算的子函数集 *********************/


#if  1

//计算谱均值WUM矩阵
//优化后的（速度提高30%）

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

//计算谱均值WUM矩阵
//优化前的
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

//计算MSD的WUW和WUM矩阵
//优化后的（速度提高30%）
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

//计算MSD的WUW和WUM矩阵
//优化前的
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




/* 根据msd值获得每帧的msd标志 */
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
	msd = (float *) emHeap_AllocZero(g_hTTS->pInterParam->nState * sizeof(float), "后端MSD：《后端模块  》");
#else
	msd = (float *) emHeap_AllocZero(g_hTTS->pInterParam->nState * sizeof(float));
#endif


if( ! g_bRearSynChn)	//英文处理
{
	nMsdFazhi = 0.398;		//专门为英文设定，不取g_hTTS->pInterParam->msd_threshold（0.4）;

#if !HTRS_REAR_DEAL_CHANGE_MSD				//不进行：MSD值的后处理

	t = 0;
	for (n = 0; n < nLineCountOfPau; n++) 
	{
		for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
		{	
			result = (n*g_hTTS->pInterParam->nState+s)*7;
			msd[s] = g_lf0param[result+6];

			if ( msd[s] > nMsdFazhi )												// 浊音状态
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)		// 状态所包含的帧置：浊音标志
					g_pRearRTCalc->m_MSDflag[t++] = VOICED;
			else																	// 清音状态
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)		// 状态所包含的帧置：清音标志
					g_pRearRTCalc->m_MSDflag[t++] = UNVOICED;
		} 
	} 
#else										//进行：MSD值的后处理
#endif
}//英文处理完毕
else					//中文处理
{
	// 初始化时间计数，按字循环处理
	t=0;
	for (n = 0; n < nLineCountOfPau; n++) {
		labFrameLength=0;
		labVoicedStartNum=0;
		lastMsdFlag=FALSE;
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_letter)		//中文汉字，非字母
		{
			// 根据状态MSD值寻找所有的清到浊转换点
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
			
			// 寻找所有候选转换点中离参考转换点最近的转换点,sil和pau不处理
			if((g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_pau) 
				&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]!= FINAL_CN_nil)) 
			{
				// 决策出来的参考转换点
				bestVoicedStart = (emInt16)(labFrameLength * syl_uvp[n]);
				minErr=10000;
				minErrId=-1;
				// 寻找误差最小的转换点，labVoicedStartList为候选转换点列表
				for (i=0;i<labVoicedStartNum;i++)
				{
					if (minErr>emAbs(labVoicedStartList[i][0]-bestVoicedStart))
					{
						minErr=emAbs(labVoicedStartList[i][0]-bestVoicedStart);
						minErrId=i;
					}
				}

				//如果不是（零声母，鼻音，边音，R）可能的浊音声母，且得到的清浊转换点为0，则继续搜索下1清浊转换点（一般列表中最多2个清浊转换点，1个前1音节残留的浊音，1个本音节浊音）
				//例如： 行头以及舞台上有关的知识
				if (labVoicedStartList[minErrId][0] == 0)
				{
					if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_zero  &&		//不为零声母
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_l &&			//不为l
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_m &&			//不为m
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_n &&			//不为n
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_r )				//不为r
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

					// 修改状态MSD值，使清音状态MSD小于0，浊音状态MSD大于1，以增强区分度
				if (minErrId>=0)
				{
					msd_change = 0;

#if	WL_REAR_DEAL_LETTER		//王林：字母的单独后处理
					// naxy 1219 如果前一音节是以下英文字母，则不必保留第一个浊音状态
					if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_f ||		//字母：
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_h ||		//字母：
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_s ||		//字母：
						g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] == FINAL_CN_letter_x )		//字母：
					{
						msd_change = 1;
					}
#endif

					// 清音状态（乘以-1）
					for (i = n*g_hTTS->pInterParam->nState;
						i<labVoicedStartList[minErrId][1];
						i++) 
					{	
#if WL_REAR_DEAL_GONG4_GONG4
						//改善合成时有些音发出“嘭嘭”跳跃式杂音
						//每个字的第1个状态清浊音决策的结果不根据《清浊音转换点》强制改变。（解决gong4音的问题：解决汉字尾部的gong4音的问题）
						if(msd[i-n*g_hTTS->pInterParam->nState]<g_hTTS->pInterParam->msd_threshold)
							msd_change = 1;
						if(msd_change == 1)
							msd[i-n*g_hTTS->pInterParam->nState] *= -1;
#else
						//原来没改善的
						msd[i-n*g_hTTS->pInterParam->nState] *= -1;
#endif
					}

					// 浊音状态（加上1）
					for (i = (n+1)*g_hTTS->pInterParam->nState-1;
						i>=labVoicedStartList[minErrId][1];	i--)
						msd[i-n*g_hTTS->pInterParam->nState] += 1.0;
				}
			} 
			// 如果是sil和pau，全部置为清音状态
			else 
			{
				for(s = 0; s < g_hTTS->pInterParam->nState; s++)
					msd[s] *= -1;
			}
			*/
		} 
		// 字母的处理
		else 
		{	

#if	WL_REAR_DEAL_LETTER		//王林：字母的单独后处理

			//英文字母的处理 naxy 1219
			for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
			{
				result = (n*g_hTTS->pInterParam->nState+s)*7;
				msd[s] = g_lf0param[result+6];
			}

			//强制为：浊音+清音
			//例子：是f型。是h型。是k型。是s型。
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_f ||		
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_h ||		
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_s ||		
			   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_x )		
			{
				for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
				{		//找到浊音开始
					if(msd[s]>g_hTTS->pInterParam->msd_threshold)
						break;
				}
				for (; s < g_hTTS->pInterParam->nState; s++) 
				{			//找到由浊变清的第一个清音帧
					if(msd[s]<g_hTTS->pInterParam->msd_threshold)
						break;
				}
				result = s;
				
				for (s = result+1; s < g_hTTS->pInterParam->nState; s++) 
				{	
					if(s >= 8 )  //20120904 加此句 （否则8K模型会有错：X。H。）
					{
						//强制更改后面错判的状态，包括msd值和谱
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
					msd[s] = 0.9;										//强制为全浊音

				//改变能量凹进去的问题：(只跟后1字母是C有关) 例如：AC。EC。IC。NC。UC。 AAC。EEC。IIC。NNC。UUC。
				if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM]  == FINAL_CN_letter_c )		//后1字母是C的话
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

#if EM_8K_MODEL_PLAN	//8K模型配置  防止w,d全浊的情况
				//防止全浊的情况  hyl 2012-10-12	
				//例如：Power Down模式。Www。WinCE。wa。wb。wc。wd。w人。w的。w为。
				//例如：LA40B530P7R。wb。wd。
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_d || 
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_w )	
				{
					msd[0] = 0.1;
				}
#else					//16K模型配置
				//防止全浊的情况  hyl  2012-04-20   单字母b，d发音不好
				if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_b ||	
					g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_d ) 
				{
					msd[0] = 0.1;
				}
#endif
				

				for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
				{		//找到清音开始
					if(msd[s]<g_hTTS->pInterParam->msd_threshold)
						break;
				}
				for (; s < g_hTTS->pInterParam->nState; s++) 
				{			//找到由清变浊的第一个浊音帧
					if(msd[s]>g_hTTS->pInterParam->msd_threshold)
						break;
				}
				result = s;
				
				for (s = result+1; s < g_hTTS->pInterParam->nState; s++) 
				{	//强制更改后面错判的状态为浊音
					if(msd[s]<g_hTTS->pInterParam->msd_threshold)
						msd[s] = 0.9;
				}			

				//改变能量凹进去的问题：(只跟后1字母是C有关) 例如：BC。DC。GC。JC。KC。PC。TC。ZC。 BBC。DDC。GGC。JJC。KKC。PPC。TTC。ZZC。
				if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM]  == FINAL_CN_letter_c )		//后1字母是C的话
				{
					g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 0.7;
					g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 0.7;
				}
				
			} 
			else if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_v )		//v
			{
				for (s = 2; s < g_hTTS->pInterParam->nState; s++) 
					msd[s] = 0.9;

				//改变能量凹进去的问题：(只跟后1字母是C有关) 例如：VC。 VVC。
				if( g_pLabRTGlobal->m_LabRam[nStartLineOfPau+n][RLAB_C_YM]  == FINAL_CN_letter_c )		//后1字母是C的话
				{
					g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 0.7;
					g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 0.7;
				}
			}

			//改善吱吱音：改变K字母的第3个状态的时长，分给前面的1状态
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_k )		//k
			{
				g_pRearRTGlobal->m_PhDuration[n][1] += g_pRearRTGlobal->m_PhDuration[n][2] -1;
				g_pRearRTGlobal->m_PhDuration[n][2] = 1;
			}
#endif

		}

//改善：《清浊判别不好》问题： 打开《强制某些拼音清音多1个状态》开关
#if	 EM_SYS_QING_ADD_ONE_STATUS

		if(g_hTTS->pRearG->stage == MODEL_MGC)	//MGC模型
		{
			pModifyOnePinYinMSD     = g_ModifyOnePinYinMSD_StrMgc;
			nModifyOnePinYinMSDLine = ModifyOnePinYinMSD_Line_StrMgc;
		}
		else							//LSP模型
		{
			//适用模型： （207-2模型）StrLsp（但基频用StrMgc，打开《强制某些拼音清音多1个状态》开关）
			pModifyOnePinYinMSD     = g_ModifyOnePinYinMSD_StrLsp_MgcLf0;
			nModifyOnePinYinMSDLine = ModifyOnePinYinMSD_Line_StrLsp_MgcLf0;

			//适用模型： （207-1模型）StrLsp
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
				//若前1状态是清音，后1状态是浊音，则将后1状态强制成清音，然后break
				if(msd[s] < g_hTTS->pInterParam->msd_threshold && msd[s+1] > g_hTTS->pInterParam->msd_threshold)
				{
					msd[s+1] = 0.1;
					break;
				}

				//针对ru4 ri4的处理，
				if( s==1 && msd[s] > g_hTTS->pInterParam->msd_threshold && msd[s+1] > g_hTTS->pInterParam->msd_threshold )
				{
					msd[s] = 0.1;
					break;
				}
			}

		}
#endif

		// 对状态MSD值进行的后处理
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] != INITIAL_CN_letter &&		//排除sil和pau和字母
			(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]!=INITIAL_CN_pau) 
			&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]!=FINAL_CN_nil)) 
		{
			// 找到清浊转换状态，以备后用
			for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
			{
				if (msd[s]<g_hTTS->pInterParam->msd_threshold)
					break;
			}
			if(s == g_hTTS->pInterParam->nState) // 全浊音
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


#if UVP_DUR_MOD_P //二级MSD判决，将MSD值在0.4到0.5之间的浊音起始边界状态判决为清音状态，naxy 2011-12-02
			// 时长较长的状态的msd不能在二次判决中修改，naxy, 2011-12-30 修改
			for(s=1; s < g_hTTS->pInterParam->nState; s++) 
			{
				if(msd[s-1] < g_hTTS->pInterParam->msd_threshold && msd[s] > g_hTTS->pInterParam->msd_threshold
					&& msd[s] < (g_hTTS->pInterParam->msd_threshold+1.1))
				{
					//处理爆破音：第1个状态小于4帧，且第2状态的msd值在0.4和0.5之间，
					//且发音方式为：g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] :3	asprt_stop塞送气	p,t,k  8	stop塞音	b,d,g
					//则不强制第2状态为清音
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
			// 修改塞音声母音节第2、3状态的MSD值，强制塞音声母为一个状态，为改善gong4，naxy 2011-12-30 
			if(    (    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_b		//声母：b
				     || g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_d)	//声母：d
				&&	g_pRearRTGlobal->m_PhDuration[n][0] > 2 )
			{
				msd[1] = 1.9;
				msd[2] = 1.9;

				//以下暂留
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
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_r)			//声母为：r
				{
					//韵母为：i  e  
					if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_iii || g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_e  )	
					{
						msd[0] = 0.1;		//	强制本状态为：清音状态
						msd[1] = 0.1;		//	强制本状态为：清音状态
						msd[2] = 0.1;		//	强制本状态为：清音状态
					}
					else			//是r声母，但非:ri, re, ru
					{
						//hyl 2012-04-12  例如：[f0]每个人卖肾的动机，其实并不复杂。。[d]  人。忍。认。仍。肉。
						if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_FWD_POS_IN_PPH] == 0  //是韵首时(且不是：ren)，强制第1个状态清音；若非韵首，则不改变
							&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]!=FINAL_CN_en)
							msd[0] = 0.1;			
					}
				}

								
#endif


#if	WL_REAR_DEAL_LITTER_QST
			// naxy1208，12月8日出现的问题后处理
			for(s=msd_change; s < g_hTTS->pInterParam->nState; s++) 
				if (msd[s]>g_hTTS->pInterParam->msd_threshold)
					break;
			vstart = s;
			
			// ch, t、p、k声母清浊转换小杂音问题： 将浊音的第1个状态的时长缩成1帧，减掉的帧数平分给清音状态
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

			// qiang 出现吱吱音的后处理，2012-01-18
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_q &&  
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_iang)	// iang
				msd[vstart] = 0.1;

			// qie2 出现吱吱音的后处理，naxy120315
			if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_q &&
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_ie &&
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_2)
				msd[vstart] = 0.1;

			/*
			//以下策略不要：很少出现
			// zh声母清浊转换小杂音问题
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zh )  
			{
				if(vstart==2)
					msd[vstart] = 0.1;
			}
			*/

			
			//以下策略暂不要：仅个别能改善   --不要删
			// i、u、e、a韵母浊音部分中间状态能量突变小的问题  //态度很好。作者来了。他的爸爸。很快统一了。
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

			// 边音l加入全浊音音节，naxy, 2011-12-30 修改
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]== INITIAL_CN_zero ||			//为零声母：y, w，a，er: 强制所有状态为浊音
				g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_lateral)			//边音：l
			{
				msd[0] = 1.9;	
				msd[1] = 1.9;	
			}

			/*
			//以下策略不要：吱吱音能改善一点，但过度仓促，且有些会变坏
			// 零声母能量小的问题
			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_zero)	//抗议。他的阿姨。学习文化。我们的世界。正在运动。很多抗议民众试图将游行推进到主要街道上。
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


//#if	WL_REAR_DEAL_FOU3						//王林音库后处理：“否”字第4第5个状态强制为浊音  2012-04-18
//			//当前音节为 “fou3”
//			if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_f 
//				&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_ou 
//				&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]==TONE_CN_3)
//			{
//				msd[3] = 1.9;	
//				msd[4] = 1.9;	
//			}
//#endif


		}


#if WL_REAR_DEAL_SYL_TOO_FAST				//王林：改变个别音节过快的情况  

		//***************  以下暂不打开，暂不用 但不删 *********************
		
		//例如：欢迎光临。恐龙是怎么灭绝的。白手创业者找不出什么竞争优势。你因为感冒不得不请假在家休息。
		//必须与语速关联起来
		//#define  V_MIN_FRAME_FOR_WL			15	//正常语速下若音节帧数小于此数，则需改变（即g_hTTS->pRearG->Speech_speed=0.9）
		//#define  V_CHANGE_MIN_FRAME_FOR_WL	18	//正常语速下改变后的音节最小帧数（即g_hTTS->pRearG->Speech_speed=0.9）
		//if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE]<=TONE_CN_4)		//声调小于4，排除轻声和sil和pau和字母
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
		//		n1 = (V_CHANGE_MIN_FRAME_FOR_WL/g_hTTS->pRearG->Speech_speed) - Len_voiced;	//要调整的帧数

		//		if( (Len_unvoiced - n1) <= vstart)		//清音段要保留至少每个状态1帧
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

		//减少吱吱音：将韵首的shi4，shi1的msd[4]强制为清音  hyl 2012-04-20
		//例如： [z1][v10]实施。史记。失身，市话。是否。请注意车灯#是否打开。请注意车灯#实施打开。请注意车灯#失身打开。请注意车灯#史记打开。市话障碍。市话障碍#请在112前#加拨长途区号。
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

		//减少吱吱音：将qiao的msd[3]强制为清音  hyl 2012-04-23
		//例如： [v10]红敲。红桥。红巧。红翘。
		if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_q 
			&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]==FINAL_CN_iao
			)
		{		
			msd[3] = 0.1;			
		}

#endif

#if WL_REAR_DEAL_GONG4_GONG4 && EM_8K_MODEL_PLAN	  //8K模型配置：gong4 gong4音的处理  2012-10-12 hyl
		//例如：欢迎观看语音合成系统的演示欢迎观看语音合成系统的演示
		//例如：“玉兴八号”两艘商船5日上午在湄公河金三角水域遭遇袭击
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_L_YM] != FINAL_CN_nil )  //前音节不是sil和pau
		{
			if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_h 
				|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM]==INITIAL_CN_s )
			{
				msd[0] = 1.9;	
			}
		}
#endif

		// 更新g_lf0param数组中的MSD值以保持一致
		for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
		{
			result = (n*g_hTTS->pInterParam->nState+s)*7;
			g_lf0param[result+6] = msd[s];
		}

		// 设置当前音节每一帧的清浊音状态，并对帧清浊标志进行后处理
		bIsFirstVoice = emFalse;		//是否本音节的第1个浊音状态
		for (s = 0; s < g_hTTS->pInterParam->nState; s++) 
		{
			// 浊音状态的处理
			if (msd[s]>g_hTTS->pInterParam->msd_threshold)
			{

				// 为解决gong4音，将浊音帧提前，所以处理程序在帧MSD赋值之前
#if	WL_REAR_DEAL_GONG4_GONG4			
				
				//改善gong4音：若是目标音节，则浊音前移2帧。naxy 1219
				//目前改了：shi  suo  sui  zhe  j  x
				//
				//目前我测了一些包括单字和连续语流的不同上下文。
				//根据观察结果，shi和suo两个音节的gong4音影响较大，根据此方法调节可以改善。
				//而其他的音节根据目前的测试结果来看，没有必要进行调整，因为不存在由清音后移造成的频谱竖条现象。
				//进一步测试以后，可以增加这个方法的调整规则，前移的帧数也可以改变。
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_sh			
					&& g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_iii		
					&& syl_uvp[n]>0.6 
					&& s>0 && g_pRearRTCalc->m_MSDflag[t-1] != VOICED && s>0 
					&& g_pRearRTCalc->m_MSDflag[t-2] != VOICED )  
				{
					g_pRearRTCalc->m_MSDflag[t-1] = VOICED;
					g_pRearRTCalc->m_MSDflag[t-2] = VOICED;
				}



				// 增加新的音节策略，naxy, 2011-12-30 修改
				// suo, sui
				if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_s			
					&& (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_uo 
					|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM]  == FINAL_CN_uei)		//uo,ui
					&& syl_uvp[n]>0.44								// 此值越小修改的范围越大，原版本是0.45，2012-01-18 
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
				// 浊音状态所包含的帧浊音位
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)
				{
					g_pRearRTCalc->m_MSDflag[t] = VOICED;
					t++;
				}
				bIsFirstVoice = emTrue ;
			}
			// 清音状态的处理
			else
			{
				// 清音状态所包含的帧清音位
				for (it = 0; it< g_pRearRTGlobal->m_PhDuration[n][s] ; it++)
				{
					g_pRearRTCalc->m_MSDflag[t] = UNVOICED;
					t++;
				}
			}
		} // for s，状态循环
	} // for n，分段字循环
}//中文处理完毕


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(msd, g_hTTS->pInterParam->nState * sizeof(float), "后端MSD：《后端模块  》");
#else
	emHeap_Free(msd, g_hTTS->pInterParam->nState * sizeof(float));
#endif

} // end of ModifyMSD




/* WUW的LDL分解 */
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

/* WUW的LDL分解 */
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

/* WUW的LDL分解 */
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


/* 前代法 */
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


/* 后代法求参数 */
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

/* GV 参数生成（仅初始化） */
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

/* 加入清音帧基频标志位 */
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
/****************** 矩阵求解运算子函数集定义完毕 ********************/




/******************** 用于获取数据的子函数集 ************************/
/* 根据决策结果获取LF0参数                                          */
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
				if( g_hTTS->pRearG->nModelFloatLeafAdd == 2 )	//基频0阶均值按4字节读，其余按2字节读
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
/*根据决策结果获取bap*/     
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
				if (g_hTTS->pRearG->nModelFloatLeafAdd == 2)	//基频0阶均值按4字节读，其余按2字节读
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
/******************* 获取数据的子函数集定义完毕 *********************/


/* 改变个别状态的能量，属于后处理调整 */
void ModifyStateGain( emInt16 nLineCountOfPau ) // naxy 1219
{
	emInt16 n, s;
	emInt16 result;
	float msd, gain;
	emInt16 temp,t;
	emInt8 Curpp,Prepp,Nextpp;
    
	LOG_StackAddr(__FUNCTION__);

   
if( !g_bRearSynChn )		//英文处理     sqb 2017/1/6
{

}   //end !g_bRearSynChn
else			//中文处理
{
	for (n = 1; n < (nLineCountOfPau-1); n++)
	{

//慧听库后处理   sqb   2017-5-26
#if HT_REAR_DEAL_PAU_ENERGY
		if (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_pau
			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_sil)
		{
			for (s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] /= 10;
		}

#endif
//****************************************************************************************//
//本段不能要，否则8K模型出错（例如：X。H。）
//#if WL_REAR_DEAL_LETTER				//英文字母
//		if(    g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_s		//字母：	
//			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_f		//字母：
//			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_h		//字母：	
//			|| g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_letter_x )		//字母：
//		{
//			g_pRearRTGlobal->m_MgcLeafNode[n][9][0] = -11;	//将第10个状态的能量赋值为0
//		}
//#endif


#if WL_REAR_DEAL_DUR_XIEXIE
		// naxy, 2012-01-13, ie,ve
		if( (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ie ||		// 韵母：ie
			 g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ve )&&		// 韵母：ve
			 g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light)		// 轻声
		{
			g_pRearRTGlobal->m_MgcLeafNode[n][8][0] -= 1.0;	
			//g_pRearRTGlobal->m_MgcLeafNode[n][9][0] -= 1.0;	
		}
#endif


#if WL_REAR_DEAL_LIGHT_GAIN		//王林音库后处理：改变部分轻声的能量（改善效果）
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light)				// 轻声
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	
			
		}
#endif

#if WL_REAR_DEAL_LIGHT_GAIN_TWO  //补偿一些能量减少多的 le5/me5/ne5/de5/men5
		//ne5 modified by wangcm 2012-03-19
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
			 g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_n )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.08;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//le5 modified by wangcm 2012-03-16
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声			
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
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声			
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
		//if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
		//	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
		//	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
		//	g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_d )
		//	                                                            //  added by wangcm 2012-03-01				
		//{
		//	for(s = 0; s < g_hTTS->pInterParam->nState; s++)
		//		g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.00; 	//added by wangcm 2012-03-01 // 0.2->0.1
		//	
		//}
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
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
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_e      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_m )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] += 0.40;	//added by wangcm 2012-03-01 // 0.2->0.1			
		}
		//end  wangcm 2012-03-16
		//ya5  added by wangcm 2012-03-20
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ia      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zero )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.17;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//end  wangcm 2012-03-20
		//men5  wangcm 2012-03-16
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
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
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_a      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_l )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.1;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//la5 end
		//wa5
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ua      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_zero )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//wa5 end
		//zi5 wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是句尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&              // 4声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_ii      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_z )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.02;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//zi5 end wangcm 2012-03-17
		//ba5  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_SEN] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_a      &&
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_b )
			                                                            //  added by wangcm 2012-03-01				
		{
			for(s = 0; s < g_hTTS->pInterParam->nState; s++)
				g_pRearRTGlobal->m_MgcLeafNode[n][s][0] -= 0.2;	//added by wangcm 2012-03-01 // 0.2->0.1
			
		}
		//ba5 end wangcm 2012-03-17
		//zhe5  wangcm 2012-03-17
		if( g_pLabRTGlobal->m_LabOneSeg[n][RLAB_SYL_BWD_POS_IN_PPH] == 0 &&				// 当前音节是韵尾
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_TONE] == TONE_CN_light &&            // 轻声
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
		//现象：会有gong4音，波形表现为浊音开始段能量太大，且变化大
		//解决：浊音开始段能量平滑缩小
		if(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_asprt_stop ||	//塞送气	p,t,k
		   g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_stop)			//塞音	b,d,g
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
		
		// qie2 韵母首gong音 naxy120315
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


		//以下不要
		//现象：会有gong4音，波形表现为浊声母时长偏长
		//解决：将浊声母的时长缩短，且平滑浊声母到韵母段的能量
		//if (g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM_METHOD] == SM_CN_METHOD_nasal)		//鼻音	m,n
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

		//改善能量不稳
		if( //g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_SM] == INITIAL_CN_sh &&  // sh
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_C_YM] == FINAL_CN_en &&	// en
			//g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_TONE] == TONE_CN_light)					// 后一音节是轻声
			(g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_lateral ||		// 后一音节是边音声母，l
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_nasal  ||		// 后一音节是鼻音声母，m、n
			g_pLabRTGlobal->m_LabOneSeg[n][RLAB_R_SM_METHOD]==SM_CN_METHOD_zero) )			// 后一音节是零声母
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


//从文件中读取多个2字节的float数
emInt32  fread_float2(void * pDstBuf, emInt32 nElementSize,emInt32 nCount, FILE * pFile)
{

#if EM_READ_LEAF_FLOAT_ALL	

	//一次性读入整个float叶节点
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

	//单个float的读(多次读资源)
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

