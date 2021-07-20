#include <stdio.h>

#include <iostream>
#include <fstream>

#include <TStopwatch.h>
#include <TStyle.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TString.h>
#include <TCanvas.h>
#include <TChain.h>
#include <TSystem.h>
#include <TGrid.h>
#include <TMath.h>

#include "AliPHOSFEEMapRun2.cxx"

using namespace std;

const Int_t Nmod=4;
const Int_t Nsru=4;
const Int_t Ndtc=40;
const Int_t Nx=64;
const Int_t Nz=56;

void CreatePedestalTable(Int_t runnumber=123456, TString file="o2_phos_Pedestals_1624049294312.root")
{
  TFile *f = new TFile(file);
  if(!f) return;

  AliPHOSFEEMapRun2 *calib = new AliPHOSFEEMapRun2();
  TFile *outfile = new TFile(Form("PedestalTable_%d.root",runnumber),"RECREATE");
  
  o2::phos::Pedestals *p = (o2::phos::Pedestals*)f->Get("ccdb_object") ;
  cout<<"Start High Gain Pedestal table creation.."<<endl;
  
  TH2F *hPEDHigh [Nmod+1];
  TH2F *hPEDLow  [Nmod+1];
  
  for (Int_t module=1; module<=Nmod; module++) {

    hPEDHigh[module] = new TH2F(Form("hPEDHighM%d",module),Form("M%d pedestals (HG)",module), 64,0,64,56,0,56);
    hPEDLow [module] = new TH2F(Form("hPEDLowM%d", module),Form("M%d pedestals (LG)",module), 64,0,64,56,0,56);

    for(Int_t isru=0;isru<Nsru;isru++){
      if(module==1 && (isru==0 || isru==1)) continue;//SRU M1-0 and M1-1 do not exist in Run3.

      for(Int_t idtc=0;idtc<Ndtc;idtc++){

	if(idtc==0 || idtc==20) continue;//for TRU.
	if((14 < idtc && idtc<20) || (34 < idtc && idtc<40)) continue;//empty dtc port

	fstream ftxt;
	Char_t name[255];
	Int_t iDTC=idtc;
  
	if (idtc==1) iDTC=15; // FEE 1 is connected to SRU port 15

	sprintf(name,"Pedestal_%d_SRUM%d-%d_DTC%d.txt",runnumber,module,isru,iDTC);
	ftxt.open(name,ios::out);
	
	for(Int_t ix=1;ix<=Nx;ix++) { // high gain
	  for(Int_t iz=1;iz<=Nz;iz++){
	    
            Int_t SRU = calib->CellToSRUID(ix-1);
            Int_t DTC = calib->CellToFEEID(iz-1);
	    
            if(isru != SRU) continue;
            if(idtc != DTC) continue;
	    
	    short absId;
	    char rld[3];

	    rld[0] = module;
	    rld[1] = ix;
	    rld[2] = iz;

	    o2::phos::Geometry::relToAbsNumbering(rld, absId);
	    
	    char relid[3];
	    o2::phos::Geometry::absToRelNumbering(absId,relid);

	    Int_t PED = round(p->getHGPedestal(absId));
	    Int_t ALTRO = calib->CellToALTRO(ix-1,iz-1);
	    Int_t CSP = calib->CellToCSPID(module,ix-1,iz-1);
	    Int_t ALTROCH_Hi = calib->CSPToALTROChannel(CSP,"High");

	    hPEDHigh[module]->SetBinContent(ix,iz,PED);
	    ftxt <<"ALTRO: " << ALTRO <<" CHANNEL: "<< ALTROCH_Hi << " PED: "<< PED << endl;

	  }// end of cell z loop
	}// end of cell x loop

	for(Int_t ix=1;ix<=Nx;ix++){// low gain
          for(Int_t iz=1;iz<=Nz;iz++){

            Int_t SRU = calib->CellToSRUID(ix-1);
            Int_t DTC = calib->CellToFEEID(iz-1);
	    
            if(isru != SRU) continue;
            if(idtc != DTC) continue;

	    short absId;
	    char rld[3];

	    rld[0] = module;
	    rld[1] = ix;
	    rld[2] = iz;

	    o2::phos::Geometry::relToAbsNumbering(rld, absId);

	    Int_t PED = round(p->getLGPedestal(absId));
            Int_t ALTRO = calib->CellToALTRO(ix-1,iz-1);
            Int_t CSP = calib->CellToCSPID(module,ix-1,iz-1);
            Int_t ALTROCH_Lo = calib->CSPToALTROChannel(CSP,"Low");

	    hPEDLow[module] ->SetBinContent(ix,iz,PED);
	    ftxt <<"ALTRO: " << ALTRO <<" CHANNEL: "<< ALTROCH_Lo << " PED: "<< PED << endl;

          }// end of cell z loop
        }// end of cell x loop

	ftxt.close();

      }//end of dtc loop
    }//end of isru loop
    
    outfile->WriteTObject(hPEDHigh[module]);
    outfile->WriteTObject(hPEDLow[module]);
    
  }//end of module loop 
}
