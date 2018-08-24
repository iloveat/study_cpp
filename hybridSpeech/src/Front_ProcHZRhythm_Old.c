#include "emPCH.h"

#include "Front_ProcHZRhythm_Old.h"



#if	!EM_SYS_SWITCH_RHYTHM_NEW

//韵律老方案的最优： PW（0.55） + PPH（0.90,0.65） + PPH特征词

void emCall Rhythm_Old()
{

	LOG_StackAddr(__FUNCTION__);

			//Each8ToPau_For_Debug();

			//韵律预测第1遍
			EvalPW(0.55);					//目前（0.55）是测试的最佳值



		#if DEBUG_LOG_SWITCH_RHYTHM_MIDDLE
			Print_To_Rhythm("log/日志_韵律.log", "a", 1,"分PW：");
		#endif



			//韵律预测第2遍
			EvalPPH(0.90, 0.65);				//目前（0.90，0.65）是测试的最佳值
		


		#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
			Print_To_Rhythm("log/日志_韵律.log", "a", 1,"分PH：");
		#endif

}



emBool IsHaveTeZhengCi( emUInt8  nCurNode )		
{

	emCharA* beforeCiOne[]	= { "等", "说", "讲" };

	emCharA* afterCiOne[]	= { "是", "并","从","但","到","而","跟","共","和","或","及","就","乃","且",
							"请","却","仍","如","若","同","像","以","与","则","至","致"};

	emCharA* afterCiTwo[]	= { "比方","并及","不但","不过","不及","不料","不如","不是","才是",
							"此外","但是","等于","都是","而且","而是","固然","好比","何况",
							"还是","即使","既然","假如","假使","尽管","就是","可是","况且",
							"例如","那就","譬如","偏偏","岂知","然而","如果","如同","若是",
							"是故","说到","似乎","虽然","所以","倘若","要是","一般","一边",
							"以便","以及","以致","因此","因而","因为","由于","于是","与同",
							"原来","只是","至于","纵然"};

	emInt32  i;
	emUInt nNextNode, nNextNextNode;

	LOG_StackAddr(__FUNCTION__);

	//判断PPH的单字《前特征词》
	if( g_pTextInfo[nCurNode/2].Len== 2 )
	{
		i = 0;
		while( i < ( sizeof(beforeCiOne)/ sizeof(emCharA *) ))
		{
			if( emMemCmp( &g_pText[nCurNode] , beforeCiOne[i], 2) == 0 )
			{
#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
				Print_To_Rhythm("log/日志_韵律.log", "a", 1,beforeCiOne[i]);
#endif
				return TRUE;
			}
			i++;
		}
	}

	nNextNode = nCurNode + g_pTextInfo[nCurNode/2].Len;
	if( g_pTextInfo[nNextNode/2].Len > 0 )
	{
		nNextNextNode = nNextNode + g_pTextInfo[nNextNode/2].Len;
		if( g_pTextInfo[nNextNextNode/2].Len > 0 )
		{
			//判断PPH的单字《后特征词》
			if( g_pTextInfo[nNextNextNode/2].Len == 2)
			{
				i = 0;
				while( i < ( sizeof(afterCiOne)/ sizeof(emCharA *) ))
				{
					if( emMemCmp( &g_pText[nNextNextNode] , afterCiOne[i], 2) == 0 )
					{
#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
						Print_To_Rhythm("输出韵律词.txt", "a", 1,afterCiOne[i]);
#endif
						return TRUE;
					}
					i++;
				}
			}

			//判断PPH的双字《后特征词》
			if( g_pTextInfo[nNextNextNode/2].Len == 4 )
			{
				i = 0;
				while( i < ( sizeof(afterCiTwo)/ sizeof(emCharA *) ))
				{
					if( emMemCmp( &g_pText[nNextNextNode] , afterCiTwo[i], 4) == 0 )
					{
#if DEBUG_LOG_SWITCH_RHYTHM_RESULT
						Print_To_Rhythm("log/日志_韵律.log", "a", 1,afterCiTwo[i]);
#endif
						return TRUE;
					}
					i++;
				}
			}
		
		}
	}

	return FALSE;

}



//****************************************************************************************************
//  功能：预测韵律词  add by songkai
//  参数：阀值 
//****************************************************************************************************
void emCall EvalPW(float fFaZhi1)
{
	emUInt8  nPrevBounderType = 0;			//此边界的上1个边界类型		    对应lastBoundaryType
	emUInt8  nPrevPos = 0;					//此边界的前词词性				对应Pos-1
	emUInt8  nPrevPrevPos = 0;				//此边界的前前词词性			对应Pos-2
	emUInt8  nPrevWLen;						//此边界的前词长				对应WLen-1
	float ProbLWSum;	           			//转移概率（韵律词的LW）   
	float ProbPWSum;	           			//转移概率（韵律词的PW）   
	float ProbPPHSum;						//转移概率（韵律词的PPH）  
	emUInt8  nCurPWCount = 0;	
	

	float fSum = 0.0;
	unsigned emCharA BounderQues[PW_PRO_NUM][PW_PRO_VALUE];				//7类：问题值类型+问题值
	emUInt16 nLineNum = 0;//for test					//在PW表中的第几行
	emUInt8 nTmp = 0;
	emUInt8 nTmp2 = 0, nTmp3 = 0;  //pTmp2,pTmp3分别指向当前边界的后节点,后后节点
	emUInt8 nNew = 0;
	emPByte pPW_Table = NULL;

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	pPW_Table = (emPByte)emHeap_AllocZero(PW_LINE_NUM * PW_LINE_BYTE, "载入PW表：《韵律模块  》");			//开辟内存空间存放读入的PW表内容
#else
	pPW_Table = (emPByte)emHeap_AllocZero(PW_LINE_NUM * PW_LINE_BYTE);			//开辟内存空间存放读入的PW表内容
#endif

	fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPW,  0);					//定位到PW表头	
	fFrontRead(pPW_Table, PW_LINE_NUM * PW_LINE_BYTE, 1, g_hTTS->fResFrontMain);	//一次性读入整个表

	while( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
	{
		float PWValue = 0.0, LWValue = 0.0, PPHValue = 0.0;
		float D_PWValue = 0.0, D_LWValue = 0.0, D_PPHValue = 0.0;
		float fValue[3] = {0.0, 0.0, 0.0};
		emUInt i = 0;
		ProbLWSum = 0.0, ProbPWSum = 0.0,  ProbPPHSum = 0.0;

		nCurPWCount += g_pTextInfo[nTmp/2].Len/2;
	
		BounderQues[0][0] = LASTBOUNDARYTYPE;    //此边界的上一边界类型值
		if(nTmp == 0)
		{
			BounderQues[0][1] = 2;
		}
		else
		{
			BounderQues[0][1] = nPrevBounderType;
		}

		BounderQues[1][0] = POS_SUB_1;   //此边界词的前词词性
		BounderQues[1][1] = g_pTextInfo[nTmp/2].Pos;
		nPrevPos = g_pTextInfo[nTmp/2].Pos;       

		BounderQues[2][0] = POS_SUB_2;   //此边界词的前前词词性
		if(nTmp == 0)
		{
			BounderQues[2][1] = START;
		}
		else
		{
			BounderQues[2][1] = nPrevPrevPos;   //前前词词性
		}
		nPrevPrevPos = g_pTextInfo[nTmp/2].Pos; //保存词性  
	
		BounderQues[3][0] = WLen_SUB_1;   //此边界词的前词词长
		BounderQues[3][1] = g_pTextInfo[nTmp/2].Len/ 2;
		nPrevWLen = BounderQues[3][1];

	
		BounderQues[4][0] = POS_ADD_1;   //此边界词的后词词性
		nTmp += g_pTextInfo[nTmp/2].Len ; //指向后词
		if(g_pTextInfo[nTmp2/2].Len != 0)
		{
			BounderQues[4][1] = g_pTextInfo[nTmp2/2].Pos;
		}
		else
		{
			BounderQues[4][1] = END;	
		}
		

		BounderQues[5][0] = POS_ADD_2;   // 此边界的后后词词性
		if(g_pTextInfo[nTmp2/2].Len != 0 || g_pTextInfo[(nTmp2+g_pTextInfo[nTmp2/2].Len)/2].Len != 0)
		{
			BounderQues[5][1] = END;
		}
		else
		{
			nTmp3 = nTmp2+g_pTextInfo[nTmp2/2].Len;		  //指向后后词
			BounderQues[5][1] = g_pTextInfo[nTmp3/2].Pos;
		}
		

		BounderQues[6][0] = WLen_ADD_1;   //此边界的后词词长
		if(g_pTextInfo[nTmp2/2].Len != 0)
		{
			BounderQues[6][1] = 0;
		}
		else
		{
			BounderQues[6][1] = g_pTextInfo[nTmp2/2].Len/ 2;
		}
		
		nPrevBounderType = 0;



		for(i = 0; i < PW_PRO_NUM; i++)
		{
			nLineNum = ErFenSearchOne_FromRAM(BounderQues[i], 0, pPW_Table, 1, PW_LINE_NUM, PW_LINE_BYTE);  //使用一次性把PW表读入内存后的新的二分查找法

			if(nLineNum < PW_LINE_NUM)
			{
				emMemCpy(fValue, pPW_Table + nLineNum * PW_LINE_BYTE + 2, 12);            //拷贝内容出来

				ProbPWSum += fValue[0];
				ProbLWSum += fValue[1];
				ProbPPHSum += fValue[2];
			}
		}

		D_PWValue  = exp(ProbPWSum);
		D_LWValue = exp(ProbLWSum);
		D_PPHValue = exp(ProbPPHSum);
		fSum = D_PWValue + D_LWValue + D_PPHValue;
		D_PWValue  /= fSum;
		D_LWValue  /= fSum;		
		D_PPHValue /= fSum;
		if(nTmp2 != NULL)
		{
			 //比较<PW><PPH><LW>的概率,根据阀值进行判断
			if( D_PWValue == FindMax(D_PWValue, D_LWValue, D_PPHValue) 
				|| ((D_PPHValue == FindMax(D_PWValue, D_LWValue, D_PPHValue)) && (D_PPHValue < fFaZhi1))
				|| (( nCurPWCount >= MAX_PW_LEN_LIMIT ) || (nCurPWCount + g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len/2) > MAX_PW_LEN_LIMIT ))  //强制分PW：PW的长度超过时
			{	//加上pw标签
				g_pTextInfo[nTmp2/2].BorderType = BORDER_PW;
				nPrevBounderType  = 1;

				nCurPWCount = 0;
			}
			else if(D_PPHValue == FindMax(D_PWValue, D_LWValue, D_PPHValue))
			{	//加上PPH标签
				g_pTextInfo[nTmp2/2].BorderType = BORDER_PPH;
				nPrevBounderType  = 1;
				nCurPWCount = 0;
			}
		}

	
		nTmp = nTmp2;
	}

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPW_Table, PW_LINE_NUM * PW_LINE_BYTE, "载入PW表：《韵律模块  》");			//开辟内存空间存放读入的PW表内容
#else
	emHeap_Free(pPW_Table, PW_LINE_NUM * PW_LINE_BYTE);						//开辟内存空间存放读入的PW表内容
#endif

}


//****************************************************************************************************
//  功能：预测韵律短语  add by songkai
//  参数：阀值 
//****************************************************************************************************

void emCall EvalPPH(float fFaZhiHigh, float fFaZhiLow )
{
	emUInt8  nPrevBounderType;			//此PW边界的上1个边界类型				对应lastBoundaryType
	emUInt8  nPrevPos[3];				//此PW边界的上词性						对应Pos-1
	emUInt8  nPrevPrevPo[3];			//此PW边界的上上词性					对应Pos-2
	emUInt8  nPrevWLen[3];				//此PW边界的上词长						对应WLen-1
	emUInt8  ndFront;					//此PW边界前至PPH边界中前所有字长		对应dFront
	emBool	 bNeedAddPPH;
	unsigned emCharA BounderQues[PPH_PRO_NUM][PPH_PRO_VALUE];	//9类：问题值类型+问题值
	emInt16 nStartNum, nLines;     //起始行号,总行数-----新增加的变量
	emUInt16 nLineNum = 0;				//在PPH表中的第几行
	float fTiaoZheng;

	emUInt8	nCurPPHCount = 0;
	emUInt8  nNextPWCount = 0;
	emUInt8  nPosSum = 0, i = 1, nLenSum = 0, j = 0, count = 0;
	emPByte  pPPH_Table_Part1 = NULL, pPPH_Table_Part2 = NULL; //定义三个指针指向PPH表的三个部分
	emPByte  pPPH_Type;							//存放前6类所有边界问题值
	float *ProbNonPPHSum = NULL;	            //存放前6类所有边界问题值转移概率（韵律短语的Non-PPH）   
	float *ProbPPHSum = NULL;	                ////存放前6所有边界问题值转移概率（韵律短语的PPH）  
	float fFaZhi;
	emUInt8 nTmp = 0;
	emUInt8 nTmp2 = 0;      //pTmp2指向上一个韵律边界
	emUInt8 nTmp3 = 0, nTmp4 = 0, nTmp5 = 0, nTmp6 = 0, nTmp7 = 0,nTmp8 = 0,nTmpNext = 0,nTmpNextNext = 0;

	LOG_StackAddr(__FUNCTION__);

#if DEBUG_LOG_SWITCH_HEAP
	pPPH_Table_Part1 = (emPByte)emHeap_AllocZero(PPH_TABLE_LEN_PART_1, "PPH第一部分：《韵律模块  》");		//为PPH的第一部分开辟内存空间
#else
	pPPH_Table_Part1 = (emPByte)emHeap_AllocZero(PPH_TABLE_LEN_PART_1);			//为PPH的第一部分开辟内存空间
#endif


	fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH,  0);            //定位到文件中的第一部分
	fFrontRead(pPPH_Table_Part1, PPH_TABLE_LEN_PART_1, 1, g_hTTS->fResFrontMain);                  //读取数据

#if DEBUG_LOG_SWITCH_HEAP
	ProbNonPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM * 4, "韵律ProbNonPPHSum：《韵律模块  》");
#else
	ProbNonPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM * 4);
#endif
	//emMemSet(ProbNonPPHSum, 0, MAX_NUM * 4);

#if DEBUG_LOG_SWITCH_HEAP
	ProbPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM  * 4, "韵律ProbPPHSum：《韵律模块  》");
#else
	ProbPPHSum = (emPByte)emHeap_AllocZero(MAX_NUM  * 4);
#endif
	//emMemSet(ProbPPHSum, 0, MAX_NUM * 4);

#if DEBUG_LOG_SWITCH_HEAP
	pPPH_Type = (emPByte)emHeap_AllocZero(MAX_NUM * 9 * 4, "韵律pPPH_Type：《韵律模块  》");
#else
	pPPH_Type = (emPByte)emHeap_AllocZero(MAX_NUM * 9 * 4);
#endif
	//emMemSet(pPPH_Type, 0, MAX_NUM * 9 * 4);


	

	while( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
	{
		
		if(g_pTextInfo[nTmp/2].BorderType == BORDER_PW)  //如果当前节点是PW节点
		{
			emMemSet(BounderQues, 0, PPH_PRO_NUM * PPH_PRO_VALUE);  //初始化为0
			emMemSet(nPrevPos, 0, 3);  //初始化为0
			emMemSet(nPrevPrevPo, 0, 3);  //初始化为0
			emMemSet(nPrevWLen, 0, 3);  //初始化为0

			//此PW边界后韵律词的中“词1，词2，。。。 词n  ”的词性之和  POS+1
			i = 1;
			j = 0;
			if(g_pTextInfo[nTmp/2].Len != 0)
			{
				nTmp3 = nTmp ;  //指向词1
				while( g_pTextInfo[nTmp3/2].Len != 0 && g_pTextInfo[nTmp/3].BorderType<= BORDER_LW)   //到下一个韵律词节点或结束
				{
					//只需要最近的三个
					if(i < 4)
					{
						BounderQues[6][i++] = g_pTextInfo[nTmp3/2].Pos;
						//记录取了最近几个.统计个数
						j++;
					}
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			BounderQues[6][0] = POS_ADD_1 * 16 + j;
			emMemCpy(pPPH_Type + count * 24, BounderQues[6], 4);

			nTmpNext = nTmp + g_pTextInfo[nTmp/2].Len;		//相当于pTmp ->Next
			nTmpNextNext = nTmpNext + g_pTextInfo[nTmpNext/2].Len;		//相当于pTmp ->Next->Next

			//此PW边界后韵律词的中“词2，词3，。。。 词n  ”的词性之和（n=1时值为空）  POS+2
			i = 1;
			j = 0;
			if( g_pTextInfo[nTmpNext/2].Len!=0 || g_pTextInfo[nTmpNextNext/2].Len!=0)  //考虑POS+2没有对应节点
			{
				BounderQues[7][1] = POS_CODE_END;
				j = 1;
			}

			
			if( g_pTextInfo[nTmpNext/2].Len!=0 || g_pTextInfo[nTmpNextNext/2].Len!=0)  
			{
				nTmp3 = nTmpNextNext; //指向词2
				//如果存在此节点并且是PW和PPH的话
				if(g_pTextInfo[nTmp3/2].Len!=0 &&  g_pTextInfo[nTmp3/2].BorderType>=BORDER_PW )  //就是说POS+2的第一个词恰好是PW
				{
					j = 1;  //这个时候认为个数为1,值为0
				}
				//如果不是PW和PPH的话,就要遍历知道是为止
				while(g_pTextInfo[nTmp3/2].Len!=0 && g_pTextInfo[nTmp3/2].BorderType<=BORDER_LW)  //到下一个韵律词节点或结束
				{
					//最近两个
					if(i < 3)  //包括词2,词3
					{
						BounderQues[7][i++] = g_pTextInfo[nTmp3/2].Pos;
						j++;
					}
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			BounderQues[7][0] = POS_ADD_2 * 16 + j;
			emMemCpy(pPPH_Type + count * 24 + 4, BounderQues[7], 4);

			//此PW边界前韵律词的中“词1，词2，。。。 词n  ”的词性之和   POS-1
			i = 0;
			j = 0;
			nTmp3 = 0; //指向头节点
			//如果没有前边界的话
			if(g_pTextInfo[nTmp2/2].Len != 0)  //也就是pTmp是第一个PW节点
			{
				//得到最近三个节点中的起始节点指针
				nTmp3 = MostNearThreeNode(0, nTmp);
				while(nTmp3 != nTmp)
				{
					nPrevPos[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			else
			{
				nTmp3 = nTmp2;
				nTmp3 = MostNearThreeNode(nTmp2+g_pTextInfo[nTmp2/2].Len, nTmp);  //得到最近三个
				if( g_pTextInfo[nTmp3/2].BorderType == BORDER_PPH )  //去掉指向PPH modify 2010-10-14
				{
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
				while(nTmp3 != nTmp)
				{
					nPrevPos[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			BounderQues[3][0] = POS_SUB_1 * 16 + j;
			BounderQues[3][1] = nPrevPos[0];  
			BounderQues[3][2] = nPrevPos[1];  
			BounderQues[3][3] = nPrevPos[2]; 
			emMemCpy(pPPH_Type + count * 24 + 8, BounderQues[3], 4);

			//此PW边界前韵律词的中“词1，词2，。。。 词n-1”的词性之和  最近三个词     POS-2  
			i = 0;
			j = 0;
			nTmp3 = 0; //指向头节点
			//没有前边界节点的话
			if(g_pTextInfo[nTmp2/2].Len == 0)  //也就是pTmp是第一个PW节点
			{
				nTmp3 = MostNearThreeNode(0, nTmp4);
				if(nTmp3 == nTmp4)    //考虑POS-2没有对应节点的情况
				{
					nPrevPrevPo[0] = POS_CODE_START;
					j = 1;
				}
				//pTmp4是前一个节点
				while(nTmp3 != nTmp4)
				{
					nPrevPrevPo[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			else
			{
				nTmp3 = nTmp2;
				if((nTmp2 + g_pTextInfo[nTmp2/2].Len) == nTmp4)  //考虑中间值间隔一个词的情况  //modify 2010-10-14
				{
					j = 1;
				}
				nTmp3 = MostNearThreeNode(nTmp2+g_pTextInfo[nTmp2/2].Len, nTmp4);
				if(g_pTextInfo[nTmp3/2].BorderType == BORDER_PPH )  //去掉指向PPH modify 2010-10-14
				{
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j = 1;  //没有的话 是一个
				}
				//pTmp4是前一个节点
				while(nTmp3 != nTmp4)
				{
					nPrevPrevPo[i++] = g_pTextInfo[nTmp3/2].Pos;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					j++;
				}
			}
			BounderQues[4][0] = POS_SUB_2 * 16 + j;
			BounderQues[4][1] = nPrevPrevPo[0];  
			BounderQues[4][2] = nPrevPrevPo[1];  
			BounderQues[4][3] = nPrevPrevPo[2];
			emMemCpy(pPPH_Type + count * 24 + 12, BounderQues[4], 4);


			//此PW边界后韵律词的中“词1，词2，。。。 词n  ”的词长之和  WLEN+1
			i = 1;
			if(g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0)
			{
				nTmp3 += g_pTextInfo[nTmp3/2].Len;  //指向词1
				while(g_pTextInfo[nTmp3/2].Len!=0 && g_pTextInfo[nTmp3/2].BorderType<=BORDER_LW)   //到下一个韵律词节点或结束
				{
					if(i < 4)
					{
						BounderQues[8][1] += g_pTextInfo[nTmp3/2].Len / 2;
						i++;
					}
				nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			BounderQues[8][0] = WLen_ADD_1 * 16 + 1;
			emMemCpy(pPPH_Type + count *24 + 16, BounderQues[8], 4);

			// 此PW边界前韵律词的中“词1，词2，。。。 词n  ”的词长之和   WLEN-1
			i = 0;
			nLenSum = 0;
			nTmp3 = 0; //指向头节点
			if(g_pTextInfo[nTmp2/2].Len!=0)  //也就是pTmp是第一个PW节点
			{
				nTmp3 = MostNearThreeNode(0, nTmp);
				while(nTmp3 != nTmp)
				{
					nPrevWLen[i++] = g_pTextInfo[nTmp3/2].Len / 2;
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
				}
			}
			else
			{
				nTmp5 = nTmp2;
				nTmp5 = MostNearThreeNode(nTmp2+g_pTextInfo[nTmp2/2].Len, nTmp);  //得到最近三个
				if( g_pTextInfo[nTmp5/2].BorderType==BORDER_PPH)
				{
					nTmp5 += g_pTextInfo[nTmp5/2].Len;
				}
				while(nTmp5 != nTmp)
				{
					nPrevWLen[i++] =g_pTextInfo[nTmp5/2].Len / 2;
					nTmp5 += g_pTextInfo[nTmp5/2].Len;
				}
			}
			BounderQues[5][0] = WLen_SUB_1 * 16 + 1;
			BounderQues[5][1] = nPrevWLen[0] + nPrevWLen[1] + nPrevWLen[2];
			BounderQues[5][2] = 0;
			BounderQues[5][3] = 0;
			emMemCpy(pPPH_Type + count * 24 + 20, BounderQues[5], 4);

			nTmp2 = nTmp; //前一个边界节点
			++count;  //统计有多少个边界
		}

		nTmp4 = nTmp;   //前一个节点
		nTmp += g_pTextInfo[nTmp/2].Len;
	}



	//构造前6类所有边界问题值
	for(i = 0; i < 6; i++)
	{
		//得到起始行数
		emMemCpy(&nStartNum, pPPH_Table_Part1 + i * 4, 2);
		//得到总行数
		emMemCpy(&nLines, pPPH_Table_Part1 + i * 4 + 2, 2);

#if DEBUG_LOG_SWITCH_HEAP
		pPPH_Table_Part2 = (emPByte)emHeap_AllocZero(nLines * PPH_LINE_BYTE, "PPH前6类边界值：《韵律模块  》");
#else
		pPPH_Table_Part2 = (emPByte)emHeap_AllocZero(nLines * PPH_LINE_BYTE);
#endif
		//emMemSet(pPPH_Table_Part2, 0, nLines * PPH_LINE_BYTE);

		fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + nStartNum * PPH_LINE_BYTE, 0);   //定位
		fFrontRead(pPPH_Table_Part2, nLines * PPH_LINE_BYTE, 1, g_hTTS->fResFrontMain);            //读取数据
		//某类问题在所有的边界中
		for(j = 0; j < count; j++)  //所有边界的某类问题值
		{
			nLineNum = ErFenSearchTwo_FromRAM(pPPH_Type,  j * 24 + i * 4, pPPH_Table_Part2, 1, nLines, PPH_LINE_BYTE);
			if(nLineNum < nLines)
			{
				float fValue[2] = {0.0, 0.0};
				fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + PPH_TABLE_LEN_PART_2 + AddBaseAddressForSix(i) + nLineNum * 2 * 4, 0); //定位
				//得到NON-PPH和PPH值
				fFrontRead(fValue, 8, 1, g_hTTS->fResFrontMain);
				*(ProbNonPPHSum + j) += fValue[0];
				*(ProbPPHSum + j) += fValue[1];
				/**(ProbNonPPHSum + j + i * count * 4) += fValue[0];
				*(ProbPPHSum + j + i * count * 4) += fValue[1];*/
			}
		}

#if DEBUG_LOG_SWITCH_HEAP
		emHeap_Free(pPPH_Table_Part2 , nLines * PPH_LINE_BYTE, "PPH前6类边界值：《韵律模块  》");
#else
		emHeap_Free(pPPH_Table_Part2,  nLines * PPH_LINE_BYTE);
#endif
	}

	count = 0;
	nTmp = 0;
	nTmp2 = 0, nTmp3 = 0, nTmp4 = 0, nTmp5 = 0, nTmp6 = 0, nTmp7 = 0;
	//读入剩下的第二部分
	emMemCpy(&nStartNum, pPPH_Table_Part1 + 24 , 2);  //剩下的第二部分的起始行

#if DEBUG_LOG_SWITCH_HEAP
	pPPH_Table_Part2 = (emPByte)emHeap_AllocZero((PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE, "PPH后3类边界值：《韵律模块  》");
#else
	pPPH_Table_Part2 = (emPByte)emHeap_AllocZero((PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE);
#endif
	//emMemSet(pPPH_Table_Part2, 0, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE);
	
	fFrontSeek(g_hTTS->fResFrontMain, g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + nStartNum * PPH_LINE_BYTE, 0);   //定位
	fFrontRead(pPPH_Table_Part2, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE, 1, g_hTTS->fResFrontMain);            //读取数据
	//再构造剩下三类
	while( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
	{
		if( g_pTextInfo[nTmp/2].BorderType <= BORDER_LW)
		{
			nCurPPHCount += g_pTextInfo[nTmp/2].Len/2;
		}


		if(g_pTextInfo[nTmp/2].BorderType == BORDER_PPH)  //如果当前节点是PW节点
		{
			nCurPPHCount = 0;
		}

		if(g_pTextInfo[nTmp/2].BorderType == BORDER_PW)  //如果当前节点是PW节点
		{
			float PPHValueSum = 0.0, NON_PPHValueSum = 0.0, sum = 0.0 ,D_PPHValue = 0.0, D_NON_PPHValue = 0.0;;
			emMemSet(BounderQues, 0, PPH_PRO_NUM * PPH_PRO_VALUE);  //初始化为0
			//DBACK:此PW边界后至PPH边界中的所有字长  
			i = 1;
			j = 0;
			if( g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0 )
			{
				nTmp3 += g_pTextInfo[nTmp3/2].Len;
				while(g_pTextInfo[nTmp/2].BorderType != BORDER_PPH)
				{
					if(g_pTextInfo[nTmp/2].BorderType != BORDER_PW)
					{
							BounderQues[0][1] += g_pTextInfo[nTmp3/2].Len/ 2;
					}
					nTmp3 += g_pTextInfo[nTmp3/2].Len;
					if(g_pTextInfo[nTmp3/2].Len == 0)
						break;
				}
			}
			BounderQues[0][0] = DBACK * 16 + 1;
			// DFRONT:此PW边界前至PPH边界中前所有字长 
			i = 1;
			nLenSum = 0;
			nTmp6 = 0;
			nTmp3 = 0; //指向头节点
			while(1)
			{
				nTmp3 += g_pTextInfo[nTmp3/2].Len;
				if(nTmp3 == nTmp)  //不存在PPH
				{
					break;
				}
				if(g_pTextInfo[nTmp3/2].BorderType == BORDER_PPH)     //考虑中间的PPH
				{
					nTmp6 = nTmp3 + g_pTextInfo[nTmp3/2].Len;
				}
			}
			if(g_pTextInfo[nTmp6/2].Len==0)  //不存在
			{
				nTmp6 = 0;  //使用默认的
			}
			while(nTmp6 != nTmp)
			{
				if(g_pTextInfo[nTmp6/2].BorderType != BORDER_PW)
				{
						BounderQues[1][1] += g_pTextInfo[nTmp6/2].Len/ 2;
				}
				nTmp6 += g_pTextInfo[nTmp6/2].Len;
			}
			BounderQues[1][0] = DFRONT * 16 + 1;
			//lastBoundary: 此PW边界上一边界类型值    
			BounderQues[2][0] = LASTBOUNDARYTYPE * 16 + 1;
			if(g_pTextInfo[nTmp2/2].Len == 0)  //也就是pTmp是第一个PW节点
			{
				//句子开头默认含有一个PPH
				BounderQues[2][1] = 2;
			}
			else
			{
				BounderQues[2][1] = 1;
				nTmp7 = nTmp2;
				while(nTmp7!= nTmp)
				{
					if(g_pTextInfo[nTmp7/2].BorderType == BORDER_PPH)
					{
						BounderQues[2][1] = 2;
					}
					nTmp7 += g_pTextInfo[nTmp7/2].Len;
				}
			}
		    for(i = 0; i < 3; i++)  //后面三类
		    {

				nLines = PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY;
				nLineNum = ErFenSearchTwo_FromRAM(BounderQues[i], 0, pPPH_Table_Part2, 1, nLines, PPH_LINE_BYTE);  //WLEN-1
				if(nLineNum < nLines)
				{
					float fValue[2] = {0.0, 0.0};
//					fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + PPH_TABLE_LEN_PART_2 + AddBaseAddressForThree(i) + nLineNum * 2 * 4, 0); //定位
					fFrontSeek(g_hTTS->fResFrontMain,  g_Res.offset_ConvertRatioOfPPH + PPH_TABLE_LEN_PART_1 + PPH_TABLE_LEN_PART_2 + PPH_TABLE_START_DBACK * 2 * 4 + nLineNum * 2 * 4, 0); //定位
					fFrontRead(fValue, 8, 1, g_hTTS->fResFrontMain);
					NON_PPHValueSum += fValue[0];
					PPHValueSum += fValue[1];
				}
		   }
	      NON_PPHValueSum += *(ProbNonPPHSum + count);
		  PPHValueSum  += *(ProbPPHSum + count);
		  D_PPHValue = exp(PPHValueSum);
		  D_NON_PPHValue = exp(NON_PPHValueSum);
		  sum = D_PPHValue + D_NON_PPHValue;
		  D_PPHValue /= sum;
		  D_NON_PPHValue /= sum;
		  if(g_pTextInfo[(nTmp+g_pTextInfo[nTmp/2].Len)/2].Len != 0)
		  {
			//计算下一个PW的长度；
			nTmp8 = nTmp + g_pTextInfo[nTmp/2].Len;;
			nNextPWCount = 0;
			while( g_pTextInfo[nTmp8/2].TextType == TextCsItem_TextType_HANZI)
			{
				nNextPWCount += g_pTextInfo[nTmp8/2].Len/2 ;
				nTmp8 += g_pTextInfo[nTmp8/2].Len;
				if( g_pTextInfo[nTmp8/2].Len == 0)
				{
					break;
				}
			}			

			//6字以上的用低阀值，6字或以下的用高阀值
			if( nCurPPHCount <= PPH_BORDER_LEN_FOR_FAZHI )
			{
				fFaZhi = fFaZhiHigh;
			}
			else
			{
				fFaZhi = fFaZhiLow;
			}


			//判断是否需要添加PPH节点
			bNeedAddPPH = FALSE;
			if( (nCurPPHCount + nNextPWCount) > MAX_PPH_LEN_LIMIT)					//PPH的长度设定的PPH最大长度
			{
				bNeedAddPPH = TRUE;
			}
			else
			{
				if(  D_PPHValue == FindMax(D_PPHValue, D_NON_PPHValue, -10)  )		//<PPH>的概率最大
				{
					if( D_PPHValue < fFaZhi )										//<PPH>的概率 < 阀值
					{
						if( IsHaveTeZhengCi( nTmp4 )	)							//有特征词,参数：PW的前节点
						{
							bNeedAddPPH = TRUE;
						}
						else
						{
							bNeedAddPPH = FALSE;
						}
					}
					else															//<PPH>的概率 >= 阀值
					{
						bNeedAddPPH = TRUE;
					}
				}
				else																//<Non-PPH>的概率最大
				{
					bNeedAddPPH = FALSE;
				}				
			}			

			//添加PPH节点
			if( bNeedAddPPH == TRUE)			//加上“PPH”
			{
				g_pTextInfo[nTmp/2].BorderType = BORDER_PPH;
				nCurPPHCount = 0;
			}
		  }
		  nTmp2 = nTmp; //前一个边界节点
		  ++count;
		}
		nTmp4 = nTmp;   //前一个节点
		nTmp += g_pTextInfo[nTmp/2].Len;
	}//end while

#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPPH_Table_Part2, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE, "PPH后3类边界值：《韵律模块  》");
#else
	emHeap_Free(pPPH_Table_Part2, (PPH_TABLE_LINES_DBACK + PPH_TABLE_LINES_DFRONT + PPH_TABLE_LINES_LASTBOUNDARY) * PPH_LINE_BYTE);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPPH_Type, MAX_NUM * 9 * 4, "韵律pPPH_Type：《韵律模块  》");
#else
	emHeap_Free(pPPH_Type, MAX_NUM * 9 * 4);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(ProbPPHSum, MAX_NUM  * 4, "韵律ProbPPHSum：《韵律模块  》");
#else
	emHeap_Free(ProbPPHSum, MAX_NUM  * 4);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(ProbNonPPHSum, MAX_NUM * 4, "韵律ProbNonPPHSum：《韵律模块  》");
#else
	emHeap_Free(ProbNonPPHSum, MAX_NUM * 4);
#endif


#if DEBUG_LOG_SWITCH_HEAP
	emHeap_Free(pPPH_Table_Part1, PPH_TABLE_LEN_PART_1, "PPH第一部分：《韵律模块  》");		//为PPH的第一部分开辟内存空间
#else
	emHeap_Free(pPPH_Table_Part1, PPH_TABLE_LEN_PART_1);			//为PPH的第一部分开辟内存空间
#endif


}


//****************************************************************************************************
//  功能：求三个数的最大值  add by songkai
//  参数：三个待比较的值 
//****************************************************************************************************
float emCall FindMax(float PWValue, float LWValue, float PPHValue)
{
	float max = 0.0;

	LOG_StackAddr(__FUNCTION__);

	if(PWValue > max)
	{
		max = PWValue;
	}
	if(LWValue > max)
	{
		max = LWValue;
	}
	if(PPHValue > max)
	{
		max = PPHValue;
	}
	return max;
}

//****************************************************************************************************
//  功能：求最近三个节点的首地址  add by songkai
//  参数1: 前一个韵律词节点地址
//  参数2: 当前韵律词节点地址
// 返回值: 返回首地址
//****************************************************************************************************
emUInt8 emCall MostNearThreeNode(emUInt8 nPre, emUInt8 nCur)
{
	emUInt8 i = 0;
	emUInt8 nTmp = 0;

	LOG_StackAddr(__FUNCTION__);

	if(nPre == nCur)
	{
		return nPre;
	}
	nTmp = nPre;
	while(nPre != nCur)
	{
		i++;
		//moify 
		if( g_pTextInfo[nPre/2].BorderType == BORDER_PPH )  //遇到PPH重新设置开始位置为PPH所在位置
		{
			nTmp = nPre;
			i = 0;
		}//end modify 2010-10-13
		nPre += g_pTextInfo[nPre/2].Len;
	}
	if(i <= 3)
		return nTmp;
	else
	{
		while(i != 3)
		{
			nTmp += g_pTextInfo[nTmp/2].Len;
			i--;
		}
		return nTmp;
	}
}


/*
功能: 为每一种类型增加基础地址
*/
emInt32  AddBaseAddressForSix(emInt32  type)
{

	LOG_StackAddr(__FUNCTION__);

	switch(type)
	{
	case 0:
		return PPH_TABLE_START_POS_ADD_1 * PPH_PRO_VALUE * 2;
		break;
	case 1:
		return PPH_TABLE_START_POS_ADD_2 * PPH_PRO_VALUE * 2;
		break;
	case 2:
		return PPH_TABLE_START_POS_SUB_1 * PPH_PRO_VALUE * 2;
		break;
	case 3:
		return PPH_TABLE_START_POS_SUB_2 * PPH_PRO_VALUE * 2;
		break;
	case 4:
		return PPH_TABLE_START_WLEN_ADD_1 * PPH_PRO_VALUE * 2;
		break;
	case 5:
		return PPH_TABLE_START_WLEN_SUB_1 * PPH_PRO_VALUE * 2;
		break;
	default:
		return -1;
		break;
	}
}

emInt32  AddBaseAddressForThree(emInt32  type)
{

	LOG_StackAddr(__FUNCTION__);

	switch(type)
	{
	case 0:
		return PPH_TABLE_START_DBACK * PPH_PRO_VALUE * 2;
		break;
	case 1:
		return PPH_TABLE_START_DFRONT * PPH_PRO_VALUE * 2;
		break;
	case 2:
		return PPH_TABLE_START_LASTBOUNDARY * PPH_PRO_VALUE * 2;
		break;
	}
}

#endif	//	EM_SYS_SWITCH_RHYTHM_NEW
