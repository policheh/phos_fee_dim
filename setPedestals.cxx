#include <dic.hxx>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>

using namespace std;

void charToHex(char *buffer,int *buf) {
  int i;
  int num = 0, tot = 0, base = 1;

  for (i=7;i>=0;i--){
    buffer[i] = toupper(buffer[i]);
    if (buffer[i] < '0' || buffer[i] > '9'){      
      if (buffer[i] > 'F') buffer[i] = 'F';
      num = buffer[i] - 'A';
      num += 10;
    }
    else num = buffer[i] - '0';
    
    tot += num*base;
    base *= 16;
  }
  *buf = tot;
}

int main(int argc, char* argv[])
{
  // 0 - LG, 1 - HG
  // altro: 0 2 3 4
  // channel: 0-15

  if(argc<4){
    printf("Use: %s <file> <sru> <dtc>,\nfor example: %s Pedestal_206461_PHOS_M2-0_DTC1.txt 2 1\n", argv[0],  argv[0]);
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
  sprintf(svc,"ALICE/PHOS/%s/%s/PEDESTAL",sruname.c_str(),feename.c_str());
  
  char tmp[255];
  int altro;
  int channel;
  int ped;
  float pd;

  ifstream f;
  f.open(filename,ios::in);

  while(!f.eof()){
    f>>tmp>>altro>>tmp>>channel>>tmp>>pd;
    ped = (int)pd;
    printf("altro: %d channel: %d ped: %d.",altro,channel,ped);
  
    int FEC =  (int)dtc%20;
    int branch = (int)dtc/20;
    int GTLaddr = FEC + branch*16;

    int b = altro*2; // # ALTRO shifted up 1 bit 
    int d = channel*2 + b*16; // channnel shifted up 1 bit + ALTRO-bit info 
  
    char sreg[255]; int addr[255];
    sprintf(sreg,"400%02x%02x6",GTLaddr,d);
    //sprintf(sreg,"%02x%02x6",GTLaddr,d);

    charToHex(sreg,addr);
    printf("\tAddress: 0x%x\n",addr[0]);
    
    // int cmd[] = {0x1,1};
    int cmd[2] = {};
    cmd[0] = addr[0]; // address
    cmd[1] = ped; // value

    DimClient::sendCommand(svc,cmd,sizeof(cmd));

  }

  f.close();
}

