#ifndef	_EMTTS__FRONT_PROC_HANZI_WORD_SEG__H_
#define _EMTTS__FRONT_PROC_HANZI_WORD_SEG__H_


#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


#define		WORDSEG_DEBUG_DISPLAY_TEXT				0			//开启：	<分词>调试时显示分词文本信息，不是必须的


#define		MAX_WORD_LEN_OF_DICT					4			//目前词典里最多为4字词
#define		MAX_SEARCH_WORD_LEN_OF_DICT				4			//程序中从词典里搜索的最大的词长为4。
#define		GB2312_DICT_INDEX_LINE_LEN				9			//词典的GB2312索引的每行的行长

//注意：以下4个值，与《词典表》和《二元文法表》的ratio一样，都乘了1000倍
#define		RATIO_OF_POS_G_MAKED					(-6000)		//当前字构造不了词组时，人为构造一个单字词时给定的ratio
#define		MIN_EXPENSE								(-10000000)	//初始化的最小开销
#define		RATIO_NEED_CUT_OF_TWO_NAME				3500		//构造两字姓名时，需减掉的ratio值
#define		RATIO_NEED_CUT_OF_THREE_NAME			7000		//构造三字姓名时，需减掉的ratio值

#define		MAX_COUNT_WordSegPinYin					2000

#define  BYTE_NO_OF_TABLE_SURNAME_ONE_LINE		8			//姓氏表的每行的字节数


struct WordItem    
{
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
    emByte              pAAAAAAText[20];	//本词条：词组内容			（可以不要，计算二元文法时有Pos即可，本词条中记录词长就足够了，调试时留着）
#endif
    emUInt8              nPosIndex;			//本词条：词性编码
    emUInt8              nCharNum;			//本词条：词长
    emInt16              nRatio;			//本词条：频率比  在原来的基础上 都乘了1000倍
    emInt16              *pPinYin;  		//本词条：多音词组的拼音：  高字节的高1位（拼音类型）； 其它15位（拼音编码）  (拼音类型： 0：默认 ；1：强制拼音；)
    emInt32              nExpense;			//本词条：开销= 本词条的词频比ratio + 二元文法转移概率	 在原来的基础上 都乘了1000倍
    emUInt8              nPrevNode;			//本词条：本节点i的本词条j的前一词条所在的节点的的索引值X（前1词条g_vecWordBuffer[X]->item[Y]）
    emUInt8              nPrevItem;			//本词条：本节点i的本词条j的前一词条的的索引值Y（前1词条g_vecWordBuffer[X]->item[Y]）
};


//目前分配的空间为：   50    * （  4   +      4    *       20         +       12          *        20         ） = 16200 字节
//					汉字个数  nItemCount  item指针   MAX_CITIAO_COUNT    WordItem的字节数   MAX_CITIAO_COUNT
struct WordBuffer     //（24 K 字节-- 仅分词模块需要）       //最多50个字
{
    emInt8              nItemCount;      				//实际词条总数
    struct WordItem		item[MAX_CITIAO_COUNT];			//词条     
};                      

void emCall WordSeg_third();
void emCall WordSeg();
void emCall WordSegInit();
void emCall WordSegUnInit();
void emCall MatchWordsOfCurNode();
void emCall AddUserDictWord();	//增加用户词典资源中的词条
void emCall AmbiguityAnalysis(emUInt8 nMaxOfHanZi, emInt16  iPrevPOSIndex, emInt16  iNextPOSIndex);
void emCall SegCurNodeToManyNode(emUInt8 nMaxOfHanZi);
void emCall GetAllCiTiaoOfCurZi(emPByte pMem_Save_CurNodeCiTiao ,emUInt32 *nOffsetMore,emUInt16 *nLenMore,emUInt32 nCiAllLen , emUInt8 nNo );
void emCall GetXingCiTiaoOfCurZi(emUInt8 nNo, struct WordItem  *xingItem1, struct WordItem  *xingItem2 );

void emCall ReadBigramMoreValue(emUInt8 nPrevPos);
emInt16 emCall GetBigramOneValue(emUInt8 nNextPos);


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_PROC_HANZI_WORD_SEG__H_ */
