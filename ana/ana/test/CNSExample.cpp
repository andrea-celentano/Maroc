#include "../src/CNSAnalysis.cpp"

int CNSExample(int chGroupSize=64)
{
	cns = new CNSAnalysis(chGroupSize);
	
	cns->Add(601.,0);
	cns->Add(602.,0);
	cns->Add(603.,0);

	cns->Add(603.,1);
	cns->Add(603.,1);
	cns->Add(603.,1);
	cns->Add(603.,1);
	
	cns->Print(0);
	cns->Print(1);
	
	cns->Plot();
	
	delete cns;
	
	return 0;
}