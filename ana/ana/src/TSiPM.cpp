/*
 *  TSiPM.cpp
 *
 */

#include "TSiPM.h"


TSiPM::TSiPM(int pixelperside){
	
	fN = pixelperside;

	fNpixel = fN*fN;
	printf("pixel per lato %d\n",fN);
	fNpixel = fN*fN;
	printf("pixel totali %d\n",fNpixel);	
	fhalf = pixelperside/2;
	printf("pixel meta' %d\n",fNpixel);		
	// add a check on parity of pixel side

	fXpixel = new float[fNpixel];
	fYpixel = new float[fNpixel];
	
	fXanode= new float[fNpixel];
	fYanode= new float[fNpixel];
	
	fXcathode= new float[fNpixel];
	fYcathode= new float[fNpixel];

	XVertex= new float[4];
	YVertex= new float[4];
	
	printf("memoria allocata \n");			
	
	
	if ((fXpixel==NULL)||(fYpixel==NULL)) {
		printf("ERROR: problem with memory\n");
	}
	
	for (int i=0; i<fNpixel; i++) {
		fXpixel[i]=.0;
		fYpixel[i]=.0;
	}
	printf("coordinate resettate\n");	
	
	
	fA = 1.90;
	fB = 1.30;
	fC = 1.60;
	fD = 3.20;
	
	fWidth = fD*fN; // dimensional outline of the device 
	
	printf("lARGHEZZA %3.1f\n",fWidth);	
	
	printf("pixel ");
	int col;
	for (int i=0; i<fNpixel; i++) {
		printf(" %d",i);	
		col = i%fN;
		switch (col) {
			case  0:fXpixel[i]= fN*fD - fC;break;
			case  1:fXpixel[i]= (fN-1)*fD - fC;break;
			case  2:fXpixel[i]= (fN-2)*fD - fC;break;
			case  3:fXpixel[i]= (fN-3)*fD - fC;break;
			case  4:fXpixel[i]= (fN-4)*fD - fC;break;
			case  5:fXpixel[i]= (fN-5)*fD - fC;break;
			case  6:fXpixel[i]= (fN-6)*fD - fC;break;
			case  7:fXpixel[i]= (fN-7)*fD - fC;break;
			case  8:fXpixel[i]= (fN-8)*fD - fC;break;
			case  9:fXpixel[i]= (fN-9)*fD - fC;break;
			case 10:fXpixel[i]= (fN-10)*fD - fC;break;
			case 11:fXpixel[i]= (fN-11)*fD - fC;break;
			case 12:fXpixel[i]= (fN-12)*fD - fC;break;
			case 13:fXpixel[i]= (fN-13)*fD - fC;break;
			case 14:fXpixel[i]= (fN-14)*fD - fC;break;
			case 15:fXpixel[i]= (fN-15)*fD - fC;break;	
			default:break;
		}
	}
	printf("\n");
	int row;
	
	printf("pixel ");
	
	for (int i=0; i<fNpixel; i++) {
		printf(" %d",i);	
		row = GetRow(i+1);
		switch (row) {
			case  1:fYpixel[i]= fN*fD - fC;break;
			case  2:fYpixel[i]= (fN-1)*fD - fC;break;
			case  3:fYpixel[i]= (fN-2)*fD - fC;break;
			case  4:fYpixel[i]= (fN-3)*fD - fC;break;
			case  5:fYpixel[i]= (fN-4)*fD - fC;break;
			case  6:fYpixel[i]= (fN-5)*fD - fC;break;
			case  7:fYpixel[i]= (fN-6)*fD - fC;break;
			case  8:fYpixel[i]= (fN-7)*fD - fC;break;
			case  9:fYpixel[i]= (fN-8)*fD - fC;break;
			case 10:fYpixel[i]= (fN-9)*fD - fC;break;
			case 11:fYpixel[i]= (fN-10)*fD - fC;break;
			case 12:fYpixel[i]= (fN-11)*fD - fC;break;
			case 13:fYpixel[i]= (fN-12)*fD - fC;break;
			case 14:fYpixel[i]= (fN-13)*fD - fC;break;
			case 15:fYpixel[i]= (fN-14)*fD - fC;break;
			case 16:fYpixel[i]= (fN-15)*fD - fC;break;	
			default:break;
		}
	}
	
	printf("\n");
	fE = 0.65;
	fF = 0.65;
	// Determino le coordinate di anodo e catodo relativamente a quelle di centro pixel
		printf("pixel ");
	for (int i=0; i<fNpixel; i++) {
				printf(" %d",i);	
		col = i%fN;
		switch (col) {
			case 0:
			case 1:
			case 4:
			case 5:
			case 8:
			case 9:
			case 12:
			case 13:				
				fYanode[i] = fYpixel[i]-fF;
				fXanode[i] = fXpixel[i]-fE;
				fYcathode[i] = fYpixel[i]+fE;
				fXcathode[i] = fXpixel[i]+fF;
				break;
			case 2:
			case 3:	
			case 6:
			case 7:	
			case 10:
			case 11:
			case 14:
			case 15:				
				fYanode[i] = fYpixel[i]+fE;
				fXanode[i] = fXpixel[i]-fF;
				fYcathode[i] = fYpixel[i]-fF;
				fXcathode[i] = fXpixel[i]+fE;
				break;

			default:
				break;
		}
	}
	
	printf("\n");
	
	XVertex[0]= 0.0;
	XVertex[1]= fWidth;
	XVertex[2]= fWidth;
	XVertex[3]= 0.0;
	
	YVertex[0]= 0.0;
	YVertex[1]= 0.0;
	YVertex[2]= fWidth;
	YVertex[3]= fWidth;

	printf("Vertex\n");

	Translate(-fhalf*fD,-fhalf*fD);
	printf("Traslate\n");
	
	MirrorX();
	printf("MirrorX\n");
	for (int i=0; i<fNpixel; i++) {
		printf("PIXEL %3d ",i+1);
		printf("COLONNA %s ",GetColumn(i).c_str());
		printf("RIGA %d; ",GetRow(i+1));
		printf("X = %5.3f ",fXpixel[i]);
		printf("Y = %5.3f ",fYpixel[i]);
		printf("\n");
	}
	printf("VERTEX: \n");
	for(int i=0;i<4;i++){
		printf("  %d              %7.3f     %7.3f\n",i+1,XVertex[i],YVertex[i]); 
	}
};

TSiPM::~TSiPM(){
	
	delete fXpixel;
	delete fYpixel;
	
	delete fXanode;
	delete fYanode;
	
	delete fXcathode;
	delete fYcathode;
	
	delete XVertex;
	delete YVertex;
}

void TSiPM::Print(){
	printf("SiPM Matrix %d x %d = %d pixels\n",fN,fN,fN*fN);
	for (int i=0; i<fNpixel; i++) {
		printf("A_%s%d, RD32SM,%6.2f,%6.2f, 0, 0, 0, 0,\r\n",GetColumn(i).c_str(),GetRow(i+1),GetXanode(i),GetYanode(i));
		printf("K_%s%d, RD32SM,%6.2f,%6.2f, 0, 0, 0, 0,\r\n",GetColumn(i).c_str(),GetRow(i+1),GetXcathode(i),GetYcathode(i));
	}
};

int TSiPM::GetRow(int pixel){
	float rowfloat = (float)pixel/fN;
	int row =  ceil (rowfloat);
	return row;
}

std::string TSiPM::GetColumn(int pixel){
	std::string str;
	int column =  pixel%fN;
	switch(column){
		case 0:str = "A";break;
		case 1:str = "B";break;
		case 2:str = "C";break;
		case 3:str = "D";break;
		case 4:str = "E";break;
		case 5:str = "F";break;
		case 6:str = "G";break;
		case 7:str = "H";break;
		case 8:str = "I";break;
		case 9:str = "J";break;
		case 10:str = "K";break;
		case 11:str = "L";break;
		case 12:str = "M";break;
		case 13:str = "N";break;
		case 14:str = "O";break;
		case 15:str = "P";break;
		default: printf("Error: break\n");
	}
	return str;
}

void TSiPM::MirrorX(){
	for(int i=0;i<fNpixel;i++){
		fXpixel[i]=-fXpixel[i]; 
		fXcathode[i]=-fXcathode[i]; 
		fXanode[i]=-fXanode[i]; 
	}
}

void TSiPM::Translate(float X0, float Y0){

	for(int i=0;i<fNpixel;i++){
		fXpixel[i]=fXpixel[i]+X0; 
		fYpixel[i]=fYpixel[i]+Y0;
		
		fXcathode[i]=fXcathode[i]+X0; 
		fYcathode[i]=fYcathode[i]+Y0;
		
		fXanode[i]=fXanode[i]+X0; 
		fYanode[i]=fYanode[i]+Y0;
	
		if(i<4){
			XVertex[i]=XVertex[i]+X0;
			YVertex[i]=YVertex[i]+Y0;
		}
	}
}

float TSiPM::GetX(int i)	{return fXpixel[i];}

float TSiPM::GetY(int i)	{return fYpixel[i];}

float TSiPM::GetXanode(int i)	{return fXanode[i];}

float TSiPM::GetYanode(int i)	{return fYanode[i];}

float TSiPM::GetXcathode(int i)	{return fXcathode[i];}

float TSiPM::GetYcathode(int i)	{return fYcathode[i];}

float TSiPM::GetXVertex(int i)	{return XVertex[i];}

float TSiPM::GetYVertex(int i)	{return YVertex[i];}



