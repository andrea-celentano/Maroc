/* SpectLibLin.cpp
 * TO BE COMPILED WITH LIBUSB
 *
 * Source file to interface SpectController using USB Linux driver
 * use libusb-1.0
 *
 * Author: Paolo Musico - INFN Genova
 * Date: 6 may 2009
 * Rev: 2.0
 *
 * Maintained by: Evaristo Cisbani, Matteo Turisini
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "MRC_Common.h"

#include "MRClo.h"

#define USB_VID 0x1556  // CERN Vendor ID
#define USB_PID 0x0388  // ISS_INFN Gamma Camera Controller Product ID

MRClo::MRClo() {

  //libusb_device_handle *SpectHandle=NULL;
  //libusb_context *fCtx=NULL;

  OutEndPoint = 0x02;
  InEndPoint = 0x86;
  XferTimeOut = 1000;	// Milliseconds

  fInterface=-1;

  fDid=-1;

}

/*
 *
 */
MRClo::~MRClo() {
}

// Writes ndata bytes at the specified addr
// Only the first 3 bytes (24 bit) of every doubleword are sent to the FrontEnd cards
// The 4th byte (MSB) of every doubleword is discarded by the control card
// ndata must be multiple of 4 and < 504
// The function returns true/false in case of success/failure
// Upon return ndata contains the effective number of bytes written
bool MRClo::SpectWriteLL(unsigned short addr, unsigned char *data, int &ndata)
{
	unsigned char buf[512];
	int i, bytes_sent, err;
	bool success;

	if( ndata > 504 )
		return false;
	if( ndata % 4 )
		return false;
	buf[0] = 'H';
	buf[1] = 'E';
	buf[2] = 'A';
	buf[3] = 'D';
	buf[4] = addr & 0xFF;
	buf[5] = (addr >> 8) & 0x03;
	buf[6] = ndata & 0xFF;
	buf[7] = (ndata >> 8) & 0xFF;
	for(i=0; i<ndata; i++)
		buf[8+i] = data[i];

	bytes_sent      = 0; //ndata + 8; 

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,
		ndata+8, &bytes_sent, XferTimeOut);

	if (bytes_sent != ndata + 8) {
	  MRC_ERR("Bytes sent %d do not correspond to byte to be sent %d (addr 0x%x, error code=0x%x)\n",bytes_sent, ndata+8, addr, err);
	};

	success = ((err == 0) && (bytes_sent == (ndata+8))) ? true : false;
	ndata = bytes_sent - 8;
	return( success );
}


// Read ndata bytes from the specified addr
// Only the first 3 bytes (24 bit) are meaningful
// The 4th bytes (MSB) of every doubleword is set to 0 by the control card
// ndata must be multiple of 4 and < 504
// The function returns true/false in case of success/failure
// Upon return ndata contains the effective number of bytes read
bool MRClo::SpectReadLL(unsigned short addr, unsigned char *data, int &ndata)
{
//	unsigned char buf[512];
	unsigned char buf[512];
	int i, bytes_xfer, err;
	bool success;

	if( ndata > 504 )
		return false;
	if( ndata % 4 )
		return false;
	buf[0] = 'H';
	buf[1] = 'E';
	buf[2] = 'A';
	buf[3] = 'D';
	buf[4] = addr & 0xFF;
	buf[5] = ((addr >> 8) & 0x03) | 0x80;
	buf[6] = ndata & 0xFF;
	buf[7] = (ndata >> 8) & 0xFF;
	bytes_xfer = 0; //8;

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,
		8, &bytes_xfer, XferTimeOut);

	success = ((err == 0) && (bytes_xfer == 8)) ? true : false;
	if( success )
	{
	  bytes_xfer      = 0; //ndata + 4;
	  err = libusb_bulk_transfer(SpectHandle, InEndPoint, buf,
				     ndata+4, &bytes_xfer, XferTimeOut);
	  success = ((err == 0) && (bytes_xfer == (ndata+4))) ? true : false;
	  if( success )
	    {
	      for(i=0; i<ndata; i++)
		data[i] = buf[4+i];
	      if( buf[0] != 'A' || buf[1] != 'N' || buf[2] != 'S' || buf[3] != 'W' )
		success = false;
		}
	  ndata = bytes_xfer - 4;
	}
	return( success );
}


// Writes ndata bytes at the specified addr
// The function adds one  byte (set to 0) every 3 to align the 24 bit data on 32 bit bounduary
// ndata must be multiple of 3 and < 378
// The function returns true/false in case of success/failure
// Upon return ndata contains the effective number of bytes written
bool MRClo::SpectWrite(unsigned short addr, unsigned char *data, int &ndata)
{
	unsigned char buf[512];
	int i, j;
	bool success;

	if( ndata > 378 )
		return false;
	if( ndata % 3 )
		return false;
	i = 0; j = 0;
	while( i<ndata )
		if( (j % 4) != 3 )
			buf[j++] = data[i++];
		else
			buf[j++] = 0;
	buf[j++] = 0;

	success = SpectWriteLL(addr, buf, j);
	i = j / 4;
	ndata = j - i;
	return( success );

}


// Read ndata bytes from the specified addr
// The function discard one byte every 4 (added from the control card)
// ndata must be multiple of 3 and < 378
// The function returns true/false in case of success/failure
// Upon return ndata contains the effective number of bytes read
bool MRClo::SpectRead(unsigned short addr, unsigned char *data, int &ndata)
{
	unsigned char buf[512];
	int i, j;
	bool success;

	if( ndata > 378 )
		return false;
	if( ndata % 3 )
		return false;
	i = ndata / 3;
	j = ndata + i;
	success = SpectReadLL(addr, buf, j);
	ndata = j;
	i = 0; j = 0;
	while( j<ndata )
		if( (j % 4) != 3 )
			data[i++] = buf[j++];
		else
			j++;
	ndata = i;
	return( success );
}


// Generates a TRIGGER pulse to the FrontEnds
// The function returns true/false in case of success/failure
bool MRClo::SpectSwTrigger(void)
{
	unsigned char buf[8];
	int bytes_sent, err;
	bool success;

	buf[0] = 'H';
	buf[1] = 'E';
	buf[2] = 'A';
	buf[3] = 'D';
	buf[4] = 0x00;
	buf[5] = CMD_SW_TRIGGER;
	buf[6] = 0x00;
	buf[7] = 0x00;

	bytes_sent = 0;

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,8, &bytes_sent, XferTimeOut);

	success = ((err == 0) && (bytes_sent == 8)) ? true : false;
	return( success );
}


// Open the USB endpoints to communicate with the Spect Control Card
// Only one Spect Control Card can be connected to the PC
// The function returns true/false in case of success/failure


bool MRClo::SpectOpen(unsigned long didm) // device id mask
{
  bool success;
  int r;
  ssize_t i;
  libusb_device **list;
  libusb_device_descriptor desc;
  libusb_config_descriptor *config;
  
  //	libusb_device *dev;

  fDid = -1;

  r = libusb_init(&fCtx);
  if (r<0) {
    MRC_ERR("libusb init error, cannot proceed (%d)\n",1);
    return false;
  }
  
  libusb_set_debug(fCtx, 1);      // 0 = no messages, 1 = err, 2 = 1 + warn, 3 = 2 + info

  ssize_t cnt = libusb_get_device_list(fCtx, &list);
  if (cnt < 0) { 
    MRC_ERR("Something wrong getting the list of usb devices\n");
    return false;
  }
  MRC_MSG("Got %ld usb devices\n",cnt);

  fInterface = -1;

  for (i=0;i<cnt;i++) {
    libusb_device *device = list[i];
    r = libusb_get_device_descriptor(device, &desc);
    if (r < 0) {
      MRC_WRN("Failed to get device config descriptor (ret=%d) of device idx=%ld\n",r,i);
      continue;
    }
    if ((desc.idVendor == USB_VID) && (desc.idProduct == USB_PID)) {
      MRC_MSG("Device enum %ld has VID=0x%x and PID=0x%x, DID=0x%x\n",i, desc.idVendor,desc.idProduct,desc.bcdDevice);
      if ((didm != 0xffff) && (desc.bcdDevice != didm)) {
	MRC_MSG("Device ID does not match ctrl_board_did (=0x%lx), try next\n",didm);
	continue;
      }
      MRC_MSG(" ... bus 0x%x and address 0x%x\n",  libusb_get_bus_number(device), libusb_get_device_address(device));

      r = libusb_open(device, &SpectHandle);
      if (r) {
	MRC_WRN("Something wrong opening the device (idx=%ld), try next (if any left)\n",i);
	SpectHandle=NULL;
	continue;
      } 

      r = libusb_get_config_descriptor(libusb_get_device(SpectHandle), 0, &config);
      MRC_MSG("Available interfaces: %d\n", config->bNumInterfaces);
      if (r < 0) {
	MRC_ERR("Failed to get device config descriptor (ret=%d)\n",r);
	libusb_close(SpectHandle);
	SpectHandle=NULL;
	continue;
      }

      // claim first available interface
      for (int i=0;i<config->bNumInterfaces;i++) {
	r = libusb_claim_interface(SpectHandle, i); //claim interface 0 (the first) of device (mine had jsut 1)
	if(r < 0) {
	  MRC_ERR("Interface %d: cannot Claim Interface (ret=%d)\n", i, r);
	  libusb_close(SpectHandle);
	  fInterface = -1;
	  SpectHandle=NULL;
	  continue;
	} else {
	  MRC_MSG("Associated to usb device 0x%x 0x%x with S/N=%d\n",desc.idVendor, desc.idProduct, desc.iSerialNumber);
	  MRC_MSG(" ... Interface %d: succesfully claimed\n",i);
	  fInterface = i;
	  break;
	}
      }

    }

    if (fInterface >=0) { break; }
    
  }
  
  libusb_free_device_list(list,1);
  
  if ((SpectHandle == NULL) || (fInterface <0)) {
    MRC_ERR("Cannot open device\n");
    return false;
  }
  
  success = SpectGetCtrlRevision(&ControllerFirmwareRevision);
  if( success ){
    MRC_MSG("USB handler succesfully allocated device with revision 0x%x\n",ControllerFirmwareRevision);
    fDid = desc.bcdDevice;
    return true;
  }

  return false;

}

int MRClo::SpectGetDeviceID() {
  return fDid;
}

bool MRClo::SpectClose(void) {

  int r;

  if (SpectHandle != NULL) { 
    r = libusb_release_interface(SpectHandle, fInterface);
    if (r!=0) {
      MRC_ERR("Cannot release interface %d\n",fInterface);
      return false;
    }
    libusb_close(SpectHandle);
  }
  if (fCtx != NULL) {
    libusb_exit(fCtx);
  }

  MRC_MSG("libusb device released, session closed (%d)\n",fInterface);

  return true;

}


bool MRClo::SpectReadCmdLL(unsigned short addr, unsigned char cmd, unsigned char *data, int &ndata)
{
	unsigned char buf[512];
	int i, bytes_xfer, err;
	bool success;

//	if( ndata > 504 )
//x	if( ndata > 508 )		return false;
	if( ndata % 4 )
		return false;
	buf[0] = 'H';
	buf[1] = 'E';
	buf[2] = 'A';
	buf[3] = 'D';
	buf[4] = addr & 0xFF;
	buf[5] = ((addr >> 8) & 0x03) | (cmd & 0x1F) | 0x80;
	buf[6] = ndata & 0xFF;
	buf[7] = (ndata >> 8) & 0xFF;

	bytes_xfer = 0;

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,
		8, &bytes_xfer, XferTimeOut);

	success = ((err == 0) && (bytes_xfer == 8)) ? true : false;
	if( success )
	{
		err = libusb_bulk_transfer(SpectHandle, InEndPoint, buf,
			ndata+4, &bytes_xfer, XferTimeOut);
		success = ((err == 0) && (bytes_xfer == (ndata+4))) ? true : false;
		if( success )
		{
			for(i=0; i<ndata; i++)
				data[i] = buf[4+i];
			if( buf[0] != 'A' || buf[1] != 'N' || buf[2] != 'S' || buf[3] != 'W' )
				success = false;
		}
 		ndata = bytes_xfer - 4;
	}
	return( success );
}

bool MRClo::SpectCtrlWrite(unsigned int addr, unsigned long data)
{
	unsigned char buf[12];
	int bytes_sent, err;
	bool success;

	buf[0] = 'H';
	buf[1] = 'E';

	buf[2] = 'A';
	buf[3] = 'D';

	buf[4] = addr&0xFF;	// ADDR = [9:0], CMD = [14:10], RW = [15]
	buf[5] = CMD_CONTROL_ACCESS;

	buf[6] = 0x04;
	buf[7] = 0x00;

	buf[8] = data & 0xFF;
	buf[9] = (data >> 8) & 0xFF;

	buf[10] = (data >> 16) & 0xFF;
	buf[11] = (data >> 24) & 0xFF;

	bytes_sent = 0;

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,
		12, &bytes_sent, XferTimeOut);

	success = ((err == 0) && (bytes_sent == 12)) ? true : false;
	return( success );
}

bool MRClo::SpectCtrlRead(unsigned int addr, unsigned long *data)
{
	unsigned char buf[8];
	int bytes_xfer, err, packetsize;
	bool success;

	buf[0] = 'H';
	buf[1] = 'E';

	buf[2] = 'A';
	buf[3] = 'D';

	buf[4] = addr&0xFF;	// ADDR = [9:0], CMD = [14:10], RW = [15]
	buf[5] = CMD_CONTROL_ACCESS | 0x80;

	buf[6] = 0x04;
	buf[7] = 0x00;

	packetsize = 8;

	bytes_xfer = 0;

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,
		packetsize, &bytes_xfer, XferTimeOut);

	success = ((err == 0) && (bytes_xfer == packetsize)) ? true : false;
	if( success )
	{
		err = libusb_bulk_transfer(SpectHandle, InEndPoint, buf,
			packetsize, &bytes_xfer, XferTimeOut);
		success = ((err == 0) && (bytes_xfer == packetsize)) ? true : false;

		*data = (buf[7] << 24) + (buf[6] << 16) + (buf[5] << 8) + buf[4];
		if( success )
		{
			if( buf[0] != 'A' || buf[1] != 'N' || buf[2] != 'S' || buf[3] != 'W' )
				success = false;
		}
	}
	return( success );
}

bool MRClo::SpectEnableDaq(int events_to_read)
{
	unsigned long static_control_register;

	SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	static_control_register |= DAQ_ENABLE;

	if( events_to_read > 0 )
	  static_control_register |= MULTIEVENT_DAQ;
	else
	  static_control_register &= ~MULTIEVENT_DAQ;
	
	static_control_register |= ((events_to_read & 0x3F) << 10); //	static_control_register |= ((events_to_read & 0x0F) << 12);
	
	MRC_DBG("\nBuffersize =  %d  [events] (if 0 or 1 = single event)\n",events_to_read);
	//printf("(SpectEnableDaq) Static Ctrl Reg =  0x%lX\n",static_control_register);

	return SpectCtrlWrite(STATIC_CONTROL_REG_ADDR, static_control_register);
}

bool MRClo::SpectDisableDaq(void)
{
	unsigned long static_control_register;

	SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	static_control_register &= ~DAQ_ENABLE;
	static_control_register &= ~MULTIEVENT_DAQ; //x

	return SpectCtrlWrite(STATIC_CONTROL_REG_ADDR, static_control_register);
}

bool MRClo::SpectEnableGate(void)
{
	unsigned long static_control_register;
	
	SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	static_control_register |= GATE_ENABLE;
	return SpectCtrlWrite(STATIC_CONTROL_REG_ADDR, static_control_register);
}

bool MRClo::SpectDisableGate(void)
{
	unsigned long static_control_register;

	SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	static_control_register &= ~GATE_ENABLE;
	return SpectCtrlWrite(STATIC_CONTROL_REG_ADDR, static_control_register);
}

bool MRClo::SpectSetTrgType(unsigned char trig_type)
{
	unsigned long static_control_register;
	
	SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	static_control_register |= (trig_type << 1) & TRG_TYPE;
	return SpectCtrlWrite(STATIC_CONTROL_REG_ADDR, static_control_register);
}

bool MRClo::SpectGetTrgType(unsigned char *trig_type)
{
	unsigned long static_control_register;
	bool success;

	success = SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	*trig_type = (static_control_register & TRG_TYPE) >> 1;
	return success;
}

bool MRClo::SpectGetNEvents(int *n_events)
{
	bool lsuccess;
	unsigned long StatusRegister;

	lsuccess		= SpectCtrlRead(STATUS_REG_ADDR, &StatusRegister);
	*n_events	= (StatusRegister & N_EVENTS_MASK) >> 22;  //	*n_events = (StatusRegister & N_EVENTS_MASK) >> 24; // obsoletes

//	printf("\n(SpectGetNEvents) NEvents  =  %d  \n",*n_events);   // debug
//  printf("(SpectGetNEvents) Status Reg =  0x%X\n",StatusRegister); //debug
	return( lsuccess );
}

bool MRClo::SpectGetDaqFifoFullFlag(bool *fifo_full)
{
	bool success;
	unsigned long StatusRegister;

	success		= SpectCtrlRead(STATUS_REG_ADDR, &StatusRegister);
	if( StatusRegister & DAQ_FIFO_FULL_FLAG ){
		*fifo_full = true;
	}	
	else{
		*fifo_full = false;
	}

//	printf("\n(SpectGetDaqFifoFullFlag) fifo_full  =  %d  \n",*fifo_full);   // debug
//	printf("(SpectGetDaqFifoFullFlag) Status Reg   =  0x%X\n",StatusRegister); //debug

		
	return( success );
}

bool MRClo::SpectGetDaqNwords(unsigned short *nwords)
{
	unsigned long CtrlStatusRegister;
	bool success;

	success = SpectCtrlRead(STATUS_REG_ADDR, &CtrlStatusRegister);
	*nwords = (CtrlStatusRegister & N_WORDS_MASK) >> 8;
	if (ControllerFirmwareRevision == 0) {*nwords &= 0xFFF; } //x
	return success;
}


bool MRClo::SpectDaqRead_LONG(unsigned char *daq_data_vector, int n_words)
{
	unsigned char	buf[8];
	int				ndata		= n_words * 4;
	int				i;
	int bytes_xfer, err;
	bool			success;	
	unsigned short	addr		= DAQ_FIFO_ADDR_LONG;
	unsigned char	cmd			= CMD_CONTROL_ACCESS;


	buf[0] = 'H';
	buf[1] = 'E';
	buf[2] = 'A';
	buf[3] = 'D';
	buf[4] = addr & 0xFF;
	buf[5] = ((addr >> 8) & 0x03) | (cmd & 0x1F) | 0x80;
	buf[6] = ndata & 0xFF;
	buf[7] = (ndata >> 8) & 0xFF;


	bytes_xfer = 0; //8;

	err = libusb_bulk_transfer(SpectHandle, OutEndPoint, buf,
		8, &bytes_xfer, XferTimeOut);

	success = ((err == 0) && (bytes_xfer == 8)) ? true : false;

	if( success )
	{
		err = libusb_bulk_transfer(SpectHandle, InEndPoint, daq_data_vector,
			ndata+4, &bytes_xfer, XferTimeOut);
		success = ((err == 0) && (bytes_xfer == (ndata+4))) ? true : false;

		if( success )
		{
			if( daq_data_vector[0] != 'A' || daq_data_vector[1] != 'N' || daq_data_vector[2] != 'S' || daq_data_vector[3] != 'W' ){success = false;}			
		}
		for(i=0; i<ndata; i++){daq_data_vector[i] = daq_data_vector[4+i];}

 		ndata = bytes_xfer - 4;
	}
	return( success );
}

bool MRClo::SpectSetIackDelay(unsigned char iack_delay)
{
	unsigned long static_control_register;

	SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	static_control_register |= (iack_delay << 4) & IACK_DELAY;
	return SpectCtrlWrite(STATIC_CONTROL_REG_ADDR, static_control_register);
}

bool MRClo::SpectGetIackDelay(unsigned char *iack_delay)
{
	unsigned long static_control_register;
	bool success;

	success = SpectCtrlRead(STATIC_CONTROL_REG_ADDR, &static_control_register);
	*iack_delay = (static_control_register & IACK_DELAY) >> 4;
	return success;
}

bool MRClo::SpectRestartDaq(void)
{
	return SpectCtrlWrite(PULSE_CONTROL_REG_ADDR, IRQ_PROCESS_RESTART);
}

bool MRClo::SpectCtrlAllClear(void)
{
	return SpectCtrlWrite(PULSE_CONTROL_REG_ADDR,CLEAR_REGISTERS | CLEAR_ALL_FSM);
}

bool MRClo::SpectClearIrqRamAddr(void)
{
	return SpectCtrlWrite(PULSE_CONTROL_REG_ADDR, IRQ_RAM_CLEAR);
}

bool MRClo::SpectGetCtrlRevision(unsigned char *revision)
{
	unsigned long CtrlStatusRegister;
	bool success;

	success = SpectCtrlRead(STATUS_REG_ADDR, &CtrlStatusRegister);
	*revision = (CtrlStatusRegister & REVISION_MASK) >> 28;
	return success;
}

bool MRClo::SpectDaqRead(unsigned char *daq_data_vector, int n_words)
{
	int i, n_times, last_data_cnt, ndata, pktsize;
	bool success;
/*
	n_times = n_words / 127;
	last_data_cnt = n_words % 127;
	success = true;
	ndata = 508;
	for(i=0; i<n_times; i++)
	{
		ndata = 508;
		success = SpectReadCmdLL(DAQ_FIFO_ADDR, CMD_CONTROL_ACCESS, daq_data_vector+508*i, ndata);
		if( ndata < 508 || success != true )
			break;
	}
	if( ndata < 508 || success != true )
		return false;
	else
	{
		ndata = last_data_cnt*4;
		success = SpectReadCmdLL(DAQ_FIFO_ADDR, CMD_CONTROL_ACCESS, daq_data_vector+508*i, ndata);
		if( ndata < (last_data_cnt*4) || success != true )
			return false;
	}
*/
	pktsize = 126*4;
	n_times = n_words / 126;
	last_data_cnt = n_words % 126;
	success = true;
	ndata = pktsize;
	for(i=0; i<n_times; i++)
	{
		ndata = pktsize;
		success = SpectReadCmdLL(DAQ_FIFO_ADDR, CMD_CONTROL_ACCESS, daq_data_vector+pktsize*i, ndata);
		if( ndata < pktsize || success != true )
			break;
	}
	if( ndata < pktsize || success != true )
		return false;
	else
	{
		ndata = last_data_cnt*4;
		success = SpectReadCmdLL(DAQ_FIFO_ADDR, CMD_CONTROL_ACCESS, daq_data_vector+pktsize*i, ndata);
		if( ndata < (last_data_cnt*4) || success != true )
			return false;
	}
	return true;
}

bool MRClo::SpectDiscrRead(unsigned char *discr_data_vector)
{
	int ndata;
	bool success;
/*
	ndata = 512;
	success = SpectReadCmdLL(IRQ_RAM_ADDR, CMD_CONTROL_ACCESS, discr_data_vector, ndata);
	if( ndata < 512 || success != true )
		return false;
	else
		return true;
*/
	success = SpectClearIrqRamAddr();
	if( success != true )
		return false;

	ndata = 256;
	success = SpectReadCmdLL(IRQ_RAM_ADDR, CMD_CONTROL_ACCESS, discr_data_vector, ndata);
	if( ndata < 256 || success != true )
		return false;
	else
	{
		ndata = 256;
		success = SpectReadCmdLL(IRQ_RAM_ADDR, CMD_CONTROL_ACCESS, discr_data_vector+256, ndata);
		if( ndata < 256 || success != true )
			return false;
		else
			return true;
	}
}

bool MRClo::SpectGetDaqDoneFlag(bool *daq_done)
{
	bool success;
	unsigned long StatusRegister;

	success = SpectCtrlRead(STATUS_REG_ADDR, &StatusRegister);
	if(!success){// mat_debug
		printf("\nError in SpectGetDaqDoneFlag while reading StatusRegister (CB)\n");
	}
	if( StatusRegister & DAQ_DONE_FLAG ){*daq_done = true;}	
	else{*daq_done = false;}
		
	return( success );
}
