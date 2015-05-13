#ifndef TSIPM_H
#define TSIPM_H

#include <stdio.h>  
#include <math.h> 
#include <string>
class TSiPM
{
private:
	int fN;
	int fNpixel;
	int fhalf;

	float fA;
	float fB;
	float fC;
	float fD;
	float fWidth;
	float fE;
	float fF;
	
	float * fXpixel;
	float * fYpixel;
	float * fXanode;
	float * fYanode;
	float * fXcathode;
	float * fYcathode;
	float * XVertex;
	float * YVertex;
	
public:
	TSiPM(int pixelperside);
	~TSiPM();
	void Print();
	void Translate(float X0, float Y0);
	float GetXanode(int i);
	float GetYanode(int i);
	float GetXcathode(int i);
	float GetYcathode(int i);
	
	float GetX(int i);
	float GetY(int i);
	
	float GetXVertex(int i);
	float GetYVertex(int i);
	
private:
	int GetRow(int pixel);
	std::string GetColumn(int pixel);
	void MirrorX();
};
#endif