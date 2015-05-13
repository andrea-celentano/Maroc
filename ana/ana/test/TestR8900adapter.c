
/*
 *
 *
 * Matteo Turisini 2012 June 2th
 *
 * small ROOT script to test the class TR8900
 * Print anode coordinates and make a picture of the pM in the space
 *
 */

//Basato su file://localhost/Users/matteo/Desktop/120617_Map_Revision/Adapter_Comparison.tiff


// RICH PROTOTYPE FRONT VIEW (WHAT THE BEAM SEES)
//////////////////////////
//	80				2	// <----------- Connettore Samtec
//79				  1	//
//////////////////////////
//----------//----------//
//			//			//
//	  S2	//	  S3	//
//			//			//
//----------//----------// <----------- 4 R900
//			//			//
//	  S1	//	  S4	//
//			//			//
//----------//----------//
//////////////////////////

// S1 AND S2 ARE ROTATED BY 180 degrees respect S3 and S4 orientation

#include "../../mylibs/TR8900.cpp"
#include "../../mylibs/TConnectR8900.cpp"

#define SCREEN_PIXX 1280
#define SCREEN_PIXY  900
#define CANVSIZE	 360

void TestR8900adapter()
{
	Tr8900			R89[4]; 
	TConnectR8900	conn89; //R89[0]=S1,
							//R89[1]=S2,
							//R89[2]=S3,
							//R89[3]=S4
	
	// Fe_id  0 and GeoAddress 0
	for (int i=0; i<4; i++) {R89[i].SetFe(0); R89[i].SetGeo(32);}	
	
	// Rotation

	R89[0].Rotate(180.0);
	R89[1].Rotate(180.0);
	R89[2].Rotate(0.0);
	R89[3].Rotate(0.0);

	
	// Traslation
	Float_t W89 = R89[0].GetWitdh();
	
	Float_t x0	=	0.5*W89;
	Float_t y0	=	0.5*W89;
	

	R89[0].Translate(-x0,-y0);
	R89[1].Translate(-x0,+y0);
	R89[2].Translate(+x0,+y0);
	R89[3].Translate(+x0,-y0);
	
	
	printf("x0 = %3.1f, y0 = %3.1f\n", x0,y0);
	for (int i =0; i<4; i++) {
		R89[i].Print();
	}
	// PIXEL COORDINATES
	
	Float_t Xcoordinates[64];
	Float_t Ycoordinates[64];
	
	int i;
	for (i=0; i<64; i++) {
		Xcoordinates[i]=0;
		Ycoordinates[i]=0;
	}
	
	for (i=0; i<64; i++) {
		if (i<16) {
			Xcoordinates[i]=R89[3].GetX(i+1); // PMT 4 BLU
			Ycoordinates[i]=R89[3].GetY(i+1);
		}
		if ((i>15)&&(i<32)) {
			Xcoordinates[i]=R89[2].GetX(i+1-16);// PMT 3 VERDE
			Ycoordinates[i]=R89[2].GetY(i+1-16);
			
		}
		if ((i>31)&&(i<48)) {
			Xcoordinates[i]=R89[1].GetX(i+1-32);// PMT 2 ROSSO
			Ycoordinates[i]=R89[1].GetY(i+1-32);
		}
		if ((i>47)&&(i<64)) {
			Xcoordinates[i]=R89[0].GetX(i+1-48);// PMT 1 NERO
			Ycoordinates[i]=R89[0].GetY(i+1-48);
		}
		printf("%d =>(%7.1f,%7.1f)\n",i,Xcoordinates[i],Ycoordinates[i]);
	}
	
	
	//------- CANVAS

	TCanvas*  mycanv = new TCanvas("mycanv","Geometry",SCREEN_PIXX-CANVSIZE,0,CANVSIZE,CANVSIZE);
	Float_t A= 40.0; // mm 
	mycanv->Range(-A,-A,A,A);
	
	TGaxis *axis1 = new TGaxis(-.85*A,-.85*A,-.85*A,.85*A,-A,A,510,"");
	axis1->SetName("axis1");
	axis1->Draw();
	
	TGaxis *axis2 = new TGaxis(-.85*A,-.85*A,.85*A,-.85*A,-A,A,510,"");
	axis2->SetName("axis2");
	axis2->Draw();
	
	int linewidth=2;
	int mycolor = 0;
	
	int anode1mark = 8;
	int anodemark = 9;


	// Drawing
	
	for(int i=0;i<4;i++){ // 4 R8900  B
		
		Double_t x[5] = {R89[i].GetXVertex(0),R89[i].GetXVertex(1),R89[i].GetXVertex(2),R89[i].GetXVertex(3),R89[i].GetXVertex(0)};
		Double_t y[5] = {R89[i].GetYVertex(0),R89[i].GetYVertex(1),R89[i].GetYVertex(2),R89[i].GetYVertex(3),R89[i].GetYVertex(0)};
		
		TPolyLine *pline = new TPolyLine(5,x,y);
		
		pline->SetFillColor(38);
		pline->SetLineColor(i+1);
		pline->SetLineWidth(linewidth);	
		pline->Draw("");
		
		TMarker * mark1 = new TMarker((double)R89[i].GetX(1),(double)R89[i].GetY(1),anode1mark);
		mark1->Draw("SAME");
		
		printf("R8900 S%d or R89[%d]\n",i+1,i);
		printf("GEO ID =  %d\n",R89[i].GetGeo());
		printf("FE  ID =  %d\n",R89[i].GetFe());
		
		printf("                             maroc    system \n");
		printf("ANODE    X[mm]      Y[mm]   channel  channel \n");
		printf(" 1     %7.2f    %7.2f     %2d       %2d \n",R89[i].GetX(1), R89[i].GetY(1),conn89.GetMaroc(1,i+1),(R89[i].GetGeo()*64+conn89.GetMaroc(1,i+1)));
		
		
		for (int k=2; k<17; k++) { // loop on anodes
			mark1 = new TMarker((double)R89[i].GetX(k) ,(double)R89[i].GetY(k),anodemark);
			mark1->Draw("SAME");
			//printf("%2d     %7.2f    %7.2f     %2d       %2d \n",k,R89[i].GetX(k),R89[i].GetY(k),conn89.GetMaroc(k,i+1),(R89[i].GetGeo()*64+conn89.GetMaroc(k,i+1)));
			
		}
		printf("\n");
	}

	// CANVAS 2 - ANODES
	TCanvas*  mycanv2 = new TCanvas("mycanv2","ANODES",SCREEN_PIXX-CANVSIZE,CANVSIZE+20,CANVSIZE,CANVSIZE);
	mycanv2->Range(-A,-A,A,A);
	TH2F *h2 = new TH2F("h2","Hamamatsu View",100,-32,32,100,-32,32);
	for (i=0; i<64; i++) {//LOOP ON ANODES		
		// Hamamatsu View
		if (i<16) {
			h2->Fill(Xcoordinates[i],Ycoordinates[i],i+1);
		}
		if ((i>15)&&(i<32)) {
			h2->Fill(Xcoordinates[i],Ycoordinates[i],i+1-16);
		}
		if ((i>31)&&(i<48)) {
			h2->Fill(Xcoordinates[i],Ycoordinates[i],i+1-32);
		}
		if ((i>47)&&(i<64)) {
			h2->Fill(Xcoordinates[i],Ycoordinates[i],i+1-48);
		}

		
		//printf("%d: %d,%d \n",i,Xcoordinates[i],Ycoordinates[i]);
	}
	h2->Draw("text");
	h2->SetMarkerSize(2.2);
	h2->SetStats(0);
	
	for(int i=0;i<4;i++){ // 4 R8900  B
		
		Double_t x[5] = {R89[i].GetXVertex(0),R89[i].GetXVertex(1),R89[i].GetXVertex(2),R89[i].GetXVertex(3),R89[i].GetXVertex(0)};
		Double_t y[5] = {R89[i].GetYVertex(0),R89[i].GetYVertex(1),R89[i].GetYVertex(2),R89[i].GetYVertex(3),R89[i].GetYVertex(0)};
		
		TPolyLine *pline = new TPolyLine(5,x,y);
		
		pline->SetFillColor(38);
		pline->SetLineColor(i+1);
		pline->SetLineWidth(linewidth);	
		pline->Draw("same");
		
	}
	
	// CANVAS 3 - PIN OF THE ADAPTER BOARD//
	TCanvas*  mycanv3 = new TCanvas("mycanv3","ADAPTER BOARD",SCREEN_PIXX-2*CANVSIZE,0,CANVSIZE,CANVSIZE);
	mycanv3->Range(-A,-A,A,A);
	TH2F *h3 = new TH2F("h3","SamtecView",100,-32,32,100,-32,32);
	
	for (int s=1; s<5; s++) { // loop on R8900 // S1,S2,S3,S4
		for (int j=1; j<17; j++) { // loop on 16 anodes
			//printf("%d/%d\n",s-1,j);
			h3->Fill(R89[s-1].GetX(j),R89[s-1].GetY(j),conn89.GetPinfromPMTandAnode(s,j));
			printf("R89[%d].GetX(%2d) = %7.1f; R89[%2d].GetY(%2d) = %7.1f\n",s-1,j,R89[s-1].GetX(j),s-1,j,R89[s-1].GetY(j));
		}
	}
	
	h3->Draw("text");
	h3->SetMarkerSize(2.2);
	h3->SetStats(0);
	
	for(int i=0;i<4;i++){ // 4 R8900  B
		
		Double_t x[5] = {R89[i].GetXVertex(0),R89[i].GetXVertex(1),R89[i].GetXVertex(2),R89[i].GetXVertex(3),R89[i].GetXVertex(0)};
		Double_t y[5] = {R89[i].GetYVertex(0),R89[i].GetYVertex(1),R89[i].GetYVertex(2),R89[i].GetYVertex(3),R89[i].GetYVertex(0)};
		
		TPolyLine *pline = new TPolyLine(5,x,y);
		
		pline->SetFillColor(38);
		pline->SetLineColor(i+1);
		pline->SetLineWidth(linewidth);	
		pline->Draw("same");
		
	}
	
	
	// CANVAS 4 - MAROC CHANNEL
	TCanvas*  mycanv4 = new TCanvas("mycanv4","ADAPTER BOARD",SCREEN_PIXX-2*CANVSIZE,CANVSIZE+20,CANVSIZE,CANVSIZE);
	mycanv4->Range(-A,-A,A,A);
	TH2F *h4 = new TH2F("h4","Maroc View",100,-32,32,100,-32,32);
	
	for (int s=1; s<5; s++) { // loop on R8900 // S1,S2,S3,S4
		for (int j=1; j<17; j++) { // loop on 16 anodes
			printf("%d/%d\n",s-1,j);
			h4->Fill(R89[s-1].GetX(j),R89[s-1].GetY(j),conn89.GetMaroc(j,s));
			printf("R89[%d].GetX(%2d) = %7.1f; R89[%2d].GetY(%2d) = %7.1f\n",s-1,j,R89[s-1].GetX(j),s-1,j,R89[s-1].GetY(j));
		}
	}
	
	h4->Draw("text");
	h4->SetMarkerSize(2.2);
	h4->SetStats(0);
	
	for(int i=0;i<4;i++){ // 4 R8900  B
		
		Double_t x[5] = {R89[i].GetXVertex(0),R89[i].GetXVertex(1),R89[i].GetXVertex(2),R89[i].GetXVertex(3),R89[i].GetXVertex(0)};
		Double_t y[5] = {R89[i].GetYVertex(0),R89[i].GetYVertex(1),R89[i].GetYVertex(2),R89[i].GetYVertex(3),R89[i].GetYVertex(0)};
		
		TPolyLine *pline = new TPolyLine(5,x,y);
		
		pline->SetFillColor(38);
		pline->SetLineColor(i+1);
		pline->SetLineWidth(linewidth);	
		pline->Draw("same");
		
	}
	printf("\n\n\n");
	
	

	
	for (i=0; i<64; i++) {// loop on channels
		if (i<16) {
			Xcoordinates[i]=R89[3].GetX(conn89.GetAnode(i));// Voglio prendere la coordinata del canale i-eimo visto da maroc, dunque devo tradurre il canale maroc nell'anodo del relativo pmt
			Ycoordinates[i]=R89[3].GetY(conn89.GetAnode(i));
		}
		if ((i>15)&&(i<32)) {
			Xcoordinates[i]=R89[2].GetX(conn89.GetAnode(i-16));
			Ycoordinates[i]=R89[2].GetY(conn89.GetAnode(i-16));
			
		}
		if ((i>31)&&(i<48)) {
			Xcoordinates[i]=R89[1].GetX(conn89.GetAnode(i-32));
			Ycoordinates[i]=R89[1].GetY(conn89.GetAnode(i-32));
		}
		if ((i>47)&&(i<64)) {
			Xcoordinates[i]=R89[0].GetX(conn89.GetAnode(i-48));
			Ycoordinates[i]=R89[0].GetY(conn89.GetAnode(i-48));
		}
		printf("%d =>(%7.1f,%7.1f)\n",i,Xcoordinates[i],Ycoordinates[i]);
	}
	
	
	// CANVAS 5 - CHANNELS CHECK
	TCanvas*  mycanv5 = new TCanvas("mycanv5","CHANNELS CHECK",SCREEN_PIXX-3*CANVSIZE,0,CANVSIZE,CANVSIZE);
	mycanv5->Range(-A,-A,A,A);
	TH2F *h5 = new TH2F("h5","Check",100,-32,32,100,-32,32);
	
	for (int i=0; i<64; i++) { // loop on cb CHANNEL
		h5->Fill(Xcoordinates[i],Ycoordinates[i],R89[0].GetGeo()*64+i);			
	}
	
	h5->Draw("text");
	h5->SetMarkerSize(1.8);
	h5->SetStats(0);
	
	for(int i=0;i<4;i++){ // 4 R8900  B
		
		Double_t x[5] = {R89[i].GetXVertex(0),R89[i].GetXVertex(1),R89[i].GetXVertex(2),R89[i].GetXVertex(3),R89[i].GetXVertex(0)};
		Double_t y[5] = {R89[i].GetYVertex(0),R89[i].GetYVertex(1),R89[i].GetYVertex(2),R89[i].GetYVertex(3),R89[i].GetYVertex(0)};
		
		TPolyLine *pline = new TPolyLine(5,x,y);
		
		pline->SetFillColor(38);
		pline->SetLineColor(i+1);
		pline->SetLineWidth(linewidth);	
		pline->Draw("same");
		
	}
}
	
	