#ifndef TBUSYFEECFG_HH
#define TBUSYFEECFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TBaseCFG.hxx"
#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// FEE card BUSY monitoring

class TBusyFeeCFG : public TBaseCFG {

public:

  TBusyFeeCFG(char* name, int feeNum, vector<TSequencerCommand*> *sequence);  
  void CalculateStatus();
  
protected:
  
  int fNumber;
  
  bool fSruBusy;
  bool fFeeBusy;
  
};

#endif
