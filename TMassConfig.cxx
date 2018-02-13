#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "dim/dis.hxx"

#include "TMassConfig.hxx"
#include "globals.hxx"

using namespace std;

// constructor
TMassConfig::TMassConfig( char *dimaddress, int type, int number, vector<TSequencerCommand*> *sequence, int *clean, int immediate, int delay )
	: DimCommand( dimaddress, "I:2" ){
		
	if( DCS_DIM_DEBUG > 1 ){
		cout << "FEE DIM adding command: " << dimaddress << endl;
	}

	char buf[200];

	// save values and set defaults
	strcpy( fDimAddress, dimaddress );
	fType = type;   // registry type
	fNumber = number;  // device number
	fImmediate = immediate;
	fDelay = delay;

	fRefresh = 10;   // refresh period to each readout cycle
	fRefreshCntr = 0;  // zero counter
	
	pSequence = sequence;
	pCleanFlag = clean;
	
	fMaskH = -1;
	fMaskL = -1;
	
	fConfigOK = 0;
	
	sprintf( buf, "%s_OK", fDimAddress );
	
	// create service
	fDimService = new DimService( buf, fConfigOK );

	if( DCS_DIM_DEBUG > 1 ){
		cout << "FEE DIM adding service: " << buf << endl;
	}

}

TMassConfig::~TMassConfig(){

	if( fDimService )
		delete fDimService;

}

void TMassConfig::commandHandler(){

	int *data, len;
	vector<int>::iterator it;

	data = (int*)getData();

	len = getSize() / sizeof( int );
	if( len != 2 ){
		if( DCS_DIM_DEBUG > 0 ){
			cout << "FEE DIM error: setting command length is " << len ;
			cout << " " << fDimAddress << endl;
		}
	}
	else{
		// check if clear command
		if( data[0] == -1 ){
			fAddress.clear();
			fValue.clear();
			fReadback.clear();
		}
		else{
			// check if the address already exists in the config (value change)
			it = find( fAddress.begin(), fAddress.end(), data[0] );
			// modify value if yes
			if( it != fAddress.end() ){
				fValue.at( int( it-fAddress.begin() )) = data[1];
				fUpdate.at( int( it-fAddress.begin() )) = 1;
			}
			// insert if not
			else{
				fAddress.push_back( data[0] );
				fValue.push_back( data[1] );
				fReadback.push_back( 0 );
				fUpdate.push_back( 1 );
				fRefreshWaiting.push_back( 0 );
			}
		}

		if( DCS_DIM_DEBUG > 1 ){
			cout << "FEE DIM command received a value: " << fDimAddress;
			cout << " register: " << data[0] << " value: " << data[1] << endl;
		}
	}
	
	// if command is immediate pass through, queue the write
	if( fImmediate )
		WriteValues( 0 );
}

// initiate value write if changed
void TMassConfig::UpdateValues( vector<TSequencerCommand*> *sequence ){
	WriteValues( 0, sequence );
}

// write sequence
void TMassConfig::WriteValues( int force, vector<TSequencerCommand*> *sequence ) {
	
	unsigned int i;
	int address;
	TSequencerCommand *command;
	vector<TSequencerCommand*> *rSequence;
	int *rCleanFlag, fakeFlag;
	
	// standard local sequence
	if( sequence == 0 ){
		rSequence = pSequence;
		rCleanFlag = pCleanFlag;
	}
	// external sequence, where we know that clean will not happen when inserting
	else{
		rSequence = sequence;
		fakeFlag = 0;
		rCleanFlag = &fakeFlag;
	}
	
	if( rSequence != 0 && rCleanFlag != 0 ){
		// write all regs
		for( i = 0; i < fAddress.size(); i++ ){
			
			if(( ! fUpdate.at( i )) && (! force ))
				continue;
			
			// specific address handling for ALTRO registers
			if( fType == DCS_DIM_ALTRO_TYPE )
				address = GetAddressAltro( fAddress.at( i ));
			else
				address = fAddress.at( i );
			
			command = new TSequencerCommand( fType, fNumber, fMaskH, fMaskL,
																				address, fValue.at( i ), fDelay );
			
			// wait if clean flag initiated
			while( *rCleanFlag )
				usleep(100);

			rSequence->push_back( command );
			
			// clean update flag
			fUpdate[i] = 0;
			
			// update value on the next readout
			fRefreshWaiting.at( i ) = DCS_DIM_READBACK_RETRY;
		}

		if( DCS_DIM_DEBUG > 1 )
			cout << "Command inserted in queue." << endl;
	}
}

// returns address index in vector, or -1
int TMassConfig::FindAddress( int address ){
	
	int result;
	
	vector<int>::iterator it;
	
	result = -1;
	
	// search for address
	it = find( fAddress.begin(), fAddress.end(), address );
	// found
	if( it != fAddress.end() )
		result = it - fAddress.begin();
	
	return result;
}

// return full address as transmitted to the ALTRO 
int TMassConfig::GetAddressAltro( int address ){
	
	int i, n;
	
	// set SRU ALTRO bit
	address |= 0x40000000;

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

// processes the readback
void TMassConfig::ReadbackProcess( vector<uint32_t> *inbuf ){
	
	unsigned int i;
	int index, value, address;
	
	// go through the buffer by address data pairs
	for( i = 0; i < inbuf->size(); i += 2 ){
		// odd buffer length protection
		if( i + 1 >= inbuf->size() )
			continue;
		
		// clear the R/W bit, if set 
		address = (int)(inbuf->at( i ) & 0x7fffffff);
		
		// if from ALTRO: clear the ALTRO bit, clear the ALTRO parity bit,
		// clear the BC/AL bit, clear the branch/fec address part
		if( address & ( 1 << 30 )){
			// next if this is not altro type register
			if( fType != DCS_DIM_ALTRO_TYPE )
				continue;
			
			// ALTRO bit 30
			// parity bit 19
			// branch and fec bits 12-16
			// bcast and bcal bits 17,18
			address = address & 0x3ff00fff;
		}
		else if( fType == DCS_DIM_ALTRO_TYPE ){
			// next if expecting altro dat but the mark is missing
			continue;
		}

		// try to find the config
		index = FindAddress( address );
		if( index == -1 )
			continue;
		
		// clear waiting for readback flag
		fRefreshWaiting.at( index ) = 0;
		
		// get the value and compare to current
		// update if differs
		value = (int)(inbuf->at( i+1 ));
		if( value != fReadback.at( index )){
			fReadback.at( index ) = value;
			
			// if( DCS_DIM_DEBUG > 2 ){
			// 	cout << "FEE DIM readout registry(m): ";
			// 	printf( "0x%08x", address );
			// 	cout << " value: ";
			// 	printf( "0x%08x", value );
			// 	cout << endl;
			// }
		}
	}
}

// compose the readback commands
void TMassConfig::Readback( int force ){
	
	unsigned int i;
	int address, rdone;
	TSequencerCommand *command;
	
	// if sequence exists
	if( pSequence != 0 && pCleanFlag != 0 ){
		// do not readout when auto readout off
		if( fRefresh != 0 ){
			rdone = 0;
			
			// read back the whole array of addresses
			for( i = 0; i < fAddress.size(); i++ ){
				// readout time
				if( fRefreshCntr + 1 >= fRefresh || force || fRefreshWaiting.at( i )){
					// set the read bit
					address = fAddress.at( i ) | 0x80000000;
						
					// clear the refresh counter if not forcing
					if( ! force )
						fRefreshCntr = 0;
						
					// add to sequence
					command = new TSequencerCommand( fType, fNumber, fMaskH,
																					 fMaskL, address, 0, 0 );

					// wait if clean flag initiated
					while( *pCleanFlag )
						usleep(100);

					pSequence->push_back( command );
					
					// read done flag
					rdone = 1;
					
					// rise the waiting for readback flag if first read attempt
					// decrement the try counter otherwise
					if( fRefreshWaiting.at( i ) == 0 )
						fRefreshWaiting[i] = DCS_DIM_READBACK_RETRY;
					else
						fRefreshWaiting[i]--;
				}
			}
			// increase refresh counter if no read this cycle
			if( rdone == 0 ){
				fRefreshCntr += 1;
			}
		}
	}
}

// compare readback and config
int TMassConfig::Compare(){

	int result;
	unsigned int i;

	result = 1;

	// compare the values, output 0 if a difference found
	for( i = 0; i < fValue.size(); i++ ){
		if( fValue.at(i) != fReadback.at(i) )
			result = 0;
	}

	if( result != fConfigOK ){
		fConfigOK = result;

		// update the service
		fDimService->updateService();
	}

	return result;
}

void TMassConfig::Reset()
{
  fConfigOK = 0;
  fDimService->updateService();
}
