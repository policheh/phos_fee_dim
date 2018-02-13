#ifndef TTRU_HH
#define TTRU_HH

#include "TDevice.hxx"
#include "TSequencerCommand.hxx"

using namespace std;

class TTru : public TDevice {
public:
  
  TTru( const char *serverRoot, int truNum, int sru = -1 );
  ~TTru();
  
  void ConfigBuildSequence ( vector<TSequencerCommand*> *sequence);
  void ResetBuildSequence( vector<TSequencerCommand*> *sequence);

  void SetCommonParameters( vector<TSequencerCommand*> *sequence);
  void MaskNoisyChannels  ( vector<TSequencerCommand*> *sequence);  
  void SetTruParameters   ( vector<TSequencerCommand*> *sequence);

  void SetBad()  { fBad = 1; }
  void SetGood() { fBad = 0; }

  int  GetPosition(); // Return "logical DTC port" of this TRU: 0 or 20.
  int  IsBad() { return fBad; }
  
private:

  int fBad;
  DimService *fBadService;

};

#endif
