/*
 *  MRCConfigurationManager.cpp
 *
 *  Author: Matteo Turisini
 *  Date: 2013 Jan 23rd
 *
 */

#include "MRCConfigurationManager.h"

MRCConfigurationManager::MRCConfigurationManager(char *file,bool analysis)
{
	// sw init
	fcfg=NULL;
	fret =0;
	ffenum=0;
	for (int idx=0; idx<MAX_NCARDS; idx++) {idxList[idx]=-1;}
		
	fcfg = new GI_Config();
	
	
	// Read config/log data from the specified file 
	ffilename = file;
	fcfg->parseFile(file); 
	
	// What follows is valid for analysis NOT for daq
	// first the number of cards and their geoaddess is read
	if (analysis) {
		ffenum = GetFeNum(); 
		for (int idx=0; idx<ffenum; idx++) {
			idxList[GetGeo(idx)]= idx;
		}
	}

}

MRCConfigurationManager::~MRCConfigurationManager(){
	if (fcfg!=NULL) {
		delete fcfg;
	}
}


int MRCConfigurationManager::GetAutoTrigger(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "auto_trigger",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetEnableZeroSuppression(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "en_zero_supp",0,"cfgReg1", fe_idx, "fe",0,"cb");		
	}
	return -1;
}


int MRCConfigurationManager::GetEnableAutoTrigger(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "en_auto_trig",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}

int MRCConfigurationManager::GetEnableExtTrigger(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "en_ext_trig",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetTestEnableEvenChannels(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "test_en_eve",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetTestEnableOddChannels(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "test_en_odd",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetInitialChannelOut(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "initial_out_ch",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetOneSixtyfour(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "one_sixtyfour",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetLed0(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "led0",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetLed1(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "led1",0,"cfgReg1", fe_idx, "fe",0,"cb");
	}
	return -1;
}

int MRCConfigurationManager::GetDaqMode(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "Daq_mode",0,"cfgReg2", fe_idx, "fe",0,"cb");	
	}
	return -1;
}


unsigned char MRCConfigurationManager::GetHitDelay(int fe_idx)
{
	if (CheckIDX(fe_idx)) {
		return (unsigned char) fcfg->get<int>(fret, 0, "hit_delay",0,"cfgReg2", fe_idx, "fe",0,"cb");	
	}
	return 0;
}


int MRCConfigurationManager::GetTrigMode(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "TrigMode",0,"cfgReg2", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetStartDaqDelay(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "StartDaqDelay",0,"cfgReg2", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetFastSettlingTime(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "FastSettlingTime",0,"cfgReg2", fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSlowSettlingTime(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "SlowSettlingTime",0,"cfgReg2", fe_idx, "fe",0,"cb");
	}
	return -1;
}

unsigned char  MRCConfigurationManager::GetHoldDelay(int idx){
	if (CheckIDX(idx)) {
		return (unsigned char) fcfg->get<int>(fret, 0, "hold1_delay",0,"cfgReg1",idx, "fe",0,"cb");
	}
	return 0;
}

int MRCConfigurationManager::GetHitEn0(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret,0,"hit_en0",0,"trigger",fe_idx, "fe",0,"cb");
	}
	return -1;
}

int MRCConfigurationManager::GetHitEn1(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret,0,"hit_en1",0,"trigger",fe_idx, "fe",0,"cb");
	}
	return -1;
}
int MRCConfigurationManager::GetHitEn2(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret,0,"hit_en2",0,"trigger",fe_idx, "fe",0,"cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSmallDac(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "small_dac"	,0,"general",0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetCmdFsbFsu(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "cmd_fsb_fsu",0,"general",0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}




int MRCConfigurationManager::GetEnableTristate(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "enb_tristate",0,"general",0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetPolarDiscri(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "polar_discri",0,"general",0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetD1D2(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "d1_d2",0,"general",0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}



int MRCConfigurationManager::GetSwFSU_100k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "sw_fsu_100k",0,"fsu",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwFSU_50k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsu_50k"	,0,"fsu",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwFSU_25k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsu_25k"	,0,"fsu",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}



int MRCConfigurationManager::GetSwFSU_40f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "sw_fsu_40f",0,"fsu",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSU_20f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsu_20f"	,0,"fsu",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}


int MRCConfigurationManager::GetSwFSB1_50k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0,"sw_fsb1_50k",0,"fsb1",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSB1_100k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsb1_100k",0,"fsb1",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSB1_100f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsb1_100f",0,"fsb1",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSB1_50f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsb1_50f",0,"fsb1",0,"fastpath",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwFSB2_50k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "sw_fsb2_50k"	,0,"fsb2"	,0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSB2_100k(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsb2_100k",0,"fsb2"	,0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSB2_100f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsb2_100f",0,"fsb2"	,0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetSwFSB2_50f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_fsb2_50f"	,0,"fsb2"	,0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetValidDC_FS(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "valid_dc_fs"	,0,"dc"		,0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetValidDC_FSB2(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "valid_dc_fsb2",0,"dc"		,0,"fastpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}


int MRCConfigurationManager::GetSwSS_1200f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_ss_1200f"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwSS_600f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_ss_600f"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwSS_300f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "sw_ss_300f"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}


int MRCConfigurationManager::GetEnableSS(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "enable_ss"	,0,"shaper"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwbBuf_2p(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "swb_buf_2p"	,0,"buffer"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwbBuf_1p(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "swb_buf_1p"	,0,"buffer"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwbBuf_250f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "swb_buf_250f",0,"buffer"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetSwbBuf_500f(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "swb_buf_500f",0,"buffer"	,0,"slowpath"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetCmdFSB(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "cmd_fsb"		,0,"preamp"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetCmdSS(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "cmd_ss"		,0,"preamp",	0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetCmdFSU(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "cmd_fsu",0,"preamp"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetEnbOutADC(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "enb_out_adc"	,0,"adc",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetInvStartGray(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "inv_start_gray",0,"adc"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetRamp_8bit(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "ramp_8bit"	,0,"adc",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetRamp_10bit(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "ramp_10bit"	,0,"adc",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}

int MRCConfigurationManager::GetH1H2Choice(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "h1h2_choice"	,0,"adc",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetEnADC(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "en_adc"		,0,"adc",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetInvDiscriADC(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return  fcfg->get<int>(fret, 0, "inv_discri_adc",0,"adc",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetEnOtaBg(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "en_otabg"	,0,"others"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetEnDAC(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "en_dac"		,0,"others"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}
int MRCConfigurationManager::GetCkMux(int fe_idx){
	if (CheckIDX(fe_idx)) {
		return fcfg->get<int>(fret, 0, "cmd_ck_mux"	,0,"others"	,0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return -1;
}


void MRCConfigurationManager::Recap()
{
	printf("Filename: %s\n",ffilename);
	printf("%s\n",GetDateString().c_str());

	if (isTrigSelf()){printf("Self Trigger Mode\n");}
	if (isTrigExt()) {printf("Ext Trigger Mode\n");}

	//GetMultievent();

	printf("%d front end card(s)\n",ffenum);
	printf("%d events" ,GetEventNumber());
	printf(" in %3.1lf seconds ",GetTimeReal());
	printf("-> Real Rate was %3.1lf Hz\n" ,GetRateReal());

	// address, fe id, mapmt, hv to be added
	printf("IDX  ADDR  DAC0  HIT DELAY  HOLD DELAY\n");
	for (int idx = 0; idx<ffenum; idx++) {
		printf("%2d    %2d   %4d     %d         %3d\n", idx,GetGeo(idx),GetDAC0(idx),GetHitDelay(idx),GetHoldDelay(idx));
	}
}

void MRCConfigurationManager::AddFeNum(int fenum){
	fcfg->AddList("fenum",fenum);	
	for (int i=0; i<fenum; i++) {
		fcfg->AddList("geo",32+i);
	}
}


void MRCConfigurationManager::AddInfoRate(int evt,float duration){
	
	fcfg->AddList("Events",evt);
	fcfg->AddListFloat("Duration",duration);	
	fcfg->AddListFloat("daqRate",evt/duration);
}

void MRCConfigurationManager::AddInfoTime(int year,
										  int month,
										  int day,
										  int hour,
										  int minute, 
										  int second){
	fcfg->AddList("Year",year);
	fcfg->AddList("Month",month);	
	fcfg->AddList("Day",day);
	fcfg->AddList("Hour",hour);
	fcfg->AddList("Minute",minute);	
	fcfg->AddList("Sec",second);
}
				 
void MRCConfigurationManager::AddProfiling(int npoll,
										   int ndataready,
										   int ntail)
	{
		fcfg->AddList("Polling",npoll);
		fcfg->AddList("DataReady",ndataready);
		fcfg->AddList("Tail",ntail);
		fcfg->AddListFloat("PollingEfficiency",((float) ndataready)/((float) npoll));		
	}


void MRCConfigurationManager::Export(char * file){
	fcfg->save(file);
	cout << "Loogbok file" << file <<endl;
}


int MRCConfigurationManager::GetEventNumber(){
	return (int) fcfg->get<int>(fret,0,"Events");
}

float MRCConfigurationManager::GetTimeReal(){
	return (float) fcfg->get<float>(fret,0,"Duration");
}

float MRCConfigurationManager::GetRateReal(){
	return (float) fcfg->get<float>(fret,0,"daqRate");
}


float MRCConfigurationManager::GetPollingEfficiency(){

	return (float) fcfg->get<float>(fret,0,"PollingEfficiency");
}


bool MRCConfigurationManager::isTrigSelf(){

	int mrc_idx =0;
	unsigned int en_auto_trig = (unsigned int) fcfg->get<int>(fret,0,"en_auto_trig",0,"cfgReg1",mrc_idx,"fe",0,"cb");
	return en_auto_trig==1? true:false;	
}
bool MRCConfigurationManager::isTrigExt(){
	int mrc_idx =0;
	unsigned int en_ext_trig = (unsigned int) fcfg->get<int>(fret,0,"en_ext_trig",0,"cfgReg1",mrc_idx,"fe",0,"cb");
	return en_ext_trig==1? true:false;	
}

std::string	MRCConfigurationManager::GetFilePrefix(){

	std::string varstring;	
	varstring = fcfg->get<std::string>(fret, 0, "file_prefix",0,"run");
	return varstring;
}

int MRCConfigurationManager::GetTimePreset(){
	
	return fcfg->get<int>(fret,0,"time_preset_sec",0,"run");

}
int	MRCConfigurationManager::GetEventPreset(){
	
	return  fcfg->get<int>(fret,0,"event_preset",0,"run");
	
}

int MRCConfigurationManager::GetDID(){
	int did= fcfg->get<int>(fret,0,"did",0,"cb");	 
	//printf("Device Identification Mask = 0x%X\n",did);
	return did;
}

int MRCConfigurationManager::GetMultievent(){
	int me= fcfg->get<int>(fret,0,"multievent",0,"cb");	 
	if (me==1) {
		printf("Single Event Mode\n");
	}
	if (me>=2) {
		printf("Multi Event Mode = %d\n",me);
	}
	
	return me;
}

std::string MRCConfigurationManager::GetDateString(){

	char year[4];
	sprintf(year,"%d",getYear());
	
	char day[2];
	int dayval = getDay();
	switch (dayval) {
		case 1:
			sprintf(day,"%dst",dayval);			
			break;
		case 2:
			sprintf(day,"%dnd",dayval);			
			break;
		case 3:
			sprintf(day,"%drd",dayval);			
			break;				
		default:
			sprintf(day,"%dth",dayval);	
			break;
	}
	char hour[2];
	sprintf(hour,"%d",getHour());

	char minute[2];
	sprintf(minute,"%d",getMinute());
	
	char buf[128];
	memset(buf,0,0);
	
	
	strcpy(buf,getPlace());
	strcat(buf,", ");	
	strcat(buf,getMonthString());
	strcat(buf," ");
	strcat(buf,day);	
	strcat(buf," ");
	strcat(buf,year);
	strcat(buf,", ");
	strcat(buf,hour);
	strcat(buf,":");	
	strcat(buf,minute);
	
	std::string str = buf;
	return str;
}

const char * MRCConfigurationManager::getPlace(){
	std::string varstring;	
	varstring = fcfg->get<std::string>(fret, 0, "at");
	return varstring.c_str();
}

unsigned int MRCConfigurationManager::getYear(){
	unsigned int val = (unsigned int) fcfg->get<int>(fret,0,"Year");
	return val;
}

unsigned int MRCConfigurationManager::getMonth(){
	unsigned int val = (unsigned int) fcfg->get<int>(fret,0,"Month");
	return val;
}

const char *  MRCConfigurationManager::getMonthString(){
	std::string string;
	unsigned int val = getMonth();
	switch (val) {
		case 1:string = "January"; break;
		case 2:string = "February"; break;
		case 3:string = "March"; break;
		case 4:string = "April"; break;
		case 5:string = "May"; break;		
		case 6:string = "June"; break;
		case 7:string = "July"; break;
		case 8:string = "August"; break;
		case 9:string = "September"; break;
		case 10:string = "October"; break;
		case 11:string = "November"; break;
		case 12:string = "December"; break;
		default:string  = "";break;
	}
	return string.c_str();
}


unsigned int MRCConfigurationManager::getDay(){
	unsigned int val =(unsigned int) fcfg->get<int>(fret,0,"Day");
	return val;
}

unsigned int MRCConfigurationManager::getHour(){
	unsigned int val =(unsigned int) fcfg->get<int>(fret,0,"Hour");
	return val;

}

unsigned int MRCConfigurationManager::getMinute(){
	unsigned int val =(unsigned int) fcfg->get<int>(fret,0,"Minute");
	return val;

}

int MRCConfigurationManager::GetIDXfrom(int geo)
{
	return (int)idxList[geo];
}
		   
unsigned char MRCConfigurationManager::GetGain(int geo, int channel) //channel = fe channel [0..63] 
{
	if ((64-channel)<=0) {
		printf("ERROR: Sorry, channel is out of range [0..63], you have %d \n",channel);
		return 0;
	}
	
	bool dummy = false;
	
	for (int i=0; i<ffenum; i++) {
		if (GetGeo(i)-geo==0) {
			dummy = true;
			break;
		}
	}
	if (!dummy) {
		printf("WARNING: Address %d is empty for this run (no card connected)\n",geo);
		return 0;
	}else {
		char gainsetting[20];
		sprintf (gainsetting, "gain_map_%d", geo);
		return  (unsigned char)fcfg->get<int>(fret,channel,gainsetting);
	}
} 


unsigned char MRCConfigurationManager::GetGeo(int idx)
{
	if (CheckIDX(idx)) {
		return (unsigned char) fcfg->get<int>(fret,idx,"geo");		
	}
	return 0;
}

float MRCConfigurationManager::GetRate()
{
	float lx = (float) fcfg->get<float>(fret,0,"daqRate");
	return lx;
}

bool MRCConfigurationManager::CheckIDX(int idx){

	if ((ffenum-idx)<=0) {
		printf("Warning: argument too big, only %d frontend card for this run, you have %d\n",ffenum, idx);
		return false;
	}
	return true; 
}


unsigned char MRCConfigurationManager::GetFeNum()
{
	unsigned char fenum = 0;
	 
	fenum = (unsigned char) fcfg->get<int>(fret,0,"fenum");
	
	return fenum;
}

unsigned int MRCConfigurationManager::GetDAC0(int fe_idx)
{
	if (CheckIDX(fe_idx)) {
		return (unsigned int) fcfg->get<int>(fret, 0, "DAC0",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return 0;
}

unsigned int MRCConfigurationManager::GetDAC1(int fe_idx)
{
	if (CheckIDX(fe_idx)) {
		return (unsigned int) fcfg->get<int>(fret, 0, "DAC1",0,"mrc3", fe_idx, "fe", 0, "cb");
	}
	return 0;
}

int	MRCConfigurationManager::GetUseGainMap (int fe_idx){
	int val;
	if (CheckIDX(fe_idx)) {
		val = fcfg->get<int>(fret, 0, "use_gain_map",0,"mrc3", fe_idx, "fe", 0, "cb");
		
		if (val==1) {
			printf("Use Gain Map\n");
		}else {
			printf("Use Default Gain Value\n");
		}

	}
	return -1;

}

unsigned char	MRCConfigurationManager::GetGainDefault(){
	return (unsigned char)fcfg->get<int>(fret, 0, "gain_default",0,"preamp",0,"mrc3", 0, "fe", 0, "cb");

}

//
void MRCConfigurationManager::Reset_TCanvas(const char * tag){
	TCanvas* dummygaincanv = (TCanvas*) gDirectory->FindObject(tag);
	if(dummygaincanv){
		delete dummygaincanv;
	}
	
}

void MRCConfigurationManager::Reset_TH1I(const char * tag){
	TH1I* dummyhisto = (TH1I*) gDirectory->FindObject(tag);
	if(dummyhisto){
		delete dummyhisto;
	}
}


void MRCConfigurationManager::ShowPreampGain()
{
	const char * cname = "Gain_Settings";
	const char * hname = "Gain_vs_Channel";
	const char * hname2 = "Gain_Distribution";
	
	Reset_TCanvas(cname);
	Reset_TH1I(hname);
	Reset_TH1I(hname2);
	
	TCanvas * gaincanv = new TCanvas(cname,ffilename,CR_PIXX-CR_SIZEOFCANV,0,CR_SIZEOFCANV,CR_SIZEOFCANV);
	TH1I * gp = new TH1I(hname,hname,4096,-0.5,4095.5);
	TH1I * gh = new TH1I(hname2,hname2, 255,-0.5, 254.5);
	
	unsigned char gword = 0;
	int channel = 0;
	int muted = 0;
	int active = 0;
	
	for (int idx=0; idx<ffenum; idx++) {
		int geo = GetGeo(idx);
		for (int ch=0; ch<64; ch++){
			gword = GetGain(geo,ch);
			if (gword!=0) { // remove muted channels
				channel = 64*geo+ch;
				gp->Fill(channel,gword);
				gh->Fill(gword);
				active++;
			}
			else {
				muted++;
			}

		}
	}
	printf("Channels : %d active %d muted\n",active,muted);
	printf("Average Gain = %3.1lf (%.1lf); ",gh->GetMean(),gh->GetMeanError());
	printf("RMS = %3.1lf (%.1lf)\n",gh->GetRMS(),gh->GetRMSError());
	
	gaincanv->Divide(2,1);	
	gaincanv->cd(1);
	gp->Draw();
	SetXYAxisTitle(gp,"Channel ","Gain Word [a.u.]");
	gaincanv->cd(2);
	gh->Draw();
	SetXYAxisTitle(gh,"Gain Word [a.u.]","Occurrency [#]");
	
	//gaincanv->DrawClone();
	//delete gaincanv;
	//delete gp;
	//delete gh;
}



void MRCConfigurationManager::SetXYAxisTitle(TH1I * histo, 
											 const char * xTitle, 
											 const char * yTitle){
	if (histo!=NULL) {
		TH1I * h = histo;
		TAxis * x = h->GetXaxis();
		x->SetTitle(xTitle);
		TAxis * y = h->GetYaxis();
		y->SetTitle(yTitle);
	}
}


void MRCConfigurationManager::ShowAnalogDelay()
{
	char cname[] = "Hold1Delay_Settings";
	char hname[] = "Hold1Delay_vs_Channel";
	char hname2[] = "Hold1Delay_Distribution";

	Reset_TCanvas(cname);
	Reset_TH1I(hname);
	Reset_TH1I(hname2);
	
	TCanvas * delaycanv = new TCanvas(cname,ffilename,CR_PIXX-CR_SIZEOFCANV,0,CR_SIZEOFCANV,CR_SIZEOFCANV);
	TH1I * dp = new TH1I(hname,hname,64,0.5,63.5);	
	TH1I * dh = new TH1I(hname2,hname2,255,0.5,254.5);

	int hold_delay=0;
	int geo= 0;
	
	for (int feidx =0; feidx<ffenum; feidx++){ 
		hold_delay =GetHoldDelay(feidx);
		geo = GetGeo(feidx);
		dp->Fill(geo,hold_delay);
		dh->Fill(hold_delay);
		printf("FE[%d] Geo = %d : Hold1_delay= %d\n",feidx,geo,hold_delay);
		
	}
	
	printf("Found %d cards\n",ffenum);
	printf("Average Gain = %3.1lf (%.1lf); ",dh->GetMean(),dh->GetMeanError());
	printf("RMS = %3.1lf (%.1lf)\n",dh->GetRMS(),dh->GetRMSError());
	
	delaycanv->Divide(2,1);	
	delaycanv->cd(1);
	dp->Draw();
	SetXYAxisTitle(dp,"Geoaddress ","Hold1 delay [a.u.]");
	delaycanv->cd(2);
	dh->Draw();
	SetXYAxisTitle(dh,"Hold1 delay ","Occurrency [#]");
	//delaycanv->Update();
	//delaycanv->SaveAs("./prova.eps");
	
}

void MRCConfigurationManager::ShowDigitalDelay()
{
	const char * cname = "HitDelay_Settings";
	const char * hname = "HitDelay_vs_Channel";
	const char * hname2 = "HitDelay_Distribution";
	
	Reset_TCanvas(cname);
	Reset_TH1I(hname);
	Reset_TH1I(hname2);
	
	TCanvas * delaycanv = new TCanvas(cname,ffilename,CR_PIXX-CR_SIZEOFCANV,0,CR_SIZEOFCANV,CR_SIZEOFCANV);
	TH1I * hp = new TH1I(hname,hname,64,-0.5,63.5);	
	TH1I * hh = new TH1I(hname2,hname2,8,-0.5,7.5);
	
	int hit_delay=0;
	int geo= 0;
	
	for (int feidx =0; feidx<ffenum; feidx++){ 
		hit_delay = GetHitDelay(feidx);
		geo = GetGeo(feidx);
		
		hp->Fill(geo,hit_delay);
		hh->Fill(hit_delay);
		printf("FE[%d] Geo = %d : Hit_delay= %d\n",feidx,geo,hit_delay);
		
	}
	
	printf("Found %d cards\n",ffenum);
	printf("Average Hit Delay = %3.1lf (%.1lf); ",hh->GetMean(),hh->GetMeanError());
	printf("RMS = %3.1lf (%.1lf)\n",hh->GetRMS(),hh->GetRMSError());
	
	delaycanv->Divide(2,1);	
	delaycanv->cd(1);
	hp->Draw();
	SetXYAxisTitle(hp,"Geoaddress ","Hit delay [a.u.]");
	delaycanv->cd(2);
	hh->Draw();
	SetXYAxisTitle(hh,"Hit delay ","Occurrency [#]");
	
}


void MRCConfigurationManager::ShowThreshold()
{
	const char * cname = "DAC0_Settings";
	const char * hname = "DAC0_vs_Channel";
	const char * hname2 = "DAC0_Distribution";
	
	Reset_TCanvas(cname);
	Reset_TH1I(hname);
	Reset_TH1I(hname2);
	
	TCanvas * DACcanv = new TCanvas(cname,ffilename,CR_PIXX-CR_SIZEOFCANV,0,CR_SIZEOFCANV,CR_SIZEOFCANV);
	TH1I * dacp = new TH1I(hname,hname,64,-0.5,63.5);	
	TH1I * dach = new TH1I(hname2,hname2,1024,-0.5,1023.5);
	
	int dac0;
	int geo;
	for (int feidx =0; feidx<ffenum; feidx++){ 
		dac0 = GetDAC0(feidx);
		geo = GetGeo(feidx);

		printf("FE[%d] Geo %d: DAC = %d\n",feidx,geo,dac0);
		dacp->Fill(geo,dac0);
		dach->Fill(dac0);
	}
	
	DACcanv->Divide(2,1);
	
	DACcanv->cd(1);
	dacp->Draw();
	SetXYAxisTitle(dacp,"Geoaddress ","DAC0 word[a.u.]");

	DACcanv->cd(2);
	dach->Draw();
	SetXYAxisTitle(dach,"DAC0 word[a.u.]","Occurency [#]");
}


int MRCConfigurationManager::ParseInLine(int argc, char *argv[]){
	return fcfg->parseInline(argc, argv);
	
}
int MRCConfigurationManager::InsertInLine(){
	return fcfg->insertInline();
}

/*
*/
