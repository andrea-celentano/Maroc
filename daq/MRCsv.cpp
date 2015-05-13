#include "MRCsv.h"
#include <unistd.h>

MRCsv::MRCsv()
{
	ftime_acqstart	= 0;
	ftime_acqstop	= 0;
	fRateMeterStart	= 0;	
	fRateMeterStop	= 0;
	fRateMeterStart_ns	= 0;	
	fRateMeterStop_ns	= 0;
	
	ftimeinfostart	= NULL;
	ftimeinfostop	= NULL;
	
	fevent			= 0;
	fduration		=.0;
	frate			=.0;

	fevent_preset	= 0;
	ftime_preset_sec= 0;
	
	fprogress_old	= 0;
	
	fpoll			= 0;
	fready			= 0;	
	ftail			= 0;	
	
	//printf("Welcome to MRC DAQ Supervisor Class\n");
}


MRCsv::~MRCsv()
{
	printf("Bye Bye\n");

}

void MRCsv::IncrCountPolling()
{
	fpoll++;
}
void MRCsv::IncrCountDataReady()
{
	fready++;
} 
void MRCsv::IncrCountTailTransfer()
{
	ftail++;
}	


void MRCsv::PrintProfiling()
{
	printf("\n+----Statistic on Acquisition-----+\n");
	printf("+ Inner Loop      :(%4ld) \n",fpoll);
	if(fready!=0||fpoll!=0){
		printf("+ Flag Data Ready :(%4ld) -> %4.1f%%\n",fready,100*((float) fready)/((float) fpoll));
	}else{
		printf("+ Flag Data Ready :(%4ld)  \n",fready);
	}
	printf("+ Tail Transfer   :(%4ld) \n",ftail);
	printf("+---------------------------------+\n");
	
}
void MRCsv::AddProfiling(GI_Config * cfg)
{
	if (cfg==NULL) {
		printf("No active configuration found...\n");
	}else {
		cfg->AddList("Polling",fpoll);
		cfg->AddList("DataReady",fready);
		cfg->AddList("Tail",ftail);
		cfg->AddListFloat("PollingEfficiency",((float) fready)/((float) fpoll));
	}

}

void MRCsv::PrintTail(int nWrds)
{
	printf("\n+------Tail Transfer   ---------+\n");
	printf("+ EventID = %ld                     \n",fevent);
	printf("+ nWords  = %d                    \n",nWrds);
	printf("+---------------------------------+\n");

}

bool MRCsv::AcqRunning()
{
	ProgressBar();
	if (fevent >= fevent_preset) {
		return false;
	} 
	if ((unsigned long int)CalculateDuration() >= ftime_preset_sec) { // or fduration directly without recalculation
		return false;
	}

	return true;
}
void MRCsv::ProgressBar()
{	
	int progress_time = (((float) CalculateDuration())/ftime_preset_sec)*PROGRESS_STEP; 
	
	int progress_evt = (((float) fevent)/fevent_preset)*PROGRESS_STEP;
	
	int progress = progress_time > progress_evt ? progress_time : progress_evt;
	
	progress = progress < PROGRESS_STEP ? progress : PROGRESS_STEP;
	
	for (int jprog = 0; jprog<(progress - fprogress_old); jprog++)
	{
		if (progress_time > progress_evt) 
		{
			printf("T");
		} else {
			printf("E");
		}
		fflush(stdout);
	}
	
	if (progress == PROGRESS_STEP){} // 
	
	fprogress_old = progress;
}


unsigned long MRCsv::GetEvents()
{
	return fevent;
}


void MRCsv::IncrEvt(unsigned long value)
{
	fevent += value; 
}


void MRCsv::SetEventPreset(unsigned long value)
{
	fevent_preset=value;
}
void MRCsv::SetTimePreset(unsigned long value)
{ 
	ftime_preset_sec = value;
}

void MRCsv::SetProgressOld(int value)
{
	fprogress_old = value;
}


unsigned long MRCsv::GetEventPreset()
{
	return fevent_preset;
}
unsigned long MRCsv::GetTimePreset()
{
	return ftime_preset_sec;
}


int MRCsv::GetProgressOld()
{
	return fprogress_old;
}	





void MRCsv::MeasureTimeStart()
{
	time(&ftime_acqstart);
	ftimeinfostart = localtime(&ftime_acqstart);
	printf ("Begin time and date: %s", asctime(ftimeinfostart));
	
}

void MRCsv::MeasureTimeStop()
{
	time(&ftime_acqstop);
	ftimeinfostop = localtime(&ftime_acqstop);
	printf ("End time and date: %s", asctime(ftimeinfostop));
}


void MRCsv::MeasureTimeNow()
{
	time(&ftime_current);
	ftimeinfocurrent = localtime(&ftime_current);
	//printf ("Current time and date: %s", asctime(ftimeinfocurrent));
}

void MRCsv::StartTimer()
{
	printf("Timer START :");
	this->Timer(&fRateMeterStart,&fRateMeterStart_ns);	
}
void MRCsv::StopTimer()
{
	printf("\nTimer STOP :");
	this->Timer(&fRateMeterStop,&fRateMeterStop_ns);	
}


double MRCsv::CalculateDuration()
{
	MeasureTimeNow();
	fduration =(double) (ftime_current-ftime_acqstart);
	//printf("Duration is %lf",fduration);
	return fduration;
}	




double MRCsv::CalculateRate()
{
	printf("**Timer is %ld sec + %ld nsecond\n",fRateMeterStart,fRateMeterStart_ns);
	printf("**Timer is %ld sec + %ld nsecond\n",fRateMeterStop,fRateMeterStop_ns);

/*
mainpolare tempo start e stop secondi e nanosecondi
per stimare durata run con alta precisione
*/


	if ((fRateMeterStop-fRateMeterStart)!=0) {
		printf("\n");
		printf("# MRC Events\t\t  = %5ld\t[#]\n",fevent);
		printf("# MRC Time_Real \t  = %.3lf\t[sec] \n",((double)(fRateMeterStop-fRateMeterStart)/GIGA));
		printf("# MRC Events/Time_Real\t  = %.1lf\t[Hz] \n",(((float) fevent*GIGA)/(fRateMeterStop-fRateMeterStart)));	

		fduration	= ((double)(fRateMeterStop-fRateMeterStart)/GIGA);
		frate		= (((float) fevent*GIGA)/(fRateMeterStop-fRateMeterStart));
	}else {
		printf("Something Wrong with Rate Meter\n");
	}	
	return frate;
}



void MRCsv::Print()
{
}

void MRCsv::AddInfoRate(GI_Config * cfg)
{
	if (cfg==NULL) {
		printf("No active configuration found...\n");
	}else {
		cfg->AddList("Events",fevent);
		cfg->AddListFloat("Duration",fduration);	
		cfg->AddListFloat("daqRate",frate);
	}
}




void MRCsv::AddInfoStart(GI_Config * cfg)
{
	if (cfg==NULL) {
		printf("No active configuration found...\n");
	}else {
		cfg->AddList("Year",1900+ftimeinfostart->tm_year);
		cfg->AddList("Month",1+ftimeinfostart->tm_mon);	
		cfg->AddList("Day",ftimeinfostart->tm_mday);
		cfg->AddList("Hour",ftimeinfostart->tm_hour);
		cfg->AddList("Minute",ftimeinfostart->tm_min);	
		cfg->AddList("Sec",ftimeinfostart->tm_sec);
	}
}

void MRCsv::AddInfoStop(GI_Config * cfg)
{
	if (cfg==NULL) {
		printf("No active configuration found...\n");
	}else {
		cfg->AddList("Year",1900+ftimeinfostop->tm_year);
		cfg->AddList("Month",1+ftimeinfostop->tm_mon);	
		cfg->AddList("Day",ftimeinfostop->tm_mday);
		cfg->AddList("Hour",ftimeinfostop->tm_hour);
		cfg->AddList("Minute",ftimeinfostop->tm_min);	
		cfg->AddList("Sec",ftimeinfostop->tm_sec);
	}
}


void MRCsv::AddData(GI_Config * cfg)
{
	AddInfoStart(cfg);
	AddInfoStop(cfg);
	AddInfoRate(cfg);
	AddProfiling(cfg);
}

void MRCsv::Timer(uint64_t *pt,uint64_t *pt_ns)
{	

	// MAC OSX
	//*pt = mach_absolute_time();
	

	// UNIX, LINUX, POSIX OS
	
	// Code for time in nanosecond in POSIX systems
	timespec mark0;
	clock_gettime(CLOCK_REALTIME,&mark0);// not good on MACOSX
	printf("Timer is %ld sec + %ld nsecond\n",mark0.tv_sec,mark0.tv_nsec);// not good on MAC
 	*pt = mark0.tv_sec;
 	*pt_ns = mark0.tv_nsec;
};




