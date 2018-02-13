#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>

#include "TBaseCFG.hxx"
#include "dim/dis.hxx"

#include "globals.hxx"

TBaseCFG::TBaseCFG(char* name, vector<TSequencerCommand*> *sequence) :
  DimService(name,fStatus),fStatus(0),fRefresh(1),fRefreshCntr(0)
{
  pSequence = sequence;
}

TBaseCFG::TBaseCFG(char* name, vector<TSequencerCommand*> *sequence, int dimType) :
  DimService(name,fStatusC),fStatus(0),fRefresh(1),fRefreshCntr(0)
{
  pSequence = sequence;
}

void TBaseCFG::Readback( int force )
{
  
  unsigned int i;
  int address, type, dtc;
  
  TSequencerCommand *command;
  
  if(fRefresh) {
    
    if(fRefreshCntr+1 >= fRefresh || force) {
      
      if(!force)
	fRefreshCntr = 0;
      
      // read back the whole array of addresses
      for( i = 0; i < fAddress.size(); i++ ){
	
	// set the read bit
	address = fAddress.at( i ) | 0x80000000;
	
	type = fType.at(i);
	dtc = fDtc.at(i);
	
	// add to sequence.
	command = new TSequencerCommand( type, dtc, -1, -1, address, 0, 0 );
	pSequence->push_back( command );  	
      }
    }
    
    else {
      fRefreshCntr += 1;
    }
    
  }
}

void TBaseCFG::ReadbackProcess( vector<uint32_t> *inbuf )
{
  unsigned int i;
  int index, value, address;
  
  // go through the buffer by address data pairs
  for( i = 1; i < inbuf->size(); i += 3 ){
    
    // odd buffer length protection
    if( i + 1 >= inbuf->size() )
      continue;
    
    // clear the R/W bit, if set 
    address = (int)(inbuf->at( i ) & 0x7fffffff);    
    index = FindAddress( address );

    if( index == -1 )
      continue;
    
    // get the value and compare to current
    // update if differs
    
    value = (int)(inbuf->at( i+1 ));

    if( value != fReadback.at( index )){
      fReadback.at( index ) = value;      
    }    
  }
  
  //When all values for all addresses are read out,
  //we can update the status, IF CHANGED
  
  int    oldStatusI = fStatus;
  string oldStatusC = fStatusC;
  
  CalculateStatus();
  
  int    newStatusI = fStatus;
  string newStatusC = fStatusC;
  
  if(newStatusI != oldStatusI)
    updateService();
  
  if(newStatusC.compare(oldStatusC))
    updateService();
  
}

int TBaseCFG::FindAddress( int address )
{
  // returns address index in vector, 
  // or -1.
  
  int result;
  vector<int>::iterator it;
  
  result = -1;
  
  // search for address
  it = find( fAddress.begin(), fAddress.end(), address );
  
  // found
  if( it != fAddress.end() )
    result = it - fAddress.begin();
  
  return result;
}

void TBaseCFG::CalculateStatus()
{
  // implement logic here
}

void TBaseCFG::addAddress(int address, int type, int dtc)
{
  fAddress.push_back(address);
  fReadback.push_back(-1);

  fType.push_back(type);
  fDtc.push_back(dtc);
}

void TBaseCFG::Init()
{}

void TBaseCFG::updateTo(const char* name)
{
  sprintf(fStatusC,"%s",name);
}

void TBaseCFG::Reset()
{
  fStatus = 0;
  updateTo("");
  
  updateService();
}
