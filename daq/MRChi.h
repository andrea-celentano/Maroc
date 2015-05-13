/*
 *	High level MAROC class
 *	To manage the individual front end configuration 
 *
 *	Authors: Turisini Matteo and Evaristo Cisbani
 *
 * Configuration File 
 * Front end parameter 
 *  libconfig – C/C++ Configuration File Library 
 * A Library For Processing Structured Configuration Files
 * Documentation and dowload at http://www.hyperrealm.com/libconfig/
 *
 * Read configuration data from a configuration file.
 * Support inline changes and data saving (logbooking) 
 *
 * note: individual asic configuration can be specified, as well as a default 
 */

// FE = front end card (MAROC + FPGA_fe)
// CB = control board (FPGA_cb)


// asic version 2 and asic version 3 supported

#ifndef __MAROC_HIGH_H__
#define __MAROC_HIGH_H__

#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <string.h>

#include "MRC_Common.h"
#include "MRClo.h"
#include "GI_Config.h" // at runtime needs libconfig

// Addresses Firmware
#define FEADDR_MAROC_CONFIG_DATA	0x00	/* Read-Write */
#define FEADDR_CONFIG_REG			0x01	/* Read-Write */
#define FEADDR_ADC_REG				0x02	/* Read Only  */ //not used!
#define FEADDR_STATUS_REG			0x03	/* Read Only  */
#define FEADDR_DAQ_FIFO				0x04	/* Read Only  */ // not used!
#define FEADDR_THR_FIFO				0x05	/* Read-Write */	// V2
#define FEADDR_CONFIG2_REG			0x06	/* Read-Write */	// V2
#define FEADDR_HITEN0_REG			0x07	/* Read-Write */	// V2
#define FEADDR_HITEN1_REG			0x08	/* Read-Write */	// V2
#define FEADDR_HITEN2_REG			0x09	/* Read-Write */	// V2
#define FEADDR_COMMAND				0x0F	/* Write Only */

#define Sleep(msec) usleep(msec * 1000)

using namespace std;

class MRChi {
 
private:

	/* Interface with the controller*/
	MRClo*			fMlo;					

	/* Frontend  discovery and check */
	unsigned int	fFeNum;			// number of fe cards
	unsigned char	fASIC[64];		// asic revision 
	unsigned char	fRevisionFW[64];// firmware revision		
	unsigned int	fBaseAddr[64];	// address  

	/* Asic configuration*/
	unsigned char	maroc_config_data[105];
	
	// Firmware configuration
	unsigned char	fe_config_register[3];
	unsigned char	fe_config_register2[3];	
	unsigned char	fe_config_register_vdd_fsb[3];	
	
	unsigned char	fe_hiten0_register[3];	
	unsigned char	fe_hiten1_register[3];	
	unsigned char	fe_hiten2_register[3];	
	
	unsigned long	fSparseThrDef;		// Sparse readout
	unsigned long	fSparseThr[64];		
	unsigned int	en_zero_supp;		
	unsigned char	DaqThreshold[192];	
	
	// Commands
	unsigned char	sw_reset_cmd[3];
	unsigned char	single_acq_cmd[3];
	unsigned char	out_ch_config_cmd[3];	
	unsigned char	maroc_config_cmd[3];
	unsigned char	delay_config_cmd[3];
	
	unsigned char	sw_clear_cmd[3];			// not used
	unsigned char	restart_auto_acq_cmd[3];	// not used

public:
	
	MRChi(MRClo *lo=NULL);
	~MRChi();
	
	// Discover how many front end card are plugged into the system
	bool fe_discovery();
	bool fe_discovery_fast();

	int	getCount(){ return (int) fFeNum; };
	int	getBaseAddr(int i){ return fBaseAddr[i];};	
	
	// Single FE card configuration
	bool ConfigureFrontEnd		(GI_Config *cfg=0, int lmrc_count=0);
	
	// Dummy Operation with ADC after power up for each card in the system
	bool fe_dummy_acq();
	
	// Reset all frontend cards in the system (configuration is preserved)
	bool fe_soft_reset();
	
	
private:
	
	/*	Set the configuration registers of each card in the system*/
	bool fe_configReg_write();
	
	// minor changes to separate ASIC version 2 and 3, decoupling it from FW revision (2013 June 27, Matteo)
	/*
	 * For a given index (i.e. front end card), read  the slow control register from the FPGA 
	 * and compare it against the passed vector (SCVect and its lenght SClen).
	 * Can be used to check the correct i/o with the frontends fpga.
	 * should return a boolean (NEEDCHANGE)
	 */
	void CheckSlowControl(int MFE_idx, int SClen,unsigned char * SCvect);
	
	
	/* Software */
	bool PrintConfiguration		(GI_Config *cfg=0, int lmrc_count=0);
	

	// Prepare registers
	bool MakeConfReg2			(GI_Config *cfg=0, int mrc_idx=0);
	bool MakeConfReg1			(GI_Config *cfg=0, int mrc_idx=0);
	bool MakeMarocSlowControl	(GI_Config *cfg=0, int mrc_idx=0);
	bool MakeDaqThresholdReg	(GI_Config *cfg=0, int mrc_idx=0);
	bool MakeHitEnabledMask		(GI_Config *cfg=0, int mrc_idx=0);
	

	// Decode MAROC slow control
	/*Print SlowControl Configuration parameters (except DAC and Gain) according to ASIC versione 2 or 3 */
	void DecodeSlowControl(unsigned char maroc_rev,unsigned char * maroc_config_data);
	
	// DAC
	void DecodeDAC(unsigned char maroc_rev,unsigned char * maroc_config_data);
	
	// GAIN
	void ReadGain(unsigned char maroc_rev,unsigned char * maroc_config_data,unsigned char channel);
	/* Reassemble gain bits for a channel to give Gain value (gword) for that channel*/
	unsigned char DecodeGain(unsigned char maroc_rev,unsigned char * maroc_config_data,unsigned char first_byte,unsigned char type);
	/* Binary gain Translate  Binary Gain [0..255] into Analog gain [0..4] */
	double GetAnalogGain(unsigned char digital_gain);
	
	// SUM
	bool DecodeCmdSum(unsigned char maroc_rev,unsigned char * maroc_config_data,unsigned char first_byte,unsigned char type);
	
	// Utils
	void resetCount(unsigned int mc=0) { fFeNum = mc; };
	
	// Depending on the ASIC REVISION it returns the size of configuration register
	// MAROC3 105 bytes
	// MAROC2 69 bytes
	
	int ConfigLen(int mrc_asic_rev);
	
	unsigned char	getFWRevision(int i)	{ return fRevisionFW[i];	};
	unsigned char	getASICRevision(int i)	{ return fASIC[i];			};
	unsigned int	getCommand()			{ return FEADDR_COMMAND; };
	unsigned int	getStatusReg()			{ return FEADDR_STATUS_REG; };
	
	// Not used
	bool reinit_Ctrl_and_Fe(); 

	
	// MAROC2 ONLY
	/*
	 * Breaks the argument in coarse and fine DAC words than reassembles it
	 */
	unsigned long IE_MakeDacWord(unsigned long lthr,unsigned char maroc_revision=3);
	/*
	 * Creates Random Configuration Vector (for test purpouse)
	 */
	void RandomConfigVectors();
	/*
	 * To test the presence of a front end card at address idx, 
	 * write and read back len bytes from vect 
	 * returns 0 in case of identity (means that the card is there and correctly answered) 
	 * otherwise return the number of byte-wise differences 
	 */
	int WriteReadMarocVector(int idx, int len,unsigned char * vect);
};
#endif