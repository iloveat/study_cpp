

#ifndef	_EMTTS__FRONT_PROC_HANZI_RHYTHM_OLD__H_
#define _EMTTS__FRONT_PROC_HANZI_RHYTHM_OLD__H_


#include "emCommon.h"


#ifdef __cplusplus
extern "C" {
#endif


#if	!EM_SYS_SWITCH_RHYTHM_NEW

#if 1
#define  PPH_BORDER_LEN_FOR_FAZHI			6		//6�����ϵ��õͷ�ֵ��6�ֻ����µ��ø߷�ֵ

#define	MAX_PW_LEN_LIMIT				4				//�����PW����󳤶�
#define	MAX_PPH_LEN_LIMIT				12				//�����PPH����󳤶�

#define LASTBOUNDARYTYPE     9       //add by songkai
#define POS_ADD_1            1		 //add by songkai
#define POS_ADD_2            2	     //add by songkai
#define POS_SUB_1            3	     //add by songkai
#define POS_SUB_2            4	     //add by songkai
#define WLen_ADD_1           5	     //add by songkai
#define WLen_SUB_1           6	     //add by songkai
#define DBACK                7	     //add by songkai
#define DFRONT               8	     //add by songkai
#endif

#if 0
#define LASTBOUNDARYTYPE     1       //add by songkai
#define POS_ADD_1            2		 //add by songkai
#define POS_ADD_2            3	     //add by songkai
#define POS_SUB_1            4	     //add by songkai
#define POS_SUB_2            5	     //add by songkai
#define WLen_ADD_1           6	     //add by songkai
#define WLen_SUB_1           7	     //add by songkai
#define DBACK                8	     //add by songkai
#define DFRONT               9	     //add by songkai
#endif

#define START		       126       //add by songkai
#define END			       127		 //add by songkai

//PW��������
#define PW_PRO_NUM          7
#define PW_PRO_VALUE        2
#define PW_LINE_NUM       479		 //add by songkai
#define PW_LINE_BYTE       14        //add by songkai


//#define PPH_LINE_NUM    14578	     //add by songkai
#define PPH_LINE_BYTE       4        //add by songkai
#define PPH_PRO_NUM		    9		 //add by songkai
#define PPH_PRO_VALUE       4		 //add by songkai


//PPH���е�һ���ֵ�����---ÿ�����͵���ʼ��(2�ֽ�)��ռ�ݵ�����(2�ֽ�)
#define PPH_TABLE_START_POS_ADD_1						   0                                 //add by songkai
#define PPH_TABLE_LINES_POS_ADD_1						6448											
#define PPH_TABLE_START_POS_ADD_2						6448
#define PPH_TABLE_LINES_POS_ADD_2						2626
#define PPH_TABLE_START_POS_SUB_1						9074
#define PPH_TABLE_LINES_POS_SUB_1						3103
#define PPH_TABLE_START_POS_SUB_2					   12177
#define PPH_TABLE_LINES_POS_SUB_2						2175
#define PPH_TABLE_START_WLEN_ADD_1					   14352
#define PPH_TABLE_LINES_WLEN_ADD_1                        26
#define PPH_TABLE_START_WLEN_SUB_1                     14378
#define PPH_TABLE_LINES_WLEN_SUB_1			              26
#define PPH_TABLE_START_DBACK		                   14404
#define PPH_TABLE_LINES_DBACK		                      86
#define PPH_TABLE_START_DFRONT                         14490
#define PPH_TABLE_LINES_DFRONT			                  86
#define PPH_TABLE_START_LASTBOUNDARY                   14576
#define PPH_TABLE_LINES_LASTBOUNDARY                       2								//add by songkai

//PPH�����������ָ��Եĳ���(�ֽ�)
#define PPH_TABLE_LEN_PART_1							  36
#define PPH_TABLE_LEN_PART_2						   58312
#define PPH_TABLE_LEN_PART_3						  116624							    //add by songkai


//����������ɱ߽���
#define MAX_NUM                                           30

void emCall Rhythm_Old();				//add by songkai
void emCall EvalPW(float);			//add by songkai
void emCall EvalPPH(float, float);		    //add by songkai
float emCall FindMax(float, float, float);													//add by songkai							

emUInt8 emCall MostNearThreeNode(emUInt8 , emUInt8 );		//add by songkai


emInt32  AddBaseAddressForThree(emInt32 );
emInt32  AddBaseAddressForSix(emInt32 );
emBool IsHaveTeZhengCi( emUInt8 )	;

#endif	// EM_SYS_SWITCH_RHYTHM_NEW


#ifdef __cplusplus
}
#endif

#endif	/* #define _EMTTS__FRONT_PROC_HANZI_RHYTHM_OLD__H_ */


