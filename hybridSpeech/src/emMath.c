/* 算术运算源文件 */

#include "emPCH.h"
#include "emCommon.h"
#include "emMath.h"


#define Overflow	(pIsMathStruct->m_bOverflow)
#define Carry		(pIsMathStruct->m_bCarry)

#define CLO		(Overflow = emFalse)
#define STO		(Overflow = emTrue)

#define CLC		(Carry = emFalse)
#define STC		(Carry = emTrue)

/* cos 函数表 */
emStatic emConst Q15 emInt16 g_nCosTable[257] =
{
	32767,	32766,	32758,	32746,	32729,	32706,	32679,	32647,
	32610,	32568,	32522,	32470,	32413,	32352,	32286,	32214,
	32138,	32058,	31972,	31881,	31786,	31686,	31581,	31471,
	31357,	31238,	31114,	30986,	30853,	30715,	30572,	30425,
	30274,	30118,	29957,	29792,	29622,	29448,	29269,	29086,
	28899,	28707,	28511,	28311,	28106,	27897,	27684,	27467,
	27246,	27020,	26791,	26557,	26320,	26078,	25833,	25583,
	25330,	25073,	24812,	24548,	24279,	24008,	23732,	23453,
	23170,	22884,	22595,	22302,	22006,	21706,	21403,	21097,
	20788,	20475,	20160,	19841,	19520,	19195,	18868,	18538,
	18205,	17869,	17531,	17190,	16846,	16500,	16151,	15800,
	15447,	15091,	14733,	14373,	14010,	13646,	13279,	12910,
	12540,	12167,	11793,	11417,	11039,	10660,	10279,	9896,
	9512,	9127,	8740,	8351,	7962,	7571,	7180,	6787,
	6393,	5998,	5602,	5205,	4808,	4410,	4011,	3612,
	3212,	2811,	2411,	2009,	1608,	1206,	804,	402,
	0,		-402,	-804,	-1206,	-1608,	-2009,	-2411,	-2811,
	-3212,	-3612,	-4011,	-4410,	-4808,	-5205,	-5602,	-5998,
	-6393,	-6787,	-7180,	-7571,	-7962,	-8351,	-8740,	-9127,
	-9512,	-9896,	-10279,	-10660,	-11039,	-11417,	-11793,	-12167,
	-12540,	-12910,	-13279,	-13646,	-14010,	-14373,	-14733,	-15091,
	-15447,	-15800,	-16151,	-16500,	-16846,	-17190,	-17531,	-17869,
	-18205,	-18538,	-18868,	-19195,	-19520,	-19841,	-20160,	-20475,
	-20788,	-21097,	-21403,	-21706,	-22006,	-22302,	-22595,	-22884,
	-23170,	-23453,	-23732,	-24008,	-24279,	-24548,	-24812,	-25073,
	-25330,	-25583,	-25833,	-26078,	-26320,	-26557,	-26791,	-27020,
	-27246,	-27467,	-27684,	-27897,	-28106,	-28311,	-28511,	-28707,
	-28899,	-29086,	-29269,	-29448,	-29622,	-29792,	-29957,	-30118,
	-30274,	-30425,	-30572,	-30715,	-30853,	-30986,	-31114,	-31238,
	-31357,	-31471,	-31581,	-31686,	-31786,	-31881,	-31972,	-32058,
	-32138,	-32214,	-32286,	-32352,	-32413,	-32470,	-32522,	-32568,
	-32610,	-32647,	-32679,	-32706,	-32729,	-32746,	-32758,	-32766,
	-32768
};


/* 饱和 */
Q15 emInt16 emCall emMath_saturate_o(emPMathStruct pIsMathStruct, Q31 emInt32 L_var1)
{

	LOG_StackAddr(__FUNCTION__);

	if ( L_var1 > EM_MAX_INT16 )
	{
		STO;
		return EM_MAX_INT16;
	}

	if ( L_var1 < EM_MIN_INT16 )
	{
		STO;
		return EM_MIN_INT16;
	}

	return (emInt16)L_var1;
}

Q15 emInt16 emCall saturate(Q31 emInt32 L_var1)
{

	LOG_StackAddr(__FUNCTION__);

	if ( L_var1 > EM_MAX_INT16 )
		return EM_MAX_INT16;

	if ( L_var1 < EM_MIN_INT16 )
		return EM_MIN_INT16;

	return (Q15 emInt16)L_var1;
}

Q31 emInt32 emCall emMath_L_sat(emPMathStruct pIsMathStruct, Q31 emInt32 L_var1)
{

	LOG_StackAddr(__FUNCTION__);

	if ( !Overflow )
		return L_var1;

	CLC;
	CLO;

	return Carry ? EM_MIN_INT32 : EM_MAX_INT32;
}

/* 双精度浮点值加法 */
Q31 emInt32 emCall L_add_1(Q31 emInt32 L_var1, Q31 emInt32 L_var2)
{
#ifdef EM_TYPE_INT64
	Q63 emInt64 H_result;

	LOG_StackAddr(__FUNCTION__);

	H_result = L_var1;
	H_result += L_var2;

	if ( H_result > EM_MAX_INT32 )
		return EM_MAX_INT32;

	if ( H_result < EM_MIN_INT32 )
		return EM_MIN_INT32;

	return (Q31 emInt32)H_result;
#else
	Q31 emInt32 L_result;

	LOG_StackAddr(__FUNCTION__);

	L_result = L_var1 + L_var2;

	if ( 0 == ((L_var1 ^ L_var2) & EM_MIN_INT32) )
		if ( (L_result ^ L_var1) & EM_MIN_INT32 )
			return L_var1 < 0 ? EM_MIN_INT32 : EM_MAX_INT32;

	return L_result;
#endif
}

/* 双精度浮点值减法 */
Q31 emInt32 emCall L_sub_1(Q31 emInt32 L_var1, Q31 emInt32 L_var2)
{
#ifdef EM_TYPE_INT64
	Q63 emInt64 H_result;

	LOG_StackAddr(__FUNCTION__);

	H_result = L_var1;
	H_result -= L_var2;

	if ( H_result > EM_MAX_INT32 )
		return EM_MAX_INT32;

	if ( H_result < EM_MIN_INT32 )
		return EM_MIN_INT32;

	return (Q31 emInt32)H_result;
#else
	Q31 emInt32 L_result;

	LOG_StackAddr(__FUNCTION__);

	L_result = L_var1 - L_var2;

	if ( (L_var1 ^ L_var2) & EM_MIN_INT32 )
		if ( (L_result ^ L_var1) & EM_MIN_INT32 )
			return L_var1 < 0 ? EM_MIN_INT32 : EM_MAX_INT32;

	return L_result;
#endif
}

/* 单精度浮点值左移操作 */
Q15 emInt16 emCall emMath_shl(Q15 emInt16 var1, Q15 emInt16 var2)
{
	Q31 emInt32 L_var1;

	LOG_StackAddr(__FUNCTION__);

	if ( var2 < 15 )
	{
		L_var1 = var1;
		L_var1 <<= var2;

		if ( L_var1 == (Q15 emInt16)L_var1 )
			return (Q15 emInt16)L_var1;
	}

	if ( 0 == var1 )
		return 0;
	else
		return (Q15 emInt16)(var1 > 0 ? EM_MAX_INT16 : EM_MIN_INT16);
}

/* 双精度浮点值左移操作 */
Q31 emInt32 emCall emMath_L_shl(Q31 emInt32 L_var1, Q15 emInt16 var2)
{
#ifdef EM_TYPE_INT64
	Q63 emInt64 H_var1;

	LOG_StackAddr(__FUNCTION__);

	if ( var2 < 31 )
	{
		H_var1 = L_var1;
		H_var1 <<= var2;

		if ( H_var1 == (Q31 emInt32)H_var1 )
			return (Q31 emInt32)H_var1;
	}

	if ( 0 == L_var1 )
		return 0;
	else
		return L_var1 > 0 ? EM_MAX_INT32 : EM_MIN_INT32;
#else

	LOG_StackAddr(__FUNCTION__);

	while ( var2 -- )
	{
		if ( L_var1 > DOUBLE_Q30 || ~L_var1 > DOUBLE_Q30 )
			return L_var1 > 0 ? EM_MAX_INT32 : EM_MIN_INT32;

		L_var1 <<= 1;
	}

	return L_var1;
#endif
}

/* 求单精度浮点值的归1化因子 */
emUInt8 emCall norm_s(Q15 emInt16 var1)
{
	emUInt8 var_out;

	LOG_StackAddr(__FUNCTION__);

	if ( 0 == var1 )
		return 0;

	if ( -1 == var1 )
		return 15;

	if ( var1 < 0 )
		var1 = (emInt16)~var1;

	for ( var_out = 0; var1 < FLOAT_Q14; ++ var_out )
		var1 <<= 1;

	return var_out;
}

/* 求双精度浮点值的归1化因子 */
emUInt8 emCall norm_l(Q31 emInt32 L_var1)
{
	emUInt8 var_out;

	LOG_StackAddr(__FUNCTION__);

	if ( 0 == L_var1 )
		return 0;

	if ( -1 == L_var1 )
		return 31;

	if ( L_var1 < 0 )
		L_var1 = ~L_var1;

	var_out = 0;

	if ( L_var1 < DOUBLE_Q15 )
	{
		L_var1 <<= 16;
		var_out += 16;
	}

	for ( ; L_var1 < DOUBLE_Q30; ++ var_out )
		L_var1 <<= 1;

	return var_out;
}

/*
Q31 emInt32 emCall Div_32(Q31 emInt32 L_num, Q15 emInt16 denom_hi, Q15 emInt16 denom_lo)
{
	Q15 emInt16 approx, hi, lo, n_hi, n_lo;
	Q31 emInt32 L_32;

	LOG_StackAddr(__FUNCTION__);

	// First approximation: 1 / L_denom = 1/denom_hi 
	approx = (Q15 emInt16)div_s(FLOAT_Q14, denom_hi);

	// 1/L_denom = approx * (2.0 - L_denom * approx) 
	L_32 = Mpy_32_16(denom_hi, denom_lo, approx);
	L_32 = L_sub(DOUBLE_Q31, L_32);
	L_Extract(L_32, &hi, &lo);
	L_32 = Mpy_32_16(hi, lo, approx);

	// L_num * (1/L_denom) 
	L_Extract(L_32, &hi, &lo);
	L_Extract(L_num, &n_hi, &n_lo);
	L_32 = Mpy_32(n_hi, n_lo, hi, lo);
	L_32 = L_shl(L_32, 2); 

	return L_32;
}

*/


/* 查表并线形插值求cos */
Q15 emInt16 emCall F_cos(Q16 emUInt16 i)
{
	emUInt16 t1, t2;
	emInt32 lt;

	LOG_StackAddr(__FUNCTION__);

	t1 = (emUInt16)((i >> 7) & 0x01FF);
	t2 = (emUInt16)(i & 0x007F);
	if ( t1 & 0x100 )
	{
		t1 = 0x1FF - t1;
		t2 = 0x80 - t2;
	}
	lt = (0x80L - t2) * g_nCosTable[t1];
	lt += (emInt32)t2 * g_nCosTable[t1 + 1];
	return (emInt16)(lt >> 7);
}


#if EM_CAL_EXP

//x at [-1～0] y(Q16) at [1～e-1]
emStatic emConst Q16 emUInt16 g_fExpTab[257] =
{
	0xffff,	0xff00,	0xfe01,	0xfd03,	0xfc07,	0xfb0b,	0xfa11,	0xf917,
	0xf81f,	0xf727,	0xf630,	0xf53b,	0xf446,	0xf352,	0xf25f,	0xf16d,
	0xf07c,	0xef8c,	0xee9d,	0xedaf,	0xecc2,	0xebd6,	0xeaea,	0xea00,
	0xe916,	0xe82e,	0xe746,	0xe65f,	0xe579,	0xe494,	0xe3b0,	0xe2cd,
	0xe1ea,	0xe109,	0xe028,	0xdf49,	0xde6a,	0xdd8c,	0xdcaf,	0xdbd2,
	0xdaf7,	0xda1d,	0xd943,	0xd86a,	0xd792,	0xd6bb,	0xd5e5,	0xd50f,
	0xd43a,	0xd367,	0xd294,	0xd1c1,	0xd0f0,	0xd020,	0xcf50,	0xce81,
	0xcdb3,	0xcce6,	0xcc19,	0xcb4d,	0xca82,	0xc9b8,	0xc8ef,	0xc826,
	0xc75f,	0xc698,	0xc5d2,	0xc50c,	0xc447,	0xc384,	0xc2c0,	0xc1fe,
	0xc13c,	0xc07c,	0xbfbb,	0xbefc,	0xbe3d,	0xbd80,	0xbcc2,	0xbc06,
	0xbb4a,	0xba90,	0xb9d5,	0xb91c,	0xb863,	0xb7ab,	0xb6f4,	0xb63d,
	0xb587,	0xb4d2,	0xb41e,	0xb36a,	0xb2b7,	0xb204,	0xb153,	0xb0a2,
	0xaff2,	0xaf42,	0xae93,	0xade5,	0xad37,	0xac8a,	0xabde,	0xab33,
	0xaa88,	0xa9de,	0xa934,	0xa88b,	0xa7e3,	0xa73b,	0xa694,	0xa5ee,
	0xa549,	0xa4a4,	0xa3ff,	0xa35c,	0xa2b9,	0xa216,	0xa174,	0xa0d3,
	0xa033,	0x9f93,	0x9ef4,	0x9e55,	0x9db7,	0x9d1a,	0x9c7d,	0x9be1,
	0x9b45,	0x9aaa,	0x9a10,	0x9976,	0x98dd,	0x9844,	0x97ac,	0x9715,
	0x967e,	0x95e8,	0x9552,	0x94bd,	0x9429,	0x9395,	0x9302,	0x926f,
	0x91dd,	0x914b,	0x90ba,	0x902a,	0x8f9a,	0x8f0a,	0x8e7c,	0x8dee,
	0x8d60,	0x8cd3,	0x8c46,	0x8bba,	0x8b2f,	0x8aa4,	0x8a19,	0x8990,
	0x8906,	0x887e,	0x87f5,	0x876e,	0x86e7,	0x8660,	0x85da,	0x8554,
	0x84cf,	0x844b,	0x83c7,	0x8343,	0x82c0,	0x823e,	0x81bc,	0x813a,
	0x80b9,	0x8039,	0x7fb9,	0x7f39,	0x7eba,	0x7e3c,	0x7dbe,	0x7d40,
	0x7cc3,	0x7c47,	0x7bcb,	0x7b4f,	0x7ad4,	0x7a59,	0x79df,	0x7966,
	0x78ed,	0x7874,	0x77fc,	0x7784,	0x770d,	0x7696,	0x761f,	0x75aa,
	0x7534,	0x74bf,	0x744b,	0x73d7,	0x7363,	0x72f0,	0x727d,	0x720b,
	0x7199,	0x7128,	0x70b7,	0x7046,	0x6fd6,	0x6f67,	0x6ef7,	0x6e89,
	0x6e1a,	0x6dac,	0x6d3f,	0x6cd2,	0x6c65,	0x6bf9,	0x6b8d,	0x6b22,
	0x6ab7,	0x6a4d,	0x69e2,	0x6979,	0x6910,	0x68a7,	0x683e,	0x67d6,
	0x676f,	0x6707,	0x66a0,	0x663a,	0x65d4,	0x656e,	0x6509,	0x64a4,
	0x6440,	0x63dc,	0x6378,	0x6315,	0x62b2,	0x624f,	0x61ed,	0x618c,
	0x612a,	0x60c9,	0x6069,	0x6008,	0x5fa9,	0x5f49,	0x5eea,	0x5e8b,
	0x5e2d
};


emStatic emConst Q16 emInt32 g_fExpPow[11] =
{
	0x00002288,	0x00005DC0,	0x0000FEEC,	0x0002B4E4,
	0x00075B84,	0x00140000,	0x00365D94,	0x0093C7F4,
	0x0193e780,	0x0443F680,	0x0B984364				
};

//2012-10-29
emInt32 emCall CalExp(emUInt8 nQ, emInt16 fVal)
{
	emInt32 fRev;
	emUInt32 fLogVal,fTemp;
	emUInt16 fCoef1, fCoef2;
	emUInt16 fRvFrac; //1-fFrac 当0==fFrac 这个值取0，正好向高位进位
	emInt32 nVal;
	emUInt16 nIndex;
	emUInt16 nRes;
	emUInt16 fHiWord ,fLowWord;

	//将浮点值分解为整数部分和小数部分：(n+x)->(n+1)-(1-x)
	fRev = -(emInt32)fVal;
	nVal = -(fRev >> nQ);

	fRvFrac = (emUInt16)(fRev << (16 - nQ)); //归一到
	nIndex = fRvFrac >> 8;

	nRes = fRvFrac & 0x00FF;
	fCoef1 = g_fExpTab[nIndex];
	fCoef2 = g_fExpTab[nIndex + 1];

	fLogVal = ((emInt32)fCoef1 * (256 - nRes) + (emInt32)fCoef2 * nRes) >> 8;	//线性内插	

	fHiWord = (emUInt16)(g_fExpPow[nVal + 2] >> 16);
	fLowWord = (emUInt16)(g_fExpPow[nVal + 2] & 0xFFFF);
	fTemp = fHiWord*fLogVal;
	fTemp += (fLowWord * fLogVal + 0x7FFF) >> 16;
	fTemp >>= 16 - nQ;
	return fTemp;
}

#endif