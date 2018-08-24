#include "emPCH.h"

#include "Front_ToLab.h"


//*****************************************************************************************************
//�������ܣ� ����Lab��ģ��
//*****************************************************************************************************
void emCall	ToLab()
{
	//     /����|�г�/�� ��|�� �� ʫ/	
	//     /����|�г�/����|����ʫ/		
	//     ying2 hang2 hang2 zhang3 he4 le5  wu2  shou3  shi1
	//     5402	 1618  1618  1723  2388 2413 6426 2203  10393

#if DEBUG_LOG_SWITCH_LAB
	FILE *fp_Lab;
#endif

	emUInt8 i,nLen;

	LOG_StackAddr(__FUNCTION__);

	//�ж��Ƿ��ǡ�һ��һ�١�������� ���ǣ��������Ϣǿ���޸ĳɡ�һ��һ�١���
	if( g_hTTS->m_ControlSwitch.m_nSpeakStyle == emTTS_STYLE_WORD)
	{	
		nLen = emStrLenA(g_pText);
		for(i = 0; i< nLen/2; i++)
		{
			g_pTextInfo[i].Len = 2;
			g_pTextInfo[i].BorderType = BORDER_PPH;		//ǿ���޸ĳɣ�ÿ�����ֶ���һ�����ɶ���
		}	
	}

#if WL_REAR_DEAL_GONG4_GONG4
	//Ϊ���ƾ���gong4����  �ѵ�1�������ó�PPH�� ���ɵ�lab�����н�Ϊ�� sil pau ��... ��ԭ��Ϊ��sil ��...��
	g_pTextInfo[0].BorderType = BORDER_PPH ;
#endif


	//�����������ڵ�Lab�е�ƴ����Ϣ������0������20�У�
	GetPinYinInfoToLab();

	//Ϊ������ʱ���Lab�������ļ�
#if DEBUG_LOG_SWITCH_LAB
	fp_Lab=fopen("�ϳ�.lab", "w");
	fwrite(&g_pLabRTGlobal->m_LabRam, 1, g_pLabRTGlobal->m_MaxLabLine*LAB_CN_LINE_LEN, fp_Lab);
	fclose(fp_Lab);
#endif

	//�����������ڵ�Lab�е�λ����Ϣ������21������40�У��������������ڵ�Lab�е���Ŀ��Ϣ������41������62�У�
	GetPlaceAndCountInfoToLab();


#if WL_REAR_DEAL_GONG4_GONG4
	//Ϊ���ƾ���gong4���� �����ɵ�lab�����н�Ϊ�� sil pau ��...     ��ԭ�ɣ�sil ��... 
	//�������׼�pau�ټ�pau���ı��˾����ֵ�Lab��Ϣ
	for( i= 2; i< g_pLabRTGlobal->m_MaxLabLine; i++)
	{
		emMemCpy(g_pLabRTGlobal->m_LabRam[i-1],g_pLabRTGlobal->m_LabRam[i],LAB_CN_LINE_LEN);
	}
	g_pLabRTGlobal->m_LabRam[1][RLAB_L_SM] = INITIAL_CN_sil;	//�ָ�sil����ĸ
	g_pLabRTGlobal->m_MaxLabLine--;
	emMemSet(&g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine][RLAB_C_SM], 0 ,LAB_CN_LINE_LEN);
#endif



	//Ϊ������ʱ���Lab�������ļ�
#if DEBUG_LOG_SWITCH_LAB
	fp_Lab=fopen("�ϳ�.lab", "w+");		//sqb
	fwrite(&g_pLabRTGlobal->m_LabRam, 1, g_pLabRTGlobal->m_MaxLabLine*LAB_CN_LINE_LEN, fp_Lab);
	fclose(fp_Lab);
#endif


	HandleLabQst();		//�������̾������


	////�������ʶ�����ʾ�ĺ��ַ������⣨���������򣬵�ƴ������=0��
	////���磺 ʨ��������һ����λ������������
	//for( i= 1; i< (g_pLabRTGlobal->m_MaxLabLine-1); i++)
	//{
	//	if(    g_pLabRTGlobal->m_LabRam[i][0] == 0 
	//		&& g_pLabRTGlobal->m_LabRam[i][1] == 0 
	//		&& g_pLabRTGlobal->m_LabRam[i][2] == 0 )
	//	{
	//		//����ƴ����ֵ�ɣ�pau��ƴ��
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


	//�������ʶ�����ʾ�ĺ��ַ������⣨���������򣬵�ƴ������=0��, �ⲿ�ֺ��ֶ���������
	//���磺 ʨ��������һ����λ������������
	for( i= 1; i< (g_pLabRTGlobal->m_MaxLabLine-1); i++)
	{
		if(    g_pLabRTGlobal->m_LabRam[i][0] == 0 
			&& g_pLabRTGlobal->m_LabRam[i][1] == 0 
			&& g_pLabRTGlobal->m_LabRam[i][2] == 0 )
		{
			//��������в�����
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
	
#if WL_REAR_DEAL_LAB_LONG_SEN			//����������������������

	#define FORCE_MAX_SYL_IN_SEN	25				//����������⣺ǿ�Ƚ�g_pLabRTGlobal->m_LabRam�е�ֵ���¸�ֵ
	#define FORCE_MAX_SYL_IN_PPH	12				//����������⣺
	#define FORCE_MAX_PW_IN_SEN		12				//����������⣺
	#define FORCE_MAX_PW_IN_PPH		6				//����������⣺
	#define FORCE_MAX_PPH_IN_SEN	5				//����������⣨����һ��һ�ٵĳ������⣩

	//����������⣨��������ľ�����35�֣���emTTS�ı�������֧��50�֣������ʱ�����ҵ�������syl_in_sen��ǿ�и�С
	//ͬʱ������һ��һ�١��еĳ�������
	for(i=0;i<g_pLabRTGlobal->m_MaxLabLine;i++)
	{

#if WL_REAR_DEAL_LETTER	 && EM_8K_MODEL_PLAN	  //8Kģ�����ã���ĸ�Ĵ���  2012-10-12 hyl
		//������ĸ�ķ�������λ�ã�������ӹ�����ĸ�����õ�����
		//���磺��ӭ�ۿ������ϳ�ϵͳ����ʾ��ӭ�ۿ������ϳ�ϵͳ����ʾS12��
		//���磺����XXXX��ױƷר�����Ʒ��720Ԫ��
		//���磺0ϵ��40Ӣ��Һ������LA40B530P7R�ڽ�һ��ʱ���ڶ���ƵƵ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_C_SM] == INITIAL_CN_letter )	 //������ĸ
		{
			if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP] > 10)		//	IP�е�ǰSYL������λ��
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP]  = 10;			

			if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP] > 10)		//	IP�е�ǰSYL�ķ���λ��	
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP]  = 10;	
		}
#endif


		//���У����ڵ�λ�������
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_IP] = FORCE_MAX_SYL_IN_SEN - 1;		//IP�е�ǰSYL������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_SEN] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_SEN] = FORCE_MAX_SYL_IN_SEN - 1;		//SEN�е�ǰSYL������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_IP] = FORCE_MAX_SYL_IN_SEN - 1;		//IP�е�ǰSYL�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_SEN] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_SEN] = FORCE_MAX_SYL_IN_SEN - 1;		//SEN�е�ǰSYL�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_IP] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_IP] = FORCE_MAX_SYL_IN_SEN ;		//��ǰIP��SYL����Ŀ
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_SEN] > FORCE_MAX_SYL_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_SEN] = FORCE_MAX_SYL_IN_SEN ;		//��ǰSEN��SYL����Ŀ

		//���ɶ����У����ڵ�λ�������
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] > FORCE_MAX_SYL_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] = FORCE_MAX_SYL_IN_PPH - 1;		//PPH�е�ǰSYL������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] > FORCE_MAX_SYL_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] = FORCE_MAX_SYL_IN_PPH - 1;		//PPH�е�ǰSYL�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] > FORCE_MAX_SYL_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] = FORCE_MAX_SYL_IN_PPH ;		//��ǰPPH��SYL����Ŀ


		//���У�PW��λ�������
		//���²�ҪҲ�ܽ���������⣬��δ������������Ǽ���
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_IP] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_IP] = FORCE_MAX_PW_IN_SEN - 1;		//IP�е�ǰPW������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_SEN] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_SEN] = FORCE_MAX_PW_IN_SEN - 1;		//SEN�е�ǰPW������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_IP] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_IP] = FORCE_MAX_PW_IN_SEN - 1;		//IP�е�ǰPW�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_SEN] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_SEN] = FORCE_MAX_PW_IN_SEN - 1;		//SEN�е�ǰPW�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_IP] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_IP] = FORCE_MAX_PW_IN_SEN ;			//��ǰIP��PW����Ŀ
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_SEN] > FORCE_MAX_PW_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_SEN] = FORCE_MAX_PW_IN_SEN ;			//��ǰSEN��PW����Ŀ

		//���ɶ����У�PW��λ�������
		//���²�ҪҲ�ܽ���������⣬��δ������������Ǽ���		
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] > FORCE_MAX_PW_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] = FORCE_MAX_PW_IN_PPH - 1;		//PPH�е�ǰPW������λ��		
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] > FORCE_MAX_PW_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] = FORCE_MAX_PW_IN_PPH - 1;		//PPH�е�ǰPW�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] > FORCE_MAX_PW_IN_PPH )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] = FORCE_MAX_PW_IN_PPH ;			//��ǰPPH��PW����Ŀ

		//PPH��λ��������� 
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_IP] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_IP] = FORCE_MAX_PPH_IN_SEN - 1;		//IP�е�ǰPPH������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_SEN] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_FWD_POS_IN_SEN] = FORCE_MAX_PPH_IN_SEN - 1;		//SEN�е�ǰPPH������λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_IP] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_IP] = FORCE_MAX_PPH_IN_SEN - 1;		//IP�е�ǰPPH�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_SEN] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_BWD_POS_IN_SEN] = FORCE_MAX_PPH_IN_SEN - 1;		//SEN�е�ǰPPH�ķ���λ��
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_IP] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_IP] = FORCE_MAX_PPH_IN_SEN ;		//��ǰIP��PPH����Ŀ
		if( g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_SEN] > FORCE_MAX_PPH_IN_SEN )
				g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_C_SEN] = FORCE_MAX_PPH_IN_SEN ;		//��ǰSEN��PPH����Ŀ

	}
#endif


#if WL_REAR_DEAL_LAB_SHORT_SEN			//�����������3�ֶ̾���lab��ģ���4�־�  //����̾�����

	//���䣺ϲ������ϲ��������ϲ���񡣼���ȵ�񣿼�ȵ������ã�����ܺã�
	if( (g_pLabRTGlobal->m_MaxLabLine-2) == 3)		//3�־�
	{
		g_pLabRTGlobal->m_LabRam[3][RLAB_PW_FWD_POS_IN_PPH] = 1;	//PPH�е�ǰPW������λ��
		g_pLabRTGlobal->m_LabRam[3][RLAB_PW_FWD_POS_IN_IP] = 1;	//IP�е�ǰPW������λ��
		g_pLabRTGlobal->m_LabRam[3][RLAB_PW_FWD_POS_IN_SEN] = 1;	//SEN�е�ǰPW������λ��
	}
#endif
}

//*****************************************************************************************************
//�������ܣ� ������һ����������3�������ʣ�
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

	//��һ��Ϊ�׵�3�ִʣ��������Ĳ��ٱ���
	emUInt16  pTextWord3OfYi[20][2]={	{0xbcbe, 0xb6c8},		//һ����
										{0xb0d1, 0xcad6},		//һ����
										{0xc4ea, 0xbcb6},		//һ�꼶
										{0xc9e3, 0xcacf},		//һ����				����1�桫-8��
										{0xc2b7, 0xb8bd},		//һ·��				����λ����ï��ҵ�����һ·����
										{0xcfef, 0xb8bd},		//һ�︽				����λ�ں���һ�︽��
										{0xc1aa, 0xb6d3},		//һ����				���е�һ�������ҵ���������ʮ��ʦ�Ӵ�
										{0xb6d4, 0xd2bb},		//һ��һ
										{0xb5e3, 0xd6d3},		//һ����
										{0xb5c8, 0xbdb1},		//һ�Ƚ�				һ�Ƚ��������һ�Ƚ�����һ�ȵ����žͳ��˽�ȥ��
										{0xb2e3, 0x0000},		//һ��+��β��ȡ�ɷ�ʽ��	��ס��һ�㡣�̳�B1�㡣�����ճ������е�ÿһ���档								
										{0xc3c5, 0x0000},		//һ��+��β��ȡ�ɷ�ʽ��	��ס��һ¥һ�š���һ�Ŷ����ҵ��ˡ�һ���Ȼ��ڡ�Ҳ��һ�������ѧ�ʡ�
										{0xc6da, 0x0000},		//һ��+��β��ȡ�ɷ�ʽ��	����6����һ��
										{0xcfdf, 0xb9a4},		//һ�߹�				������һ�߹��ˡ�������һ�߹�����
										{0xcfdf, 0xb8e8},		//һ�߸�				Ѹ�ٴں�Ϊһ�߸��֡�Ѹ�ٴں�Ϊһ�߸��ǡ�										
										{0xc2b7, 0xb7bd},		//һ·��				ǰ��2���������������һ·����
										{0x0000, 0x0000}};		//--------������

	LOG_StackAddr(__FUNCTION__);	



	//��һ��ǰ�����º��֣��������Ĳ��ٱ���
	i=0;
	pTextBeforeYi[i++] = 0xb5da;	//��
	pTextBeforeYi[i++] = 0xb3f5;	//��		���³�һ�ų�Ԫ�ա�
	pTextBeforeYi[i++] = 0xcaae;	//ʮ
	pTextBeforeYi[i++] = 0xcdf2;	//��
    pTextBeforeYi[i++] = 0xb5a5;	//��
    pTextBeforeYi[i++] = 0xcdb3;	//ͳ
    pTextBeforeYi[i++] = 0xcea8;	//Ψ
	pTextBeforeYi[i++] = 0xd6dc;	//��
	pTextBeforeYi[i++] = 0xbdec;	//��		�ı�˽�һ�λ�����ڹ�����Ҫì�ܵ���ȷ����
	pTextBeforeYi[i++] = 0xbaec;	//��		�����Ѻ�һ��������ί�ݶ�����Ϊ�й���ũ����¸�֧��
	pTextBeforeYi[i++] = 0xc6da;	//��
	pTextBeforeYi[i++] = 0xc6e4;	//��
	pTextBeforeYi[i++] = 0xd6ae;	//֮		ռȫ����1��2�˿ڵĸ�Ů  ��֮һ���Ӵ�ĵ��¼�ҩ���������ݻ�
	pTextBeforeYi[i++] = 0xb5e3;	//��		6352.1Ԫ
	pTextBeforeYi[i++] = 0xcdac;	//ͬ		ͬһ�˻���ͬһ�����˿��ƵĶ���˻�
	pTextBeforeYi[i++] = 0x0000;	//----������
	if( i > MAX_TextBeforeYi)
	{
		return;							//����
	}
       
       

	//��һ��ǰ�������������֣��������Ĳ��ٱ���
	i = 0;
	pTextShuZi[i++] = 0xd2bb;	//һ
	pTextShuZi[i++] = 0xb6fe;	//��
    pTextShuZi[i++] = 0xc8fd;	//��
    pTextShuZi[i++] = 0xcbc4;	//��
    pTextShuZi[i++] = 0xcee5;	//��
    pTextShuZi[i++] = 0xc1f9;	//��
    pTextShuZi[i++] = 0xc6df;	//��
    pTextShuZi[i++] = 0xb0cb;	//��
    pTextShuZi[i++] = 0xbec5;	//��
    pTextShuZi[i++] = 0xc1e3;	//��
	pTextShuZi[i++] = 0x0000;	//----������
	


	//��һ��������º��֣��������Ĳ��ٱ���
	i = 0;
    pTextAfterYi[i++] = 0xd4c2;	//��		
    pTextAfterYi[i++] = 0xbac5;	//��		
    pTextAfterYi[i++] = 0xd6c1;	//��		
	pTextAfterYi[i++] = 0xb6d3;	//��		һ�ӣ���һ��, 	
	pTextAfterYi[i++] = 0xb7a3;	//��		��һ��ʮ����һ��ʮ
	pTextAfterYi[i++] = 0xcaae;	//ʮ	
    pTextAfterYi[i++] = 0xc2a5;	//¥    
    pTextAfterYi[i++] = 0xbcb6;	//��        
    pTextAfterYi[i++] = 0xb2bb;	//��		˵һ��������һ�����˾Ϳޡ�
	pTextAfterYi[i++] = 0xcac7;	//��
	pTextAfterYi[i++] = 0xc6fb;	//��
	pTextAfterYi[i++] = 0xbdd6;	//��		
	pTextAfterYi[i++] = 0xbafa;	//��		ǰ��1.5����ǰ���ϸ߼ܵ������б�����һ��ͬ����
	pTextAfterYi[i++] = 0xc9f3;	//��		�����н�������Ժ����һ���о���		
	pTextAfterYi[i++] = 0xc1ac;	//��		һ���������ˡ���һ�����˼�������һ������
	pTextAfterYi[i++] = 0xc1f7;	//��		һ���ġ�����һ�������ּҡ�ֻҪһ�������͸��硣
	pTextAfterYi[i++] = 0x0000;	//----������
	//��Ҫ pTextAfterYi[i++] = 0xbdda;	//��		��һ�ڣ���һ�ڡ�һ�ڡ�һ��ź��	
	//��Ҫ pTextAfterYi[i++] = 0xc3c5;	//��		��ס��һ¥һ�š���һ�Ŷ����ҵ��ˡ�һ���Ȼ��ڡ�Ҳ��һ�������ѧ�ʡ�
	//��Ҫ pTextAfterYi[i++] = 0xb5c8;	//��		һ�Ƚ��������һ�Ƚ�����һ�ȵ����žͳ��˽�ȥ��
	//��Ҫ pTextAfterYi[i++] = 0xcfdf;	//��		��һ������С��Ѹ�ٴں�Ϊһ�߸��֡�������һ�߹��ˡ�����һ�߶��ŵľ��档����Ⱥ��һ��һ�ߡ�ֻҪ��һ��ϣ����
	//��Ҫ pTextAfterYi[i++] = 0xb2e3;	//��		��ס��һ�㡣�̳�B1�㡣һ������һ��������ʹ��Ǭ�걻��Ⱦ����һ������ص�ɫ�ʡ������ճ������е�ÿһ���档
	//��Ҫ pTextAfterYi[i++] = 0xc2b7;	//·		ǰ��1.5���������������һ·����		����һ·�񱼡�����һ·��˵����ͣ��ף��һ·˳�硣
	if( i > MAX_TextAfterYi)
	{
		return;						//����
	}


	g_nCurTextByteIndex = 0;
	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		count = g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
		nPos = g_pTextInfo[g_nCurTextByteIndex/2].Pos;

		for( i = 0; i < (count); i++ )   
		{
			if( ( (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]) & 0x8000 ) != 0x8000)	//����ǿ��ƴ�����ܱ��� hyl 20111227
			{

				//�������򣺡����������ı�����
				//  ����ԭ��PW������3���Զ��������������1���֣�������ȫ�����2���� ����������
				//	���磺  ���ɴ�	        �﷨��        �������
				//
				//			С�ϻ�        = С+�ϻ�	      = xiao3 lao2 hu3
				//			С����        = С+����	      = xiao2 lao3 ren2
				//			Ҳ+����       = Ҳ+����	      = ye3 ke2 yi3
				//			��+����       = ��+��+��	  = you3 yang2 gou3			�������������˵
				//			չ����        = չ����	      = zhan2 lan2 guan3	
				//			���쵼��      = ��+�쵼+��    = suo3 ling2 dao3 de5	
				//			������        = ��+��+��      = liang2 bai2 mi3
				//			�ȡ�����      = �ȡ�+����     = deng3 jiu3 ren2
				//			��ͳ+����     = ��ͳ+����     = chuan2 tong2 mei3 de2	�������ڷ񶨼�����Ϊ�ȴ�ͳ����
				//			����+ȡ��     = ����+ȡ��     = shen2 mei2 qu3 xiang4   ������ʶ˵���׾������ҵ�����ȡ��Ҫ������������ʷ�Ĵ��ڵ�����

				if( count== 1 )  //1�����ɴ�
				{
					//������1�����ɴʣ�����PPHβ����3�����Һ�����3����������ֲ���3��
					//�򣺱�1�����ɴʱ��2��
					if(	   g_pTextInfo[g_nCurTextByteIndex/2+1].BorderType != BORDER_PPH
						&& (( g_pTextPinYinCode[g_nCurTextByteIndex/2] & (emByte) 0x7) == 3) 
						&& (( g_pTextPinYinCode[g_nCurTextByteIndex/2+1] & (emByte) 0x7) == 3) 
						&& (  (( g_pTextPinYinCode[g_nCurTextByteIndex/2+2] & (emByte) 0x7) != 3)
						))
						    //  || g_pTextInfo[g_nCurTextByteIndex/2+2].Len == 0  ))	//�����﷨�ʵĿ�ʼ
					{
						g_pTextPinYinCode[g_nCurTextByteIndex/2] -= 1;	//���2��
					}
				}
				else		//2�ּ��������ɴ�
				{
					if( i!=(count-1))		//�������ɴ�β
					{
						if( (( g_pTextPinYinCode[g_nCurTextByteIndex/2+i] & (emByte) 0x7) == 3) && (( g_pTextPinYinCode[g_nCurTextByteIndex/2+i+1] & (emByte) 0x7) == 3)  )
						{
							//������ǰ���ɴ�Ϊ3�ִʼ����ϣ���Ϊ��1�ִ�+�����ʡ��ϲ��������ɴʣ�����ǰ3���ֶ�Ϊ3��
							//�򣺵�1�ֲ�������
							//���ࣺ������ԭ�����
							if( count >= 3 && i == 0
								&& g_pTextInfo[g_nCurTextByteIndex/2 + 1].Len > 2
								&& (( g_pTextPinYinCode[g_nCurTextByteIndex/2+i+2] & (emByte) 0x7) == 3) )	
							{
								g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 0;	//������
							}
							else							
								g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 1;	//���2��
						}
					}					
					else					//�����ɴ�β
					{
						emUInt8 nNext1,nNextCount;
						nNext1 = g_nCurTextByteIndex+g_pTextInfo[g_nCurTextByteIndex/2].Len;
						nNextCount = g_pTextInfo[nNext1/2].Len/2;

						if(    g_pTextInfo[g_nCurTextByteIndex/2+i+1].BorderType != BORDER_PPH		//����β����PPHβ
							&& (g_pTextPinYinCode[g_nCurTextByteIndex/2+i-1] & (emByte) 0x7) != 3	//��β��ǰ1�ֲ���3��
							&& (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]   & (emByte) 0x7) == 3	//��β��3��						
							&& (g_pTextPinYinCode[nNext1/2]   & (emByte) 0x7) == 3					//��1���ɴʵ���ͷ��3��
							&& (g_pTextPinYinCode[nNext1/2+1] & (emByte) 0x7) != 3)					//��1���ɴʵĵ�2�ֲ���3��
						{
							g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 1;	//���2��
						}
					}
				}



				//�������򣺡���(0xb2bb)�����Զ�������         ��ֻҪ���������滹�к����ҷ�4�������������2����
				if( g_pText[g_nCurTextByteIndex+2*i] == 0xb2 && g_pText[g_nCurTextByteIndex+2*i+1] == 0xbb && ( ( g_pTextPinYinCode[g_nCurTextByteIndex/2+i] & (emByte) 0x7 ) == 4 ) && ( ( g_pTextPinYinCode[g_nCurTextByteIndex/2+i+1] & (emByte) 0x7 ) == 4 ) )
				{
					g_pTextPinYinCode[g_nCurTextByteIndex/2+i] -= 2;
				}
			}
		}		

		for( i = 0; i < count; i++ )   
		{
			if( ( (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]) & 0x8000 ) != 0x8000)	//����ǿ��ƴ�����ܱ��� hyl 20111227
			{
				//�������򣺡�һ(0xd2bb)�����Զ�������       �����1���־���������   
				if(  g_pText[g_nCurTextByteIndex+2*i] == 0xd2 && g_pText[g_nCurTextByteIndex+2*i+1] == 0xbb )
				{
					//�����ڵ�ġ�һ����ȫ�ֱ��������Ǵ򿪵ģ����п��ܱ��� (Ŀǰֻ�ڡ����ְ���������У� �ǹرյ�)
					if(  TRUE )	
					{
						//�������һ����ǰһ����
						nBeforeZi = 0;
						nBeforeBeforeZi = 0;
						if( (g_nCurTextByteIndex/2+i) > 0 )
						{	//���ڵ��С�һ����ǰһ����						
							nBeforeZi =  (emUInt16)((g_pText[g_nCurTextByteIndex+2*i-2])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i-1]) ;
							nBeforeBeforeZi = (emUInt16)((g_pText[g_nCurTextByteIndex+2*i-4])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i-3]) ;
						}

						//�������һ���ĺ�һ����
						nNextZi = 0;
						nNextNextZi = 0;
						if( (g_nCurTextByteIndex/2+i) < (g_nLastTextByteIndex/2-1) )
						{
							nNextZi =  (emUInt16)((g_pText[g_nCurTextByteIndex+2*i+2])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i+3]) ;					
							nNextNextZi = (emUInt16)((g_pText[g_nCurTextByteIndex+2*i+4])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i+5]) ;					
						}

						//�������һ��������
						nCurTone = 0;
						nCurTone =  (g_pTextPinYinCode[g_nCurTextByteIndex/2+i] & (emByte) 0x7 ) ;

						//�������һ���ĺ�1���ֵ�����
						nNextTone =  0 ;
						if( (g_nCurTextByteIndex/2+i) < (g_nLastTextByteIndex/2-1) )
						{
							nNextTone =  (g_pTextPinYinCode[g_nCurTextByteIndex/2+i+1] & (emByte) 0x7 ) ;
						}

						//Ϊǿ��ƴ����������Դ�ڶ����ʵ䣬��Դ[=]�������ܱ���
						if( ( (g_pTextPinYinCode[g_nCurTextByteIndex/2+i]) & 0x8000 )== 0x8000)		
							continue;

						//ǰ������ĸ a,b,c......z�����ܱ���
						if( nBeforeZi >= 0xa3e1 && nBeforeZi <= 0xa3fa)		//hyl 2012-03-31
							continue;



						//��һ��Ϊ�׵�ĳЩ3�ִʣ����ܱ���
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


						//��һ�������������ʣ������ǡ��������򡱡��㡱������������ʣ��򲻱���
						//���磺ֱ�ӻظ�1��2���ɡ��������ǰ���ȳ�һ�������ӡ�
						if(    g_pTextInfo[g_nCurTextByteIndex/2+i].Pos == POS_CODE_m
							&& (nNextZi == 0xb5bd || nNextZi == 0xbbf2 || nNextZi == 0xb5e3 )
							&& g_pTextInfo[g_nCurTextByteIndex/2+i+2].Pos == POS_CODE_m )
						{
							continue;
						}


						//��һ�������ɶ���β�����ܱ���
						g_nNextTextByteIndex = g_nCurTextByteIndex + g_pTextInfo[g_nCurTextByteIndex/2].Len;
						if(    i == (count-1) 
							&& (    g_pTextInfo[g_nNextTextByteIndex/2].BorderType == BORDER_PPH
							     || g_pTextInfo[g_nNextTextByteIndex/2].Len == 0)	)
						{
							continue;
						}

						//��һ���������������ܱ���
						if( nNextTone == 5 )
							continue;


						//����һ��һ���� ���ܱ���  (ֻ���ڡ���һ��һ����һ�������ǰ���£���������Ź���)
						if( nBeforeZi == 0xc2f2  && nNextZi == 0xcbcd )
						{
							i += 2;
							continue;
						}

						//�����ֻ�ǰ���� "��"�� "��"  ��һ�������£����ȡ������ˡ�ǰ��ӵĶ������֣������ܱ���			added by hyl 2012-03-31
						//���磺 ����ھ���Ӱ��2��1��ʤ��������ھ���Ӱ��1��2��ʤ����1��1��1��
						nNeedJustZi = 0;
						if( nNextZi == 0xb1c8 || nNextZi == 0xb3cb )
							nNeedJustZi = nNextNextZi;
						if( nBeforeZi == 0xb1c8 || nBeforeZi == 0xb3cb)
							nNeedJustZi = nBeforeBeforeZi;
						if( nNeedJustZi != 0)
						{
							//pTextShuZi[]�� ������
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
							     

						//��һ��ǰ��ĳЩ���֣���pTextBeforeYi[]��pTextShuZi[]�ĳ�ʼ��ֵ����ȫ���������Ĳ�����										
						if( nBeforeZi != 0)
						{
							//pTextBeforeYi[]���������Ĳ�����
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

							//pTextShuZi[]���������Ĳ�����
							j = 0;			
							while( pTextShuZi[j] != 0x0000)		
							{
								if( nBeforeZi == pTextShuZi[j] && nBeforeZi != 0xd2bb)		//����һ���ų�����
									break;
								else
									j++;
							}
							if(  pTextShuZi[j] != 0x0000)
								continue;
						}


						//��һ����ǰ��Ϊ��һ������ǰǰ��Ϊ��Ψ �� ͳ �� �ڡ������������ǰǰ�ֲ�����Щ���򲻱���
						//Ӧ������  ���磺 Ψһһ������һһ����ͳһһ������һһ��������һһ����
						//��Ӧ���������磺 һһ��������һһ���塣23.11��  23.112��
						if( nBeforeZi ==  0xd2bb)	//ǰ��Ϊ��һ��
						{
							if(    nBeforeBeforeZi != 0xcea8	//Ψ
								&& nBeforeBeforeZi != 0xcdf2	//��
								&& nBeforeBeforeZi != 0xcdb3	//ͳ
								&& nBeforeBeforeZi != 0xd6dc	//��
								&& nBeforeBeforeZi != 0xc6da)	//��
							{
								continue;
							}

						}


						//��һ�����ĳЩ���֣���pTextAfterYi[]�ĳ�ʼ��ֵ����pTextShuZi[]�ĳ�ʼ��ֵ����ȫ���������Ĳ�����
						//��һ����������꣬�գ��㣩���������жϣ�
						//				�꣺ ����һ��ǰ�������֣�0��9�����򲻱������������
						//				�գ� ����һ��ǰ�������¡���		 �򲻱������������
						//				�㣺 �����㡱�󻹸����֣�0��9�����ӡ����������򲻱������������											
						if( nNextZi != 0)
						{
							

							//pTextAfterYi[]���������Ĳ�����
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

							//pTextShuZi[]���������Ĳ�����
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

							//��һ�������ǣ����ꡱ
							if( nNextZi == 0xc4ea )
							{
								//��һ��֮ǰ���������֣��򲻱���
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

							//��һ�������ǣ����ա�
							if( nNextZi == 0xc8d5 )
							{
								if( nBeforeZi == 0xd4c2 )		//ǰһ���ǡ��¡����򲻱���
									continue;
							}

							//��һ�������ǣ����㡱
							if( nNextZi == 0xb5e3 )
							{
								//�������һ���ĺ����
								nNextNextZi = 0;
								if( (g_nCurTextByteIndex/2+i+1) < (g_nLastTextByteIndex/2-1) )
								{
									nNextZi =  (emUInt16)((g_pText[g_nCurTextByteIndex+2*i+4])<<8)  +  (emUInt16)(g_pText[g_nCurTextByteIndex+2*i+5]) ;					
								}							

								if( nNextNextZi != 0)
								{
									//���㡱֮������ǡ��ӡ����򲻱���
									if( nNextNextZi == 0xd6d3 )		
										continue;

									//���㡱֮������ǡ��������򲻱���
									if( nNextNextZi == 0xd5fb )		
										continue;

									//���㡱֮����������֣��򲻱���
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


						//��һ��ֻ�ñ����ˡ����ݺ�1�����ֵ�������������  ���4�������2�������1,2,3�������4��						
						if( nCurTone == 1 && nNextTone == 4 )
						{  	
							g_pTextPinYinCode[g_nCurTextByteIndex/2+i] += 1;		//��һ������ĺ�����4������һ�����2��
						}						
						if( nCurTone == 1 && ( nNextTone==1 || nNextTone==2 || nNextTone==3 ) )
						{  	
							g_pTextPinYinCode[g_nCurTextByteIndex/2+i] += 3;		//��һ������ĺ�����1��2 ��3 ����һ�����4��
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
//�������ܣ� ��ȡ��һ���ɴ��к��ֵ�����
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
//�������ܣ� ��ȡ��һ���ɶ����к��ֵ�����
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
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//�����ڵ� ���� �߽�ڵ㣨PPH��
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
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//�����ڵ� ���� �߽�ڵ㣨PPH��
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
//�������ܣ� ��ȡ��һ���ɶ��������ɴʵ�����
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
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//�����ڵ� ���� �߽�ڵ㣨PPH��
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
			if( g_pTextInfo[nCur/2].BorderType != BORDER_PPH )		//�����ڵ� ���� �߽�ڵ㣨PPH��
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
//�������ܣ� �����������ڵ�Lab�е�λ����Ϣ������21������40�У�    ����λ�ô�0��ʼ����
//           �����������ڵ�Lab�е���Ŀ��Ϣ������41������62�У�
//
// sil��pau���ڲ���ʹλ�ü������ӣ�λ����Ϣֻ�����ֽڵ��й�
// sil��pau���ڲ���ʹ��Ŀ�������ӣ���Ŀ��Ϣֻ�����ֽڵ��й�
//*****************************************************************************************************
void emCall GetPlaceAndCountInfoToLab()
{
	emUInt8 nMaxOfHanZiInSEN=0;					//�����еĺ�������
	emUInt8 nMaxOfPWInSEN=0;					//�����е����ɴ�����
	emUInt8 nMaxOfPPHInSEN=0;					//�����е����ɶ�������

	emUInt8 nCountOfHanZiInCurPPH = 0;			//��ǰ���ɶ����еĺ��ּ���
	emUInt8 nCountOfPWInCurPPH = 0;				//��ǰ���ɶ����е����ɴʼ���

	emUInt8 nCountOfHanZiInSEN=0;				//�����еĺ��ּ���
	emUInt8 nCountOfPWInSEN=0;					//�����е����ɴʼ���
	emUInt8 nCountOfPPHInSEN=0;					//�����е����ɶ������
	emUInt8 nCountOfSylInSen = 0;				//�����е����ڼ���

	emUInt8 nStartCountOfSylInPPH = 0;			//�����ɶ������ʼ������
	emUInt8 nCurCountOfSylInPPH = 0;			//�����ɶ���ĵ�ǰ������

	emUInt8 nMaxOfHanZiInPrevPW = 0;			//ǰһ���ɴ��к��ֵ�����
	emUInt8 nMaxOfHanZiInPrevPPH = 0;			//ǰһ���ɶ����к��ֵ�����
	emUInt8 nMaxOfPWInPrevPPH = 0;				//ǰһ���ɶ��������ɴʵ�����

	emUInt8 nCountOfHanZiInPrevPPH = 0;			//ǰһ���ɶ����к��ֵļ���
	emUInt8 nCountOfPWInPrevPPH = 0;			//ǰһ���ɶ��������ɴʵļ���

	emUInt8 nMaxOfHanZiInNextPW = 0;			//��һ���ɴ��к��ֵ�����
	emUInt8 nMaxOfHanZiInNextPPH = 0;			//��һ���ɶ����к��ֵ�����
	emUInt8 nMaxOfPWInNextPPH = 0;				//��һ���ɶ��������ɴʵ�����

	emUInt8 i,nHanZiCount;

	LOG_StackAddr(__FUNCTION__);

	//����ñ����е�������Ϣ��ͨ��ָ�뷵�ز����� �����еĺ�������, �����е����ɴ����� , �����е����ɶ�������
	g_pLabRTGlobal->m_MaxLabLine = GetSenInfo(&nMaxOfHanZiInSEN, &nMaxOfPWInSEN, &nMaxOfPPHInSEN);		//����HMM�ϳ�ǰ�����lab��������������

	nCountOfSylInSen = 0;

	//���ȸ�ֵ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�
	for( i=0;i<g_pLabRTGlobal->m_MaxLabLine;i++)
	{
		//��ֵλ����Ϣ
		g_pLabRTGlobal->m_LabRam[i][RLAB_IP_FWD_POS_IN_SEN] = 0;				//��ֵ��30�У�SEN�е�ǰIP������λ�ã�
		g_pLabRTGlobal->m_LabRam[i][RLAB_IP_BWD_POS_IN_SEN] = 0;				//��ֵ��40�У�SEN�е�ǰIP�ķ���λ�ã�

		//��ֵ��Ŀ��Ϣ
		g_pLabRTGlobal->m_LabRam[i][RLAB_IP_NUM_IN_C_SEN] = 1;				//��ֵ��50�У���ǰSEN��IP����Ŀ��
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_L_IP] = 0;				//��ֵ��53�У�ǰһIP��SYL����Ŀ��
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_L_IP] = 0;				//��ֵ��55�У�ǰһIP��PW����Ŀ��	
		g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_L_IP] = 0;				//��ֵ��56�У�ǰһIP��PP����Ŀ��	
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_R_IP] = 0;				//��ֵ��59�У���һIP��SYL����Ŀ��
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_R_IP] = 0;				//��ֵ��61�У���һIP��PW����Ŀ��	
		g_pLabRTGlobal->m_LabRam[i][RLAB_PPH_NUM_IN_R_IP] = 0;				//��ֵ��62�У���һIP��PP����Ŀ��
	}

	//��ֵ����sil �;�βsil��λ����Ϣ ������sil��βsil�� ȫ��������λ�ö���0xff��
	for( i=21;i<41;i++)
	{
		g_pLabRTGlobal->m_LabRam[0][i] = SilPau_PlaceCode;	
		g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][i] = SilPau_PlaceCode;
	}

	//��ֵ����sil �;�βsil����Ŀ��Ϣ  
	for( i=41;i<LAB_CN_LINE_LEN;i++)
	{
		g_pLabRTGlobal->m_LabRam[0][i] = 0;		//��ȫ����ʼ��Ϊ0 ��ʵ���� ��57����62�в���0�� ���ں������¸�ֵ
		g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][i] =       0;		//��ȫ����ʼ��Ϊ0 ��ʵ���� ��51����56�в���0�� ���ں������¸�ֵ
	}

	//������ֵ������sil �;�βsil�� ��55,56,61��62,53,59��
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

	//������ֵ������sil  ��57,58,60��
	g_pLabRTGlobal->m_LabRam[0][RLAB_SYL_NUM_IN_R_PW] = GetMaxHanZiInNextPW(g_nCurTextByteIndex,emTrue);				//57�У�����һPW��SYL����Ŀ��
	g_pLabRTGlobal->m_LabRam[0][RLAB_SYL_NUM_IN_R_PPH] = GetMaxHanZiInNextPPH(g_nCurTextByteIndex,emTrue);				//58�У�����һPP��SYL����Ŀ��
	g_pLabRTGlobal->m_LabRam[0][RLAB_PW_NUM_IN_R_PPH] = GetMaxPWInNextPPH(g_nCurTextByteIndex,emTrue);				//60�У�����һPP��PW����Ŀ��

	nMaxOfHanZiInNextPPH = GetMaxHanZiInNextPPH(g_nCurTextByteIndex,emFalse);
	nMaxOfPWInNextPPH = GetMaxPWInNextPPH(g_nCurTextByteIndex,emFalse);

	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		if( g_pTextInfo[g_nCurTextByteIndex/2].BorderType == BORDER_PPH )		//�����ڵ� �� �߽�ڵ㣨PPH��
		{
			//��ֵpau��λ����Ϣ ��pau �� ȫ��������λ�ö���0xff����
			for( i=21;i<41;i++)
			{
				g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][i] = SilPau_PlaceCode;	
			}

			//��ֵpau����Ŀ��Ϣ  (��51������62�л��ڴ���������ʱ��ֵ)
			for( i=41;i<51;i++)
			{
				g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][i] = 0;		
			}

			for( i=nStartCountOfSylInPPH; i<nCurCountOfSylInPPH; i++)
			{	
				//��ֵλ����Ϣ
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] = i - nStartCountOfSylInPPH;								//��ֵPPH֮ǰ�� ��22�У�PP�е�ǰSYL������λ�ã�
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] = nCurCountOfSylInPPH - i - 1;							//��ֵPPH֮ǰ�� ��32�У�PP�е�ǰSYL�ķ���λ�ã�  
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] = nCountOfPWInCurPPH - g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] - 1;					//��ֵPPH֮ǰ�� ��35�У�PP�е�ǰPW�ķ���λ�ã�  

				//��ֵ��Ŀ��Ϣ
				g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] = nCurCountOfSylInPPH - nStartCountOfSylInPPH;			//��ֵPPH֮ǰ�� ��42�У���ǰPP��SYL����Ŀ��
				g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] = nCountOfPWInCurPPH;										//��ֵPPH֮ǰ�� ��45�У���ǰPP��PW����Ŀ��
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

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_L_PW] = nMaxOfHanZiInPrevPW;						//��ֵ��ǰpau�� ��51�У�ǰһPW��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_L_PPH] = nMaxOfHanZiInPrevPPH;					//��ֵ��ǰpau�� ��52�У�ǰһPP��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_PW_NUM_IN_L_PPH] = nMaxOfPWInPrevPPH;						//��ֵ��ǰpau�� ��54�У�ǰһPP��PW����Ŀ)

			nMaxOfHanZiInNextPW = GetMaxHanZiInNextPW(g_nCurTextByteIndex,emFalse);
			nMaxOfHanZiInNextPPH = GetMaxHanZiInNextPPH(g_nCurTextByteIndex,emFalse);
			nMaxOfPWInNextPPH = GetMaxPWInNextPPH(g_nCurTextByteIndex,emFalse);

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_R_PW] = nMaxOfHanZiInNextPW;						//��ֵ��ǰpau�� ��57�У���һPW��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_SYL_NUM_IN_R_PPH] = nMaxOfHanZiInNextPPH;					//��ֵ��ǰpau�� ��58�У���һPP��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_PW_NUM_IN_R_PPH] = nMaxOfPWInNextPPH;						//��ֵ��ǰpau�� ��60�У���һPP��PW����Ŀ)

			nCountOfSylInSen++;
		}
		
		nHanZiCount = g_pTextInfo[g_nCurTextByteIndex/2].Len/2;
		nCurCountOfSylInPPH += nHanZiCount;
		nMaxOfHanZiInNextPW = GetMaxHanZiInNextPW(g_nCurTextByteIndex,emFalse);
		
		for(i = 0; i< nHanZiCount;i++)
		{
			//��ֵλ����Ϣ
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_PW] = i;										//��ֵ��ǰ���ɴʵ� ��21�У�PW�е�ǰSYL������λ�ã�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_PW] = nHanZiCount - i - 1;					//��ֵ��ǰ���ɴʵ� ��31�У�PW�е�ǰSYL�ķ���λ�ã�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_IP] = nCountOfHanZiInSEN + i;				//��ֵ��ǰ���ɴʵ� ��23�У�IP�е�ǰSYL������λ�ã�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_IP] = nMaxOfHanZiInSEN-nCountOfHanZiInSEN-i-1;//��ֵ��ǰ���ɴʵ� ��33�У�IP�е�ǰSYL�ķ���λ�ã�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_SEN] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_FWD_POS_IN_IP];		//��ֵ��ǰ���ɴʵ� ��24�У�SEN�е�ǰSYL������λ�ã�=23�� ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_SEN] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_BWD_POS_IN_IP];		//��ֵ��ǰ���ɴʵ� ��34�У�SEN�е�ǰSYL�ķ���λ�ã�=33�� ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_PPH] = nCountOfPWInCurPPH;					//��ֵ��ǰ���ɴʵ� ��25�У�PP�е�ǰPW������λ�ã�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_SEN] = nCountOfPWInSEN;						//��ֵ��ǰ���ɴʵ� ��27�У�SEN�е�ǰPW������λ�ã�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_BWD_POS_IN_SEN] = nMaxOfPWInSEN-nCountOfPWInSEN-1;		//��ֵ��ǰ���ɴʵ� ��37�У�SEN�е�ǰPW�ķ���λ�ã�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_FWD_POS_IN_SEN];		//��ֵ��ǰ���ɴʵ� ��26�У�IP�е�ǰPW�е�����λ�ã�=27�� ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_BWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_BWD_POS_IN_SEN];		//��ֵ��ǰ���ɴʵ� ��36�У�IP�е�ǰPW�еķ���λ�ã�=37�� ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_FWD_POS_IN_SEN] = nCountOfPPHInSEN;						//��ֵ��ǰ���ɴʵ� ��29�У�SEN�е�ǰPP������λ�ã�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_BWD_POS_IN_SEN] = nMaxOfPPHInSEN-nCountOfPPHInSEN;		//��ֵ��ǰ���ɴʵ� ��39�У�SEN�е�ǰPP�ķ���λ�ã�

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_FWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_FWD_POS_IN_SEN];		//��ֵ��ǰ���ɴʵ� ��28�У�IP�е�ǰPP������λ�ã�=29�� ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_BWD_POS_IN_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_BWD_POS_IN_SEN];		//��ֵ��ǰ���ɴʵ� ��38�У�IP�е�ǰPP�ķ���λ�ã�=39�� ��IP��ͬ��SEN����SEN��ֵ����IP���ɣ�

			
			//��ֵ��Ŀ��Ϣ
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_PW] = nHanZiCount;							//��ֵ��ǰ���ɴʵ� ��41�У���ǰPW��SYL����Ŀ��
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_SEN] = nMaxOfHanZiInSEN;						//��ֵ��ǰ���ɴʵ� ��44�У���ǰSEN��SYL����Ŀ��
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_C_SEN];		//��ֵ��ǰ���ɴʵ� ��43�У���ǰIP��SYL����Ŀ��=44��
			
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_C_SEN] = nMaxOfPWInSEN;							//��ֵ��ǰ���ɴʵ� ��47�У���ǰSEN��PW����Ŀ��
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_C_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_C_SEN];		//��ֵ��ǰ���ɴʵ� ��46�У���ǰIP��PW�е���Ŀ��=47��

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_NUM_IN_C_SEN] = nMaxOfPPHInSEN;						//��ֵ��ǰ���ɴʵ� ��49�У���ǰSEN��PP����Ŀ��
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_NUM_IN_C_IP] = g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PPH_NUM_IN_C_SEN];		//��ֵ��ǰ���ɴʵ� ��48�У���ǰIP��PP����Ŀ��=49��

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_L_PW] = nMaxOfHanZiInPrevPW;					//��ֵ��ǰ���ɴʵ� ��51�У�ǰһPW��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_L_PPH] = nMaxOfHanZiInPrevPPH;					//��ֵ��ǰ���ɴʵ� ��52�У�ǰһPP��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_L_PPH] = nMaxOfPWInPrevPPH;						//��ֵ��ǰ���ɴʵ� ��54�У�ǰһPP��PW����Ŀ)

			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_R_PW] = nMaxOfHanZiInNextPW;					//��ֵ��ǰ���ɴʵ� ��57�У���һPW��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_SYL_NUM_IN_R_PPH] = nMaxOfHanZiInNextPPH;					//��ֵ��ǰ���ɴʵ� ��58�У���һPP��SYL����Ŀ)
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen+i][RLAB_PW_NUM_IN_R_PPH] = nMaxOfPWInNextPPH;						//��ֵ��ǰ���ɴʵ� ��60�У���һPP��PW����Ŀ)

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

	//����ѭ���󣬶����1��PPH��ֵ
	for( i=nStartCountOfSylInPPH; i<nCurCountOfSylInPPH; i++)
	{							
		//��ֵλ����Ϣ
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_FWD_POS_IN_PPH] = i - nStartCountOfSylInPPH;								//��ֵPPH֮ǰ�� ��22�У�PP�е�ǰSYL������λ�ã�
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_BWD_POS_IN_PPH] = nCurCountOfSylInPPH - i - 1;							//��ֵPPH֮ǰ�� ��32�У�PP�е�ǰSYL�ķ���λ�ã�  
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_BWD_POS_IN_PPH] = nCountOfPWInCurPPH - g_pLabRTGlobal->m_LabRam[i][RLAB_PW_FWD_POS_IN_PPH] - 1;					//��ֵPPH֮ǰ�� ��35�У�PP�е�ǰPW�ķ���λ�ã�  

		//��ֵ��Ŀ��Ϣ
		g_pLabRTGlobal->m_LabRam[i][RLAB_SYL_NUM_IN_C_PPH] = nCurCountOfSylInPPH - nStartCountOfSylInPPH;			//��ֵPPH֮ǰ�� ��42�У���ǰPP��SYL����Ŀ��
		g_pLabRTGlobal->m_LabRam[i][RLAB_PW_NUM_IN_C_PPH] = nCountOfPWInCurPPH;										//��ֵPPH֮ǰ�� ��45�У���ǰPP��PW����Ŀ��
	}
	nMaxOfHanZiInPrevPPH = nCountOfHanZiInPrevPPH;
	nMaxOfPWInPrevPPH = nCountOfPWInPrevPPH;
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_SYL_NUM_IN_L_PW] = nMaxOfHanZiInPrevPW;
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_SYL_NUM_IN_L_PPH] = nMaxOfHanZiInPrevPPH;
	g_pLabRTGlobal->m_LabRam[g_pLabRTGlobal->m_MaxLabLine-1][RLAB_PW_NUM_IN_L_PPH] = nMaxOfPWInPrevPPH;

}

//*****************************************************************************************************
//�������ܣ� �ں��������У������ɴ��ж���ʺϲ���һ���ڵ㣨pos�����������壩��ȥ��PW�ڵ㣬ÿ�����ֽڵ㼴Ϊ1��������PW
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
//�������ܣ� ��ñ����е�������Ϣ��ͨ��ָ�뷵�ز�����
//		ָ�뷵�أ�  �����еĺ���������		pMaxOfHanZiInSEN
//				�����е����ɴ�������	pMaxOfPWInSEN
//				�����е����ɶ���������	pMaxOfPPHInSEN
//		return���أ� �����е����ڸ����������֣�sil��pau��
//*****************************************************************************************************
emUInt8  emCall	GetSenInfo(emUInt8 *pMaxOfHanZiInSEN, emUInt8 *pMaxOfPWInSEN, emUInt8 *pMaxOfPPHInSEN)
{
	emInt16  nSylCount = 2;  //��ʼ����ǰ��2��sil����
	emInt16  nHanZiCount;

	LOG_StackAddr(__FUNCTION__);

	(*pMaxOfHanZiInSEN) = 0;
	(*pMaxOfPWInSEN) = 0;
	(*pMaxOfPPHInSEN) = 0;

	g_nCurTextByteIndex = 0;
	while(g_pTextInfo[g_nCurTextByteIndex/2].Len != 0)
	{
		
		if( g_pTextInfo[g_nCurTextByteIndex/2].BorderType == BORDER_PPH )		//�����ڵ� �� �߽�ڵ㣨PPH��
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
//�������ܣ� ���������ڵ�ƴ����Ϣ��д��Lab��ȥ����Lab�еĵ�0������20�У�
//*****************************************************************************************************
void emCall	GetPinYinInfoToLab()
{
		//��ĸ������ʽ�� ��ĸ������λ
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
						{SM_CN_METHOD_kong,				SM_CN_PLACE_kong}};			//	25	Ӣ����ĸ
						


	//��ĸ��ͷ������ʽ�� ��ĸ��β������ʽ
	emUInt8 emConst YunMuHeadAndRear[88][2]={    
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	0	��	��
						{YM_CN_FRONT_nil,				YM_CN_REAR_nil},				//	1	��	nil
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	2	��	a  
						{YM_CN_FRONT_open,				YM_CN_REAR_strectched},			//	3	��	ai  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	4	��	ao  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	5	��	an  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	6	��	ang  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	7	��	o  
						{YM_CN_FRONT_open,				YM_CN_REAR_round},				//	8	��	ou  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	9	��	e  
						{YM_CN_FRONT_open,				YM_CN_REAR_strectched},			//	10	��	ei  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	11	��	en  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	12	��	eng  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	13	��	er  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	14	��	io  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_strectched},			//	15	��	i  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	16	��	ia  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	17	��	iao  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	18	��	ian  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	19	��	iang  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_open},				//	20	��	ie  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	21	��	in  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	22	��	ing  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_nasal},				//	23	��	iong  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_round},				//	24	��	iou  
						{YM_CN_FRONT_round,				YM_CN_REAR_round},				//	25	��	u  
						{YM_CN_FRONT_round,				YM_CN_REAR_open},				//	26	��	ua  
						{YM_CN_FRONT_round,				YM_CN_REAR_open},				//	27	��	uo  
						{YM_CN_FRONT_round,				YM_CN_REAR_strectched},			//	28	��	uai  
						{YM_CN_FRONT_round,				YM_CN_REAR_strectched},			//	29	��	uei  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	30	��	uan  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	31	��	uen  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	32	��	uang  
						{YM_CN_FRONT_round,				YM_CN_REAR_nasal},				//	33	��	ueng  
						{YM_CN_FRONT_open,				YM_CN_REAR_nasal},				//	34	��	ong  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_protruded},			//	35	��	v  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_open},				//	36	��	ve  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_nasal},				//	37	��	van  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_nasal},				//	38	��	vn  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	39	��	ii  
						{YM_CN_FRONT_open,				YM_CN_REAR_open},				//	40	��	iii  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	41	��	air  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	42	��	angr  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	43	��	aor  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	44	��	engr  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	45	��	iaor  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	46	��	iar  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	47	��	ingr  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	48	��	iour  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	49	��	ir  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	50	��	our  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	51	��	uair  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	52	��	uangr  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	53	��	ueir  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	54	��	uor  
						{YM_CN_FRONT_round,				YM_CN_REAR_retroflex},			//	55	��	ur  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_retroflex},			//	56	��	vanr  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_retroflex},			//	57	��	ver  
						{YM_CN_FRONT_protruded,			YM_CN_REAR_retroflex},			//	58	��	vnr  
						{YM_CN_FRONT_open,				YM_CN_REAR_retroflex},			//	59	��	eir  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	60	��	ier  
						{YM_CN_FRONT_strectched,		YM_CN_REAR_retroflex},			//	61	��	iongr  
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	62	��	��ĸ��a
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	63	��	��ĸ��b
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	64	��	��ĸ��c
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	65	��	��ĸ��d
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	66	��	��ĸ��e
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	67	��	��ĸ��f
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	68	��	��ĸ��g
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	69	��	��ĸ��h
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	70	��	��ĸ��i
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	71	��	��ĸ��j
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	72	��	��ĸ��k
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	73	��	��ĸ��l
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	74	��	��ĸ��m
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	75	��	��ĸ��n
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	76	��	��ĸ��o
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	77	��	��ĸ��p
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	78	��	��ĸ��q
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	79	��	��ĸ��r
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	80	��	��ĸ��s
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	81	��	��ĸ��t
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	82	��	��ĸ��u
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	83	��	��ĸ��v
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	84	��	��ĸ��w
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	85	��	��ĸ��x
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong},				//	86	��	��ĸ��y
						{YM_CN_FRONT_kong,				YM_CN_REAR_kong}};				//	87	��	��ĸ��z
									


	emUInt8 nHanZiCount,i;
	emUInt16 nPinYin;
	emUInt8 nShengMu,nYunMu,nShengDiao;
	emUInt8 nCountOfSylInSen = 0;					//�����е����ڼ���

	LOG_StackAddr(__FUNCTION__);

	nCountOfSylInSen = 0;

	//����sil���ڣ� ������еı����ڵ���Ϣ 
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//����sil���ڣ� ������е�ǰ���ڵ���Ϣ ������sil�� ǰ���ڻ���sil��
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_L_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//����sil���ڣ� ����1�����е�ǰ���ڵ���Ϣ
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
		if( g_pTextInfo[g_nCurTextByteIndex/2].BorderType == BORDER_PPH )  //�����ڵ� �� �߽�ڵ㣨PPH������pau������Ϣ
		{

			//pau���ڣ� ������еı����ڵ���Ϣ 
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = INITIAL_CN_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = FINAL_CN_nil;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = TONE_CN_sil_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_pau][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_pau][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

			//pau���ڣ� ����1�����еĺ����ڵ���Ϣ
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM] = INITIAL_CN_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM] = FINAL_CN_nil;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_TONE] = TONE_CN_sil_pau;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_pau][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_pau][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

			//pau���ڣ� ����1�����е�ǰ���ڵ���Ϣ
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

			//������еı����ڵ���Ϣ
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = nShengMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = nYunMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = nShengDiao;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[nShengMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[nShengMu][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[nYunMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[nYunMu][1];

			//����1�����еĺ����ڵ���Ϣ
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM] = nShengMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM] = nYunMu;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_TONE] = nShengDiao;
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_METHOD] = ShengMuMethodAndPlace[nShengMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[nShengMu][1];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_FRONT] = YunMuHeadAndRear[nYunMu][0];
			g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_REAR] = YunMuHeadAndRear[nYunMu][1];		

			//����1�����е�ǰ���ڵ���Ϣ
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

	//��βsil���ڣ� ������еı����ڵ���Ϣ 
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_C_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//��βsil���ڣ� ������еĺ����ڵ���Ϣ ����βsil�� �����ڻ���sil��
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen][RLAB_R_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];

	//��βsil���ڣ� ����1�����еĺ����ڵ���Ϣ
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM] = INITIAL_CN_sil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM] = FINAL_CN_nil;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_TONE] = TONE_CN_sil_pau;
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_METHOD] =  ShengMuMethodAndPlace[INITIAL_CN_sil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_SM_PLACE] = ShengMuMethodAndPlace[INITIAL_CN_sil][1];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_FRONT] = YunMuHeadAndRear[FINAL_CN_nil][0];
	g_pLabRTGlobal->m_LabRam[nCountOfSylInSen-1][RLAB_R_YM_REAR] = YunMuHeadAndRear[FINAL_CN_nil][1];


	//�������sil�е���1������Ϣ		//2012-02-03 hyl modified
	nPinYin = g_pTextPinYinCode[0];
	nYunMu = (nPinYin & 0x7f00)>>8;
	nShengMu = (nPinYin & 0x00f8)>>3;
	nShengDiao = nPinYin & 0x0007;
	g_pLabRTGlobal->m_LabRam[0][RLAB_R_SM] = nShengMu;
	g_pLabRTGlobal->m_LabRam[0][RLAB_R_YM] = nYunMu;
	g_pLabRTGlobal->m_LabRam[0][RLAB_R_TONE] = nShengDiao;

}

