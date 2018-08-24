//平台配置头文件  目前平台: Win32 (X86)

#ifndef	_EMTTS__EM_API_PLATFORM__H_
#define _EMTTS__EM_API_PLATFORM__H_


//目标平台程序需要的公共头文件

#include <string.h>
//#include <stdio.h>
//#include <crtdbg.h>
//#include <memory.h>
//#include <stdlib.h>
//#include <tchar.h>
//#include <windows.h>



// 以下根据目标平台特性修改下面的配置选项

#define EM_UNIT_BITS			8			//内存基本单元位数
#define EM_BIG_ENDIAN			0			//是否是 Big-Endian 字节序
#define EM_PTR_GRID				4			//最大指针对齐值

#define EM_PTR_PREFIX						//指针修饰关键字(典型取值有 near | far, 可以为空)
#define EM_CONST				const		//常量关键字(可以为空)
#define EM_EXTERN				extern		//外部关键字
#define EM_STATIC				static		//静态函数关键字(可以为空)
#define EM_INLINE				__inline	//内联关键字(典型取值有 inline, 可以为空)
#define EM_CALL_STANDARD					//普通函数修饰关键字(典型取值有 __stdcall | __fastcall | __pascal, 可以为空)
#define EM_CALL_REENTRANT		__stdcall	//递归函数修饰关键字(典型取值有 stdcall | reentrant, 可以为空)
#define EM_CALL_VAR_ARG			__cdecl		//变参函数修饰关键字(典型取值有 cdecl, 可以为空)

#define EM_TYPE_INT8			char		//8位数据类型
#define EM_TYPE_INT16			short		//16位数据类型
#define EM_TYPE_INT32			int		//32位数据类型


//#define EM_TYPE_INT64			double		//64位数据类型


#define EM_TYPE_ADDRESS			size_t		//地址数据类型
#define EM_TYPE_SIZE			size_t		//大小数据类型

#define EM_ANSI_MEMORY			1			//是否使用 ANSI 内存操作库
#define	EM_ANSI_STRING			1			//是否使用 ANSI 字符串操作库


//根据平台编译选项决定是否以 Unicode 方式构建
#if defined(UNICODE) || defined(_UNICODE)
	#define EM_UNICODE			1			//是否以 Unicode 方式构建
#else
	#define EM_UNICODE			1			//是否以 Unicode 方式构建  sqb 2017/6/1
#endif


#endif	// #define _EMTTS__EM_API_PLATFORM__H_ 