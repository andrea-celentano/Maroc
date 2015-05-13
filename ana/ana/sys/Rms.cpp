//gROOT->ProcessLine(".x myinit.c");
#include <TTree.h>
#include <TCanvas.h>
void Rms() {
	TTree *t = new TTree("t","Data Single Run");
	t->SetMarkerStyle(21);
	t->SetMarkerSize(1.0);	
	t->ReadFile("prova.txt"); 
	
	TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c");if(old){delete old;}
	TCanvas *c = new TCanvas("c","Single Run");	
	c->Divide(2,1);
	c->cd(1);
	t->Draw("RmsADC:Ch>>hRmsADC");
	TH2F *h1 = (TH2F*)gPad->GetPrimitive("hRmsADC");
	TAxis *xaxis = h1->GetXaxis();
	xaxis->SetTitle("Channel_ID");
	TAxis *yaxis = h1->GetYaxis();
	yaxis->SetTitle("Rms[ADC unit]");
	h1->SetTitle("Rms");
	t->SetMarkerColor(2);
	t->Draw("RmsHIT:Ch","","SAME");
	gPad->SetGrid();
	
	c->cd(2);
	t->SetMarkerColor(3);
	t->SetMarkerStyle(22);
	t->Draw("(RmsHIT-RmsADC):Ch");
	
}
	
