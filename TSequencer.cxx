#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "dim/dis.hxx"

#include "TSequencer.hxx"
#include "TDimCounter.hxx"

#include "globals.hxx"

using namespace std;

// constructor
TSequencer::TSequencer( const char *serverRoot, int sruNum, const char *ipAddr, int port ){

	unsigned int i;
	char buf[300];
	
	// zero delays
	fDelaySru = 0;
	for( i = 0; i < 40; i++ )
		fDelayDtc[i] = 0;
	
	// default values
	fRbTimeStamp = 0;
	fPingTimeStamp = 0;
	fDoExit = 0;
	
// 	// create command sequence queue
// 	readbackSequence = new vector<TSequencerCommand *>;
// 	readbackSequence->clear();
// 	configSequence = new vector<TSequencerCommand *>;
// 	configSequence->clear();

	fSequence = new vector<TSequencerCommand *>;
	fSequence->clear();

	// create the SRU object (with all lower electronics)
	fSru = new TSru( serverRoot, sruNum, ipAddr );
	
	// create the network socket and connect it
	fSocket = new TSocket();
	
	sprintf( fSocket->fHostname, "%s", ipAddr );
	fSocket->fPort = port;
	
	// connect the socket
	if( fSocket->Connect() )
		cout << "error: Socket connection problem!!" << endl;
	
	
	sprintf( buf, "%s/SRU%02d", serverRoot, sruNum );
	
	// create SRU ping object
	fPing = new TPing( buf, ipAddr );

	// create counter object
	fCounter = new TDimCounter(buf);
}

TSequencer::~TSequencer(){
	
	unsigned int i;

	TSequencerCommand *command;
	
	// delete counter object
	delete fCounter;
	
	// delete ping object
	delete fPing;
	
	// delete the socket
	delete fSocket;
	
	// delete the HW objects
	delete fSru;

	// clear the queue
	for( i = 0; i < fSequence->size(); i++ ){
		command = fSequence->at(i);

		delete command;
	}
	fSequence->clear();

	delete fSequence;
}

int TSequencer::Initiate(){

	// read config file
	
	// request configuration and wait until not finished
	
// 	// force SRU readback
// 	fSru->ReadbackBuildRequestSru( 1, readbackSequence );
// 	// issue the readback commands for the SRU
// 	ExecuteCommands( readbackSequence );
// 	
// 	// sleep 
// 	usleep( 500000 );
// 	
// 	// readback the result
// 	ReadbackBuffers();
	
	if( DCS_DIM_DEBUG > 0 )
		cout << "FEE DIM server initiation completed." << endl;
	
	return 0;
}

// server work cycle
void TSequencer::Cycle(){
  
	// increase counter
	fCounter->Update();

	int ping, n;
	time_t tnow, tthen;

	time( &tnow );

	// ping SRU
	if( difftime( tnow, fPingTimeStamp ) > DCS_DIM_PING_PERIOD ){
		fPingTimeStamp = tnow;
		n = 0;
		// 3 pings to test
		do{
			ping = fPing->PingSru();
			n++;
		} while( ping == 0 && n < 3 );
	}

	// SRU dead, no commands
	if( !fPing->GetAlive() ){
	        fSru->ResetRegisters();
		fSru->ClearSequence();
	}
	else{

	  // for(int i=1; i<=14; i++) {
	  //   SendCommand(i,0x3,i);
	  // }
	  // for(int i=21; i<=34; i++) {
	  //   SendCommand(i,0x3,i-20+16);
	  // }
	  
		// build readback request
		// readback is done in N s cycles
		// do not build readback when queue is too large
// 		if( difftime( tnow, fRbTimeStamp ) > DCS_DIM_RB_PERIOD
// 			&& fSequence->size() < DCS_DIM_SEQUENCE_SIZE_LIMIT ){
		if( fSequence->size() < DCS_DIM_SEQUENCE_SIZE_LIMIT ){
			fRbTimeStamp = tnow;
			fSru->ReadbackBuildSequence();
		}
		
		// get the buit commands into global sequence
		fSru->GetSequence( fSequence );

		if( DCS_DIM_DEBUG > 1 && fSequence->size() > 0 ){
			cout << "Sequence size: " << fSequence->size() << endl;
		}
		
		// send the sequence (or part of it) to SRU
		ProcessSequence( fSequence );
		
// 		// readback the SRU reply buffers
// 		// again, to make sure all gets out
// 		if( ! DCS_DIM_BUFFER_SINGLE )
// 			ReadbackBuffers();
		
// 		if( DCS_DIM_DEBUG > 1 ){
// 			cout << endl;
// 		}
	}

	// sleep up to 1s in cycle, if the queue is empty
	// CPU load protection
	if( fSequence->size() == 0 ){
		time( &tthen );
		if( tthen == tnow )
			sleep( 1 );
	}
}

void TSequencer::Terminate(){
//	fSocket->Close();
}

// process sequence
void TSequencer::ProcessSequence( vector<TSequencerCommand *> *sequence ){

	unsigned int i, j, dmaskh, dmaskl, nproc;
	int nread[41], doread, buffernum;
	time_t tnow;
	TSequencerCommand *command;
	
	// nothing to do
	if( sequence->size() == 0 )
		return;
	
	nproc = 0;
	dmaskh = 0;
	dmaskl = 0;
	buffernum = -1;
	
	// zero nread
	memset( nread, 0, 41*sizeof( int ));
	
	// process commands, but with ceiling
	// Another ceiling is implemented on read commands. Read requests
	// are monitored for all devices and readback is initiated, once
	// any of the devices crosses a threshold of pending reads to process
	for( i = 0; i < sequence->size() && nproc < DCS_DIM_SEQUENCE_COMMAND_LIMIT; i++ ){

		time( &tnow );
		
		// if the whole SRU is in sleep, sleep
		if( difftime( tnow, fDelaySru ) < 0 ){
			if( DCS_DIM_DEBUG > 1 ){
				cout << "SRU in delay" << endl;
			}
			sleep( (unsigned int)(- difftime( tnow, fDelaySru )));
			
			time( &tnow );
		}

		// make masks of sleeping devices during this cycle
		// the mask is additive, once a device sleeps in a cycle, no more
		// commands will be executed for it. To make sure that
		// commands will not be executed in wrong order.
			
		// for( j = 0; j < 20; j++ ){
		// 	if( difftime( tnow, fDelayDtc[j+20] ) < 0 )
		// 		dmaskh |= 1 << j;
		// 	if( difftime( tnow, fDelayDtc[j] ) < 0 )
		// 		dmaskl |= 1 << j;
		// }
		
		command = sequence->at( i );
		
		// if special command in write mode
		if( command->fType == DCS_DIM_SPECIAL_TYPE &&
			( command->fAddress & 0x80000000 ) == 0 ){
			
			// readback all buffers before special command is executed
			if( DCS_DIM_BUFFER_SINGLE )
				CleanBuffers( 0 );
			else
				ReadbackBuffers();
			
			InterpretSpecialCommand( sequence, i );
			
			continue;
		}

		// check if any of the devices in mask is not on delay
		// continue to next command if yes
		if( command->fMaskH & dmaskh || command->fMaskL & dmaskl ){
			
		  if( DCS_DIM_DEBUG > 1 ){
		    cout << "device in delay ";
		    printf( "0x%06x 0x%05x",command->fMaskH & dmaskh,command->fMaskL & dmaskl );
		    cout << endl;
		  }

		  // add all affected devices by the sleep
		  // to ensure sequence order in all devices
		  dmaskh |= command->fMaskH;
		  dmaskl |= command->fMaskL;
		  
		  continue;
		}
		
		doread = 0;
		
		// increment nread for devices, readout buffers if limit reached for any
		if( command->fAddress & 0x80000000 ){
			
			// readout commands in single buffer scheme when switching target device
			if( DCS_DIM_BUFFER_SINGLE ){
				nread[0]++;
				
				// save first read device
				if( buffernum == -1 )
					buffernum = command->fNumber;
				
				// changing read device, read buffer
				if( buffernum != command->fNumber ){
					ReadbackBuffers( buffernum );
					buffernum = command->fNumber;
				}
				
				// force readout after limited amout of read commands
				if( nread[0] >= DCS_DIM_SEQUENCE_READ_LIMIT )
					doread = 1;
			}
			// keep track of readout commands in multibuffer scheme
			// to readout after DCS_DIM_SEQUENCE_READ_LIMIT read commands
			else{
				// keep track separately for each device about amount of read commands
				// force read when limit for any reached
				for( j = 0; j < 20; j++ ){
					if( command->fMaskL & ( 1 << j ))
						nread[j]++;
					if( nread[j] >= DCS_DIM_SEQUENCE_READ_LIMIT )
						doread = 1;
				}
				for( j = 0; j < 21; j++ ){
					if( command->fMaskH & ( 1 << j ))
						nread[j]++;
					if( nread[j] >= DCS_DIM_SEQUENCE_READ_LIMIT )
						doread = 1;
				}
			}
		} // read command

		// process the command
		ProcessCommand( command );
		nproc++;
		
		// read buffers if enough data waiting
		if( doread ){
			if( DCS_DIM_BUFFER_SINGLE )
				ReadbackBuffers( buffernum );
			else
				ReadbackBuffers();

			// zero nread
			memset( nread, 0, 41*sizeof( unsigned int ));
		}
	}
	
	// readback the SRU reply buffers
	// again, to make sure all gets out
	if( ! DCS_DIM_BUFFER_SINGLE )
		ReadbackBuffers();
	else if( buffernum >= 0 )
		ReadbackBuffers( buffernum );

	CleanDone( sequence );
}

// process sequence
void TSequencer::ProcessSequenceExclusive( vector<TSequencerCommand *> *sequence, int rbfreq ){

	unsigned int i, j, dmaskh, dmaskl;
	int doread, nread[41], buffernum;
	time_t tnow;
	TSequencerCommand *command;
	
	// nothing to do
	if( sequence->size() == 0 )
		return;
	
	// zero nread
	memset( nread, 0, 41*sizeof( int ));
	
	buffernum = -1;

	// process commands, but with ceiling on amount (before buffers are read)
	for( i = 0; i < sequence->size(); i++ ){

		time( &tnow );
		
		// if the whole SRU is in sleep, sleep
		if( difftime( tnow, fDelaySru ) < 0 ){
			if( DCS_DIM_DEBUG > 1 ){
				cout << "SRU in delay" << endl;
			}
			sleep( (unsigned int)(- difftime( tnow, fDelaySru )));
			
			time( &tnow );
		}

		// make masks of sleeping devices during this cycle
		// the mask is additive, once a device sleeps in a cycle, no more
		// commands will be executed for it. To make sure that
		// commands will not be executed in wrong order.
		// mask is reset in exclusive mode
		dmaskh = 0;
		dmaskl = 0;
	
		// for( j = 0; j < 20; j++ ){
		// 	if( difftime( tnow, fDelayDtc[j+20] ) < 0 )
		// 		dmaskh |= 1 << j;
		// 	if( difftime( tnow, fDelayDtc[j] ) < 0 )
		// 		dmaskl |= 1 << j;
		// }
		
		command = sequence->at( i );
		
		// check if any of the devices in mask is not on delay
		// continue to next command if yes
		if( command->fMaskH & dmaskh || command->fMaskL & dmaskl ){
			
			if( DCS_DIM_DEBUG > 1 ){
				cout << "device in delay ";
				printf( "0x%06x 0x%05x", command->fMaskH & dmaskh, command->fMaskL & dmaskl );
				cout << endl;
			}

			// sleep 1 and return to the same command if exclusive
			sleep( 1 );
			i--;
			continue;
		}

		doread = 0;
		
		// increment nread for devices, readout buffers if limit reached for any
		if( command->fAddress & 0x80000000 ){
			// readout commands in single buffer scheme when switching target device
			if( DCS_DIM_BUFFER_SINGLE ){
				nread[0]++;
				
				// save first read device
				if( buffernum == -1 )
					buffernum = command->fNumber;
				
				// changing read device, read buffer
				if( buffernum != command->fNumber ){
					ReadbackBuffers( buffernum );
					buffernum = command->fNumber;
				}
				
				// force readout after limited amout of read commands
				if( nread[0] >= DCS_DIM_SEQUENCE_READ_LIMIT )
					doread = 1;
			}
			// keep track of readout commands in multibuffer scheme
			// to readout after DCS_DIM_SEQUENCE_READ_LIMIT read commands
			else{
				// keep track separately for each device about amount of read commands
				// force read when limit for any reached
				for( j = 0; j < 20; j++ ){
					if( command->fMaskL & ( 1 << j ))
						nread[j]++;
					if( nread[j] >= rbfreq )
						doread = 1;
				}
				for( j = 0; j < 21; j++ ){
					if( command->fMaskH & ( 1 << j ))
						nread[j]++;
					if( nread[j] >= rbfreq )
						doread = 1;
				}
			}
		} // read command

		// process the command
		ProcessCommand( command );
		
		// make sure buffers are read when in exclusive mode (do not get flooded)
		if( doread ){
			if( DCS_DIM_BUFFER_SINGLE )
				ReadbackBuffers( buffernum );
			else
				ReadbackBuffers();

			// zero nread
			memset( nread, 0, 41*sizeof( unsigned int ));
		}
	}
	
	// readback the SRU reply buffers
	// again, to make sure all gets out
	if( ! DCS_DIM_BUFFER_SINGLE )
		ReadbackBuffers();
	else if( buffernum >= 0 )
		ReadbackBuffers( buffernum );

	CleanDone( sequence );
}

// process single command
void TSequencer::ProcessCommand( TSequencerCommand *command ){

	unsigned int i;
	vector<uint32_t> outbuf;
	time_t tnow;
	
	// take only not completed commands
	if( ! command->IsDone() ){

		outbuf.push_back( command->fMaskH );
		outbuf.push_back( command->fMaskL );
		outbuf.push_back( command->fAddress );
		outbuf.push_back( command->fValue );
		
		if( DCS_DIM_DEBUG > 1 ){
		  cout << "FEE DIM Command: ";
		  printf( "masks: 0x%06x 0x%05x addr=0x%08x val=0x%08x",
			  command->fMaskH,command->fMaskL,command->fAddress, command->fValue );
		  cout << " delay: " << command->fDelay;
		  cout << endl;
		}
		
		if(command->fDelay) {
		  printf("Sleeping for %d microseconds..",command->fDelay);
		  usleep(command->fDelay);
		  printf("done.\n");
		}

		fSocket->Commands( &outbuf );
		fCounter->Update();

		// set delays for devices if any
		if( command->fDelay != 0 ){
			time( &tnow );
			
			// SRU delay
			if( command->fMaskH & ( 1 << 20 ))
				fDelaySru = tnow + command->fDelay;
			
			// DTC devices delay
			for( i = 0; i < 20; i++ ){
				if( command->fMaskH & ( 1 << i ))
					fDelayDtc[i+20] = tnow + command->fDelay;
				if( command->fMaskL & ( 1 << i ))
					fDelayDtc[i] = tnow + command->fDelay;
			}
		}

		// mark the command as done
		command->SetDone();
	}
}

// special commands interpretter
void TSequencer::InterpretSpecialCommand( vector<TSequencerCommand *> *sequence, int index ){
	
	unsigned int i;
	TSequencerCommand *command;
	vector<TSequencerCommand *> seqLocal;
	TTru *tru;
	
	command = sequence->at( index );
	
	// power change
	if( command->fNumber == 40 &&
		( command->fAddress == 0x6 || command->fAddress == 0x7 )){
		if( DCS_DIM_DEBUG > 0 ){
			cout << "Power change special command" << endl;
		}
		
		fSru->PowerChangeBuildSequence( &seqLocal );
		
		ProcessSequenceExclusive( &seqLocal, 2 );

		command->SetDone();
	}
	// server exit command
	if( command->fNumber == 41 && command->fAddress == 0x1 ){
		cout << "Server exit command" << endl;
		command->SetDone();
		fDoExit = 1;
	}
	// load flash command
	if( command->fNumber == 41 && command->fAddress == 0x2 ){
		if( DCS_DIM_DEBUG > 0 ){
			cout << "SRU load flash special command" << endl;
		}
		fSru->LoadFlash();
		command->SetDone();
	}
	// initial clock select and SRU reset
	if( command->fNumber == 41 && command->fAddress == 0x3 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "Initial clock select and reset command" << endl;
		}
		fSru->InitClockResetBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		// set alignment done flag
		fSru->SetDTCAligned( 0 );

		command->SetDone();
	}
	// DTC align
	if( command->fNumber == 41 && command->fAddress == 0x4 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "DTC alignment command" << endl;
		}
		fSru->DTCAlignBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		// set alignment done flag
		fSru->SetDTCAligned();

		command->SetDone();
	}
	// Readout and trigger config
	if( command->fNumber == 41 && command->fAddress == 0x5 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "Readout and trigger config command" << endl;
		}
		fSru->RdoTrgConfigBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}
	// Altro config
	if( command->fNumber == 41 && command->fAddress == 0x6 ){
		if( DCS_DIM_DEBUG > 1 ){
		  cout << "ALTROCFG command with value "<<command->fValue<< endl;
		}
		fSru->AltroConfigBuildSequence( &seqLocal, command->fValue );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}
	// HV load
	if( command->fNumber == 41 && command->fAddress == 0x9 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "HVLOAD command with value "<<command->fValue<< endl;
		}
		fSru->HvLoadAllBuildSequence( &seqLocal, command->fValue );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}
	// FEE reset
	if( command->fNumber == 41 && command->fAddress == 0xa ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "FEE reset command" << endl;
		}
		fSru->FeeResetBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}


	// Switch OFF all FEE
	if( command->fNumber == 41 && command->fAddress == 0x61 ){
	  
		if( DCS_DIM_DEBUG > 1 ){
			cout << "FEE OFF command" << endl;
		}

		fSru->FeeOffBuildSequence( &seqLocal );
		ProcessSequenceExclusive( &seqLocal );
		command->SetDone();
	}

	// FEE config
	if( command->fNumber == 41 && command->fAddress == 0x10 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "FEE config command" << endl;
		}
		fSru->FeeConfigBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}

	// FEE config only, don't touch TRUs
	if( command->fNumber == 41 && command->fAddress == 0x62 ){
	
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "FEE config command" << endl;
	  }
	  
	  fSru->FeeConfigBuildSequence2( &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}
	
	//ZS on/off
	if( command->fNumber == 41 && command->fAddress == 0x13 ){
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "ZS_ON command with value "<<command->fValue<< endl;
	  }
	  fSru->ProcessZSOn( command->fValue, &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}
	
	//ZS offset
	if( command->fNumber == 41 && command->fAddress == 0x14 ){
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "ZS_OFFST command with value "<<command->fValue<< endl;
	  }
	  fSru->ProcessZSOffset( command->fValue, &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}

	//ZS threshold
	if( command->fNumber == 41 && command->fAddress == 0x16 ){
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "ZS_THR command with value "<<command->fValue<< endl;
	  }
	  fSru->ProcessZSThr( command->fValue, &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}
	
	// Number of samples
	if( command->fNumber == 41 && command->fAddress == 0x17 ){
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "NSMPL command with value "<<command->fValue<< endl;
	  }
	  fSru->ProcessNSamples( command->fValue, &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}

	// Number of presamples
	if( command->fNumber == 41 && command->fAddress == 0x18 ){
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "NPRESMPL command with value "<<command->fValue<< endl;
	  }
	  fSru->ProcessNPreSamples( command->fValue, &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}

	// Load pedestals
	if( command->fNumber == 41 && command->fAddress == 0x19 ){
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "PEDLOAD command with value "<<command->fValue<< endl;
	  }
	  fSru->PedestalBuildSequence(  &seqLocal );
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}	
	
	// SOR
	if( command->fNumber == 41 && command->fAddress == 0x11 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "SOR command" << endl;
		}
		//fSru->FeeConfigBuildSequence( &seqLocal );

		// Zero pedestals for pedestal run
		if( command->fValue == DCS_DIM_PEDESTAL_RUN )
			fSru->ZeroPedestalBuildSequence( &seqLocal );

		// Reload pedestals for physics, led or standalone run
		// as we need Zero Suppression there
		if( command->fValue == DCS_DIM_PHYSICS_RUN || 
		    command->fValue == DCS_DIM_LED_RUN ||
		    command->fValue == DCS_DIM_STANDALONE_RUN )
		        fSru->PedestalBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
		
		fSru->SorDone();
		
		if( DCS_DIM_DEBUG > 1 ){
			cout << "SOR command done" << endl;
		}
	}
	// TRU SOR
	if( command->fNumber == 41 && command->fAddress == 0x12 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "TRU SOR command" << endl;
		}
		fSru->TruSorBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}
	// Delay scan
	if( command->fNumber == 41 && command->fAddress == 0x15 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "Delay scan command" << endl;
		}
		fSru->DelayScanBuildSequence( &seqLocal );

		ProcessSequenceExclusive( &seqLocal );
		
		command->SetDone();
	}
	// Communication check process
	if( command->fNumber == 41 && command->fAddress == 0x20 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "Communication check command" << endl;
		}
		CommCheck();
		
		command->SetDone();
	}
	
	// TRU config
	if( command->fAddress == 0x30 ){
	  
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "TRU config command" << endl;
	  }
	  
	  tru = (TTru*)fSru->GetDevice( command->fNumber );
	  tru->ConfigBuildSequence( &seqLocal );
	  fSru->UpdateReadoutMask(tru->GetPosition());
	  
	  ProcessSequenceExclusive( &seqLocal );	
	  command->SetDone();
	}
	
	// TRU full reset
	if( command->fAddress == 0x31 ){
	  
	  if( DCS_DIM_DEBUG > 1 ){
	    cout << "TRU full reset command" << endl;
	  }
	  
	  tru = (TTru*)fSru->GetDevice( command->fNumber );
	  tru->ResetBuildSequence( &seqLocal );
	  
	  ProcessSequenceExclusive( &seqLocal );
	  command->SetDone();
	}
	
	// // TRU sor reset
	// if( command->fAddress == 0x32 ){
	// 	if( DCS_DIM_DEBUG > 1 ){
	// 		cout << "TRU full reset command" << endl;
	// 	}
	// 	tru = (TTru*)fSru->GetDevice( command->fNumber );
	// 	tru->ResetSorBuildSequence( &seqLocal );

	// 	ProcessSequenceExclusive( &seqLocal );
		
	// 	command->SetDone();
	// }
	// // TRU full reset
	// if( command->fAddress == 0x33 ){
	// 	if( DCS_DIM_DEBUG > 1 ){
	// 		cout << "TRU full reset command" << endl;
	// 	}
	// 	tru = (TTru*)fSru->GetDevice( command->fNumber );
	// 	tru->ResetErrorBuildSequence( &seqLocal );

	// 	ProcessSequenceExclusive( &seqLocal );
		
	// 	command->SetDone();
	// }
	
	


	// the sequence should be cleand in processing, something wrong if not
	if( seqLocal.size() > 0 ){
		cout << "Special sequence process error" << endl;
		for( i = 0; i < seqLocal.size(); i++ ){
			command = seqLocal.at( i );
			delete command;
		}
	}

	seqLocal.clear();
}

// remove done commands from sequence
void TSequencer::CleanDone( vector<TSequencerCommand *> *sequence ){

	unsigned int i;
	TSequencerCommand *command;
	
	vector<TSequencerCommand*> *sequenceCleaned;

	// create new vector
	sequenceCleaned = new vector<TSequencerCommand *>;
	sequenceCleaned->clear();
	
	// swap the vector content
	sequenceCleaned->swap( *sequence );
	
	// transfer not done commands to the new vector, clear the previous
	for( i = 0; i < sequenceCleaned->size(); i++ ){

		command = sequenceCleaned->at(i);
		
		if( command->IsDone() )
			delete command;
		else
			sequence->push_back( command );
	}

	sequenceCleaned->clear();

	delete sequenceCleaned;
	
}

// pull the SRU readback buffers
void TSequencer::ReadbackBuffers( int buffernum ){
	
	unsigned int i, lastsize, cursize;
	uint32_t cmd;
	time_t tnow;
	vector<int> rbnum;
	vector<uint32_t> outbuf, inbuf;
	
	if( DCS_DIM_DEBUG > 3 )
		cout << "FEE DIM reading buffers" << endl;

	time( &tnow );
	
	// if the whole SRU is in sleep, sleep
	if( difftime( tnow, fDelaySru ) < 0 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "SRU in delay" << endl;
		}
		sleep( (unsigned int)(- difftime( tnow, fDelaySru )));
	}

	// get the numbers of buffers to readout
	// single buffer fix for current sRU
	if( buffernum >= 0 )
		rbnum.push_back( buffernum );
	else
		fSru->ReadbackList( &rbnum );
	
	// configure the DTC addressing
	for( i = 0; i < rbnum.size(); i ++ ){
		outbuf.clear();
		inbuf.clear();
		
		// DTC high = SRU
		cmd = 1 << 20;
		outbuf.push_back( cmd );
		// DTC low = 0
		outbuf.push_back( 0 );
		// ADDRESS
		outbuf.push_back( 0x19 );
		// DTC number as data
		outbuf.push_back( rbnum.at(i) );
		
		if( DCS_DIM_DEBUG > 3 ){
			cout << "FEE DIM Command mask: ";
			printf( "maskh=0x%08x maskl=0x%08x", cmd, 0 );
			cout << endl;
		}
		if( DCS_DIM_DEBUG > 3 ){
			cout << "FEE DIM Command pair: ";
			printf( "addr=0x%08x val=0x%08x", 0x19, rbnum.at(i) );
			cout << endl;
		}
		
		cursize = 0;
		
		do{
			// SRU sends only one read value at a time, despite expectation is
			// that it sends the whole buffer. Hence read as long as SRU sends
			// something, up to command limit (*2 for address, value pair)
		
			// save inbuf size
			lastsize = cursize;
			
			// do the read
			fSocket->ReadbackBuffers( &outbuf, &inbuf );
			cursize = inbuf.size();
			
		} while( lastsize < cursize && cursize < DCS_DIM_SEQUENCE_COMMAND_LIMIT * 2 );
		
		// process the input
		fSru->ReadbackProcess( rbnum.at(i), &inbuf );
	}
	
	// cleanup
	rbnum.clear();
	outbuf.clear();
	inbuf.clear();
}

// pull the SRU readback buffers
void TSequencer::CleanBuffers( int buffernum ){
	
	unsigned int i, lastsize, cursize;
	uint32_t cmd;
	time_t tnow;
	vector<int> rbnum;
	vector<uint32_t> outbuf, inbuf;
	
	if( DCS_DIM_DEBUG > 3 )
		cout << "FEE DIM cleaning buffers" << endl;

	time( &tnow );
	
	// if the whole SRU is in sleep, sleep
	if( difftime( tnow, fDelaySru ) < 0 ){
		if( DCS_DIM_DEBUG > 1 ){
			cout << "SRU in delay" << endl;
		}
		sleep( (unsigned int)(- difftime( tnow, fDelaySru )));
	}

	// get the numbers of buffers to readout
	// single buffer fix for current sRU
	if( buffernum >= 0 )
		rbnum.push_back( 0 );
	else
		fSru->ReadbackList( &rbnum );
	
	// configure the DTC addressing
	for( i = 0; i < rbnum.size(); i ++ ){
		outbuf.clear();
		inbuf.clear();
		
		// DTC high = SRU
		cmd = 1 << 20;
		outbuf.push_back( cmd );
		// DTC low = 0
		outbuf.push_back( 0 );
		// ADDRESS
		outbuf.push_back( 0x19 );
		// DTC number as data
		outbuf.push_back( rbnum.at(i) );
		
		if( DCS_DIM_DEBUG > 3 ){
			cout << "FEE DIM Command mask: ";
			printf( "maskh=0x%08x maskl=0x%08x", cmd, 0 );
			cout << endl;
		}
		if( DCS_DIM_DEBUG > 3 ){
			cout << "FEE DIM Command pair: ";
			printf( "addr=0x%08x val=0x%08x", 0x19, rbnum.at(i) );
			cout << endl;
		}
		
		cursize = 0;
		
		do{
			// SRU sends only one read value at a time, despite expectation is
			// that it sends the whole buffer. Hence read as long as SRU sends
			// something, up to command limit (*2 for address, value pair)
		
			// save inbuf size
			lastsize = cursize;
			
			// do the read
			fSocket->ReadbackBuffers( &outbuf, &inbuf );
			cursize = inbuf.size();
			
		} while( lastsize < cursize && cursize < DCS_DIM_SEQUENCE_COMMAND_LIMIT * 2 );
	}
	
	// cleanup
	rbnum.clear();
	outbuf.clear();
	inbuf.clear();
}


// checks read/write communication
// uses registry 0x8c on SRU and 0x60 on FEE 1 to do repeated read/writes
void TSequencer::CommCheck(){
	
	int saveval, value, i;
	int comretry, bufretry;
	int errcom, errbuf, errwrite;
	int errcomfee, errbuffee, errwritefee;
	
	TSequencerCommand *command;
	
	// make sure there is nothing pending in the buffers
	ReadbackBuffers();
	
	command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x8d, 0, 0 );
	
	// get current value
	ReadbackRegistryDebug( 40, 0x8d, &saveval, &comretry, &bufretry );
	
	errcom = 0;
	errbuf = 0;
	errwrite = 0;
	
	for( i = 0; i < 50000; i++ ){
		
		// write to registry
		command->fValue = i;
		ProcessCommand( command );
		command->SetDone( 0 );
		
		// get current value
		ReadbackRegistryDebug( 40, 0x8d, &value, &comretry, &bufretry );
		
		if( value != i )
			errwrite++;
		
		errcom += comretry;
		errbuf += bufretry;
	}
	
	// put back the saved value
	command->fValue = saveval;
	ProcessCommand( command );
	
	// do the same with FEE register
	command->fAddress = 0x60;
	command->fNumber = 0x1;
	command->fMaskH = 0x0;
	command->fMaskL = 0x2;
	
	// get current value
	ReadbackRegistryDebug( 1, 0x60, &saveval, &comretry, &bufretry );
	
	errcomfee = 0;
	errbuffee = 0;
	errwritefee = 0;
	
	for( i = 0; i < 50000; i++ ){
		
		// write to registry
		command->fValue = i;
		ProcessCommand( command );
		command->SetDone( 0 );
		
		// get current value
		ReadbackRegistryDebug( 1, 0x60, &value, &comretry, &bufretry );
		
		if( value != i )
			errwritefee++;
		
		errcomfee += comretry;
		errbuffee += bufretry;
	}
	
	// put back the saved value
	command->fValue = saveval;
	ProcessCommand( command );

	// print out percentages
	if( DCS_DIM_DEBUG > 1 ){
		cout << "Communication check SRU ---- " << endl;
		cout << "Write errors: " << errwrite << endl;
		cout << "Read command errors: " << errcom << endl;
		cout << "Buffer read errors: " << errbuf << endl;
		cout << "------" << endl;
	}
	// print out percentages
	if( DCS_DIM_DEBUG > 1 ){
		cout << "Communication check FEE ---- " << endl;
		cout << "Write errors: " << errwritefee << endl;
		cout << "Read command errors: " << errcomfee << endl;
		cout << "Buffer read errors: " << errbuffee << endl;
		cout << "------" << endl;
	}
	
	delete command;
}


// single registry read
// tries antil success, outputs number of errors
void TSequencer::ReadbackRegistryDebug( int device, int address, int *value, int *comretry, int *bufretry ){
	
	int maskh, maskl;
	vector<uint32_t> outbuf, inbuf;
	
	*bufretry = 0;
	*comretry = 0;
	
	// masks
	if( device < 20 )
		maskl = 1 << device;
	else
		maskl = 0;
	
	if( device >= 20 )
		maskh = 1 << ( device - 20 );
	else
		maskh = 0;
	
	// cycle read command until response arrivez
	do{
		outbuf.clear();
		inbuf.clear();
		
		// send read command for the given address
		outbuf.push_back( maskh );
		outbuf.push_back( maskl );
		outbuf.push_back( 0x80000000 | address );
		outbuf.push_back( 0 );
			
		fSocket->Commands( &outbuf );
		
		usleep( 1000 );
		
		*bufretry = 0;
		
		// readback buffers, re-send command if failed 3 times
		do{
			outbuf.clear();
			inbuf.clear();
			
			outbuf.push_back( 0x100000 );
			outbuf.push_back( 0 );
			outbuf.push_back( 0x19 );
			outbuf.push_back( 40 );
			
			fSocket->ReadbackBuffers( &outbuf, &inbuf );
			
			// increment retry couter
			if( inbuf.size() == 0 )
				(*bufretry)++;
			
		} while( inbuf.size() == 0 && *bufretry < 3 );
		
		// increment retry couter
		if( inbuf.size() == 0 )
			(*comretry)++;
		
	} while( inbuf.size() == 0 );

	*value = inbuf.at( 1 );
}


// // queues commands in the queue in a form of single register read/writes
// // this function decodes generic commands into detail registry manipulation
// // this function does not prepare the command bit sequence for the UDP sender 
// void TSequencer::GatherCommands(){
// 
// 	unsigned int i;
// 	int maskh, maskl;
// 	TSequencerCommand *command, *incommand;
// 	
// 	vector<TSequencerCommand*> sequence;
// 	
// 	// get the commands
// 	fSru->GetSequence( &sequence );
// 	
// 	// nothing to do
// 	if( sequence.size() == 0 )
// 		return;
// 	
// 	// process the commands, decode, add to command queue
// 	for( i = 0; i < sequence.size(); i++ ){
// 		
// 		incommand = sequence.at( i );
// 		
// 		if( DCS_DIM_DEBUG > 1 ){
// 			cout << "FEE DIM Sequencer received command. Type:";
// 			cout << incommand->fType << " Masks:";
// 			printf( "0x%05x 0x%05x", incommand->fMaskH, incommand->fMaskL );
// 			cout << " Address:";
// 			printf( "0x%02x", incommand->fAddress );
// 			cout << " Value:" << incommand->fValue << endl;
// 		}
// 		
// 		// build mask for mass or special commands
// 		if( incommand->fMaskH < 0 || incommand->fMaskL < 0 ){
// 			
// 			// special handling for ALTRO and SRU commands
// 			if( incommand->fType == DCS_DIM_ALTRO_TYPE ){
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 			}
// 			else if( incommand->fType == DCS_DIM_SRU_TYPE ){
// 				maskh = 1 << 20;
// 				maskl = 0;
// 			}
// 			else
// 				fSru->ReadbackMask( incommand->fType, &maskh, &maskl );
// 			
// 			incommand->fMaskH = maskh;
// 			incommand->fMaskL = maskl;
// 		}
// 		
// 		// special commands
// 		if( incommand->fType == DCS_DIM_SPECIAL_TYPE ){
// 			// SOR
// 			if( incommand->fAddress == 0x0 ){
// 			}
// 			// EOR
// 			else if( incommand->fAddress == 0x1 ){
// 			}
// 			// SET
// 			else if( incommand->fAddress == 0x2 ){
// 				fSru->SetupBuildCommandSru( configSequence );
// 			}
// 			// SETALLFEE
// 			else if( incommand->fAddress == 0x3 ){
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandFee( maskh, maskl, configSequence );
// 			}
// 			// SETALLTRU
// 			else if( incommand->fAddress == 0x4 ){
// 				fSru->ReadbackMask( DCS_DIM_TRU_TYPE, &maskh, &maskl );
// 				cout << "TRU set command maskh: " << maskh << " maskl: " << maskl << endl;
// 				cout << configSequence->size() << endl;
// 				fSru->SetupBuildCommandTru( maskh, maskl, configSequence );
// 				cout << configSequence->size() << endl;
// 			}
// 			// SETALLPEDESTAL
// 			else if( incommand->fAddress == 0x5 ){
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandPedestal( maskh, maskl, configSequence );
// 			}
// 			// SETALLBIAS
// 			else if( incommand->fAddress == 0x6 ){
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandBias( maskh, maskl, configSequence );
// 				// apply the bias
// 				command = new TSequencerCommand( DCS_DIM_FEE_TYPE, maskh, maskl, 0x1e, 0, 2000000 );
// 				configSequence->push_back( command );
// 			}
// 			// SETALLALTROPHYSICS
// 			else if( incommand->fAddress == 0x7 ){
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandAltro( 0, maskh, maskl, configSequence );
// 			}
// 			// SETALLALTROPEDESTAL
// 			else if( incommand->fAddress == 0x8 ){
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandAltro( 1, maskh, maskl, configSequence );
// 			}
// 			// CONFIGURE
// 			else if( incommand->fAddress == 0x9 ){
// 				fSru->SetupBuildCommandSru( configSequence );
// 				fSru->ReadbackMask( DCS_DIM_FEE_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandFee( maskh, maskl, configSequence );
// 				fSru->SetupBuildCommandAltro( 0, maskh, maskl, configSequence );
// 				fSru->SetupBuildCommandPedestal( maskh, maskl, configSequence );
// 				fSru->SetupBuildCommandBias( maskh, maskl, configSequence );
// 				fSru->ReadbackMask( DCS_DIM_TRU_TYPE, &maskh, &maskl );
// 				fSru->SetupBuildCommandTru( maskh, maskl, configSequence );
// 				// apply the bias
// 				command = new TSequencerCommand( DCS_DIM_FEE_TYPE, maskh, maskl, 0x1e, 0, 2000000 );
// 				configSequence->push_back( command );
// 			}
// 			// FEE SET
// 			else if( incommand->fAddress == 0x10 ){
// 				fSru->SetupBuildCommandFee( incommand->fMaskH, incommand->fMaskL, configSequence );
// 			}
// 			// FEE SET PEDESTAL
// 			else if( incommand->fAddress == 0x11 ){
// 				fSru->SetupBuildCommandPedestal( incommand->fMaskH, incommand->fMaskL, configSequence );
// 			}
// 			// FEE SET BIAS
// 			else if( incommand->fAddress == 0x12 ){
// 				fSru->SetupBuildCommandBias( incommand->fMaskH, incommand->fMaskL, configSequence );
// 				// apply the bias
// 				command = new TSequencerCommand( DCS_DIM_FEE_TYPE, maskh, maskl, 0x1e, 0, 2000000 );
// 				configSequence->push_back( command );
// 			}
// 			// TRU SET
// 			else if( incommand->fAddress == 0x20 ){
// 				fSru->SetupBuildCommandTru( incommand->fMaskH, incommand->fMaskL, configSequence );
// 			}
// 		}
// 		// SRU/TRU/FEE commands that require a delay to be imposed after
// 		// identified by type/address pair
// 		// SRU DTCSYNC
// 		else if( incommand->fType == DCS_DIM_SRU_TYPE && incommand->fAddress == 0xa ){
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 3000000 );
// 			configSequence->push_back( command );
// 		}
// 		// SRU TTCRESET
// 		else if( incommand->fType == DCS_DIM_SRU_TYPE && incommand->fAddress == 0xc ){
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 500000 );
// 			configSequence->push_back( command );
// 		}
// 		// SRU ETHRESET
// 		else if( incommand->fType == DCS_DIM_SRU_TYPE && incommand->fAddress == 0xd ){
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 3000000 );
// 			configSequence->push_back( command );
// 		}
// 		// SRU TTCI2C write
// 		else if( incommand->fType == DCS_DIM_SRU_TYPE && incommand->fAddress == 0x16 ){
// 			// save incommand->fValues and write command
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 0 );
// 			configSequence->push_back( command );
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, 0x18, 0, 0 );
// 			configSequence->push_back( command );
// 		}
// 		// FEE ALTRO reset
// 		else if( incommand->fType == DCS_DIM_FEE_TYPE && incommand->fAddress == 0x19 ){
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 2000000 );
// 			configSequence->push_back( command );
// 		}
// 		// FEE HVUPDATE
// 		else if( incommand->fType == DCS_DIM_FEE_TYPE && incommand->fAddress == 0x1e ){
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 2000000 );
// 			configSequence->push_back( command );
// 		}
// 		// TRU RESET
// 		else if( incommand->fType == DCS_DIM_TRU_TYPE && incommand->fAddress == 0x4f ){
// 			// full and clock reset
// 			if( incommand->fValue == 0x1 || incommand->fValue == 0x2 )
// 				command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 3000000 );
// 			else
// 				command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 0 );
// 			configSequence->push_back( command );
// 		}
// 		// no special treatment
// 		else{
// 			// create the command object with no delay
// 			command = new TSequencerCommand( incommand->fType, incommand->fMaskH, incommand->fMaskL, incommand->fAddress, incommand->fValue, 0 );
// 			configSequence->push_back( command );
// 		}
// 		
// 	}// commands
// 	
// 		// clean the commands
// 	fSru->CleanCommands();
// 	
// }
// 
// // TODO: this should be made much smarter, so it composes command
// // sequences and takes care of the delays
// // this function forms the bit sequence for the UDP sender
// void TSequencer::ExecuteCommands( vector<TSequencerCommand*> *sequence ){
// 
// 	unsigned int i;
// 	int maskh, maskl;
// 	vector<uint32_t> outbuf;
// 	
// 	TSequencerCommand *command;
// 	
// 	maskh = -1;
// 	maskl = -1;
// 
// 	// go through the commands
// 	for( i = 0; i < sequence->size(); i++ ){
// 
// 		command = sequence->at(i);
// 		
// 		// take only not completed commands
// 		if( ! command->fDone ){
// 			// check if first command
// 			if( maskh == -1 ){
// 				// TODO: delay checks should come here to skip to next device,
// 				// when the first one in delay
// 				// SRU dealy = do not skipt to any lower
// 				//
// 				maskh = command->fMaskH;
// 				maskl = command->fMaskL;
// 				
// 				// initiate the output buffer DTC header
// 				outbuf.push_back( maskh );
// 				outbuf.push_back( maskl );
// 				
// 				if( DCS_DIM_DEBUG > 1 ){
// 					cout << "FEE DIM Command mask: ";
// 					printf( "maskh=0x%08x maskl=0x%08x", maskh, maskl );
// 					cout << endl;
// 				}
// 			}
// 			
// 			// check if still the same devices are requested, exit cycle if otherwise
// 			if( command->fMaskH == maskh && command->fMaskL == maskl ){
// 				// TODO: delay and size checks here
// 				// to check if the last command does not reqiure delay
// 				outbuf.push_back( command->fAddress );
// 				outbuf.push_back( command->fValue );
// 				
// 				if( DCS_DIM_DEBUG > 1 ){
// 					cout << "FEE DIM Command pair: ";
// 					printf( "addr=0x%08x val=0x%08x", command->fAddress, command->fValue );
// 					cout << endl;
// 				}
// 				
// 				// mark the command as done
// 				command->fDone = 1;
// 			}
// 			else{
// 				// exit the command sequence
// 				i = sequence->size();
// 			}
// 				
// 		}// not done command
// 	}// command sequence
// 	
// 	// sends commands to the socket
// 	if( outbuf.size() > 0 ){
// 		cout << "FEE DIM sending commands to socket" << endl;
// 		fSocket->Commands( &outbuf );
// 	}
// 	
// }

void TSequencer::SendCommand(int dtc, int registry, int value)
{
  // value=0 => read.
  //fSocket->CleanSocket();
  
  vector<uint32_t> cmds;
  cmds.clear();
  
  int device  = dtc;

  int address = registry; 
  if (!value) address = registry | 0x80000000;
  
  //mask with SRU bit
  if( device > 19 ){
    cmds.push_back( 1 << (device - 20 ));
    cmds.push_back( 0 );
  }
  else{
    cmds.push_back( 0 );
    cmds.push_back( 1 << device );
  }
  
  // commands
  cmds.push_back( address );
  cmds.push_back( value );
  
  // send the commands
  fSocket->Commands( &cmds );
}


