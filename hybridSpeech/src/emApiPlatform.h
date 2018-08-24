//ƽ̨����ͷ�ļ�  Ŀǰƽ̨: Win32 (X86)

#ifndef	_EMTTS__EM_API_PLATFORM__H_
#define _EMTTS__EM_API_PLATFORM__H_


//Ŀ��ƽ̨������Ҫ�Ĺ���ͷ�ļ�

#include <string.h>
//#include <stdio.h>
//#include <crtdbg.h>
//#include <memory.h>
//#include <stdlib.h>
//#include <tchar.h>
//#include <windows.h>



// ���¸���Ŀ��ƽ̨�����޸����������ѡ��

#define EM_UNIT_BITS			8			//�ڴ������Ԫλ��
#define EM_BIG_ENDIAN			0			//�Ƿ��� Big-Endian �ֽ���
#define EM_PTR_GRID				4			//���ָ�����ֵ

#define EM_PTR_PREFIX						//ָ�����ιؼ���(����ȡֵ�� near | far, ����Ϊ��)
#define EM_CONST				const		//�����ؼ���(����Ϊ��)
#define EM_EXTERN				extern		//�ⲿ�ؼ���
#define EM_STATIC				static		//��̬�����ؼ���(����Ϊ��)
#define EM_INLINE				__inline	//�����ؼ���(����ȡֵ�� inline, ����Ϊ��)
#define EM_CALL_STANDARD					//��ͨ�������ιؼ���(����ȡֵ�� __stdcall | __fastcall | __pascal, ����Ϊ��)
#define EM_CALL_REENTRANT		__stdcall	//�ݹ麯�����ιؼ���(����ȡֵ�� stdcall | reentrant, ����Ϊ��)
#define EM_CALL_VAR_ARG			__cdecl		//��κ������ιؼ���(����ȡֵ�� cdecl, ����Ϊ��)

#define EM_TYPE_INT8			char		//8λ��������
#define EM_TYPE_INT16			short		//16λ��������
#define EM_TYPE_INT32			int		//32λ��������


//#define EM_TYPE_INT64			double		//64λ��������


#define EM_TYPE_ADDRESS			size_t		//��ַ��������
#define EM_TYPE_SIZE			size_t		//��С��������

#define EM_ANSI_MEMORY			1			//�Ƿ�ʹ�� ANSI �ڴ������
#define	EM_ANSI_STRING			1			//�Ƿ�ʹ�� ANSI �ַ���������


//����ƽ̨����ѡ������Ƿ��� Unicode ��ʽ����
#if defined(UNICODE) || defined(_UNICODE)
	#define EM_UNICODE			1			//�Ƿ��� Unicode ��ʽ����
#else
	#define EM_UNICODE			1			//�Ƿ��� Unicode ��ʽ����  sqb 2017/6/1
#endif


#endif	// #define _EMTTS__EM_API_PLATFORM__H_ 