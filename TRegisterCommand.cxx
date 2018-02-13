#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "dim/dis.hxx"

#include "TRegister.hxx"
#include "TRegisterCommand.hxx"
#include "globals.hxx"

using namespace std;

// constructor
TRegisterCommand::TRegisterCommand( TRegister *reg, char *dimaddress, int immediate, int delay )
	: DimCommand( dimaddress, "I" ) {

	// pointer to parent
	pRegister = reg;

	// save values
	fImmediate = immediate;
	fDelay = delay;

	if( DCS_DIM_DEBUG > 1 )
		cout << "FEE DIM adding command: " << dimaddress << endl;
}

void TRegisterCommand::commandHandler(){

	// get command value, mark as updated
	fValue = getInt();
	fUpdate = 1;

 	if( DCS_DIM_DEBUG > 1 ){
 		cout << "FEE DIM command received: " << pRegister->GetDimAddress() << " value: " << fValue << endl;
 	}

	// if command is immediate pass through, queue the write
	if( fImmediate )
		WriteValue();
}

// initiate value write if changed
void TRegisterCommand::UpdateValue( vector<TSequencerCommand*> *sequence ) {
	if( fUpdate )
		WriteValue( sequence );
}

void TRegisterCommand::WriteValue( vector<TSequencerCommand*> *sequence ) {
	
	int address;
	TSequencerCommand *command;
	vector<TSequencerCommand *> *pSequence;
	int *pCleanFlag, fakeFlag;
	
	// standard local sequence
	if( sequence == 0 ){
		pSequence = pRegister->GetSequence();
		pCleanFlag = pRegister->GetCleanFlag();
	}
	// external sequence, where we know that clean will not happen when inserting
	else{
		pSequence = sequence;
		fakeFlag = 0;
		pCleanFlag = &fakeFlag;
	}

	if( pSequence != 0 && pCleanFlag != 0 ){
		// specific address handling for ALTRO registers
		if( pRegister->GetType() == DCS_DIM_ALTRO_TYPE )
			address = pRegister->GetAddressAltro();
		else
			address = pRegister->GetAddress();
		
		command = new TSequencerCommand( pRegister->GetType(), pRegister->GetNumber(),
																		 pRegister->GetMaskH(), pRegister->GetMaskL(),
																		 address, fValue, fDelay );
		
		// wait if clean flag initiated
		while( *pCleanFlag )
			usleep(100);

		pSequence->push_back( command );

		if( DCS_DIM_DEBUG > 1 )
			cout << "Command inserted in queue." << endl;
	}
	
	// set to read back in the next read cycle
	pRegister->ReadNext();
	
	// clean the updated flag
	fUpdate = 0;
}

