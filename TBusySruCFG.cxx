#include <stdio.h>
#include "TBusySruCFG.hxx"
#include "dim/dis.hxx"
#include "globals.hxx"

TBusySruCFG::TBusySruCFG(char* name, vector<TSequencerCommand*> *sequence) :
  TBaseCFG(name,sequence),fSruBusy(false)
{ 
  addAddress(0x4,  DCS_DIM_SRU_TYPE, 40); // SRU busy status  
}

void TBusySruCFG::CalculateStatus()
{
  fSruBusy = fReadback.at(0) & ( 1 << 30 );

  if(fSruBusy) fStatus = 1; // SRU busy  
  else fStatus = 0;
}
