/* ��׼������Դ�ļ� */

#include "emPCH.h"
#include "emCommon.h"
#include "emString.h"


/******************************************************************************
* ������        : emIntToStr
* ����          : ֵ����ת��
* ����           : [in]  nNum - ��תֵ
*                      : [out] pStr - ��������ָ��
* ����               : <emSize>   - ת�����Ĵ��ĳ���

******************************************************************************/
emSize emCall emIntToStr( emInt32 nNum, emPUInt8 pStr )
{
	emSize n, i;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pStr);

	n = 0;

	do
	{
		pStr[n++] = (emUInt8)(0x30|(nNum % 10));
		nNum /= 10;
	}
	while ( nNum );

	i = --n >> 1;
	++ i;

	while ( i-- > 0 )
	{
		emUInt8 t = pStr[i];
		pStr[i] = pStr[n - i];
		pStr[n - i] = t;
	}

	return n + 1;
}


/******************************************************************************
* ������        : emStrToInt
* ����          : ����ֵת��
* ����           : [in] pStr  - ��ָ��
*                      : [in] nLen  - ������
* ����               : <ֵ> - ��ת���ɵ�ֵ

******************************************************************************/
emInt32 emCall emStrToInt( emPCUInt8 pStr, emSize nLen )
{
	emBool s;
	emInt32 v;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pStr);

	while ( nLen && *pStr <= emChar_Space )
	{
		++ pStr;
		-- nLen;
	}

	if ( nLen && *pStr == emChar_Sub )
	{
		s = emTrue;

		do
		{
			++ pStr;
			-- nLen;
		}
		while ( nLen && *pStr <= emChar_Space );
	}
	else
	{
		s = emFalse;
	}

	v = 0;

	while ( nLen && *pStr >= emChar_0 && *pStr <= emChar_9 )
	{
		v *= 10;
		v += (emUInt8)(0x0F&(*pStr));

		++ pStr;
		-- nLen;
	}

	if ( s )
		return -v;
	else
		return v;
}


/******************************************************************************
* ������        : emStrChar
* ����          : �ڴ��в����ַ�
* ����           : [in] pStr  - ��ָ��
*                      : [in] nLen  - ������
*                      : [in] nChar - �ַ�
* ����               : emNull - ��������
*                      : <����> - ���е�һ��ƥ����ַ�ָ��

******************************************************************************/
emPCUInt8 emCall emStrChar( emPCUInt8 pStr, emSize nLen, emUInt8 nChar )
{
	emSize i;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pStr);

	for ( i = 0; i < nLen; ++ i )
	{
		if ( *pStr == nChar )
			return pStr;

		++ pStr;
	}

	return emNull;
}


/******************************************************************************
* ������        : emStrStr
* ����          : ģʽƥ��(����KMP�Ľ��㷨)  
* ����           : [in] pStr    - ĸ��ָ��
*                      : [in] nLen    - ĸ������
*                      : [in] pSubStr - �Ӵ�ָ��
*                      : [in] nSubLen - �Ӵ�����
* ����               : emNull - ��������
*                      : <����> - ĸ���е�һ��ƥ����Ӵ�ָ��

******************************************************************************/
emPCUInt8 emCall emStrStr( emPCUInt8 pStr, emSize nLen, emPCUInt8 pSubStr, emSize nSubLen )
{
	emUInt8 pNextBuf[1 + EM_PARAM_SUBSTR_MAXLEN + 1];
	emUInt8 *pNextPos;
	emSize  i, j;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pStr && pSubStr);

	if ( nLen < nSubLen )
		return emNull;

	emAssert(nSubLen <= EM_PARAM_SUBSTR_MAXLEN);

	pNextPos = pNextBuf/* - 1*/;

	-- pStr;
	-- pSubStr;

	i = 1;
	j = pNextPos[1] = 0;

	while ( i < nSubLen )
	{
		if ( 0 == j || pSubStr[i] == pSubStr[j] )
		{
			++ i;
			++ j;
			pNextPos[i] = j;
		}
		else 
			j = pNextPos[j];
	}

	i = j = 1;

	while ( i <= nLen && j <= nSubLen )
	{
		if ( 0 == j || pStr[i] == pSubStr[j] )
		{
			++ i;
			++ j;
		}
		else 
			j = pNextPos[j];	
	}

	if ( j > nSubLen )
		return pStr + (i - nSubLen);

	return emNull;
}


/******************************************************************************
* ������        : emStrComp
* ����          : �ַ����Ƚ�
* ����           : [in] s1 - ��1ָ��
*                      : [in] n1 - ��1����
*                      : [in] s2 - ��2ָ��
*                      : [in] n2 - ��2����
* ����               : emComp  - ��1,2�Ĵ�С��ϵ

******************************************************************************/
emComp emCall emStrComp( emPCUInt8 s1, emSize n1, emPCUInt8 s2, emSize n2 )
{
	emSize nLen, i;

	LOG_StackAddr(__FUNCTION__);

	emAssert(s1 && s2);

	nLen = emMin(n1, n2);

	for ( i = 0; i < nLen; ++ i )
	{
		if ( s1[i] > s2[i] )
			return emGreater;

		if ( s1[i] < s2[i] )
			return emLesser;
	}

	if ( n1 > n2 )
		return emGreater;

	if ( n1 < n2 )
		return emLesser;

	return emEqual;
}


/******************************************************************************
* ������        : emStrCompN
* ����          : �ַ����Ƚ�
* ����           : [in] s1 - ��1ָ��
*                      : [in] s2 - ��2ָ��
*                      : [in] n - �ȽϵĴ�����
* ����               : emComp  - ��1,2�Ĵ�С��ϵ

******************************************************************************/
emComp emCall emStrCompN( emPCUInt8 s1, emPCUInt8 s2, emSize n )
{
	emSize i;

	LOG_StackAddr(__FUNCTION__);

	emAssert(s1 && s2);

	for ( i = 0; i < n; ++ i )
	{
		if ( s1[i] > s2[i] )
			return emGreater;

		if ( s1[i] < s2[i] )
			return emLesser;
	}

	return emEqual;
}


/******************************************************************************
* ������        : emStrCompNI
* ����          : �ַ����Ƚ�
* ����           : [in] s1 - ��1ָ��
*                      : [in] s2 - ��2ָ��
*                      : [in] n - �ȽϵĴ�����
* ����               : emComp  - ��1,2�Ĵ�С��ϵ

******************************************************************************/
emComp emCall emStrCompNI( emPCUInt8 s1, emPCUInt8 s2, emSize n )
{
	emSize i;

	LOG_StackAddr(__FUNCTION__);

	emAssert(s1 && s2);

	for ( i = 0; i < n; ++ i )
	{
		emUInt8 c1, c2;
		c1 = s1[i];
		c2 = s2[i];

		if ( emIsAlpha(c1) && emIsAlpha(c2) )
		{
			c1 = emToUpper(c1);
			c2 = emToUpper(c2);
		}

		if ( c1 > c2 )
			return emGreater;

		if ( c1 < c2 )
			return emLesser;
	}

	return emEqual;
}

#if !EM_ANSI_STRING

emSize emCall emStrLenA( emCStrA sz )
{
	emSize n;

	LOG_StackAddr(__FUNCTION__);

	n = 0;

	while ( *sz )
	{
		++ sz;
		++ n;
	}

	return n;
}

emSize emCall emStrLenW( emCStrW sz )
{
	emSize n;

	LOG_StackAddr(__FUNCTION__);

	n = 0;

	while ( *sz )
	{
		++ sz;
		++ n;
	}

	return n;
}

#endif /* !EM_ANSI_STRING */
