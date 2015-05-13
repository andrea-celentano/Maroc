/*
 *	TCNSAnalysis.cpp
 *
 *  Analysis of CNS filter
 *
 * 2013 June 12
 * 
 * Matteo Turisini
 */ 


// Fill e Plot 

// creo in Read se GroupSIZE !=0
// Passo a block 
// che passa al suo metodo oppure 
// inizializzo un datamemeber di Block 
// cosi ci accede facile qualunque metodo di T Block


#ifndef CNSANALYSIS_H
#define CNSANALYSIS_H


#include "TH1F.h"
#include "TCanvas.h"


#define	NSUBSETMAX	4

#define NOISEBIN	100
#define NOISEMIN	599.5
#define NOISEMAX	700.5

#define	CANVPOSX	600
#define	CANVPOSY	0
#define	CANVSIZE	400


using namespace std;


class CNSAnalysis
{
private: 
	int fGroupSize; 	// chGroupSize = channel group size for common noise evaluation; common noise subtraction off if chGroupSize=0	
	int fNGroup;	// number of subset
	
	TH1F * fhNoise[NSUBSETMAX];
	
	
public:
	CNSAnalysis(int groupsize=64);
	~CNSAnalysis();
	
	int		GetNGroup(){return fNGroup;};
	int		GetGroupSize(){return fGroupSize;};	
	bool	Add(float value,int subset);
	void	Plot();

	void	Print(int subset);
	
private:
	bool	CheckSubset(int subset);

};
#endif

