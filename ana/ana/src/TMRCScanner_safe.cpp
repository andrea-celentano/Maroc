#include"TMRCScanner.h"


TMRCScanner::TMRCScanner()
{
	MRC_INF("Ready to recall a sequence of run, use Recall() \n");
}

void TMRCScanner::Recall(const char * oldrootfile)
{
	sprintf(foutfile,"%s",oldrootfile);	
	MRC_INF("Filename %s setted, ready to Plot() \n",foutfile);
}


TMRCScanner::TMRCScanner(const char * prefix, int firstrun, int lastrun, int nbulk)
{
	ResetSingleRunVariables();
	ResetRunSequence();
	
	if (FixRunNames(prefix,firstrun,lastrun)==true) {
		MRC_INF("Starting the analysis of the specified run collection\n");
		Scan(nbulk);
	}else {
		MRC_ERR("Incorrect sequence of runs ID. %d is not greter then %d\n",lastrun,firstrun);
	}	
}


TMRCScanner::~TMRCScanner()
{
	ResetRunSequence();
}




void TMRCScanner::ResetSingleRunVariables()
{
	frun=0;
	frate=0.0;
	fdelay=0;
	fdac=0;
	fDuration=0.0;				
	fPollEff=0.0;	
	fNevents=0;			
	fFeNum =0;
	
	
	for (int ch= 0; ch<4096;ch++) {
		fADCEntries[ch]	=0;	
		fADCMean[ch]	=0.;
		fADCRms[ch]		=0.;	
		fADCMax[ch]		=0.;
		fADCMin[ch]		=0.;		
		fHITEntries[ch]	=0;	
		fHITMean[ch]	=0.;	
		fHITRms[ch]		=0.;	
		fHITMax[ch]		=0.;
		fHITMin[ch]		=0.;		
		fRatioHITADC[ch]=0.;
	}
	MRC_DBG("Single run variables resetted \n");
	fHitMultMean	=0.;	
	fHitMultMeanErr	=0.;	
	fHitMultRMS		=0.;		
	fHitMultRMSErr	=0.;	
	
	MRC_DBG("Single run Multiplicity variables resetted \n");	

}

void TMRCScanner::ResetRunSequence()
{
	fNRun=0;
	ffirstRun=0;
	for (int run=0; run<MAXRUN; run++) {
		fdly[run]=0;
	}
	for (int c=0; c<NAMESIZE; c++) {
		foutfile[c]=0;
	}
	printf("Memory cleaned\n");
}



bool TMRCScanner::FixRunNames(const char * prefix,int firstrun,int lastrun)
{
	bool IsSequence;
	if (firstrun>=lastrun) {
		IsSequence= false;
	}else {
		sprintf(foutfile,"%s%s_%d_%d.root",PATH_ROOTFILE_SCAN,prefix,firstrun,lastrun);	
		fNRun = lastrun-firstrun+1;
		ffirstRun = firstrun;
		MRC_INF("%d input files\nOutfile name will be %s\n",fNRun,foutfile);
		for (int idx_run = 0;idx_run<fNRun ; idx_run++) {
			sprintf(fcurrRun[idx_run],"%s%s_%05d.bin",BINPATH,prefix,firstrun+idx_run);			
			MRC_DUM("RunName[%d] = %s\n",idx_run,fcurrRun[idx_run]);
		}
		IsSequence =  true;
	}
	return IsSequence;
}


void TMRCScanner::SingleRunAnalysis(int idx_run, int nbulk){
	
	TAnalysis * A = new TAnalysis(fcurrRun[idx_run]);	
	
	frate		= A->Get_ACQRate();
	fdac		= A->Get_DAC0();
	fdelay		= A->Get_HOLD1_Delay();
	fDuration	= A->GetDuration();				
	fPollEff	= A->GetPollingEfficiency();
	fNevents	= A->GetNevents();				
	fFeNum		= A->GetFeNum();		
	
	frun = ffirstRun+idx_run;
	
	// Read Configuration
	int nFeCard = A->GetFeNum();
	MRC_MSG("Found %d card(s)\n", nFeCard);
	
	// Read data
	printf("Parsing data... ");
	int Nread =A->Read(nbulk); // Rename ReadData
	printf("found %d bulks\n",Nread);
	
	
	
	// Single channel analysis
	int current_geo;
	int chglob;
	for (int fe_idx=0; fe_idx<nFeCard; fe_idx++) { // loop on fe cards	
		current_geo= A->GetGeo(fe_idx);
		printf("Processing geoaddress %d\n",current_geo);
		for (int chfe=0; chfe<64; chfe++) {
			chglob = current_geo*64 + chfe;
			fADCMean[chglob]	=A->GetADCMean(chglob);
			fADCRms[chglob]		=A->GetADCRms(chglob);
			fADCMax[chglob]		=A->GetADCHigh(chglob);		
			fADCMin[chglob]		=A->GetADCLow(chglob);
			fHITMean[chglob]	=A->GetHITMean(chglob);	
			fHITRms[chglob]		=A->GetHITRms(chglob);	
			fHITMax[chglob]		=A->GetHITHigh(chglob);
			fHITMin[chglob]		=A->GetHITLow(chglob);		
			fHITEntries[chglob]	=0;
			fADCEntries[chglob]	=0;
			fRatioHITADC[chglob]	=0.0;
			fADCEntries[chglob]	=A->GetADCEntries(chglob);
			fHITEntries[chglob]	=A->GetHITEntries(chglob);
			if(fADCEntries[chglob]!=0){
				fRatioHITADC[chglob]=((float)fHITEntries[chglob]/fADCEntries[chglob]);
			}
		} // loop on channel [0..63]
	} //loop on fe [0..nFeCard]
	
	printf("Extracting Event Multiplicity\n");
	
	Float_t * par = A->GetMultiplicity();
	fHitMultMean	=par[0];	
	fHitMultMeanErr	=par[1];	
	fHitMultRMS		=par[2];		
	fHitMultRMSErr	=par[3];
	
	MRC_DUM("Multiplicity:\n");
	MRC_DUM("\tMean: %.3f  (%.3f)\n",par[0],par[1]);
	MRC_DUM("\tRMS:  %.3f  (%.3f)\n",par[2],par[3]);	
	
	delete A; 
}

int TMRCScanner::Scan(int nbulk)
{
	MRC_MSG("Creating TTree variables for global data and daq configuration\n");

	ffile = new TFile(foutfile,"RECREATE");
	ftree = new TTree("T","Calibration Data");	
	CreateTreeBranches();
	if (nbulk) {
		CreateTreeBranchesSinglePixel();
	}
	
	int idx_run; 
	
	for (idx_run=0; idx_run<fNRun; idx_run++) { // loop on Run
		
		MRC_INF("Run %3d File: %s\n",idx_run+1,fcurrRun[idx_run]);
		MRC_INF("Runs %3d of %d  \n",idx_run+1,fNRun);

		SingleRunAnalysis(idx_run,nbulk);
		
		ffile->cd();
		ftree->Fill();
		ResetSingleRunVariables();
	} 
	ftree->Write();
	ftree->Scan("Run:DAC:Rate");
	ffile->Close();
	printf("Scan success, data on %s\n",foutfile);
	return idx_run; 
}






void TMRCScanner::CreateTreeBranches()
{
	MRC_MSG("System parameters\n");		
	ftree->Branch("Run"	,&frun		,"run/I"	);
	ftree->Branch("Rate"	,&frate		,"rate/F"	);
	ftree->Branch("DAC"	 ,&fdac		,"dac/I"	);
	ftree->Branch("Delay"	,&fdelay	,"delay/I"	);	 // HOLD1_Delay
	
	ftree->Branch("Duration",&fDuration	,"Duration/F");	 
	ftree->Branch("PollEff"	,&fPollEff	,"PollEff/F");	 
	ftree->Branch("Events"  ,&fNevents	,"Events/I"	);	 
	ftree->Branch("FEnum"	,&fFeNum	,"FEnum/I"	);	
	
	ftree->Branch("HitMultMean"		,&fHitMultMean,		"HitMultMean/F"	);
	ftree->Branch("HitMultMeanErr"	,&fHitMultMeanErr,	"HitMultMeanErr/F");
	ftree->Branch("HitMultRMS"		,&fHitMultRMS,		"HitMultRMS/F"	);
	ftree->Branch("HitMultRMSErr"	,&fHitMultRMSErr,	"HitMultRMSErr/F");	


}

void TMRCScanner::CreateTreeBranchesSinglePixel()
{
	MRC_MSG("Individual Channel/Pixel parameters\n");		
	ftree->Branch("ADCEntries",	fADCEntries,"ADCEntries[4096]/I");
	ftree->Branch("ADCMean",	fADCMean,	"ADCMean[4096]/F");
	ftree->Branch("ADCRms",		fADCRms,	"ADCRms[4096]/F");
	ftree->Branch("ADCMax",		fADCMax,	"ADCMax[4096]/F");
	ftree->Branch("ADCMin",		fADCMin,	"ADCMin[4096]/F");
	
	ftree->Branch("HITEntries",	fHITEntries,"HITEntries[4096]/I");
	ftree->Branch("HITMean",	fHITMean,	"HITMean[4096]/F");
	ftree->Branch("HITRms",		fHITRms,	"HITRms[4096]/F");
	ftree->Branch("HITMax",		fHITMax,	"HITMax[4096]/F");
	ftree->Branch("HITMin",		fHITMin,	"HITMin[4096]/F");
	
	ftree->Branch("RatioHITADC",fRatioHITADC,"RatioHITADC[4096]/F");
}


//void	TMRCScanner::GetElectronicsParameters(){}
//void	TMRCScanner::AnaliseSinglePixel(){}






/**********/
/* Reader */
/**********/





void TMRCScanner::Plot(const char * what, const char * cut)
{
	//printf("%s\n",foutfile);
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		ftree->Print();
		ftree->Print();	
		ftree->SetMarkerStyle(7);
		ftree->Draw(what,cut);
	}else {
		printf("File %s not opened \n",foutfile);
	}
	delete ffile;
}	

bool TMRCScanner::PlotRatevsDAC_Channel(int channel,int xmin,int xmax, int ymax)
{
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		//ftree->SetMarkerStyle(23);
		//ftree->SetLineColor(kRed);
		ftree->Draw(Form("(HITEntries[%d]/Duration):DAC>>hDAC(1024,0,1023,2001,0,2000)",channel),"HITEntries!=0","L");
		TH2F *h = (TH2F*)gPad->GetPrimitive("hDAC");
		TAxis *xaxis = h->GetXaxis();
		xaxis->SetTitle("Threshold [DAC unit]");
		xaxis->SetRange(xmin,xmax);
		TAxis *yaxis = h->GetYaxis();
		yaxis->SetTitle("Rate [Hz]");
		yaxis->SetRange(0,ymax);		
		h->SetTitle("Pulse Amplitude Integral Distribution");
	}else {
		printf("File %s not opened \n",foutfile);
		return false;
	}
	delete ffile;
	return true;
}

bool TMRCScanner::PlotRatevsDAC_GEO(int geo,int xmin,int xmax, int ymax)
{
	int channel = geo*64;
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		//ftree->SetMarkerStyle(23);
		//ftree->SetLineColor(kRed);
		ftree->Draw(Form("(HITEntries[%d]/Duration):DAC>>hDAC(1024,0,1023,2001,0,2000)",channel),"HITEntries!=0","L");
		TH2F *h = (TH2F*)gPad->GetPrimitive("hDAC");
		TAxis *xaxis = h->GetXaxis();
		xaxis->SetTitle("Threshold [DAC unit]");
		xaxis->SetRange(xmin,xmax);
		TAxis *yaxis = h->GetYaxis();
		yaxis->SetTitle("Rate [Hz]");
		yaxis->SetRange(0,ymax);		
		h->SetTitle("Pulse Amplitude Integral Distribution");
		for (int i = 1; i<(64-1); i++) {
			ftree->Draw(Form("(HITEntries[%d]/Duration):DAC",channel+i),"HITEntries!=0","SAME,L");
		}
		//ftree->Draw("Rate:DAC","","SAME,L");
	}else {
		printf("File %s not opened \n",foutfile);
		return false;
	}
	delete ffile;
	return true;
}

bool TMRCScanner::PlotRatevsDAC(int xmin,int xmax,int ymax)
{
  int channel = 0;

// Read channel range from configuration
	
  ffile = new TFile(foutfile);
  if (ffile->IsOpen()){		
    ftree = (TTree*) ffile->Get("T"); 
    //ftree->SetMarkerStyle(23);
    //ftree->SetLineColor(kRed);
     ftree->Draw("(HITEntries[0]/Duration):DAC>>hDAC(1024,0,1023,2001,0,2000)","HITEntries!=0","L");
		TH2F *h = (TH2F*)gPad->GetPrimitive("hDAC");
		TAxis *xaxis = h->GetXaxis();
		xaxis->SetTitle("Threshold [DAC unit]");
		xaxis->SetRange(xmin,xmax);
		TAxis *yaxis = h->GetYaxis();
		yaxis->SetTitle("Rate [Hz]");
		yaxis->SetRange(0,ymax);		
		h->SetTitle("Pulse Amplitude Integral Distribution");
		for (int i = 2048; i<(2048+128); i++) {
			ftree->Draw(Form("(HITEntries[%d]/Duration):DAC",channel+i),"HITEntries!=0","SAME,L");
			printf("%d\n",i);
		}
		ftree->SetLineColor(kRed);
		ftree->Draw("Rate:DAC","","SAME,L");
	}else {
		printf("File %s not opened \n",foutfile);
		return false;
	}
	delete ffile;
	return true;
}


void TMRCScanner::CmpRate(const char * rootfile1,
						  const char * rootfile2,
						  const char * rootfile3,
						  const char * rootfile4,
						  const char * rootfile5,
						  const char * rootfile6,
						  const char * rootfile7,
						  const char * rootfile8,
						  const char * rootfile9)
{
	if (rootfile1!=NULL) {
		//TCanvas *c1 = new TCanvas("c1","c1",600,500);
		TLegend* leg = new TLegend(0.1,0.7,0.48,0.9);
		// leg->SetHeader("The Legend Title");
		leg->SetHeader("Pre-Amp Gain");
		Color_t color1 = kRed;		//23
		Color_t color2 = kBlue;	//24
		Color_t color3 = kMagenta;  //25
		// Color_t color4 = kRed;  //26
		Color_t color4 = kCyan;  //26
		// Color_t color5 = kRed+3;   //27
		Color_t color5 = kYellow;   //27
		Color_t color6 = kGreen;  //28
		Color_t color7 = kCyan;  //29
		Color_t color8 = kYellow;  //30
		Color_t color9 = kMagenta;
		
		TFile* f1 = new TFile(rootfile1);
	
		if (f1->IsOpen()) {
			TTree * mytree1 = (TTree*) f1->Get("T");
			mytree1->SetMarkerStyle(22);
			mytree1->SetMarkerColor(kRed);
			mytree1->Draw("(HITEntries[2048]/Duration):DAC>>hDAC(800,160,860,1000,0,1000)","HITEntries!=0");
			// mytree1->Draw("(HITEntries[2048]/Duration):DAC>>hDAC(1024,0,1023,1000,0,1000)","HITEntries!=0","P");
			// mytree1->Draw("(HITEntries[]/Duration):DAC>>hDAC(1024,0,1023,1000,0,1000)","HITEntries!=0","L");				
			TH2F *h = (TH2F*)gPad->GetPrimitive("hDAC");
			TAxis *xaxis = h->GetXaxis();
			xaxis->SetTitle("Threshold [DAC unit]");
			xaxis->SetRange(0,1024);
			TAxis *yaxis = h->GetYaxis();
			yaxis->SetTitle("Rate [Hz]");
			yaxis->SetRange(0,1000);		
			h->SetTitle("Pulse Amplitude Integral Distribution");
			h->Draw();
			for (int i =2049 ; i<2112; i++) {
				mytree1->SetLineColor(color1);
				mytree1->Draw(Form("(HITEntries[%d]/Duration):DAC>>h",i),"HITEntries!=0","SAME");
			}
			mytree1->SetMarkerColor(color1);	
			// leg->AddEntry(mytree1,rootfile1,"L");
			leg->AddEntry(mytree1,"Gain 0.5","P");
		}else {
			printf("File %s not opened \n",foutfile);

		}

		if (rootfile2!=NULL) {
		TFile* f2 = new TFile(rootfile2);
		TTree * mytree2 = (TTree*) f2->Get("T");
			mytree2->SetMarkerStyle(22);
			mytree2->SetMarkerColor(color2);
		for (int i =2048 ; i<2112; i++) {
			mytree2->SetLineColor(color2);
			// mytree2->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
			mytree2->Draw(Form("(HITEntries[%d]/Duration):DAC>>h",i),"HITEntries!=0","SAME");
		}
		mytree2->SetMarkerColor(color2);	
			leg->AddEntry(mytree2,"Gain 1.0","P");

		}
			
		if (rootfile3!=NULL) {
		TFile* f3 = new TFile(rootfile3);
		TTree * mytree3 = (TTree*) f3->Get("T");
			mytree3->SetMarkerStyle(22);
			mytree3->SetMarkerColor(color3);
		for (int i =2048 ; i<2112; i++) {
			mytree3->SetLineColor(color3);
			mytree3->Draw(Form("(HITEntries[%d]/Duration):DAC>>h",i),"HITEntries!=0","SAME");
			// mytree3->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		mytree3->SetMarkerColor(color3);	
		//leg->AddEntry(mytree3,rootfile3,"L");
			leg->AddEntry(mytree3,"Gain 1.5","P");

		}
		
		if (rootfile4!=NULL) {
		TFile* f4 = new TFile(rootfile4);
		TTree * mytree4 = (TTree*) f4->Get("T");
			mytree4->SetMarkerStyle(22);
			mytree4->SetMarkerColor(color4);
		for (int i =2048 ; i<2112; i++) {
			mytree4->SetLineColor(color4);
			mytree4->Draw(Form("(HITEntries[%d]/Duration):DAC>>h",i),"HITEntries!=0","SAME");
			// mytree4->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		mytree4->SetMarkerColor(color4);	
		//leg->AddEntry(mytree4,rootfile4,"L");
			leg->AddEntry(mytree4,"Gain 2.0","P");

		}
		
		if (rootfile5!=NULL) {
		TFile* f5 = new TFile(rootfile5);
		TTree * mytree5 = (TTree*) f5->Get("T");
			mytree5->SetMarkerStyle(22);
			mytree5->SetMarkerColor(color5);
		for (int i =2048 ; i<2112; i++) {
			mytree5->SetLineColor(color5);
			mytree5->Draw(Form("(HITEntries[%d]/Duration):DAC>>h",i),"HITEntries!=0","SAME");
			// mytree5->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		mytree5->SetMarkerColor(color5);	
		//leg->AddEntry(mytree5,rootfile5,"L");
			leg->AddEntry(mytree5,"Gain 3.0","P");

		}
			
		if (rootfile6!=NULL) {	
		TFile* f6 = new TFile(rootfile6);
		TTree * mytree6 = (TTree*) f6->Get("T");
			mytree6->SetMarkerStyle(22);
		mytree6->SetLineColor(color6);
		mytree6->SetMarkerColor(color6);	
		for (int i =2048 ; i<2112; i++) {
		  mytree6->Draw(Form("(HITEntries[%d]/Duration):DAC>>h",i),"HITEntries!=0","SAME");
			// mytree6->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		//leg->AddEntry(mytree6,rootfile6,"L");
			leg->AddEntry(mytree6,"Gain 4.0","P");

		}
		
		if (rootfile7!=NULL) {
		TFile* f7 = new TFile(rootfile7);
		TTree * mytree7 = (TTree*) f7->Get("T");
		mytree7->SetLineColor(color7);
		mytree7->SetMarkerColor(color7);	
		for (int i =2048 ; i<2112; i++) {
			
			mytree7->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		leg->AddEntry(mytree7,rootfile7,"L");
		}
			
		if (rootfile8!=NULL) {
		TFile* f8 = new TFile(rootfile8);
		TTree * mytree8 = (TTree*) f8->Get("T");
		mytree8->SetLineColor(color8);
		mytree8->SetMarkerColor(color8);	
		for (int i =2048 ; i<2112; i++) {
			
			mytree8->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		leg->AddEntry(mytree8,rootfile8,"L");
		}
			
		if (rootfile9!=NULL) {
		TFile* f9 = new TFile(rootfile9);
		TTree * mytree9 = (TTree*) f9->Get("T");
		mytree9->SetLineColor(color9);
		mytree9->SetMarkerColor(color9);	
		for (int i =2048 ; i<2112; i++) {
			
			mytree9->Draw(Form("(HITEntries[%d]/Duration):DAC",i),"HITEntries!=0","SAME,L");
		}
		leg->AddEntry(mytree9,rootfile9,"L");
		}
		
		leg->Draw();
	}else {
		printf("Please specify at least one file");
	}
}


void	TMRCScanner::Scurve(int geo)
{
	// TODO 	1: Recuperare durata effettiva dal dal Run
	//			2: Stabilire a priori il range degli assi
	//			3: Sistema di colorazione decente per 64 canali
	
	int channel;
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		
		//printf("Analysis from channel %d to %d (geo %d)\n",geo*64,geo*64+63,geo);
		
		
		TCanvas* mycanv = new TCanvas("mycanv","Card Representation",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,1.5*SIZEOFCANV); 

		TH1F *htemp;
		
		mycanv->Update();
		for (int i = 0; i<64; i++) {
			channel = geo*64+i;
			if (i==0) {
				
				// OPT1 (PRESO DA PLOT CHANNEL)
				//  hcharge e' un TH1 fillato dal TTre Draw
				//	hcharge->GetXaxis()->SetRange(xmin,xmax);
				//  hcharge->SetMaximum(ymax);
				// 	hcharge->SetFillColor(kYellow-10);
				//  hcharge->GetXaxis()->SetTitle("Charge [ADC units]");
				//  hcharge->GetYaxis()->SetTitle("Occurrency [#]");
				//

				ftree->Draw(Form("HITEntries[%d]/Duration:DAC",channel));
			
				
				htemp = (TH1F*)gPad->GetPrimitive("htemp");
				htemp->GetXaxis()->SetRangeUser(200,500);
				htemp->GetYaxis()->SetRangeUser(0,200);
			}else {
				ftree->SetMarkerColor(100-i); // modify! change color depending on channel something like (ch_id%8)==0
				ftree->SetMarkerStyle(20);
				ftree->SetMarkerSize(0.5);
			
				ftree->Draw(Form("HITEntries[%d]/Duration:DAC",channel),"","SAME");
			}
		} // end of loop on channels
		
	//	mycanv->DrawClone();
	//	mycanv->Close();
	//	delete mycanv;
	}else {
		printf("File %s not opened \n",foutfile);
	}
	delete ffile;
}





void TMRCScanner::Cmp(const char * rootfile1,
					  const char * rootfile2,
					  const char * rootfile3,
					 const char * rootfile4,
					 const char * rootfile5,
					 // const char * rootfile6,
					 // const char * rootfile7,
					 // const char * rootfile8,
					 // const char * rootfile9,
					  const char * what)
{
	//Int_t icol10;
	TCanvas *c1 = new TCanvas("c1","c1",600,500);
	TLegend* leg = new TLegend(0.1,0.7,0.48,0.9);
	leg->SetHeader("Gain");
	
	
	Color_t color1 = kRed;	//2332
	Color_t color2 = kYellow;	//2632
	Color_t color3 = kMagenta;   // 3041
	
	Color_t color4 = kBlue;
	Color_t color5 = kCyan;
	// Color_t color6 = kGreen+1;
//	Color_t color7 = kCyan;
//	Color_t color8 = kBlue;
//	Color_t color9 = kMagenta;

	
	TFile* f1 = new TFile(rootfile1);
	if (f1->IsOpen()) {
		TTree * mytree1 = (TTree*) f1->Get("T");
		mytree1->SetMarkerColor(color1);	
		mytree1->SetMarkerStyle(6);
		mytree1->Draw(what,"HITEntries!=0");
	//	leg->AddEntry(mytree1,rootfile1,"P");
	//	leg->AddEntry(mytree1,"HV off, otherwise 1000","P");
		// leg->AddEntry(mytree1,"Gain = 1 (gword=64)","P");
		leg->AddEntry(mytree1,"Gain = 0.5","P");

		
		TFile* f2 = new TFile(rootfile2);
		if (f2->IsOpen()) {
			TTree * mytree2 = (TTree*) f2->Get("T");
			mytree2->SetMarkerColor(color2);	
			mytree2->SetMarkerStyle(6);
			mytree2->Draw(what,"HITEntries!=0","SAME");
			//leg->AddEntry(mytree2,rootfile2,"P");
			// leg->AddEntry(mytree2,"Gain = 4 (gword=255)","P");	
		leg->AddEntry(mytree2,"Gain = 1.0","P");

		}
		TFile* f3 = new TFile(rootfile3);
		if (f3->IsOpen()) {
			TTree * mytree3 = (TTree*) f3->Get("T");
			mytree3->SetMarkerColor(color3);	
			mytree3->SetMarkerStyle(6);
			mytree3->Draw(what,"HITEntries!=0","SAME");
		//	leg->AddEntry(mytree3,rootfile3,"P");
			// leg->AddEntry(mytree3,"Gain = 1/16 (gword=4)","P");	
		leg->AddEntry(mytree3,"Gain = 1.5","P");

		}
		
		
		TFile* f4 = new TFile(rootfile4);
		if (f4->IsOpen()) {
			TTree * mytree4 = (TTree*) f4->Get("T");
			mytree4->SetMarkerColor(color4);	
			mytree4->SetMarkerStyle(6);
			mytree4->Draw(what,"HITEntries!=0","SAME");
		//	leg->AddEntry(mytree4,rootfile4,"P");
			// leg->AddEntry(mytree4,"Gain = 3","P");
		leg->AddEntry(mytree4,"Gain = 2.0","P");
		}
		TFile* f5 = new TFile(rootfile5);
		if (f5->IsOpen()) {
			TTree * mytree5 = (TTree*) f5->Get("T");
			mytree5->SetMarkerColor(color5);	
			mytree5->SetMarkerStyle(6);
			mytree5->Draw(what,"HITEntries!=0","SAME");
			//leg->AddEntry(mytree5,rootfile5,"P");
			// leg->AddEntry(mytree5,"Gain = 4 (max)","P");
		leg->AddEntry(mytree5,"Gain = 3.0","P");	
		}
		// TFile* f6 = new TFile(rootfile6);
		// if (f6->IsOpen()) {
		// 	TTree * mytree6 = (TTree*) f6->Get("T");
		// 	mytree6->SetMarkerColor(color6);	
		// 	mytree6->SetMarkerStyle(22);
		// 	mytree6->Draw(what,"","SAME");
		// 	//leg->AddEntry(mytree6,rootfile6,"P");
		// 	// leg->AddEntry(mytree6,"Gain = 1/2","P");
		// leg->AddEntry(mytree6,"Gain = 4.0","P");
	
		// }
		/*
		TFile* f7 = new TFile(rootfile7);
		if (f7->IsOpen()) {
			TTree * mytree7 = (TTree*) f7->Get("T");
			mytree7->SetMarkerColor(color7);	
			mytree7->SetMarkerStyle(22);
			mytree7->Draw(what,"","SAME");
			//leg->AddEntry(mytree7,rootfile7,"P");
			leg->AddEntry(mytree7,"Gain = 1/8","P");	
		}
		TFile* f8 = new TFile(rootfile8);
		if (f8->IsOpen()) {
			TTree * mytree8 = (TTree*) f8->Get("T");
			mytree8->SetMarkerColor(color8);	
			mytree8->SetMarkerStyle(22);
			mytree8->Draw(what,"","SAME");
			//leg->AddEntry(mytree8,rootfile8,"P");
			leg->AddEntry(mytree8,"Gain = 1/16","P");	
		}
		TFile* f9 = new TFile(rootfile9);
		if (f9->IsOpen()) {
			TTree * mytree9 = (TTree*) f9->Get("T");
			mytree9->SetMarkerColor(color9);	
			mytree9->SetMarkerStyle(22);
			mytree9->Draw(what,"","SAME");
			//leg->AddEntry(mytree9,rootfile9,"P");
			leg->AddEntry(mytree9,"Gain = 1/32","P");	
		}
		 /
		*/
	}

	leg->Draw();
	c1->DrawClone();
	c1->Close();
	
}


/*
void TMRCScanner::CmpRatevsDAC(const char * rootfile1,const char * rootfile2,const char * rootfile3)
{
	Cmp("Rate:DAC",rootfile1,rootfile2,rootfile3);
}

 */




void TMRCScanner::SuperImpose64ch(const char * what,const char * vs,int geoaddr)
{
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		ftree->SetMarkerStyle(23);
		
		char nomeout[256];
		TCanvas* mycanv = new TCanvas("mycanv");
		
		for (int ch=0; ch<64; ch++) {
			int channel = geoaddr*64+ch;
			/* Superimpose
			ftree->SetMarkerColor(ch%16); // modify! change color depending on channel something like (ch_id%8)==0
			ftree->SetMarkerStyle((ch%8)+20);
			if (ch==0) {
				ftree->Draw(Form("RatioHITADC[%d]:DAC",channel));
			}else {
				ftree->Draw(Form("RatioHITADC[%d]:DAC",channel),"","SAME");
			}
			*/
			/* Print on File (.eps)*/
			ftree->Draw(Form("%s[%d]:%s",what,channel,vs));
			sprintf(nomeout,"%s%s_%s_CH%d.eps",foutfile,what,vs,channel); 
			mycanv->Print(nomeout);
			
		}
		mycanv->Close();
		
	}else {
		printf("File %s not opened, first call OpenFile() \n",foutfile);
	}
	//ftree->Print();	
	delete ffile;

}


/**********/
/* HERE!  */
/**********/

void TMRCScanner::Derive()
{
		// Calculate discrete derivative of the variable "Rate" adding a branch called deltarate
	
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T");
		
		
		// opt1 
		// leggere una entry alla volta, calcolare la derivata, scriverla in una nuova branca
		
		float rate =0.0;	// for reading the Tree
		float rate_old = 0.0; // to compute "derivative"
		
		ftree->SetBranchAddress("Rate",&rate); // collegamento tra la branca del tree "Rate" e la variabile d'appoggio
		
		int nentries = (int) ftree->GetEntries(); 
		
		printf("Points = %d\n", nentries);
		
		for (int i = 0; i < nentries; i++){
			
			rate_old = rate;

			ftree->GetEntry(i);

			printf("Run[%d] Current Rate = %3.1f\t Old Rate = %3.1f \t difference = %3.1f\n",i,rate, rate_old, rate-rate_old);

		}
		
		// save only the new version of the tree
		//t3->Write("", TObject::kOverwrite);
	
		
		// opt2
		// riversare su TH1 opportuno, calcolare le differenze e fillare su un altro TH1 (derivato), salvare la derivata dentro il rootfile.
		
		
		
	}else {
		cout << "Problem opening the file"<<foutfile<<endl;
	}
}



void TMRCScanner::TrigEff()
{
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		ftree->SetMarkerStyle(23);
	
		/* Draw Single Channel*/
		ftree->Draw("RatioHITADC[2048]:DAC");	
		
		/* Adding some others*/
		for (int i=2048; i<2048+3; i++) {
			printf("channel %d added\n",i);
			ftree->SetMarkerColor(i-2047); // modify! change color depending on channel something like (ch_id%8)==0
			ftree->SetMarkerStyle(10);
			ftree->Draw(Form("RatioHITADC[%d]:DAC",i),"","SAME");				
		}
	}else {
		printf("Problem opening the file %s\n",foutfile);
	}	
	delete ffile;
}	


void TMRCScanner::Shape(int channel)
{
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		printf("file aperto\n");
		ftree = (TTree*) ffile->Get("T"); 
		ftree->SetMarkerSize(.5);
		ftree->SetMarkerStyle(7);
		ftree->Draw(Form("ADCMean[%d]:DAC",channel));
		//ftree->SetMarkerColor(kRed);	
		//ftree->Draw(Form("ADCMin[%d]:DAC",channel),"","SAME");
		//ftree->Draw(Form("ADCMax[%d]:DAC",channel),"","SAME");	
		ftree->SetMarkerSize(.6);
		ftree->SetMarkerStyle(22);
		ftree->SetMarkerColor(kBlue);	
		ftree->Draw(Form("HITMean[%d]:DAC",channel),"","SAME");
		//ftree->SetMarkerColor(kGreen);		
		//ftree->Draw(Form("HITMin[%d]:DAC",channel),"","SAME");
		//ftree->Draw(Form("HITMax[%d]:DAC",channel),"","SAME");	
		//ftree->Draw(Form("ADCRms[%d]:DAC",channel),"","SAME");
	}else {
		printf("problemi ad aprire il file\n");
	}
	//ftree->Print();	
	delete ffile;
}




void TMRCScanner::HITSummary(int channel)
{
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		ftree->SetMarkerStyle(23);
		Double_t w =600;
		Double_t h = 600;
		TCanvas * myc = new TCanvas("c",Form("HIT Summary - channel %d",channel),w,h);
		myc->SetWindowSize(w+(w-myc->GetWw()),h +(h-myc->GetWh()));
		myc->Divide(3,2);
		myc->Update();
		myc->cd(1);
		myc->Update();
		ftree->Draw(Form("HITEntries[%d]:DAC",channel));
		myc->cd(6);
		myc->Update();
		ftree->Draw(Form("HITMin[%d]:DAC",channel));
		myc->cd(3);
		myc->Update();
		ftree->Draw(Form("HITMax[%d]:DAC",channel));
		myc->cd(2);
		myc->Update();
		ftree->Draw(Form("HITMean[%d]:DAC",channel));
		myc->cd(5);
		myc->Update();
		ftree->Draw(Form("HITRms[%d]:DAC",channel));
	}else {
		printf("File %s not opened \n",foutfile);
	}
	delete ffile;
}




void TMRCScanner::PlotMultiplicity(int opt){
	
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		ftree->SetMarkerStyle(23);
		
		switch (opt) {
			case 0:
				ftree->Draw("HitMultMean:HitMultMeanErr:DAC","","goff");
				break;
			case 1:
				ftree->Draw("HitMultRMS:HitMultRMSErr:DAC","","goff");
				break;				
			default:
				break;
		}
		
		TGraphErrors *gr = new TGraphErrors(ftree->GetSelectedRows(),ftree->GetV3(), ftree->GetV1(),NULL,ftree->GetV2());
		gr->SetTitle("TGraphErrors Example");
		gr->SetMarkerColor(4);
		gr->SetMarkerStyle(21);
		gr->Draw("ALP");
		
	}else {
		printf("File %s not opened \n",foutfile);
	}
	delete ffile;
}



