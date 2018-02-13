#include <stdio.h>
#include <iostream>
#include "dic.hxx"

using namespace std;

int main( int argc, char *argv[] ){

	int value;
	
	if( argc < 3 ){
		cout << "usage: " << argv[0] << " <service dim address> <int value>" << endl;
		exit( 0 );
	}

	value = (int)strtol( argv[2], NULL, 0 );
	
	cout << "command: " << argv[1] << " : ";
	printf( "0x%x\n", value );

	DimClient::sendCommand( argv[1], value ); 
	
	return 0;
}
