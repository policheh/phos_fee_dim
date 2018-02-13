#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;


// CSP mapping on 1 FEE card
//
//    x=     0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
//      |---------------------------------------------------
// z=1  |CSP16|17|18|19|20|21|22|23| 8| 9|10|11|12|13|14|15|
// top  |--------------------------------------------------| bottom
// z=0  |CSP 0| 1| 2| 3| 4| 5| 6| 7|24|25|26|27|28|29|30|31|
//      |---------------------------------------------------
//      |   ALTRO 2    |  ALTRO 3  |  ALTRO 0  |  ALTRO 4  |
//

// CSP  0-15 -> address 0x6 (16bits number)
// CSP 16-31 -> address 0x7 (16bits number)
// 0 means unmasked, 1 means masked.


int main(int argc, char* argv[]) {

  if(argc<2){
    printf("\tUsage: %s <SRU>, for example: %s 3\n", argv[0],argv[0]);
    return -1;
  }

  int fNumber = atoi(argv[1]);
 
  int mod,part;
  std::string feechnls;
  
  if(fNumber==1) { mod=1; part=2; } // M1-2
  if(fNumber==2) { mod=1; part=3; } // M1-3
  if(fNumber==3) { mod=2; part=0; } // M2-0
  if(fNumber==4) { mod=2; part=1; } // M2-1
  if(fNumber==5) { mod=2; part=2; } // M2-2
  if(fNumber==6) { mod=2; part=3; } // M2-3
  if(fNumber==7) { mod=3; part=0; } // M3-0
  if(fNumber==8) { mod=3; part=1; } // M3-1
  if(fNumber==9) { mod=3; part=2; } // M3-2
  if(fNumber==10){ mod=3; part=3; } // M3-3
  if(fNumber==11){ mod=4; part=0; } // M4-0
  if(fNumber==12){ mod=4; part=1; } // M4-1
  if(fNumber==13){ mod=4; part=2; } // M4-2
  if(fNumber==14){ mod=4; part=3; } // M4-3

  char filename[80];
  sprintf(filename,"M%d-%d/CSPmask.dat",mod,part);
  
  fstream file(filename);
  if(!file.good()) return -1;
  
  while(std::getline(file,feechnls)) {
    
    int csp,dtc;
    std::stringstream stream(feechnls);
    
    stream >> dtc; // FEE card number

    int CSPmask6 = 0x0, CSPmask7 = 0x0;
    int mask1chan;
    
    while (stream >> csp) {  // CSP to mask
      printf("Mask  CSP %d of FEE%02d\n",csp,dtc);

      if(csp < 16) {
	mask1chan = 1 << csp;
	CSPmask6 = CSPmask6 ^ mask1chan;
      }
      else {
	mask1chan = 1 << (csp-16);
	CSPmask7 = CSPmask7 ^ mask1chan;
      }  
    }
    
    printf("\tCSPmask6: 0x%x CSPmask7: 0x%x\n",CSPmask6,CSPmask7);
  }
}
