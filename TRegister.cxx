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
TRegister::TRegister( char *dimaddress, int address, int type, int number, vector<TSequencerCommand*> *sequence, int *clean ){

	// save values and set defaults
	strcpy( fDimAddress, dimaddress );
	fAddress = address;
	fType = type;   // registry type
	fNumber = number;  // device number
	fReadback = 0; // default readback
	fRefresh = 1;   // refresh period to each readout cycle
	fRefreshCntr = 0;  // zero counter
	fRefreshWaiting = 0;
	
	pSequence = sequence;
	pCleanFlag = clean;
	
	fMaskH = -1;
	fMaskL = -1;
	
	fDimCommand = 0;
	fDimService = 0;
}

TRegister::~TRegister(){

	if( fDimCommand )
		delete fDimCommand;
	if( fDimService )
		delete fDimService;

}

// return full address as transmitted to the ALTRO 
int TRegister::GetAddressAltro(){
	
	int address, i, n;
	
	// set SRU ALTRO bit
	address = fAddress | 0x40000000;

	// altro broadcast bit
	if( fBroadcastAltro )
		address |= 0x40000;

	// compute parity
	n = 0;
	for( i = 0; i < 19; i++ ){
		if( address & ( 1 << i ))
			n++;
	}
	// make the parity even using the parity bit
	if( n % 2 )
		address |= 0x80000;

	return address;
}

void TRegister::AddService( int refresh ){
	
	// set refresh interval
	fRefresh = refresh;
	fRefreshCntr = 0;
	
	// create service
	fDimService = new DimService( fDimAddress, fReadback );

	if( DCS_DIM_DEBUG > 1 ){
		cout << "FEE DIM adding service: " << fDimAddress << endl;
	}

}

void TRegister::AddCommand( int immediate, int delay ){
	
	char buf[200];
	
	// prepare dim address
	sprintf( buf, "%s_SET", fDimAddress );

	fDimCommand = new TRegisterCommand( this, buf, immediate, delay );
}

// update value and service if needed
void TRegister::UpdateReadback( int readback, int force ){
	
	int oldval;
	
	oldval = fReadback;
	fReadback = readback;
	
	// clear waiting for readback flag
	fRefreshWaiting = 0;
	
	if( fDimService && ( force || oldval != readback ))
		fDimService->updateService();
}


void TRegister::Readback( int force ){
	
	int address;
	TSequencerCommand *command;
	
	// no readbacks when no service
	if( fDimService == 0 )
		return;
	
	// if sequence exists
	if( pSequence != 0 && pCleanFlag != 0 ){
		// do not readout when auto readout off
		if( fRefresh > 0 ){
			// readout time
			if( fRefreshCntr + 1 >= fRefresh || force || fRefreshWaiting ){
				// set the read bit
				address = fAddress | 0x80000000;
					
				// clear the refresh counter if not forcing
				if( ! force )
					fRefreshCntr = 0;
					
				// add to sequence
				command = new TSequencerCommand( fType, fNumber, fMaskH, fMaskL, address, 0, 0 );

				// wait if clean flag initiated
				while( *pCleanFlag )
					usleep(100);

				pSequence->push_back( command );
				
				// rise the waiting for readback flag if first read attempt
				// decrement the try counter otherwise
				if( fRefreshWaiting == 0 )
					fRefreshWaiting = DCS_DIM_READBACK_RETRY;
				else
					fRefreshWaiting--;
			}
			// increase refresh counter
			else{
				fRefreshCntr += 1;
			}
		}
	}
}
