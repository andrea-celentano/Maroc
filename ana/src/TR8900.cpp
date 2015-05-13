/*
 *  Tr8900.cpp
 *  
 *
 *  Created by Matteo Turisini on 08/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

/*
 * This class handle Geometrical Data of a R8900 MAPMT.
 * The costructor generates the local coordinates of 16 anodes
 * and of the 4 vertex of the object(dimensionale outline).
 * It is possible to Rotate and Traslate the PMT
 * as a rigid body obtaining anode's coordinates in 
 * the laboratory reference system.
 * 
 *
 */

#include "TR8900.h"

Tr8900::Tr8900(){
	NPixel		= 16;
	PixelSize	= 6.00;
	Septal		= 0.40;
	Width		= 26.0;	

	for(int i=0;i<NPixel;i++){	
		Anode[i]= i+1;
		   Xc[i]= Anode2LocalX(Anode[i]);
		   Yc[i] = Anode2LocalY(Anode[i]);
	}
	XVertex[0]= -0.5*Width;
	XVertex[1]= +0.5*Width;
	XVertex[2]= +0.5*Width;
	XVertex[3]= -0.5*Width;
	
	YVertex[0]= -0.5*Width;
	YVertex[1]= -0.5*Width;
	YVertex[2]= +0.5*Width;
	YVertex[3]= +0.5*Width;
};

void Tr8900::Print(){
	printf("/**********************************/\n");
	printf("/*        PMT MODEL: R8900        */\n");
	printf("/**********************************/\n");
	printf("INDEX:   ANODE     X[mm]       Y[mm]\n");
	int i;
	for(i=0;i<NPixel;i++){	
		printf(" %2d       %2d     %7.3f     %7.3f \n",i,Anode[i],Xc[i],Yc[i]);
	}
	printf("VERTEX: \n");
	for(i=0;i<4;i++){
		printf("  %d              %7.3f     %7.3f\n",i+1,XVertex[i],YVertex[i]); 
	}
	printf("/**********************************/\n");
};

float Tr8900::Anode2LocalY(int anode){
	float y = 0.0;
	
	if(anode<1||anode>NPixel) {printf("Error in Anode2Ycenter argument: anode index out of range [1..16] you have %d\n",anode);return -1.0;}

	if(anode== 1||anode== 5||anode== 9||anode==13){y=+1.5*Septal+1.5*PixelSize;}
	if(anode== 2||anode== 6||anode==10||anode==14){y=+0.5*Septal+0.5*PixelSize;}	
	if(anode== 3||anode== 7||anode==11||anode==15){y=-0.5*Septal-0.5*PixelSize;}	
	if(anode== 4||anode== 8||anode==12||anode==16){y=-1.5*Septal-1.5*PixelSize;}
	
	// 1, 6,11,16 RIMANGONO FISSI nella trasposizione
	
	// quella che segue e' la mappa 2011
//	if(anode== 1||anode== 2||anode== 3||anode== 4){y=+1.5*Septal+1.5*PixelSize;}
//	if(anode== 5||anode== 6||anode== 7||anode== 8){y=+0.5*Septal+0.5*PixelSize;}	
//	if(anode== 9||anode==10||anode==11||anode==12){y=-0.5*Septal-0.5*PixelSize;}	
//	if(anode==13||anode==14||anode==15||anode==16){y=-1.5*Septal-1.5*PixelSize;}	

	return y;
};


float Tr8900::Anode2LocalX(int anode){ 
	float x = 0.0;
	
	if(anode==13||anode==14||anode==15||anode==16){x=+1.5*Septal+1.5*PixelSize;}
	if(anode== 9||anode==10||anode==11||anode==12){x=+0.5*Septal+0.5*PixelSize;}	
	if(anode== 5||anode== 6||anode== 7||anode== 8){x=-0.5*Septal-0.5*PixelSize;}	
	if(anode== 1||anode== 2||anode== 3||anode== 4){x=-1.5*Septal-1.5*PixelSize;}
	
	
//	if(anode== 4||anode== 8||anode==12||anode==16){x=+1.5*Septal+1.5*PixelSize;}
//	if(anode== 3||anode== 7||anode==11||anode==15){x=+0.5*Septal+0.5*PixelSize;}	
//	if(anode== 2||anode== 6||anode==10||anode==14){x=-0.5*Septal-0.5*PixelSize;}	
//	if(anode== 1||anode== 5||anode== 9||anode==13){x=-1.5*Septal-1.5*PixelSize;}
	
	return x;
};

float Tr8900::GetX(int i){
	if (i<1||i>NPixel) {
		printf("Error in Tr8900::GetX arg: index out of range[1..16], you have %d",i); return -1.0;
	}
	float x=0.0;
	x= Xc[i-1];
	return x;
};
float Tr8900::GetY(int i){
	if (i<1||i>NPixel) {
		printf("Error in Tr8900::GetY arg: index out of range[1..16], you have %d",i); return -1.0;
	}
	float y =0.0;
	y = Yc[i-1];
	return y;
};



void Tr8900::Translate(float X0, float Y0){
	int i=0;
	for(i=0;i<NPixel;i++){
		Xc[i]=Xc[i]+X0; 
		Yc[i]=Yc[i]+Y0;
		if(i<4){
			XVertex[i]=XVertex[i]+X0;
			YVertex[i]=YVertex[i]+Y0;
		}
	}
}

void Tr8900::Rotate(float angleindegrees){
	int i=0;
	double Xnew;
	double Ynew;
	double angleinradiants = (double)(PIGRECO*2*(angleindegrees/360.0));
	
	//printf("pixel map rotated by  %f or %f\n",angleindegrees,angleinradiants);
	
	for(i=0;i<NPixel;i++){
		
		Xnew = 0.0;
		Ynew = 0.0;
		
		Xnew = Xc[i]*cos(angleinradiants) - Yc[i]*sin(angleinradiants);
		Ynew = Xc[i]*sin(angleinradiants) + Yc[i]*cos(angleinradiants);
		
		
		//	printf("%d : %f -> %f  ||   %f -> %f \n",i+1,Xc[i],Xnew,Yc[i],Ynew);
		Xc[i]=(float)Xnew;
		Yc[i]=(float)Ynew;
		
		Xnew = 0.0;
		Ynew = 0.0;
		if(i<4){
			Xnew = XVertex[i]*cos(angleinradiants) - YVertex[i]*sin(angleinradiants);
			Ynew = XVertex[i]*sin(angleinradiants) + YVertex[i]*cos(angleinradiants);
			
			XVertex[i]=(float)Xnew;
			YVertex[i]=(float)Ynew;
		}
	}
}

/*
void Tr8900::Plot(){
	
	//TCanvas*  mycanv = new TCanvas("mycanv","Pixel Location H8500"); 		
	TGraph * g = new TGraph(NPixel,Xc,Yc);
	g->SetMarkerColor(2);
	g->SetMarkerSize(1.5);
	g->SetMarkerStyle(21);
	g->Draw("AP");
	
	TMarker * mark1 = new TMarker((double)GetX(1) ,(double)GetY(1),20);
	mark1->Draw("SAME");
	
}
*/

/*
void Tr8900::Paint(){
	
	
	// The top lefthand corner of the window is at pixel coordinates 10,10. The width is 900 pixels and height 900
	TCanvas*  canv = new TCanvas("canv","Outline of H8500",10,10,900,900);
	// User cordinate definition (x1,y1,x2,y2)
	float A= 300.0;
	canv->Range(-A,-A,A,A);
	
	
	TGaxis *axis1 = new TGaxis(-.85*A,-.85*A,-.85*A,.85*A,-A,A,510,"");
	axis1->SetName("axis1");
	axis1->Draw();
	
	TGaxis *axis2 = new TGaxis(-.85*A,-.85*A,.85*A,-.85*A,-A,A,510,"");
	axis2->SetName("axis2");
	axis2->Draw();
	
	Double_t x[5] = {XVertex[0],XVertex[1],XVertex[2],XVertex[3],XVertex[0]};
	Double_t y[5] = {YVertex[0],YVertex[1],YVertex[2],YVertex[3],YVertex[0]};
	TPolyLine *pline = new TPolyLine(5,x,y);
	
	pline->SetFillColor(38);
	pline->SetLineColor(2);
	pline->SetLineWidth(4);
	pline->Draw("");
	
	
	TMarker * mark1 = new TMarker((double)GetX(1) ,(double)GetY(1),20);
	mark1->Draw("SAME");
}
 */
//------------------------
//1
// ANODE PIN
//1		70
//2		66
//3		72
//4		68
//5		71
//6		67
//7		69
//8		65
//9		62
//10	58
//11	64
//12	60
//13	63
//14	59
//15	61
//16	57
//------------------------
//2
//ANODE PIN
//1		54
//2		50
//3		56
//4		52
//5		55
//6		51
//7		53
//8		49
//9		46	
//10	42
//11	48
//12	44
//13	47
//14	43
//15	45
//16	41
//------------------------
//3
//ANODE PIN
//1		38
//2		34
//3		40
//4		36
//5		39
//6		35
//7		37
//8		33
//9		30
//10	26
//11	32
//12    28
//13	31
//14	27
//15	29
//16	25
//------------------------
//4
//ANODE PIN
//1		22
//2		18
//3		24
//4		20
//5		23
//6		19
//7		21
//8		17
//9		14
//10	10
//11	16
//12	12
//13	15
//14	11
//15	13
//16	9



