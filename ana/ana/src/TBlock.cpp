#include"TBlock.h"
TBlock::~TBlock()
{	// all pointers must be resetted!
	for(int i=0; i<fmarocN; i++)
	{
		delete ffront_end[i];
	}
	if (fvme)
	{
		delete fvme; 
	}
	MRC_DBG("Instance destroyed\n");
	
};

TBlock::TBlock(unsigned char * daq_data_vector,unsigned short nword, TEvent * Event,int &idx_first,	CNSAnalysis	* CNSAnalizer)
{	
	fmarocN = 0;
	feventN	= 0;
	fvme =0;
	
	int		lgroupsize		=	0;		// CNS filter
	int		lchannel		=	0;		// FE channel
	int		ladc			=	0;		// ADC
	bool	lhit			= false;	// HIT
	Short_t lchannelsys		=	0;		// System Channel
	Short_t ldatum			=	0;		// Tree element (HIT[14]+ADC[11..0] )
	Float_t lpedestal		= 0.0;
	int		idx_current		= idx_first;
	int		lprevious_feID	=  -1;	
	int		lcurrent_feID   =	0;		
	
	if (CNSAnalizer) {
		lgroupsize		= CNSAnalizer->GetGroupSize();
		//printf("CNS filter enabled lgroupsize %d \n",lgroupsize);
	}
	
	
	//printf("New Block! idxfirst = %d\n",idx_first);
	for(int i=0; i<((int)nword*4); i+=4)
	{
		/***************/
		/* HEADER WORD */
		/***************/
		
		if( (daq_data_vector[i] & IDMASK) == MRC_HEADER_MASK ){// HEADER WORD
			ffront_end[fmarocN] = new TMaroc;
			ffront_end[fmarocN]->SetGeoaddress(daq_data_vector[i]&0x3F);
			ffront_end[fmarocN]->SetHeaderSeq (daq_data_vector[i+3]);
			ffront_end[fmarocN]->SetHeaderTime(daq_data_vector[i+2]);
			// check for new event 
			lcurrent_feID   = (int) daq_data_vector[i]&0x3F;
			//printf("current/prev feID = %d/%d\n",lcurrent_feID,lprevious_feID);
			
			if (lcurrent_feID <= lprevious_feID){// new event in MultiEvent Buffer Mode
				if(lprevious_feID!=-1){ // new event!
					// Recording previous event in the Tree
					Event->SetEvtIdx(idx_current);
					Event->SetStatusRegister(0);
					
					if (lgroupsize>0) 
					{
						this->CommonNoiseFilter(Event,CNSAnalizer);
						
					}
					
					Event->FillTree();
					Event->Print();
					Event->Clean();
					feventN++;
				//	printf("FINE EVENTO %d\n",idx_current);
					idx_current++;
					lprevious_feID = lcurrent_feID;
				}
				
			}
			lprevious_feID = (daq_data_vector[i]&0x3F);
		}
		
		/*************/
		/* DATA WORD */
		/*************/		
		if( (daq_data_vector[i] & IDMASK) == MRC_DATA_MASK ){	// DATA WORD
			lchannel  = (daq_data_vector[i]&0x3)*16 + ((daq_data_vector[i+3]&0xF0)>>4);
			ladc = daq_data_vector[i+2]  + (daq_data_vector[i+3]&0x0F) * 256;	
			lhit=false;
			if((daq_data_vector[i]& MRC_HIT_BIT)==MRC_HIT_BIT){
				lhit = true;
				ffront_end[fmarocN]->SetHit(lchannel);
			}
			ffront_end[fmarocN]->SetData(lchannel, ladc);
			lchannelsys = Event->GeoCh2AbsCh((int)ffront_end[fmarocN]->GetGeoaddress(),lchannel); 
			lpedestal	= 0.0;
			ldatum		= Event->ComposeDatum(ladc,lpedestal); // generazione Dato per Tree a partire da ADC e HIT e pedestal
			Event->SetADC(lchannelsys, ldatum); // Writing analog and digital data on Tree related variable
			if(lhit){
				Event->SetHit(lchannelsys);
				Event->IncrEvtMultiplicity();
			}
		}
		
		/***************/
		/* TRAILER WORD */
		/***************/
		if( (daq_data_vector[i] & IDMASK) == MRC_TRAILER_MASK ){// TRAILER
			ffront_end[fmarocN]->SetTrailerNwords(daq_data_vector[i+2]+daq_data_vector[i+3]*256);	// Allora assegna al campo trailer_nwords N_PAROLE_TOTALE [7..0]+ TUTTI ZERI ?????
			ffront_end[fmarocN]->SetACQmode(daq_data_vector[i]);
			fmarocN++; // note that if no trailer is found the index is not raised up and // in this case Maroc data will be  overwritten during next loop!
		}
	}// end of a block
	
	
	//Last or Single Event Buffer Mode
	Event->SetEvtIdx(idx_current);
	Event->SetStatusRegister(LAST_OF_BULK);
	if (lgroupsize>0) 
	{
		this->CommonNoiseFilter(Event,CNSAnalizer);
	}
	
	Event->FillTree();
	//printf("Ultimo EVENTO* %d\n",idx_current);
	//Event->Print();
	//printf("Event ID = %5d; Status = %4d, Multiplicity = %4d\n",Event->GetEvt(),Event->GetStatusRegister(),Event->GetEvtMultiplicity());
	Event->Clean();
	idx_first = idx_current;
}

TBlock::TBlock(int tour,unsigned char * daq_data_vector, TEvent * Event, TFileRaw * FileStat)
{
	fmarocN = 0;
	feventN	= 0;
	fvme	= NULL;
	
	FileStat->ResetCardLevel();
	
	int		lchannel;	// FE channel
	int		ladc;		// ADC
	bool	lhit;		// HIT
	Short_t lchannelsys;// System Channel
	Short_t ldatum;		// Tree element (HIT[14]+ADC[11..0] )
	Short_t lfe;		// Front End Card ID
	Short_t lseq;		// Sequential Tag
	Short_t ltim;		// Timer Tag
	Float_t lpedestal = 0.0;

	int		lprevious_feID ;	// what have been recorded (by FileStat)
	int		lcurrent_feID;		// what I read	(from data buffer)

	int i;

	for(i=0; i<(FileStat->Get_NWord()*4); i+=4)
	{
		if( (daq_data_vector[i] & IDMASK) == MRC_HEADER_MASK )
		{// HEADER WORD
			ffront_end[fmarocN] = new TMaroc;
			ffront_end[fmarocN]->SetGeoaddress(daq_data_vector[i]&0x3F);
			ffront_end[fmarocN]->SetHeaderSeq (daq_data_vector[i+3]);
			//printf("IN HEADER read sequential: %d\n",ffront_end[fmarocN]->GetHeaderSeq());
	
			ffront_end[fmarocN]->SetHeaderTime(daq_data_vector[i+2]);

			// CHECK FOR NEW EVENT -----------------------------------------------------------------------------
			// Given that Header and Trailer of a FE card on BP are always present
			// and given that card reading is performed in incremental way
			// (e.g. card 15 always precede card 16)
			// a new event is recognized by a smaller than preceeding card ID.
			// Since FileRaw set current fe ID to -1 at the beginiing of a new block
			// a  further if loop is needed see

			lcurrent_feID   = (int) daq_data_vector[i]&0x3F;
			lprevious_feID  = FileStat->Get_Card_ID();

			if(lcurrent_feID<0|| lcurrent_feID>63){ // fe_ID must be strictly smaller than 64 and bigger or equal than 0
				printf("Error: wrong fe_ID\n"); // error
			}
			else{
				if (lcurrent_feID <= lprevious_feID){// new event | begining of block
					if(lprevious_feID!=-1){ // new event!
						printf("New Event (%3d)\n",feventN);
						lprevious_feID = lcurrent_feID;
						// work on previous event
						if(tour==0){FileStat->Compare_Event_Maxima();}
						if(tour==1){FileStat->FillE_NDword();}

						FileStat->Reset_event_DW_num();
						FileStat->IncrEvent();

						if(tour==0)
						{// Recording on Tree
							Event->SetEvtIdx(FileStat->GetEvent());
							Event->FillTree();
						//	Event->Print();
							Event->Clean();
							FileStat->ResetEventLevel();
						}
						feventN++;
					}
				}
			}
			//----------------------------------------------------------
			if(tour==0){//Tree related variables updating
				lfe   = ffront_end[fmarocN]->GetGeoaddress();
				lseq  = ffront_end[fmarocN]->GetHeaderSeq();
				ltim  = ffront_end[fmarocN]->GetHeaderTime();
				//printf("lseq= %3d\n",lseq);
				Event->SetTimer(lfe,ltim);
				Event->SetSequential(lfe,lseq);
			}
			// FE STATISTICS & PARSING CHECKS
			FileStat->IncrCount(ffront_end[fmarocN]->GetGeoaddress());	// aggiorna array del coinvolgimento delle FE Card
			FileStat->Header_Supervisor();							// aggiorna/azzera contatori, ricerca errori ed infine si pone nello stato H
			FileStat->Set_Card_ID(daq_data_vector[i]&0x3F);
		}


		if( (daq_data_vector[i] & IDMASK) == MRC_DATA_MASK )
		{	// DATA WORD
			lchannel  = (daq_data_vector[i]&0x3)*16 + ((daq_data_vector[i+3]&0xF0)>>4);
																					// Per ricostruire il canale considero il nibble meno significativo del Byte[i] traslandolo verso sinistra (Entrano zeri!)
																					// Attenzione! mi interessano soltanto i due bit meno significativi del nibble (0 0 MAROC_CH[5,4])
																					// E CONSIDERO l'intero nibble MS del Byte[i+3] (0xF0) (MAROC[3..0]) Shifttandolo di 4 bit verso destra.
																					// Sommando i Byte cosi ottenuti si ha: (00)MAROC_CH[54]MAROC_CH[3..0]

																					// Attenzione:nella nuova versione del firmware la DATA word viene modificata
																					// assegnando al bit adiacente a sinistra a MAROC_CH[5..4] il ruolo di HIT (segnale digitale di convalida.)

			ladc = daq_data_vector[i+2]  + (daq_data_vector[i+3]&0x0F) * 256;	// Per ricostruire il dato adc:
																					// Considero  tutto il Byte [i+2] ovvero ADC[7..0] ed il Nibble meno significativo (0x0F) del Byte [i+3] cioè ADC[11..8]
																					// Shifto a sinistra quest'ultimo di 8 posizioni e li sommo ottenendo 0000 ADC[11..8] ADC[7..0]


			// HIT
			if((daq_data_vector[i]&MRC_HIT_BIT)==MRC_HIT_BIT)
			{
				lhit = true;
				ffront_end[fmarocN]->SetHit(lchannel);
			}
			else
			{
				lhit = false;
			}

			// Writing on block buffer
			ffront_end[fmarocN]->SetData(lchannel, ladc);// Scrittura del dato e del canale ricostruiti sull'oggetto maroc_event corrispondente


			if(tour==0){

				lchannelsys = Event->GeoCh2AbsCh((int)ffront_end[fmarocN]->GetGeoaddress(),lchannel); // generazione Canale a partire sa geoaddress e canale SCHEDA

				//if(Calibration!=0){
				//	lpedestal = Calibration->GetPedestal(lchannelsys);
				//}
				//else{
					lpedestal=0.0;
				//}
			
				ldatum		= Event->ComposeDatum(ladc,lpedestal); // generazione Dato per Tree a partire da ADC e HIT e pedestal

				//printf(" datum = %d\n", ldatum); //debug
				Event->SetADC(lchannelsys, ldatum); // Writing analog and digital data on Tree related variable
				if(lhit){Event->SetHit(lchannelsys);}
			}
			//PARSING CHECKS
			FileStat->Data_Supervisor();
		}


		if( (daq_data_vector[i] & IDMASK) == MRC_TRAILER_MASK )
		{// TRAILER
			ffront_end[fmarocN]->SetTrailerNwords(daq_data_vector[i+2]+daq_data_vector[i+3]*256);	// Allora assegna al campo trailer_nwords N_PAROLE_TOTALE [7..0]+ TUTTI ZERI ?????
			ffront_end[fmarocN]->SetACQmode(daq_data_vector[i]);

			//if(tour==0){ffront_end[fmarocN]->Print(); } //data are printed immediatly after trailer reading
			
			if(ffront_end[fmarocN]->GetTrailerNwords()==2) {FileStat->IncrEmptyCount(ffront_end[fmarocN]->GetGeoaddress());}//Empty card!
			if ( FileStat->Get_card_DW_num()+2 != ffront_end[fmarocN]->GetTrailerNwords() ){FileStat->Incr_Record_Flimsiness();}// Record Consistency Check

			fmarocN++; // note that if no trailer is found the index is not raised up and // in this case Maroc data will be  overwritten during next loop!

			FileStat->Trailer_Supervisor();//PARSING CHECKS

			if(tour==1){FileStat->UpDateCardStat();}
		}
	}// for	on DWord of daq_data_vector


	// End of a Block
	//printf("End of a Block (composed by %d events )\n",feventN);
	if (tour==0)
	{	// Last of Block Event managing
		if (FileStat->Get_event_DW_num()!=0)
		{
			FileStat->IncrEvent();
		}
		FileStat->Compare_Event_Maxima();

		Event->SetEvtIdx(FileStat->GetEvent());
		Event->SetStatusRegister(LAST_OF_BULK);
		Event->FillTree();
		//Event->Print();
		Event->Clean();
		FileStat->ResetEventLevel();
	}
	if(tour==1){FileStat->FillE_NDword();}

	feventN++;	// ed incrememta il contatore di 'eventi presenti nel blocco'

	FileStat->Set_Block_NEvent(this->GetNEvents());

	// FINITO DI LEGGERE IL BUFFER DAQ_DATA_VECTOR, PASSO A LEGGERE VME
	//	fvme = new TVME;
};

//void TBlock::CommonNoiseFilterExcludingMax(TEvent * Event, int subsetsize,CNSAnalysis * CNSAnalizer) to be implemented
void TBlock::CommonNoiseFilterExcludingMax(TEvent * Event, int subsetsize)
{	
	int		cnoise[4096]; 
	int		channel =0; 
	double	mediaA =0.0;
	double	mediaB =0.0;			
	int		massimoA = 0;
	int		zstarA=0;
	int		*buf;
	buf = new int[subsetsize];
	for (int i=0; i<subsetsize; i++) {buf[i]=0;}
	int		t=0; // inizio del buffer
	int		s=0;
	for (channel=0; channel<4096; channel++) {
		cnoise[channel]=0;
		// Calcolo della media con esclusione del sample piu' grande
		buf[t] = Event->GetADC(channel);
		t++;
		if ((channel%subsetsize)==(subsetsize-1)) {// fine del buffer
			t=0;
			// subset ready for analysis
			mediaA =0.0;
			mediaB =0.0;			
			massimoA = 0;
			zstarA=0;
			for (int z=0; z<subsetsize; z++) {
				//	printf("Channel %d,buf[%d] = %d (%3.0f)\n",channel-subsetsize+z+1,z,buf[z],Event->GetADC(channel-subsetsize+z+1));
				// evaluate mean
				mediaA = mediaA + buf[z];
				if (buf[z]>=massimoA) {
					massimoA = buf[z];
					zstarA = z;
				}
			}
			//printf("mediaA = %3.0f\n",(mediaA/subsetsize));
			//printf("massimoA = %d at buf[%d]\n",massimoA,zstarA);
			for (int z=0; z<subsetsize; z++) {
				if (z!=zstarA) {
					mediaB = mediaB+buf[z];
				}
			}
			//printf("mediaB = %3.0f\n",(mediaB/(subsetsize-1)));
			mediaA = mediaA/subsetsize;
			mediaB = mediaB/(subsetsize-1);
			//printf("diff = %.0f\n",mediaA-mediaB);
			s= 0;
			// scrittura del vettore di rumore
			for (int z = channel; s<subsetsize; z--) {
				cnoise[z]= mediaB;
				//	printf("CN[%d](%2d) = %.0f\n",z,s,mediaB);
				s++;
			} // loop on subset
		} 
	} // loop on channels
	Short_t dato =0;
	for (channel=0; channel<4096; channel++) {
		if (cnoise[channel]!=0) {
			dato = (Short_t)(((int)Event->GetADC(channel)))-cnoise[channel]+OFFSET;
			//	printf("FE%02d(%d) : %d - %d  = %d\n",channel/64,channel%64,(int)Event->GetADC(channel),cnoise[channel],dato);
			Event->SetADC(channel,dato); 
		}
	}
	delete buf;
}

void TBlock::CommonNoiseFilter(TEvent * Event, CNSAnalysis * CNSAnalizer)
{
	int		cnoise[4096]; 	
	int		channel =0;		
	int		average=0; 
	
	int		lsubsetsize = CNSAnalizer->GetGroupSize();
	
	
	int		c; // channel index inside a subset
	int		q; // channel index
	
	for (channel =0; channel<4096; channel++) {
		cnoise[channel]=0;
		average = average+Event->GetADC(channel);
		if ( (channel%lsubsetsize)==(lsubsetsize-1) ) {
			//printf("Average = %4d (ADC),Canale %4d\n",average/lsubsetsize,channel);

			if (average!=0) {
				CNSAnalizer->Add(average/lsubsetsize,(channel%64)/lsubsetsize);   
			}
			for (q=channel; c<lsubsetsize; q--) {
				cnoise[q]=average/lsubsetsize;
				if (average!=0) {
				//	printf("CN[%d](%d) = %d\n",q,c,average/lsubsetsize);
				}
				
				c++;
			}
			c=0;
			average=0;
		}
	}
	Short_t dato =0;
	for (channel=0; channel<4096; channel++) {
		if (cnoise[channel]!=0) {
			dato = (Short_t)(((int)Event->GetADC(channel)))-cnoise[channel]+OFFSET;
		//	printf("FE%02d(%d) : %d - %d  = %d\n",channel/64,channel%64,(int)Event->GetADC(channel),cnoise[channel],dato);
			Event->SetADC(channel,dato);
		}
	}
}

