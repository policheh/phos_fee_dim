#include <stdio.h>
#include "TBusyTruCFG.hxx"
#include "dim/dis.hxx"
#include "globals.hxx"

TBusyTruCFG::TBusyTruCFG(char* name, int truNum, vector<TSequencerCommand*> *sequence) :
  TBusyFeeCFG(name, truNum, sequence)
{ 
  addAddress(0x4,  DCS_DIM_SRU_TYPE, 40); // SRU busy status
  addAddress(0x74, DCS_DIM_SRU_TYPE, 40); // If it is 0x40000, then busy is due to FEE/TRU
  
  if(fNumber<20)
    addAddress(0x7b, DCS_DIM_TRU_TYPE, fNumber); // port 0-19
  else
    addAddress(0x7c, DCS_DIM_TRU_TYPE, fNumber); // port 20-39
}
