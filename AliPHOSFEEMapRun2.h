#ifndef AliPHOSFEEMapRun2_h
#define AliPHOSFEEMapRun2_h

//This class is developed for calibration of PHOS in Run-2 (2015-2017).
//Author : Daiki Sekihata (Hiroshima University), 12.November.2015
//daiki.sekihata@cern.ch


class AliPHOSFEEMapRun2 {
public:
    AliPHOSFEEMapRun2();//default constructor
    AliPHOSFEEMapRun2(int module,int cellx,int cellz);
    
    ~AliPHOSFEEMapRun2();//destructor
    
public:
    int CellToSRUID(int cellx);//this returns SRU ID [0-3].
    int CellToFEEID(int cellz);//this returns FEE ID [1-14] on branch 0, [21-34] on branch 1
    int CellToALTRO(int cellx,int cellz);//this returns ALTRO ID [2,3,0,4] on 1 FEE card.
    int CellToCSPID(int module,int cellx,int cellz);//this returns CSP ID [0-15] on 1 FEE card.
    int CSPToHVID(int csp);//this returns HVBIAS register [0x60-0x7f] in decimal.
    int CSPToALTROChannel(int csp,std::string gain);//this returns ALTRO Hi/Lo channel [0-15] for pedestal calculation.
    
    int CellToTRUID(int module,int cellx,int cellz);//this returns TRU ID [0-27];
    int CellToTRUChannel(int cellx,int cellz);//this returns TRU channel [0-111];
    void TRUHWToCellID(int ddl, int hwaddress, int &cellx, int &cellz);// this returns minimum cell ID in 1 fastOR = 2x2 crystals.
    
    void  GetElectronicsMap(int &sru, int &fee, int &altro, int &csp, int &hvch, int &altlg, int &althg){
        sru   = fSRUID;
        fee   = fFEEID;
        altro = fALTRO;
        csp   = fCSPID;
        hvch  = fHVID;
        altlg = fALTCH_LG;
        althg = fALTCH_HG;
    }
    
    void Print() const;
    
private:
    int fSRUID;
    int fFEEID;
    int fALTRO;
    int fCSPID;
    int fHVID;
    int fALTCH_LG;
    int fALTCH_HG;
    int fTRUID;
    int fTRUCH;
    
};
#endif
