/* 配置文件 */
#ifndef	_EMTTS__EM_CONFIG__H_
#define _EMTTS__EM_CONFIG__H_


#define ZIMU_READ_AS_CN_MODEL					1			//中文句中的逐字字母发音（0：调用英文音库；1：调用中文音库）


//																内存+Cache	资源		 程序+前端常量		栈
//ARM7 MTK104测试 ： 8K模型 + 8K的Cache + 后端常量 + 前端文件	40K+8K		1.5M		 0.3M+0.5M			4.5K
//ARM9 WinCE 测试 ：16K模型 + 8K的Cache + 后端常量 + 前端文件	64K+8K		1.5M		 0.3M+1.2M			4.5K
//ARM9 Linux 测试 ：16K模型 + 全部文件							64K			2.7M		 0.3M     			4.5K

// 8K模型配置  ：4.5K栈，40K堆，不支持音效模式(堆小)，连续合成的最大汉字数为50，只支持发音人[m3] [m51] [m52]
//16K模型配置  ：4.5K栈，64K堆，可支持音效模式，      连续合成的最大汉字数为80，所有发音人都支持    

//====================================		版本+试用+音效+用户信息	  =================================================================

//注意：用户信息需在资源里配置（需重新生成资源）

#define EM_VERSION						"emTTS soft V3.40"		//版本号
#define EM_TRIAL								0				//是否：试用版 	
#define EM_USER_SWITCH_VOICE_EFFECT				0				//是否：识别音效模型[e?]    //sqb 不打开

//====================================		读资源方式（只能开1个）	  ===================================================================
#define EM_RES_READ_FILE						0				//程序中直接读文件（为英文合成暂时加的）
#define EM_RES_READ_CALLBACK					1				//用户回调读资源（必须设置资源读取回调,回调时定义成直接读写或文件读写）
#define EM_RES_READ_ALL_LIB_CONST				0				//LIB常量读所有资源（此开关特别：无需irf资源文件，全部从常量读）
#define EM_RES_READ_REAR_LIB_CONST				0				//LIB常量仅读后端资源（其它资源还是从irf中回调读,后端从lib常量读）
#define EM_RES_READ_DIRECT_ANKAI				0				//安凯直接读资源（SYN6658安凯平台专用）

//====================================		配置+资源	  =====================================================================

//08K模型配置-程序    ：40K堆，按三字合成，  连续合成的最大汉字数为50，其它发音人(不支持一字一顿风格，不支持降语速)
//16K模型配置-程序    ：64K堆，按韵律词合成，连续合成的最大汉字数为80，所有发音人都支持           

#define EM_8K_MODEL_PLAN						0				//配置方案（1：8K模型配置；0：16K模型配置）
//================================================================================================================================


//英文中是否提前预估Lab音素个数：  
//提前预估的好处：后端合成Lab不会超，无须超时触发后端合成（这时前端空间还没释放，会增加内存开销），或增大Lab数组的个数（会增加内存开销）
//提前预估的坏处：预估很难准确（尤其是带数字的），导致该一句合成的被分成两句合成，被随机分段，不符合韵律
#define	EM_EN_LAB_EXPECT				0			//目前必须设置成0   （没有全面测试预估方案的正确性）

//纯英文前端：新增 =====================================================================================================================================

#define EM_OPEN_HTRS_BIANYIN			0			//是否开启：英文Htrs库变音朗读
#define EM_ENG_AS_SEN_SYNTH				0			//是否开启：按句子合成（1：按句子合成  0：按单词分段合成） 【注意：设置成1，合成中文会出问题，有时候英文首字母也有点乱】

#define	HTRS_REAR_DEAL_SP				0			//海天瑞声英文库-后处理-改变sp音素的相关
#define HTRS_REAR_DEAL_ALL_VOL			0			//海天瑞声英文库-后处理-改全部音量  //必须开：否则音会超界（例如：Other）
#define HTRS_REAR_DEAL_CHANGE_MSD		0			//海天瑞声英文库-后处理-清浊转换改变MSD标志
#define DEBUG_LOG_ENF_MSD				0			//海天瑞声英文库-英文：MSD日志

//锐化-目前测试结果取：0.7倍系数，lsp差值0.3倍，循环2遍，（循环遍数越多，锐化越多但失真也大；  循环遍数越多，lsp差值用0.4倍更好一点）
#define HTRS_REAR_DEAL_RH_PARA			0.7			//海天瑞声英文库：后端LSP锐化开关（范围：0.5--1） （=1：不锐化 ； =0.7：锐化效果目前较好； 可调节此系数观察锐化的效果）
#define HTRS_REAR_DEAL_RH_PARA_FIX		(179)		//海天瑞声英文库：后端LSP锐化开关-定点（与HTRS_REAR_DEAL_RH_PARA_FIX同时改，是它的256倍）(0.5:128, 0.6:154, 0.7:179)
#define	HTRS_RH_PARA_MIN_DIFF			0.052		//海天瑞声英文库：后端LSP锐化优化：lsp差值的最小值  = 0.3 * 3.14 / 18  ( = 0.3倍或0.4倍 * 3.14 / 频谱维度)  （0.2倍：0.037；  0.25倍：0.043；  0.3倍：0.052；  0.35倍：0.061；  0.4倍：0.069；  0.5倍：0.087）
#define	HTRS_RH_PARA_MIN_DIFF_FIX		1704		//海天瑞声英文库：后端LSP锐化优化：lsp差值的最小值-定点（与HTRS_RH_PARA_MIN_DIFF同时改，是它的Q15倍，对应关系：0.037：1212；  0.043：1409；  0.052：1704；  0.061：1999；  0.069：2261；  0.087：2851；）
#define HTRS_RH_PARA_TIMES				3			//海天瑞声英文库：后端LSP锐化优化：循环锐化遍数


#define EM_PE_HEAP_SIZE					(12288)		/* 前端堆大小(8bit平台) */
#define EM_USERDICT_RAM					0			/* 是否将用户词典加载到内存 */
#define PURE_CHANGE_RHYTHM				0			//启用：纯英文前端：L3改善
#define PURE_CHANGE_ACCENT				0			//启用：纯英文前端：Accent改善
#define EM_ENG_PURE_LOG					0			//纯英文前端-日志开关



//#define HTRS_REAR_DEAL_SPEED_FACTOR		(0.0)		//海天瑞声英文库：语速调节		（=0：不调节； =0.1：语速慢一点）	

//#define	HTRS_OPEN_QUEST05				1			//海天瑞声英文库：是否启用用《05版问题集》训的模型
//#define HTRS_CHANGE_PHONE				1			//海天瑞声英文库：是否启用部分音素转换（例如：ax->ah）：必须启用

//#define	HTRS_HANDLE_ERR_DICT			1			//海天瑞声英文库：处理错误的单词
//#define	HTRS_HANDLE_PIE					1			//海天瑞声英文库：处理撇衍生词的相关代码（必须开此开关，仅用于查看相关代码）

//#define HTRS_REAR_DEAL_FIX				0			//海天瑞声英文库：修正定点

#define HTRS_LINK_CN_EN					0			//海天瑞声英文库：中英文连接处优化（1：模拟连接处有前后语境； 0：连接处无前后语境，直接是句首句尾）  优化的好些：例如：我是scansoft公司的mew speech。你hold住了吗？你去借他的windows光盘。FEELING酒吧，ROYAL舞蹈会所，KEY美发，

//=====================================================================================================================================


//=========================================海天瑞声英文库-后处理开关-小沈添加-2014-2-20=================================================
//#define HTRS_REAR_DEAL_GAIN             1         //海天瑞声英文库:能量调整总开关
//#define HTRS_REAR_DEAL_DUR              1         //海天瑞声英文库:时长调整总开关
//
//#define HTRS_REAR_DEAL_V                1         //v开头的音不好,只保留最后一个状态的能量
//#define HTRS_REAR_DEAL_M_N              0          //M N NG 谱的问题
//#define HTRS_REAR_DEAL_IY_R             1           // year
//#define HTRS_REAR_DEAL_AH               1         //ah时长短
//#define HTRS_REAR_DEAL_Z                1         // Z的问题
//#define HTRS_REAR_DEAL_S                1         // S + 浊音的时候最后一个状态修改 
//#define HTRS_REAR_DEAL_HH				1         //hh音开始有杂音,hh时长调整
//#define HTRS_REAR_DEAL_B                1         //海天瑞声英文库：破裂音b的杂音问题
//#define HTRS_REAR_DEAL_K                1         //k的问题,降低第一状态能量
//#define HTRS_REAR_DEAL_P                1         //爆破音p的杂音问题
//#define HTRS_REAR_DEAL_G                1         //爆破音g的杂音问题
//#define HTRS_REAR_DEAL_D                1         //爆破音d的杂音问题
//#define HTRS_REAR_DEAL_DH               1         // DH
//#define HTRS_REAR_DEAL_T                1         // t
//#define HTRS_REAR_DEAL_ZH               1         // ZH
//#define HTRS_REAR_DEAL_JH               1         // JH
//#define HTRS_REAR_DEAL_CH               1         // CH
//#define HTRS_REAR_DEAL_TH               1         //TH ,something
//#define HTRS_REAR_DEAL_Y                1         // Y开头的谱部分不好
//
//#define HTRS_REAR_DEAL_LF0_WIN		    0         //基频平滑



//==========================================================================================================================










//====================================		Cache缓存+决策树内存	  =====================================================================
//期望加快《前端》
#define EM_RES_CACHE							0				//是否支持资源Cache（最少配置：（8+4）*512字节）

//====================================		读决策树数据方式	  ============================================================

//if(启用《资源Cache》或《后端Lib常量读》)，	一定要配置成0； 
//else if()，具体配置成1还是0，需测试谁更快 
#define EM_DECISION_READ_ALL					0				//是否：读入整棵决策树（1：读入整棵树； 0：多次读入需要行）															

//此开关影响不大
#define EM_READ_LEAF_FLOAT_ALL					0				//是否：一次性读入整个float叶节点（1：读入整个； 0：单个float的读）
//================================================================================================================================















//====================================		emApiPlatform.h 配置	  =========================================================

//						
//====================================		平台开关(最多开1个)	  ===================================================================

//注意： ARM MTK平台的库函数 strcmp(str1,str2)也会出错，明明相等却返回-1，最好不用strcmp函数, 用strncmp或memcmp替换

#define ANKAI_TOTAL_SWITCH						0		//
#define ARM_LINUX_SWITCH						0		//ARM Linux平台  ：总开关 (支持计时日志)
#define ARM_WINCE_TIME							0		//ARM WINCE平台  ：总开关 (支持计时日志)

//2个开关开1个或都不开（为了对比调试）
#define ARM_MTK_LOG_TO_MTK						0		//ARM MTK平台	 ：日志开关（打印到PC机VS平台）
#define ARM_MTK_LOG_TO_PC_VS					0		//ARM MTK平台	 ：日志开关（打印到MTK平台）

#define ARM_MTK_LOG_TIME						0		//ARM MTK平台	 ：计时开关（必须打开 DEBUG_LOG_SWITCH_TIME 计时主开关才有效）


//====================================		《中文合成-日志》开关	  ==================================================================================

#define	DEBUG_LOG_SWITCH_CACHE					0		//是否：	输出《日志_资源Cache》
#define	DEBUG_LOG_SWITCH_HANZI_LINK				0		//是否：	输出《日志_汉字链表》
#define	DEBUG_LOG_SWITCH_HEAP					0		//是否：	输出《日志_内存堆》和《日志_内存栈》
#define	DEBUG_LOG_SWITCH_RHYTHM_RESULT			0		//是否：	输出《日志_韵律》--最终结果
#define	DEBUG_LOG_SWITCH_RHYTHM_MIDDLE			0		//是否：	输出《日志_韵律》--中间过程
#define	DEBUG_LOG_SWITCH_POS_IN_RHYTHM			0		//是否：	在  《日志_韵律》中，是否将POS信息也输出
#define	DEBUG_LOG_SWITCH_LAB					0		//是否：	输出《合成.lab》
#define	DEBUG_LOG_SWITCH_LAB_BUF				1		//是否：	输出lab
#define	DEBUG_LOG_SWITCH_TIME					0		//是否：	输出《日志--计时》
#define	DEBUG_LOG_SWITCH_ERROR					0		//是否：	输出《错误信息》
#define	DEBUG_LOG_POLY_TEST						0		//是否：	输出《日志--多音字测试--过程--temp》,测多音字必须打开
#define	DEBUG_LOG_SWITCH_PW_COMBINE				0		//是否：	输出《日志--韵律词拼接》
#define	DEBUG_LOG_SWITCH_IS_OUTPUT_POS			0		//是否：	在  《日志--韵律词拼接》中，是否将POS信息也输出 

//========================================================================================================================================



//====================================		《英文合成-日志》开关	  ==================================================================================



//========================================================================================================================================






























//==========================================  其它注释  ==================================================================================

//注意：在《emRepackApi.h》也有开关 《DEBUG_LOG_SWITCH_WAV_FILE》《DEBUG_SYS_SWITCH_DIRECT_PLAY》）
//注意：在《emPCH.h》也有开关 

//多音字测试：打开DEBUG_LOG_POLY_TEST日志开关；注释掉后端主程序；关闭emRepackApi.h中的两个开关：输出《emTTS.wav》+ 实时播放文本

/*安凯平台移植过程：
	1. emPCH.h的开关打开，关闭所有日志开关，打开安凯的总开关
	2. 搜《emTTS_HEAP_SIZE》，包括虚拟发音人和最慢语速和最大80个汉字一节点，目前每2字需内存47K，每3字需内存60K；若加音效还需加20K内存；
*/

//静音类型： 1.设置[p1000]；   2.标点停顿；	3. 制造句尾停顿效果 GenPauseNoOutSil()

//=======================================================================================================================================



//******************************** 编程规范(为了跨平台) ***********************************************************
//【资源读写】：兼容数据块直接读写和文件读写和ROM读写
//					只允许fRearSeek；fRearRead；fFrontSeek；fFrontRead（注意：不能有返回值）
//					不允许fseek，fread，fgetc；
//
//【ANSI内存操作库】：开关：EM_ANSI_MEMORY：//是否使用 ANSI 内存操作库
//					必须用emMemSet,emMemCpy,emMemMove,emMemCmp 分别代替：memset,memcpy,memmove,memcmp
//
//【ANSI字符串操作库】：开关：EM_ANSI_STRING：//是否使用 ANSI 字符串操作库	
//					emTTS核心程序中必须用emStrLenA 分别代替：strlen
//					emTTS核心程序中：无调用（strlen，wcslen），调用1次（emStrLenW，emStrLen），多次调用（emStrLenA）
//
//【Unicode 方式构建】：开关：EM_UNICODE：//是否以 Unicode 方式构建
//					emTTS核心程序中必须用emCharA分别代替：char
//					emTTS核心程序中：无调用（char），调用1次（emCharW，emChar），多次调用（emCharA）
//
//*****************************************************************************************************************



//====================================  基本固定不改的开关  ==================================================================================

#define EM_SYS_SWITCH_FIX						0				//是否：按《定点合成》--必须是LSP模型时才能打开


#if EM_ENG_AS_SEN_SYNTH
#define	EM_SYN_SEN_HEAD_SIL						1				//是否：合成每句的句首sil
#else
#define	EM_SYN_SEN_HEAD_SIL						0				//是否：合成每句的句首sil
#endif

#define OUTPUT_FIRST_SIL_IN_TEXT				0				//是否：输出合成文本中第1句的句首sil

#define	EM_PAU_TO_MUTE							1				//是否：句中的pau停顿直接输出静音（不决策解算滤波）（目前不打开，有风险）	

// fhy 121225
#define SYN_MIX_LANG_LAB						1				//是否：合成混合语言lab

#define USER_HEAP_MIN_CONFIG					(40<<10)		// 8K模型配置：LSP模型内存需求量（MGC模型会多出20K）
#define USER_HEAP_MAX_CONFIG					(64<<10)		//16K模型配置：LSP模型内存需求量（MGC模型会多出20K）
#define USER_HEAP_VOICE_EFFECT					(20<<10)		//音效模式：内存需求量

#if EM_8K_MODEL_PLAN	//8K模型配置	只支持50个汉字			
	#define MAX_FRAME_COUNT_OF_SYL				100
	#define MAX_HANZI_COUNT_OF_LINK				50
#else					//16K模型配置
	#define MAX_FRAME_COUNT_OF_SYL				150				//后端：分段合成的每个音节平均输出的最大帧数（需考虑语速最慢的时候）：目前90是合适的（语速最慢+定点），保险为150  // [s0]测最慢语速：闯闯闯闯闯闯闯闯闯闯闯闯闯闯闯。[d]
	#define MAX_HANZI_COUNT_OF_LINK				100				//连续合成的最大汉字数（多于会被拆分）（每增加10个字，内存空间大约需多占1K）
#endif

#define	EM_PW_SEGMENT_OPEN					1					//是否：按韵律词分段 （1：按韵律词  0：按固定字）
#define	MAX_CITIAO_COUNT					20					//词的最大词条个数（目前词条最多的是词“和”，共13个词条，例如保险起见可改成16个词条）
#define SEG_ADD_PAU_TEN_STAGE_TEN_FRAME		1					//是否：1：分段合成前后加pau都是10个状态共10帧；0：目前前pau为6帧后pau为4帧（效果也行，稍差一点） （关闭此开关可：减少运算量，减少内存）


#if EM_PW_SEGMENT_OPEN				//韵律词分段：适合：大配置（不能动，否则获取分段函数会有错）
	#define EM_SYNTH_ONE_WORD_PW				0				//是否：允许合成单字韵律词（0：不允许； 1：允许）
	#define FIRST_MIN_SYNTH_LAB_COUNT			3				//安凯：进入emTTS_SynthText函数首次合成的音节数的最小值（否则会有声音卡的情况）
	#define MAX_SYNTH_LAB_COUNT					5				//后端：按韵律词分段合成《每次》合成的音节数（sil和Pau也计数），但不含合成时首尾临时加的小pau  最小为2，但为3时最佳，若空间速度允许可用4或5（能涵盖所有韵律词）
	#define	EM_STYLE_WORD_SYNTH_LAB_COUNT		2				//后端：若是一字一顿风格，则每次合成的音节数（基本为2(1汉字+1Pau)）
#else								//按字数分段：适合：小配置（不能动，否则获取分段函数会有错，例如：改成1,1,1，但获取分段函数可能会返回2，造成空间溢出）
	#define FIRST_SYNTH_LAB_COUNT				3				//后端：按字分段合成时《首次》合成的音节数（sil和Pau也计数），但不含合成时首尾临时加的小pau，最小为1
	#define MAX_SYNTH_LAB_COUNT					3				//后端：按字分段合成时《每次》合成的音节数（sil和Pau也计数），但不含合成时首尾临时加的小pau（除首次外），最小为1，必须大于首次合成的字数
	#define	EM_STYLE_WORD_SYNTH_LAB_COUNT		3				//后端：若是一字一顿风格，则每次合成的音节数（基本为2(1汉字+1Pau)）
#endif


// fhy 121225
#define NSTATE_ENG						5		//英文
#define NSTATE_CN						10		//中文
//#define	NSTATE							NSTATE_CN		//设为10 兼容英文

#if EM_ENG_AS_SEN_SYNTH
	#define MAX_SYNTH_ENG_LAB_COUNT		100		// 合成一段的最大英文lab行  【按句分段】
#else
	#define MAX_SYNTH_ENG_LAB_COUNT		8		// 合成一段的最大英文lab行  【按单词分段】   （不能小于8，LabNumOfWord()函数有风险，1个音节最大可能有8个音素）（若长单词的音素超过此数，会在LocateSegment_Eng()中再次分段）
#endif


//注意：此开关设置成1会有隐患，还未调试更正（在MTK下合成以下文本句尾会有噪音。例如：bbb。zzz。fff。dddddd。iiiiii。）
#define EM_CAL_EXP								0				//是否启用exp指数运算的定点化（每个分段滤波能节省15ms，声音略有丁点改变）


//=======================================================================================================================================



//====================================	专门针对《王林：StrLsp18维：中文261模+字母303模》音库：做的一系列后处理 ======================================================================================
	
//设置参数的
#define WL_REAR_DEAL_SPEED_FACTOR				(0.06)	//王林音库后处理：语速调节		（=0：不调节； =0.1：语速慢一点）			
#define WL_REAR_DEAL_GV_PARA					0		//王林音库后处理：后端基频GV开关 （范围：  0--1） （=0：不开GV ； =0.7：GV效果目前较好； 可调节此系数观察GV的效果）

#define WL_REAR_DEAL_RH_PARA					0.7		//王林音库后处理：后端LSP锐化开关（范围：0.5--1） （=1：不锐化 ； =0.7：锐化效果目前较好； 可调节此系数观察锐化的效果）
#define WL_REAR_DEAL_RH_PARA_FIX				(179)	//王林音库后处理：后端LSP锐化开关-定点（与WL_REAR_DEAL_RH_PARA同时改，是它的256倍）


	//需打开的    sqb 2016.12.5
#define		WL_REAR_SWITCH      (0)				//开关

#define	WL_REAR_DEAL_LETTER						WL_REAR_SWITCH		//王林音库后处理：英文字母
#define	WL_REAR_DEAL_VIRTUAL_QST				WL_REAR_SWITCH		//王林音库后处理：是否：处理虚拟发音人的一些问题
#define WL_REAR_DEAL_CHAN_YIN					WL_REAR_SWITCH		//王林音库后处理：改变部分颤音  ze4，韵母en eng an ang ong后跟零声母
#define WL_REAR_DEAL_LIGHT_GAIN					WL_REAR_SWITCH		//王林音库后处理：改变部分轻声的能量（改善效果）
#define WL_REAR_DEAL_LIGHT_GAIN_TWO		    	WL_REAR_SWITCH		//王林音库后处理：补偿一些能量减少多的 le5/me5/ne5/de5/men5	wchm
#define WL_REAR_DEAL_END_GAIN   		    	WL_REAR_SWITCH		//王林音库后处理：for zi4/pian4 unlight	wchm
#define WL_REAR_DEAL_END_GAIN_TWO           	WL_REAR_SWITCH		//王林音库后处理：for pin3 unlight	wchm
#define WL_REAR_DEAL_LIGHT_MGC					WL_REAR_SWITCH		//王林音库后处理：改变部分轻声的频谱（改善效果）
#define WL_REAR_DEAL_DUR_ZENME					WL_REAR_SWITCH		//王林音库后处理："怎么"
#define WL_REAR_DEAL_SYL_TOO_FAST				WL_REAR_SWITCH		//王林音库后处理：音节过快		????????
#define WL_REAR_DEAL_SYL_TOO_FAST_NEW			WL_REAR_SWITCH		//王林音库后处理：音节过快 wchm
#define	WL_REAR_DEAL_FOU3						WL_REAR_SWITCH		//王林音库后处理：“否”字		第4第5个状态强制为浊音
#define WL_REAR_DEAL_LOWER_F0					WL_REAR_SWITCH		//王林音库后处理：基频低		韵尾零浊声母4声基频低的调整，托运货物，评价项目, naxy 1220
#define WL_REAR_DEAL_EN_ENG_QST					WL_REAR_SWITCH		//王林音库后处理：韵母eng	所有weng，另en2和eng2后跟3声的零浊声母  例如：入瓮，老翁，她很美，[i1]sen2mei3。[i1]wen2mei3。[i1]deng2mei3。[i1]feng2mei3
#define WL_REAR_DEAL_LITTER_QST					WL_REAR_SWITCH		//王林音库后处理：小问题	主要是：gong音，吱吱音。	零声母强制为全部浊音，ch p t k声母吱吱音盖上
#define WL_REAR_DEAL_R_INITIAL					WL_REAR_SWITCH		//王林音库后处理：声母r
#define WL_REAR_DEAL_GONG4_GONG4				WL_REAR_SWITCH		//王林音库后处理：gong4音
#define WL_REAR_DEAL_FIX						WL_REAR_SWITCH		//王林音库后处理：修正定点
#define	WL_REAR_DEAL_LAB_LONG_SEN				WL_REAR_SWITCH		//王林音库后处理：Lab中解决长句问题[[[[[[[[
#define	WL_REAR_DEAL_LAB_SHORT_SEN				WL_REAR_SWITCH		//王林音库后处理：Lab中解决短句问题：3字短句在lab中模拟成4字句
#define	WL_REAR_DEAL_DCT_LIGHT					WL_REAR_SWITCH		//王林音库后处理：DCT模板：中文韵律短语尾轻声（两字句或以上，不处理字母）
#define	WL_REAR_DEAL_DCT_PPH_TAIL				WL_REAR_SWITCH		//王林音库后处理：DCT模板：中文韵律短语尾	 （不含单双短句，不处理字母）
#define	WL_REAR_DEAL_DCT_SHORT_SEN				WL_REAR_SWITCH		//王林音库后处理：DCT模板：中文单双句		  (字母的不处理): 专门处理<单字句><两字句>的基频,时长,能量。
#define WL_REAR_DEAL_DAO_LF0					WL_REAR_SWITCH		//王林音库后处理：zhao3dao4的基频变调			mdj
#define WL_REAR_DEAL_RU3_LF0					WL_REAR_SWITCH		//王林音库后处理：ru3的基频变调					mdj
#define WL_REAR_DEAL_LIANG_LF0					WL_REAR_SWITCH		//王林音库后处理：liang3的基频变调				mdj
#define WL_REAR_DEAL_DCT_PPH_TAIL_TRAIL			WL_REAR_SWITCH		//王林音库后处理：DCT模板+原始基频的中值：中文韵律短语尾	 （不含单双短句，不处理字母）	mdj
#define WL_REAR_DEAL_DCT_SEN_TAIL_LEN			WL_REAR_SWITCH		//王林音库后处理：DCT模板+增加韵尾音节时长：区分中文韵律短语尾和句尾	 （不含单双短句，不处理字母）	mdj
#define WL_REAR_DEAL_LOWER_F0_ALL				WL_REAR_SWITCH		//王林音库后处理：基频低		韵尾零浊声母4声基频低的调整，补充WL_REAR_DEAL_LOWER_F0里面没有修改的非韵尾的基频连接问题	mdj
#define WL_REAR_DEAL_ZHUO_SM					WL_REAR_SWITCH		//王林音库后处理：浊声母，声母部分时长调整
#define WL_REAR_DEAL_OU_CHAN_YIN				WL_REAR_SWITCH		//王林音库后处理：改变零声母ou颤音 例如“中东欧”				mdj
#define WL_REAR_DEAL_AI_CHAN_YIN				WL_REAR_SWITCH		//王林音库后处理：改变零声母ai颤音 例如“恩恩爱爱”				mdj
#define WL_REAR_DEAL_THIRD_TONE_LF0				WL_REAR_SWITCH		//王林音库后处理：处理连续三声（应变调）的基频连接问题			mdj
#define WL_REAR_DEAL_MA0_DCT					WL_REAR_SWITCH		//王林音库后处理：处理mao1韵尾DCT模板基频不好			mdj
#define WL_REAR_DEAL_LI_SPEC					WL_REAR_SWITCH		//王林音库后处理：处理li韵尾的时长和谱的问题	时长不增加，修改部分状态的时长
#define WL_REAR_DEAL_IANG_LF0					WL_REAR_SWITCH		//王林音库后处理：处理iang4基频拐的问题			added by mdj 2010-03-15
#define WL_REAR_DEAL_AI3_DCT					WL_REAR_SWITCH		//王林音库后处理：处理ai3韵尾基频模板的问题		added by mdj 2012-03-19
#define WL_REAR_DEAL_TONE3_LINKED_ZHUOSM        WL_REAR_SWITCH       //王林音库后处理：处理三声后跟浊声母的轻声m/l/n的三声基频低的情况  -wangcm-2012-03-20
#define WL_REAR_DEAL_DCT_SHORT_SEN_LEN			WL_REAR_SWITCH	//<王林>中文短句处理模式(字母的不处理)：专门处理：单字句，两字句   added by mdj 2012-03-26
#define WL_REAR_DEAL_LIGHT_M_N                  WL_REAR_SWITCH       //王林音库后处理：处理韵尾轻声浊声母不好的情况    -wangcm-20120401
#define WL_REAR_DEAL_VOL_TOO_LARGE             WL_REAR_SWITCH       //王林音库后处理：处理个别音量超大的问题

	//目前不打开的  不需要的  不能加
	#define	WL_REAR_DEAL_LIGHT_TONE					0		//王林音库后处理：轻声			基频降10,语速快10%,能量降10%（效果不明显，一般不打开）
	#define	WL_REAR_DEAL_SHEN2_ME5					0		//王林音库后处理：专门处理“什么”
	#define	WL_REAR_DEAL_DUR_XIEXIE					0		//王林音库后处理：谢谢 (改了时长和能量)


//========================================================================================================================================
#define  HT_REAR_DEAL_PAU_ENERGY					(1)				//慧听库停顿能量置0
#define  HT_REAR_DEAL_DUR							(1)				//图灵库整体时长增大


//====================================  界面参数：一般不动 ==================================================================================

#define EM_INTERFACE_VQ							1		//界面参数：后端频谱参数是否量化（0：不量化；1：量化）  (目前虽然设置为1，但实际上不量化了)
#define EM_INTERFACE_IS_STRAIGHT				0		//界面参数：后端是否使用straight算法,改用wolrd提取参数
#define EM_INTERFACE_IS_LSP						1		//界面参数：后端模型是否是LSP模型（0：MGC；1：LSP）

//========================================================================================================================================




//====================================  基本固定不改的开关  ==================================================================================

#define  CHABO_INDEX							8			//第几句插播
#define  CHABO_EACH_COUNT						10			//每几句插播一次

#define	EM_PROCESS_CALLBACK_OPEN				1			//是否：打开进度回调

#define EM_USER_DECODER_ADPCM					0			//是否：支持 ADPCM 解码	 若资源里有ADPCM编码的声音，则打开，一般不开
#define EM_USER_DECODER_G7231					1			//是否：支持 G7231 解码  若资源里有G7231编码的声音，则打开

#define PPH_AFTER_DEAL							1			//是否：PPH边界划分后处理

#define EM_USER_ENCODER_ADPCM					0			//是否：支持 ADPCM 编码  一般不打开
#define EM_USER_ENCODER_G7231					0			//是否：支持 G7231 编码  一般不打开 

#define EM_USER_VOICE_RESAMPLE					1			//是否：支持重采样(例如：虚拟发音人的音色调整等)


#define UVP_DUR_MOD								0			//(不能加效果不好)清浊音转换模型开关：按状态时长方法调节（与UVP_FRAME_MOD开关互斥）
#define UVP_DUR_MOD_P							0			//(不能加效果不好)在UVP_DUR_MOD打开的基础上的二级开关：二级MSD判决：是否清音状态多1个

#define	EM_SYS_SWITCH_RHYTHM_NEW				1			//是否：用《韵律预测--新方案》  注意：老方案暂不通

#define EM_PARAM_MAX_SAMPLE						16000		//最大内部采样率
#define MAX_OUTPUT_PCM_SIZE						1200		//最大的播放输出PCM数据的缓冲区的大小： 必须是2的指数倍数  hyl 2012-10-15 播放设备的单个缓存必须小于这个值

//（注意：不再用“初始化加载22颗决策树内存”策略，不足以改善后端速度，因为读模型叶节点还是很慢）
#define EM_DECISION_HEAP						0				//是否支持决策树内存(必须将EM_DECISION_READ_ALL配置成1：读入整棵决策树)

#define USER_HEAP_MIN_CONFIG_ADD				(100<<10)		// 8K模型配置：LSP模型用户能多提供的内存量（适合内存空间富裕的客户，可在初始化时将后端22颗决策树全部载入到内存）
#define USER_HEAP_MAX_CONFIG_ADD				(170<<10)		//16K模型配置：LSP模型用户能多提供的内存量（适合内存空间富裕的客户，可在初始化时将后端22颗决策树全部载入到内存）


//========================================================================================================================================





//====================================  一般不用的开关,维持现状  ==================================================================================

#define SYL_DURATION							0		//音节时长模型开关（暂关）（较好，稳定性还没做大量测试，好像偶尔会有点杂音）（句首句尾的音节时长会加长，助词的时长会变短）		 changed by naxy19
#define	REAR_LSP_POSTFILTER						0		//后端：LSP后滤波开关   目前效果不好
#define UVP_FRAME_MOD							0		//清浊音转换模型开关：按帧调整（与UVP_DUR_MOD开关互斥）	(一般不用，供测试用)
#define EM_SYS_QING_ADD_ONE_STATUS				0		//是否：打开《强制某些拼音清音多1个状态》开关 -- 改善：《清浊判别不好》问题，目前会增加0.8K全局空间，需优化
														//此开关以后不用，已有UVP_DUR_MOD开关来改善清浊转换


#define REAR_RH_ADAPTION						0		//后端：锐化是否自适应开关	目前效果不好
#if REAR_RH_ADAPTION
	//后端：相当于后端LSP锐化开关（范围：0.5--1） （=1：不锐化 ； =0.7：锐化效果目前较好； 可调节此系数观察锐化的效果）
	#define EM_SYS_VAR_REAR_RH_PARA_L					0.8		//效果较好的系数：0.8
	#define EM_SYS_VAR_REAR_RH_PARA_M					0.7		//效果较好的系数：0.7
	#define EM_SYS_VAR_REAR_RH_PARA_H					0.8		//效果较好的系数：0.8
#endif
//========================================================================================================================================




//======================================== 《定义相关》 ==================================================================================

//后端资源读取相关：
#define Table_NODE_LEN_INF			8		//后端资源存储的：所有模型决策树每行的字节数
//#define PDF_NODE_FLOAT_COUNT_DUR	20		//后端资源存储的：状态时长模型每个叶节点的float个数
#define PDF_NODE_FLOAT_COUNT_SYD 	2		//后端资源存储的：音节时长模型每个叶节点的float个数 ,		changed by naxy19
#define PDF_NODE_FLOAT_COUNT_UVP	1		//后端资源存储的：清浊音转换点模型每个叶节点的float个数 ,	changed by naxy19
#define PDF_NODE_FLOAT_COUNT_LF0	7		//后端资源存储的：基频模型每个叶节点的float个数
#define PDF_NODE_FLOAT_COUNT_BAP	6		//后端资源存储的：bap模型每个叶节点的float个数
#define PDF_NODE_FLOAT_COUNT_MGC	(NULL)	//后端资源存储的：频谱模型每个叶节点的float个数（不能宏定义，取决于资源中读入的谱的维数，存储在：g_hTTS->pRearG->nMgcLeafPrmN）
//========================================================================================================================================








#endif	/* #define _EMTTS__EM_CONFIG__H_ */
