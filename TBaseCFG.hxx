#ifndef TBASECFG_HH
#define TBASECFG_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TSequencerCommand.hxx"
#include "dim/dis.hxx"

using namespace std;

// Base class for any configuration monitoring,
// like FEE/TRU card on/off, ALTRO/APD settings correctness etc..

class TBaseCFG : public DimService {

public:
  
  TBaseCFG(char* name, vector<TSequencerCommand*> *sequence);
  TBaseCFG(char* name, vector<TSequencerCommand*> *sequence, int dimType);

  void SetRefresh(int refresh) { fRefresh = refresh; fRefreshCntr = 0; }

  void Readback   ( int force );
  void ReadbackProcess( vector<uint32_t> *inbuf );

  virtual void Init();
  virtual void Reset();


protected:
  
  void addAddress ( int address, int type, int dtc );
  void updateTo(const char* name);
  
  virtual void CalculateStatus();
  
private:

  int FindAddress( int address );
  
protected:
  
  vector<int> fAddress;   // register addresses
  vector<int> fReadback;  // readbacks from registers

  int  fStatus;      // int   status seen from DimService
  char fStatusC[80]; // char* status seen from DimService

  int fRefresh;     // readback frequency - every Nth cycle
  int fRefreshCntr; // readback counter

  vector<int> fType; // DCS_DIM_SRU_TYPE, DCS_DIM_TRU_TYPE..
  vector<int> fDtc;  // DTC port number
  
  vector<TSequencerCommand*> *pSequence;  
};

#endif
