#ifndef TSOCKET_HH
#define TSOCKET_HH

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "globals.hxx"

using namespace std;

// command sequencer
class TSocket {
	public:

		int fSocket;
		struct sockaddr_in fServer;
		struct sockaddr_in fClient;

		char fHostname[100];
		int fPort;
		int fPortLocal;
		
		// constructor
		TSocket();
		~TSocket();

		// pprocess
		int Connect(); // initiates the server
		void Close(); // closes
		void CleanSocket(); // cleans the input socket

		// command sequencing
		int Commands( vector<uint32_t> *outbuf ); 
		int Readback( vector<uint32_t> *inbuf ); 
		int ReadbackBuffers( vector<uint32_t> *outbuf, vector<uint32_t> *inbuf ); // initiates responce buffers read
		
};

#endif
