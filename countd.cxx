#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <dic.hxx>
#include <time.h>
#include "dim/dis.hxx"

using namespace std;

int main( int argc, char *argv[] ){

  if( argc < 3 ){
    cout << "usage: " << argv[0] << " <sru number>" << " <delay>" << endl;
    exit(0);
  }

  int number = atoi( argv[1] );
  cout << argv[0] << number <<" is starting looking at SRU" << number << endl;

  char cmd[255],srv[30];

  sprintf(cmd,"ALICE/PHOS/COUNTD%02d/WinCC_SET",number);
  DimCommand runCmnd(cmd,"I");

  int wCount = 0;
  sprintf(cmd,"ALICE/PHOS/COUNTD%02d/WinCC",number);
  DimService runSrvc(cmd,wCount);

  sprintf(srv,"COUNTD%02d",number);
  DimServer::start(srv);
  
  char buff[20];
  struct tm *sTm;

  char svc[255];
  sprintf(svc,"ALICE/PHOS/SRU%02d/COUNT",number);
  
  int delay = atoi(argv[2]);
  int firstCount, lastCount, firstWinCC, lastWinCC;

  while(1) {
    
    while(runCmnd.getNext()) {
      wCount = runCmnd.getInt();
      runSrvc.updateService();
    }
    
    DimCurrentInfo count(svc,-1);
    firstCount = count.getInt();

    DimCurrentInfo winCount(cmd,-1);
    firstWinCC = winCount.getInt();

    time_t now = time (0);
    sTm = localtime (&now);

    strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);

    if(firstCount == lastCount) { 
      printf ("%s %s has the same value %d after %d sec delay\n", buff, svc, firstCount, delay); 
      fflush(stdout); 
    }
    
    if(firstWinCC && firstWinCC == lastWinCC) {
      printf ("%s %s has the same value %d after %d sec delay\n", buff, cmd, firstWinCC, delay);
      fflush(stdout);
    }

    lastCount = firstCount;
    lastWinCC = firstWinCC;
    
    sleep(delay);
  }
  
}
