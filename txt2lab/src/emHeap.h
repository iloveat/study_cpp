/* �ѣ�ջ���ļ�ֱ�Ӷ�ȡ -- ����ͷ�ļ� */

#ifndef _EMTTS__EM_HEAP__H
#define _EMTTS__EM_HEAP__H


#include "emApiKernel.h"
#include "emConfig.h"
#include "emString.h"




#ifdef __cplusplus
extern "C" {
#endif


#define FALSE               0
#define TRUE                1
#define	 NOBREAK			2					//sqb 20140715 modify				���һ��nobreak������

#if ARM_MTK_LOG_TO_MTK || ARM_MTK_LOG_TIME
		#include "kal_release.h" 
		#include "kal_trace.h"
		#include "custom_config.h"
#endif


#if ARM_MTK_LOG_TO_PC_VS
	#include <stdio.h>
	FILE *g_fMTK;
#endif


emExtern emUInt32		g_BaseStack ;

//**************************  ��Դ��ȡ��ʽѡ�� ***************************************************
//************************** ��ѡ�����е�1�֣� **************************************************

#if EM_RES_READ_FILE		//��ΪӢ�ĺϳ���ʱ�ӵģ�
	#include <stdio.h>
	#define fRearSeek(file,offset,origin)			fFrontSeek(file,offset,origin)	
	#define fRearRead(buffer,nSize,nCount,file)		fFrontRead(buffer,nSize,nCount,file)

	#define fFrontSeek(file,offset,origin)			fseek(file,offset,origin)
	#define fFrontRead(buffer,nSize,nCount,file)	fread(buffer,nSize,nCount,file)
#endif



#if EM_RES_READ_CALLBACK
	#include <stdio.h>
	#define	FILE	emInt32

	#define fRearSeek(file,offset,origin)			fFrontSeek(file,offset,origin)	
	#define fRearRead(buffer,nSize,nCount,file)		fFrontRead(buffer,nSize,nCount,file)

	#define fFrontSeek(file,offset,origin)			(((origin)>(0))?(g_Res.cur_offset += offset):(g_Res.cur_offset = offset))	
	#define fFrontRead(buffer,nSize,nCount,file)	{ emResPack_Read(&g_ResPack,buffer,g_Res.cur_offset,nSize*nCount);g_Res.cur_offset += nSize*nCount;}
#endif

#if EM_RES_READ_DIRECT_ANKAI
	typedef signed long								T_S32;      /* signed 32 bit integer */

	#define SEEK_SET    0 
	#define SEEK_CUR    1 
	#define SEEK_END    2 
	#define	FILE	T_S32	

	#define fRearSeek(file,offset,origin)			fFrontSeek(file,offset,origin)	
	#define fRearRead(buffer,nSize,nCount,file)		fFrontRead(buffer,nSize,nCount,file)


	void	emCall fFrontSeek(FILE file, emInt32	offset, emInt8 origin);
	emInt32 emCall fFrontRead(char* buffer,emInt32 nSize,emInt32 nCount,FILE file) ;

	emInt32 emCall fprintf(FILE file, const char*  format, ... )  ;
	FILE  emCall fopen(char* filename,char* opentype)  ;
	void  emCall fclose(FILE file)  ;
#endif


#if EM_RES_READ_ALL_LIB_CONST
	#define	FILE	emInt32
	#define fRearSeek(file,offset,origin)			fFrontSeek(file,offset,origin)	
	#define fRearRead(buffer,nSize,nCount,file)		fFrontRead(buffer,nSize,nCount,file)

	#define fFrontSeek(file,offset,origin)			(((origin)>(0))?(g_Res.cur_offset += offset):(g_Res.cur_offset = offset))	
	#define fFrontRead(buffer,nSize,nCount,file)	{ emMemCpy(buffer,(const void *)(((emInt32)file)+g_Res.cur_offset),nSize*nCount);g_Res.cur_offset += nSize*nCount;}
#endif

#if EM_RES_READ_REAR_LIB_CONST
	#include <stdio.h>
	#define	FILE	emInt32
	#define fFrontSeek(file,offset,origin)			(((origin)>(0))?(g_Res.cur_offset += offset):(g_Res.cur_offset = offset))	
	#define fFrontRead(buffer,nSize,nCount,file)	{ emResPack_Read(&g_ResPack,buffer,g_Res.cur_offset,nSize*nCount);g_Res.cur_offset += nSize*nCount;}
	#define fRearSeek(file,offset,origin)			(((origin)>(0))?(g_Res.cur_offset += offset):(g_Res.cur_offset = offset))	
	#define fRearRead(buffer,nSize,nCount,file)		{ emMemCpy(buffer,(const void *)(((emInt32)file)+g_Res.cur_offset),nSize*nCount);g_Res.cur_offset += nSize*nCount;}
#endif

//*********************************************************************************************************
//*********************************************************************************************************


#define emAssert(f)				//Ŀǰ�����壬����������ĵ����޸Ŀɼ�
#define emGridSize(n)	((emSize)(((emSize)(n)+(EM_PTR_GRID-1))/EM_PTR_GRID*EM_PTR_GRID))

/* ������Ԫ�ظ��� */
#define emArrayLen(ar)		(sizeof(ar)/sizeof(*(ar)))
/* ��ṹ��Աƫ�� */
#define emMemberOffset(s,m)		((emSize)&(((s emPtr)0)->m))

typedef struct tagIsObjHeap emCObjHeap, emPtr emPObjHeap;
typedef struct tagIsHeap emCHeap, emPtr emPHeap;

struct tagIsObjHeap
{
	emPObjHeap	m_pPrevious;
	emPByte		m_pFreeEnd;
	emByte		m_pFreeStart[1];
};

struct tagIsHeap
{
	emPObjHeap	m_pObjHeap;
	emPByte		m_pFreeStart, m_pFreeEnd;
	emByte		m_pMemStart[1];
};


/*	ջʽ��  */
void emCall emHeap_InitH( emPHeap pHeap, emSize nSize );
void emCall emHeap_UninitH( emPHeap pHeap );


#if DEBUG_LOG_SWITCH_HEAP
	void emCall LOG_StackAddr(emCharA *pText);
#else
	#define	LOG_StackAddr(p)		((void)0)
#endif



#if DEBUG_LOG_SWITCH_HEAP

	#define emHeap_AllocZero(n,s)			emHeap_AllocZeroH(g_pHeap, n,s)
	#define emHeap_Free(p,n,s)			emHeap_FreeH(g_pHeap, p, n,s)
	#define emObjHeap_AllocZero(n,s)		emObjHeap_AllocZeroH(g_pHeap,n,s)
	#define emObjHeap_Free(p,n,s)		emObjHeap_FreeH(g_pHeap,p,n,s)

	emPointer emCall emHeap_AllocZeroH( emPHeap pHeap, emSize nSize ,emCharA *pText);
	void emCall emHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize,emCharA *pText );
	emPointer emCall emObjHeap_AllocZeroH( emPHeap pHeap, emSize nSize ,emCharA *pText);
	void emCall emObjHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize ,emCharA *pText);

#else

	#define emHeap_AllocZero(n)			emHeap_AllocZeroH(g_pHeap, n)
	#define emHeap_Free(p,n)		emHeap_FreeH(g_pHeap, p, n)
	#define emObjHeap_AllocZero(n)		emObjHeap_AllocZeroH(g_pHeap,n)
	#define emObjHeap_Free(p,n)		emObjHeap_FreeH(g_pHeap,p,n)

	emPointer emCall emHeap_AllocZeroH( emPHeap pHeap, emSize nSize );
	void emCall emHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize );
	emPointer emCall emObjHeap_AllocZeroH( emPHeap pHeap, emSize nSize );
	void emCall emObjHeap_FreeH( emPHeap pHeap, emPointer pBuffer, emSize nSize );
#endif


//******************************** �ڴ������ **********************************************************

#if !EM_ANSI_MEMORY

	void emCall emMemSet( emPointer pBuffer, emByte nValue, emSize nSize );
	void emCall emMemCpy( emPointer pDesc, emCPointer pSrc, emSize nSize );
	void emCall emMemMove( emPointer pDesc, emCPointer pSrc, emSize nSize );
	emComp emCall emMemCmp( emCPointer pDesc, emCPointer pSrc, emSize nSize );

#else

	//#include <memory.h>   #include <string.h> 

	#define emMemSet(p,d,n)	((void)memset((p),d,(emSize)(n)))
	#define emMemCpy(d,s,n)	((void)memcpy((d),(s),(emSize)(n)))
	#define emMemMove(d,s,n)	((void)memmove((d),(s),(emSize)(n)))
	#define emMemCmp(d,s,n)	((emComp)memcmp((d),(s),(emSize)(n)))

#endif

//-----------------------------------------------------------------------------------------------------------------------------------
//��˽���ڴ�������ýṹ����������¼�ڴ濪�ٺ��ͷŵ�˳��
//Ӧ�ó�������Ӧ���ڶ���ģ�飬����ʱ�ã�����ʱȫ���ͷţ�			��ĿǰӦ���ڣ�Ӣ��ǰ�ˣ�Flite��׼��ģ�顿
//�ô����ڴ��ͷ�ʱ��Ҫsize�������ڴ濪���ͷŲ���Ҫ�����Ƚ��ȳ�ԭ��
//�����������ڴ�ռ��������󣨵����ֻ�Ǿֲ�����û�������ڴ��ֵ��Ҳû��ϵ��
#define           MaxMallocMarkCount      (600)		//Ŀǰ���Ե�������417���   //���磺 [g2]1234512345123451234512345123451234512345123451234512345123451234512345123451234512345123451234512345
typedef struct
{
    emUInt32   serial;		//��ţ�����
    emUInt32   addr;		//��ַ
    emUInt32   size;		//��С
}PrivateMallocMark;

typedef struct
{
    emUInt32   serial;		//��ţ�����
    emUInt32   addr;		//��ַ
    emUInt32   size;		//��С
}PrivateFreeMark;

PrivateMallocMark		*g_PrivateMallocMark;
PrivateFreeMark			*g_PrivateFreeMark;
emUInt32				g_PrivateMallocCurMarkCount;	//��¼�ѿ��ٿռ䵫û�ͷŵĴ���
emUInt32				g_PrivateNeedFreeMarkCount;		//��¼��Ҫ�ͷŵ���δ�ܳɹ��ͷŵĴ���

emUInt8 *emHeap_PrivateAllocZero(emUInt32 size);
void	emHeap_PrivateFree(void *Ptr);
void	emHeap_PrivateInit(void);
void	emHeap_PrivateUnInit(void);

#define cst_alloc(TYPE,SIZE)		emHeap_PrivateAllocZero( sizeof(TYPE)*(SIZE) )
#define cst_free(p)					emHeap_PrivateFree( p)

emExtern	FILE *fp_HeapLog;
emExtern	FILE *fp_StackLog;


#ifdef __cplusplus
}
#endif



#endif /* !_EMTTS__EM_HEAP__H */
