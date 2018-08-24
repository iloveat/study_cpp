
#include "emPCH.h"

#include "Front_ProcHZRhythm_New.h"





void emCall Rhythm_New()		//韵律新方案
{
	emInt32  i = 0;
	emInt16 allNodes;


	PRhythmRes pRhythmRes;
	PRhythmRT	pRhythmRT;
	emInt8 nIndex,j;

	LOG_StackAddr(__FUNCTION__);

	/* 分配内存 */
#if DEBUG_LOG_SWITCH_HEAP
	pRhythmRT = emHeap_AllocZero( sizeof(struct tagRhythmRT), "韵律分析时库：《韵律模块  》");
#else
	pRhythmRT = emHeap_AllocZero( sizeof(struct tagRhythmRT));
#endif
	//emMemSet(pRhythmRT,0,sizeof(struct tagRhythmRT));

#if DEBUG_LOG_SWITCH_HEAP
	pRhythmRT->m_tSylInfo = emHeap_AllocZero( 256*sizeof(TCESylInfo), "韵律分析音节信息：《韵律模块  》");
#else
	pRhythmRT->m_tSylInfo = emHeap_AllocZero( 256*sizeof(TCESylInfo));
#endif
	//emMemSet(pRhythmRT->m_tSylInfo,0,256*sizeof(TCESylInfo));

#if DEBUG_LOG_SWITCH_HEAP
	pRhythmRes = emHeap_AllocZero( sizeof(struct tagRhythmRes) , "韵律分析资源信息：《韵律模块  》");
#else
	pRhythmRes = emHeap_AllocZero( sizeof(struct tagRhythmRes));
#endif
	//emMemSet(pRhythmRes,0,sizeof(struct tagRhythmRes));

#if DEBUG_LOG_SWITCH_HEAP
	g_ResPWRule=emHeap_AllocZero(sizeof(struct tagRuleRes),  "PW规则资源：《韵律模块  》" );
#else
	g_ResPWRule=emHeap_AllocZero(sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPWRule->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack),  "PW规则资源包：《韵律模块  》" );
#else
	g_ResPWRule->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHC45=emHeap_AllocZero(sizeof(struct tagRuleRes),  "PPH资源C45：《韵律模块  》" );
#else
	g_ResPPHC45=emHeap_AllocZero(sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHC45->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack),  "PPH资源C45包：《韵律模块  》" );
#else
	g_ResPPHC45->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHRatio=emHeap_AllocZero(sizeof(struct tagRuleRes),  "PPH概率资源：《韵律模块  》" );
#else
	g_ResPPHRatio=emHeap_AllocZero(sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	g_ResPPHRatio->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack),  "PPH概率资源包：《韵律模块  》" );
#else
	g_ResPPHRatio->m_pResPack=emHeap_AllocZero(sizeof(struct tagResPack));
#endif


	Front_Rhythm_RT_To_Link( pRhythmRT);				//汉字链表 转 pRhythmRT	


	Front_Rhythm_PW_Init( pRhythmRes);

#if PPH_AFTER_DEAL							//PPH边界划分后处理
	//保留PW拼接前的原始词性
	//保留PW拼接前的原始Rhythm
	allNodes = GetSylCount(pRhythmRT);
	for(i = 0 ; i < allNodes; i++)
	{
		pRhythmRT->m_tPoSBeforePW[i]    = pRhythmRT->m_tSylInfo[i].m_tPoS;	
		pRhythmRT->m_tRhythmBeforePW[i] = pRhythmRT->m_tRhythm[i];
	}
#endif 

	Front_Rhythm_PW( pRhythmRes, pRhythmRT);			//根据规则拼接《韵律词》，也会改变语法词的词长

#if DEBUG_LOG_SWITCH_TIME
	myTimeTrace_For_Debug(1,(emByte *)"韵律--拼韵律词", 0);
#endif

#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
	//Print_To_Rhythm("log/日志_韵律.log", "a", 1,"");
#endif

	//判断《韵律标注策略》
	if( g_hTTS->m_ControlSwitch.m_nManualRhythm == emTTS_USE_Manual_Rhythm_OPEN)
	{
		//手动标注韵律
		for( i= 0; i< MAX_MANUAL_PPH_COUNT_IN_SEN; i++)
		{
			nIndex = g_hTTS->m_ManualRhythmIndex[i];
			if( nIndex != 0)
			{
				//将手动标注处设置成：韵律短语边界
				pRhythmRT->m_tSylInfo[nIndex].m_tBoundary = BdBreath;

				//若在韵律词内部手动设置了PPH边界，则强行修改韵律词长
				for( j = nIndex-1;; j--)
				{
					//测试例句：	[z1]我#们的承诺致#力于每一#个细#节的完#美。  
					//测试例句：	[z1]我#们#的#承#诺#致#力#于#每#一#个#细#节#的#完#美。
					if( pRhythmRT->m_tRhythm[j] > 0)
					{
						//韵律词没有被切割，跳出。  (pRhythmRT->m_tRhythm[j]&0x0F)为韵律词长
						//例句：	[z1]我们的承诺#致力于#每一个细节的完美。
						if( (pRhythmRT->m_tRhythm[j]&0x0F) == (nIndex - j) )	
						{
							break;
						}

						//处理以下例句的第二个#（语法词被切割）：	“嵌入式”是一个语法词，也是一个韵律词
						//例句：	[z1]欢迎使#用宇音天下研发的嵌入#式语音合成系统
						if( pRhythmRT->m_tWordLen[j] != (nIndex - j) )
						{
							pRhythmRT->m_tRhythm[nIndex]   = pRhythmRT->m_tRhythm[j] - (nIndex - j);
							pRhythmRT->m_tWordLen[nIndex]  = pRhythmRT->m_tWordLen[j] - (nIndex - j);

							pRhythmRT->m_tRhythm[j]  = (nIndex - j);
							pRhythmRT->m_tWordLen[j]  = (nIndex - j);
							break;
						}

						//处理以下例句的第一个#（韵律词被切割）：	“使用”是一个韵律词，“使”和“用”分别是语法词
						//例句：	[z1]欢迎使#用宇音天下研发的嵌入#式语音合成系统
						if( pRhythmRT->m_tWordLen[j] != (pRhythmRT->m_tRhythm[j]&0x0F)  )
						{
							pRhythmRT->m_tRhythm[nIndex]   = pRhythmRT->m_tRhythm[j] - pRhythmRT->m_tWordLen[j];							
							pRhythmRT->m_tRhythm[j]  = pRhythmRT->m_tWordLen[j];
							break;
						}

						break;
						
					}

				}
			}
		}
	}
	else
	{
		//程序自动标注韵律
		Front_Rhythm_PPH_Init(  pRhythmRes );
		Front_Rhythm_PPH( pRhythmRes,pRhythmRT);			//分析出《韵律短语》
	}



	Front_Rhythm_Link_To_RT( pRhythmRT);				//pRhythmRT 转 汉字链表 

//#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
//	Print_To_Rhythm("log/日志_韵律.log", "a", 1,"");
//#endif

	

	/* 释放内存 */	
#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHRatio->m_pResPack, sizeof(struct tagResPack),  "PPH概率资源包：《韵律模块  》" );
#else
	emHeap_Free(g_ResPPHRatio->m_pResPack, sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHRatio, sizeof(struct tagRuleRes),  "PPH概率资源：《韵律模块  》" );
#else
	emHeap_Free(g_ResPPHRatio, sizeof(struct tagRuleRes));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHC45->m_pResPack, sizeof(struct tagResPack),  "PPH资源C45包：《韵律模块  》" );
#else
	emHeap_Free(g_ResPPHC45->m_pResPack, sizeof(struct tagResPack) );
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPPHC45, sizeof(struct tagRuleRes),  "PPH资源C45：《韵律模块  》" );
#else
	emHeap_Free(g_ResPPHC45, sizeof(struct tagRuleRes));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPWRule->m_pResPack,sizeof(struct tagResPack),  "PW规则资源包：《韵律模块  》" );
#else
	emHeap_Free(g_ResPWRule->m_pResPack,sizeof(struct tagResPack));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(g_ResPWRule,sizeof(struct tagRuleRes),  "PW规则资源：《韵律模块  》" );
#else
	emHeap_Free(g_ResPWRule,sizeof(struct tagRuleRes));
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRhythmRes, sizeof(struct tagRhythmRes) , "韵律分析资源信息：《韵律模块  》");
#else
	emHeap_Free( pRhythmRes, sizeof(struct tagRhythmRes));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRhythmRT->m_tSylInfo, 256*sizeof(TCESylInfo), "韵律分析音节信息：《韵律模块  》");
#else
	emHeap_Free( pRhythmRT->m_tSylInfo, 256*sizeof(TCESylInfo));
#endif

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free( pRhythmRT,sizeof(struct tagRhythmRT), "韵律分析时库：《韵律模块  》");
#else
	emHeap_Free( pRhythmRT,sizeof(struct tagRhythmRT));
#endif


#if DEBUG_LOG_SWITCH_TIME
	myTimeTrace_For_Debug(1,(emByte *)"韵律--分析短语", 0);
#endif

}


// ************************************************************************************************************************** 
// 强制调整PPH：1.根据“成对标点强制PPH信息”  ，	即根据：成对标点的前标点的位置 和 前后标点之间的音节个数   来调整PPH
// 目前仅支持一对成对标点的识别，若有两对及以上标点，仅识别最后一对
//                                                                                                                            
//  /作为任务的飞/控“神经中枢”/									/作为任务的飞控/“神经中枢”/
//	/一只曲子/一直回响/在我的脑海中《天堂中/有没有车来车往》/		/一只曲子/一直回响/在我的脑海中/《天堂中有没有车来车往》/
//	/“玉兴八号”两艘商船5日上午/在湄公河金三角水域遭遇袭击/		/“玉兴八号”/两艘商船5日上午/在湄公河金三角水域遭遇袭击/
//      
//强制调整PPH：2.两字短韵律清除			例如：/我们/这种沾沾自喜的情绪/在不知不觉当中/传染给了囡囡/。(第1个短韵将清除)
//										例如：/当前/工作的重中之重/。
// **************************************************************************************************************************                                                    */


void emCall Rhythm_ForcePPH_Basic()
{
//强制PPH的调整规则:
//											//成对标点：规则1：若本标点间的字数>1,且前标点不是句首，则在前标点处无条件新增
//											//成对标点：规则2：在前标点和后标点之间的PPH有条件的删除
#define PPH_PREV_COUNT_DELETE	4			//成对标点：规则3：在前标点前的几个字以内出现的PPH直接删除
#define PPH_NEXT_COUNT_MOVE		4			//成对标点：规则4：在后标点后的几个字以内出现的PPH将其移到后标点处
#define PPH_NEXT_COUNT_INSERT	7			//成对标点：规则5：在后标点后的几个字以内未出现PPH，则直接在后标点处新增PPH
#define PPH_FINAL_COUNT_INSERT	12			//成对标点：规则6：执行完以上规则后，若前标点至之后的PPH的音节数大于几个字，则直接在后标点处新增PPH
	emInt16 i,count=0, nPPHSylCount;

	LOG_StackAddr(__FUNCTION__);

	//强制调整PPH：1.根据“成对标点强制PPH信息”  ，
	if( g_ForcePPH.nSylLen > 0)
	{
		//实现：规则2
		if(g_ForcePPH.nSylLen <= 10 )		//若：前后标点间的音节<=10，之间的PPH全部删除
		{
			for( i = g_ForcePPH.nStartIndex+1 ; i<(g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen-1 );i++)
			{
				if( g_pTextInfo[i].BorderType == BORDER_PPH)
				{
					g_pTextInfo[i].BorderType = BORDER_PW;
				}
			}
		}
		else								//若：前后标点间的音节>10，
		{
			//前标点后的4个音节内的PPH删除，
			for( i = g_ForcePPH.nStartIndex+1 ; i<=(g_ForcePPH.nStartIndex + 4 );i++)
			{
				if( g_pTextInfo[i].BorderType == BORDER_PPH)
				{
					g_pTextInfo[i].BorderType = BORDER_PW;
				}
			}
			//后标点前的4个音节内的PPH删除
			for( i = (g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen-1 ) ; i>=(g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen -4);i--)
			{
				if( g_pTextInfo[i].BorderType == BORDER_PPH)
				{
					g_pTextInfo[i].BorderType = BORDER_PW;
				}
			}
		}

		//实现：规则3
		count=0;
		for( i = g_ForcePPH.nStartIndex ; i>0; i--)
		{
			if( g_pTextInfo[i].BorderType == BORDER_PPH)
			{
				g_pTextInfo[i].BorderType = BORDER_PW;
			}
			count++;
			if( count>PPH_PREV_COUNT_DELETE)
				break;
		}

		//实现：规则1
		if( g_ForcePPH.nSylLen != 1 && g_ForcePPH.nStartIndex != 0)
			g_pTextInfo[g_ForcePPH.nStartIndex ].BorderType = BORDER_PPH;

		//统计后标点处到之后的PPH边界之间有多少个音节，保存在count中
		count=0;
		for( i = g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen; i< g_nLastTextByteIndex/2 ;i++)
		{
			count++;
			if( g_pTextInfo[i+1].BorderType == BORDER_PPH)
			{
				break;
			}				
		}

		//实现：规则4
		if( count <= PPH_NEXT_COUNT_MOVE && i != (g_nLastTextByteIndex/2) )
		{
			g_pTextInfo[g_ForcePPH.nStartIndex+g_ForcePPH.nSylLen ].BorderType = BORDER_PPH;

			g_pTextInfo[i+1].BorderType = BORDER_PW;
		}

		//实现：规则5
		if( count > (PPH_NEXT_COUNT_INSERT) )
		{
			g_pTextInfo[g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen ].BorderType = BORDER_PPH;
		}

		//统计前标点处到之后的PPH边界之间有多少个音节，保存在count中
		count=0;
		for( i = g_ForcePPH.nStartIndex ; i< g_nLastTextByteIndex/2 ;i++)
		{
			count++;
			if( g_pTextInfo[i+1].BorderType == BORDER_PPH)
			{
				break;
			}				
		}

		//实现：规则6
		if( count > (PPH_FINAL_COUNT_INSERT) )
		{
			g_pTextInfo[g_ForcePPH.nStartIndex +g_ForcePPH.nSylLen ].BorderType = BORDER_PPH;
		}
	}


	//强制调整PPH：2.两字短韵律清除	
	count = 0;
	nPPHSylCount = 0;
	//判断
	if( g_hTTS->m_ControlSwitch.m_nManualRhythm == emTTS_USE_Manual_Rhythm_CLOSE)  //若：手动韵律标注是关闭的
	{
		while(g_pTextInfo[count].Len != 0 && count<=MAX_HANZI_COUNT_OF_LINK)
		{
			if( g_pTextInfo[count].BorderType == BORDER_PPH)
			{
				if(    nPPHSylCount<=2							//前1韵律短语长度<=2
					|| (g_nLastTextByteIndex/2 - count) <= 2 )	//韵律短语边界在句尾的2字内
				{
					g_pTextInfo[count].BorderType = BORDER_PW;	//两字短韵律清除
				}

				nPPHSylCount = g_pTextInfo[count].Len/2;		
			}
			else
			{
				nPPHSylCount += g_pTextInfo[count].Len/2;			
			}
			count += g_pTextInfo[count].Len/2;
		}
	}

}

void Front_Rhythm_RT_To_Link( PRhythmRT  pRhythmRT)			//汉字链表  转  pRhythmRT
{
	emUInt8 nWordCount = 0, i = 0,j;  

	emConst emUInt8 nNewPos[128] =					//词性转换表
   {0,	1,	3,	1,	1,	1,	1,	2,	3,	3,			//  0 -  9     注意：ad转成d更好（比ad转成a好）
	3,	3,	3,	3,	3,	3,	19,	19,	4,	4,			// 10 - 19
	0,	14,	1,	1,	1,	1,	1,	1,	6,	13,			// 20 - 29
	6,	6,	3,	6,	6,	6,	15,	1,	1,	1,			// 30 - 39
	1,	3,	1,	6,	13,	5,	5,	5,	6,	6,			// 40 - 49
	6,	6,	6,	20,	6,	6,	6,	6,	6,	6,			// 50 - 59		注意：nrf和nrg转成n更好（比nrf和nrg转成ngp好）
	6,	16,	8,	8,	9,	9,	9,	9,	9,	9,			// 60 - 69
	9,	9,	9,	9,	9,	9,	17,	17,	18,	17,			// 70 - 79
	17,	10,	11,	11,	11,	12,	12,	12,	12,	12,			// 80 - 89
	12,	12,	12,	12,	13,	13,	13,	13,	13,	13,			// 90 - 99
	6,	13,	13,	13,	13,	13,	0,	0,	0,	0,			//100 -109
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,			//110 -119
	0,	0,	19,	19,	1,	1,	0,	0};					//120 -127

	LOG_StackAddr(__FUNCTION__);

	g_nP1TextByteIndex = 0;

	pRhythmRT->m_nTextLen = 0;
	pRhythmRT->m_nSylCount = 0 ;

	i = 0;
	pRhythmRT->m_cText = g_pText;
	while(g_pTextInfo[g_nP1TextByteIndex/2].Len != 0)
	{		
		nWordCount = g_pTextInfo[g_nP1TextByteIndex/2].Len / 2;  //统计字数
		pRhythmRT->m_nTextLen += nWordCount*2;
		pRhythmRT->m_nSylCount += nWordCount ;

		//写每个词的词头信息
		pRhythmRT->m_tRhythm[i] = nWordCount;
		pRhythmRT->m_tWordLen[i] = nWordCount;

		/*  初始化m_nFrCost：这4种结果一摸一样（=60,=70,=80, = (nRatio+10000)/100)）； （=0 比 =60 好像更好一些，但好像分PPH太碎）   */
		pRhythmRT->m_nFrCost[i] = 60;								//？？？ 
		pRhythmRT->m_tSylInfo[i].m_iSylText = i*2;
		pRhythmRT->m_tSylInfo[i].m_tBoundary = BdNull;
		pRhythmRT->m_tSylInfo[i].m_tTone = 0;
		pRhythmRT->m_tSylInfo[i].m_tPoS = nNewPos[g_pTextInfo[g_nP1TextByteIndex/2].Pos];		//？？？
		i++;

		for(j = 1;j<nWordCount;j++)
		{
			//写每个词的非词头信息
			pRhythmRT->m_tRhythm[i]  = -1;
			pRhythmRT->m_tWordLen[i] = -1;
			pRhythmRT->m_nFrCost[i] = 60;						//？？？ 
			pRhythmRT->m_tSylInfo[i].m_iSylText = i*2;
			pRhythmRT->m_tSylInfo[i].m_tBoundary = BdNull;
			pRhythmRT->m_tSylInfo[i].m_tTone = 0;
			pRhythmRT->m_tSylInfo[i].m_tPoS = 0;
			i++;
		}
		g_nNextTextByteIndex = g_nP1TextByteIndex + g_pTextInfo[g_nP1TextByteIndex/2].Len;
		g_nP1TextByteIndex = g_nNextTextByteIndex;
	}

	//写句尾信息
	pRhythmRT->m_tRhythm[i]  = 0;
	pRhythmRT->m_tWordLen[i] = 0;
	pRhythmRT->m_nFrCost[i] = 0;
	pRhythmRT->m_tSylInfo[i].m_iSylText = i*2;
	pRhythmRT->m_tSylInfo[i].m_tBoundary = BdNull;
	pRhythmRT->m_tSylInfo[i].m_tTone = 0;
	pRhythmRT->m_tSylInfo[i].m_tPoS = 0;
	i++;
}



void Front_Rhythm_Link_To_RT(PRhythmRT pRhythmRT)		//pRhythmRT 转 汉字链表 
{
	emInt16 i = 0,count, nRhythmLen;
	emInt8  nOldNextLen, nNewCurLen, nNewNextLen;	

	emUInt8 nBefore = 0, nNext = 0, nCur = 0; 
	emUInt8 nNew = 0;

	LOG_StackAddr(__FUNCTION__);

	nCur = 0;
	nBefore = nCur;

	while( i < pRhythmRT->m_nSylCount )
	{		
		if( pRhythmRT->m_tRhythm[i] > 0 )
		{
			count = 0;

			count += (g_pTextInfo[nCur/2].Len)/2;
			nRhythmLen = ((pRhythmRT->m_tRhythm[i])&0x0F);
			while( count <  nRhythmLen)
			{
				if( pRhythmRT->m_tSylInfo[i+count].m_tBoundary == BdBreath )
				{
					g_pTextInfo[nCur/2 + count].BorderType = BORDER_PPH;
				}
				
				g_nNextTextByteIndex = nCur + g_pTextInfo[nCur/2].Len;
				nCur = g_nNextTextByteIndex;
				count += (g_pTextInfo[nCur/2].Len)/2;
			}
			
			if( (i+nRhythmLen) == pRhythmRT->m_nSylCount  )
			{
				break;
			}

			g_nNextTextByteIndex = nCur + g_pTextInfo[nCur/2].Len;
			nNext = g_nNextTextByteIndex;

			if( count >  nRhythmLen )			//处理组合：4字词+1字词：被拆成：2字词+3字词 等等
			{
				nOldNextLen = g_pTextInfo[nNext/2].Len;
				nNewCurLen	= nRhythmLen*2;
				nNewNextLen = nOldNextLen + ( count - nRhythmLen )*2;

				g_pTextInfo[nCur/2].Len = nNewCurLen;				
				g_pTextInfo[(nCur + nNewCurLen)/2].Len = nNewNextLen;
				g_pTextInfo[(nCur + nNewCurLen)/2].Pos = g_pTextInfo[nNext/2].Pos;
				g_pTextInfo[(nCur + nNewCurLen)/2].BorderType = g_pTextInfo[nNext/2].BorderType;
				g_pTextInfo[(nCur + nNewCurLen)/2].TextType= g_pTextInfo[nNext/2].TextType;
				g_pTextInfo[(nCur + nNewCurLen)/2].nBiaoDian[0] = g_pTextInfo[nCur/2].nBiaoDian[0];
				g_pTextInfo[(nCur + nNewCurLen)/2].nBiaoDian[1] = g_pTextInfo[nCur/2].nBiaoDian[1];
				g_pTextInfo[nCur/2].nBiaoDian[0] = 0;
				g_pTextInfo[nCur/2].nBiaoDian[1] = 0;
				nNext = nCur + nNewCurLen;
			}			

			if( pRhythmRT->m_tSylInfo[i+nRhythmLen].m_tBoundary == BdBreath )
			{
				g_pTextInfo[nNext/2].BorderType = BORDER_PPH;
			}
			else
			{
				g_pTextInfo[nNext/2].BorderType = BORDER_PW;
			}		
			nCur = nNext;
			i += nRhythmLen;			
		}
	}
}