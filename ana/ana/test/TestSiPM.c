/*
 * .x TestSiPM.c
 *
 *
 * Matteo Turisini 2014 Nov 25th
 *
 *
 */

#include <TCanvas.h>
#include <TGaxis.h>
#include <TPolyLine.h>
#include <TMarker.h>
#include "../src/TSiPM.cpp"
#define NSIDE 16
#define SCREEN_PIXX 1280
#define SCREEN_PIXY  900

void TestSiPM()
{
	TSiPM * pm = new TSiPM(NSIDE);
	pm->Print();
	
	Float_t Xcoordinates[NSIDE*NSIDE];
	Float_t Ycoordinates[NSIDE*NSIDE];

	for (int i=0; i<NSIDE*NSIDE; i++) {
		Xcoordinates[i] = pm->GetX(i+1);
		Ycoordinates[i] = pm->GetY(i+1);	
		printf("pixel[%d] %.3f %.3f \n",i+1,Xcoordinates[i],Ycoordinates[i]);
	}
	
	
	// Graphics
	
	Float_t A= 26.0*1.5; // mm
	Int_t D = 500; // size of the window in pixel
	Float_t B= .80;
	
	/* CANVAS 1 OUTLINE OF THE PM*/
	TCanvas*  mycanv = new TCanvas("mycanv","Outline of SiPM",SCREEN_PIXX-D,0,D,D); 		
	mycanv->Range(-A,-A,A,A);
	
	TGaxis *axis1 = new TGaxis(-B*A,-B*A,-B*A,B*A,-A,A,510,"");
	axis1->SetName("axis1");
	axis1->Draw();
	axis1->SetTitle("[mm]");
	
	TGaxis *axis2 = new TGaxis(-B*A,-B*A,B*A,-B*A,-A,A,510,"");
	axis2->SetName("axis2");
	axis2->Draw();
	axis2->SetTitle("[mm]");
	
	
	Double_t x[5] = {pm->GetXVertex(0),pm->GetXVertex(1),pm->GetXVertex(2),pm->GetXVertex(3),pm->GetXVertex(0)};
	Double_t y[5] = {pm->GetYVertex(0),pm->GetYVertex(1),pm->GetYVertex(2),pm->GetYVertex(3),pm->GetYVertex(0)};
	
	Int_t linewidth	= 2;
	Int_t mycolor	= 2;
	
	TPolyLine *pline = new TPolyLine(5,x,y);
	pline->SetFillColor(38);
	pline->SetLineColor(mycolor);
	pline->SetLineWidth(linewidth);
	pline->Draw("");	
	
	Int_t anode1mark= 8;
	Int_t anodemark = 7; //6
	
	TMarker * mark1, * mark2,* mark3, * mark4;
	
	for (int k=0; k<(NSIDE*NSIDE); k++) {// loop on anodes
		if (k==NSIDE*NSIDE-1) {
			mark1 = new TMarker((double)pm->GetX(k),(double)pm->GetY(k),anode1mark);
		}
		else {
			mark1 = new TMarker((double)pm->GetX(k),(double)pm->GetY(k),anodemark);
		}
		//mark1->Draw("SAME");
	}
	
	for (int k=0; k<(NSIDE*NSIDE); k++) {// loop on anodes
		
		mark2 = new TMarker((double)pm->GetXcathode(k),(double)pm->GetYcathode(k),anodemark);
		mark2->SetMarkerColor(2);
		mark2->Draw("SAME");
		
		mark3 = new TMarker((double)pm->GetXanode(k),(double)pm->GetYanode(k),anodemark);
		mark2->SetMarkerColor(4);
		mark3->Draw("SAME");
	}
	
	pm->Print();
	
}

