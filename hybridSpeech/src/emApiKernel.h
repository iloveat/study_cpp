//emTTS内核头文件

#ifndef	_EMTTS__EM_API_KERNEL__H_
#define _EMTTS__EM_API_KERNEL__H_


#include "emApiDefine.h"

#ifdef  __cplusplus
extern "C" {
#endif


	typedef emUInt16 emTTSErrID;
	typedef emPointer emHTTS;


	//【接口函数相关】*************************************************************************

	//创建一个实例
	emTTSErrID emCall emTTS_Create(
		emHTTS emPtr	phTTS,							//输出：实例句柄 
		emPointer		pHeap,							//输入：实例需用的内存堆指针
		emSize			nHeapSize,						//输入：实例需用的内存堆大小
		emPointer		pCBParam,						//输入：用户回调指针
		emPResPackDesc	pResPackDesc,					//输入：资源包描述数组
		emSize			nResPackCount);					//输入：资源包个数


	//被创建的实例
	emTTSErrID emCall emTTS_Destroy(
		emHTTS			hTTS);							//输入：实例句柄

	//获取实例的参数
	emTTSErrID emCall emTTS_GetParam(
		emHTTS			hTTS,							//输入：实例句柄
		emUInt32		nParamID,						//输入：参数ID 
		emPUInt32		pnParamValue);					//输出：参数值 

	//设置实例的参数
	emTTSErrID emCall emTTS_SetParam(
		emHTTS			hTTS,							//输入：实例句柄
		emUInt32		nParamID,						//输入：参数ID
		emUInt32		nParamValue);					//输入：参数值

	//结束实例的TTS合成：堵塞中的emTTS_SynthText函数会立刻退出
	emTTSErrID emCall emTTS_Exit(
		emHTTS			hTTS);							//输入：实例句柄

	//在当前实例中合成一段文本：当前线程会被堵塞，直到合成结束
	emTTSErrID emCall emTTS_SynthText(
		emHTTS			hTTS,							//输入：实例句柄
		emCPointer		pcData,							//输入：待合成文本的数据buffer指针
		emSize			nSize);						//输入：待合成文本的数据大小

	//获取emTTS内核的版本
	emTTSErrID emCall emTTS_GetVersion(
		emPByte verStr);								//输出：版本号字符串




	//【返回的：错误码：描述】******************************************************************

#define emTTS_ERR_OK					0x0000						//成功 
#define emTTS_ERR_FAILED				0xFFFF						//失败 
#define emTTS_ERR_END_OF_INPUT			0x0001						//输入流结束 
#define emTTS_ERR_EXIT					0x0002						//退出TTS 

#define emTTS_ERR_BASE					0x8000						//基本错误码数 
#define emTTS_ERR_INSUFFICIENT_HEAP		(emTTS_ERR_BASE + 1)		//堆空间不足  
#define emTTS_ERR_RESOURCE				(emTTS_ERR_BASE + 2)		//资源出错 




	//【实例：参数码：描述】*********************************************************************


#define emTTS_PARAM_OUTPUT_CALLBACK		0x00000101			//输出回调参数 
#define emTTS_PARAM_PROGRESS_CALLBACK	0x00000102			//处理进度通知回调函数参数 

#define emTTS_PARAM_INPUT_CODEPAGE		0x00000201			//输入文本代码页参数 

#define emTTS_PARAM_USE_PROMPTS			0x00000301			//[x*]：提示音参数 
#define emTTS_PARAM_READ_DIGIT			0x00000302			//[n*]：数字读法参数  
#define emTTS_PARAM_CHINESE_NUMBER_1	0x00000303			//[y*]：中文号码“1”的读法参数 
#define emTTS_PARAM_SPEAK_STYLE			0x00000304			//[f*]：发音风格参数 
#define emTTS_PARAM_VOICE_SPEED			0x00000305			//[s*]：语速参数 
#define emTTS_PARAM_VOLUME				0x00000306			//[v*]：音量参数 
#define emTTS_PARAM_VOICE_PITCH			0x00000308			//[t*]：语调参数 
#define emTTS_PARAM_PUNCTUATION			0x00000307			//[b*]：读标点参数 

#define emTTS_PARAM_ROLE				0x00000309			//[m*]：发音人参数				
#define emTTS_PARAM_ManualRhythm		0x0000030a			//[z*]：韵律策略参数 
#define emTTS_PARAM_PINYIN				0x0000030b			//[i*]：识别拼音参数 
#define emTTS_PARAM_XINGSHI				0x0000030c			//[r*]：姓氏参数 

#define emTTS_PARAM_LANG				0x0000030d			//[g*]：语种
#define	emTTS_PARAM_ZERO				0x0000030e			//[o*]：英文0的读法






	// 【代码页相关】*****************************************************************************
#define emTTS_CODEPAGE_UTF8             1                     //文本代码页：UTF-8
#define emTTS_CODEPAGE_GBK				936						//文本代码页：GBK (默认) 
#define emTTS_CODEPAGE_GB2312			937						//文本代码页：GB2312 
#define emTTS_CODEPAGE_BIG5				950						//文本代码页：Big5 
#define emTTS_CODEPAGE_UTF16LE			1200					//文本代码页：UTF-16 little-endian 
#define emTTS_CODEPAGE_UTF16BE			1201					//文本代码页：UTF-16 big-endian 
#define emTTS_CODEPAGE_UNICODE			emTTS_CODEPAGE_UTF16
#if EM_BIG_ENDIAN
#define emTTS_CODEPAGE_UTF16		emTTS_CODEPAGE_UTF16BE
#else
#define emTTS_CODEPAGE_UTF16		emTTS_CODEPAGE_UTF16LE
#endif




	// 【控制标记：常量：描述】*******************************************************************

#define emTTS_READDIGIT_AUTO				0			//数字读法：自动（默认） 
#define emTTS_READDIGIT_AS_NUMBER			1			//数字读法：按号码 
#define emTTS_READDIGIT_AS_VALUE			2			//数字读法：按数值 

#define emTTS_CHNUM1_READ_YAO				0			//号码“1”：读成“幺” (默认) 
#define emTTS_CHNUM1_READ_YI				1			//号码“1”：读成“一” 

#define emTTS_STYLE_WORD					0			//发音风格：word-by-word风格 
#define emTTS_STYLE_NORMAL					1			//发音风格：正常自然风格 (默认) 

#define emTTS_PITCH_MIN						0			//语调：最低值 
#define emTTS_PITCH_NORMAL					5			//语调：正常值5（默认） 
#define emTTS_PITCH_MAX						10			//语调：最高值 

#define emTTS_SPEED_MIN						0			//语速：最慢值 
#define emTTS_SPEED_NORMAL					5			//语速：正常值5（默认） 
#define emTTS_SPEED_MAX						10			//语速：最快值 

#define emTTS_VOLUME_MIN					0			//音量：最小值 
#define emTTS_VOLUME_NORMAL					5			//音量：正常值10（默认） 
#define emTTS_VOLUME_MAX					10			//音量：最大值 

#define emTTS_PUNCTUATION_NO_READ			0			//读标点：不读(默认) 
#define emTTS_PUNCTUATION_READ				1			//读标点：读  

#define emTTS_USE_PROMPTS_CLOSE				0			//提示音: 不识别 
#define emTTS_USE_PROMPTS_OPEN				1			//提示音: 识别（默认） 

#define emTTS_USE_XINGSHI_CLOSE				0			//姓氏开关: 关闭 （默认）
#define emTTS_USE_XINGSHI_JUSHOU_OPEN		1			//姓氏开关: 以后每句的句首强制读成姓氏 ：打开
#define emTTS_USE_XINGSHI_AFTER_OPEN		2			//姓氏开关: 仅紧跟句的句首强制读成姓氏 ：打开

#define emTTS_USE_PINYIN_CLOSE				0			//识别汉语拼音开关: 关闭（默认）
#define emTTS_USE_PINYIN_OPEN				1			//识别汉语拼音开关: 开

#define	emTTS_USE_ROLE_XIAOLIN				3			//发音人：晓玲    （女声）（默认）
#define	emTTS_USE_ROLE_Virtual_51			51			//发音人：尹小坚  （男声）
#define	emTTS_USE_ROLE_Virtual_52			52			//发音人：易小强  （男声）
#define	emTTS_USE_ROLE_Virtual_53			53			//发音人：田蓓蓓  （女声）
#define	emTTS_USE_ROLE_Virtual_54			54			//发音人：唐老鸭  （效果器）
#define	emTTS_USE_ROLE_Virtual_55			55			//发音人：小燕子  （女童声）

#define emTTS_USE_Manual_Rhythm_CLOSE		0			//关闭韵律标识 处理策略（默认）
#define emTTS_USE_Manual_Rhythm_OPEN		1			//开启韵律标识 处理策略

#define emTTS_LANG_AUTO						0			//语种：自动识别（默认）
#define emTTS_LANG_CN						1			//语种：汉语
#define emTTS_LANG_EN						2			//语种：英文

#define emTTS_EN_0_OU						0			//英文0的读法：读成“ou”
#define emTTS_EN_0_ZERO						1			//英文0的读法：读成“zero”（默认）

	//Zz	20130313
#define emTTS_EN_TIME_NORM					0			//英文时间读法：普通	按数值依次读（默认）
#define emTTS_EN_TIME_SPEC					1			//英文时间读法：特别	按口语形式读




	// 【PCM数据采样率：常量：描述】*****************************************************************

#define emTTS_CODE_PCM_8K					8000		//PCM数据：8K ，16bit，单通道 
#define emTTS_CODE_PCM_11K					11025		//PCM数据：11K，16bit，单通道
#define emTTS_CODE_PCM_16K					16000		//PCM数据：16K，16bit，单通道



#ifdef __cplusplus
}
#endif

#endif	//#define _EMTTS__EM_API_KERNEL__H_
