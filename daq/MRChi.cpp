
#include "MRChi.h"

MRChi::MRChi(MRClo *lo) {

	fMlo = lo;
	
	fFeNum = 0;

	for (int i=0;i<3;i++) {
		fe_config_register_vdd_fsb[i] = 0x0;
		maroc_config_cmd[i]		= 0x0;
		delay_config_cmd[i]		= 0x0;
		out_ch_config_cmd[i]	= 0x0;
		single_acq_cmd[i]		= 0x0;
		restart_auto_acq_cmd[i] = 0x0;
		sw_reset_cmd[i]			= 0x0;
		sw_clear_cmd[i]			= 0x0;
	};
	
	fe_config_register_vdd_fsb[0] = 0x01;	
	
	maroc_config_cmd[0]		= 0x01;
	delay_config_cmd[0]		= 0x02;
	out_ch_config_cmd[0]	= 0x04;
	single_acq_cmd[0]		= 0x08;
	restart_auto_acq_cmd[0] = 0x10;
	sw_reset_cmd[0]			= 0x20;
	sw_clear_cmd[0]			= 0x40;



	fSparseThrDef = 0xFFF;

	for (int i=0;i<64;i++) {

		fSparseThr[i] = fSparseThrDef;
		fRevisionFW[i] = 0;
		fASIC[i]=0;
	}


}

MRChi::~MRChi() {

}

int MRChi::ConfigLen(int mrc_asic_rev)
{
	if( mrc_asic_rev == 2 ){ //MAROC2
		return 69;
	}else{ //MAROC3
		return 105;
	}
}



// *** To be removed
unsigned long MRChi::IE_MakeDacWord(unsigned long thr, unsigned char maroc_revision) {

  unsigned long ldac; //64 bit
  
  if(maroc_revision==3){ // MAROC3
    ldac = thr; 
    
    MRC_DBG("Threshold DAC = 0x%lx -> %lf IrefDAC\n",thr, (thr%0x2FF)*0.03125);
    
  }

  if(maroc_revision==2){ // MAROC2
    unsigned long dac_coarse; // 16 bit "Presence" Coding (doesn't matter where, only how many is important!) 
    unsigned long dac_fine;   // 8 bit "Positional" Coding
    dac_coarse	= (thr >> 8);		// 0x00FFFFFF
    dac_fine	=   thr & 0xff;	// 0x  000000FF
    ldac = (((1 << dac_coarse) -1 ) << 8) | dac_fine; 
    
    
    MRC_DBG("IE_MakeDacWord(%4ld)\n",thr); 
    MRC_DBG("dac_coarse = %ld\n",dac_coarse);
    MRC_DBG("dac_fine   = %ld\n",dac_fine);
    MRC_DBG("1<<dac_coarse   = %d\n",1<<dac_coarse);
    MRC_DBG("(1<<dac_coarse)-1 = %d\n",(1<<dac_coarse)-1);

  }
  return ldac;
}



bool MRChi::fe_soft_reset(){	
  int n;
  bool success;
  for(unsigned int i=0; i<fFeNum; i++){
    n = 3;
    MRC_DUM(" Doing soft reset on maroc %d (addr = 0x%x)\n",i, fBaseAddr[i]);
    success |= fMlo->SpectWrite(fBaseAddr[i]+FEADDR_COMMAND, sw_reset_cmd, n);
  }
  return success;
}


bool MRChi::fe_configReg_write(){
  int n;
  bool success;
  for(unsigned int i=0; i<fFeNum; i++){
    n = 3;
    success = fMlo->SpectWrite(fBaseAddr[i]+FEADDR_CONFIG_REG, fe_config_register, n);
    if( fRevisionFW[i] == FE_REV2 ){
      n = 3;
      success = fMlo->SpectWrite(fBaseAddr[i]+FEADDR_CONFIG2_REG, fe_config_register2, n);
    }
  }
  return success;
}

bool MRChi::fe_discovery()	{	
	
  unsigned char	in_check[16][3];
  unsigned char	out_check[3];
  unsigned int	pflag[64], overlap[64];
  unsigned int	maddr;
  int n;
  int i; // loop on maroc in one BP 
  int j; // loop on BackPlane
  int k; // loop on conf reg in one FE
  unsigned int l;
  unsigned int max_trial = 50;
  bool success;
  
  srand (12434);

  for (j=0;j<4;j++) {  // cleanup
    for (i=0;i<16;i++) {
      pflag[i+j*16] = 0; 
      overlap[i+j*16] = 0xff; 
    }
  }
  printf("Discover FE cards, please wait 10 dash: ");
  for (l=0;l<max_trial; l++) {
    if ((l % (max_trial/10)) == 0){
      printf("#");
      fflush(stdout);
    }
    for (j=0;j<4;j++){ // loop on backplane
      for(i=0; i<16; i++){ // loop on maroc in one backplane (last is for "cleaning the bus")
	in_check[i][0]= (unsigned char) rand()*255; // VDD_FSB masked
	in_check[i][1]= (unsigned char) rand()*255;
	in_check[i][2]= (unsigned char) (rand()*255) & 0x3F; // To avoid LED ON
	n = 3;
	maddr = 0x100 * j + 0x10 * i;
	success = fMlo->SpectWrite(maddr+FEADDR_CONFIG_REG, in_check[i], n);
      }
      for(i=0; i<16; i++){
	n = 3;
	maddr = 0x100 * j + 0x10 * i;
	success = fMlo->SpectRead(maddr+FEADDR_CONFIG_REG, out_check, n);
	
	for (k=0; k<3; k++) {
	  if (in_check[i][k] == out_check[k]) {
	    pflag[i+16*j] += 1;
	  }
	  overlap[i+16*j] = overlap[i+16*j] & ~(in_check[i][k] ^ out_check[k]);
	}
	
      }
      // back write/read
      for(i=15; i>-1; i--){ // loop on maroc in one backplane (last is for "cleaning the bus")
	in_check[i][0]= (unsigned char) rand()*255; // VDD_FSB masked
	in_check[i][1]= (unsigned char) rand()*255;
	in_check[i][2]= (unsigned char) (rand()*255) & 0x3F; // To avoid LED ON
	n = 3;
	maddr = 0x100 * j + 0x10 * i;
	success = fMlo->SpectWrite(maddr+FEADDR_CONFIG_REG, in_check[i], n);
      }
      for(i=15; i>-1; i--){
	n = 3;
	maddr = 0x100 * j + 0x10 * i;
	success = fMlo->SpectRead(maddr+FEADDR_CONFIG_REG, out_check, n);
	//	printf("%1d/%1x W: 0x%02x %02x %02x R: 0x%02x %02x %02x\n",j,i,
	//			in_check[i][0],in_check[i][1],in_check[i][2],
	//			out_check[0],out_check[1],out_check[2]);			
	
	for (k=0; k<3; k++) {
	  if (in_check[i][k] == out_check[k]){
	    pflag[i+16*j] += 1;
	  }
	  overlap[i+16*j] = overlap[i+16*j] & ~(in_check[i][k] ^ out_check[k]);
	}
	
	
      } // for i END (FEs)
    } // for j END (BackPlane)
  } // end l (Max Trial)
  
  printf("\nSuccess count:\n");
  fFeNum = 0; 
  for (j=0;j<4;j++){
    printf("BP %1d c: ",j);
    for (i=0;i<16;i++) {
      printf("%3d ",pflag[i+16*j]);
      if (pflag[i+16*j] == max_trial*6) {
	fBaseAddr[fFeNum] = 0x100 * j + 0x10 * i;
	fFeNum++;
      }
    }
    printf("\n");
  }// END Loop on Bp 
  
  printf("Permanent success (bit-a-bit):\n"); // prints overlap
  for (j=0;j<4;j++){
    printf("BP %1d o: ",j);
    for (i=0;i<16;i++) {
      printf("%02x ",overlap[i+16*j]);
      if (pflag[i+16*j] == max_trial*6) {
      }
    }
    printf("\n");
  }// END Loop on Bp 

  if (fFeNum == 0) {
    MRC_ERR("*** NO CARD FOUND, check Backplane switches first (bus address) (%d) ***\n", 0);
    MRC_ERR("***                or try to unplug and re-plug the USB         (%d) ***\n", 1);
    MRC_ERR("***                or try POWER CYCLE HW AND RECONNECT USB      (%d) ***\n", 2);
  } else {
    printf("\n%d FE card(s) found, address(es) configured\n",fFeNum);
    for(i=0;i<(int)fFeNum;i++) {
      printf("MBA %2d : 0x%4x\n",i,fBaseAddr[i]); 
    }
  }
  printf("\n");

  for (j=0;j<4;j++){ // loop on backplane
    for(i=0; i<16; i++){ // loop on maroc in one backplane (last is for "cleaning the bus")

      RandomConfigVectors();
      maddr = 0x100 * j + 0x10 * i;
      if (WriteReadMarocVector(maddr, 105, maroc_config_data) >0) { // try v2
	WriteReadMarocVector(maddr, 69, maroc_config_data);
      }
    }
  }
  return success;

}


bool MRChi::fe_discovery_fast()	
{		
	MRC_MSG("Discover FE cards (fast method):\n");	
	unsigned int	pflag;
	unsigned int	maddr;
	int i; // loop on maroc in one BP 
	int j; // loop on BackPlane
	bool success;
	srand (12434);
	fFeNum = 0; 
	for (j=0;j<4;j++){ // loop on backplane
		MRC_DUM("Scanning Backplane %d\n",j);
		for(i=0; i<16; i++){ // loop on maroc in one backplane (last is for "cleaning the bus")
			pflag = 0;
			RandomConfigVectors();
			maddr = 0x100 * j + 0x10 * i;
			if (WriteReadMarocVector(maddr, 105, maroc_config_data) == 0) { // MRC3
				//pflag = 1;
				pflag = 3;
			} else {
				if (WriteReadMarocVector(maddr, 69, maroc_config_data) == 0) { // MRC2
					//pflag = 1;
					pflag = 2;
				}
			}
			//if (pflag == 1) {
			if (pflag != 0) {	
				// get firmware revision
				int nn = 3;
				unsigned char	StatusReg[3];
				maddr = 0x100 * j + 0x10 * i;
				success = fMlo->SpectFeRead(maddr+FEADDR_STATUS_REG, StatusReg, nn);
				if( !success ){
					MRC_ERR("USB error during Front-End FPGA revision check %d... Exiting...\n", maddr);
				} else {
					// ADDR, FW, ASIC 
					fRevisionFW[fFeNum] = StatusReg[2];
					fBaseAddr[fFeNum] = maddr;
					fASIC[fFeNum]=pflag;
					MRC_DUM("Discovered card %d at addr = 0x%3x with FW rev = %d and ASIC rev = %d\n",fFeNum, fBaseAddr[fFeNum],fRevisionFW[fFeNum],fASIC[fFeNum]);
					fFeNum++;
				}	
			}
		
		}
	}
	if (fFeNum == 0) {
		MRC_ERR("*** NO CARD FOUND, check Backplane switches first (bus address) (%d) ***\n", 0);
		MRC_ERR("***                or try to unplug and re-plug the USB         (%d) ***\n", 1);
		MRC_ERR("***                or try POWER CYCLE HW AND RECONNECT USB      (%d) ***\n", 2);
	} else {
		MRC_DUM("%d FE card(s) found, address(es) configured\n",fFeNum);
		for(i=0;i<(int)fFeNum;i++) {
			MRC_DBG("MBA %2d : 0x%4x [Firmware %d, MAROC %d]\n",i,fBaseAddr[i],fRevisionFW[i],fASIC[i]); 
		}
	}
	printf("\n");
	return true;
}

bool MRChi::MakeConfReg1(GI_Config *cfg ,int mrc_idx)
{
	//printf("MakeConfReg1\n");
	if (cfg == NULL) {
		MRC_ERR("Configuration object is NULL...Exiting\n");
		return false;
	}
	
	for (int i =0 ; i<3; i++) {
		fe_config_register[i]=0;
		//printf("ConfReg1[%d]=0x%X\n",i,fe_config_register[i]);
	}	
	int ret =0;
	
	// FW ver 1
	int auto_trigger	= cfg->get<int>(ret, 0, "auto_trigger"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int en_zero_supp	= cfg->get<int>(ret, 0, "en_zero_supp"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	
	// FW ver 1 and 2
	int en_auto_trig	= cfg->get<int>(ret, 0, "en_auto_trig"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int en_ext_trig		= cfg->get<int>(ret, 0, "en_ext_trig"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int test_en_eve		= cfg->get<int>(ret, 0, "test_en_eve"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int test_en_odd		= cfg->get<int>(ret, 0, "test_en_odd"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int hold1_delay		= cfg->get<int>(ret, 0, "hold1_delay"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int initial_out_ch	= cfg->get<int>(ret, 0, "initial_out_ch",0,"cfgReg1", mrc_idx, "fe",0,"cb");
	
	// FW ver 2
	int one_sixtyfour	= cfg->get<int>(ret, 0, "one_sixtyfour"	,0,"cfgReg1", mrc_idx, "fe",0,"cb");	 
	int led0			= cfg->get<int>(ret, 0, "led0"			,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	int led1			= cfg->get<int>(ret, 0, "led1"			,0,"cfgReg1", mrc_idx, "fe",0,"cb");
	
	if( fRevisionFW[mrc_idx] == FE_REV1 ){
		
		fe_config_register[0] =	(         auto_trigger & 0x01) << 0 | 
								(          test_en_eve & 0x01) << 1 |
								(		   test_en_odd & 0x01) << 2 |
								(		   hold1_delay & 0x1F) << 3;   
		
		fe_config_register[1] =	(   (hold1_delay >> 5) & 0x07) << 0 |  
								(	   (initial_out_ch & 0x0F) << 4);
		
		fe_config_register[2] =	(( initial_out_ch >> 4) & 0x07)  << 0 | 
					(	  en_auto_trig & 0x01) << 3 |
					(	   en_ext_trig & 0x01) << 4 |
					(	  en_zero_supp & 0x01) << 7;
	}else {	// V2
		fe_config_register[0] =	(           1 & 0x01)  << 0 |// VDD_FSB = 0 for normal operation /
					( test_en_eve & 0x01) << 1 |
					( test_en_odd & 0x01) << 2 |
					( hold1_delay & 0x1F) << 3;
		
		fe_config_register[1] =	(  (hold1_delay >> 5) & 0x07) << 0 |
					(                   0 & 0x01) << 3 |// TBD
					(     (initial_out_ch & 0x0F) << 4);
		
		fe_config_register[2] =	((initial_out_ch >> 4) & 0x07)  << 0 |
					(		 en_auto_trig & 0x01) << 3 |
					(		  en_ext_trig & 0x01) << 4 |
					(       one_sixtyfour & 0x01) << 5 |
					(                led0 & 0x01) << 6 |
					(                led1 & 0x01) << 7;
	}
	
	
	//printf("fe%dconfireg1: ",mrc_idx);
	for (int i =0 ; i<3; i++) {
		if (i==0) {
			//printf("ConfReg1 [0][1][2]=0x%02X",fe_config_register[i]);
		//	printf("0x%02X",fe_config_register[i]);
		}else {
		//	printf("%02X",fe_config_register[i]);
		}
	}	
	

	//printf(" = %d %d %d %d %d %d %03d %d %d %d %d" ,auto_trigger,en_zero_supp,en_auto_trig,en_ext_trig,test_en_eve,test_en_odd,hold1_delay,initial_out_ch,one_sixtyfour,led0,led1);
	//printf("\n");
	
	
	
	if ((fe_config_register[2] & 0x08)==0x08) {
		MRC_DBG("TRIGGER: SELF\n");
	}
	if ((fe_config_register[2] & 0x10)==0X10) {
		MRC_DBG("TRIGGER: EXT\n");
	}

 
 return true;
}

bool MRChi::MakeConfReg2(GI_Config *cfg,int mrc_idx)
{
	if (cfg == NULL) {
		MRC_ERR("Configuration object is NULL...Exiting\n");
		return false;
	}
	
	for (int i =0 ; i<3; i++) {
		fe_config_register2[i]=0;
	}	
	
	int ret =0;
	
	int DaqMode			= cfg->get<int>(ret, 0, "Daq_mode"			,0,"cfgReg2", mrc_idx, "fe",0,"cb");	
	int HitDelay		= cfg->get<int>(ret, 0, "hit_delay"			,0,"cfgReg2", mrc_idx, "fe",0,"cb");
	int TrigMode		= cfg->get<int>(ret, 0, "TrigMode"			,0,"cfgReg2", mrc_idx, "fe",0,"cb");
	int StartDaqDelay	= cfg->get<int>(ret, 0, "StartDaqDelay"		,0,"cfgReg2", mrc_idx, "fe",0,"cb");
	int FastSettlingTime= cfg->get<int>(ret, 0, "FastSettlingTime"	,0,"cfgReg2", mrc_idx, "fe",0,"cb");
	int SlowSettlingTime= cfg->get<int>(ret, 0, "SlowSettlingTime"	,0,"cfgReg2", mrc_idx, "fe",0,"cb");
	

	
	fe_config_register2[0] = (DaqMode & 0x03)  << 0 |
				(HitDelay & 0x07) << 2 |
				(TrigMode & 0x03) << 5 |
				( 0 & 0x01) << 7;	// 0 = MAROC 2, 

	fe_config_register2[1] =(		StartDaqDelay & 0x0F) << 0|
							(	 FastSettlingTime & 0x0F) << 4;

	fe_config_register2[2] =( (FastSettlingTime >> 4) & 0x03) << 0|
							(		 SlowSettlingTime & 0x3F) << 2;

	
	// if( fRevisionFW[mrc_idx] == 3 ){fe_config_register2[0] |= 0x80;}  // 1 = MAROC 3	
	if( fASIC[mrc_idx] == 3 ){fe_config_register2[0] |= 0x80;}  // 1 = MAROC 3	 new! 2013 June 27 Matteo
	
	

//	printf("fe%dconfireg2: ",mrc_idx);
	
	for (int i =0 ; i<3; i++) {
		if (i==0) {
			//printf("ConfReg2 [0][1][2]=0x%02X",fe_config_register2[i]);
		//	printf("0x%02X",fe_config_register2[i]);
		}else {
		//	printf("%02X",fe_config_register2[i]);
		}//
	}	
	
	/*
	printf(" = %d %d %d %d %d %d"	,DaqMode
									,HitDelay
									,TrigMode
									,StartDaqDelay
									,FastSettlingTime
								,SlowSettlingTime);
	

	printf("\n");
	*/
	return true;

}



bool	MRChi::PrintConfiguration(GI_Config *cfg,int lmrc_count)
{
	int			ret = 0;		// returned value
//	int			varbool = false;	// boolean 
	int			varint=0;		// integer
	std::string varstring;	// string
	

	ret = varint; // 
	//RUN info
	
	varint = cfg->get<int>(ret,0,"loglevel");
	MRC_DBG("Loglevel: %d  \n",varint);
	
	varstring = cfg->get<std::string>(ret, 0, "at");
	MRC_DBG("Place: %s\n",varstring.c_str());
	
	
	varstring = cfg->get<std::string>(ret, 0, "file_prefix",0,"run");
	MRC_DBG("Name: %s\n",varstring.c_str());
	
	varint = cfg->get<int>(ret,0,"event_preset",0,"run");
	MRC_DBG("Event Preset (evt)   : %d or \n",varint);
	
	
	varint = cfg->get<int>(ret,0,"time_preset_sec",0,"run");
	MRC_DBG("Time  Preset (sec)   : %d   seconds\n",varint);
		

	
	
	// VME
	
	// CB
	varint = cfg->get<int>(ret,0,"did",0,"cb",0,"default");
	MRC_DBG("Controller Board %d  (did)\n ",varint);
	
	
	// FE
	// trigger
	//cfgReg1
	
	//cfgReg2
	
	//mrc2
	//mrc3
	
	
	
	//FE CONFIGURATION

	printf("      CHARGE |     BINARY OUT  \n");
	printf("       delay |  dac   delay   \n");

	
	for (int fe=0; fe<lmrc_count; fe++) {
		int hold_delay = cfg->get<int>(ret, 0, "hold1_delay",0,"cfgReg1", fe, "fe",0,"cb");
		int hit_delay = cfg->get<int>(ret, 0, "hit_delay",0,"cfgReg2", fe, "fe",0,"cb");
		int threshold;

		if(fASIC[fe]==2){
			//...	
		}else {
			if (fASIC[fe]==3) {
				threshold = cfg->get<int>(ret, 0, "DAC0",0,"mrc3", fe, "fe",0,"cb");	
			}
		}

		printf("fe[%d] %4d   | %4d  %4d \n",fe,hold_delay,threshold,hit_delay);	

	}
	return true;
};


bool	MRChi::MakeDaqThresholdReg(GI_Config *cfg, int maroc_index)
{
	// Read default value and set array
	int ret;
	int thr_default = cfg->get<int>(ret, 0, "daq_thr_def",maroc_index, "fe", 0, "cb");
	for (int fech=0; fech<64; fech++) {
		fSparseThr[fech]=thr_default;
	}
		
	// Try to read map file and update the array
	char file_name[1000],mystring[1000];
	FILE *in;
	sprintf(file_name, "cfg/thr/thr_map_%d.txt", fBaseAddr[maroc_index]/16);
	if ( (in=fopen(file_name, "r")) == NULL ){
		MRC_WRN("Maroc %d, cannot open file %s, use default\n",fBaseAddr[maroc_index]/16, file_name);	
	} else {
		MRC_WRN("Maroc %d, set thr from file %s\n",fBaseAddr[maroc_index]/16,file_name);
		for (int fech=0; fech<64; fech++) {
			fgets (mystring , 250 , in);
			fSparseThr[fech] = (unsigned long)atoi(mystring);
		}
		fclose(in);
	}

	
	char DAC0setting[20];
	sprintf (DAC0setting, "thr_map_%d", fBaseAddr[maroc_index]/16);
	
	
	
	// Print definitive map
	//printf("THR MAP - FE %02d\n",maroc_index);
	for (int fech=0; fech<64; fech++) {
	//	printf("%4ld   ",fSparseThr[fech]); 
		if (fech%8==7) {
	//		printf("\n");
		}
		cfg->AddList(DAC0setting,fSparseThr[fech]);
	}
	
	// Coding Values into fe format
	
	for (int i=0;i<64;i++) {
		DaqThreshold[(i*3)+0] = fSparseThr[i] & 0xFF;
		DaqThreshold[(i*3)+1] = (fSparseThr[i] >> 8 ) & 0x0F;
		DaqThreshold[(i*3)+2] = 0;
		//printf("ch%02d sparse thr = 0x %02X %02X %02X \n",i,DaqThreshold[(i*3)+2],DaqThreshold[(i*3)+1],DaqThreshold[(i*3)+0]);

	};
	return true;
};

bool MRChi::MakeHitEnabledMask(GI_Config *cfg, int maroc_index)
{
	
	// Read value from config file
	int ret;
	int hit_en0 = cfg->get<int>(ret,0,"hit_en0",0,"trigger",maroc_index, "fe",0,"cb");
	int hit_en1 = cfg->get<int>(ret,0,"hit_en1",0,"trigger",maroc_index, "fe",0,"cb");
	int hit_en2 = cfg->get<int>(ret,0,"hit_en2",0,"trigger",maroc_index, "fe",0,"cb");
	
	//printf("HIT enabled = %x %x %x (from configuration file libconfig++)\n",hit_en0,hit_en1,hit_en2);
	
	// Code values into fe format (3 char instead of a int)
	
	for (int i=0; i<3; i++) {
		fe_hiten0_register[i] = (hit_en0 >> (i*8)) & 0xFF;
		fe_hiten1_register[i] = (hit_en1 >> (i*8)) & 0xFF;
		fe_hiten2_register[i] = (hit_en2 >> (i*8)) & 0xFF;
	}

	//printf("hit en reg 0 = %02x %02x %02x\n",fe_hiten0_register[2],fe_hiten0_register[1],fe_hiten0_register[0]);	
	//printf("hit en reg 1 = %02x %02x %02x\n",fe_hiten1_register[2],fe_hiten1_register[1],fe_hiten1_register[0]);	
//	printf("hit en reg 2 = %02x %02x %02x\n",fe_hiten2_register[2],fe_hiten2_register[1],fe_hiten2_register[0]);	
	
	return true;
};

bool MRChi::MakeMarocSlowControl(GI_Config *cfg,int mrc_idx)
{
	
	// DAC (3*24bit) is in common between Maroc 2 and 3,
	// but attention they are different 
	// and differently loaded in the ASIC Register. 

	unsigned long dac[3];
	
	for (int j=0; j<3; j++) {
		dac[j]=0;
	}
	
	
	
	unsigned char gain[64]; 
	for (int j=0; j<64; j++) {
		gain[j]=0;
	}
	
	
	
	int ret; // for GI_config returned value
	if (cfg == NULL) {
		MRC_ERR("Configuration object is NULL...Exiting\n");
		return false;
	}
	
	if( fASIC[mrc_idx] == 2 ){ 	
		
		MRC_DUM("MAROC 2 Slow Control\n");
		
		// DAC0
		// DAC1
		// DAC2
		
		// GAIN
		
		int en_ck			= cfg->get<int>(ret, 0, "en_ck",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int cmd_lvds		= cfg->get<int>(ret, 0, "cmd_lvds",		0,"mrc2", mrc_idx, "fe", 0, "cb");					
		int en_serializer	= cfg->get<int>(ret, 0, "en_serializer",0,"mrc2", mrc_idx, "fe", 0, "cb");	
		int cmd_lucid		= cfg->get<int>(ret, 0, "cmd_lucid",	0,"mrc2", mrc_idx, "fe", 0, "cb");					
		int fs_choice		= cfg->get<int>(ret, 0, "fs_choice",	0,"mrc2", mrc_idx, "fe", 0, "cb");					
		int sw_20f			= cfg->get<int>(ret, 0, "sw_20f",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int sw_40f			= cfg->get<int>(ret, 0, "sw_40f",		0,"mrc2", mrc_idx, "fe", 0, "cb");				
		int sw_25k			= cfg->get<int>(ret, 0, "sw_25k",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int sw_50k			= cfg->get<int>(ret, 0, "sw_50k",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int sw_100k			= cfg->get<int>(ret, 0, "sw_100k",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int sw_50f			= cfg->get<int>(ret, 0, "sw_50f",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int sw_100f			= cfg->get<int>(ret, 0, "sw_100f",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int en_adc			= cfg->get<int>(ret, 0, "en_adc",		0,"mrc2", mrc_idx, "fe", 0, "cb");	
		int h1h2_choice		= cfg->get<int>(ret, 0, "h1h2_choice",	0,"mrc2", mrc_idx, "fe", 0, "cb");
		int valid_ch_select	= cfg->get<int>(ret, 0,"valid_ch_select",0,"mrc2", mrc_idx, "fe", 0, "cb");
		int swss_c2			= cfg->get<int>(ret, 0, "swss_c2",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int swss_c1			= cfg->get<int>(ret, 0, "swss_c1",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		int swss_c0			= cfg->get<int>(ret, 0, "swss_c0",		0,"mrc2", mrc_idx, "fe", 0, "cb");
		
		maroc_config_data[0] =	(         en_ck & 0x01) << 0 | 
		(      cmd_lvds & 0x01) << 1 |
		( en_serializer & 0x01) << 2 | 
		(     cmd_lucid & 0x01) << 3 |
		(     fs_choice & 0x01) << 4 | 
		(        sw_20f & 0x01) << 5 |
		(        sw_40f & 0x01) << 6 | 
		(        sw_25k & 0x01) << 7;
		
		maroc_config_data[1] =	(          sw_50k & 0x01) << 0 | 
		(         sw_100k & 0x01) << 1 |
		(          sw_50f & 0x01) << 2 | 
		(         sw_100f & 0x01) << 3 |
		(          en_adc & 0x01) << 4 | 
		(     h1h2_choice & 0x01) << 5 |
		( valid_ch_select & 0x01) << 6 |
		(         swss_c2 & 0x01) << 7;
		
		maroc_config_data[2] =  (swss_c1 & 0x01) << 0 | (swss_c0 & 0x01) << 1;
		
		// dac (from ib=16+2 to ib=11*8+1
		int il, ib, ir;
		ib=2*8+2; // maroc_config_data bit index (and vector index*8)
		for (ir=2;ir>=0;ir--) { // dac index
			for (il=0;il<24;il++) { // dac bit index
				maroc_config_data[ib/8] |= (((dac[ir] >> il) & 0x01) << (ib%8));
				ib++;
				if ((ib%8)==0) {
					maroc_config_data[ib/8]=0; // clean before use
				}
			}
		}
		
		// gain (from ib=11*8+2 to ib=67*8+1)
		for (ir=63;ir>=0;ir--) { // gain index
			for (il=6;il>=0;il--) {  // gain bit index
				maroc_config_data[ib/8] |= ((((gain[ir] & 0x7e) >> il) & 0x01) << (ib%8));
				ib++;
				if ((ib%8)==0) {
					maroc_config_data[ib/8] = 0;
				}
			}
		}
		
		maroc_config_data[68] = 0;
	}
	else {
		if( fASIC[mrc_idx] == 3 ){ 	
			
			MRC_DUM("MAROC 3 Slow Control\n");
			
			// Read DAC (10 bit)
			dac[0] = cfg->get<int>(ret, 0, "DAC0",0,"mrc3", mrc_idx, "fe", 0, "cb");
			dac[1] = cfg->get<int>(ret, 0, "DAC1",0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			
			// Read GAIN (8 bit) one value for each channel. Set default value (config), than look for a map
			int gain_default = cfg->get<int>(ret, 0, "gain_default",0,"preamp",0,"mrc3", mrc_idx, "fe", 0, "cb");
			for (int fech=0; fech<64; fech++) {
				gain[fech]=gain_default;
				//printf("gain[%02d]=%d\n",fech,gain[fech]);
				
			}
			
			char file_name[1000],mystring[1000];
			
			// MAPPE DI GUADAGNO 

			FILE *in;
			sprintf(file_name, "cfg/gain/gain_map_%d.txt", fBaseAddr[mrc_idx]/16);
			
			
			if ( (in=fopen(file_name, "r")) == NULL ){
				MRC_DBG("Maroc %d, cannot open file %s, use default\n",fBaseAddr[mrc_idx]/16, file_name);	
			} else {
				MRC_DBG("Maroc %d, set gain from file %s\n",fBaseAddr[mrc_idx]/16,file_name);
				
				for (int fech=0; fech<64; fech++) {
					fgets(mystring,4,in);
					gain[fech] = (unsigned char)atoi(mystring);
					//printf("gain[%02d]=%d\n",fech,gain[fech]);
				}
				fclose(in);
			}
			
			char gainsetting[20];
			sprintf (gainsetting, "gain_map_%d", fBaseAddr[mrc_idx]/16);
			
			// PRINT GAIN (and save it on out config file)
			
			printf("GAIN - FE %02d\n",mrc_idx);
			for (int fech=0; fech<64; fech++) {
				printf("%3d   ",gain[fech]); 
				if (fech%8==7) {
					printf("\n");
				}
				cfg->AddList(gainsetting,gain[fech]);
			}
			
			
			// OTHERS (1bit)	
			
			int small_dac		= cfg->get<int>(ret, 0, "small_dac"		,0,"general",0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int cmd_fsb_fsu		= cfg->get<int>(ret, 0, "cmd_fsb_fsu"	,0,"general",0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int enb_tristate	= cfg->get<int>(ret, 0, "enb_tristate"	,0,"general",0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int polar_discri	= cfg->get<int>(ret, 0, "polar_discri"	,0,"general",0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int d1_d2			= cfg->get<int>(ret, 0, "d1_d2"			,0,"general",0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int sw_fsu_100k		= cfg->get<int>(ret, 0, "sw_fsu_100k"	,0,"fsu"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsu_50k		= cfg->get<int>(ret, 0, "sw_fsu_50k"	,0,"fsu"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsu_25k		= cfg->get<int>(ret, 0, "sw_fsu_25k"	,0,"fsu"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsu_40f		= cfg->get<int>(ret, 0, "sw_fsu_40f"	,0,"fsu"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsu_20f		= cfg->get<int>(ret, 0, "sw_fsu_20f"	,0,"fsu"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			
			int sw_fsb1_50k		= cfg->get<int>(ret, 0, "sw_fsb1_50k"	,0,"fsb1"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsb1_100k	= cfg->get<int>(ret, 0, "sw_fsb1_100k"	,0,"fsb1"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsb1_100f	= cfg->get<int>(ret, 0, "sw_fsb1_100f"	,0,"fsb1"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsb1_50f		= cfg->get<int>(ret, 0, "sw_fsb1_50f"	,0,"fsb1"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int sw_fsb2_50k		= cfg->get<int>(ret, 0, "sw_fsb2_50k"	,0,"fsb2"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsb2_100k	= cfg->get<int>(ret, 0, "sw_fsb2_100k"	,0,"fsb2"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsb2_100f	= cfg->get<int>(ret, 0, "sw_fsb2_100f"	,0,"fsb2"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_fsb2_50f		= cfg->get<int>(ret, 0, "sw_fsb2_50f"	,0,"fsb2"	,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int valid_dc_fs		= cfg->get<int>(ret, 0, "valid_dc_fs"	,0,"dc"		,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int valid_dc_fsb2	= cfg->get<int>(ret, 0, "valid_dc_fsb2"	,0,"dc"		,0,"fastpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int sw_ss_1200f		= cfg->get<int>(ret, 0, "sw_ss_1200f"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_ss_600f		= cfg->get<int>(ret, 0, "sw_ss_600f"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int sw_ss_300f		= cfg->get<int>(ret, 0, "sw_ss_300f"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int enable_ss		= cfg->get<int>(ret, 0, "enable_ss"		,0,"shaper"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int swb_buf_2p		= cfg->get<int>(ret, 0, "swb_buf_2p"	,0,"buffer"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int swb_buf_1p 		= cfg->get<int>(ret, 0, "swb_buf_1p"	,0,"buffer"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int swb_buf_250f	= cfg->get<int>(ret, 0, "swb_buf_250f"	,0,"buffer"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int swb_buf_500f	= cfg->get<int>(ret, 0, "swb_buf_500f"	,0,"buffer"	,0,"slowpath"	,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int cmd_fsb			= cfg->get<int>(ret, 0, "cmd_fsb"		,0,"preamp"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int cmd_ss			= cfg->get<int>(ret, 0, "cmd_ss"		,0,"preamp"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int cmd_fsu			= cfg->get<int>(ret, 0, "cmd_fsu"		,0,"preamp"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int enb_out_adc		= cfg->get<int>(ret, 0, "enb_out_adc"	,0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int inv_start_gray	= cfg->get<int>(ret, 0, "inv_start_gray",0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int ramp_8bit		= cfg->get<int>(ret, 0, "ramp_8bit"		,0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int ramp_10bit		= cfg->get<int>(ret, 0, "ramp_10bit"	,0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int h1_h2_choice	= cfg->get<int>(ret, 0, "h1h2_choice"	,0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int en_adc_m3		= cfg->get<int>(ret, 0, "en_adc"		,0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int inv_discri_adc	= cfg->get<int>(ret, 0, "inv_discri_adc",0,"adc"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			int en_otabg		= cfg->get<int>(ret, 0, "en_otabg"		,0,"others"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int en_dac			= cfg->get<int>(ret, 0, "en_dac"		,0,"others"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			int cmd_ck_mux		= cfg->get<int>(ret, 0, "cmd_ck_mux"	,0,"others"					,0,"mrc3", mrc_idx, "fe", 0, "cb");
			
			
			
			maroc_config_data[0] =	(         en_otabg & 0x001) << 0 | 
			(           en_dac & 0x001) << 1 |
			(        small_dac & 0x001) << 2 | 
			(           dac[1] & 0x200) >> 6 | // dac1 bit 9 MSB before!
			(           dac[1] & 0x100) >> 4 | // dac1 bit 8
			(           dac[1] & 0x080) >> 2 | // dac1 bit 7
			(           dac[1] & 0x040) >> 0 | // dac1 bit 6
			(           dac[1] & 0x020) << 2 ; // dac1 bit 5
			
			
			maroc_config_data[1] =	(           dac[1] & 0x010) >> 4 | // dac1 bit 4	//pos 0	 
			(           dac[1] & 0x008) >> 2 | // dac1 bit 3	//pos 1
			(           dac[1] & 0x004) >> 0 | // dac1 bit 2	//pos 2
			(           dac[1] & 0x002) << 2 | // dac1 bit 1	//pos 3
			(           dac[1] & 0x001) << 4 | // dac1 bit 0	//pos 4 LSB is the last!
			(           dac[0] & 0x200) >> 4 | // dac0 bit 9	//pos 5
			(           dac[0] & 0x100) >> 2 | // dac0 bit 8	//pos 6
			(           dac[0] & 0x080) >> 0 ; // dac0 bit 7	//pos 7								 
			
			
			
			maroc_config_data[2] =	(           dac[0] & 0x040) >> 6 | // dac0 bit 6	//pos 0	 
			(           dac[0] & 0x020) >> 4 | // dac0 bit 5	//pos 1		
			(           dac[0] & 0x010) >> 2 | // dac0 bit 4	//pos 2
			(           dac[0] & 0x008) >> 0 | // dac0 bit 3	//pos 3		
			(           dac[0] & 0x004) << 2 | // dac0 bit 2	//pos 4	
			(           dac[0] & 0x002) << 4 | // dac0 bit 1	//pos 5
			(           dac[0] & 0x001) << 6 | // dac0 bit 0	//pos 6		
			(	   enb_out_adc & 0x01)  << 7;
			
			
			
			maroc_config_data[3] = 	(  inv_start_gray  & 0x01) << 0 | 
			(       ramp_8bit  & 0x01) << 1 |
			(      ramp_10bit  & 0x01) << 2 | 
			(            0x0  & 0xF8) << 3;	// All 0xFF are mask_ORx_chx (128 bits) (0 means active)
			
			maroc_config_data[4] = 	0x0;
			maroc_config_data[5] = 	0x0;
			maroc_config_data[6] = 	0x0;
			maroc_config_data[7] = 	0x0;
			maroc_config_data[8] = 	0x0;
			maroc_config_data[9] = 	0x0;
			maroc_config_data[10] = 0x0;
			maroc_config_data[11] = 0x0;
			maroc_config_data[12] = 0x0;
			maroc_config_data[13] = 0x0;
			maroc_config_data[14] = 0x0;
			maroc_config_data[15] = 0x0;
			maroc_config_data[16] = 0x0;
			maroc_config_data[17] = 0x0;
			maroc_config_data[18] = 0x0;
			
			maroc_config_data[19] = (           0x0  & 0x07) << 0 |
			(    cmd_ck_mux  & 0x01) << 3 |
			(         d1_d2  & 0x01) << 4 |
			( inv_discri_adc & 0x01) << 5 |
			(  polar_discri  & 0x01) << 6 |
			(  enb_tristate  & 0x01) << 7;
			
			
			maroc_config_data[20] = (  valid_dc_fsb2 & 0x01) << 0 | 
			(    sw_fsb2_50f & 0x01) << 1 |
			(   sw_fsb2_100f & 0x01) << 2 | 
			(  sw_fsb2_100k  & 0x01) << 3 |
			(   sw_fsb2_50k  & 0x01) << 4 | 
			(   valid_dc_fs  & 0x01) << 5 |
			(   cmd_fsb_fsu  & 0x01) << 6 | 
			(   sw_fsb1_50f  & 0x01) << 7 ;
			
			maroc_config_data[21] = (   sw_fsb1_100f & 0x01) << 0 | 
			(   sw_fsb1_100k & 0x01) << 1 |
			(    sw_fsb1_50k & 0x01) << 2 | 
			(   sw_fsu_100k  & 0x01) << 3 |
			(     sw_fsu_50k & 0x01) << 4 | 
			(    sw_fsu_25k  & 0x01) << 5 |
			(    sw_fsu_40f  & 0x01) << 6 | 
			(    sw_fsu_20f  & 0x01) << 7 ;
			
			maroc_config_data[22] = (   h1_h2_choice & 0x01) << 0 | 
			(      en_adc_m3 & 0x01) << 1 |
			(    sw_ss_1200f & 0x01) << 2 | 
			(    sw_ss_600f  & 0x01) << 3 |
			(     sw_ss_300f & 0x01) << 4 | 
			(     enable_ss  & 0x01) << 5 |
			(    swb_buf_2p  & 0x01) << 6 | 
			(    swb_buf_1p  & 0x01) << 7 ;
			
			maroc_config_data[23] = (   swb_buf_500f & 0x01) << 0 |			// swb_buf_250f fino a 2013 June 19th 
			(   swb_buf_250f & 0x01) << 1 |			// swb_buf_500f fino a 2013 June 19th
			(        cmd_fsb & 0x01) << 2 | 
			(         cmd_ss & 0x01) << 3 |
			(        cmd_fsu & 0x01) << 4 | 
			(           0x00 & 0x01) << 5 |	// CMD_SUM63
			((gain[63] >> 7) & 0x01) << 6 | 
			((gain[63] >> 6) & 0x01) << 7;
			
			maroc_config_data[24] = ((gain[63] >> 5) & 0x01) << 0 |
			((gain[63] >> 4) & 0x01) << 1 |
			((gain[63] >> 3) & 0x01) << 2 |
			((gain[63] >> 2) & 0x01) << 3 | 
			((gain[63] >> 1) & 0x01) << 4 |
			((gain[63] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |	// CMD_SUM62
			((gain[62] >> 7) & 0x01) << 7;
			
			maroc_config_data[25] = ((gain[62] >> 6) & 0x01) << 0 |
			((gain[62] >> 5) & 0x01) << 1 |
			((gain[62] >> 4) & 0x01) << 2 |
			((gain[62] >> 3) & 0x01) << 3 | 
			((gain[62] >> 2) & 0x01) << 4 |
			((gain[62] >> 1) & 0x01) << 5 |
			((gain[62] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7; // CMD_SUM61
			
			
			maroc_config_data[26] = ((gain[61] >> 7) & 0x01) << 0 |
			((gain[61] >> 6) & 0x01) << 1 |
			((gain[61] >> 5) & 0x01) << 2 |
			((gain[61] >> 4) & 0x01) << 3 | 
			((gain[61] >> 3) & 0x01) << 4 |
			((gain[61] >> 2) & 0x01) << 5 |
			((gain[61] >> 1) & 0x01) << 6 |
			((gain[61] >> 0) & 0x01) << 7;
			
			maroc_config_data[27] = (           0x00 & 0x01) << 0 |// CMD_SUM60              
			((gain[60] >> 7) & 0x01) << 1 |
			((gain[60] >> 6) & 0x01) << 2 |
			((gain[60] >> 5) & 0x01) << 3 | 
			((gain[60] >> 4) & 0x01) << 4 |
			((gain[60] >> 3) & 0x01) << 5 |
			((gain[60] >> 2) & 0x01) << 6 |
			((gain[60] >> 1) & 0x01) << 7;
			
			maroc_config_data[28] =	((gain[60] >> 0) & 0x01) << 0 | 
			(           0x00 & 0x01) << 1 |// CMD_SUM59
			((gain[59] >> 7) & 0x01) << 2 |
			((gain[59] >> 6) & 0x01) << 3 |
			((gain[59] >> 5) & 0x01) << 4 |
			((gain[59] >> 4) & 0x01) << 5 |
			((gain[59] >> 3) & 0x01) << 6 |	
			((gain[59] >> 2) & 0x01) << 7;
			
			maroc_config_data[29] =	((gain[59] >> 1) & 0x01) << 0 |
			((gain[59] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM58
			((gain[58] >> 7) & 0x01) << 3 |
			((gain[58] >> 6) & 0x01) << 4 |
			((gain[58] >> 5) & 0x01) << 5 |
			((gain[58] >> 4) & 0x01) << 6 |
			((gain[58] >> 3) & 0x01) << 7;
			
			maroc_config_data[30] =	((gain[58] >> 2) & 0x01) << 0 |
			((gain[58] >> 1) & 0x01) << 1 |
			((gain[58] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM57
			((gain[57] >> 7) & 0x01) << 4 |
			((gain[57] >> 6) & 0x01) << 5 |
			((gain[57] >> 5) & 0x01) << 6 |
			((gain[57] >> 4) & 0x01) << 7; 
			
			maroc_config_data[31] =	((gain[57] >> 3) & 0x01) << 0 |	
			((gain[57] >> 2) & 0x01) << 1 |
			((gain[57] >> 1) & 0x01) << 2 |
			((gain[57] >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM56
			((gain[56] >> 7) & 0x01) << 5 |
			((gain[56] >> 6) & 0x01) << 6 |
			((gain[56] >> 5) & 0x01) << 7;
			
			maroc_config_data[32] =	((gain[56] >> 4) & 0x01) << 0 |
			((gain[56] >> 3) & 0x01) << 1 |	
			((gain[56] >> 2) & 0x01) << 2 |
			((gain[56] >> 1) & 0x01) << 3 |
			((gain[56] >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM55
			((gain[55] >> 7) & 0x01) << 6 |
			((gain[55] >> 6) & 0x01) << 7;
			
			maroc_config_data[33] =	((gain[55] >> 5) & 0x01) << 0 |
			((gain[55] >> 4) & 0x01) << 1 |
			((gain[55] >> 3) & 0x01) << 2 |	
			((gain[55] >> 2) & 0x01) << 3 |
			((gain[55] >> 1) & 0x01) << 4 |
			((gain[55] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM54
			((gain[54] >> 7) & 0x01) << 7;
			
			maroc_config_data[34] =	((gain[54] >> 6) & 0x01) << 0 |
			((gain[54] >> 5) & 0x01) << 1 |
			((gain[54] >> 4) & 0x01) << 2 |
			((gain[54] >> 3) & 0x01) << 3 |	
			((gain[54] >> 2) & 0x01) << 4 |
			((gain[54] >> 1) & 0x01) << 5 |
			((gain[54] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7;// CMD_SUM53
			
			maroc_config_data[35] =	((gain[53] >> 7) & 0x01) << 0 |
			((gain[53] >> 6) & 0x01) << 1 |
			((gain[53] >> 5) & 0x01) << 2 |
			((gain[53] >> 4) & 0x01) << 3 |
			((gain[53] >> 3) & 0x01) << 4 |	
			((gain[53] >> 2) & 0x01) << 5 |
			((gain[53] >> 1) & 0x01) << 6 |
			((gain[53] >> 0) & 0x01) << 7;
			
			maroc_config_data[36] =	(           0x00 & 0x01) << 0 |// CMD_SUM52
			((gain[52] >> 7) & 0x01) << 1 |
			((gain[52] >> 6) & 0x01) << 2 |
			((gain[52] >> 5) & 0x01) << 3 |
			((gain[52] >> 4) & 0x01) << 4 |
			((gain[52] >> 3) & 0x01) << 5 |	
			((gain[52] >> 2) & 0x01) << 6 |
			((gain[52] >> 1) & 0x01) << 7;
			
			maroc_config_data[37] =	((gain[52] >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM51
			((gain[51] >> 7) & 0x01) << 2 |
			((gain[51] >> 6) & 0x01) << 3 |
			((gain[51] >> 5) & 0x01) << 4 |
			((gain[51] >> 4) & 0x01) << 5 |
			((gain[51] >> 3) & 0x01) << 6 |	
			((gain[51] >> 2) & 0x01) << 7;
			
			maroc_config_data[38] =	((gain[51] >> 1) & 0x01) << 0 |
			((gain[51] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM50
			((gain[50] >> 7) & 0x01) << 3 |
			((gain[50] >> 6) & 0x01) << 4 |
			((gain[50] >> 5) & 0x01) << 5 |
			((gain[50] >> 4) & 0x01) << 6 |
			((gain[50] >> 3) & 0x01) << 7;
			
			maroc_config_data[39] =	((gain[50] >> 2) & 0x01) << 0 |
			((gain[50] >> 1) & 0x01) << 1 |
			((gain[50] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM49
			((gain[49] >> 7) & 0x01) << 4 |
			((gain[49] >> 6) & 0x01) << 5 |
			((gain[49] >> 5) & 0x01) << 6 |
			((gain[49] >> 4) & 0x01) << 7;
			
			maroc_config_data[40] =	((gain[49] >> 3) & 0x01) << 0 |	
			((gain[49] >> 2) & 0x01) << 1 |
			((gain[49] >> 1) & 0x01) << 2 |
			((gain[49] >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM48
			((gain[48] >> 7) & 0x01) << 5 |
			((gain[48] >> 6) & 0x01) << 6 |
			((gain[48] >> 5) & 0x01) << 7;
			
			maroc_config_data[41] =	((gain[48] >> 4) & 0x01) << 0 |
			((gain[48] >> 3) & 0x01) << 1 |	
			((gain[48] >> 2) & 0x01) << 2 |
			((gain[48] >> 1) & 0x01) << 3 |
			((gain[48] >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM47
			((gain[47] >> 7) & 0x01) << 6 |
			((gain[47] >> 6) & 0x01) << 7;
			
			
			maroc_config_data[42] =	((gain[47] >> 5) & 0x01) << 0 |
			((gain[47] >> 4) & 0x01) << 1 |
			((gain[47] >> 3) & 0x01) << 2 |	
			((gain[47] >> 2) & 0x01) << 3 |
			((gain[47] >> 1) & 0x01) << 4 |
			((gain[47] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM46
			((gain[46] >> 7) & 0x01) << 7;
			
			maroc_config_data[43] =	((gain[46] >> 6) & 0x01) << 0 |
			((gain[46] >> 5) & 0x01) << 1 |
			((gain[46] >> 4) & 0x01) << 2 |
			((gain[46] >> 3) & 0x01) << 3 |	
			((gain[46] >> 2) & 0x01) << 4 |
			((gain[46] >> 1) & 0x01) << 5 |
			((gain[46] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7;// CMD_SUM45
			
			maroc_config_data[44] =	((gain[45] >> 7) & 0x01) << 0 |
			((gain[45] >> 6) & 0x01) << 1 |
			((gain[45] >> 5) & 0x01) << 2 |
			((gain[45] >> 4) & 0x01) << 3 |
			((gain[45] >> 3) & 0x01) << 4 |	
			((gain[45] >> 2) & 0x01) << 5 |
			((gain[45] >> 1) & 0x01) << 6 |
			((gain[45] >> 0) & 0x01) << 7;
			
			maroc_config_data[45] =	(           0x00 & 0x01) << 0 |// CMD_SUM44
			((gain[44] >> 7) & 0x01) << 1 |
			((gain[44] >> 6) & 0x01) << 2 |
			((gain[44] >> 5) & 0x01) << 3 |
			((gain[44] >> 4) & 0x01) << 4 |
			((gain[44] >> 3) & 0x01) << 5 |	
			((gain[44] >> 2) & 0x01) << 6 |
			((gain[44] >> 1) & 0x01) << 7;
			
			maroc_config_data[46] =	((gain[44] >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM43
			((gain[43] >> 7) & 0x01) << 2 |
			((gain[43] >> 6) & 0x01) << 3 |
			((gain[43] >> 5) & 0x01) << 4 |
			((gain[43] >> 4) & 0x01) << 5 |
			((gain[43] >> 3) & 0x01) << 6 |	
			((gain[43] >> 2) & 0x01) << 7;
			
			maroc_config_data[47] =	((gain[43] >> 1) & 0x01) << 0 |
			((gain[43] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM42
			((gain[42] >> 7) & 0x01) << 3 |
			((gain[42] >> 6) & 0x01) << 4 |
			((gain[42] >> 5) & 0x01) << 5 |
			((gain[42] >> 4) & 0x01) << 6 |
			((gain[42] >> 3) & 0x01) << 7;	
			
			maroc_config_data[48] =	((gain[42] >> 2) & 0x01) << 0 |
			((gain[42] >> 1) & 0x01) << 1 |
			((gain[42] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM41
			((gain[41] >> 7) & 0x01) << 4 |
			((gain[41] >> 6) & 0x01) << 5 |
			((gain[41] >> 5) & 0x01) << 6 |
			((gain[41] >> 4) & 0x01) << 7;
			
			maroc_config_data[49] =	((gain[41] >> 3) & 0x01) << 0 |	
			((gain[41] >> 2) & 0x01) << 1 |
			((gain[41] >> 1) & 0x01) << 2 |
			((gain[41] >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM40
			((gain[40] >> 7) & 0x01) << 5 |
			((gain[40] >> 6) & 0x01) << 6 |
			((gain[40] >> 5) & 0x01) << 7;
			
			maroc_config_data[50] =	((gain[40] >> 4) & 0x01) << 0 |
			((gain[40] >> 3) & 0x01) << 1 |	
			((gain[40] >> 2) & 0x01) << 2 |
			((gain[40] >> 1) & 0x01) << 3 |
			((gain[40] >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM39
			((gain[39] >> 7) & 0x01) << 6 |
			((gain[39] >> 6) & 0x01) << 7;
			
			maroc_config_data[51] =	((gain[39] >> 5) & 0x01) << 0 |
			((gain[39] >> 4) & 0x01) << 1 |
			((gain[39] >> 3) & 0x01) << 2 |	
			((gain[39] >> 2) & 0x01) << 3 |
			((gain[39] >> 1) & 0x01) << 4 |
			((gain[39] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM38
			((gain[38] >> 7) & 0x01) << 7;
			
			maroc_config_data[52] =	((gain[38] >> 6) & 0x01) << 0 |
			((gain[38] >> 5) & 0x01) << 1 |
			((gain[38] >> 4) & 0x01) << 2 |
			((gain[38] >> 3) & 0x01) << 3 |	
			((gain[38] >> 2) & 0x01) << 4 |
			((gain[38] >> 1) & 0x01) << 5 |
			((gain[38] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7; // CMD_SUM37
			
			maroc_config_data[53] =	((gain[37] >> 7) & 0x01) << 0 |
			((gain[37] >> 6) & 0x01) << 1 |
			((gain[37] >> 5) & 0x01) << 2 |
			((gain[37] >> 4) & 0x01) << 3 |
			((gain[37] >> 3) & 0x01) << 4 |	
			((gain[37] >> 2) & 0x01) << 5 |
			((gain[37] >> 1) & 0x01) << 6 |
			((gain[37] >> 0) & 0x01) << 7; 
			
			maroc_config_data[54] =	(           0x00 & 0x01) << 0 |// CMD_SUM36
			((gain[36] >> 7) & 0x01) << 1 |
			((gain[36] >> 6) & 0x01) << 2 |
			((gain[36] >> 5) & 0x01) << 3 |
			((gain[36] >> 4) & 0x01) << 4 |
			((gain[36] >> 3) & 0x01) << 5 |	
			((gain[36] >> 2) & 0x01) << 6 |
			((gain[36] >> 1) & 0x01) << 7;
			
			maroc_config_data[55] =	((gain[36] >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM35
			((gain[35] >> 7) & 0x01) << 2 |
			((gain[35] >> 6) & 0x01) << 3 |
			((gain[35] >> 5) & 0x01) << 4 |
			((gain[35] >> 4) & 0x01) << 5 |
			((gain[35] >> 3) & 0x01) << 6 |	
			((gain[35] >> 2) & 0x01) << 7;
			
			maroc_config_data[56] =	((gain[35] >> 1) & 0x01) << 0 |
			((gain[35] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM34
			((gain[34] >> 7) & 0x01) << 3 |
			((gain[34] >> 6) & 0x01) << 4 |
			((gain[34] >> 5) & 0x01) << 5 |
			((gain[34] >> 4) & 0x01) << 6 |
			((gain[34] >> 3) & 0x01) << 7;
			
			maroc_config_data[57] =	((gain[34] >> 2) & 0x01) << 0 |
			((gain[34] >> 1) & 0x01) << 1 |
			((gain[34] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM33
			((gain[33] >> 7) & 0x01) << 4 |
			((gain[33] >> 6) & 0x01) << 5 |
			((gain[33] >> 5) & 0x01) << 6 |
			((gain[33] >> 4) & 0x01) << 7;
			
			maroc_config_data[58] =	((gain[33] >> 3) & 0x01) << 0 |	
			((gain[33] >> 2) & 0x01) << 1 |
			((gain[33] >> 1) & 0x01) << 2 |
			((gain[33] >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM32
			((gain[32] >> 7) & 0x01) << 5 |
			((gain[32] >> 6) & 0x01) << 6 |
			((gain[32] >> 5) & 0x01) << 7;
			
			maroc_config_data[59] =	((gain[32] >> 4) & 0x01) << 0 |
			((gain[32] >> 3) & 0x01) << 1 |	
			((gain[32] >> 2) & 0x01) << 2 |
			((gain[32] >> 1) & 0x01) << 3 |
			((gain[32] >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM31
			((gain[31] >> 7) & 0x01) << 6 |
			((gain[31] >> 6) & 0x01) << 7;
			
			maroc_config_data[60] =	((gain[31] >> 5) & 0x01) << 0 |
			((gain[31] >> 4) & 0x01) << 1 |
			((gain[31] >> 3) & 0x01) << 2 |	
			((gain[31] >> 2) & 0x01) << 3 |
			((gain[31] >> 1) & 0x01) << 4 |
			((gain[31] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM30
			((gain[30] >> 7) & 0x01) << 7;
			
			maroc_config_data[61] =	((gain[30] >> 6) & 0x01) << 0 |
			((gain[30] >> 5) & 0x01) << 1 |
			((gain[30] >> 4) & 0x01) << 2 |
			((gain[30] >> 3) & 0x01) << 3 |	
			((gain[30] >> 2) & 0x01) << 4 |
			((gain[30] >> 1) & 0x01) << 5 |
			((gain[30] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7;// CMD_SUM29
			
			maroc_config_data[62] =	((gain[29] >> 7) & 0x01) << 0 |
			((gain[29] >> 6) & 0x01) << 1 |
			((gain[29] >> 5) & 0x01) << 2 |
			((gain[29] >> 4) & 0x01) << 3 |
			((gain[29] >> 3) & 0x01) << 4 |	
			((gain[29] >> 2) & 0x01) << 5 |
			((gain[29] >> 1) & 0x01) << 6 |
			((gain[29] >> 0) & 0x01) << 7;
			
			maroc_config_data[63] =	(           0x00 & 0x01) << 0 |// CMD_SUM28
			((gain[28] >> 7) & 0x01) << 1 |
			((gain[28] >> 6) & 0x01) << 2 |
			((gain[28] >> 5) & 0x01) << 3 |
			((gain[28] >> 4) & 0x01) << 4 |
			((gain[28] >> 3) & 0x01) << 5 |	
			((gain[28] >> 2) & 0x01) << 6 |
			((gain[28] >> 1) & 0x01) << 7;
			
			maroc_config_data[64] =	((gain[28] >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM27
			((gain[27] >> 7) & 0x01) << 2 |
			((gain[27] >> 6) & 0x01) << 3 |
			((gain[27] >> 5) & 0x01) << 4 |
			((gain[27] >> 4) & 0x01) << 5 |
			((gain[27] >> 3) & 0x01) << 6 |	
			((gain[27] >> 2) & 0x01) << 7;
			
			maroc_config_data[65] =	((gain[27] >> 1) & 0x01) << 0 |
			((gain[27] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM26
			((gain[26] >> 7) & 0x01) << 3 |
			((gain[26] >> 6) & 0x01) << 4 |
			((gain[26] >> 5) & 0x01) << 5 |
			((gain[26] >> 4) & 0x01) << 6 |
			((gain[26] >> 3) & 0x01) << 7;
			
			maroc_config_data[66] =	((gain[26] >> 2) & 0x01) << 0 |
			((gain[26] >> 1) & 0x01) << 1 |
			((gain[26] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM25
			((gain[25] >> 7) & 0x01) << 4 |
			((gain[25] >> 6) & 0x01) << 5 |
			((gain[25] >> 5) & 0x01) << 6 |
			((gain[25] >> 4) & 0x01) << 7;
			
			maroc_config_data[67] =	((gain[25] >> 3) & 0x01) << 0 |	
			((gain[25] >> 2) & 0x01) << 1 |
			((gain[25] >> 1) & 0x01) << 2 |
			((gain[25] >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM24
			((gain[24] >> 7) & 0x01) << 5 |
			((gain[24] >> 6) & 0x01) << 6 |
			((gain[24] >> 5) & 0x01) << 7;
			
			maroc_config_data[68] =	((gain[24] >> 4) & 0x01) << 0 |
			((gain[24] >> 3) & 0x01) << 1 |	
			((gain[24] >> 2) & 0x01) << 2 |
			((gain[24] >> 1) & 0x01) << 3 |
			((gain[24] >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM23
			((gain[23] >> 7) & 0x01) << 6 |
			((gain[23] >> 6) & 0x01) << 7;
			
			maroc_config_data[69] =	((gain[23] >> 5) & 0x01) << 0 |
			((gain[23] >> 4) & 0x01) << 1 |
			((gain[23] >> 3) & 0x01) << 2 |	
			((gain[23] >> 2) & 0x01) << 3 |
			((gain[23] >> 1) & 0x01) << 4 |
			((gain[23] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM22
			((gain[22] >> 7) & 0x01) << 7;
			
			maroc_config_data[70] =	((gain[22] >> 6) & 0x01) << 0 |
			((gain[22] >> 5) & 0x01) << 1 |
			((gain[22] >> 4) & 0x01) << 2 |
			((gain[22] >> 3) & 0x01) << 3 |	
			((gain[22] >> 2) & 0x01) << 4 |
			((gain[22] >> 1) & 0x01) << 5 |
			((gain[22] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7;// CMD_SUM21
			
			maroc_config_data[71] =	((gain[21] >> 7) & 0x01) << 0 |
			((gain[21] >> 6) & 0x01) << 1 |
			((gain[21] >> 5) & 0x01) << 2 |
			((gain[21] >> 4) & 0x01) << 3 |
			((gain[21] >> 3) & 0x01) << 4 |	
			((gain[21] >> 2) & 0x01) << 5 |
			((gain[21] >> 1) & 0x01) << 6 |
			((gain[21] >> 0) & 0x01) << 7;
			
			maroc_config_data[72] =	(           0x00 & 0x01) << 0 |// CMD_SUM20
			((gain[20] >> 7) & 0x01) << 1 |
			((gain[20] >> 6) & 0x01) << 2 |
			((gain[20] >> 5) & 0x01) << 3 |
			((gain[20] >> 4) & 0x01) << 4 |
			((gain[20] >> 3) & 0x01) << 5 |	
			((gain[20] >> 2) & 0x01) << 6 |
			((gain[20] >> 1) & 0x01) << 7;
			
			maroc_config_data[73] =	((gain[20] >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM19
			((gain[19] >> 7) & 0x01) << 2 |
			((gain[19] >> 6) & 0x01) << 3 |
			((gain[19] >> 5) & 0x01) << 4 |
			((gain[19] >> 4) & 0x01) << 5 |
			((gain[19] >> 3) & 0x01) << 6 |	
			((gain[19] >> 2) & 0x01) << 7;
			
			maroc_config_data[74] =	((gain[19] >> 1) & 0x01) << 0 |
			((gain[19] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM18
			((gain[18] >> 7) & 0x01) << 3 |
			((gain[18] >> 6) & 0x01) << 4 |
			((gain[18] >> 5) & 0x01) << 5 |
			((gain[18] >> 4) & 0x01) << 6 |
			((gain[18] >> 3) & 0x01) << 7;
			
			maroc_config_data[75] =	((gain[18] >> 2) & 0x01) << 0 |
			((gain[18] >> 1) & 0x01) << 1 |
			((gain[18] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM17
			((gain[17] >> 7) & 0x01) << 4 |
			((gain[17] >> 6) & 0x01) << 5 |
			((gain[17] >> 5) & 0x01) << 6 |
			((gain[17] >> 4) & 0x01) << 7;
			
			maroc_config_data[76] =	((gain[17] >> 3) & 0x01) << 0 |	
			((gain[17] >> 2) & 0x01) << 1 |
			((gain[17] >> 1) & 0x01) << 2 |
			((gain[17] >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM16
			((gain[16] >> 7) & 0x01) << 5 |
			((gain[16] >> 6) & 0x01) << 6 |
			((gain[16] >> 5) & 0x01) << 7;
			
			maroc_config_data[77] =	((gain[16] >> 4) & 0x01) << 0 |
			((gain[16] >> 3) & 0x01) << 1 |	
			((gain[16] >> 2) & 0x01) << 2 |
			((gain[16] >> 1) & 0x01) << 3 |
			((gain[16] >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM15
			((gain[15] >> 7) & 0x01) << 6 |
			((gain[15] >> 6) & 0x01) << 7;
			
			maroc_config_data[78] =	((gain[15] >> 5) & 0x01) << 0 |
			((gain[15] >> 4) & 0x01) << 1 |
			((gain[15] >> 3) & 0x01) << 2 |	
			((gain[15] >> 2) & 0x01) << 3 |
			((gain[15] >> 1) & 0x01) << 4 |
			((gain[15] >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM14
			((gain[14] >> 7) & 0x01) << 7;
			
			maroc_config_data[79] =	((gain[14] >> 6) & 0x01) << 0 |
			((gain[14] >> 5) & 0x01) << 1 |
			((gain[14] >> 4) & 0x01) << 2 |
			((gain[14] >> 3) & 0x01) << 3 |	
			((gain[14] >> 2) & 0x01) << 4 |
			((gain[14] >> 1) & 0x01) << 5 |
			((gain[14] >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7;// CMD_SUM13
			
			maroc_config_data[80] =	((gain[13] >> 7) & 0x01) << 0 |
			((gain[13] >> 6) & 0x01) << 1 |
			((gain[13] >> 5) & 0x01) << 2 |
			((gain[13] >> 4) & 0x01) << 3 |
			((gain[13] >> 3) & 0x01) << 4 |	
			((gain[13] >> 2) & 0x01) << 5 |
			((gain[13] >> 1) & 0x01) << 6 |
			((gain[13] >> 0) & 0x01) << 7;
			
			maroc_config_data[81] =	(           0x00 & 0x01) << 0 |// CMD_SUM12
			((gain[12] >> 7) & 0x01) << 1 |
			((gain[12] >> 6) & 0x01) << 2 |
			((gain[12] >> 5) & 0x01) << 3 |
			((gain[12] >> 4) & 0x01) << 4 |
			((gain[12] >> 3) & 0x01) << 5 |	
			((gain[12] >> 2) & 0x01) << 6 |
			((gain[12] >> 1) & 0x01) << 7;
			
			maroc_config_data[82] =	((gain[12] >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM11
			((gain[11] >> 7) & 0x01) << 2 |
			((gain[11] >> 6) & 0x01) << 3 |
			((gain[11] >> 5) & 0x01) << 4 |
			((gain[11] >> 4) & 0x01) << 5 |
			((gain[11] >> 3) & 0x01) << 6 |	
			((gain[11] >> 2) & 0x01) << 7;
			
			maroc_config_data[83] =	((gain[11] >> 1) & 0x01) << 0 |
			((gain[11] >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM10
			((gain[10] >> 7) & 0x01) << 3 |
			((gain[10] >> 6) & 0x01) << 4 |
			((gain[10] >> 5) & 0x01) << 5 |
			((gain[10] >> 4) & 0x01) << 6 |
			((gain[10] >> 3) & 0x01) << 7;
			
			maroc_config_data[84] =	((gain[10] >> 2) & 0x01) << 0 |
			((gain[10] >> 1) & 0x01) << 1 |
			((gain[10] >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM9
			((gain[9]  >> 7) & 0x01) << 4 |
			((gain[9]  >> 6) & 0x01) << 5 |
			((gain[9]  >> 5) & 0x01) << 6 |
			((gain[9]  >> 4) & 0x01) << 7;
			
			maroc_config_data[85] =	((gain[9]  >> 3) & 0x01) << 0 |	
			((gain[9]  >> 2) & 0x01) << 1 |
			((gain[9]  >> 1) & 0x01) << 2 |
			((gain[9]  >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM8
			((gain[8]  >> 7) & 0x01) << 5 |
			((gain[8]  >> 6) & 0x01) << 6 |
			((gain[8]  >> 5) & 0x01) << 7;
			
			maroc_config_data[86] =	((gain[8]  >> 4) & 0x01) << 0 |
			((gain[8]  >> 3) & 0x01) << 1 |	
			((gain[8]  >> 2) & 0x01) << 2 |
			((gain[8]  >> 1) & 0x01) << 3 |
			((gain[8]  >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// CMD_SUM7
			((gain[7]  >> 7) & 0x01) << 6 |
			((gain[7]  >> 6) & 0x01) << 7;
			
			maroc_config_data[87] =	((gain[7]  >> 5) & 0x01) << 0 |
			((gain[7]  >> 4) & 0x01) << 1 |
			((gain[7]  >> 3) & 0x01) << 2 |	
			((gain[7]  >> 2) & 0x01) << 3 |
			((gain[7]  >> 1) & 0x01) << 4 |
			((gain[7]  >> 0) & 0x01) << 5 |
			(           0x00 & 0x01) << 6 |// CMD_SUM6
			((gain[6]  >> 7) & 0x01) << 7;
			
			maroc_config_data[88] =	((gain[6]  >> 6) & 0x01) << 0 |
			((gain[6]  >> 5) & 0x01) << 1 |
			((gain[6]  >> 4) & 0x01) << 2 |
			((gain[6]  >> 3) & 0x01) << 3 |	
			((gain[6]  >> 2) & 0x01) << 4 |
			((gain[6]  >> 1) & 0x01) << 5 |
			((gain[6]  >> 0) & 0x01) << 6 |
			(           0x00 & 0x01) << 7;// CMD_SUM5
			
			maroc_config_data[89] =	((gain[5]  >> 7) & 0x01) << 0 |
			((gain[5]  >> 6) & 0x01) << 1 |
			((gain[5]  >> 5) & 0x01) << 2 |
			((gain[5]  >> 4) & 0x01) << 3 |
			((gain[5]  >> 3) & 0x01) << 4 |	
			((gain[5]  >> 2) & 0x01) << 5 |
			((gain[5]  >> 1) & 0x01) << 6 |
			((gain[5]  >> 0) & 0x01) << 7;
			
			maroc_config_data[90] =	(           0x00 & 0x01) << 0 |// CMD_SUM4
			((gain[4]  >> 7) & 0x01) << 1 |
			((gain[4]  >> 6) & 0x01) << 2 |
			((gain[4]  >> 5) & 0x01) << 3 |
			((gain[4]  >> 4) & 0x01) << 4 |
			((gain[4]  >> 3) & 0x01) << 5 |	
			((gain[4]  >> 2) & 0x01) << 6 |
			((gain[4]  >> 1) & 0x01) << 7;
			
			maroc_config_data[91] =	((gain[4]  >> 0) & 0x01) << 0 |
			(           0x00 & 0x01) << 1 |// CMD_SUM3
			((gain[3]  >> 7) & 0x01) << 2 |
			((gain[3]  >> 6) & 0x01) << 3 |
			((gain[3]  >> 5) & 0x01) << 4 |
			((gain[3]  >> 4) & 0x01) << 5 |
			((gain[3]  >> 3) & 0x01) << 6 |	
			((gain[3]  >> 2) & 0x01) << 7;
			
			maroc_config_data[92] =	((gain[3]  >> 1) & 0x01) << 0 |
			((gain[3]  >> 0) & 0x01) << 1 |
			(           0x00 & 0x01) << 2 |// CMD_SUM2
			((gain[2]  >> 7) & 0x01) << 3 |
			((gain[2]  >> 6) & 0x01) << 4 |
			((gain[2]  >> 5) & 0x01) << 5 |
			((gain[2]  >> 4) & 0x01) << 6 |
			((gain[2]  >> 3) & 0x01) << 7;
			
			maroc_config_data[93] =	((gain[2]  >> 2) & 0x01) << 0 |
			((gain[2]  >> 1) & 0x01) << 1 |
			((gain[2]  >> 0) & 0x01) << 2 |
			(           0x00 & 0x01) << 3 |// CMD_SUM1
			((gain[1]  >> 7) & 0x01) << 4 |
			((gain[1]  >> 6) & 0x01) << 5 |
			((gain[1]  >> 5) & 0x01) << 6 |
			((gain[1]  >> 4) & 0x01) << 7;
			
			maroc_config_data[94] =	((gain[1]  >> 3) & 0x01) << 0 |	
			((gain[1]  >> 2) & 0x01) << 1 |
			((gain[1]  >> 1) & 0x01) << 2 |
			((gain[1]  >> 0) & 0x01) << 3 |
			(           0x00 & 0x01) << 4 |// CMD_SUM0
			((gain[0]  >> 7) & 0x01) << 5 |
			((gain[0]  >> 6) & 0x01) << 6 |
			((gain[0]  >> 5) & 0x01) << 7;
			
			maroc_config_data[95] =	((gain[0]  >> 4) & 0x01) << 0 |
			((gain[0]  >> 3) & 0x01) << 1 |	
			((gain[0]  >> 2) & 0x01) << 2 |
			((gain[0]  >> 1) & 0x01) << 3 |
			((gain[0]  >> 0) & 0x01) << 4 |
			(           0x00 & 0x01) << 5 |// Ctest_ch63
			(           0x00 & 0x01) << 6 |// Ctest_ch62							
			(           0x00 & 0x01) << 7 ;// Ctest_ch61
			
			
			maroc_config_data[96]  = 0x00; //Ctest_ch60 to Ctest_ch53
			maroc_config_data[97]  = 0x00; //Ctest_ch52 to Ctest_ch45
			maroc_config_data[98]  = 0x00; //Ctest_ch44 to Ctest_ch37
			maroc_config_data[99]  = 0x00; //Ctest_ch36 to Ctest_ch29
			maroc_config_data[100] = 0x00; //Ctest_ch28 to Ctest_ch21
			maroc_config_data[101] = 0x00; //Ctest_ch20 to Ctest_ch13
			maroc_config_data[102] = 0x00; //Ctest_ch12 to Ctest_ch5
			
			maroc_config_data[103] =(           0x00 & 0x01) << 0 |// Ctest_ch4
			(           0x00 & 0x01) << 1 |// Ctest_ch3							
			(           0x00 & 0x01) << 2 |// Ctest_ch2
			(           0x00 & 0x01) << 3 |// Ctest_ch1							
			(           0x00 & 0x01) << 4 |// Ctest_ch0
			(           0x00 & 0x01) << 5 |
			(           0x00 & 0x01) << 6 | 
			(           0x00 & 0x01) << 7; 	
			
			maroc_config_data[104] = 0x00; 
		}
		
		else {
			MRC_ERR("Problems with ASIC REVISION!\n");
		}
		
	}
	
	// What follow is for all version of asic
	for (int i =0 ; i<105; i++) {
		if (i==0) {
			//printf("ConfReg2 [0][1][2]=0x%02X",fe_config_register2[i]);
			//printf("MarocSlowControl (hex):\n%02X",maroc_config_data[i]);
		}else {
			//printf("%02X",maroc_config_data[i]);
		}
		if (i%35==34) {
			//printf("\n");
		}
	}
	return true;	
}



bool	MRChi::ConfigureFrontEnd(GI_Config *cfg,int lmrc_count)
{	
	if (cfg == NULL) {MRC_ERR("Configuration object is NULL...Exiting\n");return false;}
	this->PrintConfiguration(cfg,lmrc_count);
	int n;
	bool success;
	
	
	for(int maroc_index=0; maroc_index < lmrc_count; maroc_index++){

		MRC_DBG("FE %d \n", maroc_index);
		
		this->MakeConfReg1			(cfg,maroc_index);		
		this->MakeConfReg2			(cfg,maroc_index);
		this->MakeMarocSlowControl	(cfg,maroc_index); // with optional gain map from extern file
		this->MakeDaqThresholdReg	(cfg,maroc_index); // with optional thr map extern file
		this->MakeHitEnabledMask	(cfg,maroc_index);
		
		// SOFT RESET 
		success = fe_soft_reset();
		if(!success){MRC_WRN("Soft Reset problem\n");} 
		Sleep(10);
		
		// SLOW CONTROL
		//n = fConfigLen(fRevisionFW[maroc_index]);
		
		n = ConfigLen(fASIC[maroc_index]); // 2013 July 7th, Matteo 
		success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_MAROC_CONFIG_DATA, maroc_config_data, n);
		MRC_DBG("Maroc %d config data = 0x%x 0x%x\n", fBaseAddr[maroc_index]/16,fBaseAddr[maroc_index], FEADDR_MAROC_CONFIG_DATA);
		Sleep(10);
		
		
		// START FW SUBROUTINE FOR MAROC SLOW CONTROL 
		n = 3;
		success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_COMMAND, maroc_config_cmd, n);
		Sleep(10);
		if(!success){MRC_ERR("An Error occurred with SpectWrite during Configuration of MAROC chip\n");} 
		

		// CONFIGURATION REGISTER,THR AND HIT ENABLING
		if( fRevisionFW[maroc_index] == FE_REV1 ){
			fe_config_register[1] |= 0x08; 
			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_CONFIG_REG, fe_config_register, n);
		}else{	// V2  o V3
			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_CONFIG_REG, fe_config_register_vdd_fsb, n); // necessary?

			for(int i=0; i<64; i++){
				n = 3;
				success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_THR_FIFO, DaqThreshold+(i*3), n);
				if(!success){printf("Problemi con SpectWrite : DaqThreshold+(i*3) \n");} //mat debug
			}

			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_HITEN0_REG, fe_hiten0_register, n);
			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_HITEN1_REG, fe_hiten1_register, n);
			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_HITEN2_REG, fe_hiten2_register, n);
			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_CONFIG2_REG, fe_config_register2,n);
			n = 3;
			success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_CONFIG_REG, fe_config_register, n);
		
		}
		Sleep(10);
		
		// Command the FPGA to start configuration subroutines  
	
		n = 3;
		success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_COMMAND, delay_config_cmd, n);
		Sleep(10);
		
		n = 3;
		success = fMlo->SpectWrite(fBaseAddr[maroc_index]+FEADDR_COMMAND, out_ch_config_cmd, n);
		Sleep(10);
	}		
	// Saving Geoaddress information
	
	MRC_MSG("Adding fe-geo map to configuration data...\n");
	cfg->AddList("fenum",getCount());	

	for(int i=0; i<getCount(); i++){
		cfg->AddList("geo",getBaseAddr(i)/16);
		MRC_MSG("feID %2d = geo %2d, BP %d, slot %2d\n",i,getBaseAddr(i)/16,(getBaseAddr(i)/16)/16,(getBaseAddr(i)/16)%16);
	}
	return true;
};


bool MRChi::fe_dummy_acq() {

  int i;
  int n;
  bool success;


  for(i=0; i<getCount(); i++){ 
    n = 3;
    success = fMlo->SpectFeWrite(getBaseAddr(i)+getCommand(), single_acq_cmd, n);
    if( !success ){
      MRC_ERR("USB error during ADC startup %d\n", i);
      return false;
    }
  }

  return true;

}




void MRChi::RandomConfigVectors() 
{
  int i;
  for (i=0;i<105;i++){
    maroc_config_data[i] = (unsigned char) rand()*255; 
  }

}

double MRChi::GetAnalogGain(unsigned char digital_gain){

  double G = 0.0;
  G = (digital_gain&0xFF)*0.015625;
  /*
  if( (digital_gain&0x80) == 0x80 ){G += 2.000000; };
  if( (digital_gain&0x40) == 0x40 ){G += 1.000000; }; 
  if( (digital_gain&0x20) == 0x20 ){G += 0.500000; }; 
  if( (digital_gain&0x10) == 0x10 ){G += 0.250000; }; // 1/4
  if( (digital_gain&0x08) == 0x08 ){G += 0.125000; }; // 1/8
  if( (digital_gain&0x04) == 0x04 ){G += 0.062500; }; // 1/16
  if( (digital_gain&0x02) == 0x02 ){G += 0.031250; }; // 1/32
  if( (digital_gain&0x01) == 0x01 ){G += 0.015625; }; // 1/64
  */
  return G;
};



unsigned char MRChi::DecodeGain(unsigned char maroc_rev,unsigned char * maroc_config_data,unsigned char first_byte,unsigned char type){

  unsigned char lgain = 0;
  
  if(maroc_rev==3){
    unsigned char b0,b1,b2,b3,b4,b5,b6,b7;
    switch(type){
    case (0):
      b7 = (maroc_config_data[first_byte	] & 0x02) << 6;
      b6 = (maroc_config_data[first_byte	] & 0x04) << 4;
      b5 = (maroc_config_data[first_byte	] & 0x08) << 2;
      b4 = (maroc_config_data[first_byte	] & 0x10) << 0;
      b3 = (maroc_config_data[first_byte	] & 0x20) >> 2;
      b2 = (maroc_config_data[first_byte	] & 0x40) >> 4;
      b1 = (maroc_config_data[first_byte	] & 0x80) >> 6;
      b0 = (maroc_config_data[first_byte+1] & 0x01);	 // no need translation!		
      break;
    case (1):
      b7 = (maroc_config_data[first_byte	] & 0x04) << 5;
      b6 = (maroc_config_data[first_byte	] & 0x08) << 3;
      b5 = (maroc_config_data[first_byte	] & 0x10) << 1;
      b4 = (maroc_config_data[first_byte	] & 0x20) >> 1;
      b3 = (maroc_config_data[first_byte	] & 0x40) >> 3;
      b2 = (maroc_config_data[first_byte	] & 0x80) >> 5;
      b1 = (maroc_config_data[first_byte+1] & 0x01) << 1 ;
      b0 = (maroc_config_data[first_byte+1] & 0x02) >> 1;	 			
      break;
    case (2):
      b7 = (maroc_config_data[first_byte	] & 0x08) << 4;
      b6 = (maroc_config_data[first_byte	] & 0x10) << 2;
      b5 = (maroc_config_data[first_byte	] & 0x20) << 0;
      b4 = (maroc_config_data[first_byte	] & 0x40) >> 2;
      b3 = (maroc_config_data[first_byte	] & 0x80) >> 4;
      b2 = (maroc_config_data[first_byte+1] & 0x01) << 2;
      b1 = (maroc_config_data[first_byte+1] & 0x02) >> 0;
      b0 = (maroc_config_data[first_byte+1] & 0x04) >> 2;
      break;
    case (3):
      b7 = (maroc_config_data[first_byte	] & 0x10) << 3;
      b6 = (maroc_config_data[first_byte	] & 0x20) << 1;
      b5 = (maroc_config_data[first_byte	] & 0x40) >> 1;
      b4 = (maroc_config_data[first_byte	] & 0x80) >> 3;
      b3 = (maroc_config_data[first_byte+1] & 0x01) << 3;
      b2 = (maroc_config_data[first_byte+1] & 0x02) << 1;
      b1 = (maroc_config_data[first_byte+1] & 0x04) >> 1;
      b0 = (maroc_config_data[first_byte+1] & 0x08) >> 3;
      break;
    case (4):
      b7 = (maroc_config_data[first_byte	] & 0x20) << 2;
      b6 = (maroc_config_data[first_byte	] & 0x40) << 0;
      b5 = (maroc_config_data[first_byte	] & 0x80) >> 2;
      b4 = (maroc_config_data[first_byte+1] & 0x01) << 4;
      b3 = (maroc_config_data[first_byte+1] & 0x02) << 2;
      b2 = (maroc_config_data[first_byte+1] & 0x04) >> 0;
      b1 = (maroc_config_data[first_byte+1] & 0x08) >> 2;
      b0 = (maroc_config_data[first_byte+1] & 0x10) >> 4;
      break;
    case (5):
      b7 = (maroc_config_data[first_byte	] & 0x40) << 1;
      b6 = (maroc_config_data[first_byte	] & 0x80) >> 1;
      b5 = (maroc_config_data[first_byte+1] & 0x01) << 5;
      b4 = (maroc_config_data[first_byte+1] & 0x02) << 3;
      b3 = (maroc_config_data[first_byte+1] & 0x04) << 1;
      b2 = (maroc_config_data[first_byte+1] & 0x08) >> 1;
      b1 = (maroc_config_data[first_byte+1] & 0x10) >> 3;
      b0 = (maroc_config_data[first_byte+1] & 0x20) >> 5;
      break;
    case (6):
      b7 = (maroc_config_data[first_byte	] & 0x80) << 0;
      b6 = (maroc_config_data[first_byte+1] & 0x01) << 6;
      b5 = (maroc_config_data[first_byte+1] & 0x02) << 4;
      b4 = (maroc_config_data[first_byte+1] & 0x04) << 2;
      b3 = (maroc_config_data[first_byte+1] & 0x08) >> 0;
      b2 = (maroc_config_data[first_byte+1] & 0x10) >> 2;
      b1 = (maroc_config_data[first_byte+1] & 0x20) >> 4;
      b0 = (maroc_config_data[first_byte+1] & 0x40) >> 6;
				break;
    case (7):
      b7 = (maroc_config_data[first_byte+1] & 0x01) << 7;
      b6 = (maroc_config_data[first_byte+1] & 0x02) << 5;
      b5 = (maroc_config_data[first_byte+1] & 0x04) << 3;
      b4 = (maroc_config_data[first_byte+1] & 0x08) << 1;
      b3 = (maroc_config_data[first_byte+1] & 0x10) >> 1;
      b2 = (maroc_config_data[first_byte+1] & 0x20) >> 3;
      b1 = (maroc_config_data[first_byte+1] & 0x40) >> 5;
      b0 = (maroc_config_data[first_byte+1] & 0x80) >> 7;			
      break;
    } // end of switch statement s
    lgain = b0|b1|b2|b3|b4|b5|b6|b7;
  }//end of MAROC 3
  return lgain;
}

bool MRChi::DecodeCmdSum(unsigned char maroc_rev,unsigned char * maroc_config_data,unsigned char first_byte,unsigned char type){
  char lcmdsum = 0x00;

  lcmdsum = maroc_config_data[first_byte] & (1<<type);
  
  if(lcmdsum==0){
    return false;
  }else{
    return true;
  }

}


void MRChi::ReadGain(unsigned char maroc_rev,unsigned char * maroc_config_data,unsigned char channel){

  if(channel>63){

    printf("DecodeGain argument error: channel out of range [0..63], you have %d",channel);

  } else{
    
    unsigned char lgain = 0;
    bool lcmdSUM		= false;
    unsigned char firstbyte = 0;	// for example if information is written in byte 23 and 24 of SC vector first byte is 23
    unsigned char typ	= 0;		// one of the 8 encoding mode of gain and cmd sum in the SC vector 0..7 (i.e. encoded by CMD sum bit)
    
    if(maroc_rev==3){
      firstbyte = ((63 - channel)*9 + 189)/8; // maroc_config_data vector index
      typ = ((63 - channel)*9 + 189) % 8; // bit of the CMD_SUM
    }
    if(maroc_rev==2){
      // to be defined...
    }
    
    // Decode GAIN
    lgain = DecodeGain(maroc_rev,maroc_config_data,firstbyte,typ);
    // Decode CMD_sum
    lcmdSUM = DecodeCmdSum(maroc_rev,maroc_config_data,firstbyte,typ);
    // Output 
    printf("g(%02d) = %3d ; G = %1.2f ",channel,lgain,GetAnalogGain(lgain));
    if(lcmdSUM){
      MRC_DBG("Sum path enabled for this channel\n");
    }else{
      MRC_DBG("\n");
    }
  } // else
};


void MRChi::DecodeDAC(unsigned char maroc_rev,unsigned char * maroc_config_data){
  if(maroc_rev==3){
    // MSB enter first,so words must be overturned
    unsigned short mydac1 = ((maroc_config_data[0]&0x08)<<6)| // bit 9 
      ((maroc_config_data[0]&0x10)<<4)| // bit 8 
      ((maroc_config_data[0]&0x20)<<2)| // bit 7
      ((maroc_config_data[0]&0x40)<<0)| // bit 6
      ((maroc_config_data[0]&0x80)>>2)| // bit 5
      ((maroc_config_data[1]&0x01)<<4)| // bit 4
      ((maroc_config_data[1]&0x02)<<2)| // bit 3
      ((maroc_config_data[1]&0x04)<<0)| // bit 2
      ((maroc_config_data[1]&0x08)>>2)| // bit 1
      ((maroc_config_data[1]&0x10)>>4); // bit 0
    
    unsigned short mydac0 = ((maroc_config_data[1]&0x20)<<4)| // bit 9 
      ((maroc_config_data[1]&0x40)<<2)| // bit 8 
      ((maroc_config_data[1]&0x80)<<0)| // bit 7
      ((maroc_config_data[2]&0x01)<<6)| // bit 6
      ((maroc_config_data[2]&0x02)<<4)| // bit 5
      ((maroc_config_data[2]&0x04)<<2)| // bit 4
      ((maroc_config_data[2]&0x08)<<0)| // bit 3
      ((maroc_config_data[2]&0x10)>>2)| // bit 2
      ((maroc_config_data[2]&0x20)>>4)| // bit 1
      ((maroc_config_data[2]&0x40)>>6); // bit 0
    
    
    printf("DAC0 = %4d [DAC unit],0x%X\n",mydac0,mydac0);
	printf("DAC1 = %4d [DAC unit],0x%X\n",mydac1,mydac1);
  }// end of MAROC3 
  if(maroc_rev==2){
    
    unsigned int dac21 = (maroc_config_data[2]&0xFC)>>2; // no need to overturn JUST traslation , LSB enter first, (LSB is to right of MSB)
    unsigned int dac22 = (maroc_config_data[3]&0xFF)<<6;
    unsigned int dac23 = (maroc_config_data[4]&0xFF)<<14;
    unsigned int dac24 = (maroc_config_data[5]&0x03)<<22;
    
    unsigned int mydac2 = (dac24|dac23|dac22|dac21)&0x00FFFFFF;
    
    
    printf("DAC2 = 0x%08X, (%d)\n",mydac2,mydac2);
    printf("DAC2 thermometer = %d\n",mydac2>>8);
    printf("DAC2 mirror      = %d\n",mydac2&0x000000FF);
    
    
    unsigned int dac11 = (maroc_config_data[5]&0xFC)>>2;
    unsigned int dac12 = (maroc_config_data[6]&0xFF)<<6;
    unsigned int dac13 = (maroc_config_data[7]&0xFF)<<14;
    unsigned int dac14 = (maroc_config_data[8]&0x03)<<22;
    
    unsigned int mydac1 = (dac14|dac13|dac12|dac11)&0x00FFFFFF;
    
    
    printf("DAC1 = 0x%08X (%d)\n",mydac1,mydac1);
    printf("DAC1 thermometer = %d\n",mydac1>>8);
    printf("DAC1 mirror      = %d\n",mydac1&0x000000FF);
    
    unsigned int dac01 = (maroc_config_data[8]&0xFC)>>2;
    unsigned int dac02 = (maroc_config_data[9]&0xFF)<<6;
    unsigned int dac03 = (maroc_config_data[10]&0xFF)<<14;
    unsigned int dac04 = (maroc_config_data[11]&0x03)<<22;
    
    unsigned int mydac0 = (dac04|dac03|dac02|dac01)&0x00FFFFFF;
    
    printf("DAC0 = 0x%08X (%d)\n",mydac0,mydac0);
    printf("DAC0 thermometer = %d \n",mydac0>>8);
    printf("DAC0 mirror      = %d \n",mydac0&0x000000FF);
  }// end of MAROC2
};


void MRChi::DecodeSlowControl(unsigned char maroc_rev,unsigned char * maroc_config_data)
{
  if(maroc_rev==3){ // MAROC3 41 bit
    
    double RCBuf_Capacitor  = 0.0; // pF, from 0.25 to 3.75
    double SS_fdbk_Capacitor = 0.0; // pF, from 0.30 to 2.10 
    bool SS_switch1 = false; // cmd_ss
    bool SS_switch2 = false; // cmd_ss
    
    if((maroc_config_data[0]&0x01)==0x01){printf("en_otabg			= 1\n");}	
    if((maroc_config_data[0]&0x02)==0x02){printf("en_dac			= 1\n");}
    if((maroc_config_data[0]&0x04)==0x04){printf("small_dac			= 1\n");}
    if((maroc_config_data[2]&0x80)==0x80){printf("enb_out_adc		= 1\n");} 
    if((maroc_config_data[3]&0x01)==0x01){printf("inv_startCmptGray = 1\n");} 
    if((maroc_config_data[3]&0x02)==0x02){printf("ramp  8 bit		= 1\n");} 
    if((maroc_config_data[3]&0x04)==0x04){printf("ramp 10 bit		= 1\n");} 
    if((maroc_config_data[19]&0x08)==0x08){printf("cmd_CK_mux		= 1\n");} 
    if((maroc_config_data[19]&0x10)==0x10){printf("d1_d2			= 1\n");} 
    if((maroc_config_data[19]&0x20)==0x20){printf("inv_discriADC	= 1\n");} 
    if((maroc_config_data[19]&0x40)==0x40){printf("polar_discri		= 1\n");} 
    if((maroc_config_data[19]&0x80)==0x80){printf("Enb_tristate		= 1\n");} 
    if((maroc_config_data[20]&0x01)==0x01){printf("valid_dc_fsb2	= 1\n");}  
    if((maroc_config_data[20]&0x02)==0x02){printf("sw_fsb2_50f		= 1\n");}  
    if((maroc_config_data[20]&0x04)==0x04){printf("sw_fsb2_100f		= 1\n");}  
    if((maroc_config_data[20]&0x08)==0x08){printf("sw_fsb2_100k		= 1\n");}  
    if((maroc_config_data[20]&0x10)==0x10){printf("sw_fsb2_50k		= 1\n");} 
    if((maroc_config_data[20]&0x20)==0x20){printf("valid_dc_fs		= 1\n");}  
    if((maroc_config_data[20]&0x40)==0x40){printf("cmd_fsb_fsu		= 1\n");}  
    if((maroc_config_data[20]&0x80)==0x80){printf("sw_fsb1_50f		= 1\n");}  
    if((maroc_config_data[21]&0x01)==0x01){printf("sw_fsb1_100f		= 1\n");}  
    if((maroc_config_data[21]&0x02)==0x02){printf("sw_fsb1_100k		= 1\n");}  
    if((maroc_config_data[21]&0x04)==0x04){printf("sw_fsb1_50k		= 1\n");}  
    if((maroc_config_data[21]&0x08)==0x08){printf("sw_fsu_100k		= 1\n");}  
    if((maroc_config_data[21]&0x10)==0x10){printf("sw_fsu_50k		= 1\n");} 
    if((maroc_config_data[21]&0x20)==0x20){printf("sw_fsu_25k		= 1\n");}  
    if((maroc_config_data[21]&0x40)==0x40){printf("sw_fsu_40f		= 1\n");}  
    if((maroc_config_data[21]&0x80)==0x80){printf("sw_fsu_20f		= 1\n");}  
    if((maroc_config_data[22]&0x01)==0x01){printf("H1H2_choice		= 1 (HOLD1)\n");}  
    if((maroc_config_data[22]&0x02)==0x02){printf("EN_ADC			= 1\n");}  
    if((maroc_config_data[22]&0x04)==0x04){printf("sw_ss_1200f		= 1\n");	SS_fdbk_Capacitor += 1.200; }  
    if((maroc_config_data[22]&0x08)==0x08){printf("sw_ss_600f		= 1\n");	SS_fdbk_Capacitor += 0.600; }  
    if((maroc_config_data[22]&0x10)==0x10){printf("sw_ss_300f		= 1\n");	SS_fdbk_Capacitor += 0.300; } 
    if((maroc_config_data[22]&0x20)==0x20){printf("ON/OFF_ss		= 1\n");	SS_switch2 = true;}  // what is this? What Relation with cmd_ss?
    if((maroc_config_data[22]&0x40)==0x40){printf("swb_buf_2p		= 1\n"); 	RCBuf_Capacitor += 2.000;}  
    if((maroc_config_data[22]&0x80)==0x80){printf("swb_buf_1p		= 1\n");	RCBuf_Capacitor += 1.000;}  
    if((maroc_config_data[23]&0x01)==0x01){printf("swb_buf_500f		= 1\n");	RCBuf_Capacitor += 0.500;}  
    if((maroc_config_data[23]&0x02)==0x02){printf("swb_buf_250f		= 1\n");	RCBuf_Capacitor += 0.250;}  
    if((maroc_config_data[23]&0x04)==0x04){printf("cmd_fsb			= 1\n");}  
    if((maroc_config_data[23]&0x08)==0x08){printf("cmd_ss			= 1\n");	SS_switch1 = true;}  // this switch must be closed for charge measurements!  
    if((maroc_config_data[23]&0x10)==0x10){printf("cmd_fsu			= 1\n");} 
    
    if(!SS_switch1||!SS_switch2){ // Slow Shaper Path
      printf("Warning: not all Slow Shaper Switch are closed!\n");
    }else{
      printf("Slow Path Enabled\n");
      if(RCBuf_Capacitor==0.0){ // RC Buffer
	printf("Warning: RCBuffer_Capacitor is zero!\ns");
      }else{
	printf("RC Buffer Capacitor is %4.2f pF (and Time constant is %3.1f ns)\n",RCBuf_Capacitor,RCBuf_Capacitor*50);
      }
      if(SS_fdbk_Capacitor==0.0){ //Slow Shaper Feedback Reactance
	printf("Warning: Slow Shaper Feedback Capacitor is zero!\n");
      }else{
	printf("Slow Shaper Feedback Capacitor is %4.2f pF (and Time constant is %3.1f ns)\n",SS_fdbk_Capacitor,SS_fdbk_Capacitor*100);
      }
    }
    printf("\n");
  } // end of Maroc3
  
  if(maroc_rev==2){ // MAROC2 - 18 bits 
    if((maroc_config_data[0]&0x01)==0x01){printf("en_ck     = 1\n");}	
    if((maroc_config_data[0]&0x02)==0x02){printf("cmd_lvds  = 1\n");}
    if((maroc_config_data[0]&0x04)==0x04){printf("en_serial = 1\n");}
    if((maroc_config_data[0]&0x08)==0x08){printf("cmd_lucid = 1\n");}
    if((maroc_config_data[0]&0x10)==0x10){printf("fs choice = 1\n");}
    if((maroc_config_data[0]&0x20)==0x20){printf("sw_20f	= 1\n");}
    if((maroc_config_data[0]&0x20)==0x40){printf("sw_40f	= 1\n");}
    if((maroc_config_data[0]&0x20)==0x80){printf("sw_25k	= 1\n");}
    
    if((maroc_config_data[1]&0x01)==0x01){printf("sw_50k	= 1\n");}	
    if((maroc_config_data[1]&0x02)==0x02){printf("sw_100k	= 1\n");}
    if((maroc_config_data[1]&0x04)==0x04){printf("sw_50f	= 1\n");}
    if((maroc_config_data[1]&0x08)==0x08){printf("sw_100f	= 1\n");}
    if((maroc_config_data[1]&0x10)==0x10){printf("en_adc	= 1\n");}
    if((maroc_config_data[1]&0x20)==0x20){printf("h1h2choice= 1\n");}
    if((maroc_config_data[1]&0x40)==0x40){printf("vld_ch_sel= 1\n");}
    if((maroc_config_data[1]&0x80)==0x80){printf("swss_c2 	= 1\n");}
    
    if((maroc_config_data[2]&0x01)==0x01){printf("swss_c1 	= 1\n");}	
    if((maroc_config_data[2]&0x02)==0x02){printf("swss_c0 	= 1\n");}
  } // end of Maroc2 
} // end of DecodeSlowControl


void MRChi::CheckSlowControl(int MFE_idx, int SClen,unsigned char * SCvect){
	
  // Reading out from ASIC the Configuration vector
  unsigned char  maroc_config_data_back[105];	
	int i=0;
  for(i= 0;i<105;i++){
    maroc_config_data_back[i]=0;
  }
  
  bool lsuccess;
  lsuccess =  fMlo->SpectRead(fBaseAddr[MFE_idx]+FEADDR_MAROC_CONFIG_DATA,maroc_config_data_back,SClen);
  if(!lsuccess){printf("An Error occurred with SpectRead while debugging MAROC SlowControl\n");} 
  
  // Print Both
  for(i = 0; i<SClen;i++){  
    if(i==0){
      printf("SlowControlVect:\n|%02X",SCvect[i]);
    }else{
      printf("|%02X",SCvect[i]);
    }
    if(i%16==15){printf("\n");}
  } 
  printf("\n");
  
  for(i = 0; i<SClen;i++){ 
    if(i==0){
      printf("SlowControlVect Read back:\n|%02X",maroc_config_data_back[i]);
    }else{
      printf("|%02X",maroc_config_data_back[i]);
    }
    if(i%16==15){printf("\n");}
  } 
  
  // check written and read versions
  
  for(i=0;i<SClen;i++){
    if((SCvect[i]&maroc_config_data_back[i])!=SCvect[i]){
      printf("\nDifferences in Byte %3d: Wrote 0x%02X Read 0x%02X",i,SCvect[i],maroc_config_data_back[i]);
    }else{
      if((SCvect[i]&maroc_config_data_back[i])==0x00){
	if((SCvect[i]!=0)||(maroc_config_data_back[i]!=0)){
	  printf("\nDifferences in Byte %3d: Wrote 0x%02X Read 0x%02X  *",i,SCvect[i],maroc_config_data_back[i]);
	}
      }
    }
  }
  printf("\n");
  
  // Re-Writing SC configuration
  lsuccess = fMlo->SpectWrite(fBaseAddr[MFE_idx]+FEADDR_MAROC_CONFIG_DATA, SCvect, SClen);
  if(!lsuccess){printf("An Error occurred with SpectWrite during Configuration of MAROC chip\n");} 
}

/* 
 * fail if != 0
 */

int MRChi::WriteReadMarocVector(int mrc_addr, int len,unsigned char * vect){

  bool success;
	int i=0;
  // write vector
  success = fMlo->SpectWrite(mrc_addr+FEADDR_MAROC_CONFIG_DATA, vect, len);
  if (!success) {
    MRC_WRN("Cannot write maroc slow control vector into FE FPGA 0x%x\n",mrc_addr);
    return -1;
  }

  // Reading out from ASIC the Configuration vector
  unsigned char  maroc_config_data_back[105];	
	
  for(i= 0;i<105;i++){maroc_config_data_back[i]=0;}
  
  success =  fMlo->SpectRead(mrc_addr+FEADDR_MAROC_CONFIG_DATA,maroc_config_data_back,len);
  if(!success){
    MRC_WRN("Cannot read maroc slow control vector from FE FPGA 0x%x\n",mrc_addr);
    return -1;
  } 
  
  // check written and read versions
  
  int icnt = 0;

  for(i=0;i<len;i++){
    if (vect[i] != maroc_config_data_back[i]) {
      icnt++;
    }
  }

  if (icnt!=0) { 
    MRC_DUM("Write - Read %3d random uchars Maroc Slow Control Data on FPGA=0x%4x: diff = %d\n",len,mrc_addr,icnt);
  }
  return icnt;

}

bool MRChi::reinit_Ctrl_and_Fe()
{	
  bool lsuccess;
  
  lsuccess = fMlo->SpectDisableGate();
#ifdef STRONG_RESET
  lsuccess = fMlo->SpectCtrlAllClear();
  lsuccess = fMlo->SpectSetTrgType(0);
  lsuccess = fMlo->SpectSetIackDelay(DEF_IACK_DELAY);
  lsuccess = fMlo->SpectEnableDaq(EVENTS_TO_READ);
  lsuccess = fMlo->SpectCtrlWrite(FE_ENABLED_MASK_REG0_ADDR, fe_mask[0]);
  lsuccess = fMlo->SpectCtrlWrite(FE_ENABLED_MASK_REG1_ADDR, fe_mask[1]);
  lsuccess = fe_configReg_write();
#else
  lsuccess = fMlo->SpectCtrlWrite(PULSE_CONTROL_REG_ADDR, CLEAR_ALL_FSM);
#endif
  
  lsuccess = fe_soft_reset();
  
  Sleep(10);
  
  lsuccess = fMlo->SpectEnableGate();
  
  return lsuccess ;
}

