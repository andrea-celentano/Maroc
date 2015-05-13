
/*
 * .x Channel2Anode.c
 *
 *
 * Matteo Turisini 2013 September 15th
 *
 * List MAROC channel and the corresponding anodes of H8500 MAPMT 
 *
 */

#include "../src/TConnectH8500.cpp"


void Channel2Anode()
{
	TConnectH8500 conn85;	 //conn85.Print(1,0,0);
	for (int i=0; i<64; i++) {//Loop on maroc channels	(Maroc View)
		
		printf("%2d <- maroc-anode-> %2d \n",i,conn85.GetAnode(i));
	}
}

