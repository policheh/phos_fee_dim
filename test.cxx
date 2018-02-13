#include <iostream>
#include <stdio.h>
#include "TSequencer.hxx"

#define DCS_DIM_SERVER_ROOT "ALICE/PHOS"


using namespace std;

int main( int argc, char *argv[] ){
	
		int number;
		char buf[30];
		TSequencer *sequencer;
		
		if( argc < 3 ){
			cout << "usage: " << argv[0] << " <sru number> <sru IP>" << endl;
			exit(0);
		}
		
		number = atoi( argv[1] );
		cout << "SRU number: " << number << endl;
		cout << "SRU IP:     " << argv[2] << endl;
	

		sequencer = new TSequencer( "ALICE/PHOS", number, argv[2], 4097 );
		
		sprintf( buf, "SRU%02d", number );
		DimServer::start( buf );

		if( ! sequencer->Initiate()){
		  while( sequencer->GetDoExit() == 0 ){
		    //sleep(10);
		    sequencer->Cycle();
		  }
		}
		
		sequencer->Terminate();
		delete sequencer;	
		
}
