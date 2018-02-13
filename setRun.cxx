#include <dic.hxx>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {

  if(argc<2){
    printf("\tUsage: %s <run-type-number>, for example: %s 1\n", argv[0],argv[0]);
    printf("\tRun type can be: 1-PHYSICS, 2-LED, 3-PEDESTAL, 4-STANDALONE.\n");
    return -1;
  }

  int run = atoi(argv[1]);
  char svc[255];

  for(int iSRU=1; iSRU<=14; iSRU++) {
    sprintf(svc,"ALICE/PHOS/SRU%02d/ALTROCFG_SET",iSRU);
    printf("Service and runtype: %s %d\n",svc,run);
    DimClient::sendCommand(svc,run);

    sprintf(svc,"ALICE/PHOS/SRU%02d/SOR_SET",iSRU);
    DimClient::sendCommand(svc,run); // broadcast 0 to all pedestals
  }
}
