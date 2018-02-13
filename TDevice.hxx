#ifndef TDEVICE_HH
#define TDEVICE_HH

#include <iostream>

#include "TSequencerCommand.hxx"
#include "TRegister.hxx"
#include "TBaseCFG.hxx"

using namespace std;

class TDevice {
	protected:
		
		vector<TSequencerCommand*> *fSequence;
		int fCleanFlag;

		// registry arrays
		vector<TRegister *> *fConfig;
		vector<TRegister *> *fStatus;
		vector<TRegister *> *fCommand;
		vector<TRegister *> *fRegister;
		
                // monitoring services
                vector<TBaseCFG *> fConfigMonitor;

		int fNumber;
		int fType; // device type (0 = FEE, 1 = TRU);

                int fSruNumber; // parent SRU

	public:
		
                TDevice( int number, int sru = -1 );
		virtual ~TDevice();
		
		int IsFEE() { if( fType == 0 ) return 1; else return 0; }
		int IsTRU() { if( fType == 1 ) return 1; else return 0; }

		int  GetNumber(){ return fNumber; }
                int  GetSRUNumber(){ return fSruNumber; }
                void GetModuleAndPartition(int &mod, int &part);
		
		void GetSequence( vector<TSequencerCommand*> *sequence );
		void ClearSequence();
		virtual void ConfigBuildSequence( int force = 0, vector<TSequencerCommand*> *sequence = 0 );
		virtual void ReadbackBuildSequence( int force = 0 );
		virtual void ReadbackProcess( vector<uint32_t> *inbuf ); // processes the readback
                virtual void ResetRegisters();
                
                virtual void Init();       
};

#endif
 
