#ifndef TSEQUENCERCOMMAND_HH
#define TSEQUENCERCOMMAND_HH

#include <iostream>
#include <vector>

#include "dim/dis.hxx"

using namespace std;

// command sequencer command onject
class TSequencerCommand {
public:
		int fType; // device type
		int fNumber; // device number
		int fMaskH; // device DTC mask high
		int fMaskL; // device DTC mask low
		int fAddress; // command address
		int fValue; // command value
		int fDelay; // delay required after the command in micro seconds
		time_t fTimestamp; // time of command reception
		int fDone; // done
		
		TSequencerCommand( int type, int number, int maskh, int maskl, int address, int value, int delay );
		
		int IsDone(){ return fDone; }
		void SetDone( int done = 1 ){ fDone = done; }
};

#endif
