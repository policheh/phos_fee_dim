#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "TH2I.h"
#include "TFile.h"

#include <dic.hxx>

#include "AliPHOSFEEMapRun2.h"
using namespace std;

void streamCells(int module, int sruID, int feeCard, string option, TH2I* badmap)
{
    // option could be "good" for good cells or "bad".
    
    AliPHOSFEEMapRun2 map;
    
    for(int cellx=0; cellx<64; cellx++) { // cellx should be [0,63]
        for(int cellz=0; cellz<56; cellz++) { // cellz should be [0,55]
            if (map.CellToFEEID(cellz) == feeCard && map.CellToSRUID(cellx) == sruID) {
                if(option=="good") {
                    printf("Cell [%d,%d] of FEE%.2d SM%d is good.\n",cellx,cellz,feeCard,module);
                    badmap->SetBinContent(cellx+1,cellz+1,0); // mark cell as good
                }
                else {
                    printf("Cell [%d,%d] of FEE%.2d SM%d is bad.\n",cellx,cellz,feeCard,module);
                    badmap->SetBinContent(cellx+1,cellz+1,1); // mark cell as bad
                }
            }
        }
    }
}

void streamBadCSPCell(int module, int sruID, int feeCard, int badCSP, TH2I* badmap)
{
    AliPHOSFEEMapRun2 map;
    
    for(int cellx=0; cellx<64; cellx++) { // cellx should be [0,63]
        for(int cellz=0; cellz<56; cellz++) { // cellz should be [0,55]
            if (map.CellToFEEID(cellz) == feeCard && map.CellToCSPID(module,cellx,cellz) == badCSP && map.CellToSRUID(cellx) == sruID) {
                printf("Cell [%d,%d] of FEE%.2d CSP%d SM%d is bad.\n",cellx,cellz,feeCard,badCSP,module);
                badmap->SetBinContent(cellx+1,cellz+1,1); // mark cell as bad
            }
        }
    }
}

void buildBadMaps()
{
    int mod,sruID,feeCard,badCSP;
    char filename[255],tmp[80];
    
    TH2I * PHOSBadMaps[5] = {};
    char hname[80],htitl[80];
    
    for (int i=1; i<5; i++) {
        sprintf(hname,"PHOS_BadMap_mod%d",i);
        sprintf(htitl,"Bad Modules map for mod%d",i);
        PHOSBadMaps[i] = new TH2I(hname,htitl, 64,0,64, 56,0,56);
        
        for (int iBinX=1; iBinX<=PHOSBadMaps[i]->GetXaxis()->GetNbins(); iBinX++) {
            for (int iBinY=1; iBinY<=PHOSBadMaps[i]->GetYaxis()->GetNbins(); iBinY++) {
                PHOSBadMaps[i]->SetBinContent(iBinX,iBinY,1); // init all cells as bad!
            }
        }
    }
    
    for (int iSRU=1; iSRU<=14; iSRU++) {
        
        if(iSRU==1) { mod=1; sruID=2; } // M1-2
        if(iSRU==2) { mod=1; sruID=3; } // M1-3
        
        if(iSRU==3) { mod=2; sruID=0; } // M2-0
        if(iSRU==4) { mod=2; sruID=1; } // M2-1
        if(iSRU==5) { mod=2; sruID=2; } // M2-2
        if(iSRU==6) { mod=2; sruID=3; } // M2-3
        
        if(iSRU==7) { mod=3; sruID=0; } // M3-0
        if(iSRU==8) { mod=3; sruID=1; } // M3-1
        if(iSRU==9) { mod=3; sruID=2; } // M3-2
        if(iSRU==10){ mod=3; sruID=3; } // M3-3
        
        if(iSRU==11){ mod=4; sruID=0; } // M4-0
        if(iSRU==12){ mod=4; sruID=1; } // M4-1
        if(iSRU==13){ mod=4; sruID=2; } // M4-2
        if(iSRU==14){ mod=4; sruID=3; } // M4-3
        
        sprintf(filename,"M%d-%d/FEEmask.dat",mod,sruID);
        fstream file1(filename);
        
        while( file1 >> feeCard) {
            if (feeCard==15) feeCard = 1;
            streamCells(mod, sruID, feeCard, "good", PHOSBadMaps[mod]);
        }
        
        sprintf(filename,"M%d-%d/CSPmask.dat",mod,sruID);
        fstream file2(filename);
        
        std::string feechnls;
        
        while(std::getline(file2,feechnls)) {
            std::stringstream stream(feechnls);
            stream >> feeCard;
            while (stream >> badCSP) {
                streamBadCSPCell(mod, sruID, feeCard, badCSP, PHOSBadMaps[mod]);
            }
        }
        
        sprintf(filename,"M%d-%d/TRUparam_TRU00.dat",mod,sruID);
        fstream file3(filename);
        file3 >> tmp >> feeCard;
        streamCells(mod, sruID, feeCard, "bad", PHOSBadMaps[mod]);
        
        sprintf(filename,"M%d-%d/TRUparam_TRU20.dat",mod,sruID);
        fstream file4(filename);
        file4 >> tmp >> feeCard;
        streamCells(mod, sruID, feeCard, "bad", PHOSBadMaps[mod]);
        
    }
    
    TFile fout("/tmp/BadMap.root","recreate");
    
    for(Int_t mod=1; mod<5; mod++){
        if(PHOSBadMaps[mod]) PHOSBadMaps[mod]->Write();
    }
}

int main(int argc, char* argv[]) {
    
    if(argc>1) { buildBadMaps(); return 0; }
    else {
        
        cout << "  ==> Running in infinite loop.." << endl;
        int oldrun = -1;
        
        while(1) {
            DimCurrentInfo lastRun("/LOGBOOK/SUBSCRIBE/DAQ_EOR_PHS",-1);
            int newrun =  lastRun.getInt();
            
            if(newrun != oldrun) {

                printf("New run: %d\n",newrun);
                buildBadMaps();

                //put file /tmp/BadMap.root on the DCS FXS here
                oldrun = newrun;
            }
            
            sleep(1);
        }
    }
}

