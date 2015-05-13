#include "TVME.h"

void TVME::SetNData(unsigned long ndata)	{	fndata  = ndata;	};
void TVME::SetScaler(unsigned long scaler){	fscaler = scaler;	};

unsigned int TVME::GetChannels()	{return fnch;};	
unsigned long TVME::GetScaler()	{return fscaler;};	
unsigned long TVME::GetNData()	{return fndata;};


TVME::TVME()
{

	//fndata = 0;
	//fscaler= 0;

	unsigned int cnt;
	unsigned int j=0;
	unsigned int j0=0;
	unsigned int j1;


	unsigned long i=0;
	int code;
	for (i=0;i<fndata;i++)
	{
		code = (fdata[i] >> 24) & 0x7;

		// se code viene uguale a 2 -> Header
		// se code viene uguale a 0 -> Data
		// se code viene uguale a 4 -> End of Block

		if (code == 2)
		{													// header
			cnt = (fdata[i] >> 8) & 0x3F;
			j0 = j;
printf("cnt %d\n",cnt);
		}

		if (code == 0)
		{													// data
			if (j < 10) //MAX_VME_CHANNEL
			{
				fadc [j] =  fdata[i] & 0xFFF;
				fadc [j] = (fdata[i] & 0x2000) > 0 ? -fadc [j] : fadc [j];   //? due istruzioni consecutive assegnano valore ad v_adc[j] ?
				fchannel[j] = (fdata[i] >> 17) & 0xF;
				j = j+1;
			}
			else
			{
				printf("ADC index too large ... skip next data in current event\n");
			}
		}
		if (code == 4)
		{									// end of block
			for (j1=j0;j1<j;j1++)
			{
				fcount[j1] = fdata[i] & 0xFFFFFF;
			}
		}
	} // end FOR on fndata
	fnch = j;
};

int TVME::GetADCch(int ch)
{
	int i;
	int val = 0;

	for (i=0;i<fnch;i++) {
		if ((ch = fchannel[i])) {
			val = fadc [i];
			break;
		}
	}
	return val;
};

unsigned int TVME::GetCountch(int ch)
{
	int i;
	unsigned int val=0;

	for (i=0;i<fnch;i++) {
		if ((ch = fchannel[i])) {
			val = fcount[i];
			break;
		}
	}
	return val;
};

unsigned int TVME::GetCount(int i)
{
	if(i<fnch){
		return fcount[i];
	}
	else {
		return 0;
	}
};


int TVME::GetADC(int i){
	if (i < fnch){
		return fadc[i];  
	}
	else{
		return 0;         
	}
};


