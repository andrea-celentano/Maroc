
// Gestione Info provenienti dal bus VME
// numero massimo di canali del VME = 10

#ifndef TVME_H
#define TVME_H

#include <stdio.h>

class TVME
{
private:
		unsigned long	fndata;			// ex vme_ndata  (EX global)  o v_word   (locale del metodo costruttore)
		unsigned long	fscaler;		// ex vme_scaler (EX global)  o v_scaler (locale del metodocostruttore)
		unsigned long   fdata   [10];	// ex vme_data   (EX global)

				 int	fnch;			// ex nch
				 int	fadc    [10];	// ex v_adc
		unsigned int	fcount  [10];	// ex v_count
		unsigned int	fchannel[10];	// ex v_cha
		
		

public:
		TVME	();
	
	void SetNData	(unsigned long ndata);
	void SetScaler	(unsigned long scaler);
//	void SetData	();//to be defined		

	unsigned int	GetChannels	();			// return the number of channels available in the current event	
	unsigned long	GetScaler	();			// return the VME-master scaler
	unsigned long	GetNData	();			// ex GetWord	 

			 int	GetADC		(int i );	// return the ADC value in the same order provided by the VME (not necessarely the channel of the module)
			 int	GetADCch	(int ch);	// return the first ADC value ordered by ADC channel number
	unsigned int	GetCount	(int i );
	unsigned int	GetCountch	(int ch);
	         
private:
	void SetCode(){}

};


#endif