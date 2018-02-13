#ifndef TPEDCFG_HH
#define TPEDCFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TSequencerCommand.hxx"
#include "TBaseCFG.hxx"
#include "dim/dis.hxx"

using namespace std;

// Pedestal configuration monitoring.

class TPedCFG : public TBaseCFG {

public:
  
  TPedCFG(char* name, vector<TSequencerCommand*> *sequence, int sru, int pedrun);
  
  void SetPedReferenceRun(int pedrun) { fPedRefRun = pedrun; }
  
  void Init();
  void ReadPedestals();
  void CalculateStatus();  
  
private:
  
  void charToHex(char *buffer,int *buf);
  int RandomFEECard();
  
private:

  int fPedRefRun;
  int fNumber; // parent SRU number
  
  vector<vector<int> > fPedestals;
  
};

#endif
