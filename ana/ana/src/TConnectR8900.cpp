/*
 *  TConnectR8900.cpp
 *  
 *
 *  Created by Matteo Turisini on 13/07/11.
 *  Copyright 2011 Universita e INFN, Roma I (IT). All rights reserved.
 *
 */

#include "TConnectR8900.h"
#include <stdio.h>





TConnectR8900::TConnectR8900(){
	

	//PMT 1
	
	anode2pin[ 0][0] = 70; // to anode 1 correspond pin 70
	anode2pin[ 1][0] = 66; // to anode 2 correspond pin 66 
	anode2pin[ 2][0] = 72; // and so on ...
	anode2pin[ 3][0] = 68;
	anode2pin[ 4][0] = 71;
	anode2pin[ 5][0] = 67;
	anode2pin[ 6][0] = 69;
	anode2pin[ 7][0] = 65;
	anode2pin[ 8][0] = 62;
	anode2pin[ 9][0] = 58;
	anode2pin[10][0] = 64;
	anode2pin[11][0] = 60;
	anode2pin[12][0] = 63;
	anode2pin[13][0] = 59;
	anode2pin[14][0] = 61;
	anode2pin[15][0] = 57;
	
	//PMT 2
	anode2pin[ 0][1] = 54;
	anode2pin[ 1][1] = 50;
	anode2pin[ 2][1] = 56;
	anode2pin[ 3][1] = 52;
	anode2pin[ 4][1] = 55;
	anode2pin[ 5][1] = 51;
	anode2pin[ 6][1] = 53;
	anode2pin[ 7][1] = 49;
	anode2pin[ 8][1] = 46;
	anode2pin[ 9][1] = 42;
	anode2pin[10][1] = 48;
	anode2pin[11][1] = 44;
	anode2pin[12][1] = 47;
	anode2pin[13][1] = 43;
	anode2pin[14][1] = 45;
	anode2pin[15][1] = 41;
	
	//PMT 3
	anode2pin[ 0][2] = 38;
	anode2pin[ 1][2] = 34;
	anode2pin[ 2][2] = 40;
	anode2pin[ 3][2] = 36;
	anode2pin[ 4][2] = 39;
	anode2pin[ 5][2] = 35;
	anode2pin[ 6][2] = 37;
	anode2pin[ 7][2] = 33;
	anode2pin[ 8][2] = 30;
	anode2pin[ 9][2] = 26;
	anode2pin[10][2] = 32;
	anode2pin[11][2] = 28;
	anode2pin[12][2] = 31;
	anode2pin[13][2] = 27;
	anode2pin[14][2] = 29;
	anode2pin[15][2] = 25;
	
	//PMT 4
	anode2pin[ 0][3] = 22;
	anode2pin[ 1][3] = 18;
	anode2pin[ 2][3] = 24;
	anode2pin[ 3][3] = 20;
	anode2pin[ 4][3] = 23;
	anode2pin[ 5][3] = 19;
	anode2pin[ 6][3] = 21;
	anode2pin[ 7][3] = 17;
	anode2pin[ 8][3] = 14;
	anode2pin[ 9][3] = 10;
	anode2pin[10][3] = 16;
	anode2pin[11][3] = 12;
	anode2pin[12][3] = 15;
	anode2pin[13][3] = 11;
	anode2pin[14][3] = 13;
	anode2pin[15][3] = 9;
	
	
	pin2anode[ 0] = 0; // Pin 1..Pin 8 are connected to GND 
	pin2anode[ 1] = 0;
	pin2anode[ 2] = 0;
	pin2anode[ 3] = 0;
	pin2anode[ 4] = 0;
	pin2anode[ 5] = 0;
	pin2anode[ 6] = 0;
	pin2anode[ 7] = 0;
	
	pin2anode[ 8] = 16;  // Pin 9  is connected to Anode 16 (of PMT 4)
	pin2anode[ 9] = 10;
	pin2anode[10] = 14; 
	pin2anode[11] = 12;
	pin2anode[12] = 15;
	pin2anode[13] = 9;
	pin2anode[14] = 13;
	pin2anode[15] = 11;
	pin2anode[16] = 8;
	pin2anode[17] = 2;
	pin2anode[18] = 6;
	pin2anode[19] = 4;
	pin2anode[20] = 7; 
	pin2anode[21] = 1;
	pin2anode[22] = 5;
	pin2anode[23] = 3;
	
	pin2anode[24] = 16; // Pin 25  is connected to Anode 16 (of PMT 3)
	pin2anode[25] = 10;
	pin2anode[26] = 14;
	pin2anode[27] = 12;
	pin2anode[28] = 15;
	pin2anode[29] = 9;
	pin2anode[30] = 13; 
	pin2anode[31] = 11;
	pin2anode[32] = 8;
	pin2anode[33] = 2;
	pin2anode[34] = 6;
	pin2anode[35] = 4;
	pin2anode[36] = 7;
	pin2anode[37] = 1;
	pin2anode[38] = 5;
	pin2anode[39] = 3;
	
	pin2anode[40] = 16; // Pin 41  is connected to Anode 16 (of PMT 2)
	pin2anode[41] = 10;
	pin2anode[42] = 14;
	pin2anode[43] = 12;
	pin2anode[44] = 15;
	pin2anode[45] = 9;
	pin2anode[46] = 13;
	pin2anode[47] = 11;
	pin2anode[48] = 8;
	pin2anode[49] = 2;
	pin2anode[50] = 6; 
	pin2anode[51] = 4;
	pin2anode[52] = 7;
	pin2anode[53] = 1;
	pin2anode[54] = 5;
	pin2anode[55] = 3;
	
	pin2anode[56] = 16;  // Pin 57  is connected to Anode 16 (of PMT 1)
	pin2anode[57] = 10;
	pin2anode[58] = 14;
	pin2anode[59] = 12;
	pin2anode[60] = 15; 
	pin2anode[61] = 9;
	pin2anode[62] = 13;
	pin2anode[63] = 11;
	pin2anode[64] = 8;
	pin2anode[65] = 2;
	pin2anode[66] = 6;
	pin2anode[67] = 4;
	pin2anode[68] = 7;
	pin2anode[69] = 1;
	pin2anode[70] = 5; 
	pin2anode[71] = 3;
	
	pin2anode[72] = 0;	// Pin 73 to 80 are connected to GND
	pin2anode[73] = 0;
	pin2anode[74] = 0;
	pin2anode[75] = 0;
	pin2anode[76] = 0;
	pin2anode[77] = 0;
	pin2anode[78] = 0;
	pin2anode[79] = 0;

	pin2pmt[ 0] = 0;
	pin2pmt[ 1] = 0;
	pin2pmt[ 2] = 0;
	pin2pmt[ 3] = 0;
	pin2pmt[ 4] = 0;
	pin2pmt[ 5] = 0;
	pin2pmt[ 6] = 0;
	pin2pmt[ 7] = 0;
	
	
	pin2pmt[ 8] = 4;
	pin2pmt[ 9] = 4;
	pin2pmt[10] = 4;
	pin2pmt[11] = 4;
	pin2pmt[12] = 4;
	pin2pmt[13] = 4;
	pin2pmt[14] = 4;
	pin2pmt[15] = 4;
	pin2pmt[16] = 4;
	pin2pmt[17] = 4;
	pin2pmt[18] = 4;
	pin2pmt[19] = 4;
	pin2pmt[20] = 4;
	pin2pmt[21] = 4;
	pin2pmt[22] = 4;
	pin2pmt[23] = 4;
	
	pin2pmt[24] = 3;
	pin2pmt[25] = 3;
	pin2pmt[26] = 3;
	pin2pmt[27] = 3;
	pin2pmt[28] = 3;
	pin2pmt[29] = 3;
	pin2pmt[30] = 3;
	pin2pmt[31] = 3;
	pin2pmt[32] = 3;
	pin2pmt[33] = 3;
	pin2pmt[34] = 3;
	pin2pmt[35] = 3;
	pin2pmt[36] = 3;
	pin2pmt[37] = 3;
	pin2pmt[38] = 3;
	pin2pmt[39] = 3;	
	
	pin2pmt[40] = 2;
	pin2pmt[41] = 2;
	pin2pmt[42] = 2;
	pin2pmt[43] = 2;
	pin2pmt[44] = 2;
	pin2pmt[45] = 2;
	pin2pmt[46] = 2;
	pin2pmt[47] = 2;
	pin2pmt[48] = 2;
	pin2pmt[49] = 2;	
	pin2pmt[50] = 2;
	pin2pmt[51] = 2;
	pin2pmt[52] = 2;
	pin2pmt[53] = 2;
	pin2pmt[54] = 2;
	pin2pmt[55] = 2;
	
	pin2pmt[56] = 1;
	pin2pmt[57] = 1;
	pin2pmt[58] = 1;
	pin2pmt[59] = 1;
	pin2pmt[60] = 1;
	pin2pmt[61] = 1;
	pin2pmt[62] = 1;
	pin2pmt[63] = 1;
	pin2pmt[64] = 1;
	pin2pmt[65] = 1;
	pin2pmt[66] = 1;
	pin2pmt[67] = 1;
	pin2pmt[68] = 1;
	pin2pmt[69] = 1;	
	pin2pmt[70] = 1;
	pin2pmt[71] = 1;
	
	pin2pmt[72] = 0;
	pin2pmt[73] = 0;
	pin2pmt[74] = 0;
	pin2pmt[75] = 0;
	pin2pmt[76] = 0;
	pin2pmt[77] = 0;
	pin2pmt[78] = 0;
	pin2pmt[79] = 0;
}

void TConnectR8900::Print(){
	printf("// Front view (Beam)\n");
	printf("//--------------//--------------//\n");
	printf("//              //              //\n");
	printf("//              //              //\n");
	printf("//              //              //\n");
	printf("//       2      //      3       //\n");
	printf("//              //              //\n");
	printf("//              //              //\n");
	printf("//--------------//--------------//\n");
	printf("//              //              //\n");
	printf("//              //              //\n");
	printf("//              //              //\n");
	printf("//       1      //      4       //\n");
	printf("//              //              //\n");
	printf("//              //              //\n");
	printf("//--------------//--------------//\n");

	printf("\t\t\t\t\t HAMAMATSU R8900 x 4 \n");
	
	 printf("PMT  ANODE  PIN\n");
	for (int i=0; i<4; i++) {
		for (int j=0; j<16; j++) {
			printf("%2d    %2d     %2d\n",i+1,j+1,anode2pin[j][i]);
		}
		printf("\nPMT  ANODE  PIN\n");
	}
	
	printf("------------------\n");
	/* tentativo frettoloso di rappresentare tipo mappa spaziale gli anodi dei 4 PMT
	printf("\t\t\t\t\t HAMAMATSU R8900 x 4 \n");
	for (int k=9; k<73; k++) {// loop on pins
		printf("(%2d)%1d-%2d  ",k,GetPmtfromPin(k),GetAnodefromPin(k));
		if ((k-8)%8==0) {
			printf("\n");
		}
	}

	 */
}

int TConnectR8900::GetPinfromPMTandAnode(int pmt,int anode){
	if (pmt<1||pmt>4) {
		printf("Error in TConnectR8900:: GetPin() argument: pmt range is [1..4],you have %2d\n",pmt);
		return -1;
	}
	if (anode<1||anode>16) {
		printf("Error in TConnectR8900:: GetPin() argument: anode range is [1..16],you have %2d\n",anode);
		return -1;
	}
	int x = 0;
	x = anode2pin[anode-1][pmt-1];
	return x;
};


int TConnectR8900::GetAnodefromPin(int pin){
	if(pin<9||pin>72){
		printf("Error in TConnectR8900:: GetAnode() argument: pin range is [9..72],you have %2d\n",pin);
	return -1;
	}
	int x = 0;
	x = pin2anode[pin-1];
	return x;
	
};
int TConnectR8900::GetPmtfromPin(int pin){
	if(pin<9||pin>72){
		printf("Error in TConnectR8900:: GetPmt() argument: pin range is [9..72],you have %2d\n",pin);
		return -1;
	}
	int x = 0;
	x = pin2pmt[pin-1];
	return x;
};


int TConnectR8900::GetPinfromMaroc(int maroc) {
	
	int x = 0;
	if(maroc>63||maroc<0){
		printf("Error in GetPinfromMaroc: maroc index out of range[0..63], you have %d\n",maroc);
		return -1;
	}
	if(maroc%4==1){x=1;}
	if(maroc%4==2){x=-1;}
	return (maroc+9+x);
};

int TConnectR8900::GetMarocfromPin(int pin) {
	
	int x = 0;
	if(pin>72||pin<9){
		printf("Error in pinGetMarocfromPin2maroc: pin index out of range[9..72], you have %d\n",pin);
		return -1;
	}
	if(((pin-9)%4)==1){x=+1;}
	if(((pin-9)%4)==2){x=-1;}	
	return (pin-9+x);
};


int TConnectR8900::GetMaroc(int anode, int pmt){
	if (anode<1||anode>64) {
		printf("Error in GetMaroc() argument: anode index out of range [1..64] you have %d\n",anode);
		return -1;
	}
	if (pmt<1||pmt>4) {
		printf("Error in GetMaroc() argument: pmt index out of range [1..4] you have %d\n",pmt);
		return -1;
	}
	return GetMarocfromPin(GetPinfromPMTandAnode(pmt,anode));
}
int TConnectR8900::GetAnode(int maroc){//partial information
	if (maroc<0||maroc>63) {
		printf("Error in GetAnode argument: maroc index out of range [0..63] you have %d\n",maroc);
		return -1;
	}
	return GetAnodefromPin(GetPinfromMaroc(maroc));
}
int TConnectR8900::GetPmt(int maroc){ // partial information
	if (maroc<0||maroc>63) {
		printf("Error in GetAnode argument: maroc index out of range [0..63] you have %d\n",maroc);
		return -1;
	}
	return GetPmtfromPin(GetPinfromMaroc(maroc));
}
