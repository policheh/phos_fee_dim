#ifndef TREGISTER_HH
#define TREGISTER_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "globals.hxx"
#include "TRegisterCommand.hxx"
#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// register object
// each register can have command or service
// the command can be passed through immediately or used just to store the value
// service will provide readback of the registry

class TRegister {
	protected:
		// DIM commands
	  TRegisterCommand *fDimCommand;
		// DIM service
	  DimService *fDimService;
		
		vector<TSequencerCommand*> *pSequence;
		int *pCleanFlag;

		char fDimAddress[200]; // DIM address
		int fAddress; // register addresses
		int fType;    // register type
		int fNumber;  // device number
		int fReadback; // readbacks
		int fRefresh; // readback frequency - every Nth ready cycle
		int fRefreshCntr; // readback frequency counter
		int fRefreshWaiting; // if waiting for readback
		
		int fMaskH; // specific device masks if needed
		int fMaskL; // specific device masks if needed
		int fBroadcastAltro; // broadcast flag for ALTRO registers

	public:
		// constructor
		TRegister( char *dimaddress, int address, int type, int number, vector<TSequencerCommand*> *sequence = 0, int *clean = 0 );
		~TRegister();

		void AddService( int refresh = 1 );
		void AddCommand( int immediate = 1, int delay = 0 );
		
		void SetRefresh( int refresh ){ fRefresh = refresh; fRefreshCntr = 0; }
		void SetMasks( int maskh, int maskl ){ fMaskH = maskh; fMaskL = maskl; }
		void SetBroadcastAltro( int bc = 1 ){ fBroadcastAltro = bc; }
		
		void ReadNext(){ fRefreshWaiting = DCS_DIM_READBACK_RETRY; }
		
		char *GetDimAddress(){ return fDimAddress; }
		int GetAddress(){ return fAddress; }
		int GetAddressAltro();
		int GetReadback(){ return fReadback; }
		int GetType(){ return fType; }
		int GetNumber(){ return fNumber; }
		int GetMaskH(){ return fMaskH; }
		int GetMaskL(){ return fMaskL; }
		int GetBroadcastAltro(){ return fBroadcastAltro; }
		int GetValue(){ if( fDimCommand ) return fDimCommand->GetValue(); else return 0; }
		int GetUpdate(){ if( fDimCommand ) return fDimCommand->GetUpdate(); else return 0; }
		
		vector<TSequencerCommand*> *GetSequence(){ return pSequence; }
		int *GetCleanFlag(){ return pCleanFlag; }
		
		// towards PVSS
		virtual void UpdateReadback( int readback, int force = 0 );
		
		// towards SRU
		void UpdateValue( vector<TSequencerCommand*> *sequence = 0 ){ if( fDimCommand ) fDimCommand->UpdateValue( sequence ); }
		void WriteValue( vector<TSequencerCommand*> *sequence = 0 ){ if( fDimCommand ) fDimCommand->WriteValue( sequence ); }
		void Readback(int force = 0);

};

#endif

 
