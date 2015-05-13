//gROOT->ProcessLine(".x myinit.c");
#include <TTree.h>
#include <TCanvas.h>


void Efficiency() {
	gROOT->Reset();
	TTree *t = new TTree("t","Data Single Run");
	t->SetMarkerStyle(21);
	t->SetMarkerSize(1.0);	
	t->ReadFile("prova.txt");
	TCanvas *old = (TCanvas*)gROOT->GetListOfCanvases()->FindObject("c");if(old){delete old;}
	TCanvas *c = new TCanvas("c","Single Run");
	c->cd(1);
	t->Draw("Eff:Ch>>h2");
	TH2F *h = (TH2F*)gPad->GetPrimitive("h2");
	TAxis *xaxis = h->GetXaxis();
	xaxis->SetTitle("Channel_ID");
	TAxis *yaxis = h->GetYaxis();
	yaxis->SetTitle("HIT/TOTAL[#]");
	h->SetTitle("Efficiency");
	gPad->SetGrid();
}
	
