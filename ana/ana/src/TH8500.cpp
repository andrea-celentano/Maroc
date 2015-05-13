
#include "Th8500.h"

Th8500::Th8500(){
	NPixel = 64;
	PixelSize = 6.08;
	FrameSize = 6.26;
	Width = 52.0;	
	int i;
	for(i=0;i<NPixel;i++){	
		Anode[i]=i+1;
		Xc[i] = Anode2LocalX(Anode[i]);
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


void Th8500::Print(){

	printf("/**********************************/\n");
	printf("/*        PMT MODEL: H8500        */\n");
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




float Th8500::Anode2LocalY(int anode){
	float y = 0.0;

	if(anode<1||anode>NPixel) {printf("Error in Anode2Ycenter argument: anode index out of range [1..64] you have %d\n",anode);return -1.0;}

	if(anode== 1||anode== 2||anode== 3||anode== 4||anode== 5||anode== 6||anode== 7||anode== 8){y=+3.0*PixelSize+0.5*FrameSize;}
	if(anode== 9||anode==10||anode==11||anode==12||anode==13||anode==14||anode==15||anode==16){y=+2.5*PixelSize;}
	if(anode==17||anode==18||anode==19||anode==20||anode==21||anode==22||anode==23||anode==24){y=+1.5*PixelSize;}
	if(anode==25||anode==26||anode==27||anode==28||anode==29||anode==30||anode==31||anode==32){y=+0.5*PixelSize;}
	if(anode==33||anode==34||anode==35||anode==36||anode==37||anode==38||anode==39||anode==40){y=-0.5*PixelSize;}
	if(anode==41||anode==42||anode==43||anode==44||anode==45||anode==46||anode==47||anode==48){y=-1.5*PixelSize;}
	if(anode==49||anode==50||anode==51||anode==52||anode==53||anode==54||anode==55||anode==56){y=-2.5*PixelSize;}
	if(anode==57||anode==58||anode==59||anode==60||anode==61||anode==62||anode==63||anode==64){y=-3.0*PixelSize-0.5*FrameSize;}
	return y;
};


float Th8500::Anode2LocalX(int anode){ 
	float x = 0.0;

	if(anode<1||anode>NPixel) {printf("Error in XcoordinateH8500 argument: anode index out of range [1..64] you have %d\n",anode);return -1.0;}

	if(anode==8||anode==16||anode==24||anode==32||anode==40||anode==48||anode==56||anode==64){x=+3.0*PixelSize+0.5*FrameSize;}
	if(anode==7||anode==15||anode==23||anode==31||anode==39||anode==47||anode==55||anode==63){x=+2.5*PixelSize;}
	if(anode==6||anode==14||anode==22||anode==30||anode==38||anode==46||anode==54||anode==62){x=+1.5*PixelSize;}
	if(anode==5||anode==13||anode==21||anode==29||anode==37||anode==45||anode==53||anode==61){x=+0.5*PixelSize;}
	if(anode==4||anode==12||anode==20||anode==28||anode==36||anode==44||anode==52||anode==60){x=-0.5*PixelSize;}
	if(anode==3||anode==11||anode==19||anode==27||anode==35||anode==43||anode==51||anode==59){x=-1.5*PixelSize;}
	if(anode==2||anode==10||anode==18||anode==26||anode==34||anode==42||anode==50||anode==58){x=-2.5*PixelSize;}
	if(anode==1||anode== 9||anode==17||anode==25||anode==33||anode==41||anode==49||anode==57){x=-3.0*PixelSize-0.5*FrameSize;}
	return x;
};


float Th8500::GetX(int i){
	if (i<1||i>NPixel) {
		printf("Error in Th8500::GetX arg: index out of range[1..63], you have %d",i); return -1.0;
	}
	float x=0.0;
	x= Xc[i-1];
	return x;
};
float Th8500::GetY(int i){
	if (i<1||i>NPixel) {
		printf("Error in Th8500::GetY arg: index out of range[1..63], you have %d",i); return -1.0;
	}
	float y =0.0;
	y = Yc[i-1];
	return y;
};


void Th8500::Translate(float X0, float Y0){
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

void Th8500::Rotate(float angleindegrees){
	int i=0;
	double Xnew;
	double Ynew;
	double angleinradiants = (double) (PIGRECO*2*(angleindegrees/360.0));

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
