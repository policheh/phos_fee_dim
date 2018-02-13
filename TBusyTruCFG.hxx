#ifndef TBUSYTRUCFG_HH
#define TBUSYTRUCFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TBusyFeeCFG.hxx"
#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// TRU card BUSY monitoring

class TBusyTruCFG : public TBusyFeeCFG {
  
public:
  
  TBusyTruCFG(char* name, int truNum, vector<TSequencerCommand*> *sequence);  
  
};

#endif
