
/*
 * .x Anode2Channel.c
 *
 *
 * Matteo Turisini 2013 September 15th
 *
 * List Anodes of H8500 MAPMT and the corresponding MAROC channel
 *
 */


#include "../src/TH8500.cpp"
#include "../src/TConnectH8500.cpp"


void Anode2Channel()
{
	TConnectH8500 conn85;	 //conn85.Print(1,0,0);
	for (int i=0; i<64; i++) {//LOOP ON ANODES	(Hamamatsu View)
		printf("%2d <- anode-channel-> %2d \n",i,conn85.GetMaroc(i+1));
	}
}

