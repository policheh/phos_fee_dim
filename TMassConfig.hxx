#ifndef TMASSCONFIG_HH
#define TMASSCONFIG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// register object
// each register can have command or service
// the command can be passed through immediately or used just to store the value
// service will provide readback of the registry

class TMassConfig : public DimCommand {
	void commandHandler();

	private:
		// DIM service
	  DimService *fDimService;
		
		vector<TSequencerCommand*> *pSequence;
		int *pCleanFlag;

		char fDimAddress[200]; // DIM address
		vector<int> fAddress; // register addresses
		vector<int> fValue; // register values
		int fType;    // register type
		int fNumber;  // device number
		int fImmediate; // if to update immediately
		int fDelay; // delay after command
		vector<int> fUpdate;  // value update flag
		vector<int> fReadback; // readbacks
		int fRefresh; // readback frequency - every Nth ready cycle
		int fRefreshCntr; // readback frequency counter
		vector<int> fRefreshWaiting; // waiting for readback flag
		
		int fConfigOK; // config readback OK flag
		
		int fMaskH; // specific device masks if needed
		int fMaskL; // specific device masks if needed
		int fBroadcastAltro; // broadcast flag for ALTRO registers

	public:
		// constructor
		TMassConfig( char *dimaddress, int type, int number, vector<TSequencerCommand*> *sequence = 0, int *clean = 0, int immediate = 1, int delay = 0 );
		~TMassConfig();

		void SetRefresh( int refresh ){ fRefresh = refresh; fRefreshCntr = 0; }
		void SetMasks( int maskh, int maskl ){ fMaskH = maskh; fMaskL = maskl; }
		void SetBroadcastAltro( int bc = 1 ){ fBroadcastAltro = bc; }
		
		void ReadNext(){ if( fRefresh > 0 ) fRefreshCntr = fRefresh; }

		char *GetDimAddress(){ return fDimAddress; }
		int GetType(){ return fType; }
		int GetNumber(){ return fNumber; }
		int GetMaskH(){ return fMaskH; }
		int GetMaskL(){ return fMaskL; }
		int GetBroadcastAltro(){ return fBroadcastAltro; }
		
		vector<TSequencerCommand*> *GetSequence(){ return pSequence; }
		int *GetCleanFlag(){ return pCleanFlag; }
		
		// towards SRU
		void UpdateValues( vector<TSequencerCommand*> *sequence = 0 );
		void WriteValues( int force = 1, vector<TSequencerCommand*> *sequence = 0 );

		int FindAddress( int address ); // returns index of a address in vector
		int GetAddressAltro( int address ); // returns ALTRO formated address
		
		void ReadbackProcess( vector<uint32_t> *inbuf ); // processes the readback
		void Readback( int force = 0 );
		int Compare();

                void Reset();

};

#endif

 
