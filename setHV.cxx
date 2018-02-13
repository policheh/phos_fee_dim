#include <dic.hxx>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {

  if(argc<4){
    printf("Use: %s <file> <sru> <dtc>,\nfor example: %s set_bias_dtc_Module4_RCU0_branch0_card1.txt 2 1\n", argv[0],  argv[0]);
    return -1;
  }
  
  char* filename = argv[1];
  int sru = atoi(argv[2]);
  int dtc = atoi(argv[3]);

  string feename = "FEE";
  string sruname = "SRU";

  if(sru<10) sruname += "0"; sruname += argv[2];
  if(dtc<10) feename += "0"; feename += argv[3]; 
  
  char svc[255];
  sprintf(svc,"ALICE/PHOS/%s/%s/HVBIAS",sruname.c_str(),feename.c_str());
  
  int addr;
  int bias;

  ifstream f;
  f.open(filename,ios::in);

  while(!f.eof()){
    f>>hex>>addr>>hex>>bias;
    printf("Address: %x, bias: %x\n",addr,bias);
    
    int cmd[2] = {};
    cmd[0] = addr;
    cmd[1] = bias;

    DimClient::sendCommand(svc,cmd,sizeof(cmd));
    
  }
  
  char hvload[255];
  sprintf(hvload,"ALICE/PHOS/%s/%s/HVLOAD",sruname.c_str(),feename.c_str());
  
  DimClient::sendCommand(hvload,0x0);  // apply changes!
  f.close();
}
