#include <stdio.h>
#include <vector>
#include <bitset>

#include "TApdSettingsCheck.hxx"
#include "globals.hxx"

TApdSettingsCheck::TApdSettingsCheck(char* name, vector<TSequencerCommand*> *sequence, int fee, int sru) :
  TBaseCFG(name,sequence,0xe80488),fFEE(fee), fSRU(sru)
{
  ReadSettings();
}

void TApdSettingsCheck::ReadSettings()
{
  int addr,bias;
  int mod,part;
  
  if(fSRU==1) { mod=1; part=2; } // M1-2
  if(fSRU==2) { mod=1; part=3; } // M1-3
  if(fSRU==3) { mod=2; part=0; } // M2-0
  if(fSRU==4) { mod=2; part=1; } // M2-1
  if(fSRU==5) { mod=2; part=2; } // M2-2
  if(fSRU==6) { mod=2; part=3; } // M2-3
  if(fSRU==7) { mod=3; part=0; } // M3-0
  if(fSRU==8) { mod=3; part=1; } // M3-1
  if(fSRU==9) { mod=3; part=2; } // M3-2
  if(fSRU==10){ mod=3; part=3; } // M3-3
  if(fSRU==11){ mod=4; part=0; } // M4-0
  if(fSRU==12){ mod=4; part=1; } // M4-1
  if(fSRU==13){ mod=4; part=2; } // M4-2
  if(fSRU==14){ mod=4; part=3; } // M4-3
  
  char filename[80];
  FILE* fp;
  
  sprintf(filename,"APDsettings/APD_M%d-%d_FEE%.2d.dat",mod,part,fFEE);
  printf("Reading APD settings for TApdSettingsCheck of FEE%.2d from %s\n", fFEE,filename);
  
  fp = fopen(filename,"r");
  char line[1000];
  
  while(fgets(line,sizeof(line),fp)){
    
    if (*line == '#') continue; /* ignore comment line */
    sscanf(line,"%x %x",&addr,&bias);
    
    addAddress(addr,DCS_DIM_ALTRO_TYPE,fFEE);
    fValue.push_back(bias);
  }
}

void TApdSettingsCheck::CalculateStatus()
{
  
  unsigned int j;
  bitset<32> bs;
  
  for(j=0; j<fValue.size(); j++) {
    if( fValue.at(j) == fReadback.at(j)) {
      bs[j] = true;
    } 
    else {
      bs[j] = false;
    }      
  }
  
  const char* status = bs.to_string().c_str();
  updateTo(status);  
}

void TApdSettingsCheck::Init()
{

  fAddress.clear();
  fValue.clear();
  fReadback.clear();

  ReadSettings();
}
