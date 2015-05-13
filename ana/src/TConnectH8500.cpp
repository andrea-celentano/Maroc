 #include "TConnectH8500.h"
 #include <stdio.h>

//------------------------------------------------
void TConnectH8500::Print(bool looponANODES,bool looponPIN,bool looponMAROCCHANNEL,bool Summary){
	
	printf("HAMAMATSU H8500\n");
	if (looponANODES) {
		// Map
		printf("We have the following matrix of anodes:\n\n\t");
		for(int i=1;i<65; i++){ // loop on anodes
			printf("%2d  ",i);
			if (i%8==0) {
				printf("\n\t");
			}
		}
		printf("\nObtaining the correspondent front end channel is a 2 Step process");
		printf("\nSTEP1 - We have to identify the pin corresponding to each anode.");
		printf("\nTo do this we have 2 methods. First one is a look up table"); 
		printf("\nSecond one is through an algorithm.\n");
		printf("In the following table you can see the results of both methods");
		
		
		printf("\nIn parentesis the value read from the table.\n\n");
		for(int i=1;i<65; i++){ // loop on anodes
			printf("%2d (%2d)  ",GetPinfromAnode(i),anode2pin[i-1]);
			if (i%8==0) {
				printf("\n");
			}
		}
		printf("\nSTEP 2 - We identify the front end channel for each pin");
		printf("\n To do this we have only implemented an algorithm");
		printf("\nThe following table show the result of calculation.\n\n\t");
		for(int i=1;i<65; i++){
			printf("%2d  ",GetMaroc(i));
			if (i%8==0) {
				printf("\n\t");
			}
		}
		printf("\n");

	}
	
	if (looponPIN) {
		printf("PIN: loop on PIN\n");
		for(int i=9;i<73; i++){ // loop on pin
			printf("%2d  ",i);
			if (((i-8)%8)==0) {
				printf("\n");
			}
		}
		printf("------------------------------\n");
		
		printf("ANODES: Calculated (read from table)\n");
		for(int i=9;i<73; i++){ // loop on pin
			printf("%2d (%2d)  ",GetAnodefromPin(i),pin2anode[i-1]);
			if (i%8==0) {
				printf("\n");
			}
		}
		printf("------------------------------\n");
		
		printf("Correspondent MAROC CHANNEL\n");
		for(int i=9;i<73; i++){
			printf("%2d  ",GetMaroc(GetAnodefromPin(i)));
			if (i%8==0) {
				printf("\n");
			}
		}
		printf("------------------------------\n");
		printf("------------------------------\n");
		printf("------------------------------\n");
	}
	
	if (looponMAROCCHANNEL) {
		printf("MAROC CHANNEL: loop on MAROC CHANNEL\n");
		
		for(int i=0;i<64; i++){ // loop on Maroc channel
			printf("%2d  ",i);
			if (i%8==7) {
				printf("\n");
			}
		}
		printf("------------------------------\n");
		
		printf("ANODES: Calculated \n");
		for(int i=0;i<64; i++){ // loop on mAROC cHANEL
			printf("%2d  ",GetAnode(i));
			if (i%8==7) {
				printf("\n");
			}
		}
	}
	if(Summary){
		printf("H8500 Connection\n");
		printf("ANODE (pin) MAROC\n");
		
		for(int i=1;i<65;i++) {
			printf(" %2d   (%2d)   %2d\n"
				   ,i
				   ,GetPinfromAnode(i)
				   ,GetMaroc(i)
				   );
		}
	}
	
}



int TConnectH8500::GetPinfromMaroc(int maroc)
{
	int x = 0;
	if(maroc>63||maroc<0){
		printf("Error in GetPinfromMaroc: maroc index out of range[0..63], you have %d\n",maroc);
		return -1;
	}
	if(maroc%4==1){x=1;}
	if(maroc%4==2){x=-1;}
	return (maroc+9+x);
}

int TConnectH8500::GetMarocfromPin(int pin)
{
	int x = 0;
	if(pin>72||pin<9){
		printf("Error in pinGetMarocfromPin2maroc: pin index out of range[9..72], you have %d\n",pin);
		return -1;
	}
	if(((pin-9)%4)==1){x=+1;}
	if(((pin-9)%4)==2){x=-1;}	
	return (pin-9+x);
}



int TConnectH8500::GetAnodefromPin(int pin)
{
	// GetAnodefromMaroc function read values directly from the table
	// The analitic correspondance was found but with a wrong interpretation of Adapterboard (Samtec Conn turned 180 degrees)
	// so it doesn't work and i decide to came back to table description for the moment...
	
	/* //old version was: 
	int x = 0;
	if((pin-1)%8==0){x=-3;}
	if((pin-1)%8==2){x=-3;}
	if((pin-1)%8==5){x=3;}
	if((pin-1)%8==3){x=4;}
	if((pin-1)%8==7){x=7;}
	
	return (pin-7-x);
	*/
	
	if(pin>72||pin<9){
		printf("Error in pinGetAnodefromPin2maroc: pin index out of range[9..72], you have %d\n",pin);
		return -1;
	}	

	return pin2anode[pin-1];
}

int TConnectH8500::GetPinfromAnode(int anode)
{	
	// GetPinfromAnode function still work well with analitic rule, I just added a call to
	// GetRevPin wich rotate 80 pinn connector before output the result
	
	int x=0;
	if (anode<1||anode>64) {
		printf("Error in GetPinfromAnode argument: anode index out of range [1..64] you have %d\n",anode);
		return -1;
	}
	if((anode)%8==1){x=4;}
	if((anode)%8==2){x=7;}
	if((anode)%8==4){x=3;}
	if((anode)%8==5){x=-3;}
	if((anode)%8==7){x=-3;}
	
	return GetRevPin(anode+7+x);
	
}

int TConnectH8500::GetRevPin(int pin){// to fix bugs
	
	if(pin>80||pin<1){
		printf("Error in TConnectH8500rev::GetRevPin(int pin) argument: index out of range[1..80],you have %d",pin);
		return -1;
	}
	int rev_pin = 0;
	
	if (pin%2==0) {//pari
		rev_pin= 82-pin;
	}
	else {//dispari
		rev_pin= 80-pin;
	}
	return rev_pin;
};



TConnectH8500::TConnectH8500(){

	// old (wrong) correspondance: samtec turned 180 degrees
	// for the right one see below
/*
	anode2pin[0] = 12; // to anode 1 correspond pin 12  
	anode2pin[1] = 16; // to anode 2 correspond pin 16 
	anode2pin[2] = 10; // and so on ...
	anode2pin[3] = 14;
	anode2pin[4] = 9;
	anode2pin[5] = 13;
	anode2pin[6] = 11;
	anode2pin[7] = 15;
	anode2pin[8] = 20;
	anode2pin[9] = 24;
	anode2pin[10] = 18;
	anode2pin[11] = 22;
	anode2pin[12] = 17;
	anode2pin[13] = 21;
	anode2pin[14] = 19;
	anode2pin[15] = 23;
	anode2pin[16] = 28;
	anode2pin[17] = 32;
	anode2pin[18] = 26;
	anode2pin[19] = 30;
	anode2pin[20] = 25;
	anode2pin[21] = 29;
	anode2pin[22] = 27;
	anode2pin[23] = 31;
	anode2pin[24] = 36;
	anode2pin[25] = 40;
	anode2pin[26] = 34;
	anode2pin[27] = 38;
	anode2pin[28] = 33;
	anode2pin[29] = 37;
	anode2pin[30] = 35;
	anode2pin[31] = 39;
	anode2pin[32] = 44;
	anode2pin[33] = 48;
	anode2pin[34] = 42;
	anode2pin[35] = 46;
	anode2pin[36] = 41;
	anode2pin[37] = 45;
	anode2pin[38] = 43;
	anode2pin[39] = 47;
	anode2pin[40] = 52;
	anode2pin[41] = 56;
	anode2pin[42] = 50;
	anode2pin[43] = 54;
	anode2pin[44] = 49;
	anode2pin[45] = 53;
	anode2pin[46] = 51;
	anode2pin[47] = 55;
	anode2pin[48] = 60;
	anode2pin[49] = 64;
	anode2pin[50] = 58;
	anode2pin[51] = 62;
	anode2pin[52] = 57;
	anode2pin[53] = 61;
	anode2pin[54] = 59;
	anode2pin[55] = 63;
	anode2pin[56] = 68;
	anode2pin[57] = 72;
	anode2pin[58] = 66;
	anode2pin[59] = 70;
	anode2pin[60] = 65;
	anode2pin[61] = 69;
	anode2pin[62] = 67;
	anode2pin[63] = 71;

	
	
	
	*/
	 
	 //REVISION agosto 2011
	anode2pin[0] = 70; // to anode 1 correspond pin 12
	anode2pin[1] = 66; // to anode 2 correspond pin 16 
	anode2pin[2] = 72; // and so on ...
	anode2pin[3] = 68;
	anode2pin[4] = 71;
	anode2pin[5] = 67;
	anode2pin[6] = 69;
	anode2pin[7] = 65;
	anode2pin[8] = 62;
	anode2pin[9] = 58;
	anode2pin[10] = 64;
	anode2pin[11] = 60;
	anode2pin[12] = 63;
	anode2pin[13] = 59;
	anode2pin[14] = 61;
	anode2pin[15] = 57;
	anode2pin[16] = 54;
	anode2pin[17] = 50; 
	anode2pin[18] = 56;
	anode2pin[19] = 52;
	anode2pin[20] = 55;
	anode2pin[21] = 51;
	anode2pin[22] = 53;
	anode2pin[23] = 49;
	anode2pin[24] = 46;
	anode2pin[25] = 42;
	anode2pin[26] = 48;
	anode2pin[27] = 44;
	anode2pin[28] = 47;
	anode2pin[29] = 43;
	anode2pin[30] = 45;
	anode2pin[31] = 41;
	anode2pin[32] = 38;
	anode2pin[33] = 34;
	anode2pin[34] = 40;
	anode2pin[35] = 36;
	anode2pin[36] = 39;
	anode2pin[37] = 35;
	anode2pin[38] = 37;
	anode2pin[39] = 33;
	anode2pin[40] = 30;
	anode2pin[41] = 26;
	anode2pin[42] = 32;
	anode2pin[43] = 28;
	anode2pin[44] = 31;
	anode2pin[45] = 27;
	anode2pin[46] = 29;
	anode2pin[47] = 25;
	anode2pin[48] = 22;
	anode2pin[49] = 18;
	anode2pin[50] = 24;
	anode2pin[51] = 20;
	anode2pin[52] = 23;
	anode2pin[53] = 19;
	anode2pin[54] = 21;
	anode2pin[55] = 17;
	anode2pin[56] = 14;
	anode2pin[57] = 10;
	anode2pin[58] = 16;
	anode2pin[59] = 12;
	anode2pin[60] = 15;
	anode2pin[61] = 11;
	anode2pin[62] = 13;
	anode2pin[63] = 9;

	

	// old (wrong) correspondance: samtec turned 180 degrees
	// fro the right one see below
	/*
	pin2anode[0] = 0; // Pin 1..Pin 8 are connected to GND    
	pin2anode[1] = 0;
	pin2anode[2] = 0;
	pin2anode[3] = 0;
	pin2anode[4] = 0;
	pin2anode[5] = 0;
	pin2anode[6] = 0;
	pin2anode[7] = 0;
	pin2anode[8] = 5;  // Pin 9  is connected to Anode 5
	pin2anode[9] = 3;
	pin2anode[10] = 7; 
	pin2anode[11] = 1;
	pin2anode[12] = 6;
	pin2anode[13] = 4;
	pin2anode[14] = 8;
	pin2anode[15] = 2;
	pin2anode[16] = 13;
	pin2anode[17] = 11;
	pin2anode[18] = 15;
	pin2anode[19] = 9;
	pin2anode[20] = 14; 
	pin2anode[21] = 12;
	pin2anode[22] = 16;
	pin2anode[23] = 10;
	pin2anode[24] = 21;
	pin2anode[25] = 19;
	pin2anode[26] = 23;
	pin2anode[27] = 17;
	pin2anode[28] = 22;
	pin2anode[29] = 20;
	pin2anode[30] = 24; 
	pin2anode[31] = 18;
	pin2anode[32] = 29;
	pin2anode[33] = 27;
	pin2anode[34] = 31;
	pin2anode[35] = 25;
	pin2anode[36] = 30;
	pin2anode[37] = 28;
	pin2anode[38] = 32;
	pin2anode[39] = 26;
	pin2anode[40] = 37; 
	pin2anode[41] = 35;
	pin2anode[42] = 39;
	pin2anode[43] = 33;
	pin2anode[44] = 38;
	pin2anode[45] = 36;
	pin2anode[46] = 40;
	pin2anode[47] = 34;
	pin2anode[48] = 45;
	pin2anode[49] = 43;
	pin2anode[50] = 47; 
	pin2anode[51] = 41;
	pin2anode[52] = 46;
	pin2anode[53] = 44;
	pin2anode[54] = 48;
	pin2anode[55] = 42;
	pin2anode[56] = 53;
	pin2anode[57] = 51;
	pin2anode[58] = 55;
	pin2anode[59] = 49;
	pin2anode[60] = 54; 
	pin2anode[61] = 52;
	pin2anode[62] = 56;
	pin2anode[63] = 50;
	pin2anode[64] = 61;
	pin2anode[65] = 59;
	pin2anode[66] = 63;
	pin2anode[67] = 57;
	pin2anode[68] = 62;
	pin2anode[69] = 60;
	pin2anode[70] = 64; 
	pin2anode[71] = 58;
	pin2anode[72] = 0;	// Pin 73 to 80 are connected to GND
	pin2anode[73] = 0;
	pin2anode[74] = 0;
	pin2anode[75] = 0;
	pin2anode[76] = 0;
	pin2anode[77] = 0;
	pin2anode[78] = 0;
	pin2anode[79] = 0;
	
	*/
	
	//REVISION agosto 2011
	pin2anode[ 0] =  0; // Pin 1..Pin 8 are connected to GND 
	pin2anode[ 1] =  0;
	pin2anode[ 2] =  0;
	pin2anode[ 3] =  0;
	pin2anode[ 4] =  0;
	pin2anode[ 5] =  0;
	pin2anode[ 6] =  0;
	pin2anode[ 7] =  0;
	pin2anode[ 8] = 64;  // Pin 9  is connected to Anode 5
	pin2anode[ 9] = 58;
	pin2anode[10] = 62; 
	pin2anode[11] = 60;
	pin2anode[12] = 63;
	pin2anode[13] = 57;
	pin2anode[14] = 61;
	pin2anode[15] = 59;
	pin2anode[16] = 56;
	pin2anode[17] = 50;
	pin2anode[18] = 54;
	pin2anode[19] = 52;
	pin2anode[20] = 55; 
	pin2anode[21] = 49;
	pin2anode[22] = 53;
	pin2anode[23] = 51;
	pin2anode[24] = 48;
	pin2anode[25] = 42;
	pin2anode[26] = 46;
	pin2anode[27] = 44;
	pin2anode[28] = 47;
	pin2anode[29] = 41;
	pin2anode[30] = 45; 
	pin2anode[31] = 43;
	pin2anode[32] = 40;
	pin2anode[33] = 34;
	pin2anode[34] = 38;
	pin2anode[35] = 36;
	pin2anode[36] = 39;
	pin2anode[37] = 33;
	pin2anode[38] = 37;
	pin2anode[39] = 35;
	pin2anode[40] = 32; 
	pin2anode[41] = 26;
	pin2anode[42] = 30;
	pin2anode[43] = 28;
	pin2anode[44] = 31;
	pin2anode[45] = 25;
	pin2anode[46] = 29;
	pin2anode[47] = 27;
	pin2anode[48] = 24;
	pin2anode[49] = 18;
	pin2anode[50] = 22; 
	pin2anode[51] = 20;
	pin2anode[52] = 23;
	pin2anode[53] = 17;
	pin2anode[54] = 21;
	pin2anode[55] = 19;
	pin2anode[56] = 16;
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
	
}