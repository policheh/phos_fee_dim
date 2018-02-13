#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "dim/dis.hxx"

#include "globals.hxx"
#include "TPing.hxx"

TPing::TPing( const char *dimroot, const char *ipaddress ){
	
	char dimaddress[200];
	
	// save IP address
	sprintf( fAddress, "%s", ipaddress );
	
	// make dim service address
	sprintf( dimaddress, "%s/SRUPING", dimroot );
	
	fAlive = 0;

	// create service
	fDimService = new DimService( dimaddress, fAlive );

}

TPing::~TPing(){
	
	delete fDimService;
}
	

// ping done by calling external ping
// this is to avoid having to run this binary owned by root with suid bit
// as the raw socket is not accessible in linux for non-root users
int TPing::PingSru(){
	
	int result;
	FILE *pfile;
	char buf[300], rbuf[300], *ptr;
	
	// build ping command
	sprintf( buf, "ping -W 1 -c 1 -q -n %s", fAddress );
	
	result = 0;
	
	// execute external
	pfile = popen( buf, "r" );
	if( pfile == NULL ){
		cout << "Error: ping command not opened: " << buf << endl;
	}
	else{
		while( !feof( pfile )){
			// read output and check for received packet summary line
			fgets( rbuf, sizeof( rbuf), pfile );
			// check for 1 received packet
			ptr = strstr( rbuf, "1 received" );
			
			if( ptr != NULL )
				result = 1;
		}
		pclose( pfile );
	}
	
	// service update id needed
	if( result != fAlive ){
		fAlive = result;
		fDimService->updateService();
	}
	
	if( DCS_DIM_DEBUG > 0 ){
		cout << "SRU ping test: " << result << endl;
	}
	
	return result;
}
