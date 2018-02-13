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

  int dtc0, dtc20;
  char filename[80];
  std::string tmp;

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
  
  fstream f;

  sprintf(filename,"%s/TRUparam_TRU00.dat",argv[1]);  
  f.open(filename);

  f>>tmp>>dtc0;
  f.close();
   
  sprintf(filename,"%s/TRUparam_TRU20.dat",argv[1]);  
  f.open(filename);

  f>>tmp>>dtc20;
  f.close();

  sprintf(svc,"ALICE/PHOS/SRU%02d/TRU%02d/TRU_RESET_SET",sru,dtc0);
  printf("\n\tResetting TRU0:  send 1 to %s\n",svc);
  DimClient::sendCommand(svc,1);

  sprintf(svc,"ALICE/PHOS/SRU%02d/TRU%02d/TRU_RESET_SET",sru,dtc20);
  printf("\tResetting TRU20: send 1 to %s\n",svc);
  DimClient::sendCommand(svc,1);

  printf("\tSleep 2 sec..\n\n");
  sleep(2);

  sprintf(svc,"ALICE/PHOS/SRU%02d/DTC_WORD_ALIGN_SET",sru);
  DimClient::sendCommand(svc,1);
  
  sprintf(svc,"ALICE/PHOS/SRU%02d/TRU%02d/TRUCONFIG_SET",sru,dtc0);
  printf("\tConfiguring TRU0:  send 1 to %s\n",svc);
  DimClient::sendCommand(svc,1);

  sprintf(svc,"ALICE/PHOS/SRU%02d/TRU%02d/TRUCONFIG_SET",sru,dtc20);
  printf("\tConfigurung TRU20: send 1 to %s\n\n",svc);
  DimClient::sendCommand(svc,1);


}
