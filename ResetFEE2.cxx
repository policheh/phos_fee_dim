#include <dic.hxx>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {

  if(argc<2){
    printf("\tUsage: %s <sru>, for example: %s M1-2\n", argv[0],argv[0]);
    return -1;
  }

  int sru = -1;
  char svc[255];

  if(!strcmp(argv[1],      "M1-2")) {sru=1;}
  else if (!strcmp(argv[1],"M1-3")) {sru=2;}
  else if (!strcmp(argv[1],"M2-0")) {sru=3;}
  else if (!strcmp(argv[1],"M2-1")) {sru=4;}
  else if (!strcmp(argv[1],"M2-2")) {sru=5;}
  else if (!strcmp(argv[1],"M2-3")) {sru=6;}
  else if (!strcmp(argv[1],"M3-0")) {sru=7;}
  else if (!strcmp(argv[1],"M3-1")) {sru=8;}
  else if (!strcmp(argv[1],"M3-2")) {sru=9;}
  else if (!strcmp(argv[1],"M3-3")) {sru=10;}
  else if (!strcmp(argv[1],"M4-0")) {sru=11;}
  else if (!strcmp(argv[1],"M4-1")) {sru=12;}
  else if (!strcmp(argv[1],"M4-2")) {sru=13;}
  else if (!strcmp(argv[1],"M4-3")) {sru=14;}
  else {
    printf("\tSRU %s not exists.\n",argv[1]);
    printf("\tValid SRU names: M1-2 M1-3 M2-0 M2-1 M2-2 M2-3 M3-0 M3-1 M3-2 M3-3 M4-0 M4-1 M4-2 M4-3");
    printf("\n");
    return -1;
  }
  
  sprintf(svc,"ALICE/PHOS/SRU%02d/FEECONFIG2_SET",sru);
  printf("Send 1 to %s\n",svc);
  DimClient::sendCommand(svc,1);

  printf("Sleep 50 sec..\n");
  sleep(50);

  sprintf(svc,"ALICE/PHOS/SRU%02d/HVLOAD_SET",sru);
  printf("Send 0 to %s\n",svc);
  DimClient::sendCommand(svc,0);

}
