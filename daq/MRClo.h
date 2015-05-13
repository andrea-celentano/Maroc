/* 
 * Header file to interface SpectController
 * This should be independent to the used USB driver:
 * it works with Cypress USB driver and SpectLibCy.cpp
 *
 * Author: Paolo Musico - INFN Genova
 * Date: 22 mag 2008
 * Rev: 1.1
 *
 * Maintained by: Evaristo Cisbani, Matteo Turisini
 *
 */

#ifndef __MRCLO_H__
#define __MRCLO_H__


#include <libusb-1.0/libusb.h>


/* USB */ 
#define SPECT_USB_VID	0x1556	// CERN Vendor ID
#define SPECT_USB_PID	0x0388	// ISS_INFN Gamma Camera Controller Product ID


/*CONTROL BOARD FPGA*/

// Addresses 
#define PULSE_CONTROL_REG_ADDR		0x000
#define STATIC_CONTROL_REG_ADDR		0x001
#define STATUS_REG_ADDR				0x002
#define FE_ENABLED_MASK_REG0_ADDR	0x003
#define FE_ENABLED_MASK_REG1_ADDR	0x004
#define IRQ_RAM_ADDR				0x005
#define DAQ_FIFO_ADDR				0x006
#define DAQ_FIFO_ADDR_LONG			0x007

// Pulse Control Register
#define IRQ_PROCESS_RESTART			0x00000001
#define CLEAR_ALL_FSM				0x00000002
#define CLEAR_DAQ_NEVENTS			0x00000004
#define CLEAR_REGISTERS				0x40000000
#define IRQ_RAM_CLEAR				0x80000000

// Static Control Register 
#define DAQ_ENABLE					0x00000001
#define	TRG_TYPE					0x0000000E
#define	IACK_DELAY					0x00000070
#define GATE_ENABLE					0x00000080 // SW_ACQ_GATE --> FREEZE!
#define MULTIEVENT_DAQ				0x00000100
#define EVENTS_TO_READ_MASK			0x0000FC00 // is bit 9 spare?

// Status Register
#define IRQ_PROCESSING_FLAG			0x00000001
#define TRG_PROCESSING_FLAG			0x00000002
#define DAQ_PROCESSING_FLAG			0x00000004
#define DAQ_DONE_FLAG				0x00000008
#define EV_IDENTIFIED_FLAG			0x00000010
#define EV_REJECTED_FLAG			0x00000020
#define DAQ_FIFO_EMPTY_FLAG			0x00000040
#define DAQ_FIFO_FULL_FLAG			0x00000080
#define N_WORDS_MASK				0x003FFF00
#define	N_EVENTS_MASK				0x0FC00000
#define REVISION_MASK				0xF0000000

// Commands 
#define CMD_FRONTEND_ACCESS			0x00
#define CMD_SW_TRIGGER				0x04	
#define CMD_CONTROL_ACCESS			0x08	

// (not used) For more elaborated functions...
#define FSM_AUTOTRIGGER_STATUS		0x00000007
#define FSM_SINGLE_DAQ_STATUS		0x00000038
#define FSM_LOAD_MAROC_CFG_STATUS	0x000001C0
#define FSM_LOAD_OUT_CH_STATUS		0x00000E00
#define FSM_LOAD_DELAY_STATUS		0x00003000
#define FE_DAQ_DONE_FLAG			0x00020000	/* NEW NEW NEW */




class MRClo {
	
private:
	
	struct libusb_device_handle *	SpectHandle;
	
	libusb_context *				fCtx;
	
	unsigned char	ControllerFirmwareRevision;
	unsigned char	OutEndPoint;
	unsigned char	InEndPoint;
	unsigned int	XferTimeOut;	// Milliseconds
	int				fInterface;
	int				fDid;
	
		
public:

	MRClo();
	~MRClo();

	// OPEN/CLOSE
	
	/*Open the USB endpoints to communicate with the Spect Control Card
	 * Only one Spect Control Card can be connected to the PC
	 * The function returns true/false in case of success/failure*/
	bool SpectOpen(unsigned long didm=0xffff);
	/*
	 *
	 */
	bool SpectClose(void);
	/*
	 // Alternative to SpectOpen() for debug only
	 // Not to be used by most...
	 */  
	void FindFirstDevice(void);
	
	// BASIC I/O
	
	/*
  // Writes ndata bytes at the specified addr
  // Only the first 3 bytes (24 bit) of every doubleword are sent to the FrontEnd cards
  // The 4th byte (MSB) of every doubleword is discarded by the control card
  // ndata must be multiple of 4 and < 504
  // The function returns true/false in case of success/failure
  // Upon return ndata contains the effective number of bytes written
  */
	bool SpectWriteLL(	unsigned short addr, unsigned char *data, int &ndata);
	/*
	 // Writes ndata bytes at the specified addr
	 // The function adds one  byte (set to 0) every 3 to align the 24 bit data on 32 bit bounduary
	 // ndata must be multiple of 3 and < 378
	 // The function returns true/false in case of success/failure
	 // Upon return ndata contains the effective number of bytes written
	 */
	bool SpectWrite(	unsigned short addr, unsigned char *data, int &ndata);
	/*
  // Read ndata bytes from the specified addr
  // Only the first 3 bytes (24 bit) are meaningful
  // The 4th bytes (MSB) of every doubleword is set to 0 by the control card
  // ndata must be multiple of 4 and < 504
  // The function returns true/false in case of success/failure
  // Upon return ndata contains the effective number of bytes read
  */
	bool SpectReadLL(	unsigned short addr, unsigned char *data, int &ndata);	
	/*
	 // Read ndata bytes from the specified addr
	 // The function discard one byte every 4 (added from the control card)
	 // ndata must be multiple of 3 and < 378
	 // The function returns true/false in case of success/failure
	 // Upon return ndata contains the effective number of bytes read
	 */
	bool SpectRead(		unsigned short addr, unsigned char *data, int &ndata);
	
	// ANY REGISTER I/0
	
	/*
	 * Writes a register.Use: to write a register on the Controller CB_FPGA
	 * IN: address of the register
	 * IN: new value
	 * OUT: true if success
	 */
	bool SpectCtrlWrite(unsigned int addr, unsigned long data);
	/*
	 // Read a register. Use: to read a register on the Controller CB_FPGA
	 // IN: address of the register
	 // IN: new value
	 // OUT: true if success
	 */
	bool SpectCtrlRead(	unsigned int addr, unsigned long *data);
	

	// STATIC CONTROL REGISTER
	
	/* Configure and Command the data acquisition 
	 * enable/disable data acquisition 
	 * enable/disable gate
	 * set/get trigger type
	 * set/get trigger acknowledge delay
 
	 */
	
	bool SpectEnableDaq(int events_to_read);
	bool SpectDisableDaq(void);

	bool SpectEnableGate(void);
	bool SpectDisableGate(void);
	
	bool SpectSetTrgType(unsigned char trig_type);
	bool SpectGetTrgType(unsigned char *trig_type);
	
	bool SpectSetIackDelay(unsigned char iack_delay);
	bool SpectGetIackDelay(unsigned char *iack_delay);
	
	
	// PULSE CONTROL REGISTER
	/* Commands:
	 * - Restart data acquisition
	 * - Reset all
	 * - Clear IRQ RAM 
	 */
	bool SpectRestartDaq(void);
	bool SpectClearIrqRamAddr(void);
	bool SpectCtrlAllClear(void);
  
	// STATUS REGISTER
	/* Reads from status register the status of the data acquisition:
	 * - revision id 
	 * (DAQ FIFO) - nwords			
	 * (DAQ FIFO) - nevents			
	 * (DAQ FIFO) - flag daq done
	 * (DAQ FIFO) - flag daq fifo_full
	 */
	bool SpectGetCtrlRevision(unsigned char *revision);
	bool SpectGetDaqDoneFlag(bool *daq_done);	
  	bool SpectGetNEvents(int *n_events);
	bool SpectGetDaqFifoFullFlag(bool *fifo_full);	
	bool SpectGetDaqNwords(unsigned short *nwords);
	
	// SW TRIGGER 
	/* 
	 * Generates a the TRIGGER pulse to the FrontEnds
	 */
	bool SpectSwTrigger(void);
	
	// READOUT 
	
	/* Read out Charge and Binary Line (COMPLETE READ OUT)
	 * To be used for reading out the data
	 * IN: address for data
	 * IN: number of dword
	 */
	bool SpectDaqRead_LONG(unsigned char *daq_data_vector, int n_words);
	
	
	// NOT USED
	/*
	 * Read out the IRQ_RAM that contains the trigger bits 
	 */
	bool SpectDiscrRead(unsigned char *discr_data_vector);
	/*
	 // Reads any number of data words from any Control Board internal memory
	 // NOT TO BE USED BY MOST
	 */
	bool SpectReadCmdLL(unsigned short addr, unsigned char cmd, unsigned char *data, int &ndata);
	/*
	 // Reads n_words*4 DAQ data bytes into the daq_data_vector array from the Control Board DAQ fifo
	 */
	bool SpectDaqRead(unsigned char *daq_data_vector, int n_words);
	
	
	// SW 
	int GetInterface() {return fInterface;}	
	int GetDid() {return fDid;}
	int SpectGetDeviceID();


};

#define SpectFeWrite(a, b, c)	SpectWrite(a,b,c)
#define SpectFeRead(a, b, c)	SpectRead(a,b,c)
  
#endif
