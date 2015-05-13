/*
 *  Tr8900.h
 *  
 *
 *  Created by Matteo Turisini on 08/07/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef TR8900_H
#define TR8900_H

#define PIGRECO 3.141596 
#include <stdio.h>  
#include <math.h> // for rotation

class Tr8900
{
	// This class handles data about MAPMT geometry and its positioning in the space.
private:
	int	NPixel;		// Numero di anodi del PMT
	
	float PixelSize;  // Dimenzione del pixel (quadrato) in mm
	float Septal;     // Distanza di separazione tra i pixel in mm
	float Width;		// Lato del PMT (quadrato) in mm
	
	int	Anode[16];	// anode ID 
	float Xc[16];		// X centers 
	float Yc[16];		// Y centers 
	float XVertex[4];// PMT dimensional outlines along X
	float YVertex[4];// PMT dimensional outlines along Y
	
	char geo; //geographic address on electronics
	char fe; // front-end ID
	
	float Anode2LocalY(int anode);
	float Anode2LocalX(int anode);
	
public:
	Tr8900();
	//~
	void Print();
	void Translate(float X=0.0, float Y=0.0);
	void Rotate(float angle = 0.0); // angle in degrees
	//void Plot();
//	void Paint();
	
	void SetFe(char ID){fe=ID;};
	void SetGeo(char ID){geo=ID;};
	
	char GetGeo(){return geo;}; 
	char GetFe(){return fe;};
	
	float GetX(int anode); // returns coordinate of anode i 
	float GetY(int anode);
	
	float GetWitdh(){return Width;};
	float GetXVertex(int i){return XVertex[i];}; // i=0,1,2,3
	float GetYVertex(int i){return YVertex[i];}; // i=0,1,2,3
};
#endif

