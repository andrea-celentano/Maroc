#include"TMRCScanner.h"



TMRCScanner::TMRCScanner(const char * prefix, int firstrun, int lastrun, int nbulk)
{
	ResetSingleRunVariables();
	
	fNRun=0;
	ffirstRun=0;
	for (int run=0; run<MAXRUN; run++) {fdly[run]=0;}
	for (int c=0; c<MAX_NAME_LENGH; c++) {foutfile[c]=0;}

	
	if (FixRunNames(prefix,firstrun,lastrun)==true) {
		MRC_INF("Starting the analysis of the specified run collection\n");
		Scan(nbulk);
	}else {
		MRC_ERR("Incorrect sequence of runs ID. %d is not greter then %d\n",lastrun,firstrun);
	}	
}

TMRCScanner::~TMRCScanner()
{
	MRC_INF("Destroying TMRCScanner: nothing to do \n");
}


void TMRCScanner::ResetSingleRunVariables()
{
	frun			=0;
	fdelay			=0;
	fdac			=0;
	fDuration		=0.0;				
	fPollEff		=0.0;
	frate			=0.0;
	fNevents		=0;			
	fFeNum			=0;
	
	fHitMultMean	=0.;	
	fHitMultMeanErr	=0.;	
	fHitMultRMS		=0.;		
	fHitMultRMSErr	=0.;	

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
			fADCEntries[chglob]	=A->GetADCEntries(chglob);
			fHITEntries[chglob]	=A->GetHITEntries(chglob);
			
			/*
			fADCMean[chglob]	=1.;
			fADCRms[chglob]		=2.;
			fADCMax[chglob]		=3.;	
			fADCMin[chglob]		=4.;
			fHITMean[chglob]	=5.;
			fHITRms[chglob]		=6.;
			fHITMax[chglob]		=7.;
			fHITMin[chglob]		=8.;
			fADCEntries[chglob]	=66;
			fHITEntries[chglob]	=60;;
			 */
		
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
	
	InitTree(nbulk);
	
	int idx_run; 
	for (idx_run=0; idx_run<fNRun; idx_run++) { // loop on Run
		
		MRC_INF("Run %3d of %d File: %s\n",idx_run+1,fNRun,fcurrRun[idx_run]);
		
		SingleRunAnalysis(idx_run,nbulk);
		
		ffile->cd();
		ftree->Fill();
		ResetSingleRunVariables();
	} 
	ftree->Write();
	ftree->Scan("Run:DAC:Rate");
	ffile->Close();
	printf("Success, data on %s\n",foutfile);
	return idx_run; 
}


void TMRCScanner::InitTree(int nbulk){
	
	ftree->Branch("Run"				,&frun				,"run/I"			);
	ftree->Branch("Rate"			,&frate				,"rate/F"			);
	ftree->Branch("DAC"				,&fdac				,"dac/I"			);
	ftree->Branch("Delay"			,&fdelay			,"delay/I"			);	 // HOLD1_Delay
	ftree->Branch("Duration"		,&fDuration			,"Duration/F"		);	 
	ftree->Branch("PollEff"			,&fPollEff			,"PollEff/F"		);	 
	ftree->Branch("Events"			,&fNevents			,"Events/I"			);	 
	ftree->Branch("FEnum"			,&fFeNum			,"FEnum/I"			);	
	ftree->Branch("HitMultMean"		,&fHitMultMean		,"HitMultMean/F"	);
	ftree->Branch("HitMultMeanErr"	,&fHitMultMeanErr	,"HitMultMeanErr/F"	);
	ftree->Branch("HitMultRMS"		,&fHitMultRMS		,"HitMultRMS/F"		);
	ftree->Branch("HitMultRMSErr"	,&fHitMultRMSErr	,"HitMultRMSErr/F"	);	
	if (nbulk) {
		ftree->Branch("ADCEntries"	,fADCEntries		,"ADCEntries[4096]/I");
		ftree->Branch("ADCMean"		,fADCMean			,"ADCMean[4096]/F"	);
		ftree->Branch("ADCRms"		,fADCRms			,"ADCRms[4096]/F"	);
		ftree->Branch("ADCMax"		,fADCMax			,"ADCMax[4096]/F"	);
		ftree->Branch("ADCMin"		,fADCMin			,"ADCMin[4096]/F"	);
		
		ftree->Branch("HITEntries"	,fHITEntries		,"HITEntries[4096]/I");
		ftree->Branch("HITMean"		,fHITMean			,"HITMean[4096]/F"	);
		ftree->Branch("HITRms"		,fHITRms			,"HITRms[4096]/F"	);
		ftree->Branch("HITMax"		,fHITMax			,"HITMax[4096]/F"	);
		ftree->Branch("HITMin"		,fHITMin			,"HITMin[4096]/F"	);
		ftree->Branch("RatioHITADC"	,fRatioHITADC		,"RatioHITADC[4096]/F");
	}
}

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

TMRCScanner::TMRCScanner()
{
	MRC_INF("Ready to recall a sequence of run, use Recall() \n");
}

void TMRCScanner::Recall(const char * oldrootfile)
{
	sprintf(foutfile,"%s",oldrootfile);	
	MRC_INF("Filename %s setted, ready to Plot() \n",foutfile);
}


void TMRCScanner::PlotMultiplicity(int opt){
	
	ffile = new TFile(foutfile);
	if (ffile->IsOpen()) {
		ftree = (TTree*) ffile->Get("T"); 
		TGraphErrors *gr;
		switch (opt) {			
			case 1:
				ftree->Draw("HitMultRMS:HitMultRMSErr:DAC","","goff");
				gr = new TGraphErrors(ftree->GetSelectedRows(),ftree->GetV3(), ftree->GetV1(),NULL,ftree->GetV2());	
				gr->SetTitle("Event Multiplicity - RMS");
				break;	
			case 2:
				ftree->Draw("HitMultMean:HitMultMeanErr:HitMultRMS:HitMultRMSErr","","goff");	
				gr = new TGraphErrors(ftree->GetSelectedRows(),ftree->GetV1(), ftree->GetV3(),ftree->GetV2(),ftree->GetV4());
				gr->SetTitle("Event Multiplicity - Correlation Mean RMS");
				break;
			case 3:
				ftree->Draw("HitMultMean:HitMultRMS:DAC","","goff");	
				gr = new TGraphErrors(ftree->GetSelectedRows(),ftree->GetV3(), ftree->GetV1(),NULL,ftree->GetV2());
				gr->SetTitle("Event Multiplicity - Correlation Mean RMS");
				break;				
			default:
				ftree->Draw("HitMultMean:HitMultMeanErr:DAC","","goff");
				gr = new TGraphErrors(ftree->GetSelectedRows(),ftree->GetV3(), ftree->GetV1(),NULL,ftree->GetV2());
				gr->SetTitle("Event Multiplicity - Mean");
				break;
				break;
		}
		gr->SetMarkerColor(4);
		gr->SetMarkerStyle(7);
		gr->Draw("AP");
		
	}else {
		printf("File %s not opened \n",foutfile);
	}
	delete ffile;
}



