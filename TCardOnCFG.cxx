#include <stdio.h>
#include "TCardOnCFG.hxx"
#include "dim/dis.hxx"
#include "globals.hxx"

TCardOnCFG::TCardOnCFG(char* name, int feeNum, vector<TSequencerCommand*> *sequence) :
  TBaseCFG(name,sequence),fNumber(feeNum)
{ 
  addAddress(0x21,DCS_DIM_SRU_TYPE,40); // MaskL
  addAddress(0x22,DCS_DIM_SRU_TYPE,40); // MaskH
}

int TCardOnCFG::IsOn( int device ){
  
  int ison = 0;
  
  if( device < 20 ){
    if( fReadback.at(0) & ( 1 << device ))
      ison = 1;
  }
  else if( fReadback.at(1) & ( 1 << ( device - 20 )))
    ison = 1;
  
  return ison;

}

void TCardOnCFG:: Update()
{}

void TCardOnCFG::Reset()
{
  fStatus = 0;
  updateService();
}

void TCardOnCFG::CalculateStatus()
{
  fStatus = IsOn(fNumber);
}
