#include "TEvent.h"


TEvent::~TEvent()
{
	//if(fdebugmode){printf("[TEvent::~TEvent()]\n");}	
};

TEvent::TEvent()
{
	fdebugmode = false;
	if(fdebugmode){printf("[TEvent::TEvent()]\n");}
	fTdata		= NULL; // potrebbero dar problemi quando creo nuovi eventi in fase di lettura?
	fFdata		= NULL; // no perchè sono datamember di event e non di Analysis
	Clean();
};

//void TEvent::ClearTreeVariables()
void TEvent::Clean()
{
	for (Int_t i = 0; i<4096;i++) 
	{
		fADC[i]		= 0;
		fhit[i]		= 0;

		if (i<64){ 
			fTimer[i] = 0;
			fSeq[i]	  = 0;
		}
	}
	fEvt		=0;
	fStatusReg	=0;
	fEvtMulteplicity =0;
};

void TEvent::OpenFile(const char* rootfilename){

	char nomefile[100];
	sprintf(nomefile,"%s",rootfilename);
	//printf("TEvent::Openfile(%s)\n",rootfilename);
	if(fFdata!=0){
		printf("Warning: a RootFile already exists! going to delete it and creating a new one\n");
		delete fFdata;
		fFdata = 0;
	}	
	// Create a ROOT File
   // fFdata = new TFile("prova.root", "RECREATE");
	 fFdata = new TFile(nomefile, "RECREATE");
	if(fFdata == 0) {cout << "Error: cannot open output file " <<"prova.root" << endl;return;} 
	if(fFdata->IsZombie()){printf("fFdata is Zombie!\n");}

};  
void TEvent::CreateTree(){
	if(fTdata!=0){
		printf("Warning: a Tree already exists! going to delete it and creating a new one\n");
		delete fTdata;
		fTdata = 0;
	}

	fTdata = new TTree("fTdata","Tree of Data for Analysis");
	
	// for each variable  add a branch to the tree  
	fTdata->Branch("Evt"			,&fEvt				,"Evt/I");
	fTdata->Branch("EvtMultiplicity",&fEvtMulteplicity  ,"EvtMultiplicity/I");	
	fTdata->Branch("Status"			,&fStatusReg		,"Status/S");
	fTdata->Branch("ADC"      ,  fADC	   ,"ADC[4096]		/F");//new! array of adc (real variables - Float_t)
	fTdata->Branch("hit"       , fhit      ,"hit[4096]		/O");// new! array of hit (boolean variables - Bool_t)
    fTdata->Branch("Timer"     , fTimer    ,"Timer[64]		/S");
    fTdata->Branch("Sequential", fSeq      ,"Sequential[64]	/S");
}; 


void TEvent::ConfigureReading(TTree * mytreep){
	if(mytreep==0){
		printf("Error: Trying to configure reading but there's no tree to read! \n");
	}
	else
	{// ok there is a tree
		Clean();
		mytreep->SetBranchAddress("EvtMultiplicity",&fEvtMulteplicity);		
		mytreep->SetBranchAddress("Evt",&fEvt);
		mytreep->SetBranchAddress("Status",&fStatusReg);
		mytreep->SetBranchAddress("ADC",&fADC);
		mytreep->SetBranchAddress("hit",&fhit);
		mytreep->SetBranchAddress("Timer",&fTimer);
		mytreep->SetBranchAddress("Sequential",&fSeq);		
	}
};


Short_t TEvent::GeoCh2AbsCh(unsigned char geoaddress,unsigned short channel)
{
	Short_t ch;
	ch= 64*geoaddress+channel;
	if(ch<0||ch>4095){cout<<"Error: Channel out of range [0 4095]"<<endl;}
	//if(fdebugmode){printf("[TEvent::GeoCh2AbsCh(%d,%d)]\n",geoaddress,channel);}
	return ch;
};


void TEvent::SetEvtIdx(Int_t val)
{
	if ((val <0) || (val>2100000000)){ cout<< "Error: index out of bounds"<<endl; return;}
	fEvt = val;
};




void TEvent::SetSequential(Short_t fe, Short_t data)
{
	if ((fe <0) || (fe>63)){ cout<< "Error: index out of bounds"<<endl; return;}
	fSeq[fe] = data;
};

void TEvent::SetTimer(Short_t fe, Short_t data)
{
	if ((fe <0) || (fe>63)) { cout<< "Error: index out of bounds"<<endl; return;}
	fTimer[fe] = data;
};

void TEvent::SetADC(Short_t channel, Float_t data)
{
	if ((channel <0) || (channel>4095)){ cout<< "Error: index out of bounds"<<endl; return;}
	fADC[channel] = data;
};


void TEvent::SetHit(Short_t channel)
{
	if ((channel <0) || (channel>4095)){ cout<< "Error: index out of bounds"<<endl; return;}
	fhit[channel] = 1;
}



void TEvent::FillTree()
{	
	if(fdebugmode){printf("[TEvent::FillTree()]\n");}
	if(fTdata==NULL){printf("Error! fTdata = NULL\n");}
	else{ //ready to fill
		int Nbyte = fTdata->Fill(); 
		if(Nbyte<=0){
			cout <<"Error during Filling Tree. Fill returned "<< Nbyte << endl;
		}
	}
};


void TEvent::WriteTreeOnDisk()
{	// save the tree on file without closing the file
	if(fTdata==NULL){
		printf("Warning! fTdata = NULL\n data are not written");
	}
	else {
		fTdata->Write(); 
	}
	if(fdebugmode){
		printf("[TEvent::Writing Tree()] current directory is ");
		gDirectory->pwd(); // automatic \n
	}		
};


void TEvent::CloseFile()
{
	fFdata->Close();  
	if(fdebugmode){printf("[TEvent::Close File]\n");}
};



Float_t TEvent::ComposeDatum(int ADC,Float_t pedestal)
{		
	Float_t datum = (ADC - pedestal );

	if(fdebugmode){
		if(datum>2000){
			printf("%d - %4.1f = %4.2f\n",ADC,pedestal,datum);
		}
	}

	return datum;
};



void TEvent::Print()
{	// Print information about Event from the variables related to the Tree
	int i=0; 
	int k=0;
	
	
	//printf("Verbosity level = %d\n",fLevel);
	

	
	
	if((fLevel&gmaskbit1)==gmaskbit1){// Print Tag Map if !=0
		printf("Tag Map minimal\n");
		printf("*  GEO \tSEQ \tTIMER \n");
		for(i=0;i<64;i++){
			if((GetSequential(i)||GetTimer(i))!=0){
				printf("*%5d\t%3d\t  %3d ",i,GetSequential(i),GetTimer(i));
				if((i%16)==15){ // Back pannel
					printf("|| BP %d \n",k);
					k++;
				}; 
				printf("\n");
			}
		}
		printf("***********************\n");
	}
	
	if((fLevel&gmaskbit2)==gmaskbit2){// Hit Map	
		printf("Hit Map\n");
		k=0;	
		int lhitcount= 0; // // hit enabled counter (channel per event)
		for(i=0;i<4096;i++)
		{
			if((i%8)==0){printf("|");}
			printf("%d",GetHit(i));	
			if (GetHit(i)==1){// update hit counter
				lhitcount++;
			}
			if(( i & 63)==63){// end of Card
				printf("* GEO %d\n",k);
			k++;
			}
		}
		printf("\n");
		printf("totally %d channels hit enabled\n\n",lhitcount);
		lhitcount=0;
	}



	// ADC Map
	if((fLevel&gmaskbit3)==gmaskbit3){
		printf("ADC Map \n");
		k=0;
		for(i=0;i<4096;i++){
			if(GetADC(i)!=0){	
				printf("%4.1f ",GetADC(i));
				if((i&63)==63){ // end of Card
					printf("* FE %d",k);
					k++;
				};
				if((i&15)==15){ // carriage return
					printf("\n");
				};
			}
		}
	}
};

