/*
 *  TCalib.h
 *
 *  This class handle operations with system calibration parameters
 *  It manages I/O operation about Pedestal and Gain of 4096 channels
 *	So we have deal with 2 files : a Pedestal one (with cnoise) ang a Gain one
 *  The class possesses different methods for represents its data-member:
 *  a Print one 
 *  a Plot (also scattered Plot) one
 *  and a Distribution one
 *  All of them are based in data member reading.
 *  They are useful in performing:
 *  1) Pedestal substraction,
 *  2) Sparse read out settings and
 *  3) Energy Calibration
 *
 *  Created by Matteo Turisini on 22/11/10.
 *  Rev1 by Matteo Turisini on 8/3/11.
 *  Rev2 by Francesco Cusanno on May-June 2013
 *
 */

#ifndef TCALIB_H
#define TCALIB_H

#include "TH1I.h"
#include "TCanvas.h"
#include "Riostream.h"  
#include "TDirectory.h"
#include "TGraph.h"
#include "TBox.h"

#include "General.h"


#define MAXCHANNEL 4096

class TCalib   // handle/store calibration data such as pedestals, noise ... // PNG = pedestal, noise and gain
{
private:
	Float_t		fPedestal[MAXCHANNEL];	 
	Float_t		fNoise   [MAXCHANNEL];	 
	Float_t		fGain    [MAXCHANNEL]; 


	const char * ffilename;

public:
	TCalib();
	~TCalib();


	void		Print(int code=0); //1 Pedestal, 2 Noise, 3 Gain
	void		Export (const char *filename,int code);// write pedestal-noise(code =0) or gain(code =1) into a file
	void		Import (const char *filename);
	
	void		Reset();
	
	void		SetPedestal	(int channel, Float_t value) { fPedestal[channel]= value;}
	void		SetNoise	(int channel, Float_t value) { fNoise   [channel]= value;}
	void		SetGain		(int channel, Float_t value) { fGain    [channel]= value;}
	
	Float_t		GetPedestal (int channel) { return fPedestal[channel];}
	Float_t		GetNoise    (int channel) { return    fNoise[channel];}
	int			GetGain     (int channel) { return     (int)fGain[channel];}

	void		Plot        (int code=0,const char * opt="ALP",Float_t chstart=0.0, Float_t chstop=4095.0); // 0 P, 1 N, 2 G, 3 NP, 4 GP
	void		Distribution(int code=0,float binres = 4.0); // 0 P, 1 N ,2 G.
	
	void		CardNoise	(char geoaddress=32);
	void		CardPedestal(char geoaddress=32);
	void		CardGain	(char geoaddress=32);
};

#endif
