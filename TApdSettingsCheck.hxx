#ifndef TAPDSETTINGSCHECK_HH
#define TAPDSETTINGSCHECK_HH

#include "TBaseCFG.hxx"
#include "TSequencerCommand.hxx"

class TApdSettingsCheck : public TBaseCFG {
  
public: 
  
  TApdSettingsCheck(char* name, vector<TSequencerCommand*> *sequence, int fee, int sru);
  
  void ReadSettings();
  void CalculateStatus();
  void Init();
  
private:
  
  int fFEE;
  int fSRU;
  
  vector<int>  fValue; // register values from configuration file
};

#endif
