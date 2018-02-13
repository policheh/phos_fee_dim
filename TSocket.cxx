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

TSocket::TSocket(){
	fPortLocal = -1;
}

TSocket::~TSocket(){
	Close();
}

int TSocket::Connect(){

	struct hostent *host;
	struct timeval tv;

	host = NULL;

	// socket setup
	fSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if( fSocket == -1 ){
		cout << "FEE DIM Socket open failed." << endl;
		
		return -1;
	}
	
	// bind the socket on local if local port defined
	if( fPortLocal != -1 ){
		fClient.sin_family = AF_INET;
		fClient.sin_addr.s_addr= htonl( INADDR_ANY );
		fClient.sin_port=htons( fPortLocal ); //source port for outgoing packets
		bind( fSocket, (struct sockaddr *)&fClient, sizeof( fClient ));
	}

	// resolve host name
	host = gethostbyname( fHostname );
	if( host == NULL ){
		// IP
		host = gethostbyaddr( fHostname, strlen( fHostname ), AF_INET );
		if( host == NULL ){
			cout << "FEE DIM Host resolution failed. hostname = ";
			cout << fHostname << endl;
			return -1;
		}
	}

	// setup the server address
	fServer.sin_family = AF_INET;
	fServer.sin_port = htons( fPort );
	(void) memcpy( &fServer.sin_addr, host->h_addr, host->h_length );

	tv.tv_sec = 0 ;
	tv.tv_usec = 1000;

	cout << "Socket for host " << fHostname << ":" << fPort << " created." << endl;
	
 	setsockopt ( fSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv );
 
 	CleanSocket();

	return 0;
}

void TSocket::Close(){
  if( fSocket != -1 ){
    shutdown( fSocket, 2 );
    close( fSocket );
		fSocket = -1;
  }
  cout << "Socket closed" << endl;
}

void TSocket::CleanSocket(){

	int status;
	unsigned int rcvsize;
	struct sockaddr_in sender;
	char buf[128];

	usleep(50000);

	status = 1;
	// readd all there is, to clear previous data
  while( status > 0 ){
		rcvsize = sizeof( fServer );
		status = recvfrom( fSocket, buf, 127, MSG_DONTWAIT, 
											 (struct sockaddr *)&sender, &rcvsize );
	}
	if( DCS_DIM_DEBUG > 0 )
		cout << "FEE DIM socket cleaned" << endl;
}

int TSocket::Commands( vector<uint32_t> *outbuf ){

	unsigned int i;
	int status;
	uint32_t buf[DCS_DIM_UDP_BUF_MAX];
	
	if( outbuf->size() > DCS_DIM_UDP_BUF_MAX ){
		cout << "FEE DIM UDP output buffer overflow" << endl;
		return -1;
	}
	
	// copy the data and with propper ordering
	for( i = 0; i < outbuf->size(); i++ ){
		buf[i] = htonl( outbuf->at(i) );
	}
	
	// send the command, size in bytes
	status = sendto( fSocket, buf, outbuf->size() * 4, 0, 
									 (struct sockaddr *)&fServer, sizeof( fServer ));
	
	// error sending, repeat
	if( status < 0 ){
		usleep(50000);
		status = sendto( fSocket, buf, outbuf->size() * 4, 0,
										 (struct sockaddr *)&fServer, sizeof( fServer ));
		// fail on error
		if( status < 0 ){
			if( DCS_DIM_DEBUG > 0 )
				cout << "FEE DIM socket send failed" << endl;
			return -1;
		}
	}
	
	if( status != (int)(outbuf->size() * 4 )){
		cout << "FEE DIM socket send size differ" << endl;
		return -1;
	}
	
	return 0;
	
}

int TSocket::Readback( vector<uint32_t> *inbuf ){
	
	int status, i;
	unsigned int rcvsize;
	struct sockaddr_in sender;
	uint32_t buf[DCS_DIM_UDP_BUF_MAX];

	// recieve
	rcvsize = sizeof( fServer );
	status = recvfrom( fSocket, buf, DCS_DIM_UDP_BUF_MAX*4, 0, 
										 (struct sockaddr *)&sender, &rcvsize );
	
	// repeat on fail
	if( status < 0 ){
		if( DCS_DIM_DEBUG > 4 )
			cout << "FEE DIM socket recieve failed" << endl;
		return -1;
	}
	
	if( DCS_DIM_DEBUG > 4 ){
		cout << "FEE DIM socket received " << status << " bytes." << endl;
	}
	
	// byt 4 division check
	if( status % 4 != 0 ){
		cout << "FEE DIM socket received data len not divisible by 4: " << status << endl;
		return -1;
	}
	
	// copy properly ordered data
	for( i = 0; i < status / 4; i++ ){
		inbuf->push_back( ntohl( buf[i] ));
	}
	
	return 0;
}

int TSocket::ReadbackBuffers( vector<uint32_t> *outbuf, vector<uint32_t> *inbuf ){
	
	// send the readback commands, fail on error
	if( Commands( outbuf ))
		return -1;
	
// 	// give some time
// 	usleep( 50000 );
	
	// readback, fail on error
	if( Readback( inbuf ))
		return -1;
	
	return 0;
}



