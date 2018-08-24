/* 标准串操作头文件 */

#ifndef _EMTTS__EM_STRING__H_
#define _EMTTS__EM_STRING__H_


#include "emCommon.h"


#ifdef __cplusplus
extern "C" {
#endif

//对比类型及其值 
typedef emInt16 emComp;
#define emGreater	(1)
#define emEqual		(0)
#define emLesser	(-1)
#define emIsGreater(v)	((v)>0)
#define emIsEqual(v)	(0==(v))

/*
 *	字符常量
 */

#define emChar_Tab			0x09
#define emChar_Space		0x20
#define emChar_SingleQuotes	0x27
#define emChar_Add			0x2B
#define emChar_Comma		0x2C
#define emChar_Sub			0x2D
#define emChar_Dot			0x2E

#define emChar_0	0x30
#define emChar_1	0x31
#define emChar_2	0x32
#define emChar_3	0x33
#define emChar_4	0x34
#define emChar_5	0x35
#define emChar_6	0x36
#define emChar_7	0x37
#define emChar_8	0x38
#define emChar_9	0x39


#define emChar_A	0x41
#define emChar_B	0x42
#define emChar_C	0x43
#define emChar_D	0x44
#define emChar_E	0x45
#define emChar_F	0x46
#define emChar_G	0x47
#define emChar_H	0x48
#define emChar_I	0x49
#define emChar_J	0x4A
#define emChar_K	0x4B
#define emChar_L	0x4C
#define emChar_M	0x4D
#define emChar_N	0x4E
#define emChar_O	0x4F
#define emChar_P	0x50
#define emChar_Q	0x51
#define emChar_R	0x52
#define emChar_S	0x53
#define emChar_T	0x54
#define emChar_U	0x55
#define emChar_V	0x56
#define emChar_W	0x57
#define emChar_X	0x58
#define emChar_Y	0x59
#define emChar_Z	0x5A

#define emChar_a	0x61
#define emChar_b	0x62
#define emChar_c	0x63
#define emChar_d	0x64
#define emChar_e	0x65
#define emChar_f	0x66
#define emChar_g	0x67
#define emChar_h	0x68
#define emChar_i	0x69
#define emChar_j	0x6A
#define emChar_k	0x6B
#define emChar_l	0x6C
#define emChar_m	0x6D
#define emChar_n	0x6E
#define emChar_o	0x6F
#define emChar_p	0x70
#define emChar_q	0x71
#define emChar_r	0x72
#define emChar_s	0x73
#define emChar_t	0x74
#define emChar_u	0x75
#define emChar_v	0x76
#define emChar_w	0x77
#define emChar_x	0x78
#define emChar_y	0x79
#define emChar_z	0x7A


#define EM_PARAM_SUBSTR_MAXLEN	256			/* 模式匹配中最大子串长度 */

/* 数字、字母字符判定 */
#define emIsDigit(c)	((c)>=emChar_0&&(c)<=emChar_9)
#define emIsAlpha(c)	(((c)>=emChar_A&&(c)<=emChar_Z)||((c)>=emChar_a&&(c)<=emChar_z))

/* 字母大小写转换(非字母输入无意义) */
#define emToLower(c)	((c)|0x20)
#define emToUpper(c)	((c)&~0x20)


/* 求相对大值、相对小值 */
#define emMax(a,b)  (((a)>(b))?(a):(b))
#define emMin(a,b)  (((a)<(b))?(a):(b))


emSize emCall emIntToStr( emInt32 nNum, emPUInt8 pStr );
emInt32 emCall emStrToInt( emPCUInt8 pStr, emSize nLen );

emPCUInt8 emCall emStrChar( emPCUInt8 pStr, emSize nLen, emUInt8 nChar );
emPCUInt8 emCall emStrStr( emPCUInt8 pStr, emSize nLen, emPCUInt8 pSubStr, emSize nSubLen );

emComp emCall emStrComp( emPCUInt8 s1, emSize n1, emPCUInt8 s2, emSize n2 );
emComp emCall emStrCompN( emPCUInt8 s1, emPCUInt8 s2, emSize n );
emComp emCall emStrCompNI( emPCUInt8 s1, emPCUInt8 s2, emSize n );

#if EM_ANSI_STRING
	#define emStrLenA(sz)	(emSize)strlen((emConst char*)sz)
	#define emStrLenW(sz)	(emSize)wcslen((const unsigned short*)sz)
	#define emStrLenW2(sz)	(emSize)wcslen((emConst wchar_t*)sz)
	
#else
	emSize emCall emStrLenA( emCStrA sz );
	emSize emCall emStrLenW( emCStrW sz );
#endif

#if EM_UNICODE
	#define emStrLen	emStrLenW
#else
	#define emStrLen	emStrLenA
#endif

#ifdef __cplusplus
}
#endif


#endif/*!_EMTTS__EM_STRING__H_*/
