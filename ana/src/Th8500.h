/*
 * This class handle Geometrical Data of a H8500 MAPMT.
 * The costructor generates local coordinates of 64 anodes
 * and of the 4 vertex of the object(dimensionale outline).
 * It is possible to Rotate and Traslate the PMT
 * as a rigid body obtaining anode's coordinates in 
 * the laboratory reference system.
 * 
 *
 */

#ifndef TH8500_H
#define TH8500_H


#define PIGRECO 3.141596 
#include <stdio.h>  
#include <math.h> // for rotation


class Th8500
{
	// This class handles data about MAPMT geometry and its positioning in the space.
private:
	int	NPixel;		// Number of pixels (or anodes) of the MAPMT
	
	float PixelSize;  // in mm // Area covered by each pixel is a square of side PixelSize
	float FrameSize;  // in mm // Each PM has a border named FrameSize

	
	float Width;		// in mm

	int	Anode[64];	// anode ID 
	float Xc[64];		// X centers 
	float Yc[64];		// Y centers 
	float XVertex[4];// PMT dimensional outlines along X
	float YVertex[4];// PMT dimensional outlines along Y
	
	char geo; //geographic address on electronics
	char fe; // front-end ID
	
private:
	// Generation of coordinates X and Y
	float Anode2LocalY(int anode);
	float Anode2LocalX(int anode);
	

public:
	Th8500();
	//~Th8500();
	void Print();
	void Translate(float X=0.0, float Y=0.0);
	void Rotate(float angle = 0.0); // around the origin counterclockwise, angle in degrees
	
	void SetFe(char ID){fe=ID;};
	void SetGeo(char ID){geo=ID;};
	
	char GetGeo(){return geo;}; 
	char GetFe(){return fe;};

	float GetX(int anode);
	float GetY(int anode);
	
	float GetWitdh(){return Width;};
	float GetXVertex(int i){return XVertex[i];}; // i=0,1,2,3
	float GetYVertex(int i){return YVertex[i];}; // i=0,1,2,3
};
#endif