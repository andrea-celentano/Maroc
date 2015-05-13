/*
 *  TConnectH9500.cpp
 *  
 *
 *  Created by Matteo Turisini on 06/09/11.
 *  Copyright 2011 Universita e INFN, Roma I (IT). All rights reserved.
 *
 */


#include "TConnectH9500.h"
#include <stdio.h>

TConnectH9500::TConnectH9500(){
	for (int i=0; i<256; i++) {
		// Anode Assignement
		fAnode[i]=i+1;
		
		// Sig Connector Assignement
		if (i%16<4) {
			fSig[i]=1;
		}
		else {
			fSig[i]=0;
		}
		if (i%16<8&&i%16>3) {
			fSig[i]=2;
		}
		if (i%16<12&&i%16>7) {
			fSig[i]=3;
		}
		if (i%16<16&&i%16>11) {
			fSig[i]=4;
		}
		// Pin Assignement
		for (int j=0; j<256; j++) {
			fPin[j]=0;
		}
	}
	//SIG1
	fPin[  0] = 71; // anode 1 is connected to pin 71
	fPin[  1] = 69; // anode 2 is connected to pin 69
	fPin[  2] = 72; // anode 3 is connected to pin 72
	fPin[  3] = 70; // anode 4 is connected to pin 70
	
	fPin[ 16] = 67; // anode 17 is connected to pin 67
	fPin[ 17] = 65; // and so on ...
	fPin[ 18] = 68;
	fPin[ 19] = 66;
		
	fPin[ 32] = 63; //n   -> p
	fPin[ 33] = 61; //n+1 ->p-2 
	fPin[ 34] = 64; //n+2 ->p+1
	fPin[ 35] = 62; //n+3 ->p-1 // but relation between n and p is not easy to see
		
	fPin[ 48] = 59;
	fPin[ 49] = 57;
	fPin[ 50] = 60;
	fPin[ 51] = 58;
	
	fPin[ 64] = 55;
	fPin[ 65] = 53;
	fPin[ 66] = 56;
	fPin[ 67] = 54;
		
	fPin[ 80] = 51;
	fPin[ 81] = 49;
	fPin[ 82] = 52;
	fPin[ 83] = 50;

	fPin[ 96] = 47;
	fPin[ 97] = 45;
	fPin[ 98] = 48;
	fPin[ 99] = 46;
		
	fPin[112] = 43;
	fPin[113] = 41;
	fPin[114] = 44;
	fPin[115] = 42;
		
	fPin[128] = 39;
	fPin[129] = 37;
	fPin[130] = 40;
	fPin[131] = 38;

	fPin[144] = 35;
	fPin[145] = 33;
	fPin[146] = 36;
	fPin[147] = 34;
		
	fPin[160] = 31;
	fPin[161] = 29;
	fPin[162] = 32;
	fPin[163] = 30;
		
	fPin[176] = 27;
	fPin[177] = 25;
	fPin[178] = 28;
	fPin[179] = 26;
	
	fPin[192] = 23;
	fPin[193] = 21;
	fPin[194] = 24;
	fPin[195] = 22;
		
	fPin[208] = 19;
	fPin[209] = 17;
	fPin[210] = 20;
	fPin[211] = 18;

	fPin[224] = 15;
	fPin[225] = 13;
	fPin[226] = 16;
	fPin[227] = 14;
		
	fPin[240] = 11;
	fPin[241] =  9;
	fPin[242] = 12;
	fPin[243] = 10;
		
	
	// SIG2, SIG3, SIG4
	for (int m=4; m<245; m=m+16) {
		for (int n = m; n<m+12; n++) {
			fPin[n]=fPin[n-4];
		}
	}
	
	// Assigning Maroc channel to each Pin
	for (int i=0; i<256; i++) {
		fMaroc[i]=GetMarocfromPin(fPin[i]);
	}
	
};



int TConnectH9500::GetMarocfromPin(int pin)
{
	// its the same of H8500
	int x = 0;
	if(pin>72||pin<9){
		printf("Error in pinGetMarocfromPin2maroc: pin index out of range[9..72], you have %d\n",pin);
		return -1;
	}
	if(((pin-9)%4)==1){x=+1;}
	if(((pin-9)%4)==2){x=-1;}	
	return (pin-9+x);
};





void TConnectH9500::Print(){
	// ANODE MATRIX VIEW
	/*
	for (int i=1; i<257; i++) { // loop on Anodes
		if (fSig[i-1]!=0) {
			printf("%3d(%d)  ",fAnode[i-1],fSig[i-1]);
		}
		else{
			printf("%3d     ",fAnode[i-1]);
		}
	
		if (i%16==0) {
			printf("\n");
		}
	}
	 */
	// LIST VIEW
	printf("ANODE  SIG  PIN   MAROC  | SIGfromAnode(test) MarocfromAnode(test) Anode(test) \n");
	for (int i=0; i<256; i++) {
		if (fPin[i]!=0) {
			printf("%3d     %d    %2d     %2d | %d   %2d    %3d\n",fAnode[i],fSig[i],fPin[i],fMaroc[i],GetSig(fAnode[i]),GetMaroc(fAnode[i]),GetAnode(GetSig(fAnode[i]),GetMaroc(fAnode[i])));
		}
		else{
			printf("%3d     %d\n",fAnode[i],fSig[i]);
		}
		if (i%4==3) {
			printf("---------------------\n");
		}
	}
};

void TConnectH9500::PrintSIG(char sig){
	printf("H9500 Signal Connection, SIG%d \n",sig);
	for (int i=0; i<256; i++) { // shifting on the array index to find Anodes with specified Sig ID
		if (fSig[i]==sig) {
			printf("%d %3d  ",fSig[i],fAnode[i]);
			if (i%2==1) {
				printf("\n");
			}
		}
	}
	printf("--------\n\n");
};



char	TConnectH9500::GetMaroc(short anode){
	if (anode>256||anode<1) {
		printf("Error in TConnectH9500::GetMaroc(): argument out of range [1..256],you have %d ",anode);
		return -1;
	}
	else{
		for (int i=0; i<256; i++) { // loop on array index
			if (fAnode[i]==anode) {
				return fMaroc[i];
			}
		}
	
	}
	return -2;
};
char	TConnectH9500::GetSig(short anode){
	if (anode>256||anode<1) {
		printf("Error in TConnectH9500::GetSig(): argument out of range [1..256],you have %d ",anode);
		return -1;
	}
	else{
		for (int i=0; i<256; i++) { // loop on array index
			if (fAnode[i]==anode) {
				return fSig[i];
			}
		}
		
	}
	return -2;
};
short	TConnectH9500::GetAnode(char sig, char maroc){
	if (sig>4||sig<1) {
		printf("Error in TConnectH9500::GetAnode(): argument 1 out of range [1..4],you have %d ",sig);
		return -1;
	}
	if (maroc>63||maroc<0) {
		printf("Error in TConnectH9500::GetAnode(): argument 2 out of range [0..63],you have %d ",maroc);
		return -1;
	}
	for (int i=0; i<256; i++) {
		if (fSig[i]==sig) {
			if (fMaroc[i]==maroc) {
				return fAnode[i];
			}
		}
	}
	return -2;
};


