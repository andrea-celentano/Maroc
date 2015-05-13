#include "TMaroc.h"
TMaroc::TMaroc()
{	
	memset(fdata  , 0, sizeof(fdata  )); // memset Riempie blocchi di memoria: void * memset ( void * ptr, int value, size_t num );	 assegna 'value' alle prime 'num' locazioni  del blocco di memoria puntato da 'ptr'.'num' viene interpretato UNSIGNED CHAR ovvero indica il numero di Byte da riempire. memset fa parte delle funzioni della C-libreria cstring	
	memset(fhitmap, 0, sizeof(fhitmap));

	fgeo    = 0;
	fseq	= 0;
	ftimer  = 0;
	frecord	= 0;
				
	MRC_DBG("new TMaroc instance \n");	
}

void TMaroc::SetHit(int channel){

	fhitmap[channel/8] |= (1 << (channel % 8));
	MRC_DUM("Channel %2d triggered\n",channel);	
	
	// Set hit agisce mediante or inclusivo tra il byte di hitmap contenente il canale che si vuole settare	
	// ed il byte 00000001 shiftato a sinistra  di tante posizioni quanto e' il valore del canale espresso in aritmetica modulo 8 [0,1,2,3,4,5,6,7] 
	// NOTA : la convenzione sull'ordinamento e' [63...56][55...48][47...40][39...32][31...24][23...16][15...8][7...0]	
}

int TMaroc::GetHit(int channel){
	MRC_DBG("Channel %2d Hit = ",channel);	
	
	if( fhitmap[channel/8] & (1 << (channel % 8)) ){ 	
		MRC_DUM("= 1");
		return 1;
	}
	else{
		MRC_DUM("= 0");
		return 0;
	}
	MRC_DBG("\n");	
}


 
void TMaroc::SetACQmode(signed char nn){
	if((nn&0x30)==0x00){// 00 all chs
		fmode=0;
		MRC_DUM("Readout Mode: ALL\n");
	}
	if((nn&0x30)==0x10){// 01 hit chs
		fmode=1;
		MRC_DUM("Readout Mode: ONLY TRIGGERED\n");
	}
	if((nn&0x30)==0x20){// 10 above thr chs
		fmode=2;
		MRC_DUM("Readout Mode: OVER THR\n");
	}
	if((nn&0x30)==0x30){// 11 none chs
		fmode=3;
		MRC_DUM("Readout Mode: NONE\n");
	}	
};

void TMaroc::SetData(int channel, unsigned short adc_datum)
{
	fdata[channel]	= adc_datum	; 
	MRC_DUM("ADC[%2d] = %4d\n",channel,adc_datum);
};

void TMaroc::SetHeaderSeq(unsigned char  header_seq){
	fseq		= header_seq; 
	MRC_DUM("SEQUENTIAL TAG: %d\n",fseq);
};

void TMaroc::SetHeaderTime(unsigned char  header_time)
{
	ftimer	= header_time; 
	MRC_DUM("TIMER TAG: %3d\n",ftimer);
};

void TMaroc::SetGeoaddress(unsigned char  geoaddress)
{
	fgeo = geoaddress; 
	MRC_DUM("GEO ADDRESS = %d\n",geoaddress);
}; 

void TMaroc::SetTrailerNwords(unsigned short trailer_nwords)
{
	frecord = trailer_nwords; 
	MRC_DUM("Trailer NWord = %d\n",trailer_nwords );
};


int TMaroc::Simulate(TMaroc * fe)
{
	fe->SetGeoaddress(63);// attenzione cosi mode == 0
	fe->SetHeaderSeq(22);
	fe->SetHeaderTime(33);
	
	fe->SetHit(2);
	fe->SetHit(4);
	fe->SetHit(6);
	
	fe->SetData(1,1100);
	fe->SetData(3,1300);
	fe->SetData(5,1500);
	fe->SetData(7,1700);
	fe->SetData(9,1900);
	
	fe->SetTrailerNwords(6);
	fe->SetACQmode(0x00); 
	return 0;
};

//----------------------------------------------------------------------------------------------------------------------------------------------//
// PRINT:  	 
// According to thr,code and fmode (transfer mode read from raw trailer word), data are printed.
// default values are zero.
// fmode correctness is checked against effective data word read.
//---------------------------------------------------
// 1 - Trailer mode check. Possible values are: 0 ALL DATA, 1 HIT, 2 OVER THR (sparse readout), 3 NO DATA
// 2 - Threshold of printing check, 
// 3 - Headline Printing ( according to Trailer mode )
// 4 - DataPrinting ( according to lthr and lcode) if lcode is negative print nothing, if thr is negative print a warning
//---------------------------------------------------
// Data are showed according to the following encoding of (l)code 

// NONE  x000 -> 0x00 Print(0)
// HIT   x001 -> 0x01 Print(1)
// THR	 x010 -> 0x02 Print(2,<thr)
// ALL	 x011 -> 0x03 Print(3)
// THR	 x100 -> 0x04 Print(4,>thr)
// HT	 x101 -> 0x05 Print(5) // only header and trailer information		
//-----------------------------------------
// Returned values
//  0 OK
// -1 bad parameter choice
// -2 forbidden value of fmode
//----------------------------------------------------------------------------------------------------------------------------------------------//

void TMaroc::Print()
{	 	
	printf("GEO= %2d,SEQ= %3d,TIMER= %3d,MODE= %d,REC= %d\n", fgeo,fseq,ftimer,fmode,frecord);
	printf("ADC MAP\n");
	for (int i=0; i<FECHNUM; i++) {
		printf(" %4d ",fdata[i]);
		if ((i%8)==7) {
			printf("\n");
		}
	}
	printf("HIT MAP\n");
	for (int i=0; i<8; i++) {
		printf("%02X",fhitmap[i]);
	}
	printf("\n");
};

