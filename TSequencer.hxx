#ifndef TSEQUENCER_HH
#define TSEQUENCER_HH

#include <iostream>
#include <vector>

#include "TSru.hxx"
#include "TSocket.hxx"
#include "TPing.hxx"
#include "TDimCounter.hxx"
#include "TSequencerCommand.hxx"

using namespace std;

// command sequencer
class TSequencer {
	protected:
		// command data holders
		vector<TSequencerCommand *> *fSequence;
		
		TSru *fSru;
		TSocket *fSocket;
		
		TPing *fPing;
                TDimCounter *fCounter;
		
		// delay variables
		int fDelaySru;
		int fDelayDtc[40];
		
		int fRbTimeStamp;
		int fPingTimeStamp;
		int fDoExit;

	public:
		// constructor
		TSequencer( const char *serverRoot, int sruNum, const char *ipAddr, int port );
		~TSequencer();

		// pprocess
		int Initiate(); // initiates the server
		void Cycle(); // loop
		void Terminate(); // termination
		
		int GetDoExit(){ return fDoExit; }

		// command sequencing
		void ProcessSequence( vector<TSequencerCommand *> *sequence ); // processes the sequence
		void ProcessSequenceExclusive( vector<TSequencerCommand *> *sequence, int rbfreq = DCS_DIM_SEQUENCE_READ_LIMIT ); // processes the sequence
		void ProcessCommand( TSequencerCommand *command ); // process single command

		void InterpretSpecialCommand( vector<TSequencerCommand *> *sequence, int index );
		
		void CleanDone( vector<TSequencerCommand *> *sequence );
		void ReadbackBuffers( int buffernum = -1 ); // initiates response buffers read
		void CleanBuffers( int buffernum = -1 ); // cleans response buffers read
		void ReadbackRegistryDebug( int device, int address, int *value, int *comretry, int *bufretry ); 
		
		void CommCheck();
                void SendCommand(int dtc, int registry, int value);
};

#endif
