#include <dic.hxx>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "TApplication.h"
#include "TCanvas.h"

using namespace std;

int main( int argc, char *argv[] )
{

  if( argc < 2 ){
    cout << "usage: " << argv[0] << "  <sru>" << endl;
    exit(0);
  }  

  char sphy[255],sfw[255];
  int sru = atoi(argv[1]);

  string sruname = "SRU";
  if(sru<10) sruname += "0"; sruname += argv[1];
   
  int status;

  while(1) {

    for(int i=1; i<=14; i++) {
      status=0;

      if(i<10) sprintf(sphy,"ALICE/PHOS/%s/FEE0%d/PHYADDR",sruname.c_str(),i);
      else sprintf(sphy,"ALICE/PHOS/%s/FEE%d/PHYADDR",sruname.c_str(),i);
      
      if(i<10) sprintf(sfw,"ALICE/PHOS/%s/FEE0%d/FMVER",sruname.c_str(),i);
      else sprintf(sfw,"ALICE/PHOS/%s/FEE%d/FMVER",sruname.c_str(),i);
      
      // DimCurrentInfo phy(sphy,10,-1);
      DimCurrentInfo phy(sphy,-1);
      int addr = phy.getInt();

      // DimCurrentInfo fw(sfw,10,-1);
      DimCurrentInfo fw(sfw,-1);
      int fmv = fw.getInt();
      
      if(addr==i) { cout<<" PHYADDR: "<<addr; status += 1; }
      if(fmv==0x5043) { cout<<" FIRMWARE: "<<hex<<fmv; status += 1; }

      if(status==2) cout<<". OK";
      if(status>0)  cout<<endl;

    }
    
    cout<<"-------"<<endl;
    sleep(1);
  }

} 
