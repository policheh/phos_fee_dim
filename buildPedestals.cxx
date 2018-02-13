#include <dic.hxx>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>

#include "CreatePedestalTable.C"
using namespace std;

//void calcPeds(char* DA_file_name)
//{}

int main(int argc, char* argv[]) {

  int oldrun = -1;
  char cmd[255], fnam[255];

  while(1) {
    DimCurrentInfo lastRun("/LOGBOOK/SUBSCRIBE/DAQ_EOR_PHS",-1);
    int newrun =  lastRun.getInt();

    if(newrun != oldrun) {
      printf("New run: %d\n",newrun);

      sprintf(cmd,"daq-fxs-get PHS %d \\* PED",newrun);
      system(cmd);
     
      FILE* output = popen("ls run*PHS*PED","r");
      fscanf(output,"%s",fnam);
      	
      printf("File from DA: %s\n",fnam);
      CreatePedestalTable(newrun,fnam);
	
      sprintf(cmd,"rm %s",fnam);
      system(cmd);
      
      fclose(output);
      oldrun = newrun;
    }
    
    sleep(1);
  }
  
}
