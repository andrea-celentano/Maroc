
/*
 * Maroc Supervisor: 
 *
 *
 * Classe di profiling,
 *	
 * 
 */

#ifndef __MAROCSV_H__
#define __MAROCSV_H__

#include <stdio.h>		/* printf*/
#include <time.h>       /* time_t, struct tm, time, localtime */
//#include <mach/mach_time.h> /* mach_absolute_time() ONLY FOR MAC*/ 
#include <stdint.h>  

#include "GI_Config.h" //

#define	GIGA	1000000000
#define PROGRESS_STEP	20

using namespace std;

class MRCsv{
private:
	uint64_t		fRateMeterStart;	
	uint64_t		fRateMeterStop;	

	uint64_t		fRateMeterStart_ns;
	uint64_t		fRateMeterStop_ns;	
	
	time_t			ftime_acqstart;
	time_t			ftime_acqstop;
	time_t			ftime_current;
	
	struct tm *		ftimeinfostart;	
	struct tm *		ftimeinfostop;	
	struct tm *		ftimeinfocurrent;	
	
	
	double			fduration;
	double			frate;
	
	unsigned long	fevent;	
	unsigned long	fevent_preset;
	unsigned long	ftime_preset_sec;
	
	int		fprogress_old;
	
	unsigned long	fpoll;
	unsigned long	fready;	
	unsigned long	ftail;	
	
public:
	MRCsv();
	~MRCsv();
	
	void MeasureTimeStart();
	void MeasureTimeStop();
	void MeasureTimeNow();
	
	double CalculateDuration();		
	double CalculateRate();	
	
	
	void Print();
	void AddData(GI_Config * cfg=NULL);
	void StartTimer();
	void StopTimer();
	
	void SetEventPreset(unsigned long value=0);
	void SetTimePreset(unsigned long value=0);
	void SetProgressOld(int value=0);
	
	unsigned long GetEventPreset();
	unsigned long GetTimePreset();	
	int GetProgressOld();		
	
	void IncrEvt(unsigned long value=0);
	
	unsigned long GetEvents();
	
	bool AcqRunning();
	
	void IncrCountPolling(); 
	void IncrCountDataReady(); 
	void IncrCountTailTransfer();	
	
	void PrintProfiling();
	void AddProfiling(GI_Config * cfg=NULL);
	
	void PrintTail(int nWrds);

private:

	void AddInfoStart(GI_Config * cfg=NULL);
	void AddInfoStop(GI_Config * cfg=NULL);	
	void AddInfoRate(GI_Config * cfg=NULL);
	void Timer(uint64_t *pt,uint64_t *pt_ns);
	
	void ProgressBar();
	
};




#endif
