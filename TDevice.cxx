#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "globals.hxx"

#include "TDevice.hxx"
#include "TSequencerCommand.hxx"
#include "TRegister.hxx"

using namespace std;

// constructor
TDevice::TDevice( int number, int sru ){
	
	int i;
	
	fNumber = number;
	fType = -1;
	
	fConfig = new vector<TRegister *>;
	fStatus = new vector<TRegister *>;
	fCommand = new vector<TRegister *>;
	fRegister = new vector<TRegister *>;
	
	// prepare by default for full space of 16 bit register addresses
	for( i = 0; i <= 0xff; i++ )
		fRegister->push_back( 0 );

	// allocate command sequence and register arrays
	fSequence = new vector<TSequencerCommand *>;
    
	fCleanFlag = 0;
    fResponseFlag = 0;
	
	fSruNumber = sru;
}

// destructor
TDevice::~TDevice(){

	unsigned int i;

	TSequencerCommand *seqcommand;
	TRegister *reg;

	// erase all commands in queue
	for( i = 0; i < fSequence->size(); i++ ){
		seqcommand = fSequence->at(i);

		delete seqcommand;
	}
	fSequence->clear();
	delete fSequence;

	// erase all commands in queue
	for( i = 0; i < fConfig->size(); i++ ){
		reg = fConfig->at(i);

		delete reg;
	}
	fConfig->clear();
	delete fConfig;

	// erase all commands in queue
	for( i = 0; i < fStatus->size(); i++ ){
		reg = fStatus->at(i);

		delete reg;
	}
	fStatus->clear();
	delete fStatus;

	// erase all commands in queue
	for( i = 0; i < fCommand->size(); i++ ){
		reg = fCommand->at(i);

		delete reg;
	}
	fCommand->clear();
	delete fCommand;
	
	fRegister->clear();
	delete fRegister;
}

// sequence copy to higher object
// this is here to decouple sequence insertions/deletes from different
// threads (DIM / main program)
void TDevice::GetSequence( vector<TSequencerCommand*> *sequence ){
	
	unsigned int i;
	
	// set flag to block additions to the local sequence
	fCleanFlag = 1;
	
	// copy local sequence to the output and clan the local
	for( i = 0; i < fSequence->size(); i++ )
		sequence->push_back( fSequence->at( i ));
	
	// clear the local sequence
	fSequence->clear();
	
	// remove blocking flag
	fCleanFlag = 0;
}

// sequence cleanup
void TDevice::ClearSequence(){
	
	unsigned int i;
	TSequencerCommand *command;
	
	// set flag to block additions to the local sequence
	fCleanFlag = 1;
	
	// copy local sequence to the output and clan the local
	for( i = 0; i < fSequence->size(); i++ ){
		command = fSequence->at( i );
		delete command;
	}
	
	// clear the local sequence
	fSequence->clear();
	
	// remove blocking flag
	fCleanFlag = 0;
}

// build sequnce for configuration
// write updated values by default
// force writes all
void TDevice::ConfigBuildSequence( int force, vector<TSequencerCommand*> *sequence ){
	
	unsigned int i;
	TRegister *reg;
	
	for( i = 0; i < fConfig->size(); i++ ){
		reg = (TRegister*) fConfig->at( i );

		if( force )
			reg->WriteValue( sequence );
		else
			reg->UpdateValue( sequence );
		
	}
}

// build sequence for readback
// read backs registers according to readback frequence
// can be forced to read all
void TDevice::ReadbackBuildSequence( int force ){
	
	unsigned int i;
	TRegister *reg;
	
	for( i = 0; i < fConfig->size(); i++ ){
		reg = (TRegister*) fConfig->at( i );

		reg->Readback( force );
	}
	for( i = 0; i < fStatus->size(); i++ ){
		reg = (TRegister*) fStatus->at( i );

		reg->Readback( force );
	}
	
	for( i = 0; i< fConfigMonitor.size(); i++ ){
	  TBaseCFG* cfg = (TBaseCFG*)fConfigMonitor.at(i);
	  cfg->Readback( force );
	}
}

// processes readback buffer
void TDevice::ReadbackProcess( vector<uint32_t> *inbuf ){

	unsigned int i;
	unsigned int value, address;
	TRegister *reg;
	
	// first check if there is any output at all
	if( inbuf->size() == 0 )
		return;
	
	// go through the buffer by address data pairs
	for( i = 1; i < inbuf->size(); i += 3 ){
		// odd buffer length protection
		if( i + 1 >= inbuf->size() )
			continue;
		
		// clear the R/W bit, if set 
		address = (int)(inbuf->at( i ) & 0x7fffffff);
		
		// try to find the config
		reg = 0;
		if( address < fRegister->size() )
			reg = (*fRegister)[address];

		if( reg == 0 )
			continue;
		
		// get the value and compare to current
		// update if differs
		value = (unsigned int)(inbuf->at( i+1 ));
		
		reg->UpdateReadback( (int)value );
		// if( DCS_DIM_DEBUG > 2 ){
		// 	cout << "FEE DIM readout registry(d): ";
		// 	printf( "%02d : 0x%08x", fNumber, address );
		// 	cout << " value: ";
		// 	printf( "0x%08x", value );
		// 	cout << endl;
		// }
	}
	
	for( i = 0; i< fConfigMonitor.size(); i++ ){
	  TBaseCFG* cfg = (TBaseCFG*)fConfigMonitor.at(i);
	  cfg->ReadbackProcess(inbuf);
	}

}

void TDevice::GetModuleAndPartition(int &mod, int &part)
{
  // Return module and partition (like M2-1) 
  // parent for this device.

  if(fSruNumber==1) { mod=1; part=2; } // M1-2
  if(fSruNumber==2) { mod=1; part=3; } // M1-3
  if(fSruNumber==3) { mod=2; part=0; } // M2-0
  if(fSruNumber==4) { mod=2; part=1; } // M2-1
  if(fSruNumber==5) { mod=2; part=2; } // M2-2
  if(fSruNumber==6) { mod=2; part=3; } // M2-3
  if(fSruNumber==7) { mod=3; part=0; } // M3-0
  if(fSruNumber==8) { mod=3; part=1; } // M3-1
  if(fSruNumber==9) { mod=3; part=2; } // M3-2
  if(fSruNumber==10){ mod=3; part=3; } // M3-3
  if(fSruNumber==11){ mod=4; part=0; } // M4-0
  if(fSruNumber==12){ mod=4; part=1; } // M4-1
  if(fSruNumber==13){ mod=4; part=2; } // M4-2
  if(fSruNumber==14){ mod=4; part=3; } // M4-3  
  
  if(fSruNumber<1 || fSruNumber>14) {
    mod  = -1;
    part = -1;
  }  
}

void TDevice::ResetRegisters()
{
  
  unsigned int i;
  
  int readback = 0;
  int force = 1;
  
  for (i=0;  i<fConfig->size(); i++) {
    fConfig->at(i)->UpdateReadback(readback,force);
  }
  
  for (i=0;  i<fStatus->size(); i++) {
    fStatus->at(i)->UpdateReadback(readback,force);
  }
  
  for (i=0;  i<fConfigMonitor.size(); i++) {
    fConfigMonitor.at(i)->Reset();
  }    
}

void TDevice::Init()
{
  // Re-read monitoring configuration if config files were changed
  
  for( unsigned int i = 0; i < fConfigMonitor.size(); i++ ){
    fConfigMonitor.at(i)->Init();
  }  
}
