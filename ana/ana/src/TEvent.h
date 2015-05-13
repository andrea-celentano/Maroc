//
// This file contains class TEvent definition
// the methods described here can be divided into groups as follow:

#ifndef TEvent_H
#define TEvent_H

#include "TH1I.h"
#include "TCanvas.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TFile.h"
#include "Riostream.h"

#include "General.h"


const unsigned int gmaskbit0=0x00000001;
const unsigned int gmaskbit1=0x00000002;
const unsigned int gmaskbit2=0x00000004;
const unsigned int gmaskbit3=0x00000008;

class TEvent
{	
private:
	TFile*		fFdata;		// ROOT File to storage DataTree and statistic about file structure 
	TTree*		fTdata;
	
	Int_t		fEvt;		// Event ID (counter)
	Int_t		fEvtMulteplicity; // Event Multiplicity (sum of HIT on all channel)
	Short_t     fStatusReg;	// Flags Register (Errors occurrence or Last Event in Block)
	Float_t		fADC[4096]; // ADC value
	Bool_t		fhit[4096];	// digital data 
	Short_t		fTimer[64]; // (Card) Timer TAG 
	Short_t		fSeq[64];	// (Card) Sequential TAG
	bool fdebugmode;
	unsigned int	fLevel;			// coded level for printing 

public:
	//--------------------------------------//
	//	General								//
	//--------------------------------------//
			~TEvent();
			TEvent();

	//--------------------------------------//
	//	Debug								//
	//--------------------------------------//
	void	Print();	// use SetDebug Mode or see TEvent implementation for more
	
	void	ResetDebugMode	(){fdebugmode=false;}
	void	SetDebugMode	(){fdebugmode=true;}
	bool    GetDebugMode    (){return fdebugmode;}
	
	void	SetPrintLevel	(unsigned int code){fLevel=code;} // set printing level // 1 HIT true;2 under thr ;3 All; 4 over thr //5 Header and Trailer // 0 print nothing
	unsigned int  GetPrintLevel(void)	{return fLevel;}
	
	
	//--------------------------------------//
	//	TTre & TFile Managing				//
	//--------------------------------------//
	void	OpenFile(const char * rootfilename="prova.root");  //new
	void	CloseFile();  
	void	CreateTree(); //new
	void	FillTree();						// 
	void	PrintTree(){fTdata->Print();};
	void	WriteTreeOnDisk();				//file prova.root
	void	ConfigureReading(TTree * mytreep);//new

			
	//--------------------------------------//
	//	Data managing						//
	//--------------------------------------//
	void	Clean();		// clear variables associated with tree
	void	SetSequential	(Short_t fe, Short_t data);
	void	SetTimer		(Short_t fe, Short_t data);

	void	SetADC			(Short_t channel, Float_t data);
	void	SetHit			(Short_t channel);
	
	void	SetEvtIdx(Int_t val);
	void	SetStatusRegister(Short_t status){fStatusReg = status;};
	

	
	Int_t	GetEvt()					{return fEvt			;};
	Int_t	GetEvtMultiplicity()		{return fEvtMulteplicity;};	
	void	IncrEvtMultiplicity()		{fEvtMulteplicity++;};
	Short_t GetStatusRegister()			{return fStatusReg		;};

	
	Short_t GetSequential(Short_t fe)	{return fSeq[fe]		;};
	Short_t GetTimer(Short_t fe)		{return fTimer[fe]		;};

	Float_t GetADC(Short_t channel)		{return fADC[channel]	;};
	Bool_t	GetHit(Short_t channel)		{return fhit[channel]	;};

	Short_t GeoCh2AbsCh		(unsigned char geoaddress,unsigned short channel);	//secure translation ([15..0]x[63..0]->[4095..0])	 // better if global function!							
	Float_t ComposeDatum (int ADC, Float_t pedestal=0);
};


#endif