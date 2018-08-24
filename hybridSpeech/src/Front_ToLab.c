#include "emPCH.h"

#include "Front_ToLab.h"


//*****************************************************************************************************
//函数功能： 生成Lab主模块
//*****************************************************************************************************
void emCall	ToLab()
{
	//     /银行|行长/和 了|五 首 诗/	
	//     /银行|行长/和了|五首诗/		
	//     ying2 hang2 hang2 zhang3 he4 le5  wu2  shou3  shi1
	//     5402	 1618  1618  1723  2388 2413 6426 2203  10393

#if DEBUG_LOG_SWITCH_LAB
	FILE *fp_Lab;
#endif

	emUInt8 i,nLen;

	LOG_StackAddr(__FUNCTION__);

	//判断是否是“一字一顿”发音风格。 若是：将相关信息强行修改成“一字一顿”的
	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD)
	{	
		nLen = emStrLenA(g_pText);
		for(i = 0; i< nLen/2; i++)
		{
			g_pTextInfo[i].Len = 2;
			g_pTextInfo[i].BorderType = BORDER_PPH;		//强行修改成：每个汉字都是一个韵律短语
		}	
	}

#if WL_REAR_DEAL_GONG4_GONG4
	//为改善句首gong4音：  把第1个音节置成PPH， 生成的lab的排列将为： sil pau 字... （原来为：sil 字...）
	g_pTextInfo[0].BorderType = BORDER_PPH ;
#endif


	//构造所有音节的Lab中的拼音信息（即第0列至第20列）
	GetPinYinInfoToLab();

	//为调试临时输出Lab二进制文件
#if DEBUG_LOG_SWITCH_LAB
	fp_Lab=fopen("合成.lab", "w");
	fwrite(&g_pLabRTGlobal->m_LabRam, 1, g_pLabRTGlobal->m_MaxLabLine*LAB_CN_LINE_LEN, fp_Lab);
	fclose(fp_Lab);
#endif

	//构造所有音节的Lab中的位置信息（即第21列至第40列），构造所有音节的Lab中的数目信息（即第41列至第62列）
	GetPlaceAndCountInfoToLab();


#if WL_REAR_DEAL_GONG4_GONG4
	//为改善句首gong4音： 将生成的lab的排列将为： sil pau 字...     还原成：sil 字... 
	//经过句首加pau再减pau，改变了句首字的Lab信息
	for( i= 2; i< g_pLabRTGlobal->m_MaxLabLine; i++)
	{
		emMemCpy(g_pLabRTGlobal->m_LabRam[i-1],g_pLabRTGlobal->m_LabRam[i],LAB_CN_LINE_LEN);
	}
	g_pLabRTGlobal->m_LabRam[1][RLAB_L_SM] = INITIAL_CN_sil;	//恢复sil的声母
	g_pLabRTGlobal->m_MaxLabLine--;
	emMemSet(&g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine][RLAB_C_SM], 0 ,LAB_CN_LINE_LEN);
#endif



	//为调试临时输出Lab二进制文件
#if DEBUG_LOG_SWITCH_LAB
	fp_Lab=fopen("合成.lab", "w+");		//sqb
	fwrite(&g_pLabRTGlobal->m_LabRam, 1, g_pLabRTGlobal->m_MaxLabLine*LAB_CN_LINE_LEN, fp_Lab);
	fclose(fp_Lab);
#endif


	HandleLabQst();		//解决长句短句等问题


	////解决：不识别或不显示的汉字发音问题（但汉字区域，但拼音编码=0）
	////例如： 狮子座中有一颗星位于的弯曲部
	//for( i= 1; i< (g_pLabRTGlobal->m_MaxLabLine-1); i++)
	//{
	//	if(    g_pLabRTGlobal->m_LabRam[i][0] == 0 
	//		&& g_pLabRTGlobal->m_LabRam[i][1] == 0 
	//		&& g_pLabRTGlobal->m_LabRam[i][2] == 0 )
	//	{
	//		//将此拼音赋值成：pau的拼音
	//		g_pLabRTGlobal->m_LabRam[i][0] = INITIAL_CN_pau ;
	//		g_pLabRTGlobal->m_LabRam[i][1] = FINAL_CN_nil ;
	//		g_pLabRTGlobal->m_LabRam[i][2] = TONE_CN_sil_pau ;

	//		g_pLabRTGlobal->m_LabRam[i-1][6] = INITIAL_CN_pau ;
	//		g_pLabRTGlobal->m_LabRam[i-1][7] = FINAL_CN_nil ;
	//		g_pLabRTGlobal->m_LabRam[i-1][8] = TONE_CN_sil_pau ;
	//		g_pLabRTGlobal->m_LabRam[i-1][RLAB_SYL_BWD_POS_IN_PW] = 0 ;


	//		g_pLabRTGlobal->m_LabRam[i+1][3] = INITIAL_CN_pau ;
	//		g_pLabRTGlobal->m_LabRam[i+1][4] = FINAL_CN_nil ;
	//		g_pLabRTGlobal->m_LabRam[i+1][5] = TONE_CN_sil_pau ;
	//		g_pLabRTGlobal->m_LabRam[i+1][RLAB_SYL_FWD_POS_IN_PW] = 0 ;				

	//	}
	//}


	//解决：不识别或不显示的汉字发音问题（但汉字区域，但拼音编码=0）, 这部分汉字丢弃不发音
	//例如： 狮子座中有一颗星位于的弯曲部
	for( i= 1; i< (g_pLabRTGlobal->m_MaxLabLine-1); i++)
	{
		if(    g_pLabRTGlobal->m_LabRam[i][0] == 0 
			&& g_pLabRTGlobal->m_LabRam[i][1] == 0 
			&& g_pLabRTGlobal->m_LabRam[i][2] == 0 )
		{
			//将后面的行不上来
			emMemCpy( g_pLabRTGlobal->m_LabRam[i] , g_pLabRTGlobal->m_LabRam[i+1], (g_pLabRTGlobal->m_MaxLabLine-i-1)*LAB_CN_LINE_LEN  );
			g_pLabRTGlobal->m_MaxLabLine--;

			g_pLabRTGlobal->m_LabRam[i-1][6] = INITIAL_CN_pau ;
			g_pLabRTGlobal->m_LabRam[i-1][7] = FINAL_CN_nil ;
			g_pLabRTGlobal->m_LabRam[i-1][8] = TONE_CN_sil_pau ;
			g_pLabRTGlobal->m_LabRam[i-1][RLAB_SYL_BWD_POS_IN_PW] = 0 ;


			g_pLabRTGlobal->m_LabRam[i][3] = INITIAL_CN_pau ;
			g_pLabRTGlobal->m_LabRam[i][4] = FINAL_CN_nil ;
			g_pLabRTGlobal->m_LabRam[i][5] = TONE_CN_sil_pau ;
			g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PW] = 0 ;		

			i--;
		}
	}
}


void HandleLabQst()
{
	emInt16 i;

	LOG_StackAddr(__FUNCTION__);
	
#if WL_REAR_DEAL_LAB_LONG_SEN			//王林音库后处理：解决长句问题

	#define FORCE_MAX_SYL_IN_SEN	25				//解决长句问题：强迫将g_pLabRTGlobal->m_LabRam中的值重新赋值
	#define FORCE_MAX_SYL_IN_PPH	12				//解决长句问题：
	#define FORCE_MAX_PW_IN_SEN		12				//解决长句问题：
	#define FORCE_MAX_PW_IN_PPH		6				//解决长句问题：
	#define FORCE_MAX_PPH_IN_SEN	5				//解决长句问题（包括一字一顿的长句问题）

	//解决长句问题（语料中最长的句子是35字，但emTTS文本分析中支持50字，长句的时长会乱掉），将syl_in_sen等强行改小
	//同时兼解决“一字一顿”中的长句问题
	for(i=0;i<g_pLabRTGlobal->m_MaxLabLine;i++)
	{

#if WL_REAR_DEAL_LETTER	 && EM_8K_MODEL_PLAN	  //8K模型配置：字母的处理  2012-10-12 hyl
		//调整字母的反序正序位置，解决句子过长字母读不好的问题
		//例如：欢迎观看语音合成系统的演示欢迎观看语音合成系统的演示S12！
		//例如：凡至XXXX化妆品专柜购买产品满720元。
		//例如：0系列40英寸液晶电视LA40B530P7R在近一段时间内动作频频；
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_C_SM] == INITIAL_CN_letter )	 //若是字母
		{
			if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP] > 10)		//	IP中当前SYL的正序位置
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP]  = 10;			

			if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP] > 10)		//	IP中当前SYL的反序位置	
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP]  = 10;	
		}
#endif


		//句中：音节的位置与个数
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP] = FORCE_MAX_SYL_IN_SEN - 1;		//IP中当前SYL的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_SEN] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_SEN] = FORCE_MAX_SYL_IN_SEN - 1;		//SEN中当前SYL的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP] = FORCE_MAX_SYL_IN_SEN - 1;		//IP中当前SYL的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_SEN] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_SEN] = FORCE_MAX_SYL_IN_SEN - 1;		//SEN中当前SYL的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_IP] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_IP] = FORCE_MAX_SYL_IN_SEN ;		//当前IP中SYL的数目
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_SEN] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_SEN] = FORCE_MAX_SYL_IN_SEN ;		//当前SEN中SYL的数目

		//韵律短语中：音节的位置与个数
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] > FORCE_MAX_SYL_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] = FORCE_MAX_SYL_IN_PPH - 1;		//PPH中当前SYL的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] > FORCE_MAX_SYL_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] = FORCE_MAX_SYL_IN_PPH - 1;		//PPH中当前SYL的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] > FORCE_MAX_SYL_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] = FORCE_MAX_SYL_IN_PPH ;		//当前PPH中SYL的数目


		//句中：PW的位置与个数
		//以下不要也能解决长句问题，但未保险起见，还是加上
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_IP] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_IP] = FORCE_MAX_PW_IN_SEN - 1;		//IP中当前PW的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_SEN] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_SEN] = FORCE_MAX_PW_IN_SEN - 1;		//SEN中当前PW的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_IP] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_IP] = FORCE_MAX_PW_IN_SEN - 1;		//IP中当前PW的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_SEN] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_SEN] = FORCE_MAX_PW_IN_SEN - 1;		//SEN中当前PW的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_IP] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_IP] = FORCE_MAX_PW_IN_SEN ;			//当前IP中PW的数目
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_SEN] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_SEN] = FORCE_MAX_PW_IN_SEN ;			//当前SEN中PW的数目

		//韵律短语中：PW的位置与个数
		//以下不要也能解决长句问题，但未保险起见，还是加上		
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] > FORCE_MAX_PW_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] = FORCE_MAX_PW_IN_PPH - 1;		//PPH中当前PW的正序位置		
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] > FORCE_MAX_PW_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] = FORCE_MAX_PW_IN_PPH - 1;		//PPH中当前PW的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] > FORCE_MAX_PW_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] = FORCE_MAX_PW_IN_PPH ;			//当前PPH中PW的数目

		//PPH的位置与个数： 
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_IP] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_IP] = FORCE_MAX_PPH_IN_SEN - 1;		//IP中当前PPH的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_SEN] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_SEN] = FORCE_MAX_PPH_IN_SEN - 1;		//SEN中当前PPH的正序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_IP] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_IP] = FORCE_MAX_PPH_IN_SEN - 1;		//IP中当前PPH的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_SEN] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_SEN] = FORCE_MAX_PPH_IN_SEN - 1;		//SEN中当前PPH的反序位置
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_IP] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_IP] = FORCE_MAX_PPH_IN_SEN ;		//当前IP中PPH的数目
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_SEN] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_SEN] = FORCE_MAX_PPH_IN_SEN ;		//当前SEN中PPH的数目

	}
#endif


#if WL_REAR_DEAL_LAB_SHORT_SEN			//王林音库后处理：3字短句在lab中模拟成4字句  //解决短句问题

	//例句：喜欢否。你喜欢否。你们喜欢否。见到鹊否？见鹊否？这真好！这真很好！
	if( (g_pLabRTGlobal->m_MaxLabLine-2) == 3)		//3字句
	{
		g_pLabRTGlobal->m_LabRam[3][RLAB_PW_FWD_POS_IN_PPH] = 1;	//PPH中当前PW的正序位置
		g_pLabRTGlobal->m_LabRam[3][RLAB_PW_FWD_POS_IN_IP] = 1;	//IP中当前PW的正序位置
		g_pLabRTGlobal->m_LabRam[3][RLAB_PW_FWD_POS_IN_SEN] = 1;	//SEN中当前PW的正序位置
	}
#endif
}

//*****************************************************************************************************
//函数功能： 变音（一，不，连续3声，叠词）
//*****************************************************************************************************
void emCall ChangeTone()
{

#define MAX_TextAfterYi			30
#define MAX_TextBeforeYi		20

	emInt16  count = 0,nPos,i,j;
	emByte   nCurTone,nNextTone;
	emUInt16 nNextZi,nBeforeZi,nNextNextZi,nBeforeBeforeZi,nNeedJustZi;
	emUInt16  pTextShuZi[12];	

	emUInt16  pTextAfterYi[MAX_TextAfterYi];
	emUInt16  pTextBeforeYi[MAX_TextBeforeYi];
	emUInt8   nBeforeTextItem = 0;
	emUInt8   nBeforeBeforeTextItem = 0;

	//“一”为首的3字词，无条件的不再变音
	emUInt16  pTextWord3OfYi[20][2]={	{0xbcbe, 0xb6c8},		//一季度
										{0xb0d1, 0xcad6},		//一把手
										{0xc4ea, 0xbcb6},		//一年级
										{0xc9e3, 0xcacf},		//一摄氏				气温1℃～-8℃
										{0xc2b7, 0xb8bd},		//一路附				车辆位于新茂工业大道西一路附近
										{0xcfef, 0xb8bd},		//一巷附				车辆位于和圹一巷附近
										{0xc1aa, 0xb6d3},		//一联队				仅有敌一联队与我第六军暂五十五师接触
										{0xb6d4, 0xd2bb},		//一对一
										{0xb5e3, 0xd6d3},		//一点钟
										{0xb5c8, 0xbdb1},		//一等奖				一等奖。获得了一等奖。他一等到开门就冲了进去。
										{0xb2e3, 0x0000},		//一层+句尾（取巧方式）	他住在一层。商场B1层。他们日常生活中的每一层面。								
										{0xc3c5, 0x0000},		//一门+句尾（取巧方式）	他住在一楼一门。在一门洞里找到了。一门迫击炮。也是一门心灵的学问。
										{0xc6da, 0x0000},		//一期+句尾（取巧方式）	包括6号线一期
										{0xcfdf, 0xb9a4},		//一线工				我们是一线工人。他总在一线工作。
										{0xcfdf, 0xb8e8},		//一线歌				迅速蹿红为一线歌手。迅速蹿红为一线歌星。										
										{0xc2b7, 0xb7bd},		//一路方				前方2公里出高速往长江一路方向
										{0x0000, 0x0000}};		//--------结束符

	LOG_StackAddr(__FUNCTION__);	



	//“一”前跟以下汉字，无条件的不再变音
	i=0;
	pTextBeforeYi[i++] = 0xb5da;	//第
	pTextBeforeYi[i++] = 0xb3f5;	//初		正月初一古称元日。
	pTextBeforeYi[i++] = 0xcaae;	//十
	pTextBeforeYi[i++] = 0xcdf2;	//万
    pTextBeforeYi[i++] = 0xb5a5;	//单
    pTextBeforeYi[i++] = 0xcdb3;	//统
    pTextBeforeYi[i++] = 0xcea8;	//唯
	pTextBeforeYi[i++] = 0xd6dc;	//周
	pTextBeforeYi[i++] = 0xbdec;	//届		改变八届一次会议关于国内主要矛盾的正确分析
	pTextBeforeYi[i++] = 0xbaec;	//红		决定把红一方面军与军委纵队整编为中国工农红军陕甘支队
	pTextBeforeYi[i++] = 0xc6da;	//期
	pTextBeforeYi[i++] = 0xc6e4;	//其
	pTextBeforeYi[i++] = 0xd6ae;	//之		占全世界1／2人口的妇女  随之一条庞大的地下假药生产链被摧毁
	pTextBeforeYi[i++] = 0xb5e3;	//点		6352.1元
	pTextBeforeYi[i++] = 0xcdac;	//同		同一账户或同一控制人控制的多个账户
	pTextBeforeYi[i++] = 0x0000;	//----结束符
	if( i > MAX_TextBeforeYi)
	{
		return;							//出错
	}
       
       

	//“一”前跟或后跟以下数字，无条件的不再变音
	i = 0;
	pTextShuZi[i++] = 0xd2bb;	//一
	pTextShuZi[i++] = 0xb6fe;	//二
    pTextShuZi[i++] = 0xc8fd;	//三
    pTextShuZi[i++] = 0xcbc4;	//四
    pTextShuZi[i++] = 0xcee5;	//五
    pTextShuZi[i++] = 0xc1f9;	//六
    pTextShuZi[i++] = 0xc6df;	//七
    pTextShuZi[i++] = 0xb0cb;	//八
    pTextShuZi[i++] = 0xbec5;	//九
    pTextShuZi[i++] = 0xc1e3;	//零
	pTextShuZi[i++] = 0x0000;	//----结束符
	


	//“一”后跟以下汉字，无条件的不再变音
	i = 0;
    pTextAfterYi[i++] = 0xd4c2;	//月		
    pTextAfterYi[i++] = 0xbac5;	//号		
    pTextAfterYi[i++] = 0xd6c1;	//至		
	pTextAfterYi[i++] = 0xb6d3;	//队		一队，八一队, 	
	pTextAfterYi[i++] = 0xb7a3;	//罚		假一罚十，少一罚十
	pTextAfterYi[i++] = 0xcaae;	//十	
    pTextAfterYi[i++] = 0xc2a5;	//楼    
    pTextAfterYi[i++] = 0xbcb6;	//级        
    pTextAfterYi[i++] = 0xb2bb;	//不		说一不二。他一不高兴就哭。
	pTextAfterYi[i++] = 0xcac7;	//是
	pTextAfterYi[i++] = 0xc6fb;	//汽
	pTextAfterYi[i++] = 0xbdd6;	//街		
	pTextAfterYi[i++] = 0xbafa;	//胡		前方1.5公里前方上高架道往菜市北街西一胡同方向。
	pTextAfterYi[i++] = 0xc9f3;	//审		宁波市江东区法院作出一审判决。		
	pTextAfterYi[i++] = 0xc1ac;	//连		一连连长来了。他一连打了几个好球。一连串。
	pTextAfterYi[i++] = 0xc1f7;	//流		一流的。他是一流的音乐家。只要一流行他就跟风。
	pTextAfterYi[i++] = 0x0000;	//----结束符
	//不要 pTextAfterYi[i++] = 0xbdda;	//节		五一节，六一节。一节。一节藕。	
	//不要 pTextAfterYi[i++] = 0xc3c5;	//门		他住在一楼一门。在一门洞里找到了。一门迫击炮。也是一门心灵的学问。
	//不要 pTextAfterYi[i++] = 0xb5c8;	//等		一等奖。获得了一等奖。他一等到开门就冲了进去。
	//不要 pTextAfterYi[i++] = 0xcfdf;	//线		从一个无名小卒迅速蹿红为一线歌手。我们是一线工人。淮河一线对峙的局面。不拿群众一针一线。只要有一线希望。
	//不要 pTextAfterYi[i++] = 0xb2e3;	//层		他住在一层。商场B1层。一层秋雨一层凉。这使得乾陵被渲染上了一层层神秘的色彩。他们日常生活中的每一层面。
	//不要 pTextAfterYi[i++] = 0xc2b7;	//路		前方1.5公里出高速往长江一路方向。		他们一路狂奔。我们一路上说个不停。祝你一路顺风。
	if( i > MAX_TextAfterYi)
	{
		return;						//出错
	}


	g_nCurTextByteIndex = 0;
	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		count = g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
		nPos = g_pTextInfo[g_nCurTextByteIndex/2].Pos;

		for( i = 0; i < (count); i++ )   
		{
			if( ( (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]) & 0x8000 ) != 0x8000)	//不是强制拼音才能变音 hyl 20111227
			{

				//变音规则：【连续三声的变音】
				//  总体原则：PW中连续3声自动变音，除了最后1个字，其余字全部变成2声： （有特例）
				//	例如：  韵律词	        语法词        变音结果
				//
				//			小老虎        = 小+老虎	      = xiao3 lao2 hu3
				//			小老人        = 小+老人	      = xiao2 lao3 ren2
				//			也+可以       = 也+可以	      = ye3 ke2 yi3
				//			有+养狗       = 有+养+狗	  = you3 yang2 gou3			有养狗经验的人说
				//			展览馆        = 展览馆	      = zhan2 lan2 guan3	
				//			所领导的      = 所+领导+的    = suo3 ling2 dao3 de5	
				//			两佰米        = 两+佰+米      = liang2 bai2 mi3
				//			等◆九人      = 等◆+九人     = deng3 jiu3 ren2
				//			传统+美德     = 传统+美德     = chuan2 tong2 mei3 de2	并不等于否定见义勇为等传统美德
				//			审美+取向     = 审美+取向     = shen2 mei2 qu3 xiang4   生活意识说到底就是作家的审美取向要贴近创造着历史的大众的人生

				if( count== 1 )  //1字韵律词
				{
					//若：本1字韵律词：不是PPH尾，是3声，且后字是3声，但后后字不是3声
					//则：本1字韵律词变成2声
					if(	   g_pTextInfo[g_nCurTextByteIndex/2+1].BorderType != BORDER_PPH
						&& (( g_pTextPinYinCode[g_nCurTextByteIndex/2] & (emByte) 0x7) == 3) 
						&& (( g_pTextPinYinCode[g_nCurTextByteIndex/2+1] & (emByte) 0x7) == 3) 
						&& (  (( g_pTextPinYinCode[g_nCurTextByteIndex/2+2] & (emByte) 0x7) != 3)
						))
						    //  || g_pTextInfo[g_nCurTextByteIndex/2+2].Len == 0  ))	//不是语法词的开始
					{
						g_pTextPinYinCode[g_nCurTextByteIndex/2] -= 1;	//变成2声
					}
				}
				else		//2字及以上韵律词
				{
					if( i!=(count-1))		//不是韵律词尾
					{
						if( (( g_pTextPinYinCode[g_nCurTextByteIndex/2+i] & (emByte) 0x7) == 3) && (( g_pTextPinYinCode[g_nCurTextByteIndex/2+i+1] & (emByte) 0x7) == 3)  )
						{
							//若：当前韵律词为3字词及以上（且为“1字词+其他词”合并来的韵律词），且前3个字都为3声
							//则：第1字不变音。
							//其余：按总体原则变音
							if( count >= 3 && i == 0
								&& g_pTextInfo[g_nCurTextByteIndex/2 + 1].Len > 2
								&& (( g_pTextPinYinCode[g_nCurTextByteIndex/2+i+2] & (emByte) 0x7) == 3) )	
							{
								g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 0;	//不变音
							}
							else							
								g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 1;	//变成2声
						}
					}					
					else					//是韵律词尾
					{
						emUInt8 nNext1,nNextCount;
						nNext1 = g_nCurTextByteIndex+g_pTextInfo[g_nCurTextByteIndex/2].Len;
						nNextCount = g_pTextInfo[nNext1/2].Len/2;

						if(    g_pTextInfo[g_nCurTextByteIndex/2+i+1].BorderType != BORDER_PPH		//本韵尾不是PPH尾
							&& (g_pTextPinYinCode[g_nCurTextByteIndex/2+i-1] & (emByte) 0x7) != 3	//韵尾的前1字不是3声
							&& (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]   & (emByte) 0x7) == 3	//韵尾是3声						
							&& (g_pTextPinYinCode[nNext1/2]   & (emByte) 0x7) == 3					//下1韵律词的韵头是3声
							&& (g_pTextPinYinCode[nNext1/2+1] & (emByte) 0x7) != 3)					//下1韵律词的第2字不是3声
						{
							g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 1;	//变成2声
						}
					}
				}



				//变音规则：“不(0xb2bb)”的自动变音：         （只要“不”后面还有汉字且发4声，“不”变成2声）
				if( g_pText[g_nCurTextByteIndex+2*i] == 0xb2 && g_pText[g_nCurTextByteIndex+2*i+1] == 0xbb && ( ( g_pTextPinYinCode[g_nCurTextByteIndex/2+i] & (emByte) 0x7 ) == 4 ) && ( ( g_pTextPinYinCode[g_nCurTextByteIndex/2+i+1] & (emByte) 0x7 ) == 4 ) )
				{
					g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 2;
				}
			}
		}		

		for( i = 0; i < count; i++ )   
		{
			if( ( (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]) & 0x8000 ) != 0x8000)	//不是强制拼音才能变音 hyl 20111227
			{
				//变音规则：“一(0xd2bb)”的自动变音：       （最后1个字均不变音）   
				if(  g_pText[g_nCurTextByteIndex+2*i] == 0xd2 && g_pText[g_nCurTextByteIndex+2*i+1] == 0xbb )
				{
					//若本节点的“一”的全局变音策略是打开的，才有可能变音 (目前只在《数字按号码读》中， 是关闭的)
					if(  TRUE )	
					{
						//计算出“一”的前一个字
						nBeforeZi = 0;
						nBeforeBeforeZi = 0;
						if( (g_nCurTextByteIndex/2+i) > 0 )
						{	//本节点中“一”的前一个字						
							nBeforeZi =  (emUInt16)((g_pText[g_nCurTextByteIndex+2*i-2])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i-1]) ;
							nBeforeBeforeZi = (emUInt16)((g_pText[g_nCurTextByteIndex+2*i-4])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i-3]) ;
						}

						//计算出“一”的后一个字
						nNextZi = 0;
						nNextNextZi = 0;
						if( (g_nCurTextByteIndex/2+i) < (g_nLastTextByteIndex/2-1) )
						{
							nNextZi =  (emUInt16)((g_pText[g_nCurTextByteIndex+2*i+2])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i+3]) ;					
							nNextNextZi = (emUInt16)((g_pText[g_nCurTextByteIndex+2*i+4])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i+5]) ;					
						}

						//计算出“一”的声调
						nCurTone = 0;
						nCurTone =  (g_pTextPinYinCode[g_nCurTextByteIndex/2+i] & (emByte) 0x7 ) ;

						//计算出“一”的后1个字的声调
						nNextTone =  0 ;
						if( (g_nCurTextByteIndex/2+i) < (g_nLastTextByteIndex/2-1) )
						{
							nNextTone =  (g_pTextPinYinCode[g_nCurTextByteIndex/2+i+1] & (emByte) 0x7 ) ;
						}

						//为强制拼音（例如来源于多音词典，来源[=]），不能变音
						if( ( (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]) & 0x8000 )== 0x8000)		
							continue;

						//前字是字母 a,b,c......z，不能变音
						if( nBeforeZi >= 0xa3e1 && nBeforeZi <= 0xa3fa)		//hyl 2012-03-31
							continue;



						//“一”为首的某些3字词，不能变音
						j = 0;			
						while( pTextWord3OfYi[j][0] != 0x0000)		
						{
							if( nNextZi == pTextWord3OfYi[j][0] && nNextNextZi == pTextWord3OfYi[j][1] )
								break;
							else
								j++;
						}
						if(  pTextWord3OfYi[j][0] != 0x0000)
							continue;


						//“一”：本字是数词，后字是“到”“或”“点”，后后字是数词，则不变音
						//例如：直接回复1或2即可。冬天出门前先热车一到三分钟。
						if(    g_pTextInfo[g_nCurTextByteIndex/2+i].Pos == POS_CODE_m
							&& (nNextZi == 0xb5bd || nNextZi == 0xbbf2 || nNextZi == 0xb5e3 )
							&& g_pTextInfo[g_nCurTextByteIndex/2+i+2].Pos == POS_CODE_m )
						{
							continue;
						}


						//“一”在韵律短语尾，不能变音
						g_nNextTextByteIndex = g_nCurTextByteIndex + g_pTextInfo[g_nCurTextByteIndex/2].Len;
						if(    i == (count-1) 
							&& (    g_pTextInfo[g_nNextTextByteIndex/2].BorderType == BORDER_PPH
							     || g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)	)
						{
							continue;
						}

						//后一个字是轻声，不能变音
						if( nNextTone == 5 )
							continue;


						//“买一送一”， 不能变音  (只有在“买一送一”是一个词组的前提下，这条规则才管用)
						if( nBeforeZi == 0xc2f2  && nNextZi == 0xcbcd )
						{
							i += 2;
							continue;
						}

						//若后字或前字是 "比"或 "乘"  ，一定条件下（“比”，“乘”前后接的都是数字），不能变音			added by hyl 2012-03-31
						//例如： 世界冠军杨影以2∶1险胜福。世界冠军杨影以1∶2险胜福。1×1＝1。
						nNeedJustZi = 0;
						if( nNextZi == 0xb1c8 || nNextZi == 0xb3cb )
							nNeedJustZi = nNextNextZi;
						if( nBeforeZi == 0xb1c8 || nBeforeZi == 0xb3cb)
							nNeedJustZi = nBeforeBeforeZi;
						if( nNeedJustZi != 0)
						{
							//pTextShuZi[]： 是数字
							j = 0;			
							while( pTextShuZi[j] != 0x0000)		
							{
								if( nNeedJustZi == pTextShuZi[j] )
									break;
								else
									j++;
							}
							if(  pTextShuZi[j] != 0x0000)
								continue;
						}
							     

						//“一”前跟某些汉字（见pTextBeforeYi[]和pTextShuZi[]的初始化值），全部无条件的不变音										
						if( nBeforeZi != 0)
						{
							//pTextBeforeYi[]：无条件的不变音
							j = 0;			
							while( pTextBeforeYi[j] != 0x0000)		
							{
								if( nBeforeZi == pTextBeforeYi[j] )
									break;
								else
									j++;
							}
							if(  pTextBeforeYi[j] != 0x0000)
								continue;

							//pTextShuZi[]：无条件的不变音
							j = 0;			
							while( pTextShuZi[j] != 0x0000)		
							{
								if( nBeforeZi == pTextShuZi[j] && nBeforeZi != 0xd2bb)		//但“一”排除在外
									break;
								else
									j++;
							}
							if(  pTextShuZi[j] != 0x0000)
								continue;
						}


						//“一”的前字为“一”，若前前字为“唯 万 统 周 期”，则变音，若前前字不是这些，则不变音
						//应变音：  例如： 唯一一个。万一一个。统一一个。周一一个。星期一一个。
						//不应变音：例如： 一一作答。我们一一澄清。23.11。  23.112。
						if( nBeforeZi ==  0xd2bb)	//前字为“一”
						{
							if(    nBeforeBeforeZi != 0xcea8	//唯
								&& nBeforeBeforeZi != 0xcdf2	//万
								&& nBeforeBeforeZi != 0xcdb3	//统
								&& nBeforeBeforeZi != 0xd6dc	//周
								&& nBeforeBeforeZi != 0xc6da)	//期
							{
								continue;
							}

						}


						//“一”后跟某些汉字（见pTextAfterYi[]的初始化值，见pTextShuZi[]的初始化值），全部无条件的不变音
						//“一”后跟除（年，日，点）需做补充判断：
						//				年： 若“一”前还跟数字（0至9），则不变音，否则变音
						//				日： 若“一”前还跟“月”，		 则不变音，否则变音
						//				点： 若“点”后还跟数字（0至9）或“钟”或“整”，则不变音，否则变音											
						if( nNextZi != 0)
						{
							

							//pTextAfterYi[]：无条件的不变音
							j = 0;	
							while( pTextAfterYi[j] != 0x0000)		
							{
								if( nNextZi == pTextAfterYi[j] )
									break;
								else
									j++;
							}
							if(  pTextAfterYi[j] != 0x0000)
								continue;

							//pTextShuZi[]：无条件的不变音
							j = 0;	
							while( pTextShuZi[j] != 0x0000)		
							{
								if( nNextZi == pTextShuZi[j] )
									break;
								else
									j++;
							}
							if(  pTextShuZi[j] != 0x0000)
								continue;

							//下一个汉字是：“年”
							if( nNextZi == 0xc4ea )
							{
								//“一”之前跟的是数字，则不变音
								if( nBeforeZi != 0)
								{
									j = 0;			
									while( pTextShuZi[j] != 0x0000)		
									{
										if( nBeforeZi == pTextShuZi[j] )
											break;
										else
											j++;
									}
									if(  pTextShuZi[j] != 0x0000)
										continue;
								}
							}

							//下一个汉字是：“日”
							if( nNextZi == 0xc8d5 )
							{
								if( nBeforeZi == 0xd4c2 )		//前一字是“月”，则不变音
									continue;
							}

							//下一个汉字是：“点”
							if( nNextZi == 0xb5e3 )
							{
								//计算出“一”的后后字
								nNextNextZi = 0;
								if( (g_nCurTextByteIndex/2+i+1) < (g_nLastTextByteIndex/2-1) )
								{
									nNextZi =  (emUInt16)((g_pText[g_nCurTextByteIndex+2*i+4])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i+5]) ;					
								}							

								if( nNextNextZi != 0)
								{
									//“点”之后跟的是“钟”，则不变音
									if( nNextNextZi == 0xd6d3 )		
										continue;

									//“点”之后跟的是“整”，则不变音
									if( nNextNextZi == 0xd5fb )		
										continue;

									//“点”之后跟的是数字，则不变音
									j = 0;			
									while( pTextShuZi[j] != 0x0000)		
									{
										if( nNextNextZi == pTextShuZi[j] )
											break;
										else
											j++;
									}
									if(  pTextShuZi[j] != 0x0000)
										continue;
								}
							}
						}


						//“一”只好变音了。根据后1个汉字的声调来变音：  后接4声，变成2声；后接1,2,3声，变成4声						
						if( nCurTone == 1 && nNextTone == 4 )
						{  	
							g_pTextPinYinCode[g_nCurTextByteIndex/2+i] += 1;		//“一”后面的汉字是4声，“一”变成2声
						}						
						if( nCurTone == 1 && ( nNextTone==1 || nNextTone==2 || nNextTone==3 ) )
						{  	
							g_pTextPinYinCode[g_nCurTextByteIndex/2+i] += 3;		//“一”后面的汉字是1或2 或3 ，“一”变成4声
						}
					}
				}	
			}
		}						

		nBeforeBeforeTextItem = nBeforeTextItem;
		nBeforeTextItem = g_nCurTextByteIndex;

		g_nCurTextByteIndex += g_pTextInfo[g_nCurTextByteIndex/2].Len ;
	}
}

//*****************************************************************************************************
//函数功能： 获取后一韵律词中汉字的总数
//*****************************************************************************************************
emUInt8 emCall GetMaxHanZiInNextPW(emUInt8 nTextItem,emBool isSil)
{

	emUInt8 count=0;

	LOG_StackAddr(__FUNCTION__);

	if( isSil == emTrue)	
	{
		count = g_pTextInfo[nTextItem/2].Len/2;
	}
	else
	{
		g_nNextTextByteIndex = nTextItem + g_pTextInfo[nTextItem/2].Len;
		if( g_pTextInfo[g_nNextTextByteIndex/2].Len != 0)
		{
			count = g_pTextInfo[g_nNextTextByteIndex/2].Len/2;
		}
	}
	return count;
}

//*****************************************************************************************************
//函数功能： 获取后一韵律短语中汉字的总数
//*****************************************************************************************************
emUInt8 emCall GetMaxHanZiInNextPPH(emUInt8 nTextItem,emBool isSil)
{
	emUInt8 count=0;
	emBool isNext = emFalse;
	emUInt8 nCur = nTextItem;

	LOG_StackAddr(__FUNCTION__);

	if( isSil == emTrue)		
	{
		while(g_pTextInfo[nCur/2].Len != 0)
		{
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//若本节点 不是 边界节点（PPH）
			{
				count += g_pTextInfo[nCur/2].Len/2;
				nCur  += g_pTextInfo[nCur/2].Len;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		if(g_pTextInfo[nCur/2].Len != 0)
		{
			nCur  += g_pTextInfo[nCur/2].Len;
		}
		while(g_pTextInfo[nCur/2].Len != 0)
		{
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//若本节点 不是 边界节点（PPH）
			{
				if( isNext == emTrue)
				{
					count += g_pTextInfo[nCur/2].Len/2;					
				}
				nCur  += g_pTextInfo[nCur/2].Len;
			}
			else
			{
				isNext = emTrue;
				count += g_pTextInfo[nCur/2].Len/2;			
				nCur  += g_pTextInfo[nCur/2].Len;
			}
		}

	}
	return count;
}

//*****************************************************************************************************
//函数功能： 获取后一韵律短语中韵律词的总数
//*****************************************************************************************************
emUInt8 emCall GetMaxPWInNextPPH(emUInt8 nTextItem,emBool isSil)
{
	emUInt8 count=0;
	emBool isNext = emFalse;
	emUInt8 nCur = nTextItem;

	LOG_StackAddr(__FUNCTION__);

	if( isSil == emTrue)		
	{
		while(g_pTextInfo[nCur/2].Len != 0)
		{
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//若本节点 不是 边界节点（PPH）
			{
				count++;
				nCur  += g_pTextInfo[nCur/2].Len;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		if(g_pTextInfo[nCur/2].Len != 0)
		{
			nCur  += g_pTextInfo[nCur/2].Len;
		}
		while(g_pTextInfo[nCur/2].Len != 0)
		{
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//若本节点 不是 边界节点（PPH）
			{
				if( isNext == emTrue)
				{
					count++;		
				}
				nCur  += g_pTextInfo[nCur/2].Len;
			}
			else
			{
				isNext = emTrue;
				count++;
				nCur  += g_pTextInfo[nCur/2].Len;
			}
		}

	}
	return count;
}


//*****************************************************************************************************
//函数功能： 构造所有音节的Lab中的位置信息（即第21列至第40列）    所有位置从0开始起算
//           构造所有音节的Lab中的数目信息（即第41列至第62列）
//
// sil和pau音节不会使位置计数增加，位置信息只跟汉字节点有关
// sil和pau音节不会使数目计数增加，数目信息只跟汉字节点有关
//*****************************************************************************************************
void emCall GetPlaceAndCountInfoToLab()
{
	emUInt8 nMaxOfHanZiInSEN=0;					//本句中的汉字总数
	emUInt8 nMaxOfPWInSEN=0;					//本句中的韵律词总数
	emUInt8 nMaxOfPPHInSEN=0;					//本句中的韵律短语总数

	emUInt8 nCountOfHanZiInCurPPH = 0;			//当前韵律短语中的汉字计数
	emUInt8 nCountOfPWInCurPPH = 0;				//当前韵律短语中的韵律词计数

	emUInt8 nCountOfHanZiInSEN=0;				//本句中的汉字计数
	emUInt8 nCountOfPWInSEN=0;					//本句中的韵律词计数
	emUInt8 nCountOfPPHInSEN=0;					//本句中的韵律短语计数
	emUInt8 nCountOfSylInSen = 0;				//本句中的音节计数

	emUInt8 nStartCountOfSylInPPH = 0;			//本韵律短语的起始音节数
	emUInt8 nCurCountOfSylInPPH = 0;			//本韵律短语的当前音节数

	emUInt8 nMaxOfHanZiInPrevPW = 0;			//前一韵律词中汉字的总数
	emUInt8 nMaxOfHanZiInPrevPPH = 0;			//前一韵律短语中汉字的总数
	emUInt8 nMaxOfPWInPrevPPH = 0;				//前一韵律短语中韵律词的总数

	emUInt8 nCountOfHanZiInPrevPPH = 0;			//前一韵律短语中汉字的计数
	emUInt8 nCountOfPWInPrevPPH = 0;			//前一韵律短语中韵律词的计数

	emUInt8 nMaxOfHanZiInNextPW = 0;			//后一韵律词中汉字的总数
	emUInt8 nMaxOfHanZiInNextPPH = 0;			//后一韵律短语中汉字的总数
	emUInt8 nMaxOfPWInNextPPH = 0;				//后一韵律短语中韵律词的总数

	emUInt8 i,nHanZiCount;

	LOG_StackAddr(__FUNCTION__);

	//并获得本句中的三个信息（通过指针返回参数） 本句中的汉字总数, 本句中的韵律词总数 , 本句中的韵律短语总数
	g_pLabRTGlobal->m_MaxLabLine = GetSenInfo(&nMaxOfHanZiInSEN, &nMaxOfPWInSEN, &nMaxOfPPHInSEN);		//返回HMM合成前的最大lab行数，即音节数

	nCountOfSylInSen = 0;

	//最先赋值（IP等同于SEN，将SEN的值赋给IP即可）
	for( i=0;i<g_pLabRTGlobal->m_MaxLabLine;i++)
	{
		//赋值位置信息
		g_pLabRTGlobal->m_LabRam[i][RLAB_IP_FWD_POS_IN_SEN] = 0;				//赋值第30列（SEN中当前IP的正序位置）
		g_pLabRTGlobal->m_LabRam[i][RLAB_IP_BWD_POS_IN_SEN] = 0;				//赋值第40列（SEN中当前IP的反序位置）

		//赋值数目信息
		g_pLabRTGlobal->m_LabRam[i][RLAB_IP_NUM_IN_C_SEN] = 1;				//赋值第50列（当前SEN中IP的数目）
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_L_IP] = 0;				//赋值第53列（前一IP中SYL的数目）
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_L_IP] = 0;				//赋值第55列（前一IP中PW的数目）	
		g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_L_IP] = 0;				//赋值第56列（前一IP中PP的数目）	
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_R_IP] = 0;				//赋值第59列（后一IP中SYL的数目）
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_R_IP] = 0;				//赋值第61列（后一IP中PW的数目）	
		g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_R_IP] = 0;				//赋值第62列（后一IP中PP的数目）
	}

	//赋值句首sil 和句尾sil的位置信息 （句首sil句尾sil： 全部正序反序位置都是0xff）
	for( i=21;i<41;i++)
	{
		g_pLabRTGlobal->m_LabRam[0][i] = SilPau_PlaceCode;	
		g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][i] = SilPau_PlaceCode;
	}

	//赋值句首sil 和句尾sil的数目信息  
	for( i=41;i<LAB_CN_LINE_LEN;i++)
	{
		g_pLabRTGlobal->m_LabRam[0][i] = 0;		//先全部初始化为0 ，实际上 第57至第62列不是0， 会在后面重新赋值
		g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][i] =       0;		//先全部初始化为0 ，实际上 第51至第56列不是0， 会在后面重新赋值
	}

	//单独赋值：句首sil 和句尾sil的 第55,56,61，62,53,59列
	g_pLabRTGlobal->m_LabRam[0][RLAB_PW_NUM_IN_R_IP] = nMaxOfPWInSEN;	
	g_pLabRTGlobal->m_LabRam[0][RLAB_PPH_NUM_IN_R_IP] = nMaxOfPPHInSEN;	
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_PW_NUM_IN_L_IP] = nMaxOfPWInSEN;		
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_PPH_NUM_IN_L_IP] = nMaxOfPPHInSEN;	
	g_pLabRTGlobal->m_LabRam[0][RLAB_SYL_NUM_IN_R_IP] = nMaxOfHanZiInSEN;	
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_SYL_NUM_IN_L_IP] = nMaxOfHanZiInSEN;	

	nCountOfSylInSen++;

	nStartCountOfSylInPPH = 1;
	nCurCountOfSylInPPH = 1;

	g_nCurTextByteIndex = 0;

	//单独赋值：句首sil  第57,58,60列
	g_pLabRTGlobal->m_LabRam[0][RLAB_SYL_NUM_IN_R_PW] = GetMaxHanZiInNextPW(g_nCurTextByteIndex,emTrue);				//57列：（后一PW中SYL的数目）
	g_pLabRTGlobal->m_LabRam[0][RLAB_SYL_NUM_IN_R_PPH] = GetMaxHanZiInNextPPH(g_nCurTextByteIndex,emTrue);				//58列：（后一PP中SYL的数目）
	g_pLabRTGlobal->m_LabRam[0][RLAB_PW_NUM_IN_R_PPH] = GetMaxPWInNextPPH(g_nCurTextByteIndex,emTrue);				//60列：（后一PP中PW的数目）

	nMaxOfHanZiInNextPPH = GetMaxHanZiInNextPPH(g_nCurTextByteIndex,emFalse);
	nMaxOfPWInNextPPH = GetMaxPWInNextPPH(g_nCurTextByteIndex,emFalse);

	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		if( g_pTextInfo[g_nCurTextByteIndex/2].BorderType == BORDER_PPH )		//若本节点 是 边界节点（PPH）
		{
			//赋值pau的位置信息 （pau ： 全部正序反序位置都是0xff，）
			for( i=21;i<41;i++)
			{
				g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][i] = SilPau_PlaceCode;	
			}

			//赋值pau的数目信息  (第51列至第62列会在处理汉字音节时赋值)
			for( i=41;i<51;i++)
			{
				g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][i] = 0;		
			}

			for( i=nStartCountOfSylInPPH; i<nCurCountOfSylInPPH; i++)
			{	
				//赋值位置信息
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] = i - nStartCountOfSylInPPH;								//赋值PPH之前的 第22列（PP中当前SYL的正序位置）
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] = nCurCountOfSylInPPH - i - 1;							//赋值PPH之前的 第32列（PP中当前SYL的反序位置）  
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] = nCountOfPWInCurPPH - g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] - 1;					//赋值PPH之前的 第35列（PP中当前PW的反序位置）  

				//赋值数目信息
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] = nCurCountOfSylInPPH - nStartCountOfSylInPPH;			//赋值PPH之前的 第42列（当前PP中SYL的数目）
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] = nCountOfPWInCurPPH;										//赋值PPH之前的 第45列（当前PP中PW的数目）
			}

			nMaxOfHanZiInPrevPPH = nCountOfHanZiInPrevPPH;
			nMaxOfPWInPrevPPH = nCountOfPWInPrevPPH;
			nCountOfHanZiInPrevPPH = 0;			
			nCountOfPWInPrevPPH = 0;			

			nStartCountOfSylInPPH =  nCurCountOfSylInPPH + 1;
			nCurCountOfSylInPPH = nStartCountOfSylInPPH;
			nCountOfHanZiInCurPPH  = 0;			
			nCountOfPWInCurPPH = 0;	
			nCountOfPPHInSEN++;		
			
			//g_nCurTextByteIndex  += g_pTextInfo[g_nCurTextByteIndex/2].Len;

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_L_PW] = nMaxOfHanZiInPrevPW;						//赋值当前pau的 第51列（前一PW中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_L_PPH] = nMaxOfHanZiInPrevPPH;					//赋值当前pau的 第52列（前一PP中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_PW_NUM_IN_L_PPH] = nMaxOfPWInPrevPPH;						//赋值当前pau的 第54列（前一PP中PW的数目)

			nMaxOfHanZiInNextPW = GetMaxHanZiInNextPW(g_nCurTextByteIndex,emFalse);
			nMaxOfHanZiInNextPPH = GetMaxHanZiInNextPPH(g_nCurTextByteIndex,emFalse);
			nMaxOfPWInNextPPH = GetMaxPWInNextPPH(g_nCurTextByteIndex,emFalse);

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_R_PW] = nMaxOfHanZiInNextPW;						//赋值当前pau的 第57列（后一PW中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_R_PPH] = nMaxOfHanZiInNextPPH;					//赋值当前pau的 第58列（后一PP中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_PW_NUM_IN_R_PPH] = nMaxOfPWInNextPPH;						//赋值当前pau的 第60列（后一PP中PW的数目)

			nCountOfSylInSen++;
		}
		
		nHanZiCount = g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
		nCurCountOfSylInPPH += nHanZiCount;
		nMaxOfHanZiInNextPW = GetMaxHanZiInNextPW(g_nCurTextByteIndex,emFalse);
		
		for(i = 0; i< nHanZiCount;i++)
		{
			//赋值位置信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_PW] = i;										//赋值当前韵律词的 第21列（PW中当前SYL的正序位置）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_PW] = nHanZiCount - i - 1;					//赋值当前韵律词的 第31列（PW中当前SYL的反序位置）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_IP] = nCountOfHanZiInSEN + i;				//赋值当前韵律词的 第23列（IP中当前SYL的正序位置）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_IP] = nMaxOfHanZiInSEN-nCountOfHanZiInSEN-i-1;//赋值当前韵律词的 第33列（IP中当前SYL的反序位置）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_SEN] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_IP];		//赋值当前韵律词的 第24列（SEN中当前SYL的正序位置）=23列 （IP等同于SEN，将SEN的值赋给IP即可）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_SEN] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_IP];		//赋值当前韵律词的 第34列（SEN中当前SYL的反序位置）=33列 （IP等同于SEN，将SEN的值赋给IP即可）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_PPH] = nCountOfPWInCurPPH;					//赋值当前韵律词的 第25列（PP中当前PW的正序位置）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_SEN] = nCountOfPWInSEN;						//赋值当前韵律词的 第27列（SEN中当前PW的正序位置）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_BWD_POS_IN_SEN] = nMaxOfPWInSEN-nCountOfPWInSEN-1;		//赋值当前韵律词的 第37列（SEN中当前PW的反序位置）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_SEN];		//赋值当前韵律词的 第26列（IP中当前PW中的正序位置）=27列 （IP等同于SEN，将SEN的值赋给IP即可）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_BWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_BWD_POS_IN_SEN];		//赋值当前韵律词的 第36列（IP中当前PW中的反序位置）=37列 （IP等同于SEN，将SEN的值赋给IP即可）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_FWD_POS_IN_SEN] = nCountOfPPHInSEN;						//赋值当前韵律词的 第29列（SEN中当前PP的正序位置）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_BWD_POS_IN_SEN] = nMaxOfPPHInSEN-nCountOfPPHInSEN;		//赋值当前韵律词的 第39列（SEN中当前PP的反序位置）

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_FWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_FWD_POS_IN_SEN];		//赋值当前韵律词的 第28列（IP中当前PP的正序位置）=29列 （IP等同于SEN，将SEN的值赋给IP即可）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_BWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_BWD_POS_IN_SEN];		//赋值当前韵律词的 第38列（IP中当前PP的反序位置）=39列 （IP等同于SEN，将SEN的值赋给IP即可）

			
			//赋值数目信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_PW] = nHanZiCount;							//赋值当前韵律词的 第41列（当前PW中SYL的数目）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_SEN] = nMaxOfHanZiInSEN;						//赋值当前韵律词的 第44列（当前SEN中SYL的数目）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_SEN];		//赋值当前韵律词的 第43列（当前IP中SYL的数目）=44列
			
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_C_SEN] = nMaxOfPWInSEN;							//赋值当前韵律词的 第47列（当前SEN中PW的数目）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_C_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_C_SEN];		//赋值当前韵律词的 第46列（当前IP中PW中的数目）=47列

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_NUM_IN_C_SEN] = nMaxOfPPHInSEN;						//赋值当前韵律词的 第49列（当前SEN中PP的数目）
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_NUM_IN_C_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_NUM_IN_C_SEN];		//赋值当前韵律词的 第48列（当前IP中PP的数目）=49列

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_L_PW] = nMaxOfHanZiInPrevPW;					//赋值当前韵律词的 第51列（前一PW中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_L_PPH] = nMaxOfHanZiInPrevPPH;					//赋值当前韵律词的 第52列（前一PP中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_L_PPH] = nMaxOfPWInPrevPPH;						//赋值当前韵律词的 第54列（前一PP中PW的数目)

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_R_PW] = nMaxOfHanZiInNextPW;					//赋值当前韵律词的 第57列（后一PW中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_R_PPH] = nMaxOfHanZiInNextPPH;					//赋值当前韵律词的 第58列（后一PP中SYL的数目)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_R_PPH] = nMaxOfPWInNextPPH;						//赋值当前韵律词的 第60列（后一PP中PW的数目)

		}

		nMaxOfHanZiInPrevPW = nHanZiCount;
		nCountOfHanZiInPrevPPH += nHanZiCount;
		nCountOfPWInPrevPPH++;
		nCountOfHanZiInCurPPH += nHanZiCount;			
		nCountOfPWInCurPPH++;				
		nCountOfHanZiInSEN += nHanZiCount;				
		nCountOfPWInSEN++;				
		nCountOfSylInSen += nHanZiCount;
		
		g_nCurTextByteIndex  += g_pTextInfo[g_nCurTextByteIndex/2].Len;

	}

	//跳出循环后，对最后1个PPH赋值
	for( i=nStartCountOfSylInPPH; i<nCurCountOfSylInPPH; i++)
	{							
		//赋值位置信息
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] = i - nStartCountOfSylInPPH;								//赋值PPH之前的 第22列（PP中当前SYL的正序位置）
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] = nCurCountOfSylInPPH - i - 1;							//赋值PPH之前的 第32列（PP中当前SYL的反序位置）  
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] = nCountOfPWInCurPPH - g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] - 1;					//赋值PPH之前的 第35列（PP中当前PW的反序位置）  

		//赋值数目信息
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] = nCurCountOfSylInPPH - nStartCountOfSylInPPH;			//赋值PPH之前的 第42列（当前PP中SYL的数目）
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] = nCountOfPWInCurPPH;										//赋值PPH之前的 第45列（当前PP中PW的数目）
	}
	nMaxOfHanZiInPrevPPH = nCountOfHanZiInPrevPPH;
	nMaxOfPWInPrevPPH = nCountOfPWInPrevPPH;
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_SYL_NUM_IN_L_PW] = nMaxOfHanZiInPrevPW;
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_SYL_NUM_IN_L_PPH] = nMaxOfHanZiInPrevPPH;
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_PW_NUM_IN_L_PPH] = nMaxOfPWInPrevPPH;

}

//*****************************************************************************************************
//函数功能： 在汉字链表中：将韵律词中多个词合并成一个节点（pos属性已无意义），去掉PW节点，每个汉字节点即为1个单独的PW
//*****************************************************************************************************
void emCall MergeWordsOfPW()
{
	emUInt8 nCur=0;

	LOG_StackAddr(__FUNCTION__);

	while(g_pTextInfo[nCur/2].Len != 0)
	{
		if( g_pTextInfo[nCur/2].BorderType > BORDER_LW || nCur==0 )
		{
			g_nNextTextByteIndex = nCur + g_pTextInfo[nCur/2].Len;
			if(g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)
			{
				break;
			}
			while( g_pTextInfo[g_nNextTextByteIndex/2].BorderType <= BORDER_LW )
			{
				g_pTextInfo[nCur/2].Len += g_pTextInfo[g_nNextTextByteIndex/2].Len;
				g_nNextTextByteIndex = g_nNextTextByteIndex + g_pTextInfo[g_nNextTextByteIndex/2].Len;
				if(g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)
				{
					break;
				}
			}
			nCur = g_nNextTextByteIndex;
		}
	}
}


//*****************************************************************************************************
//函数功能： 获得本句中的三个信息（通过指针返回参数）
//		指针返回：  本句中的汉字总数：		pMaxOfHanZiInSEN
//				本句中的韵律词总数：	pMaxOfPWInSEN
//				本句中的韵律短语总数：	pMaxOfPPHInSEN
//		return返回： 本句中的音节个数（含汉字，sil，pau）
//*****************************************************************************************************
emUInt8  emCall	GetSenInfo(emUInt8 *pMaxOfHanZiInSEN, emUInt8 *pMaxOfPWInSEN, emUInt8 *pMaxOfPPHInSEN)
{
	emInt16  nSylCount = 2;  //初始加上前后2个sil音节
	emInt16  nHanZiCount;

	LOG_StackAddr(__FUNCTION__);

	(*pMaxOfHanZiInSEN) = 0;
	(*pMaxOfPWInSEN) = 0;
	(*pMaxOfPPHInSEN) = 0;

	g_nCurTextByteIndex = 0;
	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		
		if( g_pTextInfo[g_nCurTextByteIndex/2].BorderType == BORDER_PPH )		//若本节点 是 边界节点（PPH）
		{
			nSylCount++;
			(*pMaxOfPPHInSEN) += 1;			
		}

		nHanZiCount = g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
		(*pMaxOfHanZiInSEN) += nHanZiCount;
		(*pMaxOfPWInSEN) += 1;
		nSylCount += nHanZiCount;

		g_nCurTextByteIndex  += g_pTextInfo[g_nCurTextByteIndex/2].Len;
	}


	return nSylCount;
}



//*****************************************************************************************************
//函数功能： 将所有音节的拼音信息填写到Lab中去（即Lab中的第0列至第20列）
//*****************************************************************************************************
void emCall	GetPinYinInfoToLab()
{
		//声母发音方式， 声母发音部位
	emUInt8 emConst ShengMuMethodAndPlace[26][2]={   	
						{SM_CN_METHOD_kong,				SM_CN_PLACE_kong},			//	0	NULL
						{SM_CN_METHOD_nil,				SM_CN_PLACE_nil},			//	1	sil
						{SM_CN_METHOD_nil,				SM_CN_PLACE_nil},			//	2	pau
						{SM_CN_METHOD_zero,				SM_CN_PLACE_zero},			//	3	#
						{SM_CN_METHOD_stop,				SM_CN_PLACE_labial},		//	4	b
						{SM_CN_METHOD_asprt_affricate,	SM_CN_PLACE_retroflex},		//	5	ch
						{SM_CN_METHOD_asprt_affricate,	SM_CN_PLACE_dentalveolar},	//	6	c
						{SM_CN_METHOD_stop,				SM_CN_PLACE_dentalveolar},	//	7	d
						{SM_CN_METHOD_fricative,		SM_CN_PLACE_labiodental},	//	8	f
						{SM_CN_METHOD_stop,				SM_CN_PLACE_velar},			//	9	g
						{SM_CN_METHOD_fricative,		SM_CN_PLACE_velar},			//	10	h
						{SM_CN_METHOD_affricate,		SM_CN_PLACE_dorsal},		//	11	j
						{SM_CN_METHOD_asprt_stop,		SM_CN_PLACE_velar},			//	12	k
						{SM_CN_METHOD_lateral,			SM_CN_PLACE_dentalveolar},	//	13	l
						{SM_CN_METHOD_nasal,			SM_CN_PLACE_labial},		//	14	m
						{SM_CN_METHOD_nasal,			SM_CN_PLACE_dentalveolar},	//	15	n
						{SM_CN_METHOD_asprt_stop,		SM_CN_PLACE_labial},		//	16	p
						{SM_CN_METHOD_asprt_affricate,	SM_CN_PLACE_dorsal},		//	17	q
						{SM_CN_METHOD_fricative,		SM_CN_PLACE_retroflex},		//	18	r
						{SM_CN_METHOD_fricative,		SM_CN_PLACE_retroflex},		//	19	sh
						{SM_CN_METHOD_fricative,		SM_CN_PLACE_dentalveolar},	//	20	s
						{SM_CN_METHOD_asprt_stop,		SM_CN_PLACE_dentalveolar},	//	21	t
						{SM_CN_METHOD_fricative,		SM_CN_PLACE_dorsal},		//	22	x
						{SM_CN_METHOD_affricate,		SM_CN_PLACE_retroflex},		//	23	zh
						{SM_CN_METHOD_affricate,		SM_CN_PLACE_dentalveolar},	//	24	z
						{SM_CN_METHOD_kong,				SM_CN_PLACE_kong}};			//	25	英文字母
						


	//韵母韵头发音方式， 韵母韵尾发音方式
	emUInt8 emConst YunMuHeadAndRear[88][2]={    
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	0	：	空
						{YM_CN_FRONT_nil,				YM_CN_REAR_nil},				//	1	：	nil
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	2	：	a  
						{YM_CN_FRONT_open,				YM_CN_REAR_strectched},			//	3	：	ai  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	4	：	ao  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	5	：	an  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	6	：	ang  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	7	：	o  
						{YM_CN_FRONT_open,				YM_CN_REAR_round},				//	8	：	ou  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	9	：	e  
						{YM_CN_FRONT_open,				YM_CN_REAR_strectched},			//	10	：	ei  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	11	：	en  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	12	：	eng  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	13	：	er  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	14	：	io  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_strectched},			//	15	：	i  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	16	：	ia  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	17	：	iao  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	18	：	ian  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	19	：	iang  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	20	：	ie  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	21	：	in  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	22	：	ing  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	23	：	iong  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_round},				//	24	：	iou  
						{YM_CN_FRONT_round,				YM_CN_REAR_round},				//	25	：	u  
						{YM_CN_FRONT_round,				YM_CN_REAR_open},				//	26	：	ua  
						{YM_CN_FRONT_round,				YM_CN_REAR_open},				//	27	：	uo  
						{YM_CN_FRONT_round,				YM_CN_REAR_strectched},			//	28	：	uai  
						{YM_CN_FRONT_round,				YM_CN_REAR_strectched},			//	29	：	uei  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	30	：	uan  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	31	：	uen  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	32	：	uang  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	33	：	ueng  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	34	：	ong  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_protruded},			//	35	：	v  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_open},				//	36	：	ve  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_nasal},				//	37	：	van  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_nasal},				//	38	：	vn  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	39	：	ii  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	40	：	iii  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	41	：	air  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	42	：	angr  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	43	：	aor  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	44	：	engr  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	45	：	iaor  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	46	：	iar  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	47	：	ingr  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	48	：	iour  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	49	：	ir  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	50	：	our  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	51	：	uair  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	52	：	uangr  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	53	：	ueir  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	54	：	uor  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	55	：	ur  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_retroflex},			//	56	：	vanr  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_retroflex},			//	57	：	ver  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_retroflex},			//	58	：	vnr  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	59	：	eir  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	60	：	ier  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	61	：	iongr  
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	62	：	字母：a
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	63	：	字母：b
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	64	：	字母：c
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	65	：	字母：d
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	66	：	字母：e
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	67	：	字母：f
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	68	：	字母：g
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	69	：	字母：h
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	70	：	字母：i
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	71	：	字母：j
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	72	：	字母：k
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	73	：	字母：l
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	74	：	字母：m
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	75	：	字母：n
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	76	：	字母：o
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	77	：	字母：p
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	78	：	字母：q
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	79	：	字母：r
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	80	：	字母：s
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	81	：	字母：t
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	82	：	字母：u
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	83	：	字母：v
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	84	：	字母：w
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	85	：	字母：x
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	86	：	字母：y
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong}};				//	87	：	字母：z
									


	emUInt8 nHanZiCount,i;
	emUInt16 nPinYin;
	emUInt8 nShengMu,nYunMu,nShengDiao;
	emUInt8 nCountOfSylInSen = 0;					//本句中的音节计数

	LOG_StackAddr(__FUNCTION__);

	nCountOfSylInSen = 0;

	//句首sil音节： 填本音节中的本音节的信息 
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//句首sil音节： 填本音节中的前音节的信息 （句首sil： 前音节还是sil）
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//句首sil音节： 填下1音节中的前音节的信息
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	nCountOfSylInSen++;

	g_nCurTextByteIndex = 0;
	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		if( g_pTextInfo[g_nCurTextByteIndex/2].BorderType == BORDER_PPH )  //若本节点 是 边界节点（PPH），填pau音节信息
		{

			//pau音节： 填本音节中的本音节的信息 
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = INITIAL_CN_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = FINAL_CN_nil;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = TONE_CN_sil_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_pau][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_pau][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

			//pau音节： 填上1音节中的后音节的信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM] = INITIAL_CN_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM] = FINAL_CN_nil;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_TONE] = TONE_CN_sil_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_pau][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_pau][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

			//pau音节： 填下1音节中的前音节的信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM] = INITIAL_CN_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM] = FINAL_CN_nil;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_TONE] = TONE_CN_sil_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_pau][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_pau][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

			nCountOfSylInSen++;
		}

		nHanZiCount = g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
		for( i= 0; i< nHanZiCount; i++)
		{
			nPinYin = g_pTextPinYinCode[g_nCurTextByteIndex/2+i];
			nYunMu = (nPinYin & 0x7f00)>>8;
			nShengMu = (nPinYin & 0x00f8)>>3;
			nShengDiao = nPinYin & 0x0007;

			//填本音节中的本音节的信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = nShengMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = nYunMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = nShengDiao;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[nShengMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[nShengMu][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[nYunMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[nYunMu][1];

			//填上1音节中的后音节的信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM] = nShengMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM] = nYunMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_TONE] = nShengDiao;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_METHOD] = ShengMuMethodAndPlace[nShengMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[nShengMu][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_FRONT] = YunMuHeadAndRear[nYunMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_REAR] = YunMuHeadAndRear[nYunMu][1];		

			//填下1音节中的前音节的信息
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM] = nShengMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM] = nYunMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_TONE] = nShengDiao;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM_METHOD] = ShengMuMethodAndPlace[nShengMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_SM_PLACE] = ShengMuMethodAndPlace[nShengMu][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM_FRONT] = YunMuHeadAndRear[nYunMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+1][RLAB_L_YM_REAR] = YunMuHeadAndRear[nYunMu][1];

			nCountOfSylInSen++;
		}
		g_nCurTextByteIndex  += g_pTextInfo[g_nCurTextByteIndex/2].Len;

	}

	//句尾sil音节： 填本音节中的本音节的信息 
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//句尾sil音节： 填本音节中的后音节的信息 （句尾sil： 后音节还是sil）
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//句尾sil音节： 填上1音节中的后音节的信息
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];


	//补填句首sil中的下1音节信息		//2012-02-03 hyl modified
	nPinYin = g_pTextPinYinCode[0];
	nYunMu = (nPinYin & 0x7f00)>>8;
	nShengMu = (nPinYin & 0x00f8)>>3;
	nShengDiao = nPinYin & 0x0007;
	g_pLabRTGlobal->m_LabRam[0][RLAB_R_SM] = nShengMu;
	g_pLabRTGlobal->m_LabRam[0][RLAB_R_YM] = nYunMu;
	g_pLabRTGlobal->m_LabRam[0][RLAB_R_TONE] = nShengDiao;

}

