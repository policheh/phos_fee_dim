#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <dic.hxx>
#include <time.h>
#include "dim/dis.hxx"
#include <pthread.h>

using namespace std;

struct thread_data {
  int delay;  
  int thread_id;
  DimCurrentInfo* phyaddr;
};

void *FeeListen(void* threadarg)
{
  
  struct thread_data *args;
  args = (struct thread_data *)threadarg;

  // int number = args->sru_number;
  int i = args->thread_id;
  int delay = args->delay;

  DimCurrentInfo* phyaddr = args->phyaddr;
  // DimCurrentInfo* ison = args->ison;
  
  char buff[20];
  struct tm *sTm;
  
  int addr, correct_addr;
  // int feeON;

  //char svc[255];
  //sprintf(svc,"ALICE/PHOS/SRU%02d/FEE%02d/PHYADDR",number,i);
  
  //DimCurrentInfo phyaddr("tttt",-1);
  
  while(1) {
    // printf("SRU%02d, FEE%02d\n",number,i);
    
    addr  =  phyaddr->getInt();
    // feeON =  ison->getInt();
    
    if(i==15) correct_addr = 1;
    else correct_addr = i;
    
    
    if(addr != correct_addr) {
      
      time_t now = time (0);
      sTm = localtime (&now);
      
      strftime (buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
      printf("%s FEE%02d declares wrong address 0x%x\n",buff,i,addr);
      fflush(stdout);
    }
    
    usleep(delay);
  }
  
  pthread_exit(NULL);
}

int main( int argc, char *argv[] ){
  
  if( argc < 3 ){
    cout << "usage: " << argv[0] << " <sru number>" << " <delay>" << endl;
    exit(0);
  }

  int number = atoi( argv[1] );
  cout << argv[0] << number <<" is starting looking at FEE cards of SRU" << number;
    
  int delay = atoi(argv[2]);
  cout<<" with "<<delay<<" usec delay."<<endl;
  
  pthread_t threads_l[14];
  struct thread_data td_l[14];

  // pthread_t threads_h[14];
  // struct thread_data td_h[14];
  
  int rc;
  char svc[255];

  for(int i=2; i<=15; i++) {
    
    // td_l[i].sru_number = number;
    td_l[i].thread_id = i;
    td_l[i].delay = delay;

    sprintf(svc,"ALICE/PHOS/SRU%02d/FEE%02d/PHYADDR",number,i);
    td_l[i].phyaddr = new DimCurrentInfo(svc,-1);
    
    // sprintf(svc,"ALICE/PHOS/SRU%02d/FEE%02d/IS_ON",number,i);
    // td_l[i].ison = new DimCurrentInfo(svc,0);
    
    rc = pthread_create(&threads_l[i], NULL, FeeListen, (void*)&td_l[i]);
    if(rc) {
      cout<<" Error: unable to create thread "<<rc<<endl;
      exit(-1);
    }
  }
  
  for(int i=21; i<=34; i++) {}
  
  pthread_exit(NULL);
}
