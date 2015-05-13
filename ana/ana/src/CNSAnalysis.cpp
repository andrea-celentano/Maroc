#include "CNSAnalysis.h"

CNSAnalysis::CNSAnalysis(int groupsize)
{
	fGroupSize = groupsize;

	fNGroup = 64/fGroupSize;

	printf("CNS Filter with %d groups of %d channels\n",fNGroup,fGroupSize);
	
	for (int i=0; i<NSUBSETMAX; i++) {
			fhNoise[i] = NULL;
	}
	
	
	for (int i=0; i<fNGroup; i++) {
		fhNoise[i] = new TH1F(Form("hNoise[%d]",i),Form("hNoise[%d]",i),NOISEBIN,NOISEMIN,NOISEMAX);
		//fhNoise[i]->Print();
	}
	
}

CNSAnalysis::~CNSAnalysis()
{
	for (int i=0; i<NSUBSETMAX; i++) {
		if (fhNoise[i]!=NULL) {
			//printf("I'd like to delete fhNoise[%d]\n",i);
			delete fhNoise[i];
		}else {
			//printf("I'will not try to delete fhNoise[%d]\n",i);
		}

	}
	//printf("Bye bye\n");
}


bool CNSAnalysis::Add(float value,int subset)
{
	CheckSubset(subset);
	
	if (fhNoise[subset]!=NULL) {
		fhNoise[subset]->Fill(value);
	//	printf("Histogram[%d] filled with %f\n",subset,value);
		return true;
	}else {
		printf("Warning: wrong subset, nothing filled\n");
		return false;
	}
}


void CNSAnalysis::Print(int subset)
{
	if(CheckSubset(subset)){
		if (fhNoise[subset]!=NULL) {
			fhNoise[subset]->Print();
		}else {
			printf("Warning: fhNoise[%d] not initialized!\n",subset);
		}
	}
}

bool CNSAnalysis::CheckSubset(int subset)
{
	if (subset>=fNGroup) {
		printf("Wrong subset you have %d, maximum is %d!\n",subset,fNGroup);
		return false;
	}else{
		return true;
	}
}
void CNSAnalysis::Plot()
{
	TCanvas *lmc1 = new TCanvas("lmc1","Common Noise Subtraction",CANVPOSX,CANVPOSY,CANVSIZE,CANVSIZE);
	lmc1->Divide(1,fNGroup);
	for (int i=0; i<fNGroup; i++) {
		//lmc1->cd(i+1)->SetLogy(1);
		lmc1->cd(i+1);
		
		fhNoise[i]->Draw();
	}
	lmc1->DrawClone();
	delete lmc1;
}