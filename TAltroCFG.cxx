#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>

#include "TAltroCFG.hxx"
#include "dim/dis.hxx"

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "globals.hxx"

TAltroCFG::TAltroCFG(char * name, vector<TSequencerCommand*> *sequence, int sru) : 
  TBaseCFG(name,sequence,0xe80488),fNumber(sru)
{
  ReadFEEparams("common/FEEparam_physics.dat");    // "PHYS"
  ReadFEEparams("common/FEEparam_led.dat");        // "LED"
  ReadFEEparams("common/FEEparam_pedestal.dat");   // "PED"
  ReadFEEparams("common/FEEparam_standalone.dat"); // "STANDALONE"
  
  Init();
}

void TAltroCFG::Init()
{
  int dtc = RandomFEECard();
  PrepareAddresses(dtc); // we'll check random FEE card
}

void TAltroCFG::PrepareAddresses(int dtc)
{
  fAddress.clear();
  addAddress(0x27, DCS_DIM_INTERPRETED_TYPE, 40);  //ALTROCFG1
  
  int FEC =  (int)dtc%20;
  int branch = (int)dtc/20;
  int GTLaddr = FEC + branch*16;
  
  int altro[] = {0,2,3,4};
  int a,b;
  
  char sreg[255];
  int addr[255];

  a =  altro[0];
  b = a*2; // ALTRO shifted up 1 bit
  
  sprintf(sreg,"400%02x%01x0B",GTLaddr,b); // ZS
  charToHex(sreg,addr);
  addAddress(addr[0],DCS_DIM_ALTRO_TYPE,dtc);

  sprintf(sreg,"400%02x%01x08",GTLaddr,b); // ALTRO_ZSTHR
  charToHex(sreg,addr);
  addAddress(addr[0],DCS_DIM_ALTRO_TYPE,dtc);
  
  sprintf(sreg,"400%02x%01x0A",GTLaddr,b); // Num. of samples
  charToHex(sreg,addr);  
  addAddress(addr[0],DCS_DIM_ALTRO_TYPE,dtc);

  sprintf(sreg,"400%02x%01x0C",GTLaddr,b); // Num. of presamples
  charToHex(sreg,addr);  
  addAddress(addr[0],DCS_DIM_ALTRO_TYPE,dtc);
  
  addAddress(0x2,DCS_DIM_ALTRO_TYPE,dtc); // LG suppression
}

void TAltroCFG::CalculateStatus()
{
  updateTo("PHYS");
  // unsigned int i;
  
  // for(i=0; i<fFeeParams.size(); i++) {
  //   vector<int> par = fFeeParams.at(i);
    
  //   if( par.at(0) != fReadback.at(0) ) continue;
  //   if( par.at(1) != fReadback.at(1) ) continue;
  //   if( par.at(2) != fReadback.at(2) ) continue;
  //   if( par.at(3) != fReadback.at(3) ) continue;
  //   if( par.at(4) != fReadback.at(4) ) continue;
  //   if( par.at(5) != fReadback.at(5) ) continue;
    
  //   if(i==0)      { updateTo("PHYS");       break; }
  //   else if(i==1) { updateTo("LED");        break; }
  //   else if(i==2) { updateTo("PED");        break; }
  //   else if(i==3) { updateTo("STANDALONE"); break; }
  //   else          { updateTo("");           break; }
  // }
  
}

void TAltroCFG::charToHex(char *buffer,int *buf) {
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

void TAltroCFG::ReadFEEparams(const char* filename)
{
  char name[80];
  int zs, off, thr, nsampl, npresampl, lgsup, altroclock, pedrun;
  
  ifstream f;
  f.open(filename,ios::in);
  
  while(!f.eof())
    f>>name>>zs>>name>>thr>>name>>off>>name>>nsampl>>name>>npresampl>>name>>lgsup>>name>>altroclock>>name>>pedrun;
  
  printf(" === Reading from %s\n ===\n", filename);

  printf("ZSenable         %d\n",zs);
  printf("ZSthreshold      %d\n",thr);
  printf("ZSoffset         %d\n",off);
  printf("ALTROnSamples    %d\n",nsampl);
  printf("ALTROnPreSamples %d\n",npresampl);
  printf("LGsuppression    %d\n",lgsup);
  printf("ALTROclock       %d\n",altroclock);
  printf("PedRefRun        %d\n",pedrun);

  vector<int> par;

  par.push_back( (zs<<15) + (off<<10) + thr ); //ALTROCFG1
  par.push_back(0xa7000);         // 400%02x%01x0B ZS on, MINSEQ=3, din-fpd
  par.push_back((off<<10) + thr); // 400%02x%01x08
  par.push_back(nsampl);          // 400%02x%01x0A
  par.push_back(npresampl);       // 400%02x%01x0C
  par.push_back(lgsup);           // 0x2

  fFeeParams.push_back(par);
}

int TAltroCFG::RandomFEECard()
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

void TAltroCFG::updateTo(const char* name)
{
  sprintf(fStatusC,"%s",name);
}
