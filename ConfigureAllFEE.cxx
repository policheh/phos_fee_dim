#include <dic.hxx>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <ctime>

using namespace std;

int main(int argc, char* argv[]) {
  
  if (argc<3 ) {
    printf("Usage: %s <sru> <run type>\n",argv[0]);
    printf("       to create common paramter for SRU, FEE and ALTRO.\n");
    printf("       run type can be standalone, pedestal, led and physics\n");
    return -1;
  }
  
  char svc[255];
  int command = -111;

  clock_t startTime, endTime, clockTicksTaken;
  double timeInSeconds;

  int sru = atoi(argv[1]);
  char* type = argv[2];

  if(!strcmp(type,"UNDEF")) command = 0;
  else if(!strcmp(type,"PHYS")) command = 1;
  else if(!strcmp(type,"LED")) command = 2;
  else if(!strcmp(type,"PED")) command = 3;
  else if(!strcmp(type,"STANDALONE")) command = 4;
  else {
    printf("Allowed run types are: UNDEFINED,PHYS,LED,PED,STANDALONE\n");
    return -1;
  }  

  startTime = clock();

  string sruname = "SRU";
  if(sru<10) sruname += "0"; sruname += argv[1];

  sprintf(svc,"ALICE/PHOS/%s/FEECONFIG_SET",sruname.c_str());
  DimClient::sendCommand(svc,0);
  
  sprintf(svc,"ALICE/PHOS/%s/ALTROCFG_SET",sruname.c_str());
  DimClient::sendCommand(svc,command);

  sprintf(svc,"ALICE/PHOS/%s/HVLOAD_SET",sruname.c_str());
  DimClient::sendCommand(svc,0);  

  while(1) {

    sprintf(svc,"ALICE/PHOS/%s/ALTROCFG",sruname.c_str());
    DimCurrentInfo statusALTRO(svc,"Not available");

    sprintf(svc,"ALICE/PHOS/%s/HVCFG",sruname.c_str());
    DimCurrentInfo statusHV(svc,"Not available");

    bool altroOK = !(strcmp(type,statusALTRO.getString()));
    bool hvOK = !(strcmp("PHYS",statusHV.getString()));

    if( altroOK && hvOK ) {
      cout<<"Run type is set to "<<statusALTRO.getString()<<endl;
      break;
    }
  }
  
  endTime = clock();

  clockTicksTaken = endTime - startTime;
  timeInSeconds = clockTicksTaken / (double) CLOCKS_PER_SEC;

  cout<<sruname.c_str()<<": "<<timeInSeconds<<" sec ("<<clockTicksTaken<<" ticks)"<<endl;
  

}

