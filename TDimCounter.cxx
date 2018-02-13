#include <stdio.h>

#include "dim/dis.hxx"
#include "TDimCounter.hxx"

TDimCounter::TDimCounter(const char *dimroot) : 
  fDimService(0),fCount(0)
{
  char dimaddress[200];
  sprintf(dimaddress,"%s/COUNT",dimroot);

  fDimService = new DimService(dimaddress, fCount );
}

TDimCounter::~TDimCounter()
{
  delete fDimService;
}

void TDimCounter::Update() {
  
  fCount++;
  
  if(fCount>999999) 
    fCount = 0;
  
  fDimService->updateService();
  
}

