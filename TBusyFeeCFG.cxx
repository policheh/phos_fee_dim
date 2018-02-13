#include <stdio.h>
#include "TBusyFeeCFG.hxx"
#include "dim/dis.hxx"
#include "globals.hxx"

TBusyFeeCFG::TBusyFeeCFG(char* name, int feeNum, vector<TSequencerCommand*> *sequence) :
  TBaseCFG(name,sequence),fNumber(feeNum),fSruBusy(false),fFeeBusy(false)
{ 
  addAddress(0x4,  DCS_DIM_SRU_TYPE, 40); // SRU busy status
  addAddress(0x74, DCS_DIM_SRU_TYPE, 40); // If it is 0x40000, then busy is due to FEE/TRU
  
  if(fNumber<20)
    addAddress(0x7b, DCS_DIM_SRU_TYPE, 40); // port 0-19
  else
    addAddress(0x7c, DCS_DIM_SRU_TYPE, 40); // port 20-39
}

void TBusyFeeCFG::CalculateStatus()
{
  
  fSruBusy = fReadback.at(0) & ( 1 << 30 );
  if(!fSruBusy) {  fStatus = 0; return; } // SRU not busy
  
  if(fReadback.at(1) == 0x40000) { // FEE or TRU busy
    
    int value = fReadback.at(2);    
    for(int ibit = 0; ibit<20; ibit++) {
      
      if( !((value >> ibit) & 0x1) ) {
	if(ibit == fNumber)    { fStatus = 1; return;}
	if(ibit+20 == fNumber) { fStatus = 1; return;}
      }      
    }
  }
  
  fStatus = 0;
}
