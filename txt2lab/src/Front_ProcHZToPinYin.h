#ifndef	_EMTTS__FRONT_PROC_HANZI_TO_PINYIN__H_
#define _EMTTS__FRONT_PROC_HANZI_TO_PINYIN__H_

#include "emCommon.h"
#include "List_Rear_Lab.hxx"



#ifdef __cplusplus
extern "C" {
#endif



#define INTERVAL				9     //索引表中9个字节表示一个多音字/词     add by songkai
#define RULE_LEN				778   //一个多音字/词的最大长度              add by songkai
#define ROW_SIZE				2     //GBK默认拼音表中一行所占大小          add by songkai

#define MAX_POLYFA_PARA3_LEN	200


//以下十七个宏定义是为多音函数而定义的
#define WORD_LEN					1                                      //add by songkai
#define WORD_LEN_GREATE_THAN		2									   //add by songkai
#define WORD_LEN_LESS_THAN			3									   //add by songkai
#define MATCH_WORD					4									   //add by songkai
#define MATCH_CHAR					5								       //add by songkai
#define POS							6								       //add by songkai
#define REAL_END					7									   //add by songkai			
#define POS_END						8									   //add by songkai
#define WORD_END					9									   //add by songkai
#define PRECEDE_VERB				10									   //add by songkai		
#define SUCCEED_VERB				11									   //add by songkai
#define PRECEDE_NOUN				12									   //add by songkai	
#define SUCCEED_NOUN				13								       //add by songkai
#define WORD_BEGIN_WITH				14									   //add by songkai	
#define WORD_END_WITH				15									   //add by songkai
#define LAST_PUNC					16									   //add by songkai
#define NEXT_PUNC					17									   //add by songkai	




void emCall	ToPinYin();

void emCall	ConvertPinYinFromPolyRuler_FromRAM();			                                   //add by songkai

void emCall AddDefaultPinYin();											   //add by songkai

                             



typedef struct PolyFun            //多音函数结构							 add by songkai
{
	emInt8 IsResultTrue;
	emUInt8 IsAnd;
	emInt8 ProbCount;       //最后一个节点优先级是-1
	emUInt8 IsNotBeforeOperation;
	struct PolyFun * next;
}PolyFunAlisa;


typedef struct PolyFunAsisant   //多音函数辅助结构							add by songkai
{
	emUInt8 FunType;
	emUInt8 IsDirectNot;
	emInt16 Para1;   
	emInt16 Para2;
	emInt8  Para3[MAX_POLYFA_PARA3_LEN];
	emUInt16 Zi_Code;		//当前多音字
}PolyFunAsistant;


emBool emCall HandleFun(PolyFunAsistant*);					              //add by songkai
emBool emCall RealHandleRule(PolyFunAlisa* , emUInt16*, emUInt16*, emByte*,emInt16 ,emInt16 );//add by songkai 
emUInt8 emCall GetSpecailNode(emInt16);		                  //add by songkai
emUInt8 emCall GetLastNode();                                  //add by songkai

emInt8 emCall HandleFunWordLen(PolyFunAsistant*);						  //add by songkai
emInt8 emCall HandleFunWordLenGreateThan(PolyFunAsistant*);				  //add by songkai
emInt8 emCall HandleFunWordLenLessThan(PolyFunAsistant*);				  //add by songkai
emInt8 emCall HandleFunMatchWord(PolyFunAsistant*);						  //add by songkai
emInt8 emCall HandleFunMatchChar(PolyFunAsistant*);					      //add by songkai
emInt8 emCall HandleFunPos(PolyFunAsistant*);							  //add by songkai
emInt8 emCall HandleFunRealEnd(PolyFunAsistant*);						  //add by songkai
emInt8 emCall HandleFunPosEnd(PolyFunAsistant*);						  //add by songkai
emInt8 emCall HandleFunWordEnd(PolyFunAsistant*);						  //add by songkai
emInt8 emCall HandleFunPrecedeVerb(PolyFunAsistant*);					  //add by songkai
emInt8 emCall HandleFunSucceedVerb(PolyFunAsistant*);					  //add by songkai
emInt8 emCall HandleFunPrecedeNoun(PolyFunAsistant*);					  //add by songkai
emInt8 emCall HandleFunSucceedNoun(PolyFunAsistant*);					  //add by songkai
emInt8 emCall HandleFunWordBeginWith(PolyFunAsistant*);					  //add by songkai
emInt8 emCall HandleFunWordEndWith(PolyFunAsistant*);					  //add by songkai
emInt8 emCall HandleFunLastPunc(PolyFunAsistant*);			              //add by songkai
emInt8 emCall HandleFunNextPunc(PolyFunAsistant*);						  //add by songkai
emBool emCall HandleCurZiOrCi_FromRAM(emInt16 nLine, emInt16 nCurOffset, emInt16 nMaxHanZi,emPByte  PolyIndexTable,emInt8 *pCurZiCiRule,PolyFunAlisa *pHeadPolyFunAlisaLink);
emUInt8 emCall NeedConvertPos(emUInt8);                                   //add by songkai


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_PROC_HANZI_TO_PINYIN__H_ */