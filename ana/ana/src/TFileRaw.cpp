

#include "TFileRaw.h"



TFileRaw::~TFileRaw()
{
	if (fdebugmode){printf("[TFileRaw::~TFileRaw()]\n");}
	TH1I * dummy = (TH1I*) gDirectory->FindObject("f_histo1");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_histo2");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_histo3");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_histo4");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_histo5");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_histo6");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_hp1");
	if(dummy) delete dummy;
	dummy = (TH1I*)gDirectory->FindObject("f_hp2");
	if(dummy) delete dummy;
};


TFileRaw::TFileRaw()
{	fdebugmode=false;
	fLevel= 0; // default level

	if (fdebugmode){printf("[TFileRaw::TFileRaw()]\n");}
	// Reset all counters (of all levels) and reset Parsing FSM
	ResetFileLevel();
	ResetBlockLevel();
	ResetEventLevel();
	ResetCardLevel();
	ResetExtrema();
	ResetHistoPointers();
	Reset_FSM();
	Reset_Counters();
};

void TFileRaw::Print()
{	
		if(GetBlockIndex() % 10000 == 0 )
	{
		cout<<" Parsing block " <<GetBlockIndex()<<"..."<<endl;
	}
	
	//------- according to fFileDebugLevel 
	
	// Different kind of printing could be managed with a parameter as show below.
	// Least significant nibble is dedicated to FileStat counter/index
	// Most significant nibble is dedicated to errors in parsing or flimsiness of the trailer record
	
	// ID	       Print:	fFileDebug = 0x01 =   1	(odd value to say where we are)
	// Block size  Print:	fFileDebug = 0x02 =   2	
	// Event size  Print:	fFileDebug = 0x04 =   4	
	// Card	 size  Print:	fFileDebug = 0x08 =   8	

	// FSM State			fFileDebug = 0x10 =  16
	// Empty Card counter   fFileDebug = 0x20 =  32 (& Record flimsiness)
	// Error counter status fFileDebug = 0x40 =  64
	// Operation Flux		fFileDebug = 0x80 = 128


	// if fFileDebug = 0 (default) No info printing except for a '.' every 10.000 Blocks.
	// with fFileDebug =255 you print all.

	if (fdebugmode){printf("[TFileRaw::Print()]\n");}

	if(fLevel==1)
	{	
		printf("       IDs: Block    = %4ld, Header = %5ld Trailer = %5ld, Event = %4ld, Card = %2d;\n",fFblock,fFheader,fFtrailer,fFevent,fCfe);
	}

	if(fLevel==2)
	{	
		printf("BLOCK_size: Dword    = %4d (max = %4d),   Events = %5d (max = %5d);\n",fBdword,fBdword_max,fBevent,fBevent_max);
	}

	if(fLevel==4)
	{
		printf("EVENT_size: Channels = %4d (max = %4d)\n",fEdata,fEdata_max);
	}

	if (fLevel==8)
	{
		printf("CARD__size: Channels = %4d\n",fCdata);
	}

	if (fLevel==16)
	{
		printf("       FSM: State    = %4d \n",f_state);
	}
	
	if (fLevel==32)
	{
		printf(" Empty card counter  = %4d \n",f_Empty_Card);
		printf(" Flimsiness counter  = %4d \n",f_Record_Flimsiness);
	}
	if (fLevel==64)
	{
		printf("FSM Error Register   = ");
		if(f_FirstH_Lack	!=0){printf("1");}else{printf("0");}
		if(f_FirstHD_Lack	!=0){printf("1");}else{printf("0");}
		if(f_H_Lack			!=0){printf("1");}else{printf("0");}
		if(f_HD_Lack		!=0){printf("1");}else{printf("0");}
		if(f_DT_Lack		!=0){printf("1");}else{printf("0");}
		if(f_T_Lack			!=0){printf("1");}else{printf("0");}
		printf("\n");
	}

};


//----------------------------------------------------------------- Resetters
void TFileRaw::ResetEventLevel()
{
	if (fdebugmode){printf("[TFileRaw::ResetEventLevel()]: erased value = %d\n",fEdata);}
	fEdata		= 0; 
};
void TFileRaw::ResetBlockLevel()
{
	if (fdebugmode){printf("[TFileRaw::ResetBlockLevel()]\n");}
	fBdword		= 0; 
	fBevent		= 0; 


};
void TFileRaw::ResetFileLevel()
{
	if (fdebugmode){printf("[TFileRaw::ResetFileLevel()]\n");}
	fFblock		= 0; 
	fFevent		= 0; 
	fFheader	= 0; 
	fFtrailer	= 0; 

};

void TFileRaw::ResetCardLevel()
{
	if (fdebugmode){printf("[TFileRaw::ResetCardLevel()]\n");}
	fCdata		= 0; 
	fCfe		= -1;
};

void TFileRaw::ResetExtrema()
{
	if (fdebugmode){printf("[TFileRaw::ResetExtrema()]\n");}
	fBdword_max	= 0; 
	fBevent_max	= 0; 
	fEdata_max	= 0; 
};

void TFileRaw::ResetHistoPointers()
{
	if (fdebugmode){printf("[TFileRaw::ResetHistoPointers()]\n");}
	//making sure of pointer's initializing 
	f_histo1	= 0;
	f_histo2	= 0;
	f_histo3	= 0;
	f_histo4    = 0;
	f_histo5	= 0;
	f_histo6	= 0;

	f_hp1		= 0;
	f_hp2		= 0;

	f_canvas	= 0;
	f_canvas1	= 0;
};


void TFileRaw::Init_Chip_Histo()
{
	if (fdebugmode){printf("[TFileRaw::Init_Chip_Histo()]\n");}
// Prima di (ri)creare gli histogrammi faccio un bel check ed un eventuale pulizia
	TH1I * dummy = (TH1I*) gDirectory->FindObject("f_hp1");	if(dummy){delete dummy;}
	dummy = (TH1I*) gDirectory->FindObject("f_hp2");	if(dummy){delete dummy;}
	delete dummy;

	// Histogrammi 
	f_hp1 = new TH1I("f_hp1"," Total Reading Occurrence",64,  0.5,  64.5); 
	f_hp2 = new TH1I("f_hp2"," Empty Reading Occurrence",64,  0.5,  64.5);

};

void TFileRaw::Init_Histo()
{	
	if (fdebugmode){printf("[TFileRaw::Init_Histo()]\n");}
	int a,b,c,d,e; // int_ binning [0, double_t binning]

	//Prima un bel Refresh per evitare warnings in seguito ad eventuali reiterate esecuzioni..
	TH1I * dummy = (TH1I*) gDirectory->FindObject("f_histo1"); if(dummy){delete dummy;}
	
	dummy = (TH1I*) gDirectory->FindObject("f_histo2");if(dummy){delete dummy;}
	dummy = (TH1I*) gDirectory->FindObject("f_histo3");if(dummy){delete dummy;}
	dummy = (TH1I*) gDirectory->FindObject("f_histo4");if(dummy){delete dummy;}
	dummy = (TH1I*) gDirectory->FindObject("f_histo5");if(dummy){delete dummy;}
	dummy = (TH1I*) gDirectory->FindObject("f_histo6");if(dummy){delete dummy;}
	delete dummy;




	a  = GetMAXB_NWord()*1.2;
	f_histo1 = new TH1I("f_histo1","  DWord Distribution"   ,a,0, a); 


	b  =  (GetBlockIndex()*1.2);
	f_histo2 = new TH1I("f_histo2","  DWord Troughout File"  ,b,0, b); 
	f_histo4 = new TH1I("f_histo4","  Event Troughout File " ,b,0, b); 

	if (GetMAXB_event()==1){//Single Event
		c=4;
		f_histo3 = new TH1I("f_histo3","  Event # Distribution (Single Event)",c,0, c);  
	} 

	else{//Multi Event
		c = GetMAXB_event()*2;
		f_histo3 = new TH1I("f_histo3","  Event # Distribution (Multi Event)" ,c,0, c);  
	} 
	
	d= 70;

	f_histo5 = new TH1I("f_histo5","  Channels per Card",d,0, d); 

	if(GetMAXE_DW()!=0){e= GetMAXE_DW()*1.2;}else{e=1.2;}
	
	f_histo6 = new TH1I("f_histo6","  Channels per Event",e,0, e); 


/*

	cout<<" Terminata Lettura del File " <<endl;
	cout<<" Inizializzo gli Histogrammi a partire dai seguenti valori:"            <<endl;
	cout<<" Blocchi totali                            = "<<GetBlockIndex()     <<"->"<<b<<endl;
	cout<<" Eventi totali                             = "<<GetEvent()              <<endl;    
	cout<<" Numero Max di Parole per blocco           = "<<GetMAXB_NWord()<<"->"<<a<<endl;
	cout<<" Numero Max di Eventi per blocco           = "<<GetMAXB_event()<<"->"<<c<<endl;
	cout<<" Numero Max di Canali coinvolti per Evento = "<<GetMAXE_DW()	  <<"->"<<e<<endl;
*/

};




void TFileRaw::FillB_Nword()
{
	if (fdebugmode){printf("[TFileRaw::FillB_Nword()]\n");}
	f_histo1->Fill(fBdword,1);
	f_histo2->SetBinContent(fFblock,fBdword);

};

void TFileRaw::FillB_NEvent()
{
	if (fdebugmode){printf("[TFileRaw::FillB_NEvent()]\n");}
	f_histo3->Fill(fBevent,1);
	f_histo4->SetBinContent(fFblock,fBevent);
};


void  TFileRaw::Set_Block_NEvent(int totevent)
{
	
	// when you are goig to close a Block 
	// the Event number of the block is to be note down in the appropriate TParsing data member
	fBevent = totevent;
	if (fdebugmode){printf("[TFileRaw::Set_Block_NEvent()] Number of Event = %3d\n",GetNEvent());}
};





void TFileRaw::DrawBlockInfo(){

	if (fdebugmode){printf("[TFileRaw::DrawBlockInfo()]\n");}

	f_canvas1 = new TCanvas("f_canvas1","File Structure");  

	f_canvas1->Divide(3,2); // (col, raw)
	f_canvas1->Update();


//	printf("Cd(2)...");
	f_canvas1->cd(2);
	f_histo1->GetXaxis()->SetTitle(" # Words in Block");
	f_canvas1->Update();
//	f_histo1->GetYaxis()->SetTitle(" Occurrence");
	f_canvas1->Update();
	f_histo1->Draw();
	f_canvas1->Update();
//	printf("ok\n");
	


//	printf("Cd(1)...");
	f_canvas1->cd(1);
	f_histo2->GetXaxis()->SetTitle(" Block [#]");
	f_canvas1->Update();
	f_histo2->GetYaxis()->SetTitle(" DWord [#]");
	f_canvas1->Update();
	f_histo2->Draw();
	f_canvas1->Update();
//	printf("ok\n");


//	printf("Cd(5)...");
	f_canvas1->cd(5);
	f_histo3->GetXaxis()->SetTitle(" # Events  in Block");
	f_canvas1->Update();
//	f_histo3->GetYaxis()->SetTitle(" Occurrence");
	f_canvas1->Update();
	f_histo3->Draw();
	f_canvas1->Update();
//	printf("ok\n");


//	printf("Cd(4)...");
	f_canvas1->cd(4);
	f_histo4->GetXaxis()->SetTitle(" Block [#]");
	f_canvas1->Update();
	f_histo4->GetYaxis()->SetTitle(" Event [#]");
	f_canvas1->Update();
	f_histo4->Draw();
	f_canvas1->Update();
//	printf("ok\n");



//	printf("Cd(3)...");
	f_canvas1->cd(3);
	f_histo5->GetXaxis()->SetTitle(" # of data in FE Card");
	f_canvas1->Update();
//	f_histo5->GetYaxis()->SetTitle(" Occurrence");
	f_canvas1->Update();
	f_histo5->Draw();
	f_canvas1->Update();
//		printf("ok\n");


//	printf("Cd(6)...");
	f_canvas1->cd(6);
	f_histo6->GetXaxis()->SetTitle(" # of data in Event");
	f_canvas1->Update();
//	f_histo6->GetYaxis()->SetTitle(" Occurrence");
	f_canvas1->Update();
	f_histo6->Draw();
	f_canvas1->Update();
//	printf("ok\n");


	AddHisto2RootFile(); // salvataggio histogrammi nel file root
	
	f_canvas1->Close(); 
};

void TFileRaw::AddHisto2RootFile(){

	if (fdebugmode){printf("[TFileRaw::AddHisto2RootFile()]\n");}

	f_histo1->Write("",TObject::kOverwrite);
	f_histo2->Write();
	f_histo3->Write();
	f_histo4->Write();
	f_histo5->Write();
	f_histo6->Write();	
};


//------------------------------------------------------------------- FSM per il Parsing


void TFileRaw::Reset_FSM()
{
	// Stato iniziale della FSM
	f_state				=-1; 
	
	//contatori di erroris
	f_T_Lack			=0; 
	f_DT_Lack			=0;
	f_H_Lack			=0;
	f_HD_Lack			=0;
	f_FirstH_Lack		=0;
	f_FirstHD_Lack		=0;
	f_Empty_Card		=0;
	f_Record_Flimsiness	=0;
	
	if (fdebugmode){printf("[TFileRaw::Reset_FSM()]\n");}

};


bool TFileRaw::LookFor_T_Lack(){
	if(GetState()==2){
		f_T_Lack++;
		return true;
	}
	else{
		return false;
	}
};

bool TFileRaw::LookFor_DT_Lack(){
	if(GetState()==1){
		f_DT_Lack++;
		return true;
	}
	else{
		return false;
	}
};
bool TFileRaw::LookFor_H_Lack(){
	if(GetState()==3){
		f_H_Lack++;
		return true;
	}
	else{
		return false;
	}
};
bool TFileRaw::LookFor_FirstH_Lack(){
	if(GetState()==-1){
		f_FirstH_Lack++;
		return true;
	}
	else{
		return false;
	}
};
bool TFileRaw::LookFor_HD_Lack(){
		if(GetState()==3){
		f_HD_Lack++;
		return true;
	}
	else{
		return false;
	}
};
bool TFileRaw::LookFor_FirstHD_Lack(){
	if(GetState()==-1){
		f_HD_Lack++;
		return true;
	}
	else{
		return false;
	}
};

bool TFileRaw::Empty_Card_Check(){
	if(GetState()==1){f_Empty_Card++;return true;}
	else{return false;}	
};

void TFileRaw::UpDateCardStat(){
	if (fCdata!=0){
		f_histo5->Fill(fCdata,1);
	}


	//printf("[Fill DW_in_card]\n");
};

void TFileRaw::FillE_NDword(){
	if (fEdata!=0){
		f_histo6->Fill(fEdata,1);
	}
};





void TFileRaw::Header_Supervisor(){

	IncrHeadCount();
	ResetCardLevel();
	LookFor_T_Lack();
	LookFor_DT_Lack();
	
	SetH(); 
};

void TFileRaw::Data_Supervisor(){
	
	Incr_card_DW_num();
	Incr_event_DW_num();		
			
	LookFor_H_Lack();
	LookFor_FirstH_Lack();

	SetD();

};

void TFileRaw::Trailer_Supervisor(){
	
	IncrTrailCount(); 

	LookFor_HD_Lack();
	LookFor_FirstHD_Lack();
	Empty_Card_Check();
		
	
	SetT();
};

void TFileRaw::Compare_Block_Maxima(){
	if(Get_NWord()>fBdword_max){
		fBdword_max= Get_NWord();
	}
	if(GetNEvent()>fBevent_max){
		fBevent_max=GetNEvent();
	}
};


void TFileRaw::Compare_Event_Maxima()
{

	if(Get_event_DW_num()>fEdata_max)
	{
		fEdata_max=Get_event_DW_num();
	}
};

void TFileRaw::WriteHistoOnDisk()
{
	if (fdebugmode){printf("[TFileRaw::WriteHistoOnDisk()]\n");}
	// Scrittura Histogrammi su disco (prova.root) 
	f_hp1->Write(); 
	f_hp2->Write(); 
	
	
	
};



//--- Metodi per Statistica sul Coinvolgimento delle Carte

void TFileRaw::Reset_Counters()
{
	if (fdebugmode){printf("[TFileRaw::Reset_Counters()]\n");}
	int i;
	for(i=0;i<64;i++){
		f_maroc_present[i]=0;
		f_maroc_empty[i]=0;
	}
};

unsigned long TFileRaw::GetCount(int maroc)
{
	return f_maroc_present[maroc];
};

unsigned long TFileRaw::GetEmptyCount(int maroc)
{
	return f_maroc_empty[maroc];
};


bool TFileRaw::IsPresent(int maroc)
{
	bool x;
	if (GetCount(maroc)==0)
	{
		x=false;
	}
	else
	{
		x=true;
	}
	return x;
		
};

void TFileRaw::IncrCount(int maroc)
{
	if (maroc<0||maroc>=64)
	{
		cout<< "Error: index out of bounds"<<endl;
	}
	else 
	{
		f_maroc_present[maroc]++;
	}
};


void TFileRaw::IncrEmptyCount(int maroc)
{
	if (maroc<0||maroc>=64)
	{
		cout<< "Error: index out of bounds"<<endl;
	}
	else 
	{
		f_maroc_empty[maroc]++;
	}
};


void TFileRaw::PrintStat()
{	
	int i;
	if (fdebugmode){printf("\n[TFileRaw::PrintStat()]\n");}
	printf("Card Readings occurrence\n");
	for(i=0;i<64;i++)
	{
		printf("%6d|",(int)GetCount(i));
		if((i%16)==15){printf("\n");}
	}
	printf("\n");
	printf("Empty Card occurrence\n");
	for(i=0;i<64;i++)
	{
		printf("%6d|",(int)GetEmptyCount(i));
		if((i%16)==15){printf("\n");}
	}
	printf("\n");



};


void TFileRaw::GraphStat()
{	
	if (fdebugmode){
		printf("[TFileRaw::GraphStat()]\n");
		printf("System Counting Statistics\n");
	}
	int i ;
	
	for(i=0;i<64;i++)
	{
		if (fdebugmode){printf("chip %2d : counts %d \n",i,(int)GetCount(i));}
		f_hp1->SetBinContent(i,GetCount(i));
		f_hp2->SetBinContent(i,GetEmptyCount(i));
	}

	
	f_canvas = new TCanvas("f_canvas","FrontEnd Statistics");
	f_canvas->Divide(2,1); // (col, raw)
	f_canvas->Update(); //Update ci va per forza
	f_canvas->cd(1);
	f_hp1->GetXaxis()->SetTitle(" Fe GeoAddress");
	f_canvas->Update();
//	f_hp1->GetYaxis()->SetTitle(" Total");
	f_canvas->Update();
	f_hp1->Draw();
	f_canvas->Update();
	if(IsNotEmpty())
	{ 
		gPad->SetLogy();
	}
	f_canvas->cd(2);
	f_canvas->Update(); //Update ci va per forza
	f_hp2->GetXaxis()->SetTitle(" Fe GeoAddress");
	f_canvas->Update();
//	f_hp2->GetYaxis()->SetTitle(" Empty");
	f_canvas->Update();
	f_hp2->Draw();
	f_canvas->Update();
	
	f_canvas->Close(); 
};

bool TFileRaw::IsNotEmpty()
{
	// returns 1 if almost one card had been readed (element of f_maroc_present is non-zero) 
	// return 0 if f_maroc_present vector is Empty.
	int accu = 0;
	int i;
	bool x;
	for(i=0;i<64;i++)
	{
		accu = accu+(int)IsPresent(i);
	}
	if(accu == 0)
	{
		x= false;
	}
	else
	{
		x=true;
	}
	return x;
};

