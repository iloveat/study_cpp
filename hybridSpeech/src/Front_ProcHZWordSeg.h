#ifndef	_EMTTS__FRONT_PROC_HANZI_WORD_SEG__H_
#define _EMTTS__FRONT_PROC_HANZI_WORD_SEG__H_


#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif


#define		WORDSEG_DEBUG_DISPLAY_TEXT				0			//������	<�ִ�>����ʱ��ʾ�ִ��ı���Ϣ�����Ǳ����


#define		MAX_WORD_LEN_OF_DICT					4			//Ŀǰ�ʵ������Ϊ4�ִ�
#define		MAX_SEARCH_WORD_LEN_OF_DICT				4			//�����дӴʵ������������Ĵʳ�Ϊ4��
#define		GB2312_DICT_INDEX_LINE_LEN				9			//�ʵ��GB2312������ÿ�е��г�

//ע�⣺����4��ֵ���롶�ʵ���͡���Ԫ�ķ�����ratioһ����������1000��
#define		RATIO_OF_POS_G_MAKED					(-6000)		//��ǰ�ֹ��첻�˴���ʱ����Ϊ����һ�����ִ�ʱ������ratio
#define		MIN_EXPENSE								(-10000000)	//��ʼ������С����
#define		RATIO_NEED_CUT_OF_TWO_NAME				3500		//������������ʱ���������ratioֵ
#define		RATIO_NEED_CUT_OF_THREE_NAME			7000		//������������ʱ���������ratioֵ

#define		MAX_COUNT_WordSegPinYin					2000

#define  BYTE_NO_OF_TABLE_SURNAME_ONE_LINE		8			//���ϱ��ÿ�е��ֽ���


struct WordItem    
{
#if	WORDSEG_DEBUG_DISPLAY_TEXT	
    emByte              pAAAAAAText[20];	//����������������			�����Բ�Ҫ�������Ԫ�ķ�ʱ��Pos���ɣ��������м�¼�ʳ����㹻�ˣ�����ʱ���ţ�
#endif
    emUInt8              nPosIndex;			//�����������Ա���
    emUInt8              nCharNum;			//���������ʳ�
    emInt16              nRatio;			//��������Ƶ�ʱ�  ��ԭ���Ļ����� ������1000��
    emInt16              *pPinYin;  		//�����������������ƴ����  ���ֽڵĸ�1λ��ƴ�����ͣ��� ����15λ��ƴ�����룩  (ƴ�����ͣ� 0��Ĭ�� ��1��ǿ��ƴ����)
    emInt32              nExpense;			//������������= �������Ĵ�Ƶ��ratio + ��Ԫ�ķ�ת�Ƹ���	 ��ԭ���Ļ����� ������1000��
    emUInt8              nPrevNode;			//�����������ڵ�i�ı�����j��ǰһ�������ڵĽڵ�ĵ�����ֵX��ǰ1����g_vecWordBuffer[X]->item[Y]��
    emUInt8              nPrevItem;			//�����������ڵ�i�ı�����j��ǰһ�����ĵ�����ֵY��ǰ1����g_vecWordBuffer[X]->item[Y]��
};


//Ŀǰ����Ŀռ�Ϊ��   50    * ��  4   +      4    *       20         +       12          *        20         �� = 16200 �ֽ�
//					���ָ���  nItemCount  itemָ��   MAX_CITIAO_COUNT    WordItem���ֽ���   MAX_CITIAO_COUNT
struct WordBuffer     //��24 K �ֽ�-- ���ִ�ģ����Ҫ��       //���50����
{
    emInt8              nItemCount;      				//ʵ�ʴ�������
    struct WordItem		item[MAX_CITIAO_COUNT];			//����     
};                      

void emCall WordSeg_third();
void emCall WordSeg();
void emCall WordSegInit();
void emCall WordSegUnInit();
void emCall MatchWordsOfCurNode();
void emCall AddUserDictWord();	//�����û��ʵ���Դ�еĴ���
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
