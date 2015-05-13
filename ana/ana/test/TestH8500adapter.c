
/*
 * .x TestH8500.c
 *
 *
 * Matteo Turisini 2012 June 2th
 *
 * small ROOT script to test the class Th8500
 * Print anode coordinates and make a picture of the pM in the space
 *
 */


#include "../src/TH8500.cpp"
#include "../src/TConnectH8500.cpp"

#define SCREEN_PIXX 1280
#define SCREEN_PIXY  900

void TestH8500adapter()
{
	Th8500* pm = new Th8500(); //pm->Print();
	TConnectH8500	conn85;	 //conn85.Print(1,0,0);
	
	//TRANSLATION? NO
	//ROTATION? NO
	//pm->Rotate(180.0);
	
	
	Float_t Xcoordinates[64];
	Float_t Ycoordinates[64];
	
	int i;
	for (i=0; i<64; i++) {
		Xcoordinates[i]=0;
		Ycoordinates[i]=0;
	}
	
	for (i=0; i<64; i++) {
		Xcoordinates[i]=pm->GetX(i+1);
		Ycoordinates[i]=pm->GetY(i+1);
		//printf("anode %d : (%3.2lf,%3.2lf)\n",i+1,Xcoordinates[i],Ycoordinates[i]);
	}
	
	
	// Graphics
	
	Float_t A= 26.0*1.5; // mm
	Int_t D = 368; // size of the window in pixel
	Float_t B= .80;

	/* CANVAS 1 OUTLINE OF THE PM*/
	TCanvas*  mycanv = new TCanvas("mycanv","Outline of H8500",SCREEN_PIXX-D,0,D,D); 		
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
	Int_t anodemark = 6;
	
	for (int k=1; k<65; k++) {// loop on anodes
		if (k==1) {
			mark1 = new TMarker((double)pm->GetX(k),(double)pm->GetY(k),anode1mark);
		}
		else {
			mark1 = new TMarker((double)pm->GetX(k),(double)pm->GetY(k),anodemark);
		}
		mark1->Draw("SAME");
	}
	
	
	/* CANVAS 2 - ANODES*/
	TCanvas*  mycanv2 = new TCanvas("mycanv2","ANODES",SCREEN_PIXX-D,D+20,D,D);
	mycanv2->Range(-A,-A,A,A);
	TH2F *h2 = new TH2F("h2","Hamamatsu View",100,-32,32,100,-32,32);
	for (i=0; i<64; i++) {//LOOP ON ANODES		
		// Hamamatsu View
		h2->Fill(Xcoordinates[i],Ycoordinates[i],i+1);
		//printf("%d: %d,%d \n",i,Xcoordinates[i],Ycoordinates[i]);
	}
	h2->Draw("text");
	h2->SetMarkerSize(2.2);
	h2->SetStats(0);
	
	TPolyLine *pline = new TPolyLine(5,x,y);
	pline->SetFillColor(38);
	pline->SetLineColor(mycolor);
	pline->SetLineWidth(linewidth);
	pline->Draw("same");

	
	
	
	//CANVAS 3 - MAROC
	TCanvas*  mycanv3 = new TCanvas("mycanv3","MAROC CHANNEL",SCREEN_PIXX-2*D,D+20,D,D);
	mycanv3->Range(-A,-A,A,A);
	TH2F *h3 = new TH2F("h3","Maroc View",100,-32,32,100,-32,32);
	for (i=0; i<64; i++) {//LOOP ON ANODES		
		// Maroc View 
		h3->Fill(Xcoordinates[i],Ycoordinates[i],conn85.GetMaroc(i+1));
		//printf("%d: %d,%d \n",i,Xcoordinates[i],Ycoordinates[i]);
	}
	h3->Draw("text"); // DO NOT PRINT ZERO!!! WHY?	
	h3->SetMarkerSize(2.2);
	h3->SetStats(0);
	
	TPolyLine *pline = new TPolyLine(5,x,y);
	pline->SetFillColor(38);
	pline->SetLineColor(mycolor);
	pline->SetLineWidth(linewidth);
	pline->Draw("same");

	
	//CANVAS 4 - PIN of the Adapter Board
	TCanvas*  mycanv4 = new TCanvas("mycanv4","Adapter PIN",SCREEN_PIXX-2*D,0,D,D);
	mycanv4->Range(-A,-A,A,A);
	TH2F *h4 = new TH2F("h3","Samtec View",100,-32,32,100,-32,32);
	for (i=0; i<64; i++) {//LOOP ON ANODES		
		// Samtec View
		h4->Fill(Xcoordinates[i],Ycoordinates[i],conn85.GetPinfromAnode(i+1));
	}
	h4->Draw("text"); // DO NOT PRINT ZERO!!! WHY?	
	h4->SetMarkerSize(2.2);
	h4->SetStats(0);
	
	TPolyLine *pline = new TPolyLine(5,x,y);
	pline->SetFillColor(38);
	pline->SetLineColor(mycolor);
	pline->SetLineWidth(linewidth);
	pline->Draw("same");
	
	//h2->Print();
	
	
		

	delete pm;
}

