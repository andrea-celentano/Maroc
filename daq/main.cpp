/**
 * Applicazione: SPECT CONTROLLER
 * Descrizione : Acquisizione Dati (DAQ)
 *				 configurabile (config.txt)		       
 *																  
 * First version:1 Luglio 2010
 * Linux porting June 2012
 * Mac porting: December 2012 
 * 
 * TO DO:
 * 
 *  MRC2 configuration 
 *  Revision to meake it more elegant
 *	Documentation 
 *
 *
 *
 * Installation:
 *
 * on linux after make, do:
 *		g++ -o daq main.cpp -L./ -lmrc -L/usr/lib/x86_6-linux-gnu -lusb-1.0 -L./ -lmrc
 * on mac 
 *		g++ -o daq main.cpp -L./ -lmrc -lusb-1.0 -L/usr/local/lib -lconfig++
 */

//#include <string.h>		/* included somewhere else*/
//#include <stdio.h>
#include <signal.h>
//#include <iomanip>
//#include <iostream>
//#include <cstdlib>
//#include <unistd.h>
//#include <time.h>	/* 	time_t, struct tm  */

#define Wait(msec) usleep( msec * 1000 )

#include "MRC_Common.h"
#include "MRClo.h"		
#include "MRChi.h"
#include "MRCsv.h"
#include "GI_Config.h"


using namespace std; // e.g signal.h for interrupt from keybard
//using namespace libconfig;

#define MRC_RUN_NUMBER_FILE		"cfg/last.run"
#define DEFAULT_CONFIG_FNAME	"./config_maroc.txt"

#define MAX_EVENT_DATA			4096*3
#define DEF_IACK_DELAY			2 // max 7 (3 bit)   Actual Delay = 25ns * DEF_IACK_DELAY + 50 ns. 
#define MAX_STREG_TOUT			1000 



/* Signal handling */
static int sig_int = 1;
void ctrl_c(int);

void readDAQData( unsigned short n_words, bool multievent_daq, bool debug,MRCsv * fMsv); // Read Data from Electronics
void writeRawData(unsigned short n_words,FILE *fout);// Write Raw Data on a file


FILE * openOutFile(const char *runfile,	std::string file_prefix,GI_Config * cfg);

int writeRunNum(int rr, const char *runfile);
int readRunNum(const char *runfile);


// not used

void pollingWait();


// Global variables (used by main and other functions)

char		gNomeFileConfig[200];				// Main, OpenOutFile
unsigned char	daq_data_vector[MAX_EVENT_DATA*4];	// ReadDAQdata, WriteRawData
MRClo *fMlo;										// Everywhere, is the driver!




int main(int argc, char *argv[]){
		
	bool	dump	= false; // to be assigned to config file
	bool	success;

	//	Input Configuration 
	//_______________________________
	char	config_fname[100];
	if (argc==1){
		strcpy(config_fname,DEFAULT_CONFIG_FNAME);
	}else{
		strcpy(config_fname,argv[1]);
	}
	MRC_INF("Configuration file %s\n",config_fname);
	
	GI_Config *cfg = new GI_Config();
	cfg->parseInline(argc, argv);
	cfg->parseFile(config_fname);
	cfg->insertInline();
	

	int lret;
	int me= cfg->get<int>(lret,0,"multievent",0,"cb");	 
	if (me==1) {MRC_INF("Single Event Mode\n");}
	if (me>=2) {MRC_INF("Multi Event Mode = %d\n",me);}
	

	bool multievent_daq = (me>=2) ? true : false	;
	
	// SuperVisor (Duration, ProgressBar, Rate meter)
	//_______________________________
	
	MRCsv * fMsv = new MRCsv(); 	
	fMsv->MeasureTimeStart();
	
	// Wake up the Control Board 
	//_______________________________
	
	fMlo = new MRClo();
	
	// read device id mask for multi ctrl boards
	int ret;
	unsigned long did= cfg->get<int>(ret,0,"did",0,"cb");	 
	MRC_MSG("Device Identification Mask = 0x%lx\n",did);
	
	// Open USB Channel
	if(!fMlo->SpectOpen(did)){ MRC_ERR("Cannot open USB channel. Exiting... (%d)\n",0);return 1;}
  
	// Cleanup Controller
	success = fMlo->SpectCtrlAllClear();
	if(!success){MRC_ERR("USB error during SpectCrtlAllClear() %d Exiting...\n",-1);return 1;}
  
	// Check and Configure Front End Board(s)
	//_______________________________
	
	MRChi *mhi = new MRChi(fMlo);
  
	mhi->fe_discovery_fast();	//  set maroc_count
	if( mhi->getCount() == 0 ){ printf("No Maroc FrontEnd Cards Found. Exiting...");return 1;}

	// Cleanup Controller 
	success = fMlo->SpectCtrlAllClear(); 
	if( !success ){printf("USB error during SpectCrtlAllClear() - 1 ... Exiting...\n");return 1;}

	// FE Configuration
	mhi->ConfigureFrontEnd(cfg,(int) mhi->getCount());

	// Dummy Operation with ADC
	if (!mhi->fe_dummy_acq()) {printf("... Exiting...\n");return 1;}
	
	// Cleanup 
	success = fMlo->SpectCtrlAllClear();
	if( !success ){printf("USB error during SpectCtrlAllClear() - 2 ... Exiting...\n");return 1;}
  
	
	//Controller Configuration 
	//_______________________________
	
	// Firmware Revision Check
	unsigned char	Ctrl_FwRevision;
	success = fMlo->SpectGetCtrlRevision(&Ctrl_FwRevision);
	if( !success ){printf("USB error during SpectGetCtrlRevision()... Exiting...\n");return 1;}
	MRC_MSG("Control Board FPGA revision: %d\n", Ctrl_FwRevision);
  
	
	// FE MASK (32+32 bit)
	unsigned long	fe_mask[2];							
	fe_mask[0] = fe_mask[1] = 0;
	int		front_end;
	for(int i=0; i<mhi->getCount(); i++){
		front_end = mhi->getBaseAddr(i) >> 4;
		if( front_end < 32 ){
			fe_mask[0] |= (1 << front_end);
		}else{
			fe_mask[1] |= (1 << (front_end -32));
		}
	}
	success = fMlo->SpectCtrlWrite(FE_ENABLED_MASK_REG0_ADDR, fe_mask[0]);
	if( !success ){printf("USB error during SpectCtrlWrite(FE_ENABLED_MASK_REG0_ADDR)... Exiting...\n");return 1;}
	
	success = fMlo->SpectCtrlWrite(FE_ENABLED_MASK_REG1_ADDR, fe_mask[1]);
	if( !success ){printf("USB error during SpectCtrlWrite(FE_ENABLED_MASK_REG1_ADDR)... Exiting...\n");return 1;}
  
	
	// TRIGGER TYPE
	success = fMlo->SpectSetTrgType(0);
	if( !success ){printf("USB error during SpectSetTrgType()... Exiting...\n");return 1;}

	// IACK DELAY
	success = fMlo->SpectSetIackDelay(DEF_IACK_DELAY);
	if( !success ){printf("USB error during SpectSetIackDelay()... Exiting...\n");return 1;}
  
	// Soft Cleanup of the FE boards
	success = mhi->fe_soft_reset();
	if( !success ){printf("USB error during fe_soft_reset()... Exiting...\n");return 1;}
  
	// Run Configuration 
	//_______________________________
	
	// Event Buffer size (Control bard FPGA)
	success = fMlo->SpectEnableDaq(me); if( !success ){MRC_ERR("USB error... Exiting...\n");return 1;}
    
	// name
	std::string varstring;	
	varstring = cfg->get<std::string>(ret, 0, "file_prefix",0,"run");
	//printf("Name: %s\n",varstring.c_str());
	
	// Presets on time and event
	unsigned long event_preset = cfg->get<int>(ret,0,"event_preset",0,"run");
	printf("Preset: %ld [evt] or ",event_preset);
	unsigned long time_preset_sec = cfg->get<int>(ret,0,"time_preset_sec",0,"run");
	printf("%ld [s], Rate = %d [Hz]\n",time_preset_sec,(int)((float)event_preset/(float)time_preset_sec));

	fMsv->SetEventPreset(event_preset);
	fMsv->SetTimePreset(time_preset_sec);
	
	unsigned long maxEventInFifo = (me > event_preset) ? event_preset : me;	
	
	signal(SIGINT,ctrl_c); // if receives ctrl-c from the keyboard execute ctrl_c 
	fflush(stdout);
	
	//  Controller (Re)Configuration  
	//_______________________________
	
	// Why do we reconfigure Controller here?
	success = fMlo->SpectDisableGate();
	success = fMlo->SpectCtrlAllClear();
	success = fMlo->SpectCtrlWrite(FE_ENABLED_MASK_REG0_ADDR, fe_mask[0]);
	success = fMlo->SpectCtrlWrite(FE_ENABLED_MASK_REG1_ADDR, fe_mask[1]);
	success = fMlo->SpectSetTrgType(0);
	success = fMlo->SpectSetIackDelay(DEF_IACK_DELAY);
	success = mhi->fe_soft_reset();
    
	if( multievent_daq ){
		success = fMlo->SpectEnableDaq(maxEventInFifo);
	}else{
		success = fMlo->SpectEnableDaq(1);
	}
	Wait(10);

	FILE *	fout;
	fout = openOutFile(MRC_RUN_NUMBER_FILE,varstring,cfg);
		
	Wait(10);

	success = fMlo->SpectRestartDaq();//if( !success ){printf("USB error during SpectRestartDaq() in the extern loop... Exiting...\n");return 1;}
	success = fMlo->SpectEnableGate();//if( !success ){printf("\nUSB error during SpectEnableGate()... Exiting...\n");return 1;}
	
	/************/
	/* Pre Run  */
	/************/
	
	bool	fifo_full_flag;
	bool	daq_done_flag;
	bool	flag_data_ready;
	
	int	StatusReg_tout;
	int	StatusReg_tout_counter = 0;
	
	int     nEvents;
	unsigned short	n_words;

	fMsv->StartTimer();
	
	/********************/
	/*  Run, max speed! */
	/********************/
	do { // loop on event and time (acquistion)	
		StatusReg_tout = 0;
		n_words = 0;
		nEvents = 0;
		if( multievent_daq ){
			success  = fMlo->SpectGetNEvents(&nEvents);
			success |= fMlo->SpectGetDaqFifoFullFlag(&fifo_full_flag); 
		
			// NEW
			//success = fMlo->SpectDisableGate();
			//success = mhi->fe_soft_reset();
			//success = fMlo->SpectEnableGate();//if( !success ){printf("\nUSB error during SpectEnableGate()... Exiting...\n");return 1;}
			// END NEW
			
			if( !success ){MRC_ERR("USB error while getting N events... Exiting...\n");return 1;}
			flag_data_ready = !( (fifo_full_flag == false && nEvents < maxEventInFifo )&& success);	
		}else { 
			success = fMlo->SpectGetDaqDoneFlag(&daq_done_flag);
			nEvents = 1;
			StatusReg_tout++;
			if( !success ){printf("\nStatusReg_tout is %d Exiting...\n", StatusReg_tout);return 1;}
			flag_data_ready = !( daq_done_flag == false && success && StatusReg_tout < MAX_STREG_TOUT );
		} 
      
		if (flag_data_ready){	
					
			fMsv->IncrCountDataReady();
			if( StatusReg_tout == MAX_STREG_TOUT )
			{ //timeout
				printf("Time Out\n");
				StatusReg_tout_counter++;
				success = true;
				//success = reinit_Ctrl_and_Fe();
				if( !success ){printf("USB error during system reinitialization... Exiting...\n");return 1;}
			}else{ // Event Available 
				success = fMlo->SpectDisableGate();
				if( !success ){printf("USB error during SpectDisableGate()... Exiting...\n");return 1;}
				success = fMlo->SpectGetDaqNwords(&n_words);	// printf("nEvents / nwords = %d %d\n", nEvents, n_words);
				success = fMlo->SpectGetNEvents(&nEvents);  // da rimuovere quando il programma e' ben testato  //printf("Nev = %d,flag = %d, nwords = %d\n", nEvents, fifo_full_flag, n_words);
				//printf("Eventi %d\n",nEvents);
				if( !success ){printf("USB error during getting N words... Exiting...\n");return 1;}
				if( n_words <= MAX_EVENT_DATA ){	
					readDAQData( n_words, multievent_daq, dump,fMsv);
					writeRawData(n_words, fout);
				}else{ 
					//success = fMlo->SpectRestartDaq();
					if( !success ){printf("\nUSB error during SpectRestartDaq() ...n_words <= MAX_EVENT_DATA is false. Exiting...\n");return 1;}
				} 
				fMsv->IncrEvt(nEvents);
				
				success = fMlo->SpectEnableGate();
				if( !success ){printf("\nUSB error during SpectEnableGate() just after ReadDAQData... Exiting...\n");return 1;}  
			}//END di event available
		}//END	flag data ready
		fMsv->IncrCountPolling();	
	}while (fMsv->AcqRunning() && sig_int); // (any more time/events?,any ctrl-c pressed?)
    

	//--- read  last events (if any, useful in multievent)
	success = fMlo->SpectDisableGate();
	success = fMlo->SpectGetDaqNwords(&n_words);
	success = fMlo->SpectGetNEvents(&nEvents);  // da rimuovere quando il programma e' ben testato
	
	fMsv->IncrEvt(nEvents);
	
	//fMsv->PrintTail(n_words);
	
	if( (n_words <= MAX_EVENT_DATA) && (n_words>0)){
		readDAQData( n_words, multievent_daq, dump,fMsv);
		writeRawData(n_words, fout);
		fMsv->IncrCountTailTransfer();
	} 
	
	fMsv->StopTimer();
	fMsv->CalculateRate();
	
	fMsv->MeasureTimeStop();
	
	//fMsv->PrintProfiling();
	
	fMsv->AddData(cfg);

	cfg->save(gNomeFileConfig);
	
	fclose(fout);

	// Cleanup 
	success = fMlo->SpectCtrlAllClear();
	if( !success ){printf("USB error during SpectCtrlAllClear() - 3 ... Exiting...\n");return 1;}
	fMlo->SpectClose();
	
	delete fMlo;
	delete fMsv;
	
	return 0;
} // end of main






//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

void ctrl_c(int){
  sig_int = 0;
  printf("\n CTRL-C pressed \n\r");
}



void readDAQData( unsigned short n_words, bool multievent_daq, bool debug,MRCsv * fMsv){
	
	bool lsuccess;
	
	char local_channel= 19;
	
	if( multievent_daq ){
	
		lsuccess = fMlo->SpectDaqRead_LONG(daq_data_vector, n_words);
	
	}else{
		
		lsuccess = fMlo->SpectDaqRead(daq_data_vector, n_words);
	}			
  
	if( !lsuccess ){printf("\nUSB error during daq data reading: at words %d of event %ld. Exiting...\n", n_words, fMsv->GetEvents());exit(1);}
    
	if( debug ){
    
		for(int y=0; y<(n_words*4); y+=4){
      
			if((((daq_data_vector[y+3]&0xF0)>>4)|((daq_data_vector[y]&0X03)<<4))==local_channel){
	
				printf("Event %ld, Word %2d = 0x%02X%02X%02X", fMsv->GetEvents(),y/4,daq_data_vector[y], daq_data_vector[y+3], daq_data_vector[y+2]);
	
				if( (daq_data_vector[y] & 0xC0) == 0x00 ){ // dataword
	  
					printf(" [ch=%02d;",((daq_data_vector[y+3]&0xF0)>>4)| ((daq_data_vector[y]&0X03)<<4));
	  
					printf(" ADC=%4d;",((daq_data_vector[y+2])|((daq_data_vector[y+3]&0x0F)<<8)));
	  
					if((daq_data_vector[y]&0x04)==0x04){
	    
						printf(" hit = 1 ]\n");
	  
					}else{
	    
						printf("]\n");
	  
					}
	
				}// end of data word
	
				if( (daq_data_vector[y] & 0xC0) == 0x80 ){ // header word
	  
					printf("\n");
	
				}//end of header
	
				if( (daq_data_vector[y] & 0xC0) == 0xC0 ){ // trailer word
	  
					printf("\n");
	
				}//end of trailer 
      
			}//end of if on channel
    
		}//end of for			
 
	}// end of debug

  
	lsuccess = fMlo->SpectRestartDaq();			
  
	if( !lsuccess ){			
    
		printf("\nUSB error during SpectRestartDaq() in ReadData(...)  Exiting...\n");exit(1);			
  
	}				
 
}



void writeRawData(unsigned short n_words,FILE *fout)
{
	int zz;
	fwrite(&n_words, sizeof(unsigned short), 1, fout);	
	fwrite(daq_data_vector, sizeof(unsigned char), n_words*4, fout);		
	fflush(fout);
}





int readRunNum(const char *runfile) {
	FILE *ff;
	int rr;
	ff = fopen(runfile,"r");
	if (ff) {
		fscanf(ff,"%d",&rr);
		fclose(ff);
	} else {
		rr = -1;
	}
	return rr;
}

int writeRunNum(int rr, const char *runfile) {
	FILE *ff;
	ff = fopen(runfile,"w");
	if (ff) {
		fprintf(ff,"%d",rr);
		fclose(ff);
	} else {
		printf("WARNING: cannot update run file %s",runfile);
	}
	return 0;
}

FILE * openOutFile(const char *runfile,	std::string file_prefix,GI_Config * cfg)
{
	char nomefile[200]; // stringa che ospita il nome del file di output
	//char nomefileconf[200]; // stringa che ospita il nome del file di configurazione di output
	//char *mode;			// stringa che contiene la modalit‡ di accesso al file
	FILE *fp;
	
	int irun = readRunNum(runfile)+1;
	writeRunNum(irun, runfile);

	sprintf(nomefile,"../data/out/%s_%05d.bin",file_prefix.c_str(),irun);
	//sprintf(nomefileconf,"../out/%s_%05d.txt",file_prefix.c_str(),irun);//// May 10th 2013 moved at the end of acq application to store the number of events really acquired
	sprintf(gNomeFileConfig,"../data/out/%s_%05d.txt",file_prefix.c_str(),irun);
	
	//cfg->save(nomefileconf); // May 10th 2013 moved at the end of acq application to store the number of events really acquired

    if ( (fp=fopen(nomefile, "wb")) == NULL ){
		MRC_ERR("Cannot open output file %s\n",nomefile);
		exit(1);
	}
	printf("Output binary file %s opened\n",nomefile);
	return fp;
}
void pollingWait(){
	//Sleep(polling_wait);
}


