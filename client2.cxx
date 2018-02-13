#include <iostream>
#include <dic.hxx>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <ctype.h>

using namespace std;

int main( int argc, char *argv[] )
{
  // initmask corresponds to the DTC ports to be activated.

  // For example,  initmask_0_20 = 0x7e initiates FEE cards 
  // connected to the following ports:
  //  0 1 1 1   x x x x   x x x x   x x x x   x x x x 
  //  1 1 1 0   x x x x   x x x x   x x x x   x x x x
  // so 1111110 (bin) = 0x7e (hex).

  // Another example: switch on only FEE card on DTC port 2: 
  //  0 1 0 0   x x x x   x x x x   x x x x   x x x x 
  //  0 0 0 0   x x x x   x x x x   x x x x   x x x x
  // 00000100 (bin) = 0x4 (hex).

  // 0x7ffe #Card Switch ON from 1 to 14, out of 0-19
  // 0x7ffe #Card Switch ON from 21 to 34, out of 20-39
  
  // Working only with firmware 0x14072501 and later!!

  if( argc < 2 ){
    cout << "usage: " << argv[0] << "  <sru>" << endl;
    exit(0);
  }  

  char svc[255];
  int sru = atoi(argv[1]);
  
  string sruname = "SRU";
  if(sru<10) sruname += "0"; sruname += argv[1];
  
  sprintf(svc,"ALICE/PHOS/%s/FEECONFIG_SET",sruname.c_str());
  DimClient::sendCommand(svc,0x0);
  
}
