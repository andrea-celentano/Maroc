/*
 *  MRCConfigurationManager.cpp
 *
 *  Author: Matteo Turisini
 *  Date: 2014 Nov 9th
 *
 */

#include "MRCSetup.h"

MRCSetup::MRCSetup(char *file)
{
	fcfg = new GI_Config();
	fcfg->parseFile(file); 
	ffilename = file;
}

MRCSetup::~MRCSetup(){
	if (fcfg!=NULL) {
		delete fcfg;
	}
}
std::string	 MRCSetup::GetVersion(){
	return fcfg->get<std::string>(fret, 0,"version");
}
int	 MRCSetup::GetNfe(){
	return fcfg->get<int>(fret, 0, "N");
}
std::string	MRCSetup::GetMapmtID(int idx){	
	return fcfg->get<std::string>(fret, 0, "id",idx,"mapmts");
}
int	MRCSetup::GetHV(int idx){
	return fcfg->get<int>(fret, 0, "hv",idx,"mapmts");
}
int	MRCSetup::GetFeID(int idx){
	return fcfg->get<int>(fret, 0, "fe",idx,"mapmts");
}
int	MRCSetup::GetGeo(int idx){
	return  fcfg->get<int>(fret, 0, "geo",idx,"mapmts");
}
float MRCSetup::GetX(int idx){
	return fcfg->get<float>(fret,0,"x", 0, "pos",idx,"mapmts");
}

float MRCSetup::GetY(int idx){
	return fcfg->get<float>(fret,0,"y", 0, "pos",idx,"mapmts");

}
float MRCSetup::GetRotation(int idx){
	return fcfg->get<float>(fret,0,"deg", 0, "pos",idx,"mapmts");
}
void MRCSetup::Print(){
	int fenum = GetNfe();
	printf("File %s\n",ffilename);
	printf("%d MAROC front end card(s)\n",fenum);	
	printf("IDX\tFE\tGEO\tMAPMT\tHV\tX\tY\tDEG\n");
	for (int i=0; i<fenum; i++) {
		printf("%d\t%d\t%d\t%s\t%d\t%3.1f\t%3.1f\t%3.1f\n",i,GetFeID(i),GetGeo(i),GetMapmtID(i).c_str(),GetHV(i),GetX(i),GetY(i),GetRotation(i));
	}
	

}


