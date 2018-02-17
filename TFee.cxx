#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "dim/dis.hxx"

#include "TFee.hxx"
#include "TRegisterCalibrated.hxx"
#include "TBusyFeeCFG.hxx"
#include "TApdSettingsCheck.hxx"
#include "TResponseCheck.hxx"
#include "globals.hxx"

using namespace std;

TFee::TFee( const char *serverRoot, int truNum, int sruNum ) : TDevice( truNum, sruNum ){

	char buf[300];
	unsigned int i;
	string str;
	TRegister *reg;
	
	// set device type to FEE
	fType = 0;
	
	// // get fee root
	// sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "REGEN" );
	// // dim address, reg address, type, FEE #, sequence, sequence cleaner
	// reg = new TRegister( buf, 0x1, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	// reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
	// reg->AddService( 30 );
	// fConfig->push_back( reg );
	
	// get fee root
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "LGSEN" );
	reg = new TRegister( buf, 0x2, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
	reg->AddService( 30 );
	fConfig->push_back( reg );

	// get fee root
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "PHYADDR" );
	// dim address, reg address, type, FEE #, sequence, sequence cleaner
	reg = new TRegister( buf, 0x3, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
	reg->AddService( 2 );
	fConfig->push_back( reg );

	// Temperature 1
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "TEMP1" );
	reg = new TRegisterCalibrated( buf, 0x50, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag, 0.25 );
	reg->AddService( 30 );
	fConfig->push_back( reg );

	// Temperature 2
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "TEMP2" );
	reg = new TRegisterCalibrated( buf, 0x55, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag, 0.25 );
	reg->AddService( 30 );
	fConfig->push_back( reg );

	// Temperature 3
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "TEMP3" );
	reg = new TRegisterCalibrated( buf, 0x5a, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag, 0.25 );
	reg->AddService( 30 );
	fConfig->push_back( reg );
	
	// // get fee root
	// sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "EREG0" );
	// // dim address, reg address, type, FEE #, sequence, sequence cleaner
	// reg = new TRegister( buf, 0xa, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	// reg->AddService( 10 );
	// fStatus->push_back( reg );

	// get fee root
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "FMVER" );
	// dim address, reg address, type, FEE #, sequence, sequence cleaner
	reg = new TRegister( buf, 0x20, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	reg->AddService( 60 );
	fStatus->push_back( reg );
	

	// get fee root
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "HVLOAD" );
	// dim address, reg address, type, FEE #, sequence, sequence cleaner
	reg = new TRegister( buf, 0x1e, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
	fCommand->push_back( reg );
	

	// instatiate the mass config for HV APD bias
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "HVBIAS" );
	fHighVoltage = new TMassConfig( buf, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	fHighVoltage->SetRefresh( 60 );
	
	// instatiate the mass config for ALTRO pedestals
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "PEDESTAL" );
	fPedestal = new TMassConfig( buf, DCS_DIM_ALTRO_TYPE, fNumber, fSequence, &fCleanFlag );
	fPedestal->SetRefresh( 60 );

	// ALTRO config OK service
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "ALTRO_OK" );
	fAltroOK = new TRegister( buf, 0xff, DCS_DIM_FEE_TYPE, fNumber, fSequence, &fCleanFlag );
	fAltroOK->AddService( 0 );
	
	// BUSY service
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "BUSY" );
	TBusyFeeCFG* busyCFG = new TBusyFeeCFG( buf, fNumber, fSequence ); busyCFG->SetRefresh(30);
	fConfigMonitor.push_back( busyCFG );

	// APD settings check
	sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "APDCHECK" );
	TApdSettingsCheck* apdCheck = new TApdSettingsCheck(buf,fSequence,fNumber,GetSRUNumber());
	apdCheck->SetRefresh(30);
	fConfigMonitor.push_back(apdCheck);
    
    // Responsiveness check
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_FEE, fNumber, "IS_RESPONSIVE" );
    TResponseCheck* isResponse = new TResponseCheck(buf,fSequence,this);
    isResponse->SetRefresh(30);
    fConfigMonitor.push_back(isResponse);
	
	// scan registers to reg array
	for( i = 0; i < fConfig->size(); i++ ){
		reg = fConfig->at( i );
		(*fRegister)[reg->GetAddress()] = reg;
	}
	for( i = 0; i < fStatus->size(); i++ ){
		reg = fStatus->at( i );
		(*fRegister)[reg->GetAddress()] = reg;
	}

}

TFee::~TFee(){

	delete fHighVoltage;
	delete fAltroOK;

}


// set config altro vector and set check vector flag size
void TFee::SetAltro( vector<TRegister *> *altro ){
	
	unsigned int i;
	
	fAltro = altro;
	fAltroCheck.clear();
	
	for( i = 0; i < fAltro->size(); i++ )
		fAltroCheck.push_back( 0 );
}


void TFee::ConfigBuildSequence( int force, vector<TSequencerCommand*> *sequence ){
	
	// standard function
	TDevice::ConfigBuildSequence( force, sequence );
	
	// and specials
	fHighVoltage->WriteValues( force, sequence );
	fPedestal->WriteValues( force, sequence );
}

void TFee::ReadbackBuildSequence( int force ){
	
	// standard function
	TDevice::ReadbackBuildSequence( force );
	
	// and specials
	fHighVoltage->Readback( force );
	fPedestal->Readback( force );
}

void TFee::ReadbackProcess( vector<uint32_t> *inbuf ){
	
	// standard readback process
	TDevice::ReadbackProcess( inbuf );
	
	// HV settings specific registers
	fHighVoltage->ReadbackProcess( inbuf );
	
	// ALTRO pedestal readback
	fPedestal->ReadbackProcess( inbuf );
	
	// ALTRO config
	CheckAltro( inbuf );
	
	// compare readbacks to config and set config OK flag
	fHighVoltage->Compare();
	fPedestal->Compare();
}

// checks read back altro config against altro config
//
void TFee::CheckAltro( vector<uint32_t> *inbuf ){
	
	unsigned int i, j;
	int value, address;
	TRegister *reg;
	int result;
	
	// first check if the flag vector is of a correct size
	if( fAltro->size() != fAltroCheck.size() ){

		fAltroCheck.clear();
		for( i = 0; i < fAltro->size(); i++ )
			fAltroCheck.push_back( 0 );
	}
	
	// go through the buffer by address data pairs
	for( i = 0; i < inbuf->size(); i += 2 ){
		// odd buffer length protection
		if( i + 1 >= inbuf->size() )
			continue;
		
		// clear the R/W bit, if set 
		address = (int)(inbuf->at( i ) & 0x7fffffff);
		
		// continue if not from ALTRO
		// else keep just the registry address
		if(( address & ( 1 << 30 )) == 0 )
			continue;
		else
			address = address & 0x1f;
		
		// get read back value
		value = (int)(inbuf->at( i+1 ));
		
		// scan the whole altro config
		for( j = 0; j < fAltro->size(); j++ ){
			reg = fAltro->at( j );
			if( reg->GetAddress() == address ){
				if( reg->GetValue() != value )
					fAltroCheck[j] = 0;
				else
					fAltroCheck[j] = 1;
				
				if( DCS_DIM_DEBUG > 0 ){
					cout << "ALTRO FEE config check: ";
					printf( "0x%x 0x%x", address, value );
					cout << endl;
				}
			}
		}
	}
	
	// test all register results
	result = 1;
	for( i = 0; i < fAltroCheck.size(); i++ )
		result &= fAltroCheck[i];

	// update OK service
	fAltroOK->UpdateReadback( result );
}

void TFee::ResetRegisters()
{
  TDevice::ResetRegisters();
  fPedestal->Reset(); 
}
