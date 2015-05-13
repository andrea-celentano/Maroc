/*
 *  TConnectR8900.h
 *  
 *
 *  Created by Matteo Turisini on 13/07/11.
 *  Copyright 2011 Universita e INFN, Roma I (IT). All rights reserved.
 *
 */

#ifndef TCONN8900_H
#define TCONN8900_H

class TConnectR8900
{
private:
	int anode2pin[16][4];
	int pin2anode[80];
	int pin2pmt[80];
	// 4 H8900-Connector interface
		//int GetAnodefromPin(int pin);
	
	// Connector-Fe interface
	//int GetPinfromMaroc(int pin);
	//int GetMarocfromPin(int maroc);
	
public:
	TConnectR8900();
	//void Test();
	void Print();
	
	int GetPinfromPMTandAnode(int pmt=1,int anode=1);
	int GetAnodefromPin(int pin=9);
	int GetPmtfromPin(int pin=9);
	
	int GetPinfromMaroc(int pin=9);
	int GetMarocfromPin(int maroc=63);
	
	
	int GetMaroc(int anode,int pmt);
	int GetAnode(int maroc);
	int GetPmt(int maroc);
};
#endif