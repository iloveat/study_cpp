//实现频率范围内任意采样率的相互转换
#include "emPCH.h"
#include "emResample.h"

/* 阶数定义： 阶数越高音质越好，但耗资源越大 */
#define ORDER	4
#define ORDER2	8


#define sinc_table_size 303

emStatic emConst emInt16 _sinc_table[sinc_table_size] = 
{
	32767,	32742,	32675,	32566,	32410,	32211,	31969,	31684,	31359,	30992,
	30582,	30140,	29655,	29135,	28582,	27993,	27371,	26720,	26040,	25329,
	24598,	23840,	23059,	22259,	21441,	20607,	19758,	18897,	18026,	17147,
	16261,	15372,	14479,	13587,	12698,	11811,	10931,	10058,	9194,	8343,
	7504,	6680,	5871,	5081,	4310,	3560,	2831,	2126,	1445,	790,
	161,	-438,	-1011,	-1556,	-2071,	-2555,	-3011,	-3434,	-3827,	-4189,
	-4520,	-4819,	-5088,	-5326,	-5533,	-5710,	-5858,	-5977,	-6068,	-6130,
	-6167,	-6177,	-6162,	-6121,	-6058,	-5974,	-5868,	-5741,	-5596,	-5433,
	-5254,	-5060,	-4852,	-4631,	-4399,	-4156,	-3905,	-3646,	-3381,	-3110,
	-2836,	-2559,	-2281,	-2002,	-1724,	-1449,	-1175,	-906,	-641,	-382,
	-130,	113,	350,	578,	796,	1004,	1202,	1389,	1564,	1728,
	1879,	2018,	2145,	2258,	2359,	2447,	2523,	2586,	2636,	2674,
	2700,	2714,	2716,	2707,	2687,	2657,	2617,	2567,	2508,	2441,
	2366,	2283,	2194,	2098,	1997,	1891,	1780,	1666,	1548,	1427,
	1305,	1181,	1056,	931,	806,	681,	558,	437,	318,	201,
	87,	-22,	-128,	-230,	-328,	-421,	-510,	-593,	-671,	-743,
	-810,	-871,	-926,	-976,	-1019,	-1057,	-1089,	-1115,	-1135,	-1150,
	-1159,	-1164,	-1162,	-1157,	-1146,	-1131,	-1111,	-1088,	-1060,	-1030,
	-995,	-958,	-918,	-876,	-832,	-786,	-738,	-688,	-638,	-587,
	-536,	-484,	-432,	-380,	-329,	-279,	-230,	-181,	-134,	-88,
	-44,	-2,	37,	76,	112,	146,	179,	208,	236,	261,
	283,	303,	321,	336,	350,	360,	369,	375,	380,	382,
	382,	381,	377,	373,	366,	358,	349,	338,	327,	314,
	301,	287,	272,	257,	241,	225,	209,	193,	177,	161,
	145,	130,	114,	100,	85,	72,	58,	46,	34,	23,
	12,	2,	-5,	-14,	-21,	-28,	-34,	-39,	-43,	-47,
	-50,	-53,	-54,	-56,	-56,	-56,	-56,	-55,	-54,	-53,
	-51,	-49,	-47,	-44,	-42,	-39,	-36,	-34,	-31,	-28,
	-25,	-23,	-20,	-18,	-15,	-13,	-11,	-9,	-8,	-6,
	-5,	-4,	-3,	-2,	-1,	-1,	0,	0,	0,	0,
	0,	0,	0
};



emInt32 abs_l(emInt32 var1);							
emUInt32 mult_to_u32(emUInt32 var1, emUInt16 var2);		
emInt32 mult_32_16_to_32(emInt32 L_var1,emInt16 var2);	
emInt16 sature(emInt32 var_l);							


void ReSample_Init(PReSampleData hReSample, emUInt16 PcmInRate, emUInt16 PcmOutRate)
{

	emInt16 freq=0;
	emInt16 i = 0;

	LOG_StackAddr(__FUNCTION__);

	for (i=0; i < 9; i++)
	{
		hReSample->PrePcm[i] = 0;
	}
	
	/*  2^28 = 0x10000000 */
	hReSample->des_int = (emUInt16)( 0x10000000 / PcmOutRate );	
	hReSample->d_last = hReSample->des_int;
	hReSample->ori_int = (emUInt16)( 0x10000000 / PcmInRate );	


	if (PcmOutRate >= PcmInRate)
	{
		hReSample->gain = (emInt16)0x7fff;
	}
	else
	{
		hReSample->gain = (emInt16)( 0x8000*PcmOutRate/PcmInRate );
	}
	freq = (emInt16)(PcmOutRate>PcmInRate)?(PcmInRate):(PcmOutRate);	
	
	/* 计算pi_freq = freq*pi>>3  */
	switch(freq)
	{
	/*case 4000:
		hReSample->pi_freq = 0x622;
		break;*/
	case 6000:
		hReSample->pi_freq = 0x934;
		break;
	case 8000:
		hReSample->pi_freq = 0xc45;
		break;
	case 11025:
		hReSample->pi_freq = 0x10e9;
		break;
	case 16000:
		hReSample->pi_freq = 0x188b;
		break;
	case 22050:
		hReSample->pi_freq = 0x21d3;
		break;
	//case 24000:
	//	hReSample->pi_freq = 0x24d0;
	//	break;
	case 32000:
		hReSample->pi_freq = 0x3116;
		break;
	case 44100:
		hReSample->pi_freq = 0x43a6;
		break;
	//case 48000:
	//	hReSample->pi_freq = 0x49a1;
	//	break;
	default :
		hReSample->pi_freq = (emInt16)( (freq * 31416 / 10000) >> 3 );
		break;
	}
	if (hReSample->pi_freq<0)
	{
		hReSample->pi_freq = 0;
	}
	return;
}


emUInt32 GetMaxOutSamples(PReSampleData hReSample, emUInt32 nuSamples)
{
	emUInt32 OutSamples=0;

	LOG_StackAddr(__FUNCTION__);

	if (nuSamples==0)
	{
		return 0;
	}

	OutSamples = (emUInt32)(nuSamples*(emUInt32)hReSample->ori_int / (emUInt32)hReSample->des_int);
	OutSamples += 2;		
	return OutSamples;
}



emInt32 negate_l(emInt32 var1)      
{
	emInt32 var_out;

	LOG_StackAddr(__FUNCTION__);
	
	var_out = (var1 == EM_MIN_INT32) ? EM_MAX_INT32 : - var1;
	return (var_out);
}


emInt32 abs_l(emInt32 var1)
{
	emInt32 var_out;

	LOG_StackAddr(__FUNCTION__);

	if (var1 < 0)
    {
		var_out = negate_l(var1);
	}
	else
    {
		var_out = var1;
    }

    return (var_out);
}


emUInt32 mult_to_u32(emUInt32 var1, emUInt16 var2)
{
	emUInt32 L_produit;

	LOG_StackAddr(__FUNCTION__);
	
	L_produit = (emUInt32)var1 * (emUInt32)var2;
	
	return (L_produit);
}

emInt16 emResextract_l(emInt32 var_l)
{
	emInt16 var_out;

	LOG_StackAddr(__FUNCTION__);
	
	var_out = (emInt16)(var_l & 0x0000FFFFL);
	return (var_out);
}

emInt32 mult_to_32(emInt16 var1, emInt16 var2)
{
	emInt32 L_produit;

	LOG_StackAddr(__FUNCTION__);
	
	L_produit = (emInt32)var1 * (emInt32)var2;
	
	return (L_produit);
}

emInt32 mult_32_16_to_32(emInt32 L_var1,emInt16 var2) 
{
	emInt32 var1_hi,var1_lo;
	emInt32 var_out;

	LOG_StackAddr(__FUNCTION__);

	var1_hi =  (emInt32)(L_var1 >> 16);

	var1_lo =  (emInt32)(L_var1 & 0x0000FFFFL);

	var_out  =  ((emInt32)var1_lo*var2) >> 15;

	var_out +=  ((emInt32)var1_hi * var2 << 1);
	return var_out;
}

emInt16 sature(emInt32 var_l)
{
	emInt16 var_out;

	LOG_StackAddr(__FUNCTION__);
	
	if (var_l > 0X00007fffL)
	{		
		var_out = EM_MAX_INT16;
	}
	else if (var_l < (emInt32)0xffff8000L)
	{		
		var_out = EM_MIN_INT16;
	}
	else
	{		
		var_out = emResextract_l(var_l);
	}
	return (var_out);
}



emUInt32 ReSample(PReSampleData hReSample,emPInt16 PcmIn,emPInt16 PcmOut,emUInt32 nuSamples)
{
	emPInt16	 pPre;	
	emInt32  sum = 0;
	emInt32  temp = 0;
	emUInt32 frlen = 0;
	emUInt32 OutSamples = 0;
	emUInt32 distance = 0;
	emUInt32 i=0, j=0;

	emInt16 nReplaceDiv;
	emUInt32 nReplaceDis;

	LOG_StackAddr(__FUNCTION__);

	frlen = (emUInt32)nuSamples * hReSample->ori_int;				
	pPre = hReSample->PrePcm;

	distance = (emInt16)(-(emInt16)hReSample->d_last);				
	//distance = (emInt32)(-(emInt32)hReSample->d_last);	

	nReplaceDiv = 0;
	nReplaceDis = distance;
	for (OutSamples=0; (distance + hReSample->des_int) < frlen; ++OutSamples)				
	{
		distance += hReSample->des_int;													
		nReplaceDis += hReSample->des_int;

		while ( nReplaceDis >= hReSample->ori_int )
		{
			nReplaceDis -= hReSample->ori_int;
			++nReplaceDiv;
		}


		j = ORDER + (emInt16)(nReplaceDiv);
		sum = 0;
		for(i = (j - ORDER);i <= (j + ORDER + 1);i++)
		{

			emInt32 dw = (i - ORDER)*(emInt32)hReSample->ori_int - (emInt32)distance;
			emInt32 xx = dw >> 31;
			temp = (xx ^ dw) - xx;

			temp = mult_32_16_to_32(temp,hReSample->pi_freq) >> 6;	/* pi_freq=pi*freq>>3 */

			if(temp >= sinc_table_size)
			{
				temp = sinc_table_size - 1;
			}
			if ( i <= ORDER2 )
			{
				sum += ((emInt32)_sinc_table[temp] * pPre[i]) >> 15;
			}
			else
			{
				sum += ((emInt32)_sinc_table[temp] * PcmIn[i-ORDER2-1]) >> 15;
			}
		}

		temp = (sum * hReSample->gain) >> 15;
		if (temp > 0X00007fffL)
		{		
			temp = EM_MAX_INT16;
		}
		else if (temp < (emInt32)0xffff8000L)
		{		
			temp = EM_MIN_INT16;
		}
	
		PcmOut[OutSamples] = ((emInt16)(temp));
	}

	if (nuSamples > ORDER2)								
	{
		for (i = 0;i <= ORDER2; i++)
		{
			pPre[i] = PcmIn[nuSamples-ORDER2-1+i];
		}
	}
	else												
	{
		for (i = 0; i < ORDER2+1-nuSamples; i++)
		{
			pPre[i] = pPre[nuSamples+i];
		}
		for (i = 0; i < nuSamples; i++)
		{
			pPre[ORDER2-nuSamples+1+i] = PcmIn[i];
		}
	}

	temp = frlen - distance;
	if (temp > 0X00007fffL)
	{		
		temp = EM_MAX_INT16;
	}
	else if (temp < (emInt32)0xffff8000L)
	{		
		temp = EM_MIN_INT16;
	}
	hReSample->d_last = (emInt16)temp;		

	return OutSamples;
}
