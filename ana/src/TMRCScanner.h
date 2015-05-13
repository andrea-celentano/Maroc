/*
 *  TMRC_Scanner -  Multi-Run Analysis 
 *
 *	INTERFACE: TMRCScanner.cpp
 *
 *	DEPS: TAnalysis
 *
 *	Access raw data of consecutive series of runs using TAnalysis objects
 *  Storage of relevant single pixel parameters on a root file in ./cal folder
 *	Chip configuration are extracted from associated log configuration file (.txt associated to .bin)
 *
 *	
 *	zz = new TMRCScanner("prova",1682,1841,1000)
 *  zz->Plot("ADCMean[2083]:DAC")
 * 
 *	Author: Matteo Turisini
 *  Date: 2013, May 19th
 *	Rev: 2014, Feb 28th
 *
 *	TO DO/VERIFY/OPTIMIZE 
 *
 *  Add date, trigger mode, preamp gain
 *
 * The constructor with blank argument is used for retrieving a already processed scan
 * use this when you want to retrieve an old scan or compare many scans
 *
 * Tree variables 
 * each raw is a run.
 * Runs are described by single pixel data and side information (date,names,config,) 
 */

#ifndef TMRCSCANNER_H
#define TMRCSCANNER_H

#include "General.h"
#include "TAnalysis.h"
#include <TLegend.h>
#include <TCanvas.h>

#define MAXRUN	1024

using namespace std;

class TMRCScanner{
	
private:
	
	int			frun;					/* RUN ID				*/
	unsigned char fFeNum;				/* Total frontend cards	*/
	int			fdac;					/* Threshold (DAC0)		*/ // please, improve me with individual asic threshold
	int			fdelay;					/* HOLD1 delay line		*/
	
	unsigned int fNevents;				/* Total Events acquired*/	
	float		fDuration;				/* Duration				*/
	float		frate;					/* Events/Duration		*/	
	float		fPollEff;				/* Polling Efficiency	*/
	
	
	Int_t		fADCEntries[MAX_PIXEL];	/* Entries	[#]			*/
	Float_t		fADCMean[MAX_PIXEL];		/* Mean		[ADC units]	*/
	Float_t		fADCRms[MAX_PIXEL];		/* Rms		[ADC units]	*/
	Float_t		fADCMax[MAX_PIXEL];		/* Maximum	[ADC units]	*/
	Float_t		fADCMin[MAX_PIXEL];		/* Minimum	[ADC units]	*/	
	
	Int_t		fHITEntries[MAX_PIXEL];	
	Float_t		fHITMean[MAX_PIXEL];	
	Float_t		fHITRms[MAX_PIXEL];	
	Float_t		fHITMax[MAX_PIXEL];
	Float_t		fHITMin[MAX_PIXEL];		
	
	Float_t		fRatioHITADC[MAX_PIXEL];
	
	Float_t		fHitMultMean;			/* parameters of event multeplicity distribution*/
	Float_t		fHitMultMeanErr;
	Float_t		fHitMultRMS;	
	Float_t		fHitMultRMSErr;

	int			fNRun;						/* total number of runs	*/	
	int			ffirstRun;					/* first run			*/
	char		fcurrRun[MAXRUN][MAX_NAME_LENGH];	/* filenames			*/	
	int			fdly[MAXRUN];				/* HW parameter	(opt)	*/	

	TFile*		ffile;						
	TTree*		ftree;	
	
	char		foutfile[MAX_NAME_LENGH];			/* root filename		*/

public:

	TMRCScanner(const char * prefix, int firstrun, int lastrun, int nbulk=0);
	TMRCScanner();	
	~TMRCScanner();	
	
	void	Recall(const char * oldrootfile);	/* Retrieving already processed scan*/
	void	SingleRunAnalysis(int idx_run=0, int nbulk=10);

	void	Plot(const char * what= "Rate:DAC",const char * cut= "");	
	void	PlotMultiplicity(int opt=0); // 0 = Mean; 1 = RMS

private:
	
	void	InitTree(int bulk=0);
	void	ResetSingleRunVariables(); 	
	int		Scan(int nbulk=0);	
	bool	FixRunNames(const char * prefix,int firstrun,int lastrun);

};
#endif