// This object handle Interface between  MAPMT H8500 (anodes) and Maroc (channels)


// FE board is described analitically.
// while H8500 adapterboard is described using 
// tables, 
// 1 algorithm 
// and a patch to fix the bug coming from a wrong interpretation of the schematic (Connector roteted by 180 degree).

#ifndef TCONN8500_H
#define TCONN8500_H

class TConnectH8500{
	
private:
	// Adapter Board : PMT H8500 / Samtec Connector (80 pin) Interface
	int anode2pin[64]; //addressed by anode ID and contains the corresponding Pin ID
	int pin2anode[80]; //addressed by pin ID it contains anode ID
public:
	
	TConnectH8500();
	
	int GetAnode(int n){return GetAnodefromPin(GetPinfromMaroc(n));};
	int GetMaroc(int n){return GetMarocfromPin(GetPinfromAnode(n));};
	
	
	
	void Print(bool looponANODES = false,bool looponPIN=false,bool looponMAROCCHANNEL=false,bool Summmary=true); // loop on anodes (1,0,0); loop on pin (0,1)	
	
	
	int GetAnodefromPin(int pin); // A
	int GetMarocfromPin(int pin); // B
	
	int GetPinfromMaroc(int maroc); //C
	int GetPinfromAnode(int anode); //D
	
	int GetRevPin(int pin); // to fix a bug. It Mirror the connector on the adapterboard e.g. Pin1->80 Pin 2->79
	// FE Board : MaroC ASIC / Samtec Connector (80 pin) Interface



		
};
#endif