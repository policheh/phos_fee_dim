#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "dim/dis.hxx"

#include "TSequencerCommand.hxx"
#include "globals.hxx"

using namespace std;

// constructor
TSequencerCommand::TSequencerCommand( int type, int number, int maskh, int maskl, int address, int value, int delay ){

	// save values
	fType = type;
	fNumber = number;
	fAddress = address;
	fValue = value;
	fDelay = delay;
	fTimestamp = time( NULL );
	fDone = 0;
	
	// mask - autocomplete if needed
	if( number >= 0 && maskh == -1 && maskl == -1 ){
		if( number > 19 ){
			fMaskH = 1 << ( number - 20 );
			fMaskL = 0;
		}
		else{
			fMaskH = 0;
			fMaskL = 1 << number;
		}
	}
	else{
		fMaskH = maskh;
		fMaskL = maskl;
	}

	
	if( DCS_DIM_DEBUG > 1 ){
		cout << "Sequence command created. Type: " << fType;
		printf( " masks: 0x%x 0x%x", fMaskH, fMaskL );
		cout << " address: ";
		printf( "0x%x", fAddress );
		cout << " value: ";
		printf( "0x%x", fValue );
		cout << endl;
	}

}
