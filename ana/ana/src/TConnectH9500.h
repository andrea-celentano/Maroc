/*
 *  TConnectH9500.h
 *  
 *
 *  Created by Matteo Turisini on 06/09/11.
 *  Copyright 2011 Universita e INFN, Roma I (IT). All rights reserved.
 *
 */

// This object handle Interface between  MAPMT H9500 (anodes) and Maroc (channels)
// Each anode is first matched with one of the 4 Samtec 80-pin connectors fSig[256] = 1|2|3|4
// PinID  are assigned manually to on SIG 1 and than just copied onto SIG2,SIG3,SIG4
// Finally the maroc to pin correspondence is performed (analitically as in H8500 connector)

#ifndef TCONN9500_H
#define TCONN9500_H

class TConnectH9500
{
private:
	short fAnode[256];// Anode ID , address everything
	char	fSig[256];  // SIG connector ID  
	char	fPin[256];  // Pin ID , fSig dependent
	char	fMaroc[256];// Maroc channel, fSig dependent 
	
	int		GetMarocfromPin(int pin);
	
public:
	
	TConnectH9500();
	
	char	GetMaroc(short anode);
	char	GetSig(short anode);
	short 	GetAnode(char sig, char maroc);
	
	void	Print();
	void	PrintSIG(char sig);
};
#endif