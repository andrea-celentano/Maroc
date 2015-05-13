//gROOT->ProcessLine(".x myinit.c");
#include <TTree.h>
#include <TCanvas.h>
void Mean() {
	gROOT->Reset();
	TTree *t = new TTree("t","Data Single Run");
	t->SetMarkerStyle(21);
	t->SetMarkerSize(1.0);	
	t->ReadFile("prova.txt"); 
	TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c");if(old){delete old;}
	TCanvas *c = new TCanvas("c","Single Run");
	c->Divide(2,1);
	c->cd(1);
	
	t->Draw("MeanADC:Ch>>hADC");
	TH2F *h1 = (TH2F*)gPad->GetPrimitive("hADC");
	TAxis *xaxis = h1->GetXaxis();
	xaxis->SetTitle("Channel_ID");
	TAxis *yaxis = h1->GetYaxis();
	yaxis->SetTitle("Mean[ADC unit]");
	h1->SetTitle("Media");
	t->SetMarkerColor(2);
	t->Draw("MeanHIT:Ch","","SAME");
	gPad->SetGrid();
	
	c->cd(2);
	t->SetMarkerColor(3);
	t->SetMarkerStyle(22);
	t->Draw("(MeanHIT-MeanADC):Ch>>hHIT");
	TH2F *h2 = (TH2F*)gPad->GetPrimitive("hHIT");
	TAxis *xaxis = h2->GetXaxis();
	xaxis->SetTitle("Channel_ID");
	TAxis *yaxis = h2->GetYaxis();
	yaxis->SetTitle("Distance HIT-ADC[ADC unit]");
	h2->SetTitle("Range");
}
	
