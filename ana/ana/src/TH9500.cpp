/*
 *  Th9500.cpp
 *  
 *
 *  Created by Matteo Turisini on 06/09/11.
 *  Copyright 2011 Universita e INFN, Roma I, ISS (IT). All rights reserved.
 *
 */

#include "Th9500.h"

Th9500::Th9500(){
	fWidth		= 52.00;
	fPixelSize	=  3.04;
	fFrameSize	=  4.72;
	
	fNPixel		=  256;
	
	for(int i=0;i<fNPixel;i++){	
		fAnode[i]=i+1;
		fXc[i] = Anode2LocalX(fAnode[i]);
		fYc[i] = Anode2LocalY(fAnode[i]);
	}
	fXVertex[0]= -0.5*fWidth;
	fXVertex[1]= +0.5*fWidth;
	fXVertex[2]= +0.5*fWidth;
	fXVertex[3]= -0.5*fWidth;
	
	fYVertex[0]= -0.5*fWidth;
	fYVertex[1]= -0.5*fWidth;
	fYVertex[2]= +0.5*fWidth;
	fYVertex[3]= +0.5*fWidth;
};



void Th9500::Print(){

	printf("/**********************************/\n");
	printf("/*        PMT MODEL: H9500        */\n");
	printf("/**********************************/\n");
	printf("INDEX:   ANODE     X[mm]       Y[mm]\n");
	int i;
	for(i=0;i<fNPixel;i++){	
		printf(" %2d       %2d     %7.3f     %7.3f \n",i,fAnode[i],fXc[i],fYc[i]);
	}
	printf("VERTEX: \n");
	for(i=0;i<4;i++){
		printf("  %d              %7.3f     %7.3f\n",i+1,fXVertex[i],fYVertex[i]); 
	}
	printf("/**********************************/\n");
};



float Th9500::Anode2LocalY(short anode){
	float y = 0.0;
	
	if(anode<1||anode>fNPixel) {printf("Error in Th9500::Anode2Ycenter argument: anode index out of range [1..256] you have %d\n",anode);return -1.0;}
	
	if(anode==  1||anode==  2||anode==  3||anode==  4||anode==  5||anode==  6||anode==  7||anode==  8||anode==  9||anode== 10||anode== 11||anode== 12||anode== 13||anode== 14||anode== 15||anode== 16){y=+7.0*fPixelSize+0.5*fFrameSize;}
	if(anode== 17||anode== 18||anode== 19||anode== 20||anode== 21||anode== 22||anode== 23||anode== 24||anode== 25||anode== 26||anode== 27||anode== 28||anode== 29||anode== 30||anode== 31||anode== 32){y=+6.5*fPixelSize;}
	if(anode== 33||anode== 34||anode== 35||anode== 36||anode== 37||anode== 38||anode== 39||anode== 40||anode== 41||anode== 42||anode== 43||anode== 44||anode== 45||anode== 46||anode== 47||anode== 48){y=+5.5*fPixelSize;}
	if(anode== 49||anode== 50||anode== 51||anode== 52||anode== 53||anode== 54||anode== 55||anode== 56||anode== 57||anode== 58||anode== 59||anode== 60||anode== 61||anode== 62||anode== 63||anode== 64){y=+4.5*fPixelSize;}
	if(anode== 65||anode== 66||anode== 67||anode== 68||anode== 69||anode== 70||anode== 71||anode== 72||anode== 73||anode== 74||anode== 75||anode== 76||anode== 77||anode== 78||anode== 79||anode== 80){y=+3.5*fPixelSize;}
	if(anode== 81||anode== 82||anode== 83||anode== 84||anode== 85||anode== 86||anode== 87||anode== 88||anode== 89||anode== 90||anode== 91||anode== 92||anode== 93||anode== 94||anode== 95||anode== 96){y=+2.5*fPixelSize;}
	if(anode== 97||anode== 98||anode== 99||anode==100||anode==101||anode==102||anode==103||anode==104||anode==105||anode==106||anode==107||anode==108||anode==109||anode==110||anode==111||anode==112){y=+1.5*fPixelSize;}
	if(anode==113||anode==114||anode==115||anode==116||anode==117||anode==118||anode==119||anode==120||anode==121||anode==122||anode==123||anode==124||anode==125||anode==126||anode==127||anode==128){y=+0.5*fPixelSize;}
	if(anode==129||anode==130||anode==131||anode==132||anode==133||anode==134||anode==135||anode==136||anode==137||anode==138||anode==139||anode==140||anode==141||anode==142||anode==143||anode==144){y=-0.5*fPixelSize;}
	if(anode==145||anode==146||anode==147||anode==148||anode==149||anode==150||anode==151||anode==152||anode==153||anode==154||anode==155||anode==156||anode==157||anode==158||anode==159||anode==160){y=-1.5*fPixelSize;}
	if(anode==161||anode==162||anode==163||anode==164||anode==165||anode==166||anode==167||anode==168||anode==169||anode==170||anode==171||anode==172||anode==173||anode==174||anode==175||anode==176){y=-2.5*fPixelSize;}
	if(anode==177||anode==178||anode==179||anode==180||anode==181||anode==182||anode==183||anode==184||anode==185||anode==186||anode==187||anode==188||anode==189||anode==190||anode==191||anode==192){y=-3.5*fPixelSize;}
	if(anode==193||anode==194||anode==195||anode==196||anode==197||anode==198||anode==199||anode==200||anode==201||anode==202||anode==203||anode==204||anode==205||anode==206||anode==207||anode==208){y=-4.5*fPixelSize;}
	if(anode==209||anode==210||anode==211||anode==212||anode==213||anode==214||anode==215||anode==216||anode==217||anode==218||anode==219||anode==220||anode==221||anode==222||anode==223||anode==224){y=-5.5*fPixelSize;}
	if(anode==225||anode==226||anode==227||anode==228||anode==229||anode==230||anode==231||anode==232||anode==233||anode==234||anode==235||anode==236||anode==237||anode==238||anode==239||anode==240){y=-6.5*fPixelSize;}
	if(anode==241||anode==242||anode==243||anode==244||anode==245||anode==246||anode==247||anode==248||anode==249||anode==250||anode==251||anode==252||anode==253||anode==254||anode==255||anode==256){y=-7.0*fPixelSize-0.5*fFrameSize;}

	return y;
};


float Th9500::Anode2LocalX(short anode){ 
	float x = 0.0;
	
	if(anode<1||anode>fNPixel) {printf("Error in Th9500::Anode2Xcenter argument: anode index out of range [1..256] you have %d\n",anode);return -1.0;}

	if(anode== 16||anode== 32||anode== 48||anode== 64||anode== 80||anode== 96||anode==112||anode==128||anode==144||anode==160||anode==176||anode==192||anode==208||anode==224||anode==240||anode==256){x=+7.0*fPixelSize+0.5*fFrameSize;}
	if(anode== 15||anode== 31||anode== 47||anode== 63||anode== 79||anode== 95||anode==111||anode==127||anode==143||anode==159||anode==175||anode==191||anode==207||anode==223||anode==239||anode==255){x=+6.5*fPixelSize;}
	if(anode== 14||anode== 30||anode== 46||anode== 62||anode== 78||anode== 94||anode==110||anode==126||anode==142||anode==158||anode==174||anode==190||anode==206||anode==222||anode==238||anode==254){x=+5.5*fPixelSize;}
	if(anode== 13||anode== 29||anode== 45||anode== 61||anode== 77||anode== 93||anode==109||anode==125||anode==141||anode==157||anode==173||anode==189||anode==205||anode==221||anode==237||anode==253){x=+4.5*fPixelSize;}
	if(anode== 12||anode== 28||anode== 44||anode== 60||anode== 76||anode== 92||anode==108||anode==124||anode==140||anode==156||anode==172||anode==188||anode==204||anode==220||anode==236||anode==252){x=+3.5*fPixelSize;}
	if(anode== 11||anode== 27||anode== 43||anode== 59||anode== 75||anode== 91||anode==107||anode==123||anode==139||anode==155||anode==171||anode==187||anode==203||anode==219||anode==235||anode==251){x=+2.5*fPixelSize;}
	if(anode== 10||anode== 26||anode== 42||anode== 58||anode== 74||anode== 90||anode==106||anode==122||anode==138||anode==154||anode==170||anode==186||anode==202||anode==218||anode==234||anode==250){x=+1.5*fPixelSize;}
	if(anode==  9||anode== 25||anode== 41||anode== 57||anode== 73||anode== 89||anode==105||anode==121||anode==137||anode==153||anode==169||anode==185||anode==201||anode==217||anode==233||anode==249){x=+0.5*fPixelSize;}
	if(anode==  8||anode== 24||anode== 40||anode== 56||anode== 72||anode== 88||anode==104||anode==120||anode==136||anode==152||anode==168||anode==184||anode==200||anode==216||anode==232||anode==248){x=-0.5*fPixelSize;}
	if(anode==  7||anode== 23||anode== 39||anode== 55||anode== 71||anode== 87||anode==103||anode==119||anode==135||anode==151||anode==167||anode==183||anode==199||anode==215||anode==231||anode==247){x=-1.5*fPixelSize;}
	if(anode==  6||anode== 22||anode== 38||anode== 54||anode== 70||anode== 86||anode==102||anode==118||anode==134||anode==150||anode==166||anode==182||anode==198||anode==214||anode==230||anode==246){x=-2.5*fPixelSize;}
	if(anode==  5||anode== 21||anode== 37||anode== 53||anode== 69||anode== 85||anode==101||anode==117||anode==133||anode==149||anode==165||anode==181||anode==197||anode==213||anode==229||anode==245){x=-3.5*fPixelSize;}
	if(anode==  4||anode== 20||anode== 36||anode== 52||anode== 68||anode== 84||anode==100||anode==116||anode==132||anode==148||anode==164||anode==180||anode==196||anode==212||anode==228||anode==244){x=-4.5*fPixelSize;}
	if(anode==  3||anode== 19||anode== 35||anode== 51||anode== 67||anode== 83||anode== 99||anode==115||anode==131||anode==147||anode==163||anode==179||anode==195||anode==211||anode==227||anode==243){x=-5.5*fPixelSize;}
	if(anode==  2||anode== 18||anode== 34||anode== 50||anode== 66||anode== 82||anode== 98||anode==114||anode==130||anode==146||anode==162||anode==178||anode==194||anode==210||anode==226||anode==242){x=-6.5*fPixelSize;}
	if(anode==  1||anode== 17||anode== 33||anode== 49||anode== 65||anode== 81||anode== 97||anode==113||anode==129||anode==145||anode==161||anode==177||anode==193||anode==209||anode==225||anode==241){x=-7.0*fPixelSize-0.5*fFrameSize;}

	
	return x;
};




float Th9500::GetX(short anode){
	if (anode<1||anode>fNPixel) {
		printf("Error in Th9500::GetX arg: index out of range[1..256], you have %d\n",anode); return -1.0;
	}
	float x=0.0;
	x= fXc[anode-1];
	return x;
};


float Th9500::GetY(short anode){
	if (anode<1||anode>fNPixel) {
		printf("Error in Th9500::GetY arg: index out of range[1..256], you have %d\n ",anode); return -1.0;
	}
	float y=0.0;
	y= fYc[anode-1];
	return y;
};


void Th9500::Translate(float X0, float Y0){
	int i=0;
	for(i=0;i<fNPixel;i++){
		fXc[i]=fXc[i]+X0; 
		fYc[i]=fYc[i]+Y0;
		if(i<4){
			fXVertex[i]=fXVertex[i]+X0;
			fYVertex[i]=fYVertex[i]+Y0;
		}
	}
}

void Th9500::SetFe(char fe1,char fe2,char fe3,char fe4){
	fFe[0]=fe1;	
	fFe[1]=fe2;	
	fFe[2]=fe3;	
	fFe[3]=fe4;	
};
void Th9500::SetGeo(char geo1,char geo2,char geo3,char geo4){
	fGeo[0]=geo1;
	fGeo[1]=geo2;
	fGeo[2]=geo3;
	fGeo[3]=geo4;
};



void Th9500::Rotate(float angleindegrees){
	int i=0;
	double Xnew;
	double Ynew;
	double angleinradiants = (double)(PIGRECO*2*(angleindegrees/360.0));
	
	//printf("pixel map rotated by  %f or %f\n",angleindegrees,angleinradiants);
	
	for(i=0;i<fNPixel;i++){
		
		Xnew = 0.0;
		Ynew = 0.0;
		
		Xnew = fXc[i]*cos(angleinradiants) - fYc[i]*sin(angleinradiants);
		Ynew = fXc[i]*sin(angleinradiants) + fYc[i]*cos(angleinradiants);
		
		
		//	printf("%d : %f -> %f  ||   %f -> %f \n",i+1,Xc[i],Xnew,Yc[i],Ynew);
		fXc[i]=(float)Xnew;
		fYc[i]=(float)Ynew;
		
		Xnew = 0.0;
		Ynew = 0.0;
		if(i<4){
			Xnew = fXVertex[i]*cos(angleinradiants) - fYVertex[i]*sin(angleinradiants);
			Ynew = fXVertex[i]*sin(angleinradiants) + fYVertex[i]*cos(angleinradiants);
			
			fXVertex[i]=(float)Xnew;
			fYVertex[i]=(float)Ynew;
		}
	}
};

