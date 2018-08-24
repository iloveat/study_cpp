//�����������Ͷ���ͷ�ļ�

#ifndef	_EMTTS__EM_API_DEFINE__H_
#define _EMTTS__EM_API_DEFINE__H_


#include "emApiPlatform.h"


#define emConst		EM_CONST
#define emStatic	EM_STATIC

#ifdef EM_INLINE
	#define emInline	EM_STATIC EM_INLINE
#else
	#define emInline	EM_STATIC
#endif

#define emExtern	EM_EXTERN

#define emPtr		EM_PTR_PREFIX*
#define emCPtr		emConst emPtr
#define emPtrC		emPtr emConst
#define emCPtrC		emConst emPtr emConst

#define emCall		EM_CALL_STANDARD		//�ǵݹ��(���������)
#define emReentrant	EM_CALL_REENTRANT		//�ݹ��(�������)
#define emVACall	EM_CALL_VAR_ARG			//֧�ֱ�ε�

#define emProc		emCall emPtr
#define	emNull		(0)


//����ֵ���� 
typedef	signed EM_TYPE_INT8		emInt8;			//8λ
typedef	unsigned EM_TYPE_INT8	emUInt8;		//8λ�޷���

typedef	signed EM_TYPE_INT16	emInt16;		//16λ
typedef	unsigned EM_TYPE_INT16	emUInt16;		//16λ�޷���

typedef	signed EM_TYPE_INT32	emInt32;		//32λ
typedef	unsigned EM_TYPE_INT32	emUInt32;		//32λ�޷���

#ifdef EM_TYPE_INT64
	typedef	signed	 EM_TYPE_INT64	emInt64;	//64λ
	typedef	unsigned EM_TYPE_INT64	emUInt64;	//64λ�޷���
#endif


//��Ӧ��ָ������
typedef	emInt8		emPtr emPInt8;				//8λ
typedef	emUInt8		emPtr emPUInt8;				//8λ�޷���

typedef	emInt16		emPtr emPInt16;				//16λ
typedef	emUInt16	emPtr emPUInt16;			//16λ�޷���

typedef	emInt32		emPtr emPInt32;				//32λ
typedef	emUInt32	emPtr emPUInt32;			//32λ�޷���

#ifdef EM_TYPE_INT64
	typedef	emInt64		emPtr emPInt64;			//64λ
	typedef	emUInt64	emPtr emPUInt64;		//64λ�޷���
#endif


//����ָ������
typedef	emInt8		emCPtr emPCInt8;			//8λ
typedef	emUInt8		emCPtr emPCUInt8;		//8λ�޷���

typedef	emInt16		emCPtr emPCInt16;			//16λ
typedef	emUInt16	emCPtr emPCUInt16;			//16λ�޷���

typedef	emInt32		emCPtr emPCInt32;			//32λ
typedef	emUInt32	emCPtr emPCUInt32;			//32λ�޷���

#ifdef EM_TYPE_INT64
	typedef	emInt64		emCPtr emPCInt64;		//64λ
	typedef	emUInt64	emCPtr emPCUInt64;		//64λ�޷���
#endif



//�ڴ������Ԫ
typedef	emUInt8		emByte;
typedef	emPUInt8	emPByte;
typedef	emPCUInt8	emPCByte;

typedef	signed			emInt;
typedef	signed emPtr	emPInt;
typedef	signed emCPtr	emPCInt;

typedef	unsigned		emUInt;
typedef	unsigned emPtr	emPUInt;
typedef	unsigned emCPtr	emPCUInt;

//ָ��
typedef	void emPtr	emPointer;
typedef	void emCPtr	emCPointer;

//��ַ���ߴ�����
typedef	EM_TYPE_ADDRESS	emAddress;
typedef	EM_TYPE_SIZE	emSize;


//�ַ����Ͷ���
typedef emInt8		emCharA;
typedef emUInt16	emCharW;
#if EM_UNICODE
	typedef emCharW		emChar;
#else
	typedef emCharA		emChar;
#endif


//�ַ������Ͷ���
typedef emCharA emPtr	emStrA;
typedef emCharA emCPtr	emCStrA;
typedef emCharW emPtr	emStrW;
typedef emCharW emCPtr	emCStrW;
typedef emChar emPtr	emStr;
typedef emChar emCPtr	emCStr;


//�ı�������
#define emTextA(s)	((emCStrA)s)
#define emTextW(s)	((emCStrW)L##s)
#if EM_UNICODE
	#define emText(s)	emTextW(s)
#else
	#define emText(s)	emTextA(s)
#endif

typedef	emInt8 emBool;
#define	emTrue	(~0)
#define	emFalse	(0)

//��Դ��ض���
typedef emUInt32 emResAddress;
typedef emUInt32 emResSize;

typedef void (emProc emCBReadRes)(
	emPointer		pParameter,			//[in]�û���Դ�ص�ָ��
	emPointer		pBuffer,			//[out]��ȡ��Դ��ŵ�buffer
	emResAddress	iPos,				//[in]��ȡ����ʼ��ַ
	emResSize		nSize );			//[in]��ȡ�Ĵ�С

typedef struct tagResPackDesc emTResPackDesc, emPtr emPResPackDesc;


struct tagResPackDesc
{
	emPointer		pCBParam;			//[in]��Դ�ص�ָ��
	emCBReadRes		pfnRead;			//[in]��ȡ��Դ�ص����
	emResSize		nSize;				/* resource pack size (optional, 0 for null) */

	emPUInt8		pCacheBlockIndex;	/* cache block index (optional, size = dwCacheBlockCount) */
	emPointer		pCacheBuffer;		/* cache buffer (optional, size = dwCacheBlockSize * dwCacheBlockCount) */
	emSize			nCacheBlockSize;	/* cache block size (optional, must be 2^N) */
	emSize			nCacheBlockCount;	/* cache block count (optional, must be 2^N) */
	emSize			nCacheBlockExt;		/* cache block ext (optional) */
};


#endif	// #define _EMTTS__EM_API_DEFINE__H_ 











