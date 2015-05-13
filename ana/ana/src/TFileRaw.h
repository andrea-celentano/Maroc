
/*
 * 
 *
 *
 *
 *
 */

// Oggetto per la gestione delle informazioni sulla struttura del Raw File
// e per la verifica dell'integrità di esso.
// un blocco è una sequenza di DWORD (32 bit) di tre tipi Header, Data, Trailer. (HDDDDDTHDDTHTHT)
//

// Inconsistenza Trailer record
// Statistica di Carta: Abbondanza dati per carta
// Statistica di Evento: Abbondanza dati per Evento
// Debug Mode


#ifndef TFILERAW_H
#define TFILERAW_H



#include "TH1I.h"
#include "TCanvas.h"
#include "Riostream.h" 
#include "TDirectory.h"

#include "General.h"


class TFileRaw
{
private:
	//
	bool			fdebugmode;
	unsigned int	fLevel;
	//char			fFileDebug;	// Debug Level for printing
	const char*		fRunName;
	// File Stucture
	TH1I *			f_histo1;	// histogramma Numero di words per blocco
	TH1I *			f_histo2;	// grafico     Nwords vs block index
	TH1I *			f_histo3;	// histogramma Numero di eventi per blocco
	TH1I *			f_histo4;	// grafico     Numero di eventi per blocco vs block index
	TH1I *			f_histo5;	// histogramma Numero di DW per carta
	TH1I *			f_histo6;	// histogramma Numero di DW per evento
	TCanvas *		f_canvas1;

	// Chip involvement
	TH1I*			f_hp1;
	TH1I*			f_hp2;
	TCanvas*		f_canvas;
	unsigned long	f_maroc_present[64]; // Chip reading occurrence
	unsigned long	f_maroc_empty[64];   // Counter for Empty card

	// File level
	unsigned long	fFblock;	// Blocks counters in File
	unsigned long	fFevent;	// Events counters in File
	unsigned long	fFheader;	// Headers counters in File (proportional to Card readings)
	unsigned long	fFtrailer;	// Trailer counters in File (identical to fFHeader if no errors occours!)


	// Block Level
	unsigned short	fBdword;	// DWords number compounding a Block
	int				fBevent;	// Events in a Block
	unsigned short	fBdword_max;// maximum value of fBdword
	int				fBevent_max;// maximum value of fFevent

	// Event Level
	int				fEdata;		// Channels describing event
	int				fEdata_max;	// maximum value of fEdata

	// Card Level
	int				fCdata;		// Channels from a card reading
	int				fCfe;	    // Last readed Card's ID



	// Parsing Finite State Machine, to control formal integrity of a block during parsing operations
	// it has 4 states: 1 Header; 2 Data;	3 Trailer; -1 Initial; (reset)
	// Transitions between them are  determined by what is read.
	// If something in the sequence goes wrong we use counters
	int				f_state;
	int				f_FirstH_Lack;			//
	int				f_FirstHD_Lack;			//
	int				f_H_Lack;				//
	int				f_HD_Lack;				//
	int				f_DT_Lack;				//
	int				f_T_Lack;				//

	// Flimsiness
	int				f_Record_Flimsiness;	// Error
	int				f_Empty_Card;			// Warning



public:
					~TFileRaw();
					TFileRaw();
	
	void			ResetDebugMode	(){fdebugmode=false;}
	void			SetDebugMode	(){fdebugmode=true; printf("TFileRaw::SetDebugMode, fdebugmode = %d, value getted id %d",fdebugmode,this->GetDebugMode());}
	bool			GetDebugMode    (){return fdebugmode;}
	
	void			SetPrintLevel	(unsigned int code){fLevel=code;} // set printing level 
	unsigned int	GetPrintLevel(void)	{return fLevel;}

	
	//void			SetDebugLevel(char value){fFileDebug=value;};
	//char			GetDebugLevel(){return fFileDebug;};

	void			Print();

	void			Init_Histo();
	void			Init_Chip_Histo();
	
	void			DrawBlockInfo(); // graphics the structure of binary file in terms of block lenght (Dword,Event)
	void			GraphStat();     // represents card reading occourrence in the binary file
	
	
	void			ResetHistoPointers();
	void			ResetExtrema();
	void			ResetCardLevel();
	void			ResetEventLevel();
	void			ResetBlockLevel();
	void			ResetFileLevel();
	void			Reset_event_DW_num()		{fEdata		=0;	};	// Event


	void			SetBlock		(unsigned long idx) {fFblock=idx;	};
	void			Set_Block_NWord	(unsigned short nw)	{fBdword=nw;	}; // printf("[TFileRaw::Set_Block_NWord] Size = %d DWord\n",fBdword);
	void			Set_Block_NEvent(int totevent);
	void			Set_Card_ID		(int ID	)			{fCfe = ID;		}  // printf("ID = %d \n",fCfe); // debug

	void			IncrBlock()							{fFblock++;		}; // printf("[TFileRaw::IncrBlock] from now fFblock is %d \n", fFblock);
	void			IncrEvent()							{fFevent++;		}; // printf("[TFileRaw::IncrEvent] from now fFevent is %d \n", fFevent);



	int				GetNEvent()					{return fBevent;		};
	int				GetMAXB_event()				{return	fBevent_max;	};
    int				GetMAXE_DW()				{return	fEdata_max;		};
	unsigned short	GetMAXB_NWord()				{return	fBdword_max;	};
	unsigned long	GetBlockIndex()				{return fFblock;		};
	unsigned long	GetEvent()					{return fFevent;};




	void			Compare_Block_Maxima(); // Block
	void			Compare_Event_Maxima();	// Event

	void			FillB_Nword();			// Block
	void			FillB_NEvent();			// Block
	void			FillE_NDword();			// Event
	void			UpDateCardStat();

	//--------------------------------------------- File Diagnostic (FSM)
	void			Reset_FSM();

	void			Header_Supervisor();
	void			Data_Supervisor();
	void			Trailer_Supervisor();


	int				GetState()				{return f_state;				};
	int				Get_T_Lacks()			{return f_T_Lack;				};
	int				Get_DT_Lacks()			{return f_DT_Lack;				};
	int				Get_H_Lacks()			{return f_H_Lack;				};
	int				Get_FirstH_Lacks()		{return f_FirstH_Lack;			};
	int				Get_HD_Lacks()			{return f_HD_Lack;				};
	int				Get_FirstHD_Lacks()		{return f_FirstHD_Lack;			};
	int				Get_Empty_Card_Check()	{return f_Empty_Card;			};
	int				Get_Record_Flimsiness()	{return f_Record_Flimsiness;	};
	int				Get_card_DW_num()		{return fCdata;					};
	int				Get_Card_ID()			{return fCfe;					};
	int				Get_event_DW_num()		{return fEdata;					};
	unsigned long	GetHeadCount()			{return fFheader;				};
	unsigned long	GetTrailCount()			{return fFtrailer;				};
	unsigned short	Get_NWord()				{return fBdword;				};

	void			Incr_Record_Flimsiness(){f_Record_Flimsiness++	;};


//	Statistic (on FE cards involvement)
	// members functions that operate with element of the 'maroc_present' array
	void			IncrCount(int maroc);
	void			IncrEmptyCount(int maroc);
	bool			IsPresent(int maroc);// true if the element is non-zero
	unsigned long	GetCount(int maroc); // return the specific FE readings counter value
	unsigned long   GetEmptyCount(int maroc); // return the number of empty reading for specific FE

	// members functions that operate with element of the 'maroc_present' array
	void			Reset_Counters();
	bool			IsNotEmpty();

	void			FillEmptyCard(unsigned char geoaddress){f_hp2->Fill(geoaddress,1);}; //obsolete! // Update empty-card histogram

	void			PrintStat();
	
	void			WriteHistoOnDisk();

	void			Incr_event_DW_num()	{fEdata++;		};
	void			Incr_card_DW_num()	{fCdata++;		};


private:

	void			SetH()				{f_state=1;		};
	void			SetD()				{f_state=2;		};
	void			SetT()				{f_state=3;		};

	void			IncrHeadCount()		{fFheader++;	};
	void			IncrTrailCount()	{fFtrailer++;	};


	//--------------------------------------------------------------------------------
	// ATTENZIONE QUESTI METODI FUNZIONANO SOLO SE USATI NEL PUNTO GIUSTO!
	bool			LookFor_T_Lack();		// da usare quando incontro una Header Word
	bool			LookFor_DT_Lack();		// da usare quando incontro una Header Word
	bool			LookFor_H_Lack();		// da usare quando incontro una Data Word
	bool			LookFor_FirstH_Lack();	// da usare quando incontro una Data Word
	bool			LookFor_HD_Lack();		// da usare quando incontro una Trailer Word
	bool			LookFor_FirstHD_Lack();// da usare quando incontro una Trailer Word
	bool			Empty_Card_Check();		//warning No DataWord tra Header e Trailer
	//------------------------------------------------------------------------------------


	void			AddHisto2RootFile(); // Saving info in prova.root

};


#endif