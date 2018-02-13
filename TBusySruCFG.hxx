#ifndef TBUSYSRUCFG_HH
#define TBUSYSRUCFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TBaseCFG.hxx"
#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// SRU BUSY monitoring

class TBusySruCFG : public TBaseCFG {

public:

  TBusySruCFG(char* name, vector<TSequencerCommand*> *sequence);  
  void CalculateStatus();
  
protected:
  
  bool fSruBusy;
  
};

#endif
