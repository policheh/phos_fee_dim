#ifndef TALTROCFG_HH
#define TALTROCFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>
#include <string>

#include "TSequencerCommand.hxx"
#include "TBaseCFG.hxx"
#include "dim/dis.hxx"

using namespace std;

// ALTRO configuration monitor service.

// Collect information about ZS parameters (on/off, offset, threshold),
// num. of sampes/presamples, LG on/off.. etc, 
// and update the current configuration name, 
// which can be: PHYS, PED, LED, STANDALONE.

class TAltroCFG : public TBaseCFG {

public:

  TAltroCFG(char * name, vector<TSequencerCommand*> *sequence, int sru);
  
  void Init();
  void ReadFEEparams(const char* filename = "common/FEEparam_physics.dat");
  void CalculateStatus();
  
private:
  
  void updateTo(const char* name = "PHYS");
  void charToHex(char *buffer,int *buf);
  int RandomFEECard();
  void PrepareAddresses(int dtc);
  
private:
  
  vector<vector<int> > fFeeParams;
  int fNumber; // parent SRU number
  
};

#endif
