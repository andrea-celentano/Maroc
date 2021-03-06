#include"TAnalysis.h"
#include <math.h>
#include <TStyle.h>

int TAnalysis::Read(unsigned int block_to_read,int chGroupSize){
	char			fname_to_read[512];
	unsigned short	nwords;	
	unsigned char	daq_data_vector[200000]; 
	TBlock *		Block;
	ifstream		in;				
	TStopwatch		timer;			
	fparser = new TFileRaw(); 
	fevent = new TEvent();
	fevent->OpenFile(fnomeroot);
	fevent->CreateTree();
	fevent->Clean();
	check_zip(fnomebin, fname_to_read);	
	timer.Start();
	int idx=0;
	
	if (chGroupSize) {fCNSAnalizer = new CNSAnalysis(chGroupSize);}
	
	in.open( fname_to_read, ios::binary );
	if(!in.good())	{// check for i/o operations with the file 
		cout << "Can't open input file " << fname_to_read << endl; 
		return -1;
	}else{
		in.read((char *)&nwords,2);	
		while( in.good() && (fparser->GetBlockIndex() < block_to_read) ){
			MRC_DUM("n_word = %d, idx = %d\n",nwords,idx);
			if ((fparser->GetBlockIndex())%1000==0) {
				MRC_DBG("Parsing Block %ld\n",fparser->GetBlockIndex());
			}
			fparser->IncrBlock();
			in.read( (char*)daq_data_vector, 4 * nwords ); 
			Block = new TBlock(daq_data_vector,nwords,fevent,idx,fCNSAnalizer); 
			delete Block;
			Block = 0;			
			in.read( (char*)&nwords, 2 ); 
			idx++;
		}//while end
	} // else end
	in.close();
	in.clear();// Control bit reset	
	timer.Stop();
	if(fdebugmode){
		PrintFinalDebug(timer);
		fevent->PrintTree();
	 }
	if (chGroupSize) {
		fCNSAnalizer->Plot();
	}
	DestroyCNSAnalizer();
	fevent->WriteTreeOnDisk();
	fevent->CloseFile();

	DestroyParser();
	DestroyEvent();

	return idx;
}

int TAnalysis::ReadDGN(unsigned int block_to_read){	
	char			fname_to_read[512]; 
	unsigned char	daq_data_vector[200000];
	unsigned short	nwords;				// numero di parole del prossimo pacchetto da leggere// da passare al metodo di FileStat che ricerca la Nword massima.
	TBlock *		Block;
	ifstream		in;				// allocazione di un oggetto di tipo 'ifstream' chiamato 'in' (in  C++ ifstream fornisce una interfaccia per leggere dati ad un file come flusso d'ingresso)
	TStopwatch		timer;			// allocazione di un oggetto di tipo 'TStopwatch' chiamato 'timer'  (ROOT Library)
	fparser = new TFileRaw();
	if(fdbgParser){fparser->SetDebugMode();}
	fevent = new TEvent();
	if (fdbgEvent){fevent->SetDebugMode();}
	fevent->SetPrintLevel(fPLEvent);//0,2,4,8
	fevent->OpenFile(fnomeroot);
	fevent->CreateTree();
	fevent->Clean();
	check_zip(fnomebin, fname_to_read);			// Eventuale decompressione
	timer.Start();
	int tour;
	for(tour=0; tour<2; tour++)
	{
		in.open( fname_to_read, ios::binary );
		if(!in.good())	{// check for i/o operations with the file 
			cout << "Can't open input file " << fname_to_read << endl; 
			return -1;
		}else{
			in.read((char *)&nwords,2);	
			while( in.good() && (fparser->GetBlockIndex() < block_to_read) ){
				in.read( (char*)daq_data_vector, 4 * nwords ); 
				fparser->Set_Block_NWord(nwords);
				fparser->IncrBlock();
				if(tour==1){fparser->FillB_Nword();}
				//if(ped_sub==0){
				//	fcalibration = NULL;
				//}else{ 
				//	fcalibration = new TCalib();
				//	char nomemappa[100];
				//	sprintf(nomemappa,"%s",fpedestalmap);
				//	fcalibration->Import(nomemappa,0); //fcalibration->Import("ped11july10.txt",0);
				//}
				//printf("prima del blocco, NWORDS = %d\n",nwords);
				//printf("tour=%d\n", tour);
				//for (int i=0; i<nwords*4; i++) {
				//	if (i%4==0) {
				//		printf("Word[%d] = 0x%X",i/4,daq_data_vector[i]);
				//	}else {
				//		printf("%X",daq_data_vector[i]);
				//	}
					
				//	if (i%4==3) {
				//		printf("\n");
				//	}
				//}
				fparser->Print();
				Block = new TBlock(tour,daq_data_vector,fevent,fparser); 
				delete Block;
				//printf("dopo il blocco\n");

				Block = 0;
				fparser->Print(); // see TFileRaw implementation
				if(tour==0){fparser->Compare_Block_Maxima();}
				if(tour==1){fparser->FillB_NEvent();}	
				in.read( (char*)&nwords, 2 ); 		
			}//while end
		} // else end
		in.close();
		in.clear();// Control bit reset	
		if(tour==0){
			fparser->Init_Histo();
			fparser->ResetFileLevel();
			fparser->Reset_FSM();
			fparser->Reset_Counters();
		}
	}// for end 
	timer.Stop();
	if(fdebugmode){
		PrintFinalDebug(timer);
		fparser->PrintStat();
		fevent->PrintTree();
	}
	fevent->WriteTreeOnDisk();
	fparser->Init_Chip_Histo();
	fparser->GraphStat();
	fparser->DrawBlockInfo();
	fparser->WriteHistoOnDisk();
	fevent->CloseFile();
	DestroyParser();
	DestroyEvent();
	return 0;
};

bool TAnalysis::SystemProfile(int a, int b,double rmslow,double rmsup,double meanlow,double	meanup){
	if (!this->LoadTree(fnomeroot)) {
		return false;
	}else{
		//in ptot  x= channel; y = ADC 
		TProfile * ptot = new TProfile("ptot","Profile",fchlast - fchfirst,fchfirst-.5,fchlast-0.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
		ptot->SetErrorOption("s");
		ftree->Draw("ADC:Iteration$>>ptot","","goff,profs");
		TGraph *ped = new TGraph(ptot);  // x = channel ID; y= MEAN adc 
		TGraph *rped = new TGraph(ptot); // x = channel ID; y= RMS 
		
		ped->SetTitle("Mean Value");
		rped->SetTitle("Fluctuation");
		
			
		double rangeMean = meanup- meanlow;
		double rangeRMS  =  rmsup - rmslow;
		
		TH1F * meansys = new TH1F("meansys","",ceil(rangeMean)/a,meanlow,meanup);
		TH1F * rmssys = new TH1F("rmssys","",b*ceil(rangeRMS),rmslow,rmsup);	

		
		for (int i=0; i<(fchlast - fchfirst); i++) {  
			// replace trick
			Double_t xa,ya,za;
			rped->GetPoint(i,xa,ya); // xa = channel ID; ya = mean ADC 
			za = ptot->GetBinError(i+1);
			if (za!=0.0) {
				rped->SetPoint(i,xa,za);
				rmssys->Fill(za);
				meansys->Fill(ya);
			}

		}
		TCanvas *mc1 = new TCanvas("mc1","M3FE characteristics",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,2*SIZEOFCANV);
		mc1->Divide(2,2);
		
		mc1->cd(1);
		ped->Draw("PAW");
		TAxis *xax1 = ped->GetXaxis();
		xax1->SetTitle("Channel ID");
		TAxis *yax1 = ped->GetYaxis();
		yax1->SetTitle("Mean Charge - ADC unit");
		
		mc1->cd(2);
		rped->Draw("PAW");
		TAxis *xax2 = rped->GetXaxis();
		xax2->SetTitle("Channel ID");
		TAxis *yax2 = rped->GetYaxis();
		yax2->SetTitle("Charge Fluctuation - ADC unit");
	
		mc1->cd(3);
		meansys->Draw();
		TAxis *xax3 = meansys->GetXaxis();
		xax3->SetTitle("Charge - ADC unit");
		TAxis *yax3 = meansys->GetYaxis();
		yax3->SetTitle("Occurrency");
		
		
		mc1->cd(4);
		rmssys->Draw();	
		TAxis *xax4 = rmssys->GetXaxis();
		xax4->SetTitle("Noise - ADC unit");
		TAxis *yax4 = rmssys->GetYaxis();
		yax4->SetTitle("Occurrency");
		
		
		mc1->Print("./eps/pippo.eps");
		return true; 
	}
}

bool  TAnalysis::CheckBinSize(TH1F * h, Float_t val){
	bool lsuccess = true;
	Double_t rval =-1;
	TH1F * lh = h;
	rval = lh->GetBinWidth(3); // width of bin 3, all bins from 1 to nBIN
	if (rval!=val) {
		lsuccess = false;
		printf("Bin size = %3.3f\n",rval);
	}
	return lsuccess;
}

TF1* TAnalysis::FitSignal(TH1F * hADC){
	
	TH1F* lhADC = hADC;
	return FitSingleGaussian(lhADC);
} 

TF1* TAnalysis::FitSingleGaussian(TH1F * hADC){
	
	TH1F* lhADC = hADC;
	
	Int_t mean  = 1500;
	Int_t norm  = lhADC->GetBinContent(lhADC->GetBin(mean));
	if (norm==0) {
		norm =1;
	}
	Int_t sigma = 100;
	//printf("INITIAL norm %d at bin %d\n",norm,lhADC->GetMaximumBin());
	TF1 *fgau = new TF1("f","gaus(0)",ADCFIRST,ADCLAST);
	fgau->SetNpx(1000); // ,meglio ADCLAST-ADCFIRST ??
	fgau->SetLineColor(3);
	fgau->SetParameters(norm,mean,sigma); 
	fgau->SetParLimits(1,300,2600);
	fgau->SetParLimits(2,0,500);
	lhADC->Fit(fgau,"Q,0");
	
	
	return fgau;
} 

TF1* TAnalysis::FitPedestal(TH1F * hADC){
	
	TH1F* lhADC = hADC;
	Int_t norm  = lhADC->GetBinContent(lhADC->GetMaximumBin());

	//printf("INITIAL norm %d at bin %d\n",norm,lhADC->GetMaximumBin());
	Int_t mean = lhADC->GetMean();	
	//Int_t mean  = xmin+lhADC->GetBinWidth(1)*lhADC->GetMaximumBin();
	Int_t sigma = lhADC->GetRMS();
	TF1 *fgau = new TF1("f","gaus(0)",ADCFIRST, ADCLAST);
	fgau->SetNpx(1000);
	fgau->SetLineColor(3);
	fgau->SetParameters(norm,mean,sigma); 
	lhADC->Fit(fgau,"Q,0");	//lhADC->Fit(fgau,"M,0");
	return fgau;
} 

void TAnalysis::SetYMax(Int_t ymax){
	fymax = ymax;
} 

void TAnalysis::SetRebin(Int_t rebin){
	frebin = rebin;
}

bool TAnalysis::SetGraphicsON(bool enable_grafix){
	fgraficON = enable_grafix;
	return fgraficON;
}

bool TAnalysis::SetFitON(bool enable_fit){
	ffitON = enable_fit;
	return ffitON;
}
void TAnalysis::MakeUpMultiplicity(TH1I * h){

	TH1I * lh = h;
	lh->GetXaxis()->SetTitle("Multiplicity [#]");
	lh->GetYaxis()->SetTitle("Events [#]");
	lh->SetFillColor(kGreen);
	//h->SetStats(0);
	lh->SetBarWidth(0.80);
	lh->SetBarOffset(0.1);
	lh->SetMarkerSize(1.1);
}


void TAnalysis::MakeUp(TH1F * h,int hit){
	
	TH1F * lh = h;
	lh->GetXaxis()->SetTitle("Charge [ADC unit]");
	lh->GetYaxis()->SetTitle("Counts [#]");
	lh->SetMaximum(fymax);
	lh->Rebin(frebin);	
	
	switch (hit) {
		case 0:lh->SetFillColor(kYellow);break;
		case 1:lh->SetFillColor(kRed);break;	
		default: lh->SetFillColor(kYellow-10);break;
	}
}

void TAnalysis::StoreFitResults(TF1 * formula,int channel){
	if (formula!=NULL) {
		Double_t norm		= formula->GetParameter(0);
		Double_t normErr	= formula->GetParError(0);	
		Double_t mean		= formula->GetParameter(1);
		Double_t meanErr	= formula->GetParError(1);	
		Double_t sigma		= formula->GetParameter(2);
		Double_t sigmaErr	= formula->GetParError(3);	
		Double_t Area		= formula->Integral(mean-3*sigma,mean+3*sigma);
		Double_t chi2		= formula->GetChisquare();

	
		if (norm<=2.) {
			printf("Low statistics, unreliable fit\n");
		}else {
			printf("Channel %d Fit   ", channel);
			printf("Mean = %3.2lf(%3.2lf) ",mean,meanErr);
			printf("Sigma = %3.2lf(%3.2lf) ",sigma,sigmaErr);
			printf("Norm = %3.0lf(%3.0lf) ",norm,normErr);
			printf("Chi2 = %6.0f",chi2);
			printf("Area = %3.2lf()",Area);
			printf("\n");
		}
	}
}

void TAnalysis::SetChannel1(int channel,int hit,int xmin,int xmax){

	SetCh1(channel); 
	SetCh1ADCmin(xmin);	
	SetCh1ADCmax(xmax);
	SetCh1HIT(hit);	
}

void TAnalysis::SetChannel2(int channel2,int hit2,int xmin2,int xmax2){

	SetCh2(channel2);
	SetCh2ADCmin(xmin2);	
	SetCh2ADCmax(xmax2);
	SetCh2HIT(hit2);
}

void TAnalysis::SetMultiplicity(int m){
	fm = m;
}

Double_t TAnalysis::PlotADC(int channel,int hit,int xmin,int xmax,int m,const char * relm,int channel2,int hitch2,int xmin2,int xmax2,int opt){
	
	SetChannel1(channel,hit,xmin,xmax);
	SetChannel2(channel2,hitch2,xmin2,xmax2);
	SetMultiplicity(m);
	
	TString cutStr, titleStr;
	TF1 * lfit= NULL;
	Int_t n = 0;
 	TCut c1 = Form("hit[%d]==0",fch1);
 	TCut c2 = Form("hit[%d]==1",fch1);
	TCut c3 = Form("hit[%d]==0",fch2);
	TCut c4 = Form("hit[%d]==1",fch2);
	TCut c5 = Form("ADC[%d]>=%d",fch1,fADCmin1);
	TCut c6 = Form("ADC[%d]<=%d",fch1,fADCmax1);
	TCut c7 = Form("ADC[%d]>=%d",fch2,fADCmin2);
	TCut c8 = Form("ADC[%d]<=%d",fch2,fADCmax2);
	TCut crange = c5&&c6&&c7&&c8;
	TCut cmult; 
	TCut cut;	
	if (this->LoadTree(fnomeroot)) {
		if (this->IsGoodSystemChannel(fch1)&&this->IsGoodSystemChannel(fch2)) {
			switch (fhit1) {
				case 0:
					cut = c1;
					switch (fhit2) {
						case 0: cut = cut&&c3;break;
						case 1: cut = cut&&c4;break;
						default:cut = c1;break;
					}
					titleStr.Form("%s  Charge when hit==0  Channel %d",fRunName,fch1);
					break;
				case 1:
					cut = c2;
					switch (fhit2) {
						case 0: cut = cut&&c3;break;
						case 1: cut = cut&&c4;break;
						default:cut = c2;break;
					}
					titleStr.Form("%s Ch%d hit==1",fRunName,fch1);
					break;
				default: 
					cut = "";
					titleStr.Form("%s  Channel %d",fRunName,fch1);
					break;
			}
			if(fm!=-1){
				if (fm==1) {
					cmult = Form("EvtMultiplicity%s%d",relm,fm);
				}else {
					cmult = Form("EvtMultiplicity%s%d",relm,fm);
				}
				cut = cut&&cmult;
			}
			
			if (opt==0) { // SINGLE CHANNEL PLOT
				TH1F * hADC = new TH1F("hADC",titleStr,fADCmax1-fADCmin1,fADCmin1-0.5,fADCmax1-0.5);
				//n = ftree->Draw(Form("ADC[%d]>>hADC",fch1),cut&&crange,"goff");	
				n = ftree->Draw(Form("ADC[%d]>>hADC",fch1),cut&&crange,"goff");
					if (ffitON) {
					switch (fhit1) {
						case 0:lfit =FitPedestal(hADC); break;
						case 1:lfit = FitSingleGaussian(hADC);break;
						default:lfit = FitSignal(hADC);break;
					}
					StoreFitResults(lfit,fch1);
				}
				if (fgraficON) {
					MakeUp(hADC,fhit1);	
					CheckBinSize(hADC,1.000);
					TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
					TCanvas* mycanv = new TCanvas("mycanv","Single Channel ADC Spectrum",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
					mycanv->cd(1)->SetLogy(1);
					hADC->DrawCopy();
					if (lfit!=NULL) {
						lfit->Draw("same");
					}
				}
			}else { // SCATTER PLOT
				TH2F * h2ADC = new TH2F("h2ADC",titleStr,fADCmax1-fADCmin1,fADCmin1-0.5,fADCmax1-0.5,fADCmax2-fADCmin2,fADCmin2-0.5,fADCmax2-0.5);
				n = ftree->Draw(Form("ADC[%d]:ADC[%d]>>h2ADC",fch1,fch2),cut&&crange,"goff");
				if (fgraficON) {
					h2ADC->SetMarkerStyle(20);
					h2ADC->SetMarkerSize(0.5);
					TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
					TCanvas* mycanv = new TCanvas("mycanv","Scatter ADC Spectrum",PIXX-SIZEOFCANV,0,SIZEOFCANV,SIZEOFCANV);
					mycanv->cd();
					h2ADC->DrawCopy();
				}
			}

		}else {
			MRC_ERR("Bad Channel %d\n",fch1);
			return -2;
		}
	}else {
		MRC_ERR("Bad File %s\n",fnomeroot);
		return -1;
	}
	this->CloseRootFile();
	Double_t nn = (Double_t) n;
	return nn;
}

bool TAnalysis::PlotChannel(int channel,bool save, int xmin,int xmax,int ymax, int multiplicity){	
	int idx = channel-1;

	TF1 *fgau3 = new TF1("fgau3","gaus(0)+gaus(3)",1200., 1800.);
	fgau3->SetNpx(1200);
	fgau3->SetLineColor(3);
	fgau3->SetParameters(10,1320,10,3,1503,100); // ymax, mean, sigma	
	fgau3->SetParLimits(1,1200,1400);
	fgau3->SetParLimits(4,1600,1700);
	
	TF1 *fgau1a = new TF1("fgau1a","gaus(0)",1200., 1800.);
	TF1 *fgau1b = new TF1("fgau1b","gaus(0)",1200., 1800.);
	fgau1a->SetNpx(1200);
	fgau1b->SetNpx(1200);
	
	if (!this->LoadTree(fnomeroot)) {
		MRC_ERR("Problem loading rootfile\n");
		return false;
	}else{
		
		if (!this->IsGoodSystemChannel(channel)){
			MRC_ERR("Bad Channel ID\n");			
			return false;
		}else {
						
			TCanvas* canv_single = new TCanvas("canv_single","Single Channel Spectrum",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
			canv_single->cd(1)->SetLogy(1);
			
			TH1F * hcharge = new TH1F("hcharge",Form("%s  Channel %d",fRunName,channel),4096,-0.5,4095.5);
			TH1F * hdiscri = new TH1F("hdiscri","digital output",4096,-0.5,4095.5);	
			
			/***********/
			/* Analog  */
			/***********/			
			ftree->Draw(Form("ADC[%d]>>hcharge",channel)); 
			
			hcharge->SetFillColor(kYellow-10);
			hcharge->GetXaxis()->SetTitle("Charge [ADC units]");
			hcharge->GetYaxis()->SetTitle("Occurrency [#]");
			
			hcharge->Fit(fgau3,"M");
			
			
			fparam_adc[idx].entries	= hcharge->GetEntries();
			fparam_adc[idx].mean	= hcharge->GetMean();
			fparam_adc[idx].rms		= hcharge->GetRMS();
			
			Int_t lbminADC = 0;
			Int_t lbmaxADC = 0;
			Int_t k = 0;
			for(int j=0; j<hcharge->GetNbinsX()+1; j++){ // ricerca del bin max e del bin minimo che siano !=0
				if(hcharge->GetBinContent(j)!=0){
					if(k==0){
						lbminADC=j;
					}
					k++;
					lbmaxADC = j;
				}
			}
			
			fparam_adc[idx].max = hcharge->GetBinCenter(lbmaxADC);
			fparam_adc[idx].min = hcharge->GetBinCenter(lbminADC);
			
			fparam_adc[idx].isEmpty = false;
	
			if (xmax==0) {
				hcharge->GetXaxis()->SetRange(ceil(lbminADC*.98),ceil(lbmaxADC*1.02));
			}else{
				hcharge->GetXaxis()->SetRange(xmin,xmax);
				hcharge->SetMaximum(ymax);
			
			}

			
			/***********/
			/* Digital */
			/***********/	
			gPad->Update();
			
			if (multiplicity==-1) {
				ftree->Draw(Form("ADC[%d]>>hdiscri",channel),Form("hit[%d]==1",channel),"same");
			}else{
				ftree->Draw(Form("ADC[%d]>>hdiscri",channel),Form("hit[%d]==1&&EvtMultiplicity==%d",channel,multiplicity),"same");
				//ftree->Draw("EvtMultiplicity",Form("hit[%d]==1",channel),"same");

			}

			hdiscri->SetFillColor(kRed);

			
			hdiscri->Fit(fgau3,"M,0");
			

		
			fgau1a->SetParameter(0,fgau3->GetParameter(0));
			fgau1a->SetParameter(1,fgau3->GetParameter(1));
			fgau1a->SetParameter(2,fgau3->GetParameter(2));	
			
			fgau1b->SetParameter(0,fgau3->GetParameter(3));
			fgau1b->SetParameter(1,fgau3->GetParameter(4));
			fgau1b->SetParameter(2,fgau3->GetParameter(5));	
			
			fgau1a->SetLineColor(kBlue);
			fgau1b->SetLineColor(kBlue);
			
			
			fgau1a->Draw("SAME");
			fgau1b->Draw("SAME");
			fgau3->Draw("same");
			
			fparam_hit[idx].entries	= hdiscri->GetEntries();
			fparam_hit[idx].mean		= hdiscri->GetMean();
			fparam_hit[idx].rms		= hdiscri->GetRMS();
			
			Int_t lbminADChit = 0;
			Int_t lbmaxADChit = 0;
			Int_t khit = 0;
			for(int j=0; j<hdiscri->GetNbinsX()+1; j++){
				if(hdiscri->GetBinContent(j)!=0){
					if(khit==0){
						lbminADChit=j;
					}
					khit++;
					lbmaxADChit = j;
				}
			}
			fparam_hit[idx].max = hdiscri->GetBinCenter(lbmaxADChit);
			fparam_hit[idx].min = hdiscri->GetBinCenter(lbminADChit);
			
			fparam_hit[idx].isEmpty = false;
			
			
			/*if (fCR!=0) {
				int id = fCR->GetIDXfrom(abs2geo(channel));
				unsigned char gain = fCR->GetGain(abs2geo(channel),channel%64);
				unsigned char hold = fCR->GetHoldDelay(id);
				unsigned char hitd = fCR->GetHitDelay(id);
				unsigned int dac = fCR->GetDAC0(id);				
				
			 TPaveText * pt = new TPaveText(0.13,0.80,0.45,0.90, "NDC"); // NDC sets coords (xlow,ylow,xup,yup)
				pt->AddText(Form("Gain %d\n,Hold Delay %d,DAC0 %d, Hit Delay %d",gain,hold,dac,hitd));
				pt->Draw();     
			}*/
			
			
			TLegend * leg = new TLegend(.3,.6,.6,.8);
			
			leg->AddEntry(hcharge,"Analog spectrum","F");
			leg->AddEntry(hdiscri,"Binary response","F");
			leg->AddEntry(fgau3,"Double gaussian fit", "L");	
			leg->AddEntry(fgau1a,"Gaussian", "L");			
			
			leg->Draw();
			
			if (save) {
				char nomerun[MAX_NAME_LENGH];
				sprintf(nomerun,"%s%s_Ch%d.eps",PATH_GRAPH,fRunName,channel); 
				printf("Saving...\n");
				canv_single->Print(nomerun);
			}else {
				printf("Drawing...\n");
				canv_single->DrawClone();
			}

			canv_single->Close();				
			
		}
		this->CloseRootFile();
	}
	
	
	/********/
	/* User */
	/********/
	
	printf("CHANNEL %d\tADDRESS: mrc_channel %2d on geo %2d \n",channel,channel%64,abs2geo(channel));	
	printf("TRIG Events = %d on %d (Ratio = %.3lf)\n",fparam_hit[idx].entries,fparam_adc[idx].entries,(Float_t)fparam_hit[idx].entries/fparam_adc[idx].entries);
	
	printf("-------------------------------------------------SPECTRUM %d\n",channel);
	printf("\tMEAN\tRMS\tMIN\tMAX\n");
	printf("ADC \t%.1lf\t%.1lf\t%.1lf\t%.1lf\n",fparam_adc[idx].mean,fparam_adc[idx].rms,fparam_adc[idx].min,fparam_adc[idx].max);
	printf("HIT \t%.1lf\t%.1lf\t%.1lf\t%.1lf\n",fparam_hit[idx].mean,fparam_hit[idx].rms,fparam_hit[idx].min,fparam_hit[idx].max);	
	printf("-------------------------------------------------\n");
	

	fparam_hit[idx].isEmpty=false; // probably redundant
	fparam_adc[idx].isEmpty=false; // probably redundant	
	return true;
}

bool TAnalysis::ResetSpectrParam(){
	for (int i=0; i<4096; i++) {
		
	
		fparam_adc[i].entries =0;
		fparam_hit[i].entries =0;
	
		fparam_adc[i].mean	=0.0;
		fparam_hit[i].mean	=0.0;
	
		fparam_adc[i].rms =0.0;
		fparam_hit[i].rms =0.0;
	
		fparam_adc[i].max =0.0;
		fparam_hit[i].max =0.0;
	
		fparam_adc[i].min =0.0;
		fparam_hit[i].min =0.0;
	
		fparam_adc[i].isEmpty = true;	
		fparam_hit[i].isEmpty = true;
	
	}
	return true;
	
}

int	TAnalysis::GetHITEntries(int channel){
	int idx = channel-1;
	if(fparam_hit[idx].isEmpty==true)
	{	
		Extract();
		
	}	
	return fparam_hit[idx].entries;
}

int	TAnalysis::GetADCEntries(int channel){
	int idx = channel-1;
	if(fparam_adc[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_adc[idx].entries;
}

double	TAnalysis::GetADCMean(int channel){
	int idx = channel-1;	
	if(fparam_adc[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_adc[idx].mean;
}
double	TAnalysis::GetADCHigh(int channel){
	int idx = channel-1;	
	if(fparam_adc[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_adc[idx].max;
}
double	TAnalysis::GetADCLow(int channel){
	int idx = channel-1;	
	if(fparam_adc[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_adc[idx].min;
}
double	TAnalysis::GetADCRms(int channel){
	int idx = channel-1;	
	if(fparam_adc[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_adc[idx].rms;
}
double	TAnalysis::GetHITMean(int channel){
	int idx = channel-1;	
	if(fparam_hit[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_hit[idx].mean;
}
double	TAnalysis::GetHITHigh(int channel){
	int idx = channel-1;	
	if(fparam_hit[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_hit[idx].max;
}
double	TAnalysis::GetHITLow(int channel){
	int idx = channel-1;
	if(fparam_hit[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_hit[idx].min;
}
double	TAnalysis::GetHITRms(int channel){
	int idx = channel-1;	
	if(fparam_hit[idx].isEmpty==true)
	{	
		Extract();
	}	
	return fparam_hit[idx].rms;
}

bool	TAnalysis::Export(){
	//gSystem->mkdir("./outpippo/pappo",true); // create folder

	FILE * fout = fopen("prova.txt","w");

	bool lsuccess = OpenRootFile(fnomeroot);if (!lsuccess) {printf("Error in OpenRootfile occurres...Exiting"); return lsuccess;}
	lsuccess = GetTree(); if (!lsuccess) {printf("Error in GetTree occurres...Exiting"); return lsuccess;}
	TH2F * ht2adc = new TH2F("ht2adc","ADC histo",4096,-.5,4095.5,4096, .5, 4096.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
	ftree->Draw("ADC:Iteration$>>ht2adc","","goff");	
	//TCut c_iter = Form("(Iteration$>=%d)&&(Iteration$<%d)",0,4095);
	TCut c_hit = Form("(hit==1)");
	TH2F * ht2hit = new TH2F("ht2hit","HIT histo",4096,-.5,4095.5,4096, .5, 4096.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
	ftree->Draw("ADC:Iteration$>>ht2hit",c_hit,"goff");	
	TH1F *hADC[4096];
	TH1F *hHIT[4096];
	Int_t lbminADC = 0;
	Int_t lbmaxADC = 0;
	Int_t lbminHIT = 0;
	Int_t lbmaxHIT = 0;
	Int_t k = 0;
	Int_t h = 0;
	fprintf(fout,"Ch/D:TotADC/D:MeanADC/F:RmsADC/F:MinADC/D:MaxADC/D:TotHIT/D:MeanHIT/F:RmsHIT/F:MinHIT/D:MaxHIT/D:Eff/F\n");
	for (int i=0; i<4096; i++) { // loop on all channel 
		hADC[i] = (TH1F*) ht2adc->ProjectionY(Form("ht2adc_ch%d",i),i+1,i+1,"");	
		hHIT[i] = (TH1F*) ht2hit->ProjectionY(Form("ht2hit_ch%d",i),i+1,i+1,"");
		k = 0;
		h = 0;	
		// search for limits of ADC histogram, bin content  !=0
		for(int j=0; j<4096; j++){ // loop on bins on adc histogram, 
			if(hADC[i]->GetBinContent(j)!=0){
				if(k==0){
					lbminADC=j;
				}
				k++;
				lbmaxADC = j;
			}
		}
		// search for limits of HIT histogram bin, content  !=0
		for(int z=0; z<4096; z++){ // loop on bins on hit histogram, 
			if(hHIT[i]->GetBinContent(z)!=0){
				if(h==0){
					lbminHIT=z;
				}
				h++;
				lbmaxHIT = z;
			}
		}
		/* PRINT (ON FILE)*/
		if (hADC[i]->GetEntries()&&hHIT[i]->GetEntries()) {
			if (h==0) { // channel always under threshold on this run
				printf("*");
			}
			fprintf(fout,"%4d",i);
			fprintf(fout,"%6.0lf %5.1lf %5.1lf %4d %4d ",hADC[i]->GetEntries(),hADC[i]->GetMean(),hADC[i]->GetRMS(),lbminADC,lbmaxADC);
			fprintf(fout,"%6.0lf %5.1lf %5.1lf %4d %4d",hHIT[i]->GetEntries(),hHIT[i]->GetMean(),hHIT[i]->GetRMS(),lbminHIT,lbmaxHIT);	
			fprintf(fout," %.3lf",hHIT[i]->GetEntries()/hADC[i]->GetEntries());
			fprintf(fout,"\n");// adc units
		}
		lbminADC = 0;
		lbmaxADC = 0;
		lbminHIT = 0;
		lbmaxHIT = 0;
	}
	fclose(fout);
	delete ht2adc;
	delete ht2hit;
 	return true;
}

bool	TAnalysis::Extract(){
	bool lsuccess = OpenRootFile(fnomeroot);if (!lsuccess) {printf("Error in OpenRootfile occurres...Exiting"); return lsuccess;}
	lsuccess = GetTree(); if (!lsuccess) {printf("Error in GetTree occurres...Exiting"); return lsuccess;}

	TH2F * ht2adc = new TH2F("ht2adc","ADC histo",4096,-.5,4095.5,4096, .5, 4096.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
	ftree->Draw("ADC:Iteration$>>ht2adc","","goff");	

	//TCut c_iter = Form("(Iteration$>=%d)&&(Iteration$<%d)",0,4095);
	TCut c_hit = Form("(hit==1)");
	TH2F * ht2hit = new TH2F("ht2hit","HIT histo",4096,-.5,4095.5,4096, .5, 4096.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
	ftree->Draw("ADC:Iteration$>>ht2hit",c_hit,"goff");	
		
//	cout <<" ADC binx " <<ht2adc->GetNbinsX()<<" ; biny =  "<< ht2adc->GetNbinsY()<<endl;
//	cout <<" HIT binx " <<ht2hit->GetNbinsX()<<" ; biny =  "<< ht2hit->GetNbinsY()<<endl;
	
	TH1F *hADC[4096];
	TH1F *hHIT[4096];
	
	Int_t lbminADC = 0;
	Int_t lbmaxADC = 0;
	Int_t lbminHIT = 0;
	Int_t lbmaxHIT = 0;
	
	Int_t k = 0;
	Int_t h = 0;
	
	MRC_DBG("Loop on Channels\n");
	for (int i=0; i<4096; i++) { // loop on channel 
		hADC[i] = (TH1F*) ht2adc->ProjectionY(Form("ht2adc_ch%d",i),i+1,i+1,"");	
		hHIT[i] = (TH1F*) ht2hit->ProjectionY(Form("ht2hit_ch%d",i),i+1,i+1,"");
		
		k = 0;
		h = 0;	
		
		// search for limits of ADC histogram, bin content  !=0
		for(int j=0; j<4096; j++){ // loop on bins on adc histogram, 
			if(hADC[i]->GetBinContent(j)!=0){
				if(k==0){
					lbminADC=j;
				}
				k++;
				lbmaxADC = j;
			}
		}
		
		// search for limits of HIT histogram bin, content  !=0
		for(int z=0; z<4096; z++){ // loop on bins on hit histogram, 
			if(hHIT[i]->GetBinContent(z)!=0){
				if(h==0){
					lbminHIT=z;
				}
				h++;
				lbmaxHIT = z;
			}
		}
		
		fparam_adc[i].entries	= hADC[i]->GetEntries();
		fparam_adc[i].mean		= hADC[i]->GetMean();
		fparam_adc[i].rms		= hADC[i]->GetRMS();
		fparam_adc[i].max		= hADC[i]->GetBinCenter(lbmaxADC); // or lbmaxADC
		fparam_adc[i].min		= hADC[i]->GetBinCenter(lbminADC); // or lbminADC
		fparam_adc[i].isEmpty	= false;
		
	
		fparam_hit[i].entries	= hHIT[i]->GetEntries();
		fparam_hit[i].mean		= hHIT[i]->GetMean();
		fparam_hit[i].rms		= hHIT[i]->GetRMS();
		fparam_hit[i].max		= hHIT[i]->GetBinCenter(lbmaxHIT); 
		fparam_hit[i].min		= hHIT[i]->GetBinCenter(lbminHIT); 
		fparam_hit[i].isEmpty	= false;
		
	
		/* PRINT
		if (hADC[i]->GetEntries()&&hHIT[i]->GetEntries()) {
			
			
			if (h==0) { // channel always under threshold on this run
				printf("*");
			}
			printf("Pixel[%4d]",i);
			printf(":ADC (%6.0lf,%4.0lf,%3.0lf,%4d,%4d)",hADC[i]->GetEntries(),hADC[i]->GetMean(),hADC[i]->GetRMS(),lbminADC,lbmaxADC);
			printf(",HIT (%6.0lf,%4.0lf,%3.0lf,%4d,%4d)",hHIT[i]->GetEntries(),hHIT[i]->GetMean(),hHIT[i]->GetRMS(),lbminHIT,lbmaxHIT);	
			
			
			printf("=>Eff = %.3lf",hHIT[i]->GetEntries()/hADC[i]->GetEntries());			
			
			printf("\n");// adc units
			
			printf("Channel[%4d]",i);
			printf(":ADC (%6d,%4.0lf,%3.0lf,%4.0lf,%4.0lf)",fparam_adc[i].entries,fparam_adc[i].mean,fparam_adc[i].rms,fparam_adc[i].min,fparam_adc[i].max);
			printf(",HIT (%6d,%4.0lf,%3.0lf,%4.0lf,%4.0lf)",fparam_hit[i].entries,fparam_hit[i].mean,fparam_hit[i].rms,fparam_hit[i].min,fparam_hit[i].max);	
			
		
			printf("=>Eff = %.3lf",((double)fparam_hit[i].entries)/fparam_adc[i].entries);			
			
			printf("\n");// adc units
			 
		}
			*/ 
		lbminADC = 0;
		lbmaxADC = 0;
		lbminHIT = 0;
		lbmaxHIT = 0;

	}
	
	if(ht2adc!=NULL){delete ht2adc;}
	if(ht2hit!=NULL){delete ht2hit;}
	for (int ch=0; ch<4096; ch++) {
		if(hADC[ch]!=NULL){delete hADC[ch];}
		if(hHIT[ch]!=NULL){delete hHIT[ch];}
	}
	return true;
}

bool	TAnalysis::CardSpectrum(int geoaddr,int adcmin,int adcmax){	
	bool lsuccess = OpenRootFile(fnomeroot);if (!lsuccess) {printf("Error in OpenRootfile occurres...Exiting"); return lsuccess;}
	lsuccess = GetTree(); if (!lsuccess) {printf("Error in GetTree occurres...Exiting"); return lsuccess;}
	int lchsys = geoaddr*64; ; // first channel in Controller Board Reference[0..4095] 
	TCut c_iter = Form("(Iteration$>=%d)&&(Iteration$<%d)",lchsys,lchsys+64);
	TCut c_hit = Form("hit==1");
	TH2F * ht2 = new TH2F("ht2","ADC histo",64,lchsys-.5,lchsys+64-0.5,adcmax-adcmin, adcmin-0.5, adcmax-0.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
	ftree->Draw("ADC:Iteration$>>ht2",c_iter,"goff");
	ftree->Draw("ADC:Iteration$>>ht2digital",c_hit,"goff");
	TH1F *ht2p[64];
	TH1F *ht2digitalp[64];
	fconnectH8500 = new TConnectH8500();
	TCanvas* mycanv = new TCanvas("mycanv","Card Representation",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,1.5*SIZEOFCANV); 
	mycanv->Divide(8,8);
	mycanv->Update();
	for (int i=0; i<64; i++) { // loop on channel 
		//mycanv->cd(i+1)->SetLogy(1); // MAROC VIEW
		mycanv->cd(fconnectH8500->GetAnode(i))->SetLogy(1); // HAMAMATSU VIEW

		ht2p[i]	= (TH1F*)ht2->ProjectionY(Form("ht2_%dpy",i),i+1,i+1,"");
		ht2digitalp[i] = (TH1F*)ht2->ProjectionY("pippo",i+1,i+1);			
		
		printf("%02d Mean	%6.3lf; RMS %4.2lf\n",i,ht2p[i]->GetMean(),ht2p[i]->GetRMS());
		cout <<setw(2)<<i<< " : MeanError	 :" << ht2p[i]->GetMeanError()	<< endl;
		cout <<setw(2)<<i<< " : RMSError	 :" << ht2p[i]->GetRMSError()	<< endl;
		ht2p[i]->Rebin(frebin);
		ht2p[i]->Draw();
		ht2digitalp[i]->Rebin(frebin);
		ht2digitalp[i]->SetFillColor(kRed);
		ht2digitalp[i]->Draw("SAME");
		mycanv->Update();
	}
	//	mycanv->Print(Form("Geo%02dSpectrum%d_%s.eps",geoaddr,code,fRunName)); //or .gif
	//	lsuccess = this->CloseRootFile();
	return lsuccess;
};

//Long64_t tot_entries = ftree->GetSelectedRows();
//Long64_t hit_entries = ftree->GetSelectedRows();
//cout << "Entries = "<<tot_entries<<" hit true = "<< hit_entries<<"=> "<<(100*(hit_entries/tot_entries))<<"%"<<endl;

bool TAnalysis::PlotDataset(char code){   
	bool success; 
	
	success = this->LoadTree(fnomeroot);

	TCanvas* mycanv = new TCanvas("mycanv","All Data");
	mycanv->cd(1)->SetLogy(1);
	
	char nomerun[MAX_NAME_LENGH];

	
	if(code==0){ // all adc !=0 in data set
		ftree->Draw("ADC");
		sprintf(nomerun,"%sDataSet_%s.eps",PATH_GRAPH,fRunName);  // or .gif instead of .eps
	}
	if(code==1){ // all adc !=0 in data set
		ftree->Draw("ADC","ADC!=0");
		sprintf(nomerun,"%sNonZeroDataset_%s.eps",PATH_GRAPH,fRunName); 
	}
	if(code==2){ // all adc hit enabled
		ftree->SetFillColor(kRed);
		ftree->Draw("ADC","hit==1");
		sprintf(nomerun,"%sDatasetHIT_%s.eps",PATH_GRAPH,fRunName); 
	}
	if(code==3){ // adc SUM for each event
		ftree->Draw("Sum$(ADC)");
		sprintf(nomerun,"%sSum_%s.eps",PATH_GRAPH,fRunName); 
	}
		
	if(code==4){ // ADC Profile Whole System with non-zero adc (otherwise bin prolem). Must find a better cut!
		mycanv->cd(1)->SetLogy(0);
		ftree->Draw("ADC:Iteration$","ADC!=0","profs");	
		sprintf(nomerun,"%sADCprofile_%s.eps",PATH_GRAPH,fRunName); 
	}
	if(code==5){// Trigger Efficiency Whole System with non-zero adc (otherwise bin problem). Must find a better cut!
		TH1F * lhisto	= NULL;
		Int_t i			= 0;
		Float_t lcheff	= 0.0;
		mycanv->cd(1)->SetLogy(0);
		ftree->Draw("(((ADC&hit)/hit)*100):Iteration$","ADC!=0","prof");
		
		//doesn't work
		lhisto = (TH1F*)gPad->GetPrimitive("lhisto"); // 1D
		if(lhisto){
			for(i=0;i<lhisto->GetNbinsX();i++){
				lcheff = (lhisto->GetBinContent(i));
				if(lcheff!=0.0){
					printf("%d: %5.1f%%\n",i,lcheff);
				}
			}// for i
		} // if lhisto
		else {
			printf("Sorry doesn't work try with code 7 instead of 5\n");
		}

		
		sprintf(nomerun,"%sTriggerEfficiency_%s.eps",PATH_GRAPH,fRunName); 
		delete lhisto;
		lhisto = 0;
	}
	if(code==6){ // Single Channel Spectrum
		printf("Please use SingleChannel Methods");
	}
	if(code==7){ // Accessing the Histogram in Batch Mode
		// draw the histogram
		ftree->Draw("ADC");
		// get the histogram from the current pad
		TH1F *htemp = (TH1F*)gPad->GetPrimitive("htemp");
		// now we have full use of the histogram
		cout<<htemp->GetEntries()<<endl;
		cout<<"nbinx"<<htemp->GetNbinsX()<<endl;	
		cout<<"UNDERFLOW: "<<htemp->GetBinContent(0)<<endl;
		for (int i=0; i<htemp->GetNbinsX()+1; i++) {
			printf("bin[%d} = %5.0lf\n",i,htemp->GetBinContent(i));
		}
		cout<<"First bin"<<htemp->GetBinContent(1)<<endl;
		cout<<"Last bin"<<htemp->GetBinContent(htemp->GetNbinsX())<<endl;
		cout<<"OVERFLOW"<<htemp->GetBinContent(htemp->GetNbinsX()+1)<<endl;
		printf("filename not setted\n");
		
		
	}
	if (code>=8) {
		printf("Warning: parameter 2 is from 0 to 7\n");
		return false;
	}
	
	
	// Other possibilities
	//Double_t ADCmin = 1500.;
	//Double_t ADCmax = 1700.;
	//int mybin = (int) ADCmax-ADCmin;
	
	//TH1F * htemp = new TH1F("htemp","htemp title",mybin,ADCmin,ADCmax);
	///TH1 *h1 = new TH1("h1","h1",50,0.,150.); 
	// Redirection
	//T->Draw("fNtrack>> h1");
	
	
	// Other Cuts 
	// ADC[%d]!=0
	// Form("ADC[%d]>%d",channel,thr)  // over thr
	//  
	// "hit[%d]==0" // Hit ==0

	mycanv->Print(nomerun);
	mycanv->Close();
	success = this->CloseRootFile();
	return success;
};

bool TAnalysis::LoadTree(const char* pathfileroot){
	bool success; 
	success = OpenRootFile(pathfileroot);
	if (!success) {printf("Error in OpenRootfile occurres...Exiting"); return success;}
	success = GetTree();
	if (!success) {printf("Error in GetTree occurres...Exiting"); return success;}
	return success;
	
}

void TAnalysis::Print(){
	printf("Welcome to TAnalysis Class!\n");
	printf("Folder used by the applications are:\n");
	printf("%s\n",PATH_GRAPH);
	printf("%s\n",PATH_GEOM);
	printf("%s\n",PATH_PDSTAL);
	printf("%s\n",PATH_DIAGN);
	printf("%s\n",PATH_ROOTFILE);
	

	if(fcalibration!=0)	{printf("TCalib Object\n");}
	if(fevent!=0)		{printf("TEvent Object\n");}
	if(fparser!=0)		{printf("TFileRaw Object\n");}
	if(frootfile!=0)	{printf("TFile Object\n");}
	if(ftree!=0)		{printf("TTree Object\n");}
	if(fdbgBlock)		{printf("Debug of TBlock	\n");}
	if(fdbgParser)		{printf("Debug of TFileRaw with Level %d\n",fPLParser);}
	if(fdbgEvent)		{printf("Debug of TEvent with Level %3d\n",fPLEvent);}
	if(fdbgMaroc)		{printf("Debug of TMaroc	with Level %3d and Threshold %6d\n",fPLMaroc,fPTMaroc);}
	
	cout <<"run name = "<<fRunName<<endl;

}

void TAnalysis::PrintFinalDebug(TStopwatch ltimer){
	int lcardnum = 0; // contatore di numero di carte presenti
	cout <<"\n Read Blocks      = " << fparser->GetBlockIndex()
	<<"\n Elapsed time     = " <<(int) ltimer.RealTime() << "  seconds"
	<<"\n Processing rate  = " <<(int) fparser->GetBlockIndex()/ltimer.RealTime() <<  "  Blocks/second"
	<< endl;
	
	cout <<"\n FE List: \n "<<endl;
	
	
	int cardcount =0;
	long tot_read =0;
	
	for(int i=0; i<64; i++)// Loop on Geoaddress
	{
		if( fparser->IsPresent(i) )
		{
			cardcount++;
			cout<<"\tFE Address "<<i<<" (BP: "<< abs2bp(i*64)<<" SLOT: "<<fe2slot(i)<<") Read   "<<fparser->GetCount(i)<<"  times"<<endl;
			tot_read=tot_read+fparser->GetCount(i);
			lcardnum++;
		}
	}
	cout<<"  \t\t\t\t      --------\n \t\t\t\t       "<< tot_read<<"\n"<<endl;
	cout<<"\t Total number of HEADER   =    "<< fparser->GetHeadCount()  <<endl; 
	cout<<"\t Total number of Trailer  =    "<< fparser->GetTrailCount() <<endl;		
	cout <<"\n Errori nella sequenza delle Word:\n"<<endl;
	cout <<"\t"<< fparser->Get_T_Lacks()      << " \t Assenza Trailer carta precedente\n "<<endl; 
	cout <<"\t"<< fparser->Get_H_Lacks()      << " \t Assenza Header carta attuale \n "<<endl; 
	cout <<"\t"<< fparser->Get_HD_Lacks()     << " \t Mancano Header e Dati carta attuale (Doppio Trailer) \n "<<endl; 
	cout <<"\t"<< fparser->Get_DT_Lacks()     << " \t Assenza Dati e Trailer carta precedente (Doppio Header) \n "<<endl;
	cout <<"\t"<< fparser->Get_FirstH_Lacks() << " \t Subito un Dato ad inizio file \n "<<endl; 
	cout <<"\t"<< fparser->Get_FirstHD_Lacks()<< " \t Subito un Trailer ad inizio file \n "<<endl; 
	cout<<"/------------------------------------------------------------------------/"<<endl;
	cout <<"\n Warnings:\n"<<endl;
	cout <<"\t"<< fparser->Get_Empty_Card_Check()<< " \t Carte vuote (solo H e T)  \n "<<endl; 
	cout<<"/------------------------------------------------------------------------/"<<endl;
	cout <<"\n Errori di inconsistenza delle informazioni:\n"<<endl;
	cout <<"\t"<< fparser->Get_Record_Flimsiness()<< " \t Inconsistenze  tra Trailer record e DW# effettivamente lette \n "<<endl; 
	
	cout<<"/************************************************************************/"<<endl;
};

bool TAnalysis::Diagnostic(){
	ReadDGN();
	bool lsuccess; 
	lsuccess = OpenRootFile(fnomeroot);
	if (!lsuccess) {
		MRC_ERR("Error: something wrong with %s\n",fnomeroot);
		return lsuccess;
	}
	
	TH1I * hp; 
	TCanvas* cp1 = new TCanvas("cp1",fRunName,PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV+200);
	
	cp1->Divide(2,2);
	hp= (TH1I*) frootfile->Get("f_histo1");
	hp->SetTitle("BlockSize - DWord Distribution");
	cp1->Update();
	cp1->cd(1);//cp1->SetLogy(1);
	hp->Draw();
	cp1->Update();
	cp1->cd(2);
	hp= (TH1I*) frootfile->Get("f_histo3");
	hp->SetTitle("EventSize - DWord Distribution");
	hp->Draw();
	cp1->Update();
	cp1->cd(3);//->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_histo5");
	hp->SetTitle("DWord	/FrontEnd");	
	hp->Draw();	
	cp1->Update();
	cp1->cd(4);//->SetLogy(1);
	hp=(TH1I*) frootfile->Get("f_hp1");
	hp->SetTitle("Readings from Geaddress");
	hp->Draw();
	cp1->Update();
	
	
	
	/*
	cp1->Divide(2,4); // (col, raw)
	hp= (TH1I*) frootfile->Get("f_histo1");
	hp->SetTitle("BlockSize - DWord Distribution");
	cp1->Update();
	cp1->cd(1);//cp1->SetLogy(1);
	hp->Draw();
	cp1->Update();
	cp1->cd(2);//->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_histo2");	
	hp->SetTitle("BlockSize - DWord vs BlockID");
	hp->Draw();
	cp1->Update();
	cp1->cd(3);
	cp1->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_histo3");
	hp->SetTitle("EventSize - DWord Distribution");
	hp->Draw();
	cp1->Update();
	cp1->cd(4);//->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_histo4");
	hp->SetTitle("EventSize - DWord vs EventID");
	hp->Draw();	
	cp1->Update();
	cp1->cd(5);//->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_histo5");
	hp->SetTitle("DWord	/FrontEnd");	
	hp->Draw();	
	cp1->Update();
	cp1->cd(6);//->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_histo6");
	hp->SetTitle("DWord	/ Event");
	hp->Draw();
	cp1->Update();
	cp1->cd(7);//->SetLogy(1);
	hp=(TH1I*) frootfile->Get("f_hp1");
	hp->SetTitle("Readings from Geaddress");
	hp->Draw();
	cp1->Update();
	cp1->cd(8);//->SetLogy(1);
	hp= (TH1I*) frootfile->Get("f_hp2");
	hp->SetTitle("Empty Readings");
	hp->Draw();
	*/

	//	char nome[100];
	//	sprintf(nome,"%s%s_BlockSize.eps",PATH_DIAGN,fRunName); 
	//	cp1->Print(nome);
	// delete cp1;
	return lsuccess;
}

void TAnalysis::check_zip(char *fname_in, char *fname_out){
	char command[256];
	if( strstr(fname_in, ".gz") ){	// Filename contains ".gz"
		cout << "Unzipping " << fname_in << " ..." << endl;
		sprintf(command, "gzip -d -f -c %s > %s", fname_in, TEMP_FNAME);
		system(command);
		strcpy(fname_out, TEMP_FNAME);
	}
	else{
		strcpy(fname_out, fname_in);
	}
};

int  TAnalysis::fe2slot(int fe){
	if(fe<0||fe>63){
		MRC_ERR("Error in fe2slot argument, index out of bound [0..63], you have %d\n",fe);
		return -1;
	}
	return (fe%16);
}		

int TAnalysis::abs2bp(int channel){
	bool success = IsGoodSystemChannel(channel);
	if(!success){MRC_ERR("Error in abs2bp argument: invalid channel\n");}
	return (channel / 1024);
}

int TAnalysis::abs2geo(int channel){
	bool success = IsGoodSystemChannel(channel);
	if(!success){MRC_ERR("Error in abs2geo argument: invalid channel\n");}
	return (channel / 64);
}

int TAnalysis::abs2maroc(int channel){	
	bool success = IsGoodSystemChannel(channel);
	if(!success){MRC_ERR("Error in abs2maroc argument: invalid channel\n");}
	return (channel % 64);
}

int TAnalysis::loc2abs(int geo, int maroc){	
	int abschannel = geo*64+maroc;
	bool success = IsGoodSystemChannel(abschannel);
	if(success){
		return abschannel;
	}
	else{
		MRC_ERR("Error in loc2abs\n");
		return -1;
	}
}

bool TAnalysis::IsGoodSystemChannel(int n){
	if ((n<0)||(n>4095)){
		MRC_ERR("Bad channel number. Range is [0...4095], you have  %d\n",n);
		return false;
	}
	return true;
};

bool TAnalysis::IsGoodGeoAddress(int n){
	if ((n<0)||(n>63)){
		MRC_ERR("Bad channel number. Range is [0...63], you have  %d\n",n);
		return false;
	}
	return true;
}

bool TAnalysis::OpenRootFile(const char * filename){	
	bool lsuccess;
	frootfile = new TFile(filename); 
	lsuccess = frootfile->IsOpen();
	if (!lsuccess) {
		MRC_ERR("Warning: %s is not open\n",filename);
		return false;
	}
	return true;
};

bool TAnalysis::CloseRootFile(){
	if (frootfile!=NULL) {
		frootfile->Close();
		//printf("myrootfile closed\n");
		return true;
	}
	else {
		MRC_WRN("Warning myrootfile pointer is NULL\n");
		return false;
	}
}; 

bool TAnalysis::GetTree(){
	if (frootfile) {
		bool lsuccess;
		lsuccess = frootfile->IsOpen();
		if (!lsuccess) { 
			cout << "Sorry no file opened"<<endl;
			return false;
		}
		ftree = (TTree*) frootfile->Get("fTdata");
		return true;
	}
	else {
		printf("Warning: Open the rootfile before! \n");
		return false;
	}
};

void TAnalysis::SaveGainMap(char* filename){
	if(fdebugmode){printf("[TAnalysis::SaveGainMap] on file: %s\n",filename);}
	if (fcalibration!= NULL){
		fcalibration->Export(filename,1);
	}
	else{
		//	cout << "WARNING: no data avaible, read data first!" << endl; //obsolete
	}
};

void TAnalysis::LoadGainMap(char * filename){
	if(fdebugmode){printf("[TAnalysis::LoadGainMap] from file: %s\n",filename);}
	if (fcalibration!= NULL){
		fcalibration->Import(filename);
	}
	else{
		//	cout << "WARNING: object not allocated yet, read data first!" << endl; //obsolete
	}
	//	fcalibration->Plot(Form("From calibration file %s",filename));
	
}
//------------------------------------------------
// delete any previous object pointed by fevent and fevent =0, than 
// read all the branch of a specific entry from ftree); 
bool TAnalysis::GetEvent(int entry){
	if(entry<0){
		MRC_ERR("Error in argument passed to. Must be positive value!\n");
	return false;
	}
	else{
		if(ftree!=0){
			if(entry <= ftree->GetEntries()){ //Entry disponibile;
				ftree->GetEntry(entry);
				return true;
			}
			else{
				MRC_WRN("Warning: value passed is too big!\n");
				return false;
			}
		}
		else{
			MRC_ERR("Error while getting event: No tree avaible\n");
			return false;
		}
	}
};

void TAnalysis::SetDebugOptions(bool dbgBlock,bool dbgMaroc,unsigned int PLMaroc ,unsigned int PTMaroc, bool dbgEvent,unsigned int PLEvent,bool dbgParser,unsigned int PLParser)
{
	if(fdebugmode){printf("[TAnalysis::SetDebugOption(...)]\n");}
	
	fdbgEvent	= dbgEvent;		
	fdbgBlock	= dbgBlock;		
	fdbgParser	= dbgParser;		
	fdbgMaroc	= dbgMaroc;		
	fPLEvent	= PLEvent;
	fPLParser	= PLParser;	
	fPLMaroc	= PLMaroc;	
	fPTMaroc	= PTMaroc;
	
	this->Print();
};
void TAnalysis::DebugInstructions()
{
	printf("\nYou can choose different print options while reading/debugging RawFiles\n Set them with TAnalysis::SetDebugOptions() according to what follows:\n\n");
	
	printf("Flux & Operations\n");
	cout <<"(1)			TBlock Methods"<<endl;
	cout <<"(0,1)			TMaroc Methods" <<endl;
	cout <<"(0,0,0,0,1)		TEvent Methods "<<endl;
	cout <<"(0,0,0,0,0,0,1)		TFileRaw Methods"<<endl;
	
	printf("\nData at trailer \n");
	cout <<"(0,0,1)			HIT true data"<<endl;
	cout <<"(0,0,3)			ALL data	"<<endl;
	cout <<"(0,0,5)			Header and Trailer data"<<endl;
	cout <<"(0,0,2,1600)		under thr data	"<<endl;
	cout <<"(0,0,4,1600)		above thr data	"<<endl;
	
	printf("\nInformation at Event's closure\n");
	cout <<"(0,0,0,0,0,1)		ID and Status"<<endl;
	cout <<"(0,0,0,0,0,2)		Tag Map"<<endl;
	cout <<"(0,0,0,0,0,4)		Complete Hit Map"<<endl; 
	cout <<"(0,0,0,0,0,8)		Complete ADC Map "<<endl;
	
	printf("\nInformation at Block's closure\n");
	cout <<"(0,0,0,0,0,0,0,1)	Block ID, Header counter, Trailer Counter, Event Counter, Last Card Read"<<endl;
	cout <<"(0,0,0,0,0,0,0,2)	DWord counter, Number of Event"<<endl;
	cout <<"(0,0,0,0,0,0,0,4)	Block Size in terms of Channels acquired"<<endl;
	cout <<"(0,0,0,0,0,0,0,8)	(Think is Last-read-)Card Size in terms of Channels acquired"<<endl;
	cout <<"(0,0,0,0,0,0,0,16)	State of the parsing FSM"<<endl;
	cout <<"(0,0,0,0,0,0,0,32)	Empty card Counter and Flimsiness Counter"<<endl;
	cout <<"(0,0,0,0,0,0,0,64)	FSM error Register status"<<endl;
};


int TAnalysis::ParseFileName(const char * pathfilenamebin){
	string	mystring	= pathfilenamebin;
	int		lenght		= mystring.size();
	int		idx_end		=0;
	int		idx_start	=0;
	for (int i=0; i<lenght; i++) {
		MRC_DUM("%c  ",mystring[i]);
		if (mystring[i]==46) {
			idx_end = i;
			MRC_DUM("\n");
		}
	}
	MRC_DUM("INDIETRO\n");
	for (int i=lenght-1; i>0; i--) {
			MRC_DUM("%c  ",mystring[i]);
		if (mystring[i]==47) {
			idx_start = i+1	;
			i=0;
			MRC_DUM("*\n");
		}
		else { 
			idx_start=0;
			MRC_DUM("\n");
		}
	}
	int k=0;
	for (int i = idx_start;i<idx_end; i++) {
		fRunName[k] = mystring[i];
		k++;
	}
	fRunName[k]='\0';
	
	
	//printf(" %s ( %d char)\n",fRunName,k);	
	return k;
}

void TAnalysis::DisplayFE(const char * fileraw,char geoaddress,int nevent, double min,double max,double sleep){
	
	//------------------  Import Pixel Map  -----------
	Float_t x[MAX_PIXEL];
	Float_t y[MAX_PIXEL];
	int channel;
	for (channel=0; channel<MAX_PIXEL; channel++) {
		x[channel]=0.;
		y[channel]=0.;
	}
	char pathfile[200];
	sprintf(pathfile,"%sMarocViewGeo%02d.txt",PATH_MAP,geoaddress);
	printf("%s\n",pathfile);
	FILE * lin =fopen(pathfile,"r");
	if(lin!=NULL){
		//printf("File %s opened\n",lin);
		for (int i=0; i<MAX_PIXEL; i++) {
			fscanf(lin,"%d %f %f",&channel,&x[i],&y[i]);
			if (x[i]!=0) {
				//printf("%04d : %6.2f  %6.2f\n",channel,x[i],y[i]);
			}
		}
	}else {
		printf("Problem with file.\n");
	}
	fclose(lin);
	
	//------------------  BinDisplay  -----------
	
	Float_t A= 30.0; // Range in X e Y [mm]  
	
	TCanvas* dummy0 = (TCanvas*) gDirectory->FindObject("mycanv");
	if(dummy0) delete dummy0;
	TCanvas*  mycanv = new TCanvas("mycanv","DisplayMarocView",PIXX-SIZEOFCANV,0,SIZEOFCANV,SIZEOFCANV);
	mycanv->Range(-A,-A,A,A);
	
	
	
	TAnalysis * Analisi  = new TAnalysis(fileraw);
	Analisi->LoadTree(fnomeroot);	
	TEvent * evento = new TEvent();
	evento->ConfigureReading(Analisi->GetTreePointer());
	TH1F* hfe[64];
	
	const short ch0 = geoaddress*64; // first fe channel by the CB point of view 
	
	for (int i=0;i<64;i++ ) {
		hfe[i] = new TH1F(Form("hfe%d",ch0+i),"ADC vs event",nevent,0,nevent-1);
	}
	for (int ev=0; ev<nevent; ev++) {
		TH2F * dummy = (TH2F*) gDirectory->FindObject("h2");
		if(dummy) delete dummy;
		Int_t binning = 64;
		
		TH2F *h2 = new TH2F("h2",Form("Display Event %d of %d",ev,nevent-1),binning,0,64,10,0,10);
		evento->Clean();
		Analisi->GetEvent(ev); // load event e
		for (channel=0; channel<MAX_PIXEL; channel++)
		{
			if (x[channel]!=0) {
				h2->Fill(x[channel],y[channel],(evento->GetADC(channel)));	
				//printf("%4d ",evento->GetADC(channel));
				if ((channel-ch0)%8==7) {
					//	printf("\n");
				}
			}
		}
		for (int i=0;i<64;i++ ) {
			hfe[i]->Fill((double)ev,evento->GetADC(ch0+i));			
		}
		//gStyle->SetPalette(1);
		h2->Draw("LEGO2Z 0"); //h2->Draw("COLZ");
		h2->GetZaxis()->SetRangeUser(min,max);
		gPad->SetTheta(30); // default is 30
		gPad->SetPhi(20); // default is 30
		gPad->Update();
		mycanv->Update();
		gROOT->ProcessLine(Form(".! sleep %f",sleep));
	}
	mycanv->DrawClone();
	delete mycanv;
	
	//------------------  Drawing Summary  -----------
	TCanvas* allch  = new TCanvas("allch","ADC",PIXX-SIZEOFCANV,SIZEOFCANV+45,SIZEOFCANV,SIZEOFCANV);
	for (int i=0; i<64; i++) {
		if (i ==0) {
			hfe[i]->Draw();
			TAxis *xaxis = hfe[i]->GetXaxis();
			xaxis->SetTitle("event - #");
			TAxis *yaxis = hfe[i]->GetYaxis();
			yaxis->SetTitle("ADC - adc unit ");
		}
		else {
			hfe[i]->SetLineColor(i*2);
			hfe[i]->Draw("same");
		}
	}
	allch->DrawClone();
	delete allch;
	Analisi->CloseRootFile();
};	

void TAnalysis::DisplayBP(const char * fileraw,char bp_code,int max_event, double min,double max,double sleep){
	//------------------  Import Pixel Map  -----------
	Float_t x[MAX_PIXEL];
	Float_t y[MAX_PIXEL];
	int channel;

	for (channel=0; channel<MAX_PIXEL; channel++) {
		x[channel]=0.;
		y[channel]=0.;
	}
	char pathfile[200];
	sprintf(pathfile,"%sMarocViewBP%d.txt",PATH_MAP,bp_code);
	//printf("%s\n",pathfile);
	FILE * lin =fopen(pathfile,"r");
	if(lin!=NULL){
		//printf("File %s opened\n",lin);
		for (int i=0; i<MAX_PIXEL; i++) {
			fscanf(lin,"%d %f %f",&channel,&x[i],&y[i]);
			if (x[i]!=0) {
				//			printf("%04d : %6.2f  %6.2f\n",channel,x[i],y[i]);
			}
		}
	}else {
		printf("Problem with file\n");
	}
	fclose(lin);
	
	//------------------  BinDisplay  -----------
	
	Float_t A= 30.0; // Range in X e Y [mm]  
	
	TCanvas* dummy0 = (TCanvas*) gDirectory->FindObject("mycanv3");
	if(dummy0) delete dummy0;
	TCanvas*  mycanv3 = new TCanvas("mycanv3","Geometry2",PIXX-SIZEOFCANVX,0,SIZEOFCANVX,SIZEOFCANVY);
	mycanv3->Range(-A,-A,A,A);
	
	TAnalysis * Analisi  = new TAnalysis(fileraw);
	Analisi->LoadTree(fnomeroot);	
	TEvent * evento = new TEvent();
	evento->ConfigureReading(Analisi->GetTreePointer());
	TH1F* hfe[512];
	
	const short ch0 = bp_code*512; // first fe channel by the CB point of view 
	
	for (int i=0;i<512;i++ ) {
		hfe[i] = new TH1F(Form("hfe%d",ch0+i),"ADC vs event",max_event,0,max_event-1);
	}
	for (int ev=0; ev<max_event; ev++) {
		TH2F * dummy = (TH2F*) gDirectory->FindObject("h2");
		if(dummy) delete dummy;
		Int_t binning = 512;
		TH2F *h2 = new TH2F("h2",Form("Display Event %d of %d",ev,max_event-1),binning,0,512,10,0,10);
		evento->Clean();
		Analisi->GetEvent(ev); // load event e
		
		// VETTORE
		int cn[MAX_PIXEL];
		
		int media=0;
		
		int k,j;
		for (j=0; j<MAX_PIXEL; j++) {
			cn[j]=0;
		}
		j=0;
		
		for (int i=0; i<MAX_PIXEL; i++) {
			
			//printf("channel[%d](%d) = %4d\n",i,i%32,evento->GetADC(i));
			media = media+evento->GetADC(i);
			//	printf("cn[%d][%d][%d] = %d\n",i,j,k,cn[i]);
			if (i%32==31) {
				//		printf("media calcolata = %d , %d\n",media/32,i);
				//		printf("scrivo il vettore CN:\n");
				for (j=i; k<32; j--) {
					cn[j]=media/32;
					//	printf("CN[%d](%d) = %d\n",j,k,media/32);
					k++;
				}
				k=0;
				j=0;
				media =0;
			}
			
		}
		
		for (int i=0; i<MAX_PIXEL; i++) {
			//	printf("common noise[%d] = %d\n",i,cn[i]);
		}
		
		
		
		for (channel=0; channel<MAX_PIXEL; channel++)
		{
			if (x[channel]!=0) {
				h2->Fill(x[channel],y[channel],(evento->GetADC(channel))-cn[channel]);	
				//h2->Fill(x[channel],y[channel],(evento->GetADC(channel)));	
				//printf("%4d ",evento->GetADC(channel)-cn[channel]);
				if ((channel-ch0)%8==7) {
					//	printf("\n");
				}
			}
		}
		for (int i=0;i<512;i++ ) {
			hfe[i]->Fill((double)ev,evento->GetADC(ch0+i));			
		}
		//gStyle->SetPalette(1);
		//h2->Draw("COLZ");
		
		h2->Draw("LEGO2Z 0"); //
		h2->GetZaxis()->SetRangeUser(min,max);
		gPad->SetTheta(0); // default is 30
		gPad->SetPhi(0); // default is 30
		gPad->Update();
		mycanv3->Update();
		gROOT->ProcessLine(Form(".! sleep %f",sleep));
	}
	//mycanv3->DrawClone();
	delete mycanv3;
	
	//------------------  Drawing Summary  -----------
	TCanvas* allch  = new TCanvas("allch","ADC",PIXX-SIZEOFCANVY,SIZEOFCANVY+45,SIZEOFCANVY,SIZEOFCANVY);
	for (int i=0; i<512; i++) {
		if (i ==0) {
			hfe[i]->Draw();
			TAxis *xaxis = hfe[i]->GetXaxis();
			xaxis->SetTitle("event - #");
			
			TAxis *yaxis = hfe[i]->GetYaxis();
			yaxis->SetTitle("ADC - adc unit ");
			
			
		}
		else {
			hfe[i]->SetLineColor((i%64)*2);
			hfe[i]->Draw("same");
		}
	}
	
	//allch->DrawClone();
	delete allch;
	Analisi->CloseRootFile();

};

// Code for Text BOX on a Canvas

// 
//TPaveText * pt = new TPaveText(0.40,0.75,0.7,0.85, "NDC"); // NDC sets coords (xlow,ylow,xup,yup)
//	pt->SetFillColor(0); // text is black on white
//	pt->SetTextSize(0.05); 
//	pt->SetTextAlign(5);
//pt->AddText(Form("Whatever you want  %d ",23));
//pt->Draw();   

bool TAnalysis::ReadConfiguration(){
	bool success = false;
	fCR = new MRCConfigurationManager(fnomeconfig,true); 
	//fCR->Recap();
	return success;
}

void TAnalysis::Reset(){
	
	for (int i=0; i<MAX_NAME_LENGH; i++) {
		fRunName[i]=0;
		fnomeroot[i]=0;
	}
	
	fdebugmode = false;
	
	fcalibration= NULL;	
	fparser		= NULL;	
	fevent		= NULL;	
	frootfile	= NULL; 
	ftree		= NULL;
	fconnectH8500= NULL;
	fCR			= NULL;
	fCNSAnalizer=NULL;
	
	fdbgEvent	= false;		
	fdbgBlock	= false;		
	fdbgParser	= false;		
	fdbgMaroc	= false;		
	fPLEvent	= 0;
	fPLParser	= 0;	
	fPLMaroc	= 0;	
	fPTMaroc	= 1235;
	fchfirst	= 0;
	fchlast		= 4096;

	// Configure Behaviour
	SetFitON();
	SetGraphicsON();	

	// Configure Layout ADC Spectra
	SetYMax(); 
	SetRebin();
	
	// Default channel  
	SetCh1(); 
	SetCh1ADCmin();	
	SetCh1ADCmax();
	SetCh1HIT();
	
	// Default channel 2 (for Xtalk)
	SetCh2();
	SetCh2ADCmin();	
	SetCh2ADCmax();
	SetCh2HIT();	
}

void TAnalysis::DestroyCNSAnalizer(){
	if (fCNSAnalizer!=0) {
		delete fCNSAnalizer;
		fCNSAnalizer=0;
	}

}

void TAnalysis::DestroyConnH8500(){
	if (fconnectH8500!=NULL) {
		delete fconnectH8500;
		fconnectH8500 = 0;
	}
};

void TAnalysis::DestroyEvent(){
	if (fevent!=NULL) {
		delete fevent;
		fevent = 0;
	}
};

void TAnalysis::DestroyParser(){
	if (fparser!=NULL) {
		delete fparser;
		fparser = 0;
	}
};

void TAnalysis::DestroyCalibration(){
	if (fcalibration!=NULL) {
		delete fcalibration;
		fcalibration = 0;
	}
};

bool TAnalysis::SystemSpectrum(const char *pedestalfile){
	if (!this->LoadTree(fnomeroot)) {
		return false;
	}else{ 
		Int_t dataArraySize = 4096;
		Int_t numEntries = ftree->GetEntries();
		// cout << "Number of entries in the Tree: " << numEntries << endl; //DEBUG	
		TH1F * Spectrum = new TH1F("Spectrum","",160,40000.,50000.);	
		FILE * pedfile;
		pedfile = fopen(pedestalfile,"rt");
		char header1[10], header2[10], header3[10];
		Float_t pedest, sigma, pedestal[dataArraySize];
		//Float_t noise[dataArraySize];
		Int_t channel, activeChannels=0;
		
		for(Int_t ii=0;ii<dataArraySize;ii++){ 
			pedestal[ii] = 0.;
		//	noise[ii] = 0.;
		}
	    
		//
		if(pedfile!=NULL){
			fscanf(pedfile,"%s %s %s \n", header1, header2,header3);
			//printf("%s, %s, %s \n",header1, header2, header3); //DEBUG
			while(fscanf(pedfile,"%d %f %f",&channel,&pedest,&sigma)!=EOF){
				//
				//printf("channel: %d, pedestal: %f \n",channel,pedestal); // DEBUG
				pedestal[channel] = pedest;
		//		noise[channel] = sigma;
				activeChannels++;
			}
			//
			fclose(pedfile);
			printf("File %s succesfully imported\n",pedestalfile);
			printf("Number of imported channels: %d\n",activeChannels);
			//
			float_t adc[dataArraySize];
			
			ftree->SetBranchAddress("ADC",&adc);
			//
			for(Int_t j=0;j<numEntries;j++){
				float_t adcSum = 0.0;
				ftree->GetEntry(j);
				//Int_t evtNb = ftree->GetEvent(j);
				for(Int_t jj=0;jj<dataArraySize;jj++){
					//if(jj<1056||jj>1087){ //DEBUG
					adcSum = adcSum + adc[jj] - pedestal[jj];
				} //
				Spectrum->Fill(adcSum);
			}
		}else{
			printf("Warning: file %s does not exist! \n",pedestalfile);
	    	}
		//
		TCanvas *mc1 = new TCanvas("mc1","Spectrum",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,2*SIZEOFCANV);
		mc1->cd();
		Spectrum->Draw("");
		TAxis *xax1 = Spectrum->GetXaxis();
		xax1->SetTitle("ADC value");
		TAxis *yax1 = Spectrum->GetYaxis();
		yax1->SetTitle("Counts");
		
		return true; 
	}
	return false;
}

bool TAnalysis::MakePedestal(const char * pedestalfile){
	if (!this->LoadTree(fnomeroot)){
		return false;
	}else{
		//in ptot  x= channel; y = ADC 
		TProfile * ptot = new TProfile("ptot","Profile",fchlast - fchfirst,fchfirst-.5,fchlast-0.5); //name,title,nbinx,xlow,xup,nbiny,ylow,yup
		ptot->SetErrorOption("s");
		ftree->Draw("ADC:Iteration$>>ptot","","goff,profs");
		// CURRENTLY WE DO NOT WANT PLOTS
		//TGraph *ped = new TGraph(ptot);  // x = channel ID; y= MEAN adc 
		TGraph *rped = new TGraph(ptot); // x = channel ID; y= RMS 
		// CURRENTLY WE DO NOT WANT PLOTS
		//
		//ped->SetTitle("Mean Value");
		//rped->SetTitle("Fluctuation");	       	
		//
		ofstream pedfile;
		
		pedfile.open(pedestalfile);
		
		pedfile << "Channel Pedestal Noise Gain\n" ;
		
		for (int i=0; i<(fchlast - fchfirst); i++) {  
			// replace trick
			Double_t xa,ya,za,ga;
			rped->GetPoint(i,xa,ya); // xa = channel ID; ya = mean ADC 
			za = ptot->GetBinError(i+1);
			ga = this->Get_Gain(i);
			if (za!=0.0) {
				pedfile << 
				setiosflags(ios::fixed)<< 
				setprecision(0) << xa << " "<< 
				dec << 
				setw(5)<< 
				setiosflags(ios::fixed) <<
				setprecision(2) << 
				ya << " " << 
				setw(3) << 
				setprecision(2) << 
				za <<  " "<< 
				setiosflags(ios::fixed)<<
				setprecision(0) << ga << "\n";
			}	
		}
		pedfile.close();
		cout << "pedestal file " << pedestalfile << " written" << endl;
		return true; 
	}
}


// Qui servono andamento e distribuzione di:
// 1) Molteplicita' Evento
// 2) Attivita' di canale 


//fTdata->Draw("hit[2048]:Evt") // attivita' singolo canale (poco significativo)
//fTdata->Draw("hit[2048]") // spettro binario di canale (poco significativo)
//fTdata->Draw("Sum$(hit):Evt")// molteplicita' di evento
//fTdata->Draw("Sum$(hit)")// spettro della molteplicita'

Float_t * TAnalysis::GetMultiplicity(){

	LoadTree(fnomeroot);
	TH1I * hm = new TH1I("hm","Multiplicity Distribution",MAX_PIXEL,-0.5,MAX_PIXEL-0.5); 
	ftree->Draw("Sum$(hit)>>hm","","GOFF");
	
	static Float_t m[4];
	
	m[0]=hm->GetMean();
	m[1]=hm->GetMeanError();
	m[2]=hm->GetRMS();
	m[3]=hm->GetRMSError();
	
	return m;
}

int TAnalysis::PrintStat(TH1I * h){
	if (h==NULL) {
		MRC_WRN("TH1I == NULL,nothing to print\n");
		return -1;
	}
	printf("\tMean = %.3f ",h->GetMean());
	printf("\tError %.3f\n",h->GetMeanError());
	printf("\tRMS  = %.3f ",h->GetRMS());
	printf("\tError %.3f\n",h->GetRMSError());
	//for (int i=0; i<h->GetSize()+1; i++) { 
	//	printf("multeplicity[%3.0f] = %5.0f\n",hm->GetBinCenter(i),hm->GetBinContent(i));
	//}
	int overflow =  h->GetBinContent(h->GetSize()+1);
	if (overflow!=0) {printf("Warning: %d events in overflow \n",overflow);}
	int underflow = h->GetBinContent(0);
	if (underflow!=0) {printf("Warning: %d events in underflow \n",overflow);}	
	return overflow+underflow;
}


/* Calcolo della occupazione di canale oppure attivita' del canale vs Event (scatterplot)
 * nel primo caso ritorna la channel occupancy, nel secondo il numero totale di eventi. 
 */
Double_t TAnalysis::PlotHit(int channel,char option){
	Double_t ret=-1.;
	if (LoadTree(fnomeroot)!=0) {
		if (this->IsGoodSystemChannel(channel)){
			TCanvas * old		= (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
			TCanvas * mycanv	= new TCanvas("mycanv","Single Channel HIT Spectrum",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
			TH1F * hHIT = NULL;
			Int_t n;
			//Double_t p1;
			Double_t p2;
			switch (option) {
				case 0:
					hHIT	= new TH1F("hHIT","Binary Histogram",2,-0.5,1.5);
					n		= ftree->Draw(Form("hit[%d]>>hHIT",channel),"","goff");
				//	p1		= hHIT->GetBinContent(1); // hit ==0
					p2		= hHIT->GetBinContent(2); // hit ==1
					ret		= p2/n;
					mycanv->cd(1)->SetLogy(1);
					hHIT->DrawCopy();
					break;
				case 1:
					n		= ftree->Draw(Form("hit[%d]:Evt",channel),"","L");
					ret = n; 
					break;
				default:
					MRC_WRN("unknown option. Use 0 for distribution, 1 for scatter plot (vs event)\n");
					break;
			}
		}else {
			MRC_ERR("Bad channel: %d is not in the range 0..4095\n",channel);
		}		
	}else {
		MRC_ERR("File %s not found\n",fnomeroot);
	}
	return ret;
}


Double_t TAnalysis::HitMultiplicity(char option,int mmax){
	Double_t ret=-1.;
	if (LoadTree(fnomeroot)!=0) {
		TCanvas * old		= (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
		TCanvas * mycanv	= new TCanvas("mycanv","Multiplicity Spectrum",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,SIZEOFCANV);
		TH1I * hm  = NULL;
		Int_t n;
		Double_t p1,p2;
		TPaveText * pt;
		switch (option) {
			case 0:
				hm		= new TH1I("hm",Form("Event Multiplicity %s Threshold = %d",fRunName,Get_DAC0()),mmax,-0.5,mmax-0.5); 
				n		= ftree->Draw("EvtMultiplicity>>hm","","GOFF");	
				p1		= hm->GetBinContent(2); 
				p2		= hm->GetBinContent(3); 						
				ret		= p1/n;
				
				mycanv->cd(1)->SetLogy(1);
				mycanv->SetGrid(1,1);
				
				hm->SetMaximum(5*n);
				MakeUpMultiplicity(hm);
				
				pt = new TPaveText(0.70,0.50,0.85,0.70, "NDC"); // NDC sets coords (xlow,ylow,xup,yup)
				pt->AddText(Form("Single %4.1f %%",100*(p1/n)));
				pt->AddText(Form("Double %4.1f %%",100*(p2/n)));
				pt->AddText(Form("Other  %4.1f %%",100*(n-p1-p2)/n));
				
				hm->DrawCopy("BAR TEXT30"); // TEXT
				pt->Draw("SAME");
				break;
			case 1:
				n = ftree->Draw("EvtMultiplicity:Evt","","L");
				ret = n; 
				break;
			default:
				MRC_WRN("unknown option. Use 0 for distribution, 1 for scatter plot (vs event)\n");
				break;
		}
		if (true) {
			char nomerun[MAX_NAME_LENGH];
			sprintf(nomerun,"%sMultiplicity%s.eps",PATH_GRAPH,fRunName); 
			mycanv->Print(nomerun);
		}
		this->CloseRootFile();
	}else {
		MRC_ERR("File %s not found\n",fnomeroot);
	}
	
	return ret;
}



void TAnalysis::ResetCoordinates(){
	for (int channel=0; channel<MAX_PIXEL; channel++) {
		fx[channel]=0.;
		fy[channel]=0.;
	}
}

bool TAnalysis::SetCoordinates(const char * mapfile){
	bool success; 
	FILE * lin =fopen(mapfile,"r");
	if(lin!=NULL){
		MRC_DBG("File %s opened...setting coordinates \n",mapfile);
		for(int channel = 0; channel<MAX_PIXEL; channel++){
			fscanf(lin,"%d %f %f",&channel,&fx[channel],&fy[channel]);
			if (fx[channel]!=0) {
				MRC_DUM("%04d : %6.2f  %6.2f\n",channel,fx[channel],fy[channel]);
			}
		}
		success = true;
	}else {
		MRC_ERR("Problem with file\n");		
		success = false;
	}
	fclose(lin);
	return success;
}

int TAnalysis::GetNEvent(){
	
	if (!LoadTree(fnomeroot)) {
		printf("problem loading parsed data");
	}
	return ftree->GetEntries();
}


Int_t TAnalysis::HitDisplay(int m,int opt, int first,int last,float sleep_val,const char * mapfile){
	
  gStyle->SetOptStat(0);
	ResetCoordinates();
	if (!SetCoordinates(mapfile)) {printf("problem with coordinates file %s\n",mapfile);return -1;}
		
	Float_t A1 = 120; // mm 

	TCanvas*  mycanv = new TCanvas("mycanv","HitDisplayALL",PIXX-SIZEOFCANV,0,SIZEOFCANV,SIZEOFCANV);
	mycanv->Range(-A1,-A1,A1,A1);
	
	int NEvtsMAX = GetNEvent();
	if (opt!=0) {
		if (last<first) return -1;
		if (last<=NEvtsMAX) NEvtsMAX = last;
		printf("from event %d to event %d %s\n",first,NEvtsMAX,fnomeroot);
	}
	TEvent * evento = new TEvent();
	evento->ConfigureReading(GetTreePointer());
	
	Float_t semisize = 50;
	Int_t binx = 16;
	Int_t biny = 16;
	Double_t xlo = -semisize;
	Double_t ylo = -semisize;
	Double_t xhi = +semisize;
	Double_t yhi = +semisize;
	
	TH2F *hsum = new TH2F("hsum",Form("%s %d events",fnomeroot,NEvtsMAX),binx,xlo,xhi,biny,ylo,yhi);	
	
	int counts = 0;
	for (int ev=first; ev<NEvtsMAX; ev++) {	//Loop on Events
		evento->Clean();
		GetEvent(ev);
		int lm = evento->GetEvtMultiplicity();
		if (m==-1) {
			if (opt==0) {
				for (int ch=0; ch<MAX_PIXEL; ch++){
					if (fx[ch]!=0.) { // Exclude the origin where are mapped the not-connected channels
						if(evento->GetHit(ch)){
							hsum->Fill(fx[ch],fy[ch],1);			
						}
					}
				}
			}else {
				TH2F *h2 = new TH2F("h2",Form("Event Display %d of %d %s",ev,NEvtsMAX,fnomeroot),binx,xlo,xhi,biny,ylo,yhi);
				for (int ch=0; ch<MAX_PIXEL; ch++){
					if (fx[ch]!=0.) { // Exclude the origin where are mapped the not-connected channels
						if(evento->GetHit(ch)){
							hsum->Fill(fx[ch],fy[ch],1);	
							h2->Fill(fx[ch],fy[ch],1);							
						}
					}
				}
				h2->Draw("COLZ");
				mycanv->Update();
				usleep(sleep_val*1000000);		
				delete h2;
			}
		}else {
			if (m==lm) {
				counts++;
				if (opt==0) {
					for (int ch=0; ch<MAX_PIXEL; ch++){
						if (fx[ch]!=0.) { // Exclude the origin where are mapped the not-connected channels
							if(evento->GetHit(ch)){
								hsum->Fill(fx[ch],fy[ch],1);			
							}
						}
					}
				}else {
					TH2F *h2 = new TH2F("h2",Form("Event Display %d of %d %s",ev,NEvtsMAX,fnomeroot),binx,xlo,xhi,biny,ylo,yhi);
					for (int ch=0; ch<MAX_PIXEL; ch++){
						if (fx[ch]!=0.) { // Exclude the origin where are mapped the not-connected channels
							if(evento->GetHit(ch)){
								hsum->Fill(fx[ch],fy[ch],1);	
								h2->Fill(fx[ch],fy[ch],1);							
							}
						}
					}
					h2->Draw("COLZ");
					mycanv->Update();
					usleep(sleep_val*1000000);		
					delete h2;
				}
			}
		}
	}
	hsum->Draw("TEXT,COLZ");
	// Save or Delete? ans: to fix 
	char nome[200];
	sprintf(nome,"./eps/%s_COLZ.eps",fRunName);
	mycanv->DrawClone();
	mycanv->Print(nome);
	delete mycanv;
	CloseRootFile();
	return counts;
}


TAnalysis::TAnalysis(const char * fileraw,bool havecfg){
	
	Reset();
	ResetSpectrParam();
	
	sprintf(fnomebin,"%s",fileraw); 
	//	printf("Input File Name: %s\n",fileraw);
	
	int kk = ParseFileName(fileraw); // set fRunName
	
	if(kk>0){ 
		
		sprintf(fnomeroot,"%s%s.root",PATH_ROOTFILE,fRunName); 
		
		MRC_DBG("Current RootFile : %s\n",fnomeroot);
		
		if (havecfg) {
			
			sprintf(fnomeconfig,"%s%s.txt",CFGPATH,fRunName);
			
			MRC_DBG("CFG : %s\n",fnomeconfig);
			
			ReadConfiguration();
			
			MRC_DBG("DATA RATE = %3.1lf [evts/sec]\n",fCR->GetRate());
			
		}else{
			printf("Configuration file missing\n");
		}	
	}else {
		printf("Sorry, filename missing...");
	}
	
}

TAnalysis::~TAnalysis(){
	DestroyEvent();
	DestroyParser();
	DestroyCalibration();
	DestroyConnH8500();
	
	if(frootfile!=NULL){
		delete frootfile;
		frootfile = NULL;
	}
	if (fCR!=NULL) {
		delete fCR;
		fCR=NULL;
	}	
};	

unsigned char TAnalysis::Get_Gain(int channel){return fCR->GetGain(abs2geo(channel),abs2maroc(channel));}
unsigned char TAnalysis::GetGeo(int idx){return fCR->GetGeo(idx);}
unsigned char TAnalysis::GetFeNum()	{return fCR->GetFeNum();}
float TAnalysis::Get_ACQRate() {return fCR->GetRate();}
float TAnalysis::GetDuration(){return fCR->GetTimeReal();}
float TAnalysis::GetPollingEfficiency(){return fCR->GetPollingEfficiency();}
int	TAnalysis::Get_DAC0(){return fCR->GetDAC0();}
int	TAnalysis::Get_HOLD1_Delay(){return (int)fCR->GetHoldDelay();}
int	TAnalysis::GetNevents(){return fCR->GetEventNumber();}


void TAnalysis::SetCh1(Int_t echannel)		{fch1		=echannel;} 
void TAnalysis::SetCh1ADCmin(Int_t ADCmin)	{fADCmin1	=ADCmin;}	
void TAnalysis::SetCh1ADCmax(Int_t ADCmax)	{fADCmax1	=ADCmax;}	
void TAnalysis::SetCh1HIT(Int_t hit)		{fhit1		=hit;}	

void TAnalysis::SetCh2(Int_t echannel)		{fch2		=echannel;}
void TAnalysis::SetCh2ADCmin(Int_t ADCmin)	{fADCmin2	=ADCmin;}	
void TAnalysis::SetCh2ADCmax(Int_t ADCmax)	{fADCmax2	=ADCmax;}
void TAnalysis::SetCh2HIT(Int_t hit)		{fhit2		=hit;}	


void TAnalysis::GetNeighbours(int channel, char opt,int * size, int **arr){

	int * larr = *arr; // puntatore larr = CONTENUTO di arr
	//int lgeo = channel/64; 
	int lchannel = channel; //lgeo*64 betwwen 0..63
	int lsize = 8;
	if (opt==0) {// MAROC CHANNEL
		lsize =2;
		larr[0]=lchannel-1;
		larr[1]=lchannel+1;
		if (lchannel==0)  {lsize=1;larr[0]=lchannel+1;}
		if (lchannel==63) {lsize=1;larr[0]=lchannel-1;}
	
	}if (opt==1) {
		
		int col= lchannel%8;
		int raw= lchannel/8;
		
		//printf("channel %4d col %d raw %d: ",lchannel,col,raw);
		switch (col) {
			case 0:
				lsize=3;
				switch (raw) {
					case 0:
						//printf("BR");
						larr[0]=lchannel+8;
						larr[1]=lchannel+12;
						larr[2]=lchannel+4;
						break;
					case 7:
						//printf("TR");
						larr[0]=lchannel+4;
						larr[1]=lchannel-4;
						larr[2]=lchannel-8;
						break;					
					default:
						//printf("ER");
						lsize=5;	
						larr[0]=lchannel+8;
						larr[1]=lchannel-8;	
						larr[2]=lchannel+4;
						larr[3]=lchannel-4;							
						larr[4]=lchannel+12;
						break;
				}
				break;	
			case 6:
				lsize=3;
				switch (raw) {
					case 0://printf("BL");
						larr[0]=lchannel+8;
						larr[1]=lchannel+4;
						larr[2]=lchannel-4;
						break;
					case 7://printf("TL");
						larr[0]=lchannel-4;
						larr[1]=lchannel-8;
						larr[2]=lchannel-12;
						break;					
					default:
						//printf("EL");	
						lsize=5;
						larr[0]=lchannel+8;
						larr[1]=lchannel-8;	
						larr[2]=lchannel+4;
						larr[3]=lchannel-4;							
						larr[4]=lchannel-12;
						break;
				}	
				break;
			default: // any coulumn but 0 and 6 
				if (raw==0) {
					//printf("EB");
					lsize=5;
					larr[0]=lchannel+8;
					larr[1]=lchannel+4;
					switch (col) {
						case 1:	
							larr[2]=lchannel+11;
							larr[3]=lchannel+3;							
							larr[4]=lchannel+12;
							break;
						case 2:
							larr[2]=lchannel+13;
							larr[3]=lchannel+5;							
							larr[4]=lchannel+12;
							break;
						case 3:
							larr[2]=lchannel+10;
							larr[3]=lchannel+2;							
							larr[4]=lchannel+12;
							break;
						case 4:	
							larr[2]=lchannel+5;
							larr[3]=lchannel-3;							
							larr[4]=lchannel-4;
							break;
						case 5:
							larr[2]=lchannel+6;
							larr[3]=lchannel-2;							
							larr[4]=lchannel-4;
							break;
						case 7:
							larr[2]=lchannel+3;
							larr[3]=lchannel-5;							
							larr[4]=lchannel-4;
							break;
						default:
							printf("ERROR\n");
							break;
					}
				}else {
					if (raw==7) { 	
					//	printf("ET");
						lsize=5;
						larr[0]=lchannel-8; 
						larr[1]=lchannel-4;	
						switch (col) {
							case 1:						
								larr[2]=lchannel+3; 
								larr[3]=lchannel-5;
								larr[4]=lchannel+4;
								break;
							case 2:		
								larr[2]=lchannel+4;
								larr[3]=lchannel+5;							
								larr[4]=lchannel-3;
								break;
							case 3:	
								larr[2]=lchannel+4;
								larr[3]=lchannel+2;							
								larr[4]=lchannel-6;
								break;
							case 4:
								larr[2]=lchannel-12;
								larr[3]=lchannel-3;							
								larr[4]=lchannel-11;	 
								break;
							case 5:
								larr[2]=lchannel-12;
								larr[3]=lchannel-2;							
								larr[4]=lchannel-10;	 
								break;
							case 7:
								larr[2]=lchannel-13;
								larr[3]=lchannel-5;							
								larr[4]=lchannel-13;	
								break;
							default:printf("ERROR\n");break;
						}
						
					}else { 
						lsize=8;
						larr[0]=lchannel+8; 
						larr[1]=lchannel-8; 
						larr[2]=lchannel+4; 
						larr[3]=lchannel-4; 
						switch (col) {
							case 1:
								larr[4]=lchannel+12; 
								larr[5]=lchannel+11; 
								larr[6]=lchannel+3; 
								larr[7]=lchannel-5; 
								break;
							case 2:
								larr[4]=lchannel+12; 
								larr[5]=lchannel+13; 
								larr[6]=lchannel-3; 
								larr[7]=lchannel+5; 								
								break;
							case 3:
								larr[4]=lchannel+12; 
								larr[5]=lchannel+10; 
								larr[6]=lchannel+2; 
								larr[7]=lchannel-6; 							
								break;
							case 4:
								larr[4]=lchannel-12;
								larr[5]=lchannel-11; 
								larr[6]=lchannel-3; 
								larr[7]=lchannel+5; 								
								break;
							case 5:
								larr[4]=lchannel-12;
								larr[5]=lchannel-10; 
								larr[6]=lchannel-2; 
								larr[7]=lchannel+6; 								
								break;
							case 7:
								larr[4]=lchannel-12; 
								larr[5]=lchannel-13; 
								larr[6]=lchannel-5; 
								larr[7]=lchannel+3; 								
								break;		
							default:
								printf("ERROR\n");
								break;
						}
					}

				}
				break;
		}
	}
	*size = lsize;
} 

// Probability of observing n events when mean intensity is m is poisson distributed P(n,m)
// P(n,m) = m^(n)*exp(-m)/n!
// P(0,m) = exp(-m) // numero di piedistalli attesi
// P(1,m) = m*exp(-m)// numero di eventi single foton attesi

//  timelapse for 64 channels 4 minutes (XTalk XTalkMAPMT m=2 first neighborough)

Int_t Multiplicity(int maxevent = 10, int maxmultiplicity = 64)
{	
	const int nfile = 3;
	const char * file[10];
	int i=0;
	for (i=0; i<nfile; i++) {
		file[i]=Form("../data/raw/runM_%05d.bin",i+36);
		printf("FILE[%d] %s\n",i,file[i]);
		TAnalysis * A = new TAnalysis(file[i]);
		A->Read(maxevent);
		A->HitMultiplicity(0,maxmultiplicity);
		delete A;
		
	}
	return i-1;	
}

Int_t CrosstalkDistribution(Int_t ch1=2048,Int_t ch2=2049,int mMax=32){
	TAnalysis *	 A = new TAnalysis("../data/raw/runM_00036.bin");
	TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("mycanv");if(old){delete old;}
	TCanvas* mycanv = new TCanvas("mycanv","Multiplicity Distribution",PIXX-2*SIZEOFCANV,0,2*SIZEOFCANV,1.5*SIZEOFCANV); 
	TH1F * dummy = (TH1F*) gDirectory->FindObject("h");if(dummy) delete dummy;
	TH1F * h= (TH1F*) new TH1F("h", Form(" Event Multiplicity involving %d  and %d",ch1,ch2),mMax,0-0.5,mMax-0.5);
	for (int m=0; m<=mMax; m++){
	Double_t n = A->PlotADC(ch1,1,1200,2600,m,"==",ch2,1,1200,2600,1);
		//printf("\t\t\t\t\t\t %2d %4.0f \n",m,n);
		h->Fill(m,n);//for (int ii=0; ii<n; ii++) {h->Fill(m);}
	}
	h->DrawCopy();
	mycanv->Update();
	return 0;
}



Int_t Crosstalk(Int_t channel = 2049,Int_t m = 2){
	
	//TAnalysis *	A = new TAnalysis("../data/out/SelfTrig_run_00099.bin"); // DAC= 249, LOW THR
	//TAnalysis *	 A = new TAnalysis("../data/out/SelfTrig_run_00020.bin");  // DAC0=170, pedestal 
	TAnalysis *	 A = new TAnalysis("../data/out/SelfTrig_run_00350.bin");// DAC0= 350, high threshold
	
	
	int size = 0;
	int * arr = new int[8];
	
	//A->Read(nomefile,100000);

	//A->SetFitON(true);
	
	Int_t ADCmin1 = 1200;
	Int_t ADCmax1 = 2600;
	Int_t ADCmin2 = 1200;
	Int_t ADCmax2 = 2600;	
	
	Double_t Entries = A->PlotADC(2048);
	
	Double_t lEntries[4096];
	Double_t lNPed[4096];
	Double_t lNHit[4096];
	Double_t lNHit1[4096];		
	Double_t lNHit2[4096];	
	Double_t lNHit2prev[4096];	
	Double_t lNHit3ormore[4096];	
	Double_t intensity[4096];	
//	Double_t lNxtalk[4096][4096];	
	
	for (int ch = 0; ch<4096; ch++) {
		lEntries[ch]=Entries;
		lNPed[ch]=0.;
		lNHit[ch]=0.;
		lNHit1[ch]=0.;	
		lNHit2[ch]=0.;		
		lNHit3ormore[ch]=0.0;
		intensity[ch]=0.;
		
//		for (int cc = 0; cc<4096; cc++) {
//			lNxtalk[ch][cc]=0.0;	
//		}		
	}
	
	
	printf("Channel  Entries  Pedestal Poiss  N(1) Signal Single Double XtMRC XtMAPMT ( XtTOT Miss More)\n");
	for (int ch1 = 2048; ch1<=2111; ch1++) {
	
		printf("%7d ",ch1);
		printf("%8d ", (Int_t)lEntries[ch1]);
		
		// Pedestal Events
		lNPed[ch1]  = A->PlotADC(ch1,0);
		intensity[ch1] = -1 * log(lNPed[ch1]/Entries);		
		printf("%8d ", (Int_t)lNPed[ch1]);	
		printf("%6.3f ", intensity[ch1]);
		printf("%5d ", (Int_t)(intensity[ch1]*exp(-intensity[ch1])*Entries));	
		
		// Signal
		lNHit[ch1]  = A->PlotADC(ch1,1);
		printf("%6d ", (Int_t)lNHit[ch1]);	
		lNHit1[ch1] = A->PlotADC(ch1,1,ADCmin1,ADCmax1,1,"==");	
		printf("%6d ", (Int_t)lNHit1[ch1]);		
		lNHit2[ch1] = A->PlotADC(ch1,1,ADCmin1,ADCmax1,2,"==");		
		printf("%6d ", (Int_t)lNHit2[ch1]);			
		
		// Xtalk Maroc
		A->GetNeighbours(ch1,0,&size,&arr); // 1 MAMPT, 0 MAROC	
		int ch2;
		Double_t XtalkMRC = 0.;
		for (int i =0; i<size; i++) {
			ch2 = arr[i];
			lNHit2prev[ch1] = A->PlotADC(ch1,1,ADCmin1,ADCmax1,2,"==",ch2,1,ADCmin2,ADCmax2);	
			XtalkMRC += lNHit2prev[ch1];
			//printf(" Ch%d_&&_Ch%d %4d \n",ch1,ch2, (Int_t)lNHit2prev[ch1]);	
		}
		printf("%5d",(Int_t)XtalkMRC);
		
		// Xtalk MAPMT
		A->GetNeighbours(ch1,1,&size,&arr); // 1 MAMPT, 0 MAROC	
		Double_t XtalkMAPMT = 0.;
		for (int i =0; i<size; i++) {
			ch2 = arr[i];
			//printf(" Fuori: vicino_%d (%d) %d \n",i,arr[i],A->abs2maroc(arr[i]));
			lNHit2prev[ch1] = A->PlotADC(ch1,1,ADCmin1,ADCmax1,2,"==",ch2,1,ADCmin2,ADCmax2);	
			XtalkMAPMT += lNHit2prev[ch1];
			//printf(" Ch%d_&&_Ch%d %4d \n",ch1,ch2, (Int_t)lNHit2prev[ch1]);	
		}
		printf("%5d   ",(Int_t)XtalkMAPMT);
		
		// Xtalk Total
		Int_t Xtot = (Int_t)XtalkMAPMT+(Int_t)XtalkMRC;
		printf("%6d",Xtot);
		//printf("%6.0f ",10000*(Xtot/lEntries[ch1]));		
		
		
		// Remaining events
		printf("%6d  ",(Int_t)lNHit2[ch1]-Xtot);
		//printf("%6.1f  ",10000*((lNHit2[ch1]-Xtot)/lEntries[ch1]));
	
		lNHit3ormore[ch1] = A->PlotADC(ch1,1,ADCmin1,ADCmax1,3,">=");
		printf("%4d ", (Int_t)lNHit3ormore[ch1]);
		//printf("%6.0f ", 10000*(lNHit3ormore[ch1]/lEntries[ch1]));
		
		printf("\n");
	}
	

	A->SetGraphicsON(true);
	
	Float_t semisize = 25;
	Int_t binx = 8;
	Int_t biny = 8;
	Double_t xlo = -semisize;
	Double_t ylo = -semisize;
	Double_t xhi = +semisize;
	Double_t yhi = +semisize;
	
	TH2F * dummy = (TH2F*) gDirectory->FindObject("h2");if(dummy) delete dummy;
	
	TH2F *h2 = new TH2F("h2",Form("CrossTalk[%d] ",channel),binx,xlo,xhi,biny,ylo,yhi);
	
	A->ResetCoordinates();
	const char * mapfile = "./Maps/channelmap.txt";
	bool success = A->SetCoordinates(mapfile);if (!success) {printf("problem with file %s\n",mapfile);}
	

	for (int ch2 = 2048; ch2<=2111; ch2++) {
		//if (ch2!=2065) {
			Double_t x = A->PlotADC(channel,1,ADCmin1,ADCmax1,m,">=",ch2,1,ADCmin2,ADCmax2);
			h2->Fill(A->GetX(ch2),A->GetY(ch2),x);	
			printf("%5.0lf events involving  %d AND %d (%2d)\n",x,channel,ch2,A->abs2maroc(ch2));	

		//}
	}
	Float_t A1 = 26*1.5; // mm 
	TCanvas*  mycanv = new TCanvas("mycanv","Xtalk",PIXX-SIZEOFCANV,0,SIZEOFCANV,SIZEOFCANV);
	mycanv->Range(-A1,-A1,A1,A1);
	h2->Draw("TEXT,COLZ");
	//h2->Draw("LEGO");	
	mycanv->Update();
	
	
	// Save or Delete? ans: to fix 
	char nome[200];
	sprintf(nome,"./eps/prova.eps");
	mycanv->DrawClone();
	mycanv->Print(nome);
	delete mycanv;
	
	 
	delete arr;
	delete A;
	return 0;
}



Int_t Parse(){

	const int maxfile=5; 
	const int maxsize=200; 	
	int runid[maxfile];
	
	runid[0]= 453;
	runid[1]= 601;	
	runid[2]= 610;
	runid[3]= 932;
	runid[4]= 934;

	/* prefix THR
	runid[0]= 546;
	runid[1]= 611;	
	runid[2]= 919;
	runid[3]= 920;
	runid[4]= 921;	
	runid[5]= 922;	
	runid[6]= 1158;	
	// Decay
	runid[7]= 1267;	
	runid[8]= 1359;	
	runid[9]= 1451;	
	runid[10]= 1543;	
	runid[11]= 1635;	
	runid[12]= 1727;	
	runid[13]= 1819;	
	runid[14]= 1911;
	runid[15]= 2003;	
	runid[16]= 2095;	
	runid[17]= 2187;	
	runid[18]= 2279;	
	runid[19]= 2371;	
	runid[20]= 2463;	
	runid[21]= 2555;	
	runid[22]= 2647;	
	runid[23]= 2739;	
	runid[24]= 2831;	
	runid[25]= 2923;	
	runid[26]= 3015;	
	runid[27]= 3107;	
	runid[28]= 3199;	
	runid[29]= 3291;	
	runid[30]= 3383;	
	runid[31]= 3475;	
	runid[32]= 3567;	
	runid[33]= 3659;	
	runid[34]= 3751;
	runid[35]= 3843;
	runid[36]= 3935;
	runid[37]= 4027;	
	runid[38]= 4119;	
	runid[39]= 4211;	
	runid[40]= 4303;	
	runid[41]= 4395;	
	runid[42]= 4487;	
	runid[43]= 4579;	
	runid[44]= 4763;	
	runid[45]= 4855;	
	runid[46]= 4947;	
	runid[47]= 5039;	
	runid[48]= 5131;	
	runid[49]= 5223;

	*/
	int nevents = 0;
	char filename[maxfile][maxsize];
	printf("Hello! %d file going to be processed\n",maxfile);
	for (int j=0; j<maxfile; j++) {
		for (int k=0; k<maxsize; k++) {
			filename[j][k]=0;
		}
		sprintf(filename[j],"../data/out/Laser_%05d.bin",runid[j]);
		printf("%s",filename[j]);
		TAnalysis * A = new TAnalysis(filename[j]);
		nevents = A->Read(1000000);
		printf(" Events  = %6d\n",nevents);
		delete A;
	}
	return 0;
}


