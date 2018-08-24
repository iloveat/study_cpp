/* 算术运算头文件 */

#ifndef _EMTTS__EM_MATH__H_
#define _EMTTS__EM_MATH__H_


#include "emCommon.h"


#ifdef __cplusplus
extern "C" {
#endif



#define emAbs(n1)	( (n1 >= 0L ) ? (n1) : (-(n1)) )
#define emFabs(f1)	( (f1 >= 0.0 ) ? (f1) : (-(f1)) )


/*
 *	浮点值修饰前缀
 */

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
#define Q63


/*
 *	浮点常量
 */

#define FLOAT_Q(x)	(1<<(x))
#define DOUBLE_Q(x)	(1L<<(x))

#define FLOAT_Q0	0x0001
#define FLOAT_Q1	0x0002
#define FLOAT_Q2	0x0004
#define FLOAT_Q3	0x0008
#define FLOAT_Q4	0x0010
#define FLOAT_Q5	0x0020
#define FLOAT_Q6	0x0040
#define FLOAT_Q7	0x0080
#define FLOAT_Q8	0x0100
#define FLOAT_Q9	0x0200
#define FLOAT_Q10	0x0400
#define FLOAT_Q11	0x0800
#define FLOAT_Q12	0x1000
#define FLOAT_Q13	0x2000
#define FLOAT_Q14	0x4000
#define FLOAT_Q15	0x7FFF

#define DOUBLE_Q15	0x00008000L
#define DOUBLE_Q16	0x00010000L
#define DOUBLE_Q17	0x00020000L
#define DOUBLE_Q18	0x00040000L
#define DOUBLE_Q19	0x00080000L
#define DOUBLE_Q20	0x00100000L
#define DOUBLE_Q21	0x00200000L
#define DOUBLE_Q22	0x00400000L
#define DOUBLE_Q23	0x00800000L
#define DOUBLE_Q24	0x01000000L
#define DOUBLE_Q25	0x02000000L
#define DOUBLE_Q26	0x04000000L
#define DOUBLE_Q27	0x08000000L
#define DOUBLE_Q28	0x10000000L
#define DOUBLE_Q29	0x20000000L
#define DOUBLE_Q30	0x40000000L
#define DOUBLE_Q31	0x7FFFFFFFL


/*
 *	浮点值类型
 */

typedef struct tagMathStruct emTMathStruct, emPtr emPMathStruct;

struct tagMathStruct
{
	emBool	m_bOverflow;
	emBool	m_bCarry;
};

#define EM_MATH_STRUCT emTMathStruct tMathStruct = { 0 }; emPMathStruct pIsMathStruct = &tMathStruct;


/*
 *	浮点操作
 */

/* 调整浮点值的精度 */
#define emFloat_ShiftUp(f,n)	((f)<<(n))
#define emFloat_ShiftDown(f,n)	(((f)+DOUBLE_Q((n)-1))>>(n))
#define emFloat_Shift(f,q0,q1)	(((q0)<=(q1))?emFloat_ShiftUp(f,(q1)-(q0)):emFloat_ShiftDown(f,(q0)-(q1)))


/*
 *	浮点算术运算
 *
 *	L_	- 类型为Long(默认为Short)
 *	i_	- 整型运算
 *	_s	- 输入为Short
 *	_l	- 输入为Long
 *	_r	- with rounding
 *	_c	- with carry
 */

/* 饱和 */
Q15 emInt16 emCall emMath_saturate_o(emPMathStruct pIsMathStruct, Q31 emInt32 L_var1);
#define saturate_o(L_var1)			emMath_saturate_o(pIsMathStruct,L_var1)
Q15 emInt16 emCall saturate(Q31 emInt32 L_var1);
Q31 emInt32 emCall emMath_L_sat(emPMathStruct pIsMathStruct, Q31 emInt32 L_var1);
#define L_sat(L_var1)				emMath_L_sat(pIsMathStruct,L_var1)

/* 四舍五入 */
#define emRound( var, q )			( ((var) + (1<<(q-1))) >> q)

/* 加,乘加 */
#define add(var1,var2)				(Q15 emInt16)((var1)+(var2))
#define add_1(var1,var2)			saturate((Q31 emInt32)(var1)+(var2))
#define L_add(L_var1,L_var2)		((Q31 emInt32)(L_var1)+(L_var2))
Q31 emInt32 emCall L_add_1(Q31 emInt32 L_var1, Q31 emInt32 L_var2);
#define L_mac(L_var3,var1,var2)		L_add(L_var3,L_mult(var1,var2))
#define L_mac_1(L_var3,var1,var2)	L_add_1(L_var3,L_mult(var1,var2))

/* 减,乘减 */
#define sub(var1,var2)				(Q15 emInt16)((var1)-(var2))
#define sub_1(var1,var2)			saturate((Q31 emInt32)(var1)-(var2))
#define L_sub(L_var1,L_var2)		((Q31 emInt32)(L_var1)-(L_var2))
Q31 emInt32 emCall L_sub_1(Q31 emInt32 L_var1, Q31 emInt32 L_var2);
#define L_msu(L_var3,var1,var2)		L_sub(L_var3,L_mult(var1,var2))
#define L_msu_1(L_var3,var1,var2)	L_sub_1(L_var3,L_mult(var1,var2))

/* 乘 */
#define mult(var1,var2)				(Q15 emInt16)(((Q31 emInt32)(var1)*(var2))>>15)
#define mult_1(var1,var2)			((EM_MIN_INT16==(var1)&&EM_MIN_INT16==(var2))?EM_MAX_INT16:mult(var1,var2))
#define mult_r(var1,var2)			(Q15 emInt16)(((Q31 emInt32)(var1)*(var2)+FLOAT_Q14)>>15)
#define mult_r_1(var1,var2)			((EM_MIN_INT16==(var1)&&EM_MIN_INT16==(var2))?EM_MAX_INT16:mult_r(var1,var2))
#define L_mult(var1,var2)			(((Q31 emInt32)(var1)*(var2))<<1)
#define L_mult_1(var1,var2)			((EM_MIN_INT16==(var1)&&EM_MIN_INT16==(var2))?EM_MAX_INT32:L_mult(var1,var2))
#ifdef EM_TYPE_INT64
#define L_mls(L_var1,var2)			(Q31 emInt32)(((Q63 emInt64)(L_var1)*(var2) + 16384L)>>15)
#else
#define L_mls(L_var1,var2)			L_mac((((L_var1)&0xFFFF)*(var2) + 16384L)>>15,extract_h(L_var1),var2)
/*#define L_mls(L_var1,var2)			L_mac((((L_var1)&0xFFFF)*(var2))>>15,extract_h(L_var1),var2)*/
#endif
#define L_mls_1(L_var1,var2)		((EM_MIN_INT32==(L_var1)&&EM_MIN_INT16==(var2))?EM_MAX_INT32:L_mls(L_var1,var2))
#define i_mult(a,b)					((a)*(b))

/* 除 */
#define div_s(var1,var2)			(((var1)>=(var2))?EM_MAX_INT16:(Q15 emInt16)(((Q31 emInt32)(var1)<<15)/(var2)))
#define div_l(L_var1,var2)			((((L_var1)>>16)>=(var2))?EM_MAX_INT16:(Q15 emInt16)(((L_var1)/(var2))>>1))

/* 左移 */
Q15 emInt16 emCall emMath_shl(Q15 emInt16 var1, Q15 emInt16 var2);
Q31 emInt32 emCall emMath_L_shl(Q31 emInt32 L_var1, Q15 emInt16 var2);
#define shl(var1,var2)				(((var2)>0)?((Q15 emInt16)(var1)<<(var2)):((var1)>>-(var2)))
#define shl_1(var1,var2)			(((var2)>0)?emMath_shl(var1,var2):((var1)>>-(var2)))
#define L_shl(L_var1,var2)			(((var2)>0)?((L_var1)<<(var2)):((L_var1)>>-(var2)))
#define L_shl_1(L_var1,var2)		(((var2)>0)?emMath_L_shl(L_var1,var2):((L_var1)>>-(var2)))

/* 右移 */
#define shr(var1,var2)				(((var2)<0)?(Q15 emInt16)((var1)<<-(var2)):((var1)>>(var2)))
#define shr_1(var1,var2)			(((var2)<0)?emMath_shl(var1,(Q15 emInt16)-(var2)):((var1)>>(var2)))
#define shr_2(var1,var2)			(((var1)<0)?(~((~(var1))>>(var2))):((var1)>>(var2)))
#define shr_r(var1,var2)			(((var2)>0)?(((var1)+DOUBLE_Q((var2)-1))>>(var2)):((var1)<<-(var2)))
#define L_shr(L_var1,var2)			(((var2)<0)?((L_var1)<<-(var2)):((L_var1)>>(var2)))
#define L_shr_1(L_var1,var2)		(((var2)<0)?emMath_L_shl(L_var1,(Q15 emInt16)-(var2)):((L_var1)>>(var2)))
#define L_shr_2(L_var1,var2)		(((L_var1)<0)?(~((~(L_var1))>>(var2))):((L_var1)>>(var2)))
#define L_shr_r(L_var1,var2)		(((var2)>0)?(((L_var1)+DOUBLE_Q((var2)-1))>>(var2)):((L_var1)<<-(var2)))

/* 双精度值四舍五入到单精度值 */
#define round(L_var1)				extract_h(L_add(L_var1,DOUBLE_Q15))
#define round_1(L_var1)				extract_h(L_add_1(L_var1,DOUBLE_Q15))

/* 取负 */
#define negate(var1)				(Q15 emInt16)(((var1)==EM_MIN_INT16)?EM_MAX_INT16:(Q15 emInt16)-(var1))
#define L_negate(L_var1)			(Q31 emInt32)(((L_var1)==EM_MIN_INT32)?EM_MAX_INT32:(Q31 emInt32)-(L_var1))

/* 取绝对值 */
#define abs_ss(var1)				(((var1)>=0)?(var1):-(var1))
#define abs_s(var1)					(Q15 emInt16)(((var1)==EM_MIN_INT16)?EM_MAX_INT16:(Q15 emInt16)abs_ss(var1))
#define L_abs(L_var1)				(Q31 emInt32)(((L_var1)==EM_MIN_INT32)?EM_MAX_INT32:(Q31 emInt32)abs_ss(L_var1))

/* 求归1化因子 */
emUInt8 emCall norm_s(Q15 emInt16 var1);
emUInt8 emCall norm_l(Q31 emInt32 L_var1);

/* 生成双精度值 */
#define L_deposit_h(var1)			((Q31 emInt32)(var1)<<16)
#define L_deposit_l(var1)			(Q31 emInt32)(var1)

/* 拆分双精度值 */
#define extract_h(L_var1)			(Q15 emInt16)((L_var1)>>16)
#define extract_l(L_var1)			(Q15 emInt16)((L_var1)&0x0000FFFFL)
#define L_Extract(L_var1,var2,var3)	{*(var2)=extract_h(L_var1);*(var3)=extract_l(L_sub((L_var1)>>1,(L_deposit_l(*(var2))<<15)));}

/* 双精度数据间的运算 */
#define L_Comp(var1,var2)			L_mac(L_deposit_h(var1),(var2),1)

/* 双精度乘法 */
#define Mpy_32(var1,var2,var3,var4)	L_mac(L_mac(L_mult(var1,var3),mult(var1,var4),1),mult(var2,var3),1)
#define Mpy_32_16(var1,var2,var3)	L_mac(L_mult(var1,var3),mult(var2,var3),1)
//Q31 emInt32 emCall Div_32(Q31 emInt32 L_num, Q15 emInt16 denom_hi, Q15 emInt16 denom_lo);

/* 查表粗略求三角函数: 输入为Q16量化的周期数(仅支持前半个周期) */
#define F_cos_s(i)	g_nCosTable[((i)>>7)&0x00FF]
#define F_sin_s(i)	F_cos_s(0x4000-(i))

/* 查表并线性插值求三角函数: 输入为Q16量化的周期数 */
Q15 emInt16 emCall F_cos(Q16 emUInt16 i);
#define F_sin(i)	F_cos(0x4000-(i))

#ifdef __cplusplus
}
#endif


#endif/*!_EMTTS__EM_MATH__H_*/
