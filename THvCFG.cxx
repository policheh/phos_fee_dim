#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include "THvCFG.hxx"
// #include "dim/dis.hxx"

#include "globals.hxx"

THvCFG::THvCFG(char* name, vector<TSequencerCommand*> *sequence, int sru) :
  TBaseCFG(name,sequence,0xe80488),fNumber(sru)
{
  Init();
}

void THvCFG::Init()
{
  unsigned int i;

  for(i=0; i<fPedestals.size(); i++) {
    vector<int> par = fPedestals.at(i);
    par.clear();
  }
  
  fPedestals.clear();
  fAddress.clear();
  fAddress.clear();
  
  ReadPedestals();
}

void THvCFG::ReadPedestals() {
  
  int addr,bias;
  int mod,part;
  
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
    
  char filename[80];
  FILE* fp;

  int i = RandomFEECard();

  sprintf(filename,"APDsettings/APD_M%d-%d_FEE%.2d.dat",mod,part,i);
  printf("Reading pedestals for THvCFG from %s\n",filename);
  vector<int> par; // pedestals of one FEE card
    
  fp = fopen(filename,"r");
  char line[1000];
  
  while(fgets(line,sizeof(line),fp)){
    
    if (*line == '#') continue; /* ignore comment line */
    sscanf(line,"%x %x",&addr,&bias);
    printf("Address: 0x%x  Bias: 0x%x\n",addr,bias);
      
    addAddress(addr,DCS_DIM_ALTRO_TYPE,i);
    par.push_back(bias); 
  }

  fPedestals.push_back(par);

}

void THvCFG::CalculateStatus()
{

  updateTo("PHYS");
  // unsigned int i,j;
  
  // for(i=0; i<fPedestals.size(); i++) {
  //   vector<int> par = fPedestals.at(i);

  //   for(j=0; j<16; j++) {

  //     if( par.at(j) == fReadback.at(j)) { 
  // 	// printf("Bias[%d]: 0x%x  Readback[%d]: 0x%x\n", j,par.at(j),j,fReadback.at(j)); 
  //     }

  //     else {
  // 	// printf("  --> Bias[%d]: 0x%x  Readback[%d]: 0x%x\n", j,par.at(j),j,fReadback.at(j));
  // 	updateTo(""); 
  //     }
      
  //   }

  //   // printf("--------------\n\n");
  // }
}

void THvCFG::updateTo(const char* name)
{
  sprintf(fStatusC,"%s",name);
}

int THvCFG::RandomFEECard()
{
  // Find random FEE card which is switched on.
  
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
