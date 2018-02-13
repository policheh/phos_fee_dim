#ifndef TDIMCOUNTER_HH
#define TDIMCOUNTER_HH

#include "dim/dis.hxx"
using namespace std;

class TDimCounter
{

public:
  
  TDimCounter(const char *dimroot);
  ~TDimCounter();
  
  void Update();
  int getCount() { return fCount; }
  
private:
  
  DimService *fDimService;
  int fCount;
};

#endif
