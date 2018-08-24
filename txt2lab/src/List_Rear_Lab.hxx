/*  后端的列表定义文件：声母，韵母，声调，LAB信息等   */

#ifndef	_EMTTS__LIST_REAR_LAB__HXX_
#define _EMTTS__LIST_REAR_LAB__HXX_


#ifdef __cplusplus
extern "C" {
#endif


//中文：声调  ********************************************************************************************
#define TONE_CN_1			1		//一声
#define TONE_CN_2			2		//二声
#define TONE_CN_3			3		//三声
#define TONE_CN_4			4		//四声
#define TONE_CN_light		5		//轻声
#define TONE_CN_sil_pau		6		//sil声和pau声
#define TONE_CN_letter		7		//虚拟字母声调


//中文:声母发音方式  ********************************************************************************************
#define SM_CN_METHOD_kong				0	// 
#define SM_CN_METHOD_affricate			1	// 塞擦音		包含声母：z,zh,j
#define SM_CN_METHOD_asprt_affricate	2	// 塞擦送气		包含声母：c,ch,q
#define SM_CN_METHOD_asprt_stop			3	// 塞送气		包含声母：p,t,k
#define SM_CN_METHOD_fricative			4	// 擦音			包含声母：f,s,sh,r,x,h
#define SM_CN_METHOD_lateral			5	// 边音			包含声母：l
#define SM_CN_METHOD_nasal				6	// 鼻音			包含声母：m,n
#define SM_CN_METHOD_nil				7	// 静音段		包含声母：nil
#define SM_CN_METHOD_stop				8	// 塞音			包含声母：b,d,g
#define SM_CN_METHOD_zero				9	// 零声母	


//中文:声母发音位置  ********************************************************************************************
#define SM_CN_PLACE_kong				0	// 
#define SM_CN_PLACE_dentalveolar		1	// 舌尖音		包含声母：d,t,z,c,s,n,l
#define SM_CN_PLACE_dorsal				2	// 舌面音		包含声母：j,q,x
#define SM_CN_PLACE_labial				3	// 唇音			包含声母：b,p,m
#define SM_CN_PLACE_labiodental			4	// 唇齿音		包含声母：f
#define SM_CN_PLACE_nil					5	// 静音段		包含声母：nil
#define SM_CN_PLACE_retroflex			6	// 卷舌音		包含声母：zh,ch,sh,r
#define SM_CN_PLACE_velar				7	// 软腭音		包含声母：g,k,h
#define SM_CN_PLACE_zero				8	// 零声母		包含声母：zero

//中文:韵母的韵头发音方式  ********************************************************************************************

#define YM_CN_FRONT_kong				0	//
#define YM_CN_FRONT_nil					1	// 静音段		包含韵母：nil
#define YM_CN_FRONT_open				2	// 开			包含韵母：a,ai,air,an,ang,angr,ao,aor,o,ong,ou,our,e,ei,eir,en,eng,engr,er,ii,iii
#define YM_CN_FRONT_protruded			3	// 撮			包含韵母：v,van,vanr,ve,ver,vn,vnr
#define YM_CN_FRONT_round				4	// 合			包含韵母：u,ua,uai,uair,uan,uang,uangr,uei,ueir,uen,ueng,uo,uor,ur
#define YM_CN_FRONT_strectched			5	// 齐			包含韵母：i,ia,ian,iang,iao,iaor,iar,ie,in,ing,ingr,io,iong,iou,iour,ir
#define YM_CN_FRONT_nasal				6	// 鼻音			一般没有

//中文:韵母的韵尾发音方式  ********************************************************************************************

#define YM_CN_REAR_kong					0	//
#define YM_CN_REAR_nasal				1	// 鼻音			包含韵母：an,en,ian,in,uan,uen,van,vn,ang,eng,iang,ing,iong,ong,uang,ueng
#define YM_CN_REAR_nil					2	// 静音段		包含韵母：nil
#define YM_CN_REAR_open					3	// 开			包含韵母：a,ao,e,ia,iao,ie,io,o,ua,uo,ve,ii,iii
#define YM_CN_REAR_protruded			4	// 撮			包含韵母：v
#define YM_CN_REAR_retroflex			5	// 卷舌音		包含韵母：air,angr,aor,eir,engr,er,iaor,iar,ingr,iour,ir,our,uair,uangr,ueir,uor,ur,vanr,ver,vnr
#define YM_CN_REAR_round				6	// 合			包含韵母：iou,ou,u
#define YM_CN_REAR_strectched			7	// 齐			包含韵母：ai,ei,i,uai,uei


//中文:声母  ********************************************************************************************
//									声母发音方式编号	声母发音类型编号
#define INITIAL_CN_kong		0		//
#define INITIAL_CN_sil		1		//	7	5	
#define INITIAL_CN_pau		2		//	7	5	
#define INITIAL_CN_zero		3		//	9	8	零声母
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
#define INITIAL_CN_letter	25		//	0	0	虚拟英文字母的声母



//中文:韵母  ********************************************************************************************
//									韵头方式编号	韵尾方式编号	备注
#define FINAL_CN_kong		0		//
#define FINAL_CN_nil		1		//	1	2  含pau，sil  
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
#define FINAL_CN_i  		15		//	5	7  含bi,di,ji,li,mi,ni,pi,qi,ti,xi,yi  
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
#define FINAL_CN_ii  		39		//	2	3  含zi,ci,si  
#define FINAL_CN_iii  		40		//	2	3  含zhi,chi,shi,ri  

//儿化韵母
#define FINAL_CN_air  		41		//	2	5  含(ar,air,anr)：banr,char,far,gair,ganr,hair,kair,lanr,mar,nair,nar,pair,panr,tanr,zair
#define FINAL_CN_angr  		42		//	2	5  含tangr
#define FINAL_CN_aor  		43		//	2	5  含gaor,laor,maor,naor,shaor,zhaor
#define FINAL_CN_engr  		44		//	2	5  含fengr
#define FINAL_CN_iaor  		45		//	5	5  含diaor,piaor,tiaor
#define FINAL_CN_iar  		46		//	5	5  含(iar,ianr,iangr)：bianr,dianr,jianr,lianr,nianr,pianr,qianr,tianr,xianr,xiar,yanr,biangr
#define FINAL_CN_ingr  		47		//	5	5  含dingr
#define FINAL_CN_iour  		48		//	5	5  含liur,niur
#define FINAL_CN_ir  		49		//	5	5  含(ir,inr)：dir,jinr,lir,nir,pir,yir
#define FINAL_CN_our  		50		//	2	5  含(our,ongr,or)：dour,hour,shour,tour,dongr,dor
#define FINAL_CN_uair  		51		//	4	5  含(uar,uair,uanr)：guanr,guar,huair,huar,kuair,wair,wanr
#define FINAL_CN_uangr  	52		//	4	5  含kuangr
#define FINAL_CN_ueir  		53		//	4	5  含(ueir,uir,unr)：duir,dunr,gunr,huir,kunr,shuir,tuir,weir,zhuir,zhunr
#define FINAL_CN_uor  		54		//	4	5  含guor,huor,wor,zuor
#define FINAL_CN_ur  		55		//	4	5  含zhur
#define FINAL_CN_vanr  		56		//	3	5  含juanr,quanr,yuanr
#define FINAL_CN_ver  		57		//	3	5  含juer
#define FINAL_CN_vnr  		58		//	3	5  含(vr,nvr)：qur,xur
#define FINAL_CN_eir  		59		//	2	5  含(eir,enr,iir,iiir)：beir,benr,genr,menr,penr,renr,sheir,shenr,teir,shir,sir
#define FINAL_CN_ier  		60		//	5	5  含bier,dier
#define FINAL_CN_iongr  	61		//	5	5  含xiongr,diongr

//英文字母韵母
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



//中文:LAB列定义  ********************************************************************************************

#define	RLAB_C_SM						0			//	本音节声母编码
#define	RLAB_C_YM						1			//	本音节韵母编码
#define	RLAB_C_TONE						6			//	本音节声调编码
#define	RLAB_L_SM						7			//	前音节声母编码
#define	RLAB_L_YM						8			//	前音节韵母编码
#define	RLAB_L_TONE						13			//	前音节声调编码   6
#define	RLAB_R_SM						14			//	后音节声母编码    7
#define	RLAB_R_YM						15			//	后音节韵母编码  8
#define	RLAB_R_TONE						20			//	后音节声调编码   9
#define	RLAB_C_SM_METHOD				2			//	本音节声母发音方式编码  10
#define	RLAB_C_SM_PLACE					3			//	本音节声母发音部位编码  11
#define	RLAB_C_YM_FRONT					4			//	本音节韵头发音方式编码  12
#define	RLAB_C_YM_REAR					5			//	本音节韵尾发音方式编码  13
#define	RLAB_L_SM_METHOD				9			//	前音节声母发音方式编码  14
#define	RLAB_L_SM_PLACE					10			//	前音节声母发音部位编码  15
#define	RLAB_L_YM_FRONT					11			//	前音节韵头发音方式编码  16
#define	RLAB_L_YM_REAR					12			//	前音节韵尾发音方式编码   17
#define	RLAB_R_SM_METHOD				16			//	后音节声母发音方式编码  18
#define	RLAB_R_SM_PLACE					17			//	后音节声母发音部位编码  19
#define	RLAB_R_YM_FRONT					18			//	后音节韵头发音方式编码  20
#define	RLAB_R_YM_REAR					19			//	后音节韵尾发音方式编码  21

//#define	RLAB_C_SM						0			//	本音节声母编码
//#define	RLAB_C_YM						1			//	本音节韵母编码
//#define	RLAB_C_TONE						2			//	本音节声调编码
//#define	RLAB_L_SM						3			//	前音节声母编码
//#define	RLAB_L_YM						4			//	前音节韵母编码
//#define	RLAB_L_TONE						5			//	前音节声调编码
//#define	RLAB_R_SM						6			//	后音节声母编码
//#define	RLAB_R_YM						7			//	后音节韵母编码
//#define	RLAB_R_TONE						8			//	后音节声调编码
//#define	RLAB_C_SM_METHOD				9			//	本音节声母发音方式编码
//#define	RLAB_C_SM_PLACE					10			//	本音节声母发音部位编码
//#define	RLAB_C_YM_FRONT					11			//	本音节韵头发音方式编码
//#define	RLAB_C_YM_REAR					12			//	本音节韵尾发音方式编码
//#define	RLAB_L_SM_METHOD				13			//	前音节声母发音方式编码
//#define	RLAB_L_SM_PLACE					14			//	前音节声母发音部位编码
//#define	RLAB_L_YM_FRONT					15			//	前音节韵头发音方式编码
//#define	RLAB_L_YM_REAR					16			//	前音节韵尾发音方式编码
//#define	RLAB_R_SM_METHOD				17			//	后音节声母发音方式编码
//#define	RLAB_R_SM_PLACE					18			//	后音节声母发音部位编码
//#define	RLAB_R_YM_FRONT					19			//	后音节韵头发音方式编码
//#define	RLAB_R_YM_REAR					20			//	后音节韵尾发音方式编码
#define	RLAB_SYL_FWD_POS_IN_PW			21			//	PW中当前SYL的正序位置
#define	RLAB_SYL_FWD_POS_IN_PPH			22			//	PP中当前SYL的正序位置
#define	RLAB_SYL_FWD_POS_IN_IP			23			//	IP中当前SYL的正序位置				//
#define	RLAB_SYL_FWD_POS_IN_SEN			24			//	SEN中当前SYL的正序位置
#define	RLAB_PW_FWD_POS_IN_PPH			25			//	PP中当前PW的正序位置
#define	RLAB_PW_FWD_POS_IN_IP			26			//	IP中当前PW中的正序位置				//
#define	RLAB_PW_FWD_POS_IN_SEN			27			//	SEN中当前PW的正序位置
#define	RLAB_PPH_FWD_POS_IN_IP			28			//	IP中当前PP的正序位置				//
#define	RLAB_PPH_FWD_POS_IN_SEN			29			//	SEN中当前PP的正序位置
#define	RLAB_IP_FWD_POS_IN_SEN			30			//	SEN中当前IP的正序位置				//
#define	RLAB_SYL_BWD_POS_IN_PW			31			//	PW中当前SYL的反序位置
#define	RLAB_SYL_BWD_POS_IN_PPH			32			//	PP中当前SYL的反序位置
#define	RLAB_SYL_BWD_POS_IN_IP			33			//	IP中当前SYL的反序位置				//
#define	RLAB_SYL_BWD_POS_IN_SEN			34			//	SEN中当前SYL的反序位置
#define	RLAB_PW_BWD_POS_IN_PPH			35			//	PP中当前PW的反序位置
#define	RLAB_PW_BWD_POS_IN_IP			36			//	IP中当前PW中的反序位置				//
#define	RLAB_PW_BWD_POS_IN_SEN			37			//	SEN中当前PW的反序位置
#define	RLAB_PPH_BWD_POS_IN_IP			38			//	IP中当前PP的反序位置				//
#define	RLAB_PPH_BWD_POS_IN_SEN			39			//	SEN中当前PP的反序位置
#define	RLAB_IP_BWD_POS_IN_SEN			40			//	SEN中当前IP的反序位置				//
#define	RLAB_SYL_NUM_IN_C_PW			41			//	当前PW中SYL的数目
#define	RLAB_SYL_NUM_IN_C_PPH			42			//	当前PP中SYL的数目
#define	RLAB_SYL_NUM_IN_C_IP			43			//	当前IP中SYL的数目				//
#define	RLAB_SYL_NUM_IN_C_SEN			44			//	当前SEN中SYL的数目
#define	RLAB_PW_NUM_IN_C_PPH			45			//	当前PP中PW的数目
#define	RLAB_PW_NUM_IN_C_IP				46			//	当前IP中PW中的数目				//
#define	RLAB_PW_NUM_IN_C_SEN			47			//	当前SEN中PW的数目
#define	RLAB_PPH_NUM_IN_C_IP			48			//	当前IP中PP的数目				//
#define	RLAB_PPH_NUM_IN_C_SEN			49			//	当前SEN中PP的数目
#define	RLAB_IP_NUM_IN_C_SEN			50			//	当前SEN中IP的数目				//
#define	RLAB_SYL_NUM_IN_L_PW			51			//	前一PW中SYL的数目
#define	RLAB_SYL_NUM_IN_L_PPH			52			//	前一PP中SYL的数目
#define	RLAB_SYL_NUM_IN_L_IP			53			//	前一IP中SYL的数目				//
#define	RLAB_PW_NUM_IN_L_PPH			54			//	前一PP中PW的数目
#define	RLAB_PW_NUM_IN_L_IP				55			//	前一IP中PW的数目				//
#define	RLAB_PPH_NUM_IN_L_IP			56			//	前一IP中PP的数目				//
#define	RLAB_SYL_NUM_IN_R_PW			57			//	后一PW中SYL的数目
#define	RLAB_SYL_NUM_IN_R_PPH			58			//	后一PP中SYL的数目
#define	RLAB_SYL_NUM_IN_R_IP			59			//	后一IP中SYL的数目				//
#define	RLAB_PW_NUM_IN_R_PPH			60			//	后一PP中PW的数目
#define	RLAB_PW_NUM_IN_R_IP				61			//	后一IP中PW的数目				//
#define	RLAB_PPH_NUM_IN_R_IP			62			//	后一IP中PP的数目				//


//英文:LAB列定义  ********************************************************************************************
#define	ELAB_C01_C_pp							0			//本PP编码
#define	ELAB_C02_L_pp							1			//前PP编码
#define	ELAB_C03_R_pp							2			//后PP编码
#define	ELAB_C04_LL_pp							3			//前前PP编码
#define	ELAB_C05_RR_pp							4			//后后PP编码
#define	ELAB_D01_C_stress						5			//当前SYL的重音类型
#define	ELAB_D02_L_stress						6			//前一SYL的重音类型
#define	ELAB_D03_R_stress						7			//后一SYL的重音类型
#define	ELAB_D04_LL_stress						8			//前前SYL的重音类型
#define	ELAB_D05_RR_stress						9			//后后SYL的重音类型
#define	ELAB_E01_L_StrNumInWord					10			//本WORD中前stress音节与本音节的距离
#define	ELAB_E02_R_StrNumInWord					11			//本WORD中后stress音节与本音节的距离
#define	ELAB_F01_L_Border_Of_Syl				12			//本SYL的左边界（<=）
#define	ELAB_F02_R_Border_Of_Syl				13			//本SYL的右边界（<=）
#define	ELAB_G01_L_Border_Of_Word				14			//本WORD的左边界（<=）
#define	ELAB_G02_R_Border_Of_Word				15			//本WORD的右边界（<=）
#define	ELAB_H01_RelPos_SylInWord				16			//本WORD中的SYL相对位置
#define	ELAB_H02_RelPos_SylInPph				17			//本PPH中的SYL相对位置
#define	ELAB_H03_RelPos_WordInPph				18			//本PPH中的WORD相对位置
#define	ELAB_H04_RelPos_WordInSen				19			//本SEN中的WORD相对位置
#define	ELAB_H05_RelPos_PphInSen				20			//本SEN中的PPH相对位置
#define	ELAB_I01_FwdPos_SylInWord				21			//WORD中当前SYL的正序位置
#define	ELAB_I02_FwdPos_SylInPph				22			//PPH中当前SYL的正序位置
#define	ELAB_I03_FwdPos_WordInPph				23			//PPH中当前WORD的正序位置
#define	ELAB_I04_FwdPos_WordInSen				24			//SEN中当前WORD的正序位置
#define	ELAB_I05_FwdPos_PphInSen				25			//SEN中当前PPH的正序位置
#define	ELAB_J01_BwdPos_SylInWord				26			//WORD中当前SYL的反序位置
#define	ELAB_J02_BwdPos_SylInPph				27			//PPH中当前SYL的反序位置
#define	ELAB_J03_BwdPos_WordInPph				28			//PPH中当前WORD的反序位置
#define	ELAB_J04_BwdPos_WordInSen				29			//SEN中当前WORD的反序位置
#define	ELAB_J05_BwdPos_PphInSen				30			//SEN中当前PPH的反序位置
#define	ELAB_K01_PpNum_InCSyl					31			//当前SYL中音素的数目
#define	ELAB_K02_SylNum_InCWord					32			//当前WORD中SYL的数目
#define	ELAB_K03_SylNum_InCPph					33			//当前PPH中SYL的数目
#define	ELAB_K04_WordNum_InCPph					34			//当前PPH中WORD的数目
#define	ELAB_K05_WordNum_InCSen					35			//当前SEN中WORD的数目
#define	ELAB_K06_PphNum_InCSen					36			//当前SEN中PPH的数目
#define	ELAB_K07_PpNum_InLSyl					37			//前一SYL中音素的数目
#define	ELAB_K08_SylNum_InLWord					38			//前一WORD中SYL的数目
#define	ELAB_K09_WordNum_InLPph					39			//前一PPH中WORD的数目
#define	ELAB_K10_PpNum_InRSyl					40			//后一SYL中音素的数目
#define	ELAB_K11_SylNum_InRWord					41			//后一WORD中SYL的数目
#define	ELAB_K12_WordNum_InRPph					42			//后一PPH中WORD的数目
#define	ELAB_L01_PpToLVow_NumInWord				43			//本单词内本音素与前元音的距离
#define	ELAB_L02_PpToRVow_NumInWord				44			//本单词内本音素与后元音的距离
#define	ELAB_U01_C_Syl_Accent					45			//本SYL的accent音节重音类型
#define	ELAB_U02_C_Word_Accent					46			//本word中是否含有重读音节
#define	ELAB_V01_AccentNum_BeforeInPph			47			//本PPH中本SYL前的accent音节数
#define	ELAB_V02_AccentNum_AfterInPph			48			//本PPH中本SYL后的accent音节数
#define	ELAB_V03_LAccent_SylNumInPph			49			//本PPH中前accent音节与本SYL的距离
#define	ELAB_V04_RAccent_SylNumInPph			50			//本PPH中后accent音节与本SYL的距离
#define	ELAB_V05_LAccent_WordNumInPph			51			//本PPH中本word与前accent单词的距离
#define	ELAB_V06_RAccent_WordNumInPph			52			//本PPH中本word与后accent单词的距离
#define	ELAB_W01_C_PPH_Tone						53			//本PPH的句调
#define	ELAB_W02_L_PPH_Tone						54			//前PPH的句调
#define	ELAB_W03_R_PPH_Tone						55			//后PPH的句调
#define	ELAB_W04_LL_PPH_Tone					56			//前前PPH的句调
#define	ELAB_W05_RR_PPH_Tone					57			//后后PPH的句调
#define	ELAB_W06_C_pp							58			//本音素是否是异常音
#define	ELAB_B01_C_T0							59			//本pp类型0编码
#define	ELAB_B01_C_T1							60			//本pp类型1编码
#define	ELAB_B01_C_T2							61			//本pp类型2编码
#define	ELAB_B01_C_T3							62			//本pp类型3编码
#define	ELAB_B01_C_T4							63			//本pp类型4编码
#define	ELAB_B01_C_T5							64			//本pp类型5编码
#define	ELAB_B01_C_T6							65			//本pp类型6编码
#define	ELAB_B01_C_T7							66			//本pp类型7编码
#define	ELAB_B02_L_T0							67			//前pp类型0编码
#define	ELAB_B02_L_T1							68			//前pp类型1编码
#define	ELAB_B02_L_T2							69			//前pp类型2编码
#define	ELAB_B02_L_T3							70			//前pp类型3编码
#define	ELAB_B02_L_T4							71			//前pp类型4编码
#define	ELAB_B02_L_T5							72			//前pp类型5编码
#define	ELAB_B02_L_T6							73			//前pp类型6编码
#define	ELAB_B02_L_T7							74			//前pp类型7编码
#define	ELAB_B03_R_T0							75			//后pp类型0编码
#define	ELAB_B03_R_T1							76			//后pp类型1编码
#define	ELAB_B03_R_T2							77			//后pp类型2编码
#define	ELAB_B03_R_T3							78			//后pp类型3编码
#define	ELAB_B03_R_T4							79			//后pp类型4编码
#define	ELAB_B03_R_T5							80			//后pp类型5编码
#define	ELAB_B03_R_T6							81			//后pp类型6编码
#define	ELAB_B03_R_T7							82			//后pp类型7编码
#define	ELAB_B04_LL_T0							83			//前前pp类型0编码
#define	ELAB_B04_LL_T1							84			//前前pp类型1编码
#define	ELAB_B04_LL_T2							85			//前前pp类型2编码
#define	ELAB_B04_LL_T3							86			//前前pp类型3编码
#define	ELAB_B04_LL_T4							87			//前前pp类型4编码
#define	ELAB_B04_LL_T5							88			//前前pp类型5编码
#define	ELAB_B04_LL_T6							89			//前前pp类型6编码
#define	ELAB_B04_LL_T7							90			//前前pp类型7编码
#define	ELAB_B05_RR_T0							91			//后后pp类型0编码
#define	ELAB_B05_RR_T1							92			//后后pp类型1编码
#define	ELAB_B05_RR_T2							93			//后后pp类型2编码
#define	ELAB_B05_RR_T3							94			//后后pp类型3编码
#define	ELAB_B05_RR_T4							95			//后后pp类型4编码
#define	ELAB_B05_RR_T5							96			//后后pp类型5编码
#define	ELAB_B05_RR_T6							97			//后后pp类型6编码
#define	ELAB_B05_RR_T7							98			//后后pp类型7编码

//英文:音素编码定义  ********************************************************************************************
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


//本单词内本音素与后元音的距离----判断是否是单词尾部
#define ELAB_PpToRVow_NumInWord_x   255        //x表示音素后无元音
#define ELAB_PpToLVow_NumInWord_x   255        //x表示音素前无元音



	#define E_RLAB_PH_NUM_IN_C_SYL			31			//当前SYL中音素的数目
	#define E_RLAB_SYL_NUM_IN_C_PW			32			//当前PW中SYL的数目
	#define E_RLAB_R_BORDER_IN_WORD			15			//当前单词的右边界




#ifdef __cplusplus
}
#endif

#endif /* !_EMTTS__LIST_REAR_LAB__HXX_ */	