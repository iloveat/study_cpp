#ifndef	_EMTTS__FRONT_CODE_CONVERT__H_
#define _EMTTS__FRONT_CODE_CONVERT__H_

#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_END_PUNC	10							//�����������ĸ�������������أ�


emTTSErrID	emCall  CodeConvertToGBK();				//���б���ת����ȫ��ת��GBK����
emTTSErrID	emCall  ConvertGbkOrGB2312ToGbk(emInt16* pParaEndPuncCodeSet );
emTTSErrID	emCall  ConvertBig5ToGbk(emInt16* pParaEndPuncCodeSet );
emTTSErrID	emCall  ConvertUnicodeToGbk(emInt16* pParaEndPuncCodeSet );

emInt16		emCall	GetWord( emPByte pCurStr, emUInt32  nCurIndex);

void		emCall	FanToJian();					//����ת���� ��g_hTTS->m_pDataAfterConvert������ݽ���
emBool		emCall	QuDouHao( emPByte pDataOfBasic, emByte nSumOfConverted, emBool flag);
void		emCall	QuKongGe( );


emUInt8 emCall	CheckInfo();
void emCall	 GetChaBoText();

emBool emCall IsDotSplitSen(emByte cL_DataOfBasic,emByte cH_Next, emByte cL_Next,emByte nSumOfConverted);


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_CODE_CONVERT__H_ */