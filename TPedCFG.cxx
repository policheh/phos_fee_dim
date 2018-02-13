#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include "TPedCFG.hxx"
#include "dim/dis.hxx"
#include "globals.hxx"

TPedCFG::TPedCFG(char * name, vector<TSequencerCommand*> *sequence, int sru, int pedrun) :
  TBaseCFG(name,sequence,0xe80488),fPedRefRun(pedrun),fNumber(sru)
{
  Init();
}

void TPedCFG::Init() 
{ 

  //int fCard = 2; // random FEE card
  int fCard = RandomFEECard();
  fAddress.clear();

  int mod;
  int part;
  
  if(fNumber==1) { mod=1; part=2; } // M1-2
  if(fNumber==2) { mod=1; part=3; } // M1-3
  if(fNumber==3) { mod=2; part=0; } // M2-0
  if(fNumber==4) { mod=2; part=1; } // M2-1
  if(fNumber==5) { mod=2; part=2; } // M2-2
  if(fNumber==6) { mod=2; part=3; } // M2-3
  if(fNumber==7) { mod=3; part=0; } // M3-0
  if(fNumber==8) { mod=3; part=1; } // M3-1
  if(fNumber==9) { mod=3; part=2; } // M3-2
  if(fNumber==10){ mod=3; part=3; } // M3-3
  if(fNumber==11){ mod=4; part=0; } // M4-0
  if(fNumber==12){ mod=4; part=1; } // M4-1
  if(fNumber==13){ mod=4; part=2; } // M4-2
  if(fNumber==14){ mod=4; part=3; } // M4-3
  
  char filename[255];
  sprintf(filename,"Pedestal/%d/Pedestal_%d_SRUM%d-%d_DTC%d.txt",fPedRefRun,fPedRefRun,mod,part,fCard);
  
  ifstream f;
  f.open(filename,ios::in);
 
  int  addr[255];
  char sreg[255],tmp[255]; 
  
  int   altro,channel,ped;
  float pd;

  vector<int> par; // pedestals of one FEE card

  while(!f.eof()){
      
    f>>tmp>>altro>>tmp>>channel>>tmp>>pd;
    ped = (int)pd;
    
    int FEC =  (int)fCard%20;
    int branch = (int)fCard/20;
    
    if(fCard==15) {
      FEC =  (int)1%20;
      branch = (int)1/20;
    }
      
    int GTLaddr = FEC + branch*16;
    int b = altro*2; // # ALTRO shifted up 1 bit 
    int d = channel*2 + b*16; // channnel shifted up 1 bit + ALTRO-bit info 

    sprintf(sreg,"400%02x%02x6",GTLaddr,d);
    charToHex(sreg,addr);

    addAddress(addr[0],DCS_DIM_ALTRO_TYPE,fCard); 
    par.push_back(ped);
  }
 
  fPedestals.push_back(par);
}

void TPedCFG::CalculateStatus()
{
  
  updateTo("1");
  // unsigned int i,j;

  // for(i=0; i<fPedestals.size(); i++) {
  //   vector<int> par = fPedestals.at(i);

  //   for(j=0; j<64; j++) {
  //     int rdb = fReadback.at(j) & 0x00ff;

  //     if( par.at(j) == rdb) { 
  // 	// printf("Pedestal[%d]: 0x%x  Readback[%d]: 0x%x\n", j,par.at(j),j,rdb); 
  //     }

  //     else {
  // 	// printf("  --> Pedestal[%d]: 0x%x  Readback[%d]: 0x%x\n", j,par.at(j),j,rdb);
  // 	updateTo(""); 
  //     }
      
  //   }

  //   // printf("--------------\n\n");
  // }
}

void TPedCFG::charToHex(char *buffer,int *buf) {
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

int TPedCFG::RandomFEECard()
{
  // Find random FEE card which is switched on for sure.
  
  int mod;
  int part;
  
  if(fNumber==1) { mod=1; part=2; } // M1-2
  if(fNumber==2) { mod=1; part=3; } // M1-3
  if(fNumber==3) { mod=2; part=0; } // M2-0
  if(fNumber==4) { mod=2; part=1; } // M2-1
  if(fNumber==5) { mod=2; part=2; } // M2-2
  if(fNumber==6) { mod=2; part=3; } // M2-3
  if(fNumber==7) { mod=3; part=0; } // M3-0
  if(fNumber==8) { mod=3; part=1; } // M3-1
  if(fNumber==9) { mod=3; part=2; } // M3-2
  if(fNumber==10){ mod=3; part=3; } // M3-3
  if(fNumber==11){ mod=4; part=0; } // M4-0
  if(fNumber==12){ mod=4; part=1; } // M4-1
  if(fNumber==13){ mod=4; part=2; } // M4-2
  if(fNumber==14){ mod=4; part=3; } // M4-3

  char filename[255];
  sprintf(filename,"M%d-%d/SRUparam.dat",mod,part);

  ifstream f;
  f.open(filename,ios::in);

  char tmp[255];
  int DTCrefport;
  
  while(!f.eof())
    f>>tmp>>DTCrefport;
  
  printf("RandomFEECard: %d\n",DTCrefport);
  return DTCrefport;
}
