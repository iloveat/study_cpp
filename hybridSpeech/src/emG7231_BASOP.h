
#include "emMath.h"

#if 0

#ifndef	_EMTTS__EM_G7231_BASOP__H_
#define _EMTTS__EM_G7231_BASOP__H_

/*___________________________________________________________________________
 |                                                                           |
 |                                               |
 |                                                                           |
 | $Id $
 |___________________________________________________________________________|
*/
/*#define CHKOF*/
#if defined(CHKOF)
emExtern emBool Overflow;
emExtern emBool Carry;
emExtern emBool Error;
#endif

#define EM_MAX_INT32 (Q31 emInt32)0x7fffffffL
#define EM_MIN_INT32 (Q31 emInt32)0x80000000L

#define EM_MAX_INT16 ((Q15 emInt16)+32767)	/* 0x7fff */
#define EM_MIN_INT16 ((Q15 emInt16)-32768)	/* 0x8000 */

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
#ifdef CHKOF

Q15 emInt16 add (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short add,           1   */
Q31 emInt32 L_add (Q31 emInt32 L_var1, Q31 emInt32 L_var2);    /* Long add,        2 */
Q15 emInt16 round (Q31 emInt32 L_var1);             /* Round,               1   */

Q15 emInt16 sub (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short sub,           1   */
Q31 emInt32 L_sub (Q31 emInt32 L_var1, Q31 emInt32 L_var2);    /* Long sub,        2 */


Q15 emInt16 shl (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short shift left,    1   */
Q31 emInt32 L_shl (Q31 emInt32 L_var1, Q15 emInt16 var2);      /* Long shift left, 2 */

#ifdef HALF_OPT

#define L_msu(L_var3,var1,var2)     L_sub(L_var3,L_mult(var1,var2))
#define L_mac(L_var3,var1,var2)     L_add(L_var3,L_mult(var1,var2))
#define shr(var1,var2)				((var2 > 0) ? (var1 >> var2) : (var1 << (-(var2))))
#define L_shr(L_var1,L_var2)        ((L_var2 > 0) ? (L_var1 >> L_var2) : (L_var1 << (-(L_var2))))
#define mult(var1,var2)				(Q15 emInt16)(((((Q31 emInt32)var1) * ((Q31 emInt32)var2)) & (Q31 emInt32)0xffff8000) >> 15)
#define L_mult(var1,var2)			(Q31 emInt32)(((Q31 emInt32)var1 * (Q31 emInt32)var2) << 1) 
#define mult_r(var1,var2)			(Q15 emInt16)((((((Q31 emInt32)var1) * ((Q31 emInt32)var2)) + (Q31 emInt32)0x00004000L) & (Q31 emInt32)0xffff8000) >> 15)

#else

Q31 emInt32 L_msu (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2);   /* Msu,  1  */
Q31 emInt32 L_mac (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2);   /* Mac,  1  */
Q15 emInt16 shr (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short shift right,   1   */
Q31 emInt32 L_shr (Q31 emInt32 L_var1, Q15 emInt16 var2);      /* Long shift right, 2*/
Q15 emInt16 mult (Q15 emInt16 var1, Q15 emInt16 var2);   /* Short mult,          1   */
Q31 emInt32 L_mult (Q15 emInt16 var1, Q15 emInt16 var2); /* Long mult,           1   */
Q15 emInt16 mult_r (Q15 emInt16 var1, Q15 emInt16 var2);       /* Mult with round, 2 */

#endif


#else/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define add(var1 , var2)			(Q15 emInt16)((var1)+(var2))
#define L_add(L_var1,L_var2)		((Q31 emInt32)(L_var1)+(Q31 emInt32)(L_var2))
#define L_mac(L_var3,var1,var2)     L_add(L_var3,L_mult(var1,var2))

#define sub(var1 , var2)			(Q15 emInt16)((var1)-(var2))
#define L_sub(L_var1,L_var2)        ((Q31 emInt32)(L_var1)-(Q31 emInt32)(L_var2))
#define L_msu(L_var3,var1,var2)     L_sub(L_var3,L_mult(var1,var2))

#define mult(var1,var2)				(Q15 emInt16)(((Q31 emInt32)(var1)*(Q31 emInt32)(var2))>>15)
#define L_mult(var1,var2)			(Q31 emInt32)(((Q31 emInt32)(var1)*(Q31 emInt32)(var2))<<1)
#define mult_r(var1,var2)			(Q15 emInt16)(((Q31 emInt32)(var1)*(Q31 emInt32)(var2)+0x00004000L)>>15)

#define shl(var1,var2)			    (Q15 emInt16)(((var2)>0)?((var1)<<(var2)):((var1)>>-(var2)))
#define L_shl(L_var1,L_var2)        (Q31 emInt32)(((L_var2)>0)?((L_var1)<<(L_var2)):((L_var1)>>-(L_var2)))
#define shr(var1,var2)				(Q15 emInt16)(((var2)>0)?((var1)>>(var2)):((var1)<<-(var2)))
#define L_shr(L_var1,L_var2)        (Q31 emInt32)(((L_var2)>0)?((L_var1)>>(L_var2)):((L_var1)<<-(L_var2)))
#define round(L_var1)				extract_h(L_add(L_var1,0x00008000L))
/*#define round(L_var1)				round_1(L_var1)*/

#endif
/********************************************************************************/
Q15 emInt16 add_1 (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short add,           1   */
Q31 emInt32 L_add_1 (Q31 emInt32 L_var1, Q31 emInt32 L_var2);    /* Long add,        2 */
/*Q31 emInt32 L_mac_1 (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2);   / * Mac,  1  * / */
#define L_mac_1(L_var3,var1,var2)     L_add_1(L_var3,L_mult(var1,var2))

Q15 emInt16 sub_1 (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short sub,           1   */
Q31 emInt32 L_sub_1 (Q31 emInt32 L_var1, Q31 emInt32 L_var2);    /* Long sub,        2 */
Q31 emInt32 L_msu_1 (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2);   /* Msu,  1  */

Q15 emInt16 mult_1 (Q15 emInt16 var1, Q15 emInt16 var2);   /* Short mult,          1   */
Q31 emInt32 L_mult_1 (Q15 emInt16 var1, Q15 emInt16 var2); /* Long mult,           1   */
Q15 emInt16 mult_r_1 (Q15 emInt16 var1, Q15 emInt16 var2);       /* Mult with round, 2 */

Q15 emInt16 shl_1 (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short shift left,    1   */
Q31 emInt32 L_shl_1 (Q31 emInt32 L_var1, Q15 emInt16 var2);      /* Long shift left, 2 */

/* 在程序中有大部分的右移操作的var2参数都是在(0,15)的数所以有针对性的用shr_2和L_shr_2
来代替shr_1和L_shr_1可以达到优化效果也不会产生错误. */
Q15 emInt16 shr_1 (Q15 emInt16 var1, Q15 emInt16 var2);    /* Short shift right,   1   */
#define shr_2(var1,var2)  (Q15 emInt16)((var1 < 0)?(~((~var1) >> var2)):(var1 >> var2))
Q31 emInt32 L_shr_1 (Q31 emInt32 L_var1, Q15 emInt16 var2);      /* Long shift right, 2*/
#define L_shr_2(L_var1,L_var2)   (Q31 emInt32)((L_var1 < 0)?(~((~L_var1) >> L_var2)):(L_var1 >> L_var2))

#define round_1(L_var1)                extract_h (L_add_1 (L_var1, (Q31 emInt32) 0x00008000L))
/*Q15 emInt16 round_1 (Q31 emInt32 L_var1);             / * Round,               1   * / */
/********************************************************************************/
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/
Q31 emInt32 L_shr_r (Q31 emInt32 L_var1, Q15 emInt16 var2); /* Long shift right with round,  3             */
Q15 emInt16 norm_s (Q15 emInt16 var1);             /* Short norm,           15  */
Q15 emInt16 div_s (Q15 emInt16 var1, Q15 emInt16 var2); /* Short division,       18  */
Q15 emInt16 norm_l (Q31 emInt32 L_var1);           /* Long norm,            30  */
Q15 emInt16 div_l( Q31 emInt32  L_num, Q15 emInt16 den );
#define i_mult(a,b)              (a*b)
/*Q15 emInt16 i_mult(Q15 emInt16 a, Q15 emInt16 b);*/
#define L_mls(Lv, v)  L_mac( L_shr( (Lv & (Q31 emInt32) 0x0000ffff) * (Q31 emInt32) v , (Q15 emInt16) 15 ), v, extract_h(Lv) )
Q31 emInt32 L_mls_1( Q31 emInt32, Q15 emInt16 ) ;        /* Wght ?? */
#define  shr_r(var1,var2)   shr(var1,var2)
/* the following functions are not used in this Codec. */
/*
Q15 emInt16 shr_r (Q15 emInt16 var1, Q15 emInt16 var2);        / * Shift right with  round, 2           * /
Q31 emInt32 L_macNs (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2); / * Mac without sat, 1   * /
Q31 emInt32 L_msuNs (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2); / * Msu without sat, 1   * /
Q31 emInt32 L_add_c (Q31 emInt32 L_var1, Q31 emInt32 L_var2);  / * Long add with c, 2 * /
Q31 emInt32 L_sub_c (Q31 emInt32 L_var1, Q31 emInt32 L_var2);  / * Long sub with c, 2 * /
Q15 emInt16 mac_r (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2); / * Mac with rounding,2 * /
Q15 emInt16 msu_r (Q31 emInt32 L_var3, Q15 emInt16 var1, Q15 emInt16 var2); / * Msu with rounding,2 * /
Q31 emInt32 L_sat (Q31 emInt32 L_var1);            / * Long saturation,       4  * /
*/
/*--------------------------------------------------------------------------*/
/*
Q31 emInt32 L_deposit_h (Q15 emInt16 var1);        / * 16 bit var1 -> MSB,     2 * /
Q31 emInt32 L_deposit_l (Q15 emInt16 var1);        / * 16 bit var1 -> LSB,     2 * /
Q15 emInt16 extract_h (Q31 emInt32 L_var1);         / * Extract high,        1   * /
Q15 emInt16 extract_l (Q31 emInt32 L_var1);         / * Extract low,         1   * /
Q15 emInt16 abs_s (Q15 emInt16 var1);               / * Short abs,           1   * /
Q31 emInt32 L_abs (Q31 emInt32 L_var1);            / * Long abs,              3  * /
Q15 emInt16 negate (Q15 emInt16 var1);              / * Short negate,        1   * /
Q31 emInt32 L_negate (Q31 emInt32 L_var1);                / * Long negate,     2 * /
*/
/*--------------------------------------------------------------------------*/
/* 程序中不会出现溢出的地方，用下面的宏代替相应的函数 */
/*****************************************************************/
/* 不用进行溢出处理部分 */
#define L_deposit_h(var1)			((Q31 emInt32)(var1)<<16)
#define L_deposit_l(var1)			(Q31 emInt32)(var1)
#define extract_h(L_var1)			(Q15 emInt16)((L_var1)>>16)
#define extract_l(L_var1)			(Q15 emInt16)(L_var1)
#define abs_s(var1)			    	(Q15 emInt16)(((var1)==EM_MIN_INT16)?EM_MAX_INT16:((var1)<0?(-var1):(var1)))
#define L_abs(L_var1)               (Q31 emInt32)(((L_var1)==EM_MIN_INT32)?EM_MAX_INT32:(((L_var1)<0)?-(L_var1):(L_var1)))
#define negate(var1)				(Q15 emInt16)(((var1)==EM_MIN_INT16)?EM_MAX_INT16:-(var1))
#define L_negate(L_var1)            (Q31 emInt32)(((L_var1)==EM_MIN_INT32)?EM_MAX_INT32:-(L_var1))
/*****************************************************************/

#endif/*!_EMTTS__EM_G7231_BASOP__H_*/

#endif
