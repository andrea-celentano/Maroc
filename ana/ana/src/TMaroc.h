//  This file contains the class Maroc Event
//  It manages single Front-end card informations
//
//  Author: Matteo Turisini
//	November 2010
//  rev1 August 2011
#ifndef TMAROC_H
#define TMAROC_H

#include<stdio.h>
#include<string.h> //memset
#include "General.h"


class TMaroc{ 
private:	

	
	unsigned char	fhitmap[8];	// trigger bit( 1 bit x 64 canali) // hits
	unsigned short	fdata[FECHNUM];	// adc  word  (12 bit x 64 canali) // datawords
	unsigned char	fseq;		// Sequential Tag			EVENT[7..0] //  assegnata dal controller al pacchetto dati e incrementata ad ogni nuovo trigger
	unsigned char	ftimer;		// Timer Tag				TIMER[7..0] //  relativa ai dati della scheda		
	unsigned short	frecord;	// Record number  N_PAROLE_TOTALE[7..0] //  Numero Dword dei dati appena trasmessi(check).
	unsigned char	fgeo;		// address of the card				  GEOADDR[5..0] //  xx00 0000  geoaddress -
	signed	 char	fmode;		// Acquisitio Mode	M[1..0] //	xxxx xx00  all  -
															    //	xxxx xx01  hit  -
																//	xxxx xx10  thr  -
																//	xxxx xx11  none -
public:
	TMaroc();
	
	void		Print(); 
	int		Simulate(TMaroc * fe);

	void	SetHit			(int channel); // assign digital datum to a maroc 'channel' [0..63] 
	void	SetData			(int channel, unsigned short adc_datum); // assign an analog datum to maroc 'channel' [0..63] 
	void	SetHeaderSeq    (unsigned char  header_seq);// assign a sequential number representing event_ID to this card
	void	SetHeaderTime   (unsigned char  header_time);// assign a time stamps representing  (local) time to this card
	void	SetGeoaddress	(unsigned char  geoaddress); // assign an address to this card
	void	SetTrailerNwords(unsigned short trailer_nwords); // assing the number of Words to this card reading
	void	SetACQmode		(signed char nn); // assign a value to acquisition mode for this card ;attention! info written in the MSnibble (see raw data format ...)
	
	
	int				GetHit	(int channel);//Legge lo stato del bit di trigger di uno specifico canale	
	unsigned short	GetData	(int channel)	{return fdata[channel]	;} 
	unsigned char	GetHeaderSeq(void)		{return fseq		;}
	unsigned char	GetHeaderTime(void)		{return ftimer	;}
	unsigned char	GetGeoaddress(void)		{return fgeo			;}
	unsigned short	GetTrailerNwords(void)	{return frecord	;}
	signed	 char	GetACQmode(void)		{return fmode			;} 	
};
#endif