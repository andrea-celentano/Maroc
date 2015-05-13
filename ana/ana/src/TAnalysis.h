/*
 *	TAnalysis.cpp
 *
 *	This is the object manages:
 *
 *		a) File Parsing with statistics
 *		b) Event Collection
 *		c) Calibration Parameters Extraction 
 *      d) Channels Tuning
 *
 *	Author: Matteo Turisini
 * Date: 2013, January 26th
 *
 * May 2013: extended version by Francesco Cusanno
 */ 

#ifndef TANALYSIS_H
#define TANALYSIS_H

#include "General.h"
#include "TBlock.h"
#include "TEvent.h"
#include "TFileRaw.h"
#include "TCalib.h"
#include "CNSAnalysis.h"
#include "MRCConfigurationManager.h"
#include "TConnectH8500.h"
#include "TStopwatch.h" 

#include <TF1.h>
#include <TLegend.h>
#include <TProfile.h> 
#include <TGraphErrors.h>
#include <TH2F.h>
#include <TCut.h>
#include <TPad.h>
#include <TPaveText.h>
#include <TPaveStats.h>

#include <unistd.h> // sleep

#define YMAX	10000
#define REBIN	1


using namespace std;

struct spectrum {
	Int_t	entries;
	Float_t mean;
	Float_t rms;
	Float_t max;
	Float_t min;
	bool	isEmpty; /* has been written 0, has been resetted 1*/
};


class TAnalysis
{
private:
	spectrum	fparam_adc[MAX_PIXEL];
	spectrum	fparam_hit[MAX_PIXEL];
	spectrum	fparam_singlehit[MAX_PIXEL];

	float		fx[MAX_PIXEL]; // Spatial coordinates
	float		fy[MAX_PIXEL];
	
	char		fnomebin[MAX_NAME_LENGH];	
	char		fRunName[MAX_NAME_LENGH];
	char		fnomeroot[MAX_NAME_LENGH];
	char		fnomeconfig[MAX_NAME_LENGH];	

	bool		fdebugmode;
	
	int			fchfirst;		//
	int			fchlast;		//
	
	bool		fdbgEvent ;		// debug mode for TEvent
	bool		fdbgBlock ;		// debug mode for TBlock
	bool		fdbgParser;		// debug mode for TFileRaw
	bool		fdbgMaroc ;		// debug mode for TMaroc 
	
	unsigned int fPLEvent;	// Printing Level of TEvent
	unsigned int fPLParser;	// Printing Level of TFileRaw
	unsigned int fPLMaroc;	// Printing Level of TMaroc
	unsigned int fPTMaroc;  // Printing Threshold of TMaroc
	
	MRCConfigurationManager * fCR;
	TCalib		* fcalibration;	 
	TEvent		* fevent;	
	TFileRaw    * fparser;	
	TFile		* frootfile; 
	TTree		* ftree;
	TConnectH8500 * fconnectH8500; // mapping between H8500 anodes and frontend channels
	CNSAnalysis	* fCNSAnalizer;
	
	Int_t	fymax;
	Int_t	frebin;
	
	bool	ffitON;
	bool	fgraficON;
	
	Int_t fch1,fADCmin1,fADCmax1,fhit1;
	Int_t fch2,fADCmin2,fADCmax2,fhit2;
	Int_t fm; // cut on multiplicity
	
public:
	
	TAnalysis(const char * fileraw = "", bool havecfg=true);
	// Set I/O filenames. Set pointers to TFile and TTree
	
	~TAnalysis();
	// free memory
	
	int Read(unsigned int block_to_read=10,int chGroupSize=0);
	// Parse detector data (.bin) creating a file (.root) with  organized data (Ttree) 
	// Returns the number of events read
	
	// A Common Noise Suppression filter (CNS) is optional 
	// The filter act on each frontend and event by event 
	// subtracting to the samples the mean value i.e. the common noise
	// a more sophisticated version calculates the mean excluding the maximum.
	// other can be easly implemented modifing TBlock class.
	// chGroupSize=0 means no filtering (default)
	 
	
	bool Diagnostic();
	// Parse detector data (.bin) creating a file (.root) with  organized data (Ttree) 
	// histograms about the raw file structure in terms of block,events, fe involvement.
	// Histos are plotted and stored in the rootfile
	
	
	
	bool SystemProfile(int adcbin_mean =32, int adcbininv_rms=2,double rmslow =.5,double rmsup=15.5,double meanlow =0.,double meanup =1200.);		
	// Global Representation of ADC data
	// binning can be adjusted (num of adc channel (mean plot) and adcbininv_rms fraction of adc channel (RMS plot)
	// other parameter set the ranges of meanADC and rmsADC
	
	
	bool SystemSpectrum(const char *pedestalfile = "./pedestal.txt");
	//  Plot ADC sum with pedestal subtraction
	// ] A->SystemSpectrum("./ped/130604_pedestal.txt")

	//
	bool MakePedestal(const char *pedestalfile = "pedestal.txt");
	// Export pedestal map calculated as meanADC of analized run
	// only !=0 values are written
	// A->MakePedestal("./ped/130604_pedestal.txt")
	
	/*******************/
	/*   FE Analysis   */
	/*******************/
	
	bool Extract(); // process all channels extracting parameters
	
	bool Export(); // like extract but producing an output file
	
	
	
	
	/********************/
	/* Channel Spectrum */
	/*
	 * returns the number of selected events
	 *
	 * 	A->PlotADC(ch1)			// pure ADC spectrum
	 * 	A->PlotADC(ch1,hit[ch1]);	// Cut on binary output, 0 low, 1 high, -1 don't care
	 *  A->PlotADC(ch1,hit[ch1],adcmin[ch1],adc1max[ch1]);// Cut on charge output, 
	 *  A->PlotADC(ch1,hit[ch1],adcmin[ch1],adc1max[ch1],m,"==");// Cut on event multiplicity
	 *  A->PlotADC(ch1,hit[ch1],adcmin[ch1],adc1max[ch1],m,"==",ch2,hit[ch2],adcmin[ch2],adc1max[ch2]) // cut on ch2
	 *  A->PlotADC(ch1,hit[ch1],adcmin[ch1],adc1max[ch1],m,"==",ch2,hit[ch2],adcmin[ch2],adc1max[ch2],1) // scatter
	 *
	 * SetRebin
	 * SetYMax
	 * SetGraphicsON()
	 * SetFitON() 
	 *******/

	Double_t PlotADC(int channelA	=	CH_DEFAULT,
					 int hitA		=	-1,
					 int xminA		=	ADCMIN,
					 int xmaxA		=	ADCMAX,
					 int m			=	-1,
					 const char * rel = "==",
					 int channelB	=	CH2_DEFAULT,
					 int hitB		=	-1,
					 int xminB		=	0,
					 int xmaxB		=       4095,
					 int opt		=	0);
	
	TF1 * FitPedestal(TH1F * hADC=NULL);
	TF1 * FitSignal	 (TH1F * hADC=NULL);
	TF1 * FitSingleGaussian(TH1F * hADC=NULL);
	
	void SetYMax(Int_t ymax=YMAX); 
	void SetRebin(Int_t rebin=REBIN);
	bool SetGraphicsON(bool enable_grafix = true);
	bool SetFitON(bool enable_fit = false);
	
	void StoreFitResults(TF1 * formula =NULL,int channel=0);
	
	void SetChannel1(int channel=CH_DEFAULT, int hit=-1,int xmin=ADCMIN,int xmax=ADCMAX);
	void SetChannel2(int channel=CH2_DEFAULT,int hit=-1,int xmin=ADCMIN,int xmax=ADCMAX);
	void SetMultiplicity(int m=-1);
		
	
	Double_t PlotHit(int channel= CH_DEFAULT,char option = 0); // 0 = distribution, 1 = vs event
	
	bool PlotChannel(int channel=CH_DEFAULT,bool save=false, int xmin=ADCMIN,int xmax=ADCMAX,int ymax= YMAX,int multiplicity=-1); // multiplicity =1 select only events with multiplicity =1; default is suppressed m = -1


	
	/***************************/
	/* Single Channel Analysis */
	/***************************/
	
	// Salva tutti gli spettri e plotta le efficienze HIT/ADC

	/**************/
	/* FE Spectra */
	/*************/
	
	bool CardSpectrum(int card = GEO_DEFAULT,int adc_min = ADCMIN,int adc_max = ADCMAX);	
	
	
	/********/
	/* HIT  */
	/********/
	
	Float_t * GetMultiplicity();
	Double_t HitMultiplicity(char option = 0,int mmax =64);
	
	int HitDisplay(int multiplicity = -1,int opt=0, int first =0 ,int last =100,float sleep= 0.001,const char * mapfile = "./Maps/channelmap.txt");// opt=0 SumDisplay
	
	/***********/
	/* Imaging */
	/***********/
	void ResetCoordinates();
	bool SetCoordinates(const char * mapfile);
	float GetX(int pixel){return fx[pixel];};	
	float GetY(int pixel){return fy[pixel];};	
	
	/*****************************************/
	/* Extract data from parsed data (.root) */
	/*****************************************/
	
	int		GetNEvent();
	
	int		GetHITEntries(int channel =CH_DEFAULT);
	int		GetSingleHITEntries(int channel =CH_DEFAULT);
	int		GetADCEntries(int channel =CH_DEFAULT);
	
	double	GetADCMean(int channel =CH_DEFAULT);
	double	GetADCHigh(int channel =CH_DEFAULT);
	double	GetADCLow(int channel =CH_DEFAULT);
	double	GetADCRms(int channel =CH_DEFAULT);
	
	double	GetHITMean(int channel =CH_DEFAULT);
	double	GetHITHigh(int channel =CH_DEFAULT);
	double	GetHITLow(int channel =CH_DEFAULT);
	double	GetHITRms(int channel =CH_DEFAULT);
	
	double	GetSingleHITMean(int channel =CH_DEFAULT);
	double	GetSingleHITHigh(int channel =CH_DEFAULT);
	double	GetSingleHITLow(int channel =CH_DEFAULT);
	double	GetSingleHITRms(int channel =CH_DEFAULT);


	bool	ResetSpectrParam();
	
	/*****************************************/
	/* Extract data from configuration (.txt)*/
	/*****************************************/
	
	unsigned char	Get_Gain(int channel); 
	float			Get_ACQRate();		// 2013 May 10th used in trigger efficiency curves 
	int				Get_DAC0();			// 2013 May 12th used in trigger efficiency curves
	int				Get_HOLD1_Delay();	// 2013 June 25th for analog timing calibration
	int				GetNevents();
	unsigned char	GetGeo(int idx=0);
	unsigned char	GetFeNum();
	float			GetDuration();
	float			GetPollingEfficiency();
	
	/***********************************/
	/* Event Display (Electronic view) */
	/***********************************/
	void DisplayFE(const char * rootfilename,char geo= GEO_DEFAULT,int nevent =100, double min=0,double max=4096,double sleep=0.001);
	void DisplayBP(const char * rootfilename,char bp_code= 4,int nevent =100, double min=0,double max=4096,double sleep=0.001);
	
	void Print();
	
	void DebugON(){fdebugmode=true;};
	void DebugOFF(){fdebugmode=false;};

	bool ReadConfiguration(); 
	
	bool PlotDataset (char code=0);// 0 All dataset  // 1 All !=0 ; //2 Hit  //3 Sum  //4 Profile ADC //5 Hit Efficiency // 6 Single Channel // 7 Accessing the Histogram in Batch Mode
	
	bool LoadTree(const char* pathfileroot);
	
	void SaveGainMap(char* filename);
	void LoadGainMap(char * filename);

	void DebugInstructions();
	void SetDebugOptions(bool dbgBlock = false,bool dbgMaroc = false,unsigned int PLMaroc  = 0 ,unsigned int PTMaroc  = 1234, bool dbgEvent = false,unsigned int PLEvent  = 0,bool dbgParser= false,unsigned int PLParser = 0);// e.g. SetDebugOptions(1) DEBUG Block; SetDebugOptions(0,1,1,235) Debug of TMaroc	with Level   1 and Threshold    235; SetDebugOptions(0,0,0,0,1,8) Debug of TEvent with Level   8; SetDebugOptions(0,0,0,0,0,0,1,3)Debug of TFileRaw with Level 3

	void GetNeighbours(int pixel= 2049, char opt=0,int * size=NULL, int ** arr=NULL); // opt: 0=maroc 1= mapmt 

	int PrintStat(TH1I * h = NULL);
	
private:
	
	void Reset();
	
	int ReadDGN(unsigned int block_to_read = 10000);
	
	bool GetTree();
	bool GetEvent(int treeentry);
	
	TTree * GetTreePointer(){return ftree;};//
	
	int  fe2slot(int fe);				// from fe [0..63] to slot in a backplane
	int  abs2bp(int channel);			// from electronic channel [0...4095] to Backplane [0..3] 
	int  abs2geo(int channel);			// from electronic channel [0...4095] to Geoaddress [0..63] 
	int  abs2maroc(int channel);		// from electronic channel [0...4095] to Front-End (local) channel [0..63] 
	int  loc2abs(int geo, int maroc);	// from geoaddress[0..63] and local maroc channel[0..63] to Absolute channel[0..4095]
	
	bool IsGoodSystemChannel(int n);	// is in range [0..4095] ?
	bool IsGoodGeoAddress(int n);		// is in range [0..63] ?
	
	void DestroyEvent();
	void DestroyParser();
	void DestroyCalibration();
	void DestroyConnH8500();
	void DestroyCNSAnalizer();
	
	void PrintFinalDebug(TStopwatch ltimer);
	void check_zip(char *fname_in, char *fname_out);
	int ParseFileName(const char * pathfilenamebin); 
	
	bool OpenRootFile(const char * filename);
	bool CloseRootFile();
	
	void MakeUp(TH1F * h = NULL,int hit=-1); // ADC plot
	void MakeUpMultiplicity(TH1I * h=NULL); // Multiplicity Plot
	bool CheckBinSize(TH1F * h = NULL, Float_t val=1.);
	
	void SetCh1(Int_t echannel= CH_DEFAULT); 
	void SetCh1ADCmin(Int_t ADCmin =ADCFIRST);	
	void SetCh1ADCmax(Int_t ADCmax=ADCLAST);	
	
	void SetCh2(Int_t echannel= CH2_DEFAULT);
	void SetCh2ADCmin(Int_t ADCmin =ADCFIRST);	
	void SetCh2ADCmax(Int_t ADCmax= ADCLAST);
	
	void SetCh1HIT(Int_t hit=-1);	
	void SetCh2HIT(Int_t hit=-1);
	

	
};
#endif



/* std::string as argument EXAMPLE
 * 
 
  file .h
 int Parse(unsigned int block_to_read = 10,std::string options = "pippo");
 
 file .cpp
 int TAnalysis::Parse(unsigned int block_to_read,std::string options){
 
 printf("Sei dentro il nuovo Reader\n");
 printf("numero di bulk da leggere =  %d\n", block_to_read);
 cout << options<< endl;
 printf("La cui dimensione e' %ld caratteri\n",options.length());
 
 
 
 std::string string1 ("CNS");
 
 if (options.compare("CNS")!=0) {
 cout << options<< "is not"<<string1<<endl;
 }else {
 cout << "You want a CNS, isn't it?"<<endl;
 }
 
 
 
 return 0;
 }
 
 
 
 */


