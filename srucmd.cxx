#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "globals.hxx"
#include "TSocket.hxx"


int main(int argc, char *argv[]){
	
	TSocket *mysock;
	vector<uint32_t> cmds, rbs;
	int address, value, device;
	unsigned int i;
	
	if( argc < 5 ){
		cout << "usage: srucmd <ip> <port> <r/w> <device> <registry> [value]" << endl;
		cout << "  where device is the number of DTC link, 40 for SRU itself" << endl;
		cout << "  where registry and values are hexa like 0x1245" << endl;
		exit(0);
	}
	
	mysock = new TSocket();
	
	sprintf( mysock->fHostname, "%s", argv[1] );
	mysock->fPort = (int)(atof(argv[2]));
	
	cout << "ip: " << mysock->fHostname << endl;
	cout << "port: " << mysock->fPort << endl;
	
	if( mysock->Connect() ){
		cout << "Socket connection not established, exit." << endl;
		exit(0);
	}
	
	address = (int)(atof( argv[5] ));
	value = 0;
	device = (int)(atof( argv[4] ));
	
	// set the read bit if needed
	// get the value to write
	if( ! strcmp( argv[3], "r" ) )
	  address = address | 0x80000000;
	else if( argc > 5 )
		value = (int)(atof( argv[6] ));
	else{
		cout << "wrong usage" << endl;
		exit(0);
	}
	
	cmds.clear();
	rbs.clear();
	
	cout << "writing" << endl;
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
	mysock->Commands( &cmds );
	
	if( !strcmp( argv[3], "r" ) ){
		cout << "reading" << endl;
		// setup the readback
		cmds.clear();
		cmds.push_back( 1 << 20 );
		cmds.push_back( 0 );
		cmds.push_back( 0x19 );
		cmds.push_back( 40 );
		
		mysock->ReadbackBuffers( &cmds, &rbs );
		cout << rbs.size() << endl;
 		for( i = 0; i < rbs.size(); i+=2 ){
			cout << i << endl;
 			printf( "0x%x : 0x%x\n", rbs[i], rbs[i+1] );
 		}
	}
	
	delete mysock;
	
	cout << "done" << endl;
	return 0;
}
