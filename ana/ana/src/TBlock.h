/* 
 * file: TBlock.h
 *
 * To parse the content of one data package (USB bulk), identifying events, offline filtering. diagnostic on raw file structure. 
 * Case 1 (faster): dataparsing  + Common Noise Suppression filter optional
 * Case 2 (slower): data parsing + diagnostic on file structure
 *
 * Creation Date: 2011, March 8th, INFN-ISS
 * Last revision: 2013, May 6th, ISS
 *
 * Author: Matteo Turisini & Evaristo Cisbani
 *
 * for more information see below
 */

#ifndef TBLOCK_H
#define TBLOCK_H
#include "TMaroc.h"
#include "TVME.h"
#include "TEvent.h"
#include "TFileRaw.h"
#include "TCalib.h"
#include "CNSAnalysis.h"
#include "General.h"

#define IDMASK				0xC0	// select bit [16,15] of a Dword
#define MRC_HEADER_MASK		0x80	// 10 => Header
#define MRC_DATA_MASK		0x00	// 00 => Data
#define MRC_TRAILER_MASK	0xC0	// 11 => Trailer
#define MRC_HIT_BIT			0x04

#define MAXFE				1000	// in a USB package (Bulk)
#define LAST_OF_BULK		666		// tag to identify the last event in a bulk (that in ME could be partially acquired in sparse readout modes)
#define OFFSET				730

class TBlock
{
private:
	TVME*	fvme;
	TMaroc* ffront_end[MAXFE];	// fe data buffer
	int		fmarocN;			// array index (fe reading in this block) incremented after each trailer
	int 	feventN;			// event counter (in this block
	void	CommonNoiseFilter(TEvent * Event,CNSAnalysis * CNSAnalizer);
	void	CommonNoiseFilterExcludingMax(TEvent * Event, int subsetsize);	
	//	void	CommonNoiseFilterExcludingMax(TEvent * Event, int subsetsize,CNSAnalysis * CNSAnalizer); // TO BE IMPLEMENTED
	public:
			~TBlock	();
			TBlock(int tour,unsigned char * daq_data_vector,TEvent *,TFileRaw *); // Diagnostic
			TBlock(unsigned char * daq_data_vector,unsigned short nword, TEvent * Event,int &idx_first,CNSAnalysis * CNSAnalizer); // Default
	
	TVME*	GetVME		()					{return fvme	;}	
	int		GetNEvents	(void)				{return feventN	;} // passed FileStat for diagnostics
	int		GetNmaroc	(void)				{return fmarocN	;}	
	int		GetDataByIdx(int idx, int ch)	{return (ffront_end[idx]->GetData(ch) );}
	int		GetMaroc	(int i)				{return ffront_end[i]->GetGeoaddress();}
};
#endif


/*
 * A File Raw is a sequence of packages broken up by a Record word (called NWord)
 * A package could be made up of one or more events (Single or Multievent),
 * An event is a  triggered readout operated by the Control Board.
 * The number of card's channel involved and the very number of card per event 
 * could change in relation with readout modality (ALL,HIT,OVER THR, NONE)

 *  which is used to read data packages contained in a File Raw.
 *  A File Raw is a sequence of packages broken up by a Record word (called NWord)
 *  A package could be made up of one or more events (Single or Multievent),
 *  An event is a  triggerd readout operated by the Control Board.
 *  The number of card's channel involved
 *  and the very number of card per event could change
 *  in relation with transfer modality (all, none or subdued to same threshold crossing)
 
 * To create an object of Block Type we pass to it's constructor
 * a buffer containing data to be parsed and some object pointers so that they
 * could process informations on data structure and record events.
 * Since it creates object of Maroc types we have also to pass to it some infomation needed by TMaroc constructor.
 * Note that it is in charge of recognize events.
 * Also note that a TMaroc object is created every time an header word is found in daq_data_vector (data buffer)
 * Data buffer is read by DWord
 * Header, data and trailer ar recognized by proper masking  daq_data_vector[i] & 0xC0
 
 */