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

#include "TBlock.h"
#include "TEvent.h"
#include "TFileRaw.h"
#include "TCalib.h"
#include "General.h"
#include "CNSAnalysis.h"
#include "TF1.h"
#include "TLegend.h"


#include "MRCConfigurationManager.h"

#include "TConnectH8500.h"


#include "TStopwatch.h" 

#include "TProfile.h" 
#include <TGraphErrors.h>
#include <TH2F.h>
#include <TCut.h>
#include <TPad.h>
#include <TPaveText.h>

#define FE_DEFAULT 32   //
#define CH_DEFAULT 2085	//
#define	XMIN 500		// PlotChannel Range
#define	XMAX 1900		// PlotChannel Range
#define	YMAX 3000		// PlotChannel Range

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
	spectrum	fparam_adc[4096];
	spectrum	fparam_hit[4096];

	
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
	/********************/

	Double_t PlotADC(int channel=CH_DEFAULT,int xmin=1200,int xmax=2000,int=10000,int rebin = 1); // new, 2014 Jan 21th
	Double_t PlotHIT(int channel=CH_DEFAULT,int xmin=1200,int xmax=2000,int=10000,int rebin = 1); // new, 2014 Jan 21th


	Double_t PlotADCFit(int channel=CH_DEFAULT,int xmin=1200,int xmax=2000,int=10000,int rebin = 1); // new, 2014 Jan 21th
	
	bool PlotChannel(int channel=CH_DEFAULT,bool save=false, int xmin=0,int xmax=0,int ymax= 10000);
	// Single channel analog spectrum (Charge) is plotted
	// in red triggerd events (hit)
	// Parameters of both histogram are written in the "spectrum" struct associated to that channel 
	// save=true save .eps in ./Spectra and delete it from screen. save=false(default) the plot persist on the screen  
	// Arguments 3,4,5 sets the axis range. default (xmax==0) range is data dependent (.98min and 1.02max on X axis) 
	// A->PlotChannel(2085)						// fast look at a pixel
	// A->PlotChannel(2085,true)				// save plot (and remove it from screen)
	// A->PlotChannel(2085,false,100,1000,3000)	// save plot with user defined axis
	// last example are useful for comparisons.


	bool PlotADC_OLD(int channel=CH_DEFAULT);	// replaced by PlotADC on 2014 Jan 21th	

	
	/***************************/
	/* Single Channel Analysis */
	/***************************/
	
	// Salva tutti gli spettri e plotta le efficienze HIT/ADC

	/**************/
	/* FE Spectra */
	/*************/
	
	bool CardSpectrum(int card = FE_DEFAULT,double ADClo=0.5,double ADChi=4095.5);	
	// CardHITSpectum(int card = FE_DEFAULT)
	// CardSpectrumCompare(int card = FE_DEFAULT)
	
	

	
	
	/*****************************************/
	/* Extract data from parsed data (.root) */
	/*****************************************/
	
	int		GetHITEntries(int channel =CH_DEFAULT);
	int		GetADCEntries(int channel =CH_DEFAULT);
	
	double	GetADCMean(int channel =CH_DEFAULT);
	double	GetADCHigh(int channel =CH_DEFAULT);
	double	GetADCLow(int channel =CH_DEFAULT);
	double	GetADCRms(int channel =CH_DEFAULT);
	
	double	GetHITMean(int channel =CH_DEFAULT);
	double	GetHITHigh(int channel =CH_DEFAULT);
	double	GetHITLow(int channel =CH_DEFAULT);
	double	GetHITRms(int channel =CH_DEFAULT);
	

	bool		ResetSpectrParam();
	
	/*****************************************/
	/* Extract data from configuration (.txt)*/
	/*****************************************/
	
	unsigned char	Get_Gain(int channel); 
	float	Get_ACQRate(); // 2013 May 10th used in trigger efficiency curves 
	int		Get_DAC0(); // 2013 May 12th used in trigger efficiency curves
	int		Get_HOLD1_Delay(); // 2013 June 25th for analog timing calibration
	
	// Get Hold1 delay
	
	/***********************************/
	/* Event Display (Electronic view) */
	/***********************************/
	void DisplayFE(const char * rootfilename,char geo= FE_DEFAULT,int nevent =100, double min=0,double max=4096,double sleep=0.001);
	void DisplayBP(const char * rootfilename,char bp_code= 4,int nevent =100, double min=0,double max=4096,double sleep=0.001);
	
	void Print();
	
	void DebugON(){fdebugmode=true;};
	void DebugOFF(){fdebugmode=false;};

	bool ReadConfiguration(); 
	
	bool PlotDataset (char code=0);// 0 All dataset  // 1 All !=0 ; //2 Hit  //3 Sum  //4 Profile ADC //5 Hit Efficiency // 6 Single Channel // 7 Accessing the Histogram in Batch Mode
	bool OldCardSpectrum(int card = FE_DEFAULT,char code=0, char SiPM=0); 
	
	bool LoadTree(const char* pathfileroot);
	
	void SaveGainMap(char* filename);
	void LoadGainMap(char * filename);

	void DebugInstructions();
	void SetDebugOptions(bool dbgBlock = false,bool dbgMaroc = false,unsigned int PLMaroc  = 0 ,unsigned int PTMaroc  = 1234, bool dbgEvent = false,unsigned int PLEvent  = 0,bool dbgParser= false,unsigned int PLParser = 0);// e.g. SetDebugOptions(1) DEBUG Block; SetDebugOptions(0,1,1,235) Debug of TMaroc	with Level   1 and Threshold    235; SetDebugOptions(0,0,0,0,1,8) Debug of TEvent with Level   8; SetDebugOptions(0,0,0,0,0,0,1,3)Debug of TFileRaw with Level 3
	
	
	

	
	

	
private:
	
	void Reset();
	
	int ReadDGN(unsigned int block_to_read = 10000);
	
	bool GetTree();
	bool GetEvent(int treeentry);
	
	MRCConfigurationManager * GetConfigurationPointer(){return fCR;};
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


