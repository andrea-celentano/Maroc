#ifndef MRCSETUP_H
#define MRCSETUP_H
#include "GI_Config.h"
#include <string.h>

class MRCSetup
{
private:
	GI_Config * fcfg;
	char *ffilename ;
	int fret;
	
public:
	MRCSetup(char *file);
	~MRCSetup();	
	std::string	GetVersion();
	int			GetNfe();
	std::string	GetMapmtID(int idx);
	int			GetHV(int idx);
	int			GetFeID(int idx);
	int			GetGeo(int idx);
	float		GetX(int idx);
	float		GetY(int idx);
	float		GetRotation(int idx);
	void		Print();
};
#endif