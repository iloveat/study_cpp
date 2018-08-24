#ifndef	_EMTTS__EM_TTS_COMMON__H_
#define _EMTTS__EM_TTS_COMMON__H_

#include "emConfig.h"
#include "emHeap.h"
#include "emDebug.h"


#ifdef __cplusplus
extern "C" {
#endif


#define strtok(L,M)		(emPByte)strtok((char *)L,(const char*)M)			//MTK平台必须的，否则类型不匹配

//----------------------------------------ARM MTK平台的打印--------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------

//例如：
//kal_prompt_trace(MOD_IVTTS, "模拟MTK平台开始打印日志");			//MTK日志开关打开时，才有效
//kal_sleep_task(1);
#if ARM_MTK_LOG_TO_PC_VS

	#define kal_prompt_trace(a,b,c);			{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c); fclose(g_fMTK);}

	#define kal_prompt_trace2(a,b);				{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b); fclose(g_fMTK);}
	#define kal_prompt_trace3(a,b,c);			{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c); fclose(g_fMTK);}
	#define kal_prompt_trace4(a,b,c,d);			{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c,d); fclose(g_fMTK);}
	#define kal_prompt_trace5(a,b,c,d,e);		{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c,d,e); fclose(g_fMTK);}
	#define kal_prompt_trace6(a,b,c,d,e,f);		{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c,d,e,f); fclose(g_fMTK);}
	#define kal_prompt_trace7(a,b,c,d,e,f,g);	{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c,d,e,f,g); fclose(g_fMTK);}
	#define kal_prompt_trace8(a,b,c,d,e,f,g,h);	{g_fMTK = fopen("log/日志_MTK.log","a"); fprintf(g_fMTK,b,c,d,e,f,g,h); fclose(g_fMTK);}
	
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


//边界值定义
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


//音效常量  sqb
#define emTTS_USE_VoiceMode_CLOSE		    0			//关闭：音效模式（关闭）
//#define emTTS_USE_VoiceMode_FarAndNear	    1			//音效：忽远忽近
//#define emTTS_USE_VoiceMode_Echo		    2			//音效：回声
//#define emTTS_USE_VoiceMode_Robot		    3			//音效：机器人
//#define emTTS_USE_VoiceMode_Chorus		    4			//音效：合唱
//#define emTTS_USE_VoiceMode_UnderWater	    5			//音效：水下
#define emTTS_USE_VoiceMode_Reverb		    6			//音效：混响


#define emTTS_USE_YIBIANYIN_CLOSE			0			//一的变音: 关闭 
#define emTTS_USE_YIBIANYIN_OPEN			1			//一的变音: 开

// fhy 121225
#define SYNTH_CN						0
#define SYNTH_ENG						1
#define SYN_MIX_LANG					2

//*****************************************	 Define常量定义  ************************************************************************

#define  REAR_FRAME_LENGTH						80			//后端：帧长



//虚拟发音人：重采样系数  以1为中心设置    数值越小，声音越尖
#define RESAMPLE_SCALE_ROLE_Virtual_51			(1.18)		//正方向
#define RESAMPLE_SCALE_ROLE_Virtual_52			(1.32)		//正方向
#define RESAMPLE_SCALE_ROLE_Virtual_53			(1.07)		//正方向
#define RESAMPLE_SCALE_ROLE_Virtual_54			(0.80)		//反方向
#define RESAMPLE_SCALE_ROLE_Virtual_55			(0.80)		//反方向	

//虚拟发音人：基频系数    以0为中心设置    数值方向一般与重采样系数的方向一样，若相反，声音效果比较怪（如：54：唐老鸭）
#define F0_SCALE_ROLE_Virtual_51				(0.2)		//正方向	
#define F0_SCALE_ROLE_Virtual_52				(0.4)		//正方向
#define F0_SCALE_ROLE_Virtual_53				(0.10)		//正方向
#define F0_SCALE_ROLE_Virtual_54				(0.60)		//正方向	-- 只有这一种基频方向不同
#define F0_SCALE_ROLE_Virtual_55				(-0.25)		//反方向	

//虚拟发音人：时长系数    以0为中心设置    数值越大，语速越快
#define DUR_SCALE_ROLE_Virtual_51				(0.16)		//正方向	
#define DUR_SCALE_ROLE_Virtual_52				(0.29)		//正方向
#define DUR_SCALE_ROLE_Virtual_53				(0.064)		//正方向	
#define DUR_SCALE_ROLE_Virtual_54				(-0.18)		//反方向	
#define DUR_SCALE_ROLE_Virtual_55				(-0.18)		//反方向	


//虚拟发音人：针对Htrs英文库变音  
//Re-0.09999996，F0-0.12，DUR-0.090000011 ：目前测试的最佳搭配
//Re-0.10，F0-0.12，DUR-0.09：有点杂音
//Re-0.05，F0-0.06，DUR-0.05：有杂音
//Re-0.10，F0-0.25，DUR-0.18：有杂音
//Re-0.15，F0-0.19，DUR-0.13：有杂音
//Re-0.20，F0-0.25，DUR-0.18：无杂音，太像小孩
#define HTRS013_ENG_RESAMPLE_BASE				(-0.09999996)		//改变基础值：重采样系数
#define HTRS013_ENG_F0_BASE						(-0.12)				//改变基础值：基频系数
#define HTRS013_ENG_DUR_BASE					(-0.090000011)		//改变基础值：时长系数



//-------------------------------------------------------------------------------------------------------------------------

#define MAX_COMPANY_INFO_LEN					300			//用户信息的总字节数（包括前面的识别语）	
#define	COMPANY_INFO_NEED_COMPARE_LEN			22

#define MAX_HANZI_COUNT_OF_LINK_NEED_HEAP		(MAX_HANZI_COUNT_OF_LINK+10)	//开辟空间时须多开辟10个，例如在MAX_HANZI_COUNT_OF_LINK个汉字的末尾，来了一串数字或字母，一般不分开
#define CORE_CONVERT_BUF_MAX_LEN				(MAX_HANZI_COUNT_OF_LINK*2+2)	//编码转换后的编码存储区的最大字节个数   //以END_WORD_OF_BUFFER表示结束

#define END_WORD_OF_BUFFER			  			0xFFFF		//所有用到buffer的结束判断符
#define INVALID_CODE							0xA3A0		//空格：编码转换中无效编码(可丢弃的)：转成统一编码（可改）
#define SPACE_CODE								0x20		//空格：
#define TAB_CODE								0x09		//Tab键：

#define KEYWORD_SEARCH_COUNT		 			6			//表示从连续汉字串最后几个汉字中搜寻：“热线““等关键字，打开“按号码读”开关

#define  MaxLine_Of_OnePos_Of_TableBigram		126			//将《二元文发表》的固定Pos1转移到126个不同的Pos2的值一次性取入到内存

#define		MAX_MANUAL_PPH_COUNT_IN_SEN			50			//一句话中：手动设置韵律PPH的最大个数


#define  RES_OFFSET_ERROR				0


//标点符号：静音时长定义（单位：毫秒）
#if EM_SYN_SEN_HEAD_SIL
	#define	FRONT_SIL_MUTE					50				//若合成句首sil：一句话的前sil在后端的时长，最短为50ms
#else
	#define	FRONT_SIL_MUTE					0				//若不合成句首sil：
#endif

#if EM_USER_SWITCH_VOICE_EFFECT
	#define	LAST_SIL_MUTE					250			//一句话的后sil在后端的时长，最短为250ms（为了音效的回响好），但长了也不行（顿号的停顿会为负数）
#else
	#define	LAST_SIL_MUTE					50			
#endif

#define	PUNC_MUTE_JuHao					(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//句号
#define	PUNC_MUTE_WenHao				(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//问号
#define	PUNC_MUTE_GanTanHao				(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//感叹号
#define	PUNC_MUTE_ShengLueHao			(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//省略号
#define	PUNC_MUTE_HuanHangHao			(550 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//换行号
#define	PUNC_MUTE_FenHao				(500 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//分号
#define	PUNC_MUTE_DouHao				(450 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//逗号
#define	PUNC_MUTE_MaoHao				(350 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//冒号
#define	PUNC_MUTE_DunHao				(350 - FRONT_SIL_MUTE - LAST_SIL_MUTE)			//顿号

//文本数据类型
#define DATATYPE_HANZI 				 	0x10			// 数据类型宏定义： 汉字
#define DATATYPE_FUHAO 				 	0x11			// 数据类型宏定义： 符号
#define DATATYPE_SHUZI 				 	0x12			// 数据类型宏定义： 数字
#define DATATYPE_YINGWEN 			 	0x13			// 数据类型宏定义： 英文
#define DATATYPE_END 				 	0x14			// 数据类型宏定义： 结尾符
#define DATATYPE_UNKNOWN 			 	0x15			// 数据类型宏定义： 不识别的数据

//文本句子类型
#define SEN_TYPE_EN						0x10			//英文句
#define SEN_TYPE_CN						0x11			//中文句
#define SEN_TYPE_OTHER					0x12			//不识别句

//文本类型
#define  TextCsItem_TextType_HANZI		1
#define  TextCsItem_TextType_ZIMU		3

//提示音类型
#define PROMPT_SOUND					1
#define PROMPT_MSG						2

//PPH边界类型
#define  BORDER_NONE					0
#define  BORDER_LW						1
#define  BORDER_PW						2
#define  BORDER_PPH						3


#define  MODEL_MGC						0
#define  MODEL_LSP						1


//中文：词性：编号 ********************************************************************************************
//								包含词的个数	词性			例子
#define		POS_CODE_kong	0		//	0					
#define		POS_CODE_a		1		//	3584	形容词			一样、万幸、一致、上进、不人道
#define		POS_CODE_ad		2		//	745		副形词			不利、不好意思、不得已、狂热、狡猾
#define		POS_CODE_Ag		3		//	441		形容词语素		狭、猛、珍、琐、瑞、甚、皂
#define		POS_CODE_an		4		//	300		名形词			直接、真实、矛盾、礼貌、神秘
#define		POS_CODE_b		5		//	1869	区别词			戏剧式、成套、成心、成批、扁平
#define		POS_CODE_Bg		6		//	9		区别词语素		当、棕、橙、次、殿
#define		POS_CODE_c		7		//	253		连词			等到、紧接着、纵使、继而、综上所述、而且、若
#define		POS_CODE_d		8		//	1500	副词			苦心、草草、莞尔、莫非、蓄意、蓦然
#define		POS_CODE_dc		9		//	35						不大、几乎、分外、十分、尤其
#define		POS_CODE_df		10		//	14						不必、不曾、不用、不要、勿、没有
#define		POS_CODE_Dg		11		//	55		副语语素		蜂、诚、贼、陡、飞、惧、倏
#define		POS_CODE_ds		12		//	4		处所性副词		到处、四处、处处、随处
#define		POS_CODE_dt		13		//	20		时间性副词		一直、仍旧、从来、偶尔、往往
#define		POS_CODE_dv		14		//	1						大振
#define		POS_CODE_dw		15		//	7						就、都、也、光、凡、单、只
#define		POS_CODE_e		16		//	67		叹词			呀、呃、呐、呜呼、呦、呵呵
#define		POS_CODE_eg		17		//	1						诺
#define		POS_CODE_f		18		//	332		方位词			四周、外侧、对面、尽头、左
#define		POS_CODE_ft		19		//	1						底
#define		POS_CODE_g		20		//	17871	语素			巧、巨、巩、巫、差、疏、己、已
#define		POS_CODE_h		21		//	16		前接成分		次、泛、老、超、过、阿、非
#define		POS_CODE_i		22		//	7748	成语			非亲非故、强人所难、欢欣鼓舞、生不逢时、约定俗成
#define		POS_CODE_ia		23		//	192		形容词性成语	形影不离、彬彬有礼、忠心耿耿、悠然自得、惴惴不安
#define		POS_CODE_ib		24		//	87						承前启后、排山倒海、无独有偶、显而易见、梦寐以求
#define		POS_CODE_ic		25		//	67						死去活来、气喘吁吁、污七八糟、活灵活现、淋漓尽致
#define		POS_CODE_id		26		//	116						深更半夜、生生世世、由此及彼、真心实意、简明扼要
#define		POS_CODE_im		27		//	51						一往无前、万不得已、三三两两、出神入化、力所能及
#define		POS_CODE_in		28		//	537						匠心独运、匹夫之勇、十年寒窗、吉日良辰、后起之秀
#define		POS_CODE_iv		29		//	2998	动词性成语		吞吞吐吐、含情脉脉、听天由命、吹吹打打、呕心沥血
#define		POS_CODE_j		30		//	4631	简称略语		一国两制、村委、一中一台、一大、村干
#define		POS_CODE_jb		31		//	432						中外、中顾委、亚太、亚美、京、经过、伊、关贸
#define		POS_CODE_jd		32		//	1						同比
#define		POS_CODE_jm		33		//	3						年均、日均、人均
#define		POS_CODE_jn		34		//	439						一中全会、一国两制、一汽、世博会、两弹一星
#define		POS_CODE_jv		35		//	15						偷漏税、征管、除四害、上下班、打砸抢、扶贫帮困
#define		POS_CODE_k		36		//	43		后接成分		法、艇、观、论、边、长
#define		POS_CODE_l		37		//	5958	习用语			长一智、长久之计、悱恻缠绵、悲从中来、一一列举
#define		POS_CODE_la		38		//	69						无人过问、无路可走、琅琅上口、模糊不清、满头大汗
#define		POS_CODE_lb		39		//	29						脱贫致富、至亲至爱、霸王别姬、强有力、事实上
#define		POS_CODE_lc		40		//	6						合不拢嘴、有滋有味、笨手笨脚、滚瓜溜圆、细致入微
#define		POS_CODE_ld		41		//	62						不失时机、不管怎样、且慢、从今以后、冬去春来
#define		POS_CODE_lm		42		//	16						准确无误、年复一年、很早以前、正儿八经、背对背
#define		POS_CODE_ln		43		//	629		名词性习用语	专科学校、丝绸之路、世界大战、中产阶级、交换价值
#define		POS_CODE_lv		44		//	1146	动词性习用语	一一列举、一一对应、上档次、下苦功、不可逆转
#define		POS_CODE_m		45		//	538		数词			一万、一两千、片刻、百、百八十
#define		POS_CODE_mg		46		//	12						丁、丙、庚、卯
#define		POS_CODE_mq		47		//	42						一丁点儿、次年、一个、百倍、首次
#define		POS_CODE_n		48		//	67377	名词			
#define		POS_CODE_na		49		//	7						共和国、基金会、罪名、议案、肿瘤科、道口、集团
#define		POS_CODE_nf		50		//	1						地球
#define		POS_CODE_Ng		51		//	1661	名语素			皂、的、皇、盅、盏、益、监
#define		POS_CODE_nh		52		//	1						法
#define		POS_CODE_nr		53		//	6162	姓名			丁关根、万九如、丁丁、三毛、丘吉尔
#define		POS_CODE_nrf	54		//	949		姓名中的姓		丛、东方、东郭、东野、主父、乌雅
#define		POS_CODE_nrg	55		//	4591	姓名中的名		法、泰华、法祖、泽东、泽民、泽全
#define		POS_CODE_ns		56		//	4177	地名			乌鲁木齐、乐亭、九华山、拜泉、招远
#define		POS_CODE_nt		57		//	534		机构名			挪威队、振邦队、捷克队、捷安特队、摩洛哥队
#define		POS_CODE_nx		58		//	5						霡霂、八点黑、酱、潋、扌
#define		POS_CODE_nz		59		//	7152	其他专名		摩罗、摩罗丹、撒尔荷、萨尼族、福沙
#define		POS_CODE_nzg	60		//	1						苗族
#define		POS_CODE_o		61		//	257		拟声词			突突、簌簌、轰、轰隆隆、辘辘
#define		POS_CODE_p		62		//	127		介词			连、透过、遵照、鉴于、针对
#define		POS_CODE_pt		63		//	1						自
#define		POS_CODE_q		64		//	738		量词			钟、钧、钱、铲、锅、锤
#define		POS_CODE_qb		65		//	3						些、点、点儿
#define		POS_CODE_qc		66		//	50						片、瓣、畦、绺、缕、股
#define		POS_CODE_qd		67		//	104						英寸、赫兹、顷、丈、里
#define		POS_CODE_qe		68		//	98						个、件、册、副、卷、发
#define		POS_CODE_Qg		69		//	19		量词性语素		叶、坪、孔、扎、文、柄、涓
#define		POS_CODE_qj		70		//	29						对、帖、打、批、旬
#define		POS_CODE_ql		71		//	1						倍
#define		POS_CODE_qr		72		//	22						勺、坛、壶、杯、桌
#define		POS_CODE_qt		73		//	23		时间有关量词	点、秒、载、阵、光年、分
#define		POS_CODE_qv		74		//	37						匝、周、回、圈、场、局、层
#define		POS_CODE_qz		75		//	12						成、折、款、种、等
#define		POS_CODE_r		76		//	516		代词			雷其、一切、三者、个别、为什么、乃
#define		POS_CODE_Rg		77		//	10		代词性语素		甚、苟、若、予、伊、兹
#define		POS_CODE_rr		78		//	50		人称代词		彼此、您、我、旁人、本人、某
#define		POS_CODE_ry		79		//	28		疑问代词		焉、谁、那、什么、何、几多
#define		POS_CODE_ryw	80		//	10						为什么、为何、咋、如何、怎么
#define		POS_CODE_s		81		//	412		处所词			树上、校内、校门口、桌上、桥上
#define		POS_CODE_t		82		//	519		时间词			次年、正午、正月、此际、每周一
#define		POS_CODE_Tg		83		//	51		时语素			民、汗、清、现、禹、秋
#define		POS_CODE_tt		84		//	45						前朝、前清、前端、南、南端
#define		POS_CODE_u		85		//	45		助词			等、者、而外、般、见、说来
#define		POS_CODE_ud		86		//	1		助词的			的
#define		POS_CODE_ue		87		//	1		助词得			得
#define		POS_CODE_ug		88		//	6						矣、价、将（两个）
#define		POS_CODE_ui		89		//	1		助词地			地
#define		POS_CODE_ul		90		//	1		助词丁			丁
#define		POS_CODE_uo		91		//	1		助词过			过
#define		POS_CODE_us		92		//	1		助词所			所
#define		POS_CODE_uz		93		//	1		助词着			着
#define		POS_CODE_v		94		//	20838	动词			说来、峙立、崇仰、崛起、崩坍
#define		POS_CODE_vb		95		//	1						调整
#define		POS_CODE_vd		96		//	94		副动词			带头、平行、开门、强制、循环、慕名
#define		POS_CODE_Vg		97		//	904		动语素			慰、慷、憷、戏、战
#define		POS_CODE_vi		98		//	5290	不及物动词		战和、积肥、称帝、移位、稍息
#define		POS_CODE_vl		99		//	39		动词性惯用语	竟是、等于、算、统称、自称
#define		POS_CODE_vn		100		//	8861	名动词			自立、震荡、挺举、挽留、捆绑
#define		POS_CODE_vq		101		//	28						起、起来、过去、过来、进去、进来
#define		POS_CODE_vt		102		//	7		及物动词		提议、选举、决定、发起、宣告
#define		POS_CODE_vu		103		//	41						乐意、会、值得、只能、可能
#define		POS_CODE_vv		104		//	1						减产
#define		POS_CODE_vx		105		//	6		形式动词		有、给予、进行、予以、加以
#define		POS_CODE_w		106		//	0		标点			
#define		POS_CODE_wd		107		//	2		逗号			，
#define		POS_CODE_wf		108		//	2		分号			；
#define		POS_CODE_wj		109		//	3		句号			。
#define		POS_CODE_wky	110		//	8		右括号			全角：）
#define		POS_CODE_wkz	111		//	8		左括号			全角：（
#define		POS_CODE_wm		112		//	2		冒号			：
#define		POS_CODE_wp		113		//	0					
#define		POS_CODE_ws		114		//	1		省略号			…
#define		POS_CODE_wt		115		//	2		叹号			！
#define		POS_CODE_wu		116		//	1		顿号			、
#define		POS_CODE_ww		117		//	2		问号			？
#define		POS_CODE_wy		118		//	4		引号			
#define		POS_CODE_wyy	119		//	3		右引号			全角：”
#define		POS_CODE_wyz	120		//	3		左引号			全角：“
#define		POS_CODE_x		121		//	126		非语素字			套、德尔塔、派、谬、肉
#define		POS_CODE_y		122		//	49		语气词			不成、么、之乎、也、丁
#define		POS_CODE_yg		123		//	4						咧、耳（两个）
#define		POS_CODE_z		124		//	1733	状态词			争雄、井然、上乘、不凡、不懈
#define		POS_CODE_zg		125		//	2						訇
#define		POS_CODE_START	126		//	0		用于韵律预测	Pos-2=START
#define		POS_CODE_END	127		//	0		用于韵律预测	Pos+2=END



//浮点值修饰前缀
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



//*****************************************	 结构体定义  ************************************************************************

/* RESAMPLE   结构定义 */
typedef struct _tagReSampleData
{
    emInt16 PrePcm[9];		// 存储的先前9个PCM原始采样点ORDER=4 PrePcmLen = 2*ORDER+1 
	emInt16 gain;			// 增益
	emInt16 pi_freq;		// 最小频率
	emUInt16 des_int;		// 目标语音数据两个采样点之间代表的声音长度
	emUInt16 ori_int;		// 原始语音数据两个采样点之间代表的声音长度
	emUInt16 d_last;		// 目标语音最新声音位置	
} ;

typedef struct _tagReSampleData TReSampleData, emPtr PReSampleData;

#define MAX_CN_LAB_LINE_NUM		((emUInt16)(MAX_HANZI_COUNT_OF_LINK*1.4))				//中文最大Lab行		(需乘1.4倍，准备给sil和pau)

#define MIN_EN_LAB_LINE_NUM		(MAX_SYNTH_ENG_LAB_COUNT)

#if EM_ENG_AS_SEN_SYNTH
	#define MAX_EN_LAB_LINE_NUM		(400)												//按句子合成：英文最大Lab行
#else
	#define MAX_EN_LAB_LINE_NUM		(MAX_PHONE_INWORD + MAX_SYNTH_ENG_LAB_COUNT + 10)	//按单词合成：英文最大Lab行		//例子：[g2]bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
#endif

#define LAB_CN_LINE_LEN       				63									//中文：Lab最大列数
#define LAB_EN_LINE_LEN       				99									//英文：Lab最大列数


struct tagLabRTGlobal
{
	emByte emPtr m_LabRam[ MAX_CN_LAB_LINE_NUM ];											//HMM合成前的Lab信息（必须取中文，英文中最大的行数）
	emByte		m_LabOneSeg[MAX_SYNTH_ENG_LAB_COUNT+2][LAB_EN_LINE_LEN];					//存储一个合成单元的Lab（必须取中文，英文中最大的列数）
	emUInt16	m_MaxLabLine;																//HMM合成前的最大lab行数，即音节数		//zz 2014-6-26	一句中的音素lab数可能超过256  例如：100个w

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
	emUInt8     Pos;						//词性 
	emUInt8     Len;						//词长 
	emUInt8     TextType;					//内容类型   1.汉字（含标点） 3. 字母
	emUInt8     BorderType	;				//边界类型 0：字  1：LW   2：PW  3：PPH  
	emByte		nBiaoDian[2];				//本词后面跟的标点（主用用途：1.《武林外传》等多音字的判别； 2. 顿号冒号的判别）
};


struct ForcePPHInfo
{
	emUInt8		nStartIndex;		//强制PPH的：成对标点的前标点的位置
	emUInt8		nSylLen;			//强制PPH的：成对标点的前后标点之间的音节个数
	emUInt8		nCount;				//强制PPH的：计数器（关于音节个数的）
	emUInt8		bIsStartCount;		//是否：开始成对标点的前后标点之间的音节个数的计数
};

struct RearOffsetStr
{
	emInt32	uvpMod;			//偏移量：音节清浊转换点模型, changed by naxy19
	emInt32 sydMod;			//偏移量：音节时长模型, changed by naxy19
	emInt32	durMod;			//偏移量：时长模型
	emInt32	mgcWuw;			//偏移量：基频方差矩阵, changed by naxy17
	emInt32	mgcMod;			//偏移量：频谱模型
	emInt32	lf0Mod;			//偏移量：基频模型
	emInt32	bapMod;			//偏移量：bap模型
	emInt32	gv;				//偏移量：GV参数
};

typedef struct RearOffsetStr emPtr PRearOffsetStr;

typedef struct _emRearG
{
	emInt16		frame_length;		/* 帧长, default = 80 */
	float		Speech_speed;		/* 语速 */
	emUInt16	nModelSample;		/* 采样率 */
	emInt8		nModelFloatLen;		/* 后端所有模型的float叶节点所占的字节数：一般为2或4*/
	emInt8		nModelFloatLeafAdd; /* 后端所有模型的叶节点长度需加： 当float占2字节时，若此值=2，则特殊处理：基频模型0阶均值占4字节,频谱模型能量维0阶均值占4字节*/
	float		f0_mean;			/* 基频调整因子 */
	float		alpha;				/* default = 0.42 */
	float		gamma;				/* default = 0 */
	float		beta;				/* beta = 0.4 */	

	emBool		bIsVQ;				//频谱模型是否矢量量化   -- 界面参数
	emBool		stage;				//频谱类型		0：MGC  1：LSP	  -- 界面参数
	emBool      bIsStraight;		//用的是否是Straight训练方法

	emInt16		nMgcLeafPrmN;		//谱的页节点的参数数目

	struct RearOffsetStr	offset_cn, offset_eng;		//后端资源偏移量,cn-中文模型，eng-英文模型

}emRearG;

typedef struct _emInterParam
{
	emInt16		nState;					/* 状态数, default = 10 */
	emInt16		length;					/* 每个合成小段的帧数, max = 720 */
	emInt16		width;					/* 动态窗宽度, default = 3*/
	emInt16		static_length;			/* 静态特征维数, default = 25*/
	float		msd_threshold;			/* msd阈值, default = 0.4 */
	emInt16		win_coefficient[3][3];	/* 窗系数, default = [0 1 0; -0.5 0 0.5; 1 -2 1]   全部乘2，化成整型 */
}emInterParam;


struct tagControlSwitch
{
	emInt16				m_nCodePageType ;					//输入文本代码页类型

	emInt16				m_nRoleIndex;						//发音人				[m*]
	emInt8				m_bUsePromptsPolicy	;				//提示音和预录音处理策略[x*] 
	emInt8				m_nReadDigitPolicy 	;				//数字处理策略			[n*]
	emInt8				m_bYaoPolicy ;						//“一”的读法：		[y*] 
	emInt8				m_nSpeakStyle;						//发音风格				[f*]
	emInt8				m_nVoiceSpeed ;						//播放速度				[s*]
	emInt8				m_nVoicePitch;						//播放语调				[t*] 
	emInt8				m_nVolumn; 							//音量大小				[v*]
	emInt8				m_bXingShi 	;						//姓氏策略				[r*] 
	emInt8				m_bShiBiePinYinPolicy;	            //识别拼音				[i*] 
	emInt8				m_nManualRhythm ;					//韵律标注				[z*] 
	emInt8				m_bPunctuation;						//标点处理策略			[b*]		//我们独有的

	emInt8				m_nVoiceMode;						//音效模式				[e*]

	emInt8				m_nLangType ;						//语种					[g*] 
	emInt8				m_bZeroPolicy	 ;					//英文0的读法			[o*] 
	emInt8				m_bTimePolicy	 ;					//英文时间的读法		[c*]		//Zz


	emInt8				m_bYiBianYinPolicy;					//一的变音策略（0：变音禁止； 1：变音打开）
	emInt8				m_bIsPhoneNum ;						//后面可能出现的数字串读号码


	emInt8				m_bIsHandleToBiFen ;	            //是否读成比分，读成时为1，不读为0

	emInt8				bIsPauseNoOutPutEndSil;				//遇到[p*]停顿不输出本句的句尾sil
	emInt8				bIsPauseNoOutPutStartSil;			//遇到[p*]停顿不输出本句的句首sil

	emInt8				bIsReadDian;						//“.”是否读出
};

typedef struct tagControlSwitch TControlSwitch, emPtr PControlSwitch;

struct emHTTS_G
{
	emPointer			pCBParam;							//用户回调参数  输出音频数据文件的指针

	emPointer			fResFrontMain;						//指向前端资源的的指针
	emPointer			fResCurRearMain;					//指向后端资源的的指针
	emCBReadRes			m_cbReadRes;						//读取资源的回调函数指针

	emRearG				*pRearG;
	emInterParam		*pInterParam;
	emBool				m_FirstSynthSegCount;				//本次合成文本第几个分段计数。（例如：本段中1个句子的第1个小分段 = 1）
	emBool				m_bIsSynFirstAddPau;				//合成句的后端：是否已合成了首个分段前后加的临时pau
	emBool				m_bIsSynFirstTxtZi;					//是否：是进入合成的合成首字


	//emBool				m_bRunning;							//状态：是否在合成
    emBool				m_bStop	;							//状态：是否停止合成


	TControlSwitch		m_ControlSwitch;					//控制开关（含文本标记等控制开关）

	emByte				m_ShuZiBuffer[60] ;					//存放要发音的汉字（例如数字，￥，@等）		//只要遇到非汉字编码结束

	emInt16				m_nDataType;						//字串中第1个字的字符类型
	emTTSErrID			m_nErrorID;							//错误类型，只是临时用，及时处理，否则可能会被其它函数改变

	emBool				m_bIsMuteNeedResample;				//静音是否需重采样
	emUInt32			m_CurMuteMs;						//存储当前静音的毫秒数（各种情况时）
	emUInt32			m_NextMuteMs;						//存储下次静音的毫秒数（各种情况时）
	emBool				m_bIsStartBoFang;					//是否已经播音


	emInt16				m_nCurIndexOfAllHanZiBuffer; 		//当前处理到的HanZiBuffer的索引位置
	emInt16				m_nCurIndexOfEnBuffer; 				//当前处理到的英文Buffer的索引位置

	emUInt16			m_nNeedSynSize;						//需要合成的文本字节数

	emCPointer			m_pc_DataOfBasic ;					//从参数中获取的原始文本数据的指针      //按数据区长度识别结束
	emUInt32			m_nCurIndexOfBasicData; 			//当前处理到的基本文本数据的索引位置

	emPByte				m_pDataAfterConvert;				//编码转换后的文本数据的指针，最大字节个数为：CORE_CONVERT_BUF_MAX_LEN，以END_WORD_OF_BUFFER表示结束
	emInt16				m_nCurIndexOfConvertData; 			//当前处理到的编码转换后的文本数据的索引位置
	emBool				m_bConvertWWW;						//当前转换的编码中是否已出现WWW.

	emInt8				m_nPrevSenType;						//前句的语种类别

	emInt16				m_nHanZiCount_In_HanZiLink; 		//汉字链表中的汉字个数

	emBool				m_bIsErToLiang;						//是否“二”读成“两”

	emBool				m_bResample;						//输出的声音是否需要重采样

	emPByte				m_pPCMBuffer;
	emInt32				m_pPCMBufferSize;

	emPByte				m_Info;

	emInt16				m_ChaBoCount;						//插播计数

	struct promptInfo	m_structPromptInfo[1];	
	struct promptInfo	*m_nCurPointOfPcmInfo; 				//当前处理到的指向PcmInfo的指针


	emInt8				m_ManualRhythmIndex[MAX_MANUAL_PPH_COUNT_IN_SEN];		//一句话中：手动设置韵律PPH的索引数组
	emInt8				m_CurManualRhythmIndex;									//一句话中：手动设置韵律当前PPH的索引

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	emBool				bTextTailWrap;					//本行结尾是换行符，为日志打印韵律用
#endif

	emBool              m_HaveEnWordInCnType;			//本中文句中是否含有英文单词正合成的状态： 0：无英文单词  1：有英文单词正合成句首段； 2：有英文单词正合成句中的几个段； 3：有英文单词正合成句尾段

	emUInt8				m_nParaStartLine;				//控制从第几个音素开始合成
	emUInt8				m_nEnSynCount;					//英文中循环调用后端合成的计数

	float				m_TimeRemain;					//存放后端时长规整的余值

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

emExtern emUInt32				(*g_pOutputCallbackFun)( emPointer , emUInt16,emPByte,emSize);		//音频输出回调函数
emExtern emUInt32				(*g_pProgressCallbackFun)( emPointer , emUInt32 ,emSize);			//进度回调函数


#if DEBUG_LOG_SWITCH_ERROR
emExtern FILE					*g_fLogError;						//错误日志文件指针
#endif

#if EM_USER_VOICE_RESAMPLE
	emExtern TReSampleData		g_objResample[1];					// 重采样对象 
#endif

emExtern emUInt32				emTTS_OutputVoice( emPointer , emUInt16,emPByte,emSize);		//音频输出统一函数
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




