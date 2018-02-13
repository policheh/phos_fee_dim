#ifndef TREGISTERCOMMAND_HH
#define TREGISTERCOMMAND_HH

#include <iostream>
#include "TSequencerCommand.hxx"

#include "dim/dis.hxx"

class TRegister;

using namespace std;

// DIM command class for register object
// creates command channel for a registry
class TRegisterCommand : public DimCommand {
	void commandHandler();
	
	private:
		// parent object pointers
		TRegister *pRegister;

		// data holder
		int fImmediate; // immediate queue
		int fValue; // configured value
		int fUpdate; // updated flag
		int fDelay; // command delay
		
	public:
		TRegisterCommand( TRegister *reg, char *dimaddress, int immediate = 0, int delay = 0 );
		
		int GetValue(){ return fValue; }
		int GetUpdate(){ return fUpdate; }
		void ClearUpdate(){ fUpdate = 0; }
		
		void UpdateValue( vector<TSequencerCommand*> *sequence = 0 ); // initiateds WriteValue if value updated
		void WriteValue( vector<TSequencerCommand*> *sequence = 0 );  // writes the value
		
};

#endif
