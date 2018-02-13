#ifndef TCARDONCFG_HH
#define TCARDONCFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TBaseCFG.hxx"
#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// FEE card on/off state monitoring 
// using low- and high power masks.

class TCardOnCFG : public TBaseCFG {

public:

  TCardOnCFG(char* name, int feeNum, vector<TSequencerCommand*> *sequence);
  
  void CalculateStatus();

  int IsOn(int device);
  void Update();
  void Reset();
  
private:
  
  int fNumber;
  // int fIsOn;
  
};

#endif
