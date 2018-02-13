#ifndef THVCFG_HH
#define THVCFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TBaseCFG.hxx"
#include "TSequencerCommand.hxx"
// #include "dim/dis.hxx"

using namespace std;

// HV configuration monitoring.

class THvCFG : public TBaseCFG {
  
public:
  
  THvCFG(char* name, vector<TSequencerCommand*> *sequence, int sru);
  
  void CalculateStatus();  
  void ReadPedestals();
  void Init();
  
private:

  void updateTo(const char* name);
  int  RandomFEECard();
  
private:  
  vector<vector<int> > fPedestals;
  int fNumber;
  
};

#endif
