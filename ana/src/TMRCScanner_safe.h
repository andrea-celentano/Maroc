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
 */

#ifndef TMRCSCANNER_H
#define TMRCSCANNER_H

#include "General.h"
#include "TAnalysis.h"
#include "TLegend.h"
#include "TCanvas.h"

#define MAXRUN	1024
#define	MAXCHAN	4096
#define NAMESIZE 256


using namespace std;

class TMRCScanner{
	
private:
	
	/* Tree variables 
	 * each raw is a run.
	 * Runs are described by single pixel data and side information (date,names,config,) 
	 */
	
	int			frun;					/* RUN ID				*/
	unsigned char fFeNum;				/* Total frontend cards	*/
	int			fdac;					/* Threshold (DAC0)		*/ // please, improve me with individual asic threshold
	int			fdelay;					/* HOLD1 delay line		*/
	
	unsigned int fNevents;				/* Total Events acquired*/	
	float		fDuration;				/* Duration				*/
	float		frate;					/* Events/Duration		*/	
	float		fPollEff;				/* Polling Efficiency	*/
	
	
	Int_t		fADCEntries[MAXCHAN];	/* Entries	[#]			*/
	Float_t		fADCMean[MAXCHAN];		/* Mean		[ADC units]	*/
	Float_t		fADCRms[MAXCHAN];		/* Rms		[ADC units]	*/
	Float_t		fADCMax[MAXCHAN];		/* Maximum	[ADC units]	*/
	Float_t		fADCMin[MAXCHAN];		/* Minimum	[ADC units]	*/	
	
	Int_t		fHITEntries[MAXCHAN];	
	Float_t		fHITMean[MAXCHAN];	
	Float_t		fHITRms[MAXCHAN];	
	Float_t		fHITMax[MAXCHAN];
	Float_t		fHITMin[MAXCHAN];		
	
	Float_t		fRatioHITADC[MAXCHAN];
	
	Float_t		fHitMultMean;			/* parameters of event multeplicity distribution*/
	Float_t		fHitMultMeanErr;
	Float_t		fHitMultRMS;	
	Float_t		fHitMultRMSErr;

	
	/* Set/Reset*/
	int		fNRun;						/* total number of runs	*/	
	int		ffirstRun;					/* first run			*/
	char	fcurrRun[MAXRUN][NAMESIZE];	/* filenames			*/	
	int		fdly[MAXRUN];				/* HW parameter	(opt)	*/	
	
	
	TFile*	ffile;						
	TTree*	ftree;	
	
	char	foutfile[NAMESIZE];			/* root filename		*/
	
	
public:
	
	~TMRCScanner();	
	
	/*
	 * The constructor with blank argument is used for retrieving a already processed scan
	 * use this when you want to retrieve an old scan or compare many scans
	 */

	TMRCScanner();	
	
	
	/* Multi Run analysis (	 Data Extraction from a single Scan)
	 * Iterate single run analysis reading decoding nbulk from raw data and extracting pixel parameters from adc spectra
	 * Useful to study the calibration.
	 * by default nbulk is zero and only the global rate is extracted with no individual chanl information (faster)
	 */
	TMRCScanner(const char * prefix, int firstrun, int lastrun, int nbulk=0);
	
	
	void Recall(const char * oldrootfile);	/* Retrieving already processed scan*/

	
	void SingleRunAnalysis(int idx_run=0, int nbulk=10);
	
	/***************/
	/* Single Scan */	
	/***************/
	
	
	/* void	Plot(const char * what= "Rate:DAC");	 */
	void	Plot(const char * what= "Rate:DAC",const char * cut= "");	
	
	bool	PlotRatevsDAC_Channel( int channel=2048,int xmin=0,int xmax=1024,int ymax = 1500);
	bool	PlotRatevsDAC_GEO(int geo=32,int xmin=0,int xmax=1024,int ymax = 1500);
	bool	PlotRatevsDAC(int xmin=0,int xmax=1024,int ymax = 1500);
	
	
	
	// Plot single channel
	//zz->Plot("HITEntries[2048]/Duration:DAC")
	
	// manca il controllo degli assi  (range e titoli)
	
	// fare versione TH1
	// fare versione TH2 per avere poi Tgraph
	
	
	
	void	PlotMultiplicity(int opt=0); // 0 = Mean; 1 = RMS

	
	
	
	/* Funziona ma mancano linee e regolazione assi*/
	void	Scurve(int geo = 32);	/* # Hits vs Threshold*/
	

	/* Salva i plot di singolo canale, puo' essere utile -> meglio un bool save*/
	// cambiargli nome, ha funzione di iteratore
	void	SuperImpose64ch(const char * what = "RatioHITADC",const char * versus = "DAC",int geoaddr=32);
	
	/* Serve a poco*/
	void	HITSummary(int channel = 2101);
	//void	ADCSummary();
	
	/* Metodo specializzato di Plot ma grafica piu carina*/
	void	Shape(int channel);	
	
	/* vecchi metodi*/
	void	TrigEff();	
	void	Derive();/*Calculate the discrete derivative of the variable "Rate" adding a branch called deltarate*/
	
	
	/**************/
	/* Multi Scan */	
	/**************/
	void	Cmp(const char * rootfile1="",
				const char * rootfile2="",
				const char * rootfile3="",
				const char * rootfile4="",
				const char * rootfile5="",
		    //	const char * rootfile6="",
			//	const char * rootfile7="",
			//	const char * rootfile8="",
			//	const char * rootfile9="",
				const char * what="");
	
	void CmpRate(const char * rootfile1=NULL,
				 const char * rootfile2=NULL,
				 const char * rootfile3=NULL,
				 const char * rootfile4=NULL,
				 const char * rootfile5=NULL,
				 const char * rootfile6=NULL,
				 const char * rootfile7=NULL,
				 const char * rootfile8=NULL,
				 const char * rootfile9=NULL);

private:
	
	
	
	void	CreateTreeBranches();
	void	CreateTreeBranchesSinglePixel();
	
	void	ResetRunSequence();
	void	ResetSingleRunVariables(); 	

	int		Scan(int nbulk=0);	
	bool	FixRunNames(const char * prefix,int firstrun,int lastrun);

};
#endif

/*
 Scan Esterni
 
 TEMPO - e.g. Un run ogni ora per studiare stabilita'
 RITARDO ESTERNO - raw timing
 
 Scan Interni
 
 hold1_dly - fine timing of  
 dac - 
 hit_delay -
 gain - 
 
 
 ESEMPI
 
 Salvare una sequenza di grafici .eps
 Registrare valori che una grandezza assume per ciascuno dei run della sequenza.
 
 Le grandezze utilizzate sono sostanzialmente quelle disponibili a TAnalysis
 Utilizzo pensato da riga di comando root
 
 Dall'esterno si seleziona la serie di run, i canali ed il numero di eventi da analizzare. 
 
 - first run, 
 - last run, 
 - prefisso,
 - numero di eventi, 
 - first channel, 
 - number of channel
 
 
 */ 
