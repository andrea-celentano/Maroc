/*
 *  Th9500.h
 *  
 *
 *  Created by Matteo Turisini on 06/09/11.
 *  Copyright 2011 Universita e INFN, Roma I,ISS (IT). All rights reserved.
 *
 */


#ifndef TH9500_H
#define TH9500_H

#ifndef PIGRECO
#define PIGRECO 3.141596 
#endif

#include <stdio.h>  
#include <math.h> // for rotation

class Th9500 { // This class handles data about MAPMT geometry and its positioning in the space.
	
private:
	int fNPixel;		// Number of Pixel or anodes of the MAPMT
	
	float fPixelSize;  // in mm // Area covered by each pixel is a square of side PixelSize
	float fFrameSize;  // in mm // Each PM has a border named FrameSize
	
	
	float fWidth;		// in mm
	
	short fAnode[256];	// anode ID 
	float fXc[256];		// X centers 
	float fYc[256];		// Y centers 
	float fXVertex[4];	// PMT dimensional outlines along X
	float fYVertex[4];	// PMT dimensional outlines along Y
	
	char fGeo[4];		//4 geographic address on backplane for each h9500
	char fFe[4];		//4 front-end ID

	
	
private:
	// Generation of coordinates X and Y
	float	Anode2LocalY(short anode);
	float	Anode2LocalX(short anode);

public:
	Th9500();
	//~Th9500();
	
	void	Print();
	
	void	Translate(float X=0.0, float Y=0.0);
	void	Rotate(float angle = 0.0); // around the origin counterclockwise, angle in degrees

	float GetX(short anode);
	float GetY(short anode);
	
	float GetWitdh(){return fWidth;};
	float GetXVertex(int i){return fXVertex[i];}; // i=0,1,2,3
	float GetYVertex(int i){return fYVertex[i];}; // i=0,1,2,3
	
	void	SetFe(char fe1 ,char fe2, char fe3, char fe4);
	void	SetGeo(char geo1,char geo2,char geo3,char geo4);
	char	GetFe(int idx){return fFe[idx];};	// idx= 0,1,2,3.      4 Card for each h9500
	char	GetGeo(int idx){return fGeo[idx];};  // idx= 0,1,2,3.      4 Card for each h9500

};


#endif