#ifndef	_EMTTS__FRONT_CODE_CONVERT__H_
#define _EMTTS__FRONT_CODE_CONVERT__H_

#include "emCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_END_PUNC	10							//结束标点符最大的个数（与数组相关）


emTTSErrID	emCall  CodeConvertToGBK();				//进行编码转换，全部转成GBK编码
emTTSErrID	emCall  ConvertGbkOrGB2312ToGbk(emInt16* pParaEndPuncCodeSet );
emTTSErrID	emCall  ConvertBig5ToGbk(emInt16* pParaEndPuncCodeSet );
emTTSErrID	emCall  ConvertUnicodeToGbk(emInt16* pParaEndPuncCodeSet );

emInt16		emCall	GetWord( emPByte pCurStr, emUInt32  nCurIndex);

void		emCall	FanToJian();					//繁简转换： 对g_hTTS->m_pDataAfterConvert里的数据进行
emBool		emCall	QuDouHao( emPByte pDataOfBasic, emByte nSumOfConverted, emBool flag);
void		emCall	QuKongGe( );


emUInt8 emCall	CheckInfo();
void emCall	 GetChaBoText();

emBool emCall IsDotSplitSen(emByte cL_DataOfBasic,emByte cH_Next, emByte cL_Next,emByte nSumOfConverted);


#ifdef __cplusplus
}
#endif


#endif	/* #define _EMTTS__FRONT_CODE_CONVERT__H_ */