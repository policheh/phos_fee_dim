#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <iostream>
#include <strings.h>

using namespace std;

int main()
{
   int sockfd,n,i,t;
   struct sockaddr_in servaddr,cliaddr;
   socklen_t len;
   uint32_t mesg[2048];
	 int isread;

   sockfd=socket(AF_INET,SOCK_DGRAM,0);

   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
   servaddr.sin_port=htons(4097);
   bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	 
	 isread = 0;
	 t = 0;

   for (;;)
   {
      len = sizeof(cliaddr);
      n = recvfrom(sockfd,mesg,2048*4,0,(struct sockaddr *)&cliaddr,&len);
//			cout << "." << endl;
			
// 			if( n > 0 ){
// 				cout << "Received " << n << " bytes from port " << cliaddr.sin_port << endl;
// 			}
			if( n > 0 && n % 4 != 0 )
				cout << "  !!! not divisible by 4" << endl;
			for( i = 0; i < n / 4; i+=2 ){
				// buffer read request
				if( ntohl( mesg[i] ) == 0x19 )
					isread = 1;
				else
					isread = 0;

				if( i > 0 ){
					if( i == 2 ){
						if( ! isread )
							printf( "Mask: 0x%06x 0x%05x; ", ntohl( mesg[0] ), ntohl( mesg[1] ));
					}
					if( ntohl( mesg[i] ) & 0x80000000 )
						cout << "R";
					if( ntohl( mesg[i] ) & 0x40000000 )
						cout << "A";
					
					if( ! isread )
						printf( " 0x%x 0x%x;", ntohl( mesg[i] ), ntohl( mesg[i+1] ));
		
// 				cout << "Pair: ";
// 				printf( "0x%08x 0x%08x\n", ntohl( mesg[i] ), ntohl( mesg[i+1] ));
				}
				
			}
			
			// transmit fixed read request
			if( isread ){
// 				cout << " reply.";
				mesg[0] = htonl( 0x80000022 );
				mesg[1] = htonl( 0x11 );
				mesg[2] = htonl( 0x80000023 );
				mesg[3] = htonl( 0x12 );
				mesg[4] = htonl( 0x80000024 );
				mesg[5] = htonl( 0x13 );
				mesg[6] = htonl( 0x80000025 );
				mesg[7] = htonl( 0x14 );
				mesg[8] = htonl( 0x80000020 );
				mesg[9] = htonl( t );
//				sendto(sockfd,mesg,10*4,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
				sendto(sockfd,mesg,0,0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));
				t++;
			}
			else{
				cout << endl;
			}
   }
} 
