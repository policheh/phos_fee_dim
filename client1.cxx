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

  if( argc < 4 ){
    cout << "usage: " << argv[0] << "  <sru> "<<" <initmask_0_19>  <initmask_20_39>" << endl;
    exit(0);
  }  

  int sru = atoi(argv[1]);
  int initmask_0_19  = (int)(atof(argv[2]));
  int initmask_20_39 = (int)(atof(argv[3]));
  
  string sruname = "SRU";
  if(sru<10) sruname += "0"; sruname += argv[1];

  char svc[255];
  sprintf(svc,"ALICE/PHOS/%s/LOADFLASH_SET",sruname.c_str());

  if( argc > 4) { // load flash
    DimClient::sendCommand(svc,0x0);
    sleep(10);
  }

  sprintf(svc,"ALICE/PHOS/%s/DTCMASKL_SET",sruname.c_str());
  DimClient::sendCommand(svc,initmask_0_19);

  sprintf(svc,"ALICE/PHOS/%s/DTCMASKH_SET",sruname.c_str());
  DimClient::sendCommand(svc,initmask_20_39);

  sprintf(svc,"ALICE/PHOS/%s/TTC_CLOCK_SET",sruname.c_str());
  DimClient::sendCommand(svc,0x00000029);
  sleep(2);
  
  //Reset SRU
  sprintf(svc,"ALICE/PHOS/%s/SRU_RESET_SET",sruname.c_str());
  DimClient::sendCommand(svc,0x0);
  sleep(8);

  //Phase Scan
  sprintf(svc,"ALICE/PHOS/%s/SRU_PHASE_SCAN_SET",sruname.c_str());
  DimClient::sendCommand(svc,0x10000D2);
  sleep(2);
  
  //DTC word alignment
  sprintf(svc,"ALICE/PHOS/%s/DTC_WORD_ALIGN_SET",sruname.c_str());
  DimClient::sendCommand(svc,0x0);
  sleep(1);
 
  //DTC Error Test
  sprintf(svc,"ALICE/PHOS/%s/DTC_ERR_TEST_SET",sruname.c_str());
  DimClient::sendCommand(svc,0x0);
  sleep(1);

  sprintf(svc,"ALICE/PHOS/%s/DTC_READOUT_MASKL_SET",sruname.c_str());
  DimClient::sendCommand(svc,initmask_0_19 ^ 0xfffff);  // Readout mask of DTC0-19 links.

  sprintf(svc,"ALICE/PHOS/%s/DTC_READOUT_MASKH_SET",sruname.c_str());
  DimClient::sendCommand(svc,initmask_20_39 ^ 0xfffff); // Readout mask of DTC20-39 links.

  //Set physical address
  char phyad[80];

  for(int i=1; i<=14; i++) {
    if(i<10)
      sprintf(phyad,"ALICE/PHOS/%s/FEE0%d/PHYADDR_SET",sruname.c_str(),i);
    else
      sprintf(phyad,"ALICE/PHOS/%s/FEE%d/PHYADDR_SET",sruname.c_str(),i);
    DimClient::sendCommand(phyad,i);
  }
  
  for(int i=21; i<=34; i++) {
    sprintf(phyad,"ALICE/PHOS/%s/FEE%d/PHYADDR_SET",sruname.c_str(),i);
    DimClient::sendCommand(phyad,i-20+16);
  }

  printf("SRU%d finished.\n",sru);
}
