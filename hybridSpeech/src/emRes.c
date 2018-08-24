#include "emPCH.h"

#include "emRes.h"

#if ANKAI_TOTAL_SWITCH
	#include <stdarg.h>
#endif

struct tagRes g_Res;

void GetTotalResOffset()
{



//《客户信息表》资源ID：	
emStatic emConst struct tagGUID  ResGUID_CompanyInfo = 
	{ 0x1397A2B7, 0x6099, 0x4433, { 0xBE, 0x44, 0xEC, 0xD0, 0x51, 0xC9, 0x2F, 0x10 } };


//《试用版插播表》资源ID：	
emStatic emConst struct tagGUID  ResGUID_InsertPlayInfo = 
	{ 0x2946FF28, 0x533A, 0x4df9, { 0xB2, 0xBC, 0x59, 0xA7, 0xE1, 0xE8, 0xE6, 0x1E } };



//《汉字符号BIG5转GBK表》资源ID：	{99F5D2E2-CFFB-4206-94A2-F8344714DDC8}
emStatic emConst struct tagGUID  ResGUID_CodeBig5ToGbk = 
	{ 0x99f5d2e2, 0xcffb, 0x4206, { 0x94, 0xa2, 0xf8, 0x34, 0x47, 0x14, 0xdd, 0xc8 } };
//《汉字Unicode转GBK表》资源ID：	{02BDF194-EE13-4027-9DB7-349F8F995C09}
emStatic emConst struct tagGUID  ResGUID_CodeHanziUnicodeToGbk = 
	{ 0x2bdf194, 0xee13, 0x4027, { 0x9d, 0xb7, 0x34, 0x9f, 0x8f, 0x99, 0x5c, 0x9 } };
//《符号Unicode转GBK表》资源ID：	{6FB63C7B-F2D3-4738-85C6-C80963D4CB98}
emStatic emConst struct tagGUID  ResGUID_CodeFuhaoUnicodeToGbk = 
	{ 0x6fb63c7b, 0xf2d3, 0x4738, { 0x85, 0xc6, 0xc8, 0x9, 0x63, 0xd4, 0xcb, 0x98 } };



//《汉字量词表》资源ID：	{F9DDD963-B1BF-4e34-96B0-073CB7D4CB0A}
emStatic emConst struct tagGUID  ResGUID_HanZiLiangCi = 
	{ 0xf9ddd963, 0xb1bf, 0x4e34, { 0x96, 0xb0, 0x7, 0x3c, 0xb7, 0xd4, 0xcb, 0xa } };
//《英文量词转汉字表》资源ID：	{0E00C6FB-6477-4a67-A9A1-6FDAF696EDC3}
emStatic emConst struct tagGUID  ResGUID_EnglishLiangCiToHanZi = 
	{ 0xe00c6fb, 0x6477, 0x4a67, { 0xa9, 0xa1, 0x6f, 0xda, 0xf6, 0x96, 0xed, 0xc3 } };
//《汉字因素让数字读成号码表》资源ID：	{1118B95F-0D8E-4d00-BA6C-A7BFBA55020C}
emStatic emConst struct tagGUID  ResGUID_HanZiLetDigitToPhone = 
	{ 0x1118b95f, 0xd8e, 0x4d00, { 0xba, 0x6c, 0xa7, 0xbf, 0xba, 0x55, 0x2, 0xc } };
//《GBK符号转汉字表》资源ID：	{C34369FC-38EC-4418-B27A-387E15DDA95B}
emStatic emConst struct tagGUID  ResGUID_GbkFuhaoToHanzi = 
	{ 0xc34369fc, 0x38ec, 0x4418, { 0xb2, 0x7a, 0x38, 0x7e, 0x15, 0xdd, 0xa9, 0x5b } };
//《GBK默认拼音表》资源ID：	{9B368CAF-3708-4640-8089-CF30CAD6FD81}
emStatic emConst struct tagGUID  ResGUID_GBKDefaultPinyin = 
	{ 0x9b368caf, 0x3708, 0x4640, { 0x80, 0x89, 0xcf, 0x30, 0xca, 0xd6, 0xfd, 0x81 } };
//《多音字词规则库表索引》资源ID：	{B1BF675B-CB11-4a0a-8D5B-2098C827B2C5}
emStatic emConst struct tagGUID  ResGUID_PolyRuleIndex = 
	{ 0xb1bf675b, 0xcb11, 0x4a0a, { 0x8d, 0x5b, 0x20, 0x98, 0xc8, 0x27, 0xb2, 0xc5 } };
//《多音字词规则库表》资源ID：	{0062399E-7C28-4ebe-8811-545DD0899372}
emStatic emConst struct tagGUID  ResGUID_PolyRule = 
	{ 0x62399e, 0x7c28, 0x4ebe, { 0x88, 0x11, 0x54, 0x5d, 0xd0, 0x89, 0x93, 0x72 } };
//《拼音查询索引表》资源ID：	{092D799E-B8BF-4b7b-9194-C9F075509262}
emStatic emConst struct tagGUID  ResGUID_PinYinInquireIndex = 
	{ 0x92d799e, 0xb8bf, 0x4b7b, { 0x91, 0x94, 0xc9, 0xf0, 0x75, 0x50, 0x92, 0x62 } };
//《拼音查询表》资源ID：	{D866E2E2-A6E8-4345-B9B5-48C47B0C50CF}
emStatic emConst struct tagGUID  ResGUID_PinYinInquire = 
	{ 0xd866e2e2, 0xa6e8, 0x4345, { 0xb9, 0xb5, 0x48, 0xc4, 0x7b, 0xc, 0x50, 0xcf } };

//《文法转移概率表》资源ID：	{4E5B843B-3BF1-49d3-97DC-C6EF686FF2C8}
emStatic emConst struct tagGUID  ResGUID_ConvertRatioOfGram = 
	{ 0x4e5b843b, 0x3bf1, 0x49d3, { 0x97, 0xdc, 0xc6, 0xef, 0x68, 0x6f, 0xf2, 0xc8 } };
//《姓氏表》资源ID：	{58FE25BE-1EB6-4814-B574-20863C9742A8}
emStatic emConst struct tagGUID  ResGUID_SurnameRule = 
	{ 0x58fe25be, 0x1eb6, 0x4814, { 0xb5, 0x74, 0x20, 0x86, 0x3c, 0x97, 0x42, 0xa8 } };

//《GB2312词条偏移表》资源ID：	{750AFF02-E8AE-4402-918A-A2106C4A1BA1}
emStatic emConst struct tagGUID  ResGUID_DictCnPtGB2312Index = 
	{ 0x750aff02, 0xe8ae, 0x4402, { 0x91, 0x8a, 0xa2, 0x10, 0x6c, 0x4a, 0x1b, 0xa1 } };
//《词典表》资源ID：	{ACEDBB1D-0138-4863-B837-42CBD8EF47EA}
emStatic emConst struct tagGUID  ResGUID_DictCnPt = 
	{ 0xacedbb1d, 0x138, 0x4863, { 0xb8, 0x37, 0x42, 0xcb, 0xd8, 0xef, 0x47, 0xea } };

//《用户词典表》资源ID：{D16E1E73-9E1F-430e-8F98-9F955FFC4A86}
emStatic emConst struct tagGUID  ResGUID_DictCnUser = 
{ 0xd16e1e73, 0x9e1f, 0x430e, { 0x8f, 0x98, 0x9f, 0x95, 0x5f, 0xfc, 0x4a, 0x86 } };


//《后端音库》资源ID：	{1999DC00-35D5-489a-9C10-311BEBC5AF9D}								
emStatic emConst struct tagGUID  ResGUID_RearVoiceLib01 =									
	{ 0x1999dc00, 0x35d5, 0x489a, { 0x9c, 0x10, 0x31, 0x1b, 0xeb, 0xc5, 0xaf, 0x9d } };


//《后端音库》资源ID：	{FCA66E20-B7EF-4719-8DAC-3B317822F9C9}
emStatic emConst struct tagGUID  ResGUID_RearVoiceLib03 =									
	{ 0xfca66e20, 0xb7ef, 0x4719, { 0x8d, 0xac, 0x3b, 0x31, 0x78, 0x22, 0xf9, 0xc9 } };

//《后端音库》资源ID：	{D22C81D3-E599-4ef4-A599-1D5CDAE97F95}
emStatic emConst struct tagGUID  ResGUID_RearVoiceLib_EN =									
	{ 0xD22C81D3, 0xE599, 0x4ef4, { 0xA5, 0x99, 0x1D, 0x5C, 0xDA, 0xE9, 0x7F, 0x95 } };


//《提示音偏移长度表》资源ID：	{CB3E7428-579B-48c7-987D-F95106D15C9B}
emStatic emConst struct tagGUID  ResGUID_PromptIndex = 
	{ 0xcb3e7428, 0x579b, 0x48c7, { 0x98, 0x7d, 0xf9, 0x51, 0x6, 0xd1, 0x5c, 0x9b } };


// 《英文前端：GBK符号转英文表》资源ID：	{E763881D-FD89-4a7b-95C6-FD2E88C5B79C}
emStatic emConst struct tagGUID  ResGUID_EnF_GbksymbolToEn = 
{ 0xe763881d, 0xfd89, 0x4a7b, { 0x95, 0xc6, 0xfd, 0x2e, 0x88, 0xc5, 0xb7, 0x9c } };


#if EM_SYS_SWITCH_RHYTHM_NEW

//《PW规则》资源ID：	{51DDD15C-22B3-4272-8642-CBE2F9A847A7}
emStatic emConst struct tagGUID  ResGUID_PWRule_New = 
	{ 0x51DDD15C, 0x22B3, 0x4272, { 0x86, 0x42, 0xCB, 0xE2, 0xF9, 0xA8, 0x47, 0xA7 } };

//《PPHC45规则》资源ID：	{2E2FE9A6-632E-40ff-B7BD-8947D3E04CB4}
emStatic emConst struct tagGUID  ResGUID_PPHC45_New= 
{ 0x2e2fe9a6, 0x632e, 0x40ff, { 0xb7, 0xbd, 0x89, 0x47, 0xd3, 0xe0, 0x4c, 0xb4 } };

//《PPHRatio规则》资源ID：	{4DD8695D-637B-42c0-8543-A51699BA14EA}
emStatic emConst struct tagGUID  ResGUID_PPHRatio_New= 
{ 0x4dd8695d, 0x637b, 0x42c0, { 0x85, 0x43, 0xa5, 0x16, 0x99, 0xba, 0x14, 0xea } };

#else

//《PW韵律转移概率表》资源ID：	{908C3271-7250-4de9-BD59-9501C3FC141F}
emStatic emConst struct tagGUID  ResGUID_ConvertRatioOfPW = 
	{ 0x908c3271, 0x7250, 0x4de9, { 0xbd, 0x59, 0x95, 0x1, 0xc3, 0xfc, 0x14, 0x1f } };
//《PPH韵律转移概率表》资源ID：	{9DD87F0F-A059-4cc2-AACA-D57433D5A16E}
emStatic emConst struct tagGUID  ResGUID_ConvertRatioOfPPH = 
	{ 0x9dd87f0f, 0xa059, 0x4cc2, { 0xaa, 0xca, 0xd5, 0x74, 0x33, 0xd5, 0xa1, 0x6e } };

#endif


//《繁体转简体表》资源ID：	{633A1DBE-6A2A-4194-B518-A00BA8838B76}
emStatic emConst struct tagGUID  ResGUID_FanToJian= 
{ 0x633A1DBE, 0x6A2A, 0x4194, { 0xb5, 0x18, 0xa0, 0x0b, 0xa8, 0x83, 0x8b, 0x76 } };

//********************************** 以下9个表：纯英文前端（不含数字等处理）
// {4B8B5553-FB88-41af-ABFB-20A87409471F}
emStatic emConst struct tagGUID  ResGUID_EnPF_DictNormal =
{ 0x4b8b5553, 0xfb88, 0x41af, { 0xab, 0xfb, 0x20, 0xa8, 0x74, 0x09, 0x47, 0x1f } };

// {6DDD7E81-6F3F-47aa-91DA-22B580486CBB}
emStatic emConst struct tagGUID  ResGUID_EnPF_DictUser =
{ 0x6ddd7e81, 0x6f3f, 0x47aa, { 0x91, 0xda, 0x22, 0xb5, 0x80, 0x48, 0x6c, 0xbb } };

// {AD90D8F4-52F9-48f2-AC6B-3FBAB38A1E65}
emStatic emConst struct tagGUID  ResGUID_EnPF_LTSaz =
{ 0xad90d8f4, 0x52f9, 0x48f2, { 0xac, 0x6b, 0x3f, 0xba, 0xb3, 0x8a, 0x1e, 0x65 } }; 

// {F0DEAF22-35E6-440c-9E79-A0A739AD48C3}
emStatic emConst struct tagGUID  ResGUID_EnPF_LTSStress =
{ 0xf0deaf22, 0x35e6, 0x440c, { 0x9e, 0x79, 0xa0, 0xa7, 0x39, 0xad, 0x48, 0xc3 } };

// {8F65C611-C259-4c2f-B3B7-F18D780B13F8}
emStatic emConst struct tagGUID  ResGUID_EnPF_PosTernar =
{ 0x8f65c611, 0xc259, 0x4c2f, { 0xb3, 0xb7, 0xf1, 0x8d, 0x78, 0xb, 0x13, 0xf8 } };

// {9907596F-689E-49f8-945A-7BC7E3BB7586}
emStatic emConst struct tagGUID  ResGUID_EnPF_L3DistribProb =
{ 0x9907596f, 0x689e, 0x49f8, { 0x94, 0x5a, 0x7b, 0xc7, 0xe3, 0xbb, 0x75, 0x86 } };

// {10ACEFD5-EB7E-4c13-A3F1-1F727A880128}
emStatic emConst struct tagGUID  ResGUID_EnPF_L3C45 =
{ 0x10acefd5, 0xeb7e, 0x4c13, { 0xa3, 0xf1, 0x1f, 0x72, 0x7a, 0x88, 0x1, 0x28 } };

// {3826CA7E-F166-44b7-B316-031782B36A9A}
emStatic emConst struct tagGUID  ResGUID_EnPF_ToBIAccentC45 =
{ 0x3826ca7e, 0xf166, 0x44b7, { 0xb3, 0x16, 0x03, 0x17, 0x82, 0xb3, 0x6a, 0x9a } };

// {22A880A1-B13F-4cb2-9C60-C6C364400A31}
emStatic emConst struct tagGUID  ResGUID_EnPF_ToBIToneC45 = 
{ 0x22a880a1, 0xb13f, 0x4cb2, { 0x9c, 0x60, 0xc6, 0xc3, 0x64, 0x40, 0x0a, 0x31 } };


	emInt32 resTotalCount,resMaxCount;
	emPByte pRes_Table = NULL;

	//LOG_StackAddr(__FUNCTION__);


	fFrontSeek(g_hTTS->fResFrontMain,12 ,0);
	fFrontRead(&resTotalCount,sizeof(emInt32 ),1, g_hTTS->fResFrontMain);
	resMaxCount = resTotalCount;

	
#if DEBUG_LOG_SWITCH_HEAP
	pRes_Table = (emPByte)emHeap_AllocZero( resMaxCount * 24, "整个资源信息：《资源模块  》" );		
#else
	pRes_Table = (emPByte)emHeap_AllocZero( resMaxCount * 24 );		
#endif
	

	fFrontRead(pRes_Table, resTotalCount * 24, 1, g_hTTS->fResFrontMain);	//一次性将所有资源的（GUID，偏移量，长度）全部读入到内存

	
	g_Res.offset_CodeBig5ToGbk = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_CodeBig5ToGbk);
	g_Res.offset_CodeHanziUnicodeToGbk = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_CodeHanziUnicodeToGbk);
	g_Res.offset_CodeFuhaoUnicodeToGbk = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_CodeFuhaoUnicodeToGbk);
	g_Res.offset_HanZiLiangCi = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_HanZiLiangCi);
	g_Res.offset_EnglishLiangCiToHanZi = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnglishLiangCiToHanZi);
	g_Res.offset_HanZiLetDigitToPhone = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_HanZiLetDigitToPhone);
	g_Res.offset_GbkFuhaoToHanzi = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_GbkFuhaoToHanzi);
	g_Res.offset_GBKDefaultPinyin = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_GBKDefaultPinyin);
	g_Res.offset_PolyRuleIndex= GetOneResOffset(pRes_Table, resTotalCount, ResGUID_PolyRuleIndex);
	g_Res.offset_PolyRule = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_PolyRule);
	g_Res.offset_PinYinInquireIndex = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_PinYinInquireIndex);
	g_Res.offset_PinYinInquire = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_PinYinInquire);
	g_Res.offset_ConvertRatioOfGram = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_ConvertRatioOfGram);
	g_Res.offset_SurnameRule = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_SurnameRule);
	g_Res.offset_DictCnPtGB2312Index = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_DictCnPtGB2312Index);
	g_Res.offset_DictCnPt = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_DictCnPt);
	g_Res.offset_DictCnUser = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_DictCnUser);
	g_Res.offset_PromptIndex = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_PromptIndex);

#if EM_SYS_SWITCH_RHYTHM_NEW
	g_Res.offset_PWRule_New = GetOneResOffset(pRes_Table, resTotalCount,   ResGUID_PWRule_New);
	g_Res.offset_PPHC45_New = GetOneResOffset(pRes_Table, resTotalCount,   ResGUID_PPHC45_New);
	g_Res.offset_PPHRatio_New = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_PPHRatio_New);
#else
	g_Res.offset_ConvertRatioOfPW = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_ConvertRatioOfPW);
	g_Res.offset_ConvertRatioOfPPH = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_ConvertRatioOfPPH);
#endif

	g_Res.offset_FanToJian = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_FanToJian);
	g_Res.offset_CompanyInfo = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_CompanyInfo);
	g_Res.offset_InsertPlayInfo = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_InsertPlayInfo);

	g_Res.offset_RearVoiceLib01 = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_RearVoiceLib01);
	g_Res.offset_RearVoiceLib03 = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_RearVoiceLib03);
	g_Res.offset_RearVoiceLib_Eng = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_RearVoiceLib_EN);

	//英文前端
	g_Res.offset_EnF_GbksymbolToEn = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnF_GbksymbolToEn);

	//纯英文前端（不含数字等处理）
	g_Res.offset_EnPF_PosTernar         = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_PosTernar);
	g_Res.offset_EnPF_DictNormal        = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_DictNormal);
	g_Res.offset_EnPF_DictUser          = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_DictUser);
	g_Res.offset_EnPF_L3C45             = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_L3C45);
	g_Res.offset_EnPF_L3DistribProb     = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_L3DistribProb);
	g_Res.offset_EnPF_LTSStress         = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_LTSStress);
	g_Res.offset_EnPF_LTSaz             = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_LTSaz);
	g_Res.offset_EnPF_ToBIAccentC45     = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_ToBIAccentC45);
	g_Res.offset_EnPF_ToBIToneC45       = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_EnPF_ToBIToneC45);

#if EM_RES_READ_REAR_LIB_CONST
	fRearSeek(g_hTTS->fResCurRearMain,12 ,0);
	fRearRead(&resTotalCount,sizeof(emInt32 ),1, g_hTTS->fResCurRearMain);

	fRearRead(pRes_Table, resTotalCount * 24, 1, g_hTTS->fResCurRearMain);	//一次性将所有资源的（GUID，偏移量，长度）全部读入到内存


	g_Res.offset_RearVoiceLib01 = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_RearVoiceLib01);
	g_Res.offset_RearVoiceLib03 = GetOneResOffset(pRes_Table, resTotalCount, ResGUID_RearVoiceLib03);
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRes_Table , resMaxCount * 24, "整个资源信息：《资源模块  》" );		
#else
	emHeap_Free( pRes_Table,  resMaxCount * 24 );		
#endif

             


}


emInt32 GetOneResOffset(emPByte pResTablePara, emInt32 resTotalCountPara, struct tagGUID  sResID)
{
	emInt32 i,j,nOffset ;
	struct tagGUID tResID;

	LOG_StackAddr(__FUNCTION__);

	for(i=0; i<resTotalCountPara; i++)
	{
		emMemCpy(&(tResID.Data1),pResTablePara + 24*i + 0   ,sizeof(tResID.Data1));
		emMemCpy(&(tResID.Data2),pResTablePara + 24*i + 4   ,sizeof(tResID.Data2));
		emMemCpy(&(tResID.Data3),pResTablePara + 24*i + 6   ,sizeof(tResID.Data3));
		for(j=0;j<8;j++)
		{
			emMemCpy(&(tResID.Data4[j]),pResTablePara + 24*i + 8 + j  ,1);
		}
		if( IsSameGUID(&sResID, &tResID))
		{
			emMemCpy(&nOffset,pResTablePara + 24*i + 16 ,4);
			return nOffset;
		}
	}
	return	RES_OFFSET_ERROR;
}

emBool IsSameGUID(struct tagGUID  * pGuid1, struct tagGUID  * pGuid2)
{
	emUInt8 i;

	LOG_StackAddr(__FUNCTION__);

	if ( pGuid1->Data1 != pGuid2->Data1 )
		return emFalse;

	if ( pGuid1->Data2 != pGuid2->Data2 )
		return emFalse;

	if ( pGuid1->Data3 != pGuid2->Data3 )
		return emFalse;

	for ( i = 0; i < 8; ++ i )
		if ( pGuid1->Data4[i] != pGuid2->Data4[i] )
			return emFalse;

	return emTrue;
}


emPRes emCall emRes_Create( emResAddress iStart)
{
	emPRes pThis;

#if DEBUG_LOG_SWITCH_HEAP
	pThis = (emPRes)emObjHeap_AllocZero(sizeof(*pThis),"emPRes资源：《纯英文前端》");
	pThis->m_pResPack = (emPResPack2)emObjHeap_AllocZero(sizeof(*pThis->m_pResPack),"emPResPack2资源：《纯英文前端》");
#else
	pThis = (emPRes)emObjHeap_AllocZero(sizeof(*pThis));
	pThis->m_pResPack = (emPResPack2)emObjHeap_AllocZero(sizeof(*pThis->m_pResPack));
#endif

	pThis->m_iCurrent = pThis->m_iStart = iStart;
	pThis->m_pResPack->m_pResParam = g_hTTS->fResFrontMain;
	pThis->m_pResPack->m_nSize = 0;
	pThis->m_pResPack->m_cbReadRes = g_ResPack.m_cbReadRes;

	return pThis;
}

void emCall emRes_Release( emPRes pThis)
{
#if DEBUG_LOG_SWITCH_HEAP
	emObjHeap_Free(pThis->m_pResPack, sizeof(*pThis->m_pResPack),"emPResPack2资源：《纯英文前端》");
	emObjHeap_Free(pThis, sizeof(*pThis), "emPRes资源：《纯英文前端》");
#else
	emObjHeap_Free(pThis->m_pResPack, sizeof(*pThis->m_pResPack));
	emObjHeap_Free(pThis, sizeof(*pThis));
#endif
}



emUInt8 emCall emRes_Get8(emPRes pThis)
{
	emUInt8 nRet;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pThis);

	/* 读取数据 */
	emRes_Read(pThis->m_pResPack, &nRet, pThis->m_iCurrent, emResSize_Int8);

	/* 调整游标 */
	pThis->m_iCurrent += emResSize_Int8;
	emAssert(pThis->m_iCurrent <= pThis->m_iEnd);

	return nRet;
}

emUInt16 emCall emRes_Get16(emPRes pThis)
{
	emUInt8 nRet[emResSize_Int16 / emResSize_Int8];

	LOG_StackAddr(__FUNCTION__);

	emAssert(pThis);
	
	/* 读取数据 */
	emRes_Read(pThis->m_pResPack, nRet, pThis->m_iCurrent, emResSize_Int16);

	/* 调整游标 */
	pThis->m_iCurrent += emResSize_Int16;
	emAssert(pThis->m_iCurrent <= pThis->m_iEnd);

	/* 组织数据 */
	return em16From8s(nRet);
}

emUInt32 emCall emRes_Get32(emPRes pThis)
{
	emUInt8 nRet[emResSize_Int32 / emResSize_Int8];

	LOG_StackAddr(__FUNCTION__);

	emAssert(pThis);

	/* 读取数据 */
	emRes_Read(pThis->m_pResPack, nRet, pThis->m_iCurrent, emResSize_Int32);

	/* 调整游标 */
	pThis->m_iCurrent += emResSize_Int32;
	emAssert(pThis->m_iCurrent <= pThis->m_iEnd);

	/* 组织数据 */
	return em32From8s(nRet);
}

void emCall emRes_Read8s(emPRes pThis, emPUInt8 pBuffer, emResSize nCount)
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pThis && pBuffer && nCount);

	/* 读取数据 */
	emRes_Read(pThis->m_pResPack, pBuffer, pThis->m_iCurrent, nCount * emResSize_Int8);

	/* 调整游标 */
	pThis->m_iCurrent += nCount * emResSize_Int8;
	emAssert(pThis->m_iCurrent <= pThis->m_iEnd);
}

void emCall emRes_Read16s(emPRes pThis, emPUInt16 pBuffer, emResSize nCount)
{
	LOG_StackAddr(__FUNCTION__);

	/* 读取数据 */
	emResPack_Read(pThis->m_pResPack, pBuffer, pThis->m_iCurrent, nCount * emResSize_Int16);

	/* 调整游标 */
	pThis->m_iCurrent += nCount * emResSize_Int16;
}

void emCall emRes_Read32s(emPRes pThis, emPUInt32 pBuffer, emResSize nCount)
{

	/* 读取数据 */
	emResPack_Read(pThis->m_pResPack, pBuffer, pThis->m_iCurrent, nCount * emResSize_Int32);

	/* 调整游标 */
	pThis->m_iCurrent += nCount * emResSize_Int32;

}


void emCall emRes_Map8s( emPRes pThis, emPUInt8 buffer, emResSize nCount)
{

	LOG_StackAddr(__FUNCTION__);

	emAssert(pThis && nCount);

	/* 分配堆空间并读取以模拟映射 */

	emRes_Read(pThis->m_pResPack, buffer, pThis->m_iCurrent, nCount * emResSize_Int8);

	/* 调整游标 */
	pThis->m_iCurrent += nCount * emResSize_Int8;
	emAssert(pThis->m_iCurrent <= pThis->m_iEnd);
}

void emCall emRes_Map16s( emPRes pThis, emPUInt16 buffer,emResSize nCount)
{
	emPUInt8 pRet;

	LOG_StackAddr(__FUNCTION__);

	emAssert(pThis && nCount);

	/* 分配堆空间并读取以模拟映射 */
	emRes_Read(pThis->m_pResPack, buffer, pThis->m_iCurrent, nCount * emResSize_Int16);

	/* 调整游标 */
	pThis->m_iCurrent += nCount * emResSize_Int16;
	emAssert(pThis->m_iCurrent <= pThis->m_iEnd);
}




#if ANKAI_TOTAL_SWITCH

emInt32 GetTickCount()
{
	return get_tick_count();
}
#endif


#if EM_RES_READ_DIRECT_ANKAI

void emCall fFrontSeek(FILE file, emInt32	offset, emInt8 origin)
{
	LOG_StackAddr(__FUNCTION__);

	switch( origin)
	{
		case SEEK_SET:	g_Res.cur_offset = offset;	break;
		case SEEK_CUR:	g_Res.cur_offset += offset;	break;
		case SEEK_END:	break;
		default:		break;
	}
	
}


emInt32 emCall fFrontRead(char* buffer,emInt32 nSize,emInt32 nCount,FILE file)  
{
	emInt32 nRetSize;

	LOG_StackAddr(__FUNCTION__);

	nRetSize = nSize*nCount;
	TTS_Res_direct_read(g_Res.cur_offset, buffer, nRetSize );
	g_Res.cur_offset += nRetSize;
	return nRetSize;
	
}


emInt32 emCall fprintf(FILE file, const char*  format, ... )  
{
	char printf_buf[640] = {0};

	LOG_StackAddr(__FUNCTION__);

	va_list ap;
	va_start(ap, format);
	vsprintf(printf_buf, format, ap);
	va_end(ap);
	uart_write_str(printf_buf);
	return 1;
}


FILE  emCall fopen(char* filename,char* opentype)  
{
	LOG_StackAddr(__FUNCTION__);
	return NULL;

}

void  emCall fclose(FILE file)  
{
	LOG_StackAddr(__FUNCTION__);
	;
}

#endif


