#include "TCalib.h"

TCalib::TCalib()
{	
	Reset();
}


TCalib::~TCalib()
{
	
}

void TCalib::Reset()
{
	for (int i=0;i<4096;i++)
	{
		fPedestal[i] = 0.0;
		fNoise   [i] = 0.0;
		fGain    [i] = 0.0;
		
	//	fAnodeX	 [i] = 0.0;
	//	fAnodeY	 [i] = 0.0;

	}
}



void TCalib::Export(const char * filename,int code)
{
	FILE * lout;
	int i;

	// ...insert here code for overwrite an existing file (if there is one)
	// hint try to open it in reading mode.

	lout = fopen(filename,"w");
	if(lout!=NULL){
		if(code==0){								// PEDESTAL
			fprintf(lout,"Channel Pedestal Noise\n");
			for(i=0;i<4096;i++){
				fprintf(lout,"%5d %6.1f %6.1f\n",i,GetPedestal(i),GetNoise(i));
			}
			printf("Message: Pedestal data successfully recorded on file %s \n",filename);
			
		}
		if(code==1){								// GAIN
			fprintf(lout,"Channel Gain\n");
			for(i=0;i<4096;i++){
				fprintf(lout,"%5d %6d\n",i,GetGain(i));
			}
			printf("Message: Gain data successfully recorded on file %s \n",filename);
		}
		fclose(lout);
	}
	else{
		printf("Warning: file %s not written\n",filename);
	}
}


void TCalib::Import(const char * filename)
{
	ffilename = filename;
	FILE * lin;
	int i;
	char str1[10], str2[10], str3[10],str4[10];

	int channel =   0;
	//archer/Hadron/en/Proposal_e.html
	int nHeaderLines = 1;

	Float_t P  = 0.0;
	Float_t N  = 0.0;
	Float_t G  = 0.0;

	lin = fopen(filename,"rt");
	if(lin!=NULL){
		for(i=0;i<nHeaderLines;i++){ // Header to skip
				fscanf(lin,"%s %s %s %s\n",str1,str2,str3,str4);
				printf("%s %s %s %s\n",str1,str2,str3,str4);
		}
		
		while(fscanf(lin,"%d %f %f %f",&channel,&P,&N,&G)!=EOF){   
			printf("%4d: %6.1f %6.1f %3.0f\n",channel,P,N,G); // debug
			SetPedestal(channel,P);
			SetNoise(channel,N);
			SetGain(channel,G);
		}
		fclose(lin);
		printf("File %s succesfully imported\n",filename);
	}
	else{
		printf("Warning: file %s do not exist!\n",filename);
	}	
}



void TCalib::Print(int code) 
{	// for debug
	// Print some data according to 'code' as follows
	// 1 - Pedestal Map; 
	// 2 - Noise Map; 
	// 3 - Gain Map;


	if(code==1){
		for(int i=0; i<4096; i++){
			printf("%6.1f",fPedestal[i]);
			if((i%16)==15){printf("|* %d\n",i);}
		}
		printf("Pedestal Map\n");
	}
	if(code==2){
		for(int i=0; i<4096; i++){
			printf("%6.1f",fNoise[i]);
			if((i%16)==15){printf("|* %d\n",i);}
		}
		printf("Noise Map\n");
	}
	if(code==3){
		for(int i=0; i<4096; i++){
			printf("%6f",fGain[i]);
			if((i%16)==15){printf("|* %d\n",i);}
		}
		printf("Gain Map\n");
	}
};

void TCalib::Distribution(int code,float binres)
{
	int lnbin = 4096*binres;
	
	TH1D * dummy = (TH1D*) gDirectory->FindObject("h"); if(dummy){delete dummy;}
	
	TH1D * h = new TH1D("h","title",lnbin,0.5,4095.5);
	

	
	Double_t noiseMin = 0.;
	Double_t noiseMax = 100.;
	
	
	Double_t pedMin = 0.5;
	Double_t pedMax = 4095.5;
	
	//Int_t mymarkerstyle = 20;
//	Int_t mymarkersize = 1;
	//	Double_t chMin = 900.;
	//	Double_t chMax = 2550.;
	
	
		for(int i=0;i<4096;i++){
			if(code==0){
				h->Fill(GetPedestal(i),1);
				h->SetTitle(ffilename);
				h->GetXaxis()->SetTitle("Pedestal[channel]");
				h->GetXaxis()->SetRangeUser(pedMin,pedMax);
				h->GetYaxis()->SetTitle("occurrency[#]");
			}
			if(code==1){
				h->Fill(GetNoise(i),1);
				h->SetTitle(ffilename);
				h->GetXaxis()->SetTitle("RMS[channel]");
				h->GetXaxis()->SetRangeUser(noiseMin,noiseMax);
				h->GetYaxis()->SetTitle("occurrency");
			}
			if(code==2){
				h->Fill(GetGain(i),1);
				h->SetTitle("Gain Distribution");
			}	
		}
	//TCanvas* mycanv = new TCanvas("mycanv","Parameter Distribution"); 
	h->Draw();
	
	//g->SetLineWidth(1);
	//g->SetMarkerColor(2);
	//g->SetMarkerStyle(24);
    //g->SetMarkerSize(8);
	
};



void TCalib::Plot(int code,const char * myopt,Float_t chstart, Float_t chstop)
{	// Shows Map and Scatter Plot according to what follows: 
	// 0 - Pedestals; 
	// 1 - Noises; 
	// 2 - Gains;
	// 3 - Noise vs Pedestal
	// 4 - Gain vs Pedestal
	// 5 - Noise vs Local channel
	
	Float_t llocalchannel[4096];
	Float_t lchannel[4096];
	
	
	printf("%f %f \n ",chstart,chstop);
	
	
	for (int i=0;i<4096;i++){
		lchannel[i] = (Float_t) i;
		if ((lchannel[i] >chstart)&&(lchannel[i] <chstop)) {
			llocalchannel[i] = (Float_t) (i%64);
		}
		else {
			llocalchannel[i]=(Float_t) 0.0;
		}

		
		printf("%4.0f %4.0f %4.1f\n",lchannel[i],llocalchannel[i],fNoise[i]);
	}
	
	

	TCanvas* mycanv = new TCanvas("mycanv","Calibration Parameter"); 
	TGraph * g =0;

	switch(code){
		case 0: g = new TGraph(4096,lchannel, fPedestal		);break;
		case 1: g = new TGraph(4096,lchannel, fNoise		);break;
		case 2: g = new TGraph(4096,lchannel, fGain			);break;
		case 3: g = new TGraph(4096,fPedestal,fNoise		);break;
		case 4: g = new TGraph(4096,fPedestal,fGain			);break;
		case 5: g = new TGraph(4096,llocalchannel,fNoise	);break;
		case 6: g = new TGraph(4096,llocalchannel,fPedestal	);break;
	}
	

     // "L" A simple poly-line between every points is drawn
     // "F" A fill area is drawn
     // "A" Axis are drawn around the graph
     // "C" A smooth curve is drawn
     // " * " A star is plotted at each point
     // "P" The current marker of the graph is plotted at each point
     // "B" A bar chart is drawn at each point

	

	g->Draw(myopt);
	g->SetLineColor(4);
	//g->SetLineWidth(1);
	g->SetMarkerColor(2);
	//g->SetMarkerStyle(24);
    g->SetMarkerSize(8);
	
	Double_t chMin = 2000;
	Double_t chMax = 2200.;
	
	Double_t noiseMin = 0.;
	Double_t noiseMax = 10;
	
	
	Double_t pedMin = 0.;
	Double_t pedMax = 4096.;
	
	Int_t mymarkerstyle = 20;
	Int_t mymarkersize = 1;
	

	switch(code){
		case 0: 
			g->SetTitle(ffilename);
			g->GetXaxis()->SetRangeUser(chMin,chMax);
			g->GetXaxis()->SetTitle("channel");
			g->GetYaxis()->SetRangeUser(pedMin,pedMax);
			g->GetYaxis()->SetTitle("adc");
			break;

		case 1: 		
			g->SetTitle(ffilename);
			g->GetXaxis()->SetRangeUser(chMin,chMax);
			g->GetXaxis()->SetTitle("channel");
			g->GetYaxis()->SetRangeUser(noiseMin,noiseMax);
			g->GetYaxis()->SetTitle("adc");
			break;

		case 2: 
			g->SetTitle("Gain Plot");
			g->GetXaxis()->SetTitle("channel");
			g->GetYaxis()->SetTitle("adc");
			break;
		case 3: 
			g->Draw("AP");
			g->SetTitle("Noise versus Pedestal");
			g->GetXaxis()->SetRangeUser(pedMin,pedMax);
			g->GetXaxis()->SetTitle("adc");
			g->GetYaxis()->SetRangeUser(noiseMin,noiseMax);
			g->GetYaxis()->SetTitle("adc");
			g->SetMarkerStyle(mymarkerstyle);	
			g->SetMarkerSize(mymarkersize);
			break;
		case 4: 
			g->SetTitle("Gain versus Pedestal");
			g->GetXaxis()->SetTitle("adc");
			g->GetYaxis()->SetTitle("adc");
			break;
		case 5: 
			g->Draw("AP");
			g->SetMarkerStyle(mymarkerstyle);	
			g->SetMarkerSize(mymarkersize);
			g->SetTitle("Noise vs Local Channel");
			g->GetXaxis()->SetTitle("maroc channel");
			g->GetYaxis()->SetRangeUser(noiseMin,noiseMax);
			g->GetYaxis()->SetTitle("adc");
			g->SetMarkerColor(3);
			
			break;
		case 6:
			g->Draw("AP");
			g->SetMarkerStyle(mymarkerstyle);	
			g->SetMarkerSize(mymarkersize);
			g->SetTitle("Pedestal vs Local Channel");
			g->GetXaxis()->SetTitle("maroc channel");
			g->GetYaxis()->SetTitle("adc");
			g->GetYaxis()->SetRangeUser(pedMin,pedMax);
			break;
	}
	mycanv->Update();
};


void TCalib::CardNoise(char geoaddress)
{
	if (geoaddress>63||geoaddress<0) {
		printf("Error in CardNoise argument: geoaddress range is [0..63], you have %d",geoaddress);
	}
	else{
		int ch=0;
		switch (geoaddress) {
			case 16:  ch=1024;break;
			case 17:  ch=1088;break;
			case 18:  ch=1152;break;
			case 19:  ch=1216;break;
			case 20:  ch=1280;break;
			case 21:  ch=1344;break;
			case 32:  ch=2048;break;
			case 33:  ch=2112;break;
			case 34:  ch=2176;break;
			case 35:  ch=2240;break;
			case 36:  ch=2304;break;
			case 37:  ch=2368;break;
			default:  ch=1024; printf("Warning: GEOaddress 16!\n"); break;
		}
		Plot(5,"ALP",ch,ch+64);// 
	}	
};


void TCalib::CardPedestal(char geoaddress)
{
	if (geoaddress>63||geoaddress<0) {
		printf("Error in CardPedestal argument: geoaddress range is [0..63], you have %d",geoaddress);
	}
	else{
		int ch=0;
		switch (geoaddress) {
			case 16:  ch=1024;break;
			case 17:  ch=1088;break;
			case 18:  ch=1152;break;
			case 19:  ch=1216;break;
			case 20:  ch=1280;break;
			case 21:  ch=1344;break;
			case 32:  ch=2048;break;
			case 33:  ch=2112;break;
			case 34:  ch=2176;break;
			case 35:  ch=2240;break;
			case 36:  ch=2304;break;
			case 37:  ch=2368;break;
			default:  ch=1024; printf("Warning: GEOaddress 16!\n"); break;
		}
		Plot(6,"ALP",ch,ch+64);// 
	}	
};




void TCalib::CardGain(char geoaddress)
{
	if (geoaddress>63||geoaddress<0) {
		printf("Error in CardPedestal argument: geoaddress range is [0..63], you have %d",geoaddress);
	}
	else{
		int ch=0;
		switch (geoaddress) {
			case 16:  ch=1024;break;
			case 17:  ch=1088;break;
			case 18:  ch=1152;break;
			case 19:  ch=1216;break;
			case 20:  ch=1280;break;
			case 21:  ch=1344;break;
			case 32:  ch=2048;break;
			case 33:  ch=2112;break;
			case 34:  ch=2176;break;
			case 35:  ch=2240;break;
			case 36:  ch=2304;break;
			case 37:  ch=2368;break;
			default:  ch=1024; printf("Warning: GEOaddress 16!\n"); break;
		}
		Plot(2,"ALP",ch,ch+64);// 
	}	
};




