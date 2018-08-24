/*  ��˵��б����ļ�����ĸ����ĸ��������LAB��Ϣ��   */

#ifndef	_EMTTS__LIST_REAR_LAB__HXX_
#define _EMTTS__LIST_REAR_LAB__HXX_


#ifdef __cplusplus
extern "C" {
#endif


//���ģ�����  ********************************************************************************************
#define TONE_CN_1			1		//һ��
#define TONE_CN_2			2		//����
#define TONE_CN_3			3		//����
#define TONE_CN_4			4		//����
#define TONE_CN_light		5		//����
#define TONE_CN_sil_pau		6		//sil����pau��
#define TONE_CN_letter		7		//������ĸ����


//����:��ĸ������ʽ  ********************************************************************************************
#define SM_CN_METHOD_kong				0	// 
#define SM_CN_METHOD_affricate			1	// ������		������ĸ��z,zh,j
#define SM_CN_METHOD_asprt_affricate	2	// ��������		������ĸ��c,ch,q
#define SM_CN_METHOD_asprt_stop			3	// ������		������ĸ��p,t,k
#define SM_CN_METHOD_fricative			4	// ����			������ĸ��f,s,sh,r,x,h
#define SM_CN_METHOD_lateral			5	// ����			������ĸ��l
#define SM_CN_METHOD_nasal				6	// ����			������ĸ��m,n
#define SM_CN_METHOD_nil				7	// ������		������ĸ��nil
#define SM_CN_METHOD_stop				8	// ����			������ĸ��b,d,g
#define SM_CN_METHOD_zero				9	// ����ĸ	


//����:��ĸ����λ��  ********************************************************************************************
#define SM_CN_PLACE_kong				0	// 
#define SM_CN_PLACE_dentalveolar		1	// �����		������ĸ��d,t,z,c,s,n,l
#define SM_CN_PLACE_dorsal				2	// ������		������ĸ��j,q,x
#define SM_CN_PLACE_labial				3	// ����			������ĸ��b,p,m
#define SM_CN_PLACE_labiodental			4	// ������		������ĸ��f
#define SM_CN_PLACE_nil					5	// ������		������ĸ��nil
#define SM_CN_PLACE_retroflex			6	// ������		������ĸ��zh,ch,sh,r
#define SM_CN_PLACE_velar				7	// ������		������ĸ��g,k,h
#define SM_CN_PLACE_zero				8	// ����ĸ		������ĸ��zero

//����:��ĸ����ͷ������ʽ  ********************************************************************************************

#define YM_CN_FRONT_kong				0	//
#define YM_CN_FRONT_nil					1	// ������		������ĸ��nil
#define YM_CN_FRONT_open				2	// ��			������ĸ��a,ai,air,an,ang,angr,ao,aor,o,ong,ou,our,e,ei,eir,en,eng,engr,er,ii,iii
#define YM_CN_FRONT_protruded			3	// ��			������ĸ��v,van,vanr,ve,ver,vn,vnr
#define YM_CN_FRONT_round				4	// ��			������ĸ��u,ua,uai,uair,uan,uang,uangr,uei,ueir,uen,ueng,uo,uor,ur
#define YM_CN_FRONT_strectched			5	// ��			������ĸ��i,ia,ian,iang,iao,iaor,iar,ie,in,ing,ingr,io,iong,iou,iour,ir
#define YM_CN_FRONT_nasal				6	// ����			һ��û��

//����:��ĸ����β������ʽ  ********************************************************************************************

#define YM_CN_REAR_kong					0	//
#define YM_CN_REAR_nasal				1	// ����			������ĸ��an,en,ian,in,uan,uen,van,vn,ang,eng,iang,ing,iong,ong,uang,ueng
#define YM_CN_REAR_nil					2	// ������		������ĸ��nil
#define YM_CN_REAR_open					3	// ��			������ĸ��a,ao,e,ia,iao,ie,io,o,ua,uo,ve,ii,iii
#define YM_CN_REAR_protruded			4	// ��			������ĸ��v
#define YM_CN_REAR_retroflex			5	// ������		������ĸ��air,angr,aor,eir,engr,er,iaor,iar,ingr,iour,ir,our,uair,uangr,ueir,uor,ur,vanr,ver,vnr
#define YM_CN_REAR_round				6	// ��			������ĸ��iou,ou,u
#define YM_CN_REAR_strectched			7	// ��			������ĸ��ai,ei,i,uai,uei


//����:��ĸ  ********************************************************************************************
//									��ĸ������ʽ���	��ĸ�������ͱ��
#define INITIAL_CN_kong		0		//
#define INITIAL_CN_sil		1		//	7	5	
#define INITIAL_CN_pau		2		//	7	5	
#define INITIAL_CN_zero		3		//	9	8	����ĸ
#define INITIAL_CN_b		4		//	8	3	
#define INITIAL_CN_ch		5		//	2	6	
#define INITIAL_CN_c		6		//	2	1	
#define INITIAL_CN_d		7		//	8	1	
#define INITIAL_CN_f		8		//	4	4	
#define INITIAL_CN_g		9		//	8	7	
#define INITIAL_CN_h		10		//	4	7	
#define INITIAL_CN_j		11		//	1	2	
#define INITIAL_CN_k		12		//	3	7	
#define INITIAL_CN_l		13		//	5	1	
#define INITIAL_CN_m		14		//	6	3	
#define INITIAL_CN_n		15		//	6	1	
#define INITIAL_CN_p		16		//	3	3	
#define INITIAL_CN_q		17		//	2	2	
#define INITIAL_CN_r		18		//	4	6	
#define INITIAL_CN_sh		19		//	4	6	
#define INITIAL_CN_s		20		//	4	1	
#define INITIAL_CN_t		21		//	3	1	
#define INITIAL_CN_x		22		//	4	2	
#define INITIAL_CN_zh		23		//	1	6	
#define INITIAL_CN_z		24		//	1	1	
#define INITIAL_CN_letter	25		//	0	0	����Ӣ����ĸ����ĸ



//����:��ĸ  ********************************************************************************************
//									��ͷ��ʽ���	��β��ʽ���	��ע
#define FINAL_CN_kong		0		//
#define FINAL_CN_nil		1		//	1	2  ��pau��sil  
#define FINAL_CN_a  		2		//	2	3    
#define FINAL_CN_ai  		3		//	2	7    
#define FINAL_CN_ao  		4		//	2	3    
#define FINAL_CN_an  		5		//	2	1    
#define FINAL_CN_ang  		6		//	2	1    
#define FINAL_CN_o  		7		//	2	3    
#define FINAL_CN_ou  		8		//	2	6    
#define FINAL_CN_e  		9		//	2	3    
#define FINAL_CN_ei  		10		//	2	7    
#define FINAL_CN_en  		11		//	2	1    
#define FINAL_CN_eng  		12		//	2	1    
#define FINAL_CN_er  		13		//	2	5    
#define FINAL_CN_io  		14		//	5	3    
#define FINAL_CN_i  		15		//	5	7  ��bi,di,ji,li,mi,ni,pi,qi,ti,xi,yi  
#define FINAL_CN_ia  		16		//	5	3    
#define FINAL_CN_iao  		17		//	5	3    
#define FINAL_CN_ian  		18		//	5	1    
#define FINAL_CN_iang  		19		//	5	1    
#define FINAL_CN_ie  		20		//	5	3    
#define FINAL_CN_in  		21		//	5	1    
#define FINAL_CN_ing  		22		//	5	1    
#define FINAL_CN_iong  		23		//	5	1    
#define FINAL_CN_iou  		24		//	5	6    
#define FINAL_CN_u  		25		//	4	6    
#define FINAL_CN_ua  		26		//	4	3    
#define FINAL_CN_uo  		27		//	4	3    
#define FINAL_CN_uai  		28		//	4	7    
#define FINAL_CN_uei  		29		//	4	7    
#define FINAL_CN_uan  		30		//	4	1    
#define FINAL_CN_uen  		31		//	4	1    
#define FINAL_CN_uang  		32		//	4	1    
#define FINAL_CN_ueng  		33		//	4	1    
#define FINAL_CN_ong  		34		//	2	1    
#define FINAL_CN_v  		35		//	3	4    
#define FINAL_CN_ve  		36		//	3	3    
#define FINAL_CN_van  		37		//	3	1    
#define FINAL_CN_vn  		38		//	3	1    
#define FINAL_CN_ii  		39		//	2	3  ��zi,ci,si  
#define FINAL_CN_iii  		40		//	2	3  ��zhi,chi,shi,ri  

//������ĸ
#define FINAL_CN_air  		41		//	2	5  ��(ar,air,anr)��banr,char,far,gair,ganr,hair,kair,lanr,mar,nair,nar,pair,panr,tanr,zair
#define FINAL_CN_angr  		42		//	2	5  ��tangr
#define FINAL_CN_aor  		43		//	2	5  ��gaor,laor,maor,naor,shaor,zhaor
#define FINAL_CN_engr  		44		//	2	5  ��fengr
#define FINAL_CN_iaor  		45		//	5	5  ��diaor,piaor,tiaor
#define FINAL_CN_iar  		46		//	5	5  ��(iar,ianr,iangr)��bianr,dianr,jianr,lianr,nianr,pianr,qianr,tianr,xianr,xiar,yanr,biangr
#define FINAL_CN_ingr  		47		//	5	5  ��dingr
#define FINAL_CN_iour  		48		//	5	5  ��liur,niur
#define FINAL_CN_ir  		49		//	5	5  ��(ir,inr)��dir,jinr,lir,nir,pir,yir
#define FINAL_CN_our  		50		//	2	5  ��(our,ongr,or)��dour,hour,shour,tour,dongr,dor
#define FINAL_CN_uair  		51		//	4	5  ��(uar,uair,uanr)��guanr,guar,huair,huar,kuair,wair,wanr
#define FINAL_CN_uangr  	52		//	4	5  ��kuangr
#define FINAL_CN_ueir  		53		//	4	5  ��(ueir,uir,unr)��duir,dunr,gunr,huir,kunr,shuir,tuir,weir,zhuir,zhunr
#define FINAL_CN_uor  		54		//	4	5  ��guor,huor,wor,zuor
#define FINAL_CN_ur  		55		//	4	5  ��zhur
#define FINAL_CN_vanr  		56		//	3	5  ��juanr,quanr,yuanr
#define FINAL_CN_ver  		57		//	3	5  ��juer
#define FINAL_CN_vnr  		58		//	3	5  ��(vr,nvr)��qur,xur
#define FINAL_CN_eir  		59		//	2	5  ��(eir,enr,iir,iiir)��beir,benr,genr,menr,penr,renr,sheir,shenr,teir,shir,sir
#define FINAL_CN_ier  		60		//	5	5  ��bier,dier
#define FINAL_CN_iongr  	61		//	5	5  ��xiongr,diongr

//Ӣ����ĸ��ĸ
#define FINAL_CN_letter_a  	62		//	0	0   
#define FINAL_CN_letter_b  	63		//	0	0    
#define FINAL_CN_letter_c  	64		//	0	0     
#define FINAL_CN_letter_d  	65		//	0	0     
#define FINAL_CN_letter_e  	66		//	0	0    
#define FINAL_CN_letter_f  	67		//	0	0     
#define FINAL_CN_letter_g  	68		//	0	0  
#define FINAL_CN_letter_h  	69		//	0	0      
#define FINAL_CN_letter_i  	70		//	0	0      
#define FINAL_CN_letter_j  	71		//	0	0      
#define FINAL_CN_letter_k  	72		//	0	0      
#define FINAL_CN_letter_l  	73		//	0	0      
#define FINAL_CN_letter_m  	74		//	0	0      
#define FINAL_CN_letter_n  	75		//	0	0      
#define FINAL_CN_letter_o  	76		//	0	0      
#define FINAL_CN_letter_p  	77		//	0	0      
#define FINAL_CN_letter_q  	78		//	0	0      
#define FINAL_CN_letter_r  	79		//	0	0      
#define FINAL_CN_letter_s  	80		//	0	0      
#define FINAL_CN_letter_t  	81		//	0	0      
#define FINAL_CN_letter_u  	82		//	0	0      
#define FINAL_CN_letter_v  	83		//	0	0      
#define FINAL_CN_letter_w  	84		//	0	0      
#define FINAL_CN_letter_x  	85		//	0	0      
#define FINAL_CN_letter_y  	86		//	0	0      
#define FINAL_CN_letter_z  	87		//	0	0      



//����:LAB�ж���  ********************************************************************************************

#define	RLAB_C_SM						0			//	��������ĸ����
#define	RLAB_C_YM						1			//	��������ĸ����
#define	RLAB_C_TONE						6			//	��������������
#define	RLAB_L_SM						7			//	ǰ������ĸ����
#define	RLAB_L_YM						8			//	ǰ������ĸ����
#define	RLAB_L_TONE						13			//	ǰ������������   6
#define	RLAB_R_SM						14			//	��������ĸ����    7
#define	RLAB_R_YM						15			//	��������ĸ����  8
#define	RLAB_R_TONE						20			//	��������������   9
#define	RLAB_C_SM_METHOD				2			//	��������ĸ������ʽ����  10
#define	RLAB_C_SM_PLACE					3			//	��������ĸ������λ����  11
#define	RLAB_C_YM_FRONT					4			//	��������ͷ������ʽ����  12
#define	RLAB_C_YM_REAR					5			//	��������β������ʽ����  13
#define	RLAB_L_SM_METHOD				9			//	ǰ������ĸ������ʽ����  14
#define	RLAB_L_SM_PLACE					10			//	ǰ������ĸ������λ����  15
#define	RLAB_L_YM_FRONT					11			//	ǰ������ͷ������ʽ����  16
#define	RLAB_L_YM_REAR					12			//	ǰ������β������ʽ����   17
#define	RLAB_R_SM_METHOD				16			//	��������ĸ������ʽ����  18
#define	RLAB_R_SM_PLACE					17			//	��������ĸ������λ����  19
#define	RLAB_R_YM_FRONT					18			//	��������ͷ������ʽ����  20
#define	RLAB_R_YM_REAR					19			//	��������β������ʽ����  21

//#define	RLAB_C_SM						0			//	��������ĸ����
//#define	RLAB_C_YM						1			//	��������ĸ����
//#define	RLAB_C_TONE						2			//	��������������
//#define	RLAB_L_SM						3			//	ǰ������ĸ����
//#define	RLAB_L_YM						4			//	ǰ������ĸ����
//#define	RLAB_L_TONE						5			//	ǰ������������
//#define	RLAB_R_SM						6			//	��������ĸ����
//#define	RLAB_R_YM						7			//	��������ĸ����
//#define	RLAB_R_TONE						8			//	��������������
//#define	RLAB_C_SM_METHOD				9			//	��������ĸ������ʽ����
//#define	RLAB_C_SM_PLACE					10			//	��������ĸ������λ����
//#define	RLAB_C_YM_FRONT					11			//	��������ͷ������ʽ����
//#define	RLAB_C_YM_REAR					12			//	��������β������ʽ����
//#define	RLAB_L_SM_METHOD				13			//	ǰ������ĸ������ʽ����
//#define	RLAB_L_SM_PLACE					14			//	ǰ������ĸ������λ����
//#define	RLAB_L_YM_FRONT					15			//	ǰ������ͷ������ʽ����
//#define	RLAB_L_YM_REAR					16			//	ǰ������β������ʽ����
//#define	RLAB_R_SM_METHOD				17			//	��������ĸ������ʽ����
//#define	RLAB_R_SM_PLACE					18			//	��������ĸ������λ����
//#define	RLAB_R_YM_FRONT					19			//	��������ͷ������ʽ����
//#define	RLAB_R_YM_REAR					20			//	��������β������ʽ����
#define	RLAB_SYL_FWD_POS_IN_PW			21			//	PW�е�ǰSYL������λ��
#define	RLAB_SYL_FWD_POS_IN_PPH			22			//	PP�е�ǰSYL������λ��
#define	RLAB_SYL_FWD_POS_IN_IP			23			//	IP�е�ǰSYL������λ��				//
#define	RLAB_SYL_FWD_POS_IN_SEN			24			//	SEN�е�ǰSYL������λ��
#define	RLAB_PW_FWD_POS_IN_PPH			25			//	PP�е�ǰPW������λ��
#define	RLAB_PW_FWD_POS_IN_IP			26			//	IP�е�ǰPW�е�����λ��				//
#define	RLAB_PW_FWD_POS_IN_SEN			27			//	SEN�е�ǰPW������λ��
#define	RLAB_PPH_FWD_POS_IN_IP			28			//	IP�е�ǰPP������λ��				//
#define	RLAB_PPH_FWD_POS_IN_SEN			29			//	SEN�е�ǰPP������λ��
#define	RLAB_IP_FWD_POS_IN_SEN			30			//	SEN�е�ǰIP������λ��				//
#define	RLAB_SYL_BWD_POS_IN_PW			31			//	PW�е�ǰSYL�ķ���λ��
#define	RLAB_SYL_BWD_POS_IN_PPH			32			//	PP�е�ǰSYL�ķ���λ��
#define	RLAB_SYL_BWD_POS_IN_IP			33			//	IP�е�ǰSYL�ķ���λ��				//
#define	RLAB_SYL_BWD_POS_IN_SEN			34			//	SEN�е�ǰSYL�ķ���λ��
#define	RLAB_PW_BWD_POS_IN_PPH			35			//	PP�е�ǰPW�ķ���λ��
#define	RLAB_PW_BWD_POS_IN_IP			36			//	IP�е�ǰPW�еķ���λ��				//
#define	RLAB_PW_BWD_POS_IN_SEN			37			//	SEN�е�ǰPW�ķ���λ��
#define	RLAB_PPH_BWD_POS_IN_IP			38			//	IP�е�ǰPP�ķ���λ��				//
#define	RLAB_PPH_BWD_POS_IN_SEN			39			//	SEN�е�ǰPP�ķ���λ��
#define	RLAB_IP_BWD_POS_IN_SEN			40			//	SEN�е�ǰIP�ķ���λ��				//
#define	RLAB_SYL_NUM_IN_C_PW			41			//	��ǰPW��SYL����Ŀ
#define	RLAB_SYL_NUM_IN_C_PPH			42			//	��ǰPP��SYL����Ŀ
#define	RLAB_SYL_NUM_IN_C_IP			43			//	��ǰIP��SYL����Ŀ				//
#define	RLAB_SYL_NUM_IN_C_SEN			44			//	��ǰSEN��SYL����Ŀ
#define	RLAB_PW_NUM_IN_C_PPH			45			//	��ǰPP��PW����Ŀ
#define	RLAB_PW_NUM_IN_C_IP				46			//	��ǰIP��PW�е���Ŀ				//
#define	RLAB_PW_NUM_IN_C_SEN			47			//	��ǰSEN��PW����Ŀ
#define	RLAB_PPH_NUM_IN_C_IP			48			//	��ǰIP��PP����Ŀ				//
#define	RLAB_PPH_NUM_IN_C_SEN			49			//	��ǰSEN��PP����Ŀ
#define	RLAB_IP_NUM_IN_C_SEN			50			//	��ǰSEN��IP����Ŀ				//
#define	RLAB_SYL_NUM_IN_L_PW			51			//	ǰһPW��SYL����Ŀ
#define	RLAB_SYL_NUM_IN_L_PPH			52			//	ǰһPP��SYL����Ŀ
#define	RLAB_SYL_NUM_IN_L_IP			53			//	ǰһIP��SYL����Ŀ				//
#define	RLAB_PW_NUM_IN_L_PPH			54			//	ǰһPP��PW����Ŀ
#define	RLAB_PW_NUM_IN_L_IP				55			//	ǰһIP��PW����Ŀ				//
#define	RLAB_PPH_NUM_IN_L_IP			56			//	ǰһIP��PP����Ŀ				//
#define	RLAB_SYL_NUM_IN_R_PW			57			//	��һPW��SYL����Ŀ
#define	RLAB_SYL_NUM_IN_R_PPH			58			//	��һPP��SYL����Ŀ
#define	RLAB_SYL_NUM_IN_R_IP			59			//	��һIP��SYL����Ŀ				//
#define	RLAB_PW_NUM_IN_R_PPH			60			//	��һPP��PW����Ŀ
#define	RLAB_PW_NUM_IN_R_IP				61			//	��һIP��PW����Ŀ				//
#define	RLAB_PPH_NUM_IN_R_IP			62			//	��һIP��PP����Ŀ				//


//Ӣ��:LAB�ж���  ********************************************************************************************
#define	ELAB_C01_C_pp							0			//��PP����
#define	ELAB_C02_L_pp							1			//ǰPP����
#define	ELAB_C03_R_pp							2			//��PP����
#define	ELAB_C04_LL_pp							3			//ǰǰPP����
#define	ELAB_C05_RR_pp							4			//���PP����
#define	ELAB_D01_C_stress						5			//��ǰSYL����������
#define	ELAB_D02_L_stress						6			//ǰһSYL����������
#define	ELAB_D03_R_stress						7			//��һSYL����������
#define	ELAB_D04_LL_stress						8			//ǰǰSYL����������
#define	ELAB_D05_RR_stress						9			//���SYL����������
#define	ELAB_E01_L_StrNumInWord					10			//��WORD��ǰstress�����뱾���ڵľ���
#define	ELAB_E02_R_StrNumInWord					11			//��WORD�к�stress�����뱾���ڵľ���
#define	ELAB_F01_L_Border_Of_Syl				12			//��SYL����߽磨<=��
#define	ELAB_F02_R_Border_Of_Syl				13			//��SYL���ұ߽磨<=��
#define	ELAB_G01_L_Border_Of_Word				14			//��WORD����߽磨<=��
#define	ELAB_G02_R_Border_Of_Word				15			//��WORD���ұ߽磨<=��
#define	ELAB_H01_RelPos_SylInWord				16			//��WORD�е�SYL���λ��
#define	ELAB_H02_RelPos_SylInPph				17			//��PPH�е�SYL���λ��
#define	ELAB_H03_RelPos_WordInPph				18			//��PPH�е�WORD���λ��
#define	ELAB_H04_RelPos_WordInSen				19			//��SEN�е�WORD���λ��
#define	ELAB_H05_RelPos_PphInSen				20			//��SEN�е�PPH���λ��
#define	ELAB_I01_FwdPos_SylInWord				21			//WORD�е�ǰSYL������λ��
#define	ELAB_I02_FwdPos_SylInPph				22			//PPH�е�ǰSYL������λ��
#define	ELAB_I03_FwdPos_WordInPph				23			//PPH�е�ǰWORD������λ��
#define	ELAB_I04_FwdPos_WordInSen				24			//SEN�е�ǰWORD������λ��
#define	ELAB_I05_FwdPos_PphInSen				25			//SEN�е�ǰPPH������λ��
#define	ELAB_J01_BwdPos_SylInWord				26			//WORD�е�ǰSYL�ķ���λ��
#define	ELAB_J02_BwdPos_SylInPph				27			//PPH�е�ǰSYL�ķ���λ��
#define	ELAB_J03_BwdPos_WordInPph				28			//PPH�е�ǰWORD�ķ���λ��
#define	ELAB_J04_BwdPos_WordInSen				29			//SEN�е�ǰWORD�ķ���λ��
#define	ELAB_J05_BwdPos_PphInSen				30			//SEN�е�ǰPPH�ķ���λ��
#define	ELAB_K01_PpNum_InCSyl					31			//��ǰSYL�����ص���Ŀ
#define	ELAB_K02_SylNum_InCWord					32			//��ǰWORD��SYL����Ŀ
#define	ELAB_K03_SylNum_InCPph					33			//��ǰPPH��SYL����Ŀ
#define	ELAB_K04_WordNum_InCPph					34			//��ǰPPH��WORD����Ŀ
#define	ELAB_K05_WordNum_InCSen					35			//��ǰSEN��WORD����Ŀ
#define	ELAB_K06_PphNum_InCSen					36			//��ǰSEN��PPH����Ŀ
#define	ELAB_K07_PpNum_InLSyl					37			//ǰһSYL�����ص���Ŀ
#define	ELAB_K08_SylNum_InLWord					38			//ǰһWORD��SYL����Ŀ
#define	ELAB_K09_WordNum_InLPph					39			//ǰһPPH��WORD����Ŀ
#define	ELAB_K10_PpNum_InRSyl					40			//��һSYL�����ص���Ŀ
#define	ELAB_K11_SylNum_InRWord					41			//��һWORD��SYL����Ŀ
#define	ELAB_K12_WordNum_InRPph					42			//��һPPH��WORD����Ŀ
#define	ELAB_L01_PpToLVow_NumInWord				43			//�������ڱ�������ǰԪ���ľ���
#define	ELAB_L02_PpToRVow_NumInWord				44			//�������ڱ��������Ԫ���ľ���
#define	ELAB_U01_C_Syl_Accent					45			//��SYL��accent������������
#define	ELAB_U02_C_Word_Accent					46			//��word���Ƿ����ض�����
#define	ELAB_V01_AccentNum_BeforeInPph			47			//��PPH�б�SYLǰ��accent������
#define	ELAB_V02_AccentNum_AfterInPph			48			//��PPH�б�SYL���accent������
#define	ELAB_V03_LAccent_SylNumInPph			49			//��PPH��ǰaccent�����뱾SYL�ľ���
#define	ELAB_V04_RAccent_SylNumInPph			50			//��PPH�к�accent�����뱾SYL�ľ���
#define	ELAB_V05_LAccent_WordNumInPph			51			//��PPH�б�word��ǰaccent���ʵľ���
#define	ELAB_V06_RAccent_WordNumInPph			52			//��PPH�б�word���accent���ʵľ���
#define	ELAB_W01_C_PPH_Tone						53			//��PPH�ľ��
#define	ELAB_W02_L_PPH_Tone						54			//ǰPPH�ľ��
#define	ELAB_W03_R_PPH_Tone						55			//��PPH�ľ��
#define	ELAB_W04_LL_PPH_Tone					56			//ǰǰPPH�ľ��
#define	ELAB_W05_RR_PPH_Tone					57			//���PPH�ľ��
#define	ELAB_W06_C_pp							58			//�������Ƿ����쳣��
#define	ELAB_B01_C_T0							59			//��pp����0����
#define	ELAB_B01_C_T1							60			//��pp����1����
#define	ELAB_B01_C_T2							61			//��pp����2����
#define	ELAB_B01_C_T3							62			//��pp����3����
#define	ELAB_B01_C_T4							63			//��pp����4����
#define	ELAB_B01_C_T5							64			//��pp����5����
#define	ELAB_B01_C_T6							65			//��pp����6����
#define	ELAB_B01_C_T7							66			//��pp����7����
#define	ELAB_B02_L_T0							67			//ǰpp����0����
#define	ELAB_B02_L_T1							68			//ǰpp����1����
#define	ELAB_B02_L_T2							69			//ǰpp����2����
#define	ELAB_B02_L_T3							70			//ǰpp����3����
#define	ELAB_B02_L_T4							71			//ǰpp����4����
#define	ELAB_B02_L_T5							72			//ǰpp����5����
#define	ELAB_B02_L_T6							73			//ǰpp����6����
#define	ELAB_B02_L_T7							74			//ǰpp����7����
#define	ELAB_B03_R_T0							75			//��pp����0����
#define	ELAB_B03_R_T1							76			//��pp����1����
#define	ELAB_B03_R_T2							77			//��pp����2����
#define	ELAB_B03_R_T3							78			//��pp����3����
#define	ELAB_B03_R_T4							79			//��pp����4����
#define	ELAB_B03_R_T5							80			//��pp����5����
#define	ELAB_B03_R_T6							81			//��pp����6����
#define	ELAB_B03_R_T7							82			//��pp����7����
#define	ELAB_B04_LL_T0							83			//ǰǰpp����0����
#define	ELAB_B04_LL_T1							84			//ǰǰpp����1����
#define	ELAB_B04_LL_T2							85			//ǰǰpp����2����
#define	ELAB_B04_LL_T3							86			//ǰǰpp����3����
#define	ELAB_B04_LL_T4							87			//ǰǰpp����4����
#define	ELAB_B04_LL_T5							88			//ǰǰpp����5����
#define	ELAB_B04_LL_T6							89			//ǰǰpp����6����
#define	ELAB_B04_LL_T7							90			//ǰǰpp����7����
#define	ELAB_B05_RR_T0							91			//���pp����0����
#define	ELAB_B05_RR_T1							92			//���pp����1����
#define	ELAB_B05_RR_T2							93			//���pp����2����
#define	ELAB_B05_RR_T3							94			//���pp����3����
#define	ELAB_B05_RR_T4							95			//���pp����4����
#define	ELAB_B05_RR_T5							96			//���pp����5����
#define	ELAB_B05_RR_T6							97			//���pp����6����
#define	ELAB_B05_RR_T7							98			//���pp����7����

//Ӣ��:���ر��붨��  ********************************************************************************************
#define	EngPP_aa	1
#define	EngPP_ae	2
#define	EngPP_ah	3
#define	EngPP_ao	4
#define	EngPP_aw	5
#define	EngPP_ay	6
#define	EngPP_b		7
#define	EngPP_ch	8
#define	EngPP_d		9
#define	EngPP_dh	10
#define	EngPP_eh	11
#define	EngPP_er	12
#define	EngPP_ey	13
#define	EngPP_f		14
#define	EngPP_g		15
#define	EngPP_hh	16
#define	EngPP_ih	17
#define	EngPP_iy	18
#define	EngPP_jh	19
#define	EngPP_k		20
#define	EngPP_l		21
#define	EngPP_m		22
#define	EngPP_n		23
#define	EngPP_ng	24
#define	EngPP_ow	25
#define	EngPP_oy	26
#define	EngPP_p		27
#define	EngPP_r		28
#define	EngPP_s		29
#define	EngPP_sh	30
#define	EngPP_t		31
#define	EngPP_th	32
#define	EngPP_uh	33
#define	EngPP_uw	34
#define	EngPP_v		35
#define	EngPP_w		36
#define	EngPP_y		37
#define	EngPP_z		38
#define	EngPP_zh	39
#define	EngPP_ax	40
#define	EngPP_silh	51
#define	EngPP_silt	52
#define	EngPP_sp	53


//�������ڱ��������Ԫ���ľ���----�ж��Ƿ��ǵ���β��
#define ELAB_PpToRVow_NumInWord_x   255        //x��ʾ���غ���Ԫ��
#define ELAB_PpToLVow_NumInWord_x   255        //x��ʾ����ǰ��Ԫ��



	#define E_RLAB_PH_NUM_IN_C_SYL			31			//��ǰSYL�����ص���Ŀ
	#define E_RLAB_SYL_NUM_IN_C_PW			32			//��ǰPW��SYL����Ŀ
	#define E_RLAB_R_BORDER_IN_WORD			15			//��ǰ���ʵ��ұ߽�




#ifdef __cplusplus
}
#endif

#endif /* !_EMTTS__LIST_REAR_LAB__HXX_ */	