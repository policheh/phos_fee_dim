#include <dic.hxx>
#include <stdio.h>
#include <iostream>
using namespace std;

int main()
{
  
  while(1) {
    DimCurrentInfo fw("ALICE/PHOS/SRU02/FEE06/PHYADDR",10,-1);
    int addr = fw.getInt();
    cout<<" PHYADDR: "<<addr<<endl;
    sleep(1);
  }

  // //subscribe to service, print "not available" is no such service found
  // DimInfo sv("TEST/CLIENT_LIST",(char*)"not available");
  
  // while(1) {
  //   printf("List of all clients: %s\n",sv.getString());
  //   sleep(5);
  // }
  
} 
