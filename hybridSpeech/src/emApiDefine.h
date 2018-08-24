//基本数据类型定义头文件

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

#define emCall		EM_CALL_STANDARD		//非递归的(不可重入的)
#define emReentrant	EM_CALL_REENTRANT		//递归的(可重入的)
#define emVACall	EM_CALL_VAR_ARG			//支持变参的

#define emProc		emCall emPtr
#define	emNull		(0)


//基本值类型 
typedef	signed EM_TYPE_INT8		emInt8;			//8位
typedef	unsigned EM_TYPE_INT8	emUInt8;		//8位无符号

typedef	signed EM_TYPE_INT16	emInt16;		//16位
typedef	unsigned EM_TYPE_INT16	emUInt16;		//16位无符号

typedef	signed EM_TYPE_INT32	emInt32;		//32位
typedef	unsigned EM_TYPE_INT32	emUInt32;		//32位无符号

#ifdef EM_TYPE_INT64
	typedef	signed	 EM_TYPE_INT64	emInt64;	//64位
	typedef	unsigned EM_TYPE_INT64	emUInt64;	//64位无符号
#endif


//相应的指针类型
typedef	emInt8		emPtr emPInt8;				//8位
typedef	emUInt8		emPtr emPUInt8;				//8位无符号

typedef	emInt16		emPtr emPInt16;				//16位
typedef	emUInt16	emPtr emPUInt16;			//16位无符号

typedef	emInt32		emPtr emPInt32;				//32位
typedef	emUInt32	emPtr emPUInt32;			//32位无符号

#ifdef EM_TYPE_INT64
	typedef	emInt64		emPtr emPInt64;			//64位
	typedef	emUInt64	emPtr emPUInt64;		//64位无符号
#endif


//常量指针类型
typedef	emInt8		emCPtr emPCInt8;			//8位
typedef	emUInt8		emCPtr emPCUInt8;		//8位无符号

typedef	emInt16		emCPtr emPCInt16;			//16位
typedef	emUInt16	emCPtr emPCUInt16;			//16位无符号

typedef	emInt32		emCPtr emPCInt32;			//32位
typedef	emUInt32	emCPtr emPCUInt32;			//32位无符号

#ifdef EM_TYPE_INT64
	typedef	emInt64		emCPtr emPCInt64;		//64位
	typedef	emUInt64	emCPtr emPCUInt64;		//64位无符号
#endif



//内存基本单元
typedef	emUInt8		emByte;
typedef	emPUInt8	emPByte;
typedef	emPCUInt8	emPCByte;

typedef	signed			emInt;
typedef	signed emPtr	emPInt;
typedef	signed emCPtr	emPCInt;

typedef	unsigned		emUInt;
typedef	unsigned emPtr	emPUInt;
typedef	unsigned emCPtr	emPCUInt;

//指针
typedef	void emPtr	emPointer;
typedef	void emCPtr	emCPointer;

//地址、尺寸类型
typedef	EM_TYPE_ADDRESS	emAddress;
typedef	EM_TYPE_SIZE	emSize;


//字符类型定义
typedef emInt8		emCharA;
typedef emUInt16	emCharW;
#if EM_UNICODE
	typedef emCharW		emChar;
#else
	typedef emCharA		emChar;
#endif


//字符串类型定义
typedef emCharA emPtr	emStrA;
typedef emCharA emCPtr	emCStrA;
typedef emCharW emPtr	emStrW;
typedef emCharW emCPtr	emCStrW;
typedef emChar emPtr	emStr;
typedef emChar emCPtr	emCStr;


//文本常量宏
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

//资源相关定义
typedef emUInt32 emResAddress;
typedef emUInt32 emResSize;

typedef void (emProc emCBReadRes)(
	emPointer		pParameter,			//[in]用户资源回调指针
	emPointer		pBuffer,			//[out]读取资源存放的buffer
	emResAddress	iPos,				//[in]读取的起始地址
	emResSize		nSize );			//[in]读取的大小

typedef struct tagResPackDesc emTResPackDesc, emPtr emPResPackDesc;


struct tagResPackDesc
{
	emPointer		pCBParam;			//[in]资源回调指针
	emCBReadRes		pfnRead;			//[in]读取资源回调入口
	emResSize		nSize;				/* resource pack size (optional, 0 for null) */

	emPUInt8		pCacheBlockIndex;	/* cache block index (optional, size = dwCacheBlockCount) */
	emPointer		pCacheBuffer;		/* cache buffer (optional, size = dwCacheBlockSize * dwCacheBlockCount) */
	emSize			nCacheBlockSize;	/* cache block size (optional, must be 2^N) */
	emSize			nCacheBlockCount;	/* cache block count (optional, must be 2^N) */
	emSize			nCacheBlockExt;		/* cache block ext (optional) */
};


#endif	// #define _EMTTS__EM_API_DEFINE__H_ 











