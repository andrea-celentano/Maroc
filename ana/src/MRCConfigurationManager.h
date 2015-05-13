/*
 *  MRCConfigurationManager.h
 *  
 *
 *	Reader of configuration data
 *	use libconfig e GIconfig
 *
 *
 *
 *  Created by Matteo Turisini on 21/06/12.
 *  Copyright 2012 Universita e INFN, Roma I (IT). All rights reserved.
 *
 *
 */

#pragma once

//#include "../daq/GI_Config.h"	// if coupled with daq
#include "./GI_Config.h"		// if stand alone

#include <TCanvas.h>
#include <TH1I.h>
#include <TROOT.h>
#include "General.h"

using namespace std;
using namespace libconfig;

#define CR_PIXX 1280	// Monitor size in Pixel
#define CR_PIXY 900		// Monitor size in Pixel
#define CR_SIZEOFCANV 480

#define MAX_NCARDS 64

class MRCConfigurationManager
{

private:
	int ffenum;
	char * ffilename;
	int idxList[64];
	GI_Config * fcfg;	
	int fret;
public:
	MRCConfigurationManager(char *file,bool analysis);
	~MRCConfigurationManager();
	
	// GENERAL
	void Recap();
	int ParseInLine(int argc, char *argv[]);
	int InsertInLine();
	
	

	int GetIDXfrom(int geo=32);
	
	void AddInfoRate(int evt,float duration);
	void AddFeNum(int fenum);
	void AddInfoTime(int year,int month,int day,int hour,int minute, int second);
	void AddProfiling(int npoll,int ndataready,int ntail);
	void Export(char * file);
	unsigned int GetDAC1(int fe_idx);
	int	GetUseGainMap (int fe_idx);	
	unsigned char	GetGainDefault();	
	void Reset_TCanvas(const char * tag);
	void Reset_TH1I(const char * tag);	
	void ShowPreampGain();	
	
	void SetXYAxisTitle(TH1I * histo, const char * xTitle, const char * yTitle);
	void ShowAnalogDelay();
	void ShowDigitalDelay();
	void ShowThreshold();	
	
	std::string	GetFilePrefix();
	
	// CONTROL BOARD FPGA 
	int GetDID();
	int GetMultievent();	
	
	bool isTrigSelf();
	bool isTrigExt();
	
	// PROFILING
	float  GetRate(); 
	
	// RUN CONFIGURATION
	int	GetEventPreset();	
	int GetTimePreset();	
	unsigned char GetFeNum();
	unsigned char GetGeo(int idx=0);
	unsigned char GetHoldDelay(int idx=0);
	unsigned char GetHitDelay(int idx=0);
	unsigned int  GetDAC0(int idx=0);
	
	// RUN STATISTICS
	int	GetEventNumber();	
	float GetTimeReal();
	float GetRateReal();
	float GetPollingEfficiency();
	
	// DATE AND HOUR
	std::string GetDateString();
	const char * getPlace();	
	unsigned int getYear();	
	unsigned int getMonth();	
	const char * getMonthString();	
	unsigned int getDay();
	unsigned int getHour();
	unsigned int getMinute();
	
	// MAROC3	
	unsigned char GetGain(int idx=32, int channel=0); 
	
	int GetAutoTrigger(int fe_idx);
	int GetEnableZeroSuppression(int fe_idx);
	int GetEnableAutoTrigger(int fe_idx);
	int GetEnableExtTrigger(int fe_idx);
	int GetTestEnableEvenChannels(int fe_idx);
	int GetTestEnableOddChannels(int fe_idx);
	int GetInitialChannelOut(int fe_idx);	
	int GetOneSixtyfour(int fe_idx);	
	int GetLed0(int fe_idx);	
	int GetLed1(int fe_idx);
	int GetDaqMode(int fe_idx);
	int GetTrigMode(int fe_idx);
	int GetStartDaqDelay(int fe_idx);
	int GetFastSettlingTime(int fe_idx);
	int GetSlowSettlingTime(int fe_idx);	
	int GetHitEn0(int fe_idx);	
	int GetHitEn1(int fe_idx);	
	int GetHitEn2(int fe_idx);			
	int GetSmallDac(int fe_idx);
	int GetCmdFsbFsu(int fe_idx);
	int GetEnableTristate(int fe_idx);
	int GetPolarDiscri(int fe_idx);
	int GetD1D2(int fe_idx);
	int GetSwFSU_100k(int fe_idx);
	int GetSwFSU_50k(int fe_idx);
	int GetSwFSU_25k(int fe_idx);
	int GetSwFSU_40f(int fe_idx);
	int GetSwFSU_20f(int fe_idx);
	int GetSwFSB1_50k(int fe_idx);
	int GetSwFSB1_100k(int fe_idx);
	int GetSwFSB1_100f(int fe_idx);
	int GetSwFSB1_50f(int fe_idx);
	int GetSwFSB2_50k(int fe_idx);
	int GetSwFSB2_100k(int fe_idx);
	int GetSwFSB2_100f(int fe_idx);
	int GetSwFSB2_50f(int fe_idx);
	int GetValidDC_FS(int fe_idx);
	int GetValidDC_FSB2(int fe_idx);
	int GetSwSS_1200f(int fe_idx);
	int GetSwSS_600f(int fe_idx);
	int GetSwSS_300f(int fe_idx);
	int GetEnableSS(int fe_idx);
	int GetSwbBuf_2p(int fe_idx);
	int GetSwbBuf_1p(int fe_idx);
	int GetSwbBuf_250f(int fe_idx);
	int GetSwbBuf_500f(int fe_idx);					
	int GetCmdFSB(int fe_idx);	
	int GetCmdSS(int fe_idx);
	int GetCmdFSU(int fe_idx);
	int GetEnbOutADC(int fe_idx);
	int GetInvStartGray(int fe_idx);
	int GetRamp_8bit(int fe_idx);
	int GetRamp_10bit(int fe_idx);
	int GetH1H2Choice(int fe_idx);
	int GetEnADC(int fe_idx);
	int GetInvDiscriADC(int fe_idx);
	int GetEnOtaBg(int fe_idx);
	int GetEnDAC(int fe_idx);
	int GetCkMux(int fe_idx);
	
private:
	bool CheckIDX(int idx);
	void ReadLibconfig(char * filehexname); // 

};
