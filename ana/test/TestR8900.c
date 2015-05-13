/*
 * .x TestR8900.c
 *
 *
 * Matteo Turisini 2012 June 2th
 *
 * small ROOT script to test the class Tr8900
 * Print anode coordinates and make a picture of the pM in the space
 *
 */

#include "../../mylibs/Tr8900.cpp"

void TestR8900()
{
	Tr8900* pm = new Tr8900(); 
	
	pm->Rotate(0.0); // degrees counterclockwise
	pm->Translate(0.0,0.0); //(x,y)
	
	pm->Print();

	float Xcoordinates[16];
	float Ycoordinates[16];
	int i;
	for (i=0; i<16; i++) {
		Xcoordinates[i]=0;
		Ycoordinates[i]=0;
	}
	for (i=0; i<16; i++) {
		Xcoordinates[i]=pm->GetX(i+1);
		Ycoordinates[i]=pm->GetY(i+1);
		//printf("anode %2d : (%5.2lf,%5.2lf)\n",i+1,Xcoordinates[i],Ycoordinates[i]);
	}
	
	Int_t D = 300; // size of the window in pixel
	TCanvas*  canv = new TCanvas("canv","Outline of R8900",1280-D,0,D,D);
	Float_t A1 = 26.0*1.5; // mm 
	canv->Range(-A1,-A1,A1,A1);
	TGaxis *axis1 = new TGaxis(-.85*A1,-.85*A1,-.85*A1,.85*A1,-A1,A1,510,"");
	axis1->SetName("axis1");
	axis1->Draw();
	TGaxis *axis2 = new TGaxis(-.85*A1,-.85*A1,.85*A1,-.85*A1,-A1,A1,510,"");
	axis2->SetName("axis2");
	axis2->Draw();
	Double_t x[5] = {pm->GetXVertex(0),pm->GetXVertex(1),pm->GetXVertex(2),pm->GetXVertex(3),pm->GetXVertex(0)};
	Double_t y[5] = {pm->GetYVertex(0),pm->GetYVertex(1),pm->GetYVertex(2),pm->GetYVertex(3),pm->GetYVertex(0)};
	TPolyLine *pline = new TPolyLine(5,x,y);
	
	Int_t linewidth	= 2;
	Int_t mycolor	= 2;

	pline->SetFillColor(38);
	pline->SetLineColor(mycolor);
	pline->SetLineWidth(linewidth);
	pline->Draw("");
		
	Int_t anode1mark= 8;
	Int_t anodemark = 6;

	for (int k=1; k<17; k++) {// loop on anodes
		if (k==1) {
			mark1 = new TMarker((double)pm->GetX(k),(double)pm->GetY(k),anode1mark);
		}
		else {
			mark1 = new TMarker((double)pm->GetX(k),(double)pm->GetY(k),anodemark);
		}
		mark1->Draw("SAME");
	}

	/*  // Paint with TGraph
	 TCanvas*  mycanv = new TCanvas("mycanv","Pixel Location HR8900",1280-2*D,0,D,D); 		
	 TGraph * g = new TGraph(16,Xcoordinates,Ycoordinates);
	 g->SetMarkerColor(4);
	 g->SetMarkerSize(4.0);
	 g->SetMarkerStyle(21);
	 g->Draw("AP");
	 TMarker * mark1 = new TMarker((double)pm->GetX(1) ,(double)pm->GetY(1),20);
	 mark1->Draw("SAME");
	 */
	delete pm;
}

