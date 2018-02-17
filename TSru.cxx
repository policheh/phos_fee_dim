#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "dim/dis.hxx"

#include "TSru.hxx"
#include "TSocket.hxx"
#include "globals.hxx"

#include "TAltroCFG.hxx"
#include "THvCFG.hxx"
#include "TPedCFG.hxx"
#include "TCardOnCFG.hxx"
#include "TBusySruCFG.hxx"
#include "TRegisterCalibrated.hxx"

using namespace std;

TSru::TSru( const char *serverRoot, int sruNum, const char *hostname, TSocket* socket ){
    
    fSocket = socket;
    
    fPedRefRun = 0;
    ReadConfiguration();
    
    fZSOffset=1;
    fZSThr=3;
    
    char buf[300];
    unsigned int i;
    string str;
    
    TFee *fee;
    TRegister *reg;
    
    vector<int> address;
    vector<string> name;
    vector<int> refresh;
    
    fNumber = sruNum;
    fDTCAligned = 1; // BVP 04.02.2015. How to use this??
    sprintf( fHostname, "%s", hostname );
    
    // create the vector object holders
    fFee = new vector<TFee*>;
    fTru = new vector<TTru*>;
    fDevice = new vector<TDevice*>;
    
    // 40 places for devices, zero initially
    for( i = 0; i < 40; i++ )
        fDevice->push_back( 0 );
    
    fConfig = new vector<TRegister *>;
    fStatus = new vector<TRegister *>;
    fCommand = new vector<TRegister *>;
    fAltro = new vector<TRegister *>;
    fRegister = new vector<TRegister *>;
    
    // prepare by default for full space of SRU register addresses
    for( i = 0; i <= 0x10ef; i++ )
        fRegister->push_back( 0 );
    
    // allocate command sequence and register arrays
    fSequence = new vector<TSequencerCommand *>;
    fCleanFlag = 0;
    
    // get sru root
    sprintf( buf, "%s/%s%02d", serverRoot, DCS_DIM_SRU, sruNum );
    
    // create the FEE card objects
    for(i=2; i<=15; i++){
        fee = new TFee( buf, i, sruNum );
        fee->SetAltro( fAltro );
        fFee->push_back( fee );
        (*fDevice)[i] = fee;
    }
    
    for(i=21; i<=34; i++){
        fee = new TFee( buf, i, sruNum );
        fee->SetAltro( fAltro );
        fFee->push_back( fee );
        (*fDevice)[i] = fee;
    }
    
    // create TRUs
    CreateTRU(buf,0);
    CreateTRU(buf,20);
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTCSWL" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x6, DCS_DIM_SPECIAL_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTCSWH" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x7, DCS_DIM_SPECIAL_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTCMASKL" );
    // dim address, reg address, type, device #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x21, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->AddService(2);
    fConfig->push_back( reg );
    fPowerL = reg;
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTCMASKH" );
    // dim address, reg address, type, device #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x22, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    reg->AddService( 2 );
    fConfig->push_back( reg );
    fPowerH = reg;
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "TTC_CLOCK" );
    // reg = new TRegister( buf, 0x20, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SRU_RESET" );
    reg = new TRegister( buf, 0x10, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fConfig->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SRU_PHASE_SCAN" );
    reg = new TRegister( buf, 0x11, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fConfig->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTC_WORD_ALIGN" );
    reg = new TRegister( buf, 0x12, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fConfig->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTC_ERR_TEST" );
    reg = new TRegister( buf, 0x45, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fConfig->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTC_READOUT_MASKL" );
    reg = new TRegister( buf, 0x23, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->AddService( 2 );
    fConfig->push_back( reg );
    fReadoutL = reg;
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTC_READOUT_MASKH" );
    reg = new TRegister( buf, 0x24, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->AddService( 2 );
    fConfig->push_back( reg );
    fReadoutH = reg;
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "TRIGMODE" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x12, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DPMSEL" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x84, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "STL1LATENCY" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x8e, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "STL2LATENCY" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x8f, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "RDOCLKSEL" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x99, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 1 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DDLMASK" );
    // // dim address, reg address, type, device #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0xa4, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // reg->AddService( 30 );
    // fConfig->push_back( reg );
    
    // status registers
    
    // SRU firmware version
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SRUFMID" );
    reg = new TRegister( buf, 0x3, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddService( 60 );
    fStatus->push_back( reg );
    
    // SRU status
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SRUST" );
    reg = new TRegister( buf, 0x4, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddService(60);
    reg->AddService(3);
    fStatus->push_back( reg );
    
    // SRU temperature
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "TEMP1" );
    TRegisterCalibrated* regc = new TRegisterCalibrated( buf, 0x7, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag, 503./64./1024., -273. );
    regc->SetBitwiseANDCalibrationFactor(0xFFFF);
    regc->AddService( 30 );
    fStatus->push_back( regc );
    
    //SRU BUSY status
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "BUSY" );
    TBusySruCFG* busyCFG = new TBusySruCFG( buf, fSequence ); busyCFG->SetRefresh(30);
    fConfigMonitor.push_back( busyCFG );
    
    // command registers
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SCLKSYNC" );
    // reg = new TRegister( buf, 0xa, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "RELOADFLASH" );
    reg = new TRegister( buf, 0x40, DCS_DIM_SRU_TYPE, 40, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    
    
    
    // commands that are virtual are asigned to device 41
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "EXIT" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x1, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "LOADFLASH" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x2, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "INITCLKRESET" );
    // // dim address, reg address, type, TRU #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x3, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTCALIGN" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x4, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "RDOTRGCFG" );
    // // dim address, reg address, type, TRU #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x5, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "ALTROCFG" );
    reg = new TRegister( buf, 0x6, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "HVLOAD" );
    reg = new TRegister( buf, 0x9, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "FEECONFIG" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x10, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SOR" );
    reg = new TRegister( buf, 0x11, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "TRUSOR" );
    // // dim address, reg address, type, TRU #, sequence, sequence cleaner
    // reg = new TRegister( buf, 0x12, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    // reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    // fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "ZS_ON" );
    reg = new TRegister( buf, 0x13, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "ZS_OFFS" );
    reg = new TRegister( buf, 0x14, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DELAYSCAN" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0x15, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "ZS_THR" );
    reg = new TRegister( buf, 0x16, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "NSMPL" );
    reg = new TRegister( buf, 0x17, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "NPRESMPL" );
    reg = new TRegister( buf, 0x18, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "PEDLOAD" );
    reg = new TRegister( buf, 0x19, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "FEEOFF" );
    reg = new TRegister( buf, 0x61, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "FEECONFIG2" );
    reg = new TRegister( buf, 0x62, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "COMMCHECK" );
    reg = new TRegister( buf, 0x20, DCS_DIM_SPECIAL_TYPE, 41, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    fCommand->push_back( reg );
    
    
    // ALTRO registers, broadcast
    sprintf( buf, "%s/%s%02d/ALTRO/%s", serverRoot, DCS_DIM_SRU, fNumber, "ZSTHR" );
    reg = new TRegister( buf, 0x8, DCS_DIM_ALTRO_TYPE, -1, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->SetBroadcastAltro();
    fAltro->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/ALTRO/%s", serverRoot, DCS_DIM_SRU, fNumber, "TRCFG" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0xa, DCS_DIM_ALTRO_TYPE, -1, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->SetBroadcastAltro();
    fAltro->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/ALTRO/%s", serverRoot, DCS_DIM_SRU, fNumber, "DPCFG" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0xb, DCS_DIM_ALTRO_TYPE, -1, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->SetBroadcastAltro();
    fAltro->push_back( reg );
    
    sprintf( buf, "%s/%s%02d/ALTRO/%s", serverRoot, DCS_DIM_SRU, fNumber, "BFNPT" );
    // dim address, reg address, type, TRU #, sequence, sequence cleaner
    reg = new TRegister( buf, 0xc, DCS_DIM_ALTRO_TYPE, -1, fSequence, &fCleanFlag );
    reg->AddCommand( DCS_DIM_IMMEDIATE, 0 );
    reg->SetBroadcastAltro();
    fAltro->push_back( reg );
    
    
    // SorDone service
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "SORDONE" );
    fSorDone = new TRegister( buf, 0xff, DCS_DIM_SRU_TYPE, fNumber, fSequence, &fCleanFlag );
    fSorDone->AddService( 0 );
    
    // scan registers to reg array
    for( i = 0; i < fConfig->size(); i++ ){
        reg = fConfig->at( i );
        (*fRegister)[reg->GetAddress()] = reg;
    }
    for( i = 0; i < fStatus->size(); i++ ){
        reg = fStatus->at( i );
        (*fRegister)[reg->GetAddress()] = reg;
    }
    
    // // prepare special registers service
    // sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "DTCLINEREGS" );
    // fDtcLineRegsService = new DimService( buf, "I:2;I:40;I:40", (void*)&fDtcLineRegs, sizeof( fDtcLineRegs ));
    
    // ALTRO configuration monitoring
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "ALTROCFG" );
    TAltroCFG* altroCFG = new TAltroCFG(buf,fSequence,fNumber); altroCFG->SetRefresh(0);
    fConfigMonitor.push_back(altroCFG);
    
    // HV configuration monitoring
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "HVCFG" );
    THvCFG* hvCFG = new THvCFG(buf,fSequence,fNumber); hvCFG->SetRefresh(0);
    fConfigMonitor.push_back(hvCFG);
    // fHvCFGService = new DimService(buf,fHvCFG);
    
    // Pedestals configuration monitoring
    sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_SRU, fNumber, "PEDESTAL_OK" );
    TPedCFG* pedCFG = new TPedCFG(buf,fSequence,fNumber,fPedRefRun); pedCFG->SetRefresh(0);
    fConfigMonitor.push_back(pedCFG);
    //fPedCFGService = new DimService(buf,fPedCFG);
    // fPedCFGService->SetPedReferenceRun(fPedRefRun);
    
    for (int i=1; i<=15; i++) {
        sprintf( buf, "%s/%s%02d/FEE%02d/ISON", serverRoot, DCS_DIM_SRU, fNumber,i);
        TCardOnCFG* cc = new TCardOnCFG(buf,i,fSequence); cc->SetRefresh(4);
        fConfigMonitor.push_back(cc);
    }
    
    for (int i=21; i<=34; i++) {
        sprintf( buf, "%s/%s%02d/FEE%02d/ISON", serverRoot, DCS_DIM_SRU, fNumber,i);
        TCardOnCFG* cc = new TCardOnCFG(buf,i,fSequence); cc->SetRefresh(4);
        fConfigMonitor.push_back(cc);
    }
    
    UpdateRegisterMasks();
}

TSru::~TSru(){
    
    unsigned int i;
    
    TSequencerCommand *seqcommand;
    TRegister *reg;
    TTru *tru;
    TFee *fee;
    
    // erase all commands in queue
    for( i = 0; i < fSequence->size(); i++ ){
        seqcommand = fSequence->at(i);
        
        delete seqcommand;
    }
    fSequence->clear();
    delete fSequence;
    
    // erase all config registers
    for( i = 0; i < fConfig->size(); i++ ){
        reg = fConfig->at(i);
        
        delete reg;
    }
    fConfig->clear();
    delete fConfig;
    
    // erase all status registers
    for( i = 0; i < fStatus->size(); i++ ){
        reg = fStatus->at(i);
        
        delete reg;
    }
    fStatus->clear();
    delete fStatus;
    
    // erase all command registers
    for( i = 0; i < fCommand->size(); i++ ){
        reg = fCommand->at(i);
        
        delete reg;
    }
    fCommand->clear();
    delete fCommand;
    
    // erase all altro registers
    for( i = 0; i < fAltro->size(); i++ ){
        reg = fAltro->at(i);
        
        delete reg;
    }
    fAltro->clear();
    delete fAltro;
    
    fRegister->clear();
    delete fRegister;
    
    // erase all TRUs
    for( i = 0; i < fTru->size(); i++ ){
        tru = fTru->at(i);
        
        delete tru;
    }
    fTru->clear();
    delete fTru;
    
    // erase all FEEs
    for( i = 0; i < fFee->size(); i++ ){
        fee = fFee->at(i);
        
        delete fee;
    }
    fFee->clear();
    delete fFee;
    
    // delete fDtcLineRegsService;
    // delete fSorDone;
    
    // delete fHvCFGService;
    //delete fPedCFGService;
    
}

// returns 1 if given device is configured on, 0 otherwise
int TSru::IsOn( int device ){
    
    int ison;
    ison = 0;
    
    int maskL = GetPowerMaskL();
    int maskH = GetPowerMaskH();
    
    if( device < 20 ){
        if( maskL & ( 1 << device ))
            ison = 1;
    }
    
    else if( maskH & ( 1 << ( device - 20 )))
        ison = 1;
    
    int isResponsive = 1;
    
    if(ison) {
        isResponsive = IsResponsive(device);
        if(!isResponsive) isResponsive = IsResponsive(device);
        if(!isResponsive) isResponsive = IsResponsive(device);
    }
    
    return (ison && isResponsive);
    // return 1; // BVP, 4.02.2015
}

int TSru::IsResponsive(int device)
{
    
    vector<uint32_t> cmds, rbs;
    int value = 0, isResponsive = 0;
    
    TDevice *dev = fDevice->at(device);
    if(!dev) return 0;
    
    if(dev->IsTRU())
        return 1; // TRU is always responsive
    
    int address = 0x20; // FEE card FW version
    address = address | 0x80000000; // read
    
    cmds.clear();
    rbs.clear();
    
    //mask with SRU bit
    if( device > 19 ){
        cmds.push_back( 1 << (device - 20 ));
        cmds.push_back( 0 );
    }
    else{
        cmds.push_back( 0 );
        cmds.push_back( 1 << device );
    }
    
    // commands
    cmds.push_back( address );
    cmds.push_back( value );
    
    // send the commands
    fSocket->Commands( &cmds );
    
    // setup the readback
    cmds.clear();
    cmds.push_back( 1 << 20 );
    cmds.push_back( 0 );
    cmds.push_back( 0x19 );
    cmds.push_back( 40 );
    
    fSocket->ReadbackBuffers( &cmds, &rbs );
    
    if(rbs.size()) {
        if(rbs[2] == 0x5043) isResponsive = 1;
        printf("Device: %d. rbs.size(): %d. rbs[2]: 0x%x. Responsive: %d\n",
               device,(unsigned int)rbs.size(),rbs[2],isResponsive);
        
        for( unsigned int i = 0; i < rbs.size(); i+=2 ){
            printf( "0x%x : 0x%x\n", rbs[i], rbs[i+1] );
        }
    }
    
    dev->SetResponseFlag(isResponsive);
    return isResponsive;
}

// gets maks of enabled FEEs
void TSru::GetFeeMasks( int *maskh, int *maskl ){
    
    unsigned int i;
    TDevice *dev;
    
    *maskh = 0;
    *maskl = 0;
    
    for( i = 0; i < 20; i++ ){
        dev = fDevice->at( i );
        if( dev == 0 )
            continue;
        
        if( IsOn( i ) && dev->IsFEE() )
            (*maskl) |= 1 << i;
    }
    for( i = 0; i < 20; i++ ){
        dev = fDevice->at( i + 20 );
        if( dev == 0 )
            continue;
        
        if( IsOn( i + 20 ) && dev->IsFEE() )
            (*maskh) |= 1 << i;
    }
}

// gets maks of enabled TRUs
void TSru::GetTruMasks( int *maskh, int *maskl ){
    
    unsigned int i;
    TDevice *dev;
    
    *maskh = 0;
    *maskl = 0;
    
    for( i = 0; i < 20; i++ ){
        dev = fDevice->at( i );
        if( dev == 0 )
            continue;
        
        if( IsOn( i ) && dev->IsTRU() )
            (*maskl) |= 1 << i;
    }
    for( i = 0; i < 20; i++ ){
        dev = fDevice->at( i + 20 );
        if( dev == 0 )
            continue;
        
        if( IsOn( i + 20 ) && dev->IsTRU() )
            (*maskh) |= 1 << i;
    }
}

// updates masks of registers which rely on device ON/OFF status
void TSru::UpdateRegisterMasks(){
    
    int feemaskh, feemaskl;
    unsigned int i;
    TRegister *reg;
    
    // get FEE masks
    GetFeeMasks( &feemaskh, &feemaskl );
    
    for( i = 0; i < fAltro->size(); i++ ){
        reg = fAltro->at( i );
        reg->SetMasks( feemaskh, feemaskl );
    }
}

// gets the sequence of commands waiting for execution for given device
// this function is here to transter easily in between DIM and main process threads
//
void TSru::GetSequence( vector<TSequencerCommand*> *sequence ){
    
    unsigned int i;
    TDevice *dev;
    
    // set flag to block additions to the local sequence
    fCleanFlag = 1;
    
    // copy local sequence to the output and clan the local
    for( i = 0; i < fSequence->size(); i++ )
        sequence->push_back( fSequence->at( i ));
    
    // clear the local sequence
    fSequence->clear();
    
    // remove blocking flag
    fCleanFlag = 0;
    // printf("fDevice->size(): %d\n",(int)fDevice->size());
    // go through all devices
    for( i = 0; i < fDevice->size(); i++ ){
        dev = fDevice->at( i );
        // ommit those not instantiated
        if( dev != 0 ){
            // printf("dev %d is not zero!",i);
            // if(dev->IsFEE()) printf(" Is's FEE.\n");
            // if(dev->IsTRU()) printf(" Is's TRU.\n");
            // if(!dev->IsTRU()&& !dev->IsFEE()) printf(" Is's neither FEE nor TRU.\n");
            
            // get sequence or clear it in case device is OFF
            if( IsOn( i )) {
                dev->GetSequence( sequence );
                // cout << "Device "<<i<<" on. "<<"New sequence size: " << sequence->size() << endl;
            }
            else
                dev->ClearSequence();
        }
    }
}

// sequence cleanup
void TSru::ClearSequence(){
    
    unsigned int i;
    TSequencerCommand *command;
    TDevice *dev;
    
    // set flag to block additions to the local sequence
    fCleanFlag = 1;
    
    // copy local sequence to the output and clan the local
    for( i = 0; i < fSequence->size(); i++ ){
        command = fSequence->at( i );
        delete command;
    }
    
    // clear the local sequence
    fSequence->clear();
    
    // remove blocking flag
    fCleanFlag = 0;
    
    // go through all devices
    for( i = 0; i < fDevice->size(); i++ ){
        dev = fDevice->at( i );
        // ommit those not instantiated
        if( dev != 0 ){
            dev->ClearSequence();
        }
    }
}

// builds sequence of commands necessary for readback of devices
// the commands are placed in device sequence
void TSru::ReadbackBuildSequence( int force ){
    
    unsigned int i;
    TRegister *reg;
    TDevice *dev;
    
    for( i = 0; i < fConfig->size(); i++ ){
        reg = (TRegister*) fConfig->at( i );\
        
        reg->Readback( force );
    }
    for( i = 0; i < fStatus->size(); i++ ){
        reg = (TRegister*) fStatus->at( i );\
        
        reg->Readback( force );
    }
    
    // go through all devices
    for( i = 0; i < fDevice->size(); i++ ){
        dev = fDevice->at( i );
        
        // ommit those not instantiated or not configured ON
        // if( dev != 0) {
        //   if(IsOn(i)) printf("Device %d is on, ",i);
        //   printf("fDTCAligned == %d\n",fDTCAligned);
        // }
        
        if( dev != 0 && IsOn( i ) && fDTCAligned == 1 )
            dev->ReadbackBuildSequence( force );
    }
    
    // HV configuration readback
    //fHvCFGService->Readback(force);
    
    // Pedestals configuration readback
    // fPedCFGService->Readback(force);
    
    // for (unsigned int i=0; i<fCardOnCFGService.size(); i++) {
    //   fCardOnCFGService.at(i)->Readback(force);
    // }
    
    for( i = 0; i< fConfigMonitor.size(); i++ ){
        TBaseCFG* cfg = (TBaseCFG*)fConfigMonitor.at(i);
        cfg->Readback( force );
    }
}

// reads back the DTC monitoring registers
void TSru::ReadbackDtcLineRegsBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    int i;
    TSequencerCommand *command;
    
    // undocumented regs 0x20 and 0x21
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x20 | 0x80000000, 0, 0 );
    sequence->push_back( command );
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x21 | 0x80000000, 0, 0 );
    sequence->push_back( command );
    
    // readback data
    for( i = 0x1028; i < 0x1050; i++ ){
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, i | 0x80000000, 0, 0 );
        sequence->push_back( command );
    }
    
    // readback errors
    for( i = 0x10A0; i < 0x10C8; i++ ){
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, i | 0x80000000, 0, 0 );
        sequence->push_back( command );
    }
}

// compose list of devices for which SRU reply buffers should be scanned
void TSru::ReadbackList( vector<int> *rbnum ){
    
    unsigned int i;
    
    // SRU always
    rbnum->push_back( 40 );
    
    // readback only the ones, that are ON
    for( i = 0; i < 40; i++ )
        if( IsOn( i ) && fDTCAligned == 1 )
            rbnum->push_back( i );
}

// processes readback buffer
void TSru::ReadbackProcess( int num, vector<uint32_t> *inbuf ){
    
    unsigned int i;
    unsigned int value, address;
    
    TRegister *reg;
    TDevice *dev;
    
    // first check if there is any output at all
    if( inbuf->size() == 0 )
        return;
    
    if( DCS_DIM_DEBUG > 3 ){
        cout << "FEE DIM readout process for hardware number " << num << endl;
    }
    
    // handle the SRU readback to the objects
    if( num == DCS_DIM_SRU_DTC_NUM  ){
        // first check if there is any output at all
        if( inbuf->size() == 0 )
            return;
        
        // go through the buffer by address data pairs
        for( i = 1; i < inbuf->size(); i += 3 ){
            // odd buffer length protection
            if( i + 1 >= inbuf->size() )
                continue;
            
            // clear the R/W bit, if set
            address = (int)(inbuf->at( i ) & 0x7fffffff);
            
            // try to find the config
            reg = 0;
            if( address < fRegister->size() )
                reg = (*fRegister)[address];
            
            // get the value
            value = (unsigned int)(inbuf->at( i+1 ));
            
            // standard reg
            if( reg != 0 ){
                // reg update
                reg->UpdateReadback( (int)value );
                if( DCS_DIM_DEBUG > 2 ){
                    cout << "FEE DIM readout registry(s): ";
                    printf( "%02d : 0x%08x", 40, address );
                    cout << " value: ";
                    printf( "0x%08x", value );
                    cout << endl;
                }
            }
            // special DTC regs
            else{
                // if( DCS_DIM_DEBUG > 2 ){
                // 	cout << "FEE DIM readout registry(p): ";
                // 	printf( "%02d : 0x%08x", 40, address );
                // 	cout << " value: ";
                // 	printf( "0x%08x", value );
                // 	cout << endl;
                // }
                
                // regs 0x20 and 0x21 undocumented regs
                if( address == 0x20 || address == 0x21 )
                    fDtcLineRegs.lock[address - 0x20] = value;
                
                // two ranges of special DTC regs
                if( address >= 0x1028 && address < 0x1050 )
                    fDtcLineRegs.data[address - 0x1028] = value;
                
                if( address >= 0x10A0 && address < 0x10C8 )
                    fDtcLineRegs.data[address - 0x10A0] = value;
                
                // update service after last reg readout
                if( address == 0x10C7 )
                    fDtcLineRegsService->updateService();
            }
        }
    }
    // check if the device with the given number exists and handle the readback
    else if( (*fDevice)[num] != 0 ){
        dev = fDevice->at( num );
        dev->ReadbackProcess( inbuf );
    }
    
    // HV configuration
    // fHvCFGService->ReadbackProcess(inbuf);
    // fHvCFGService->Update();
    
    // Pedestals configuration
    // fPedCFGService->ReadbackProcess(inbuf);
    // fPedCFGService->Update();
    
    // for (unsigned int i=0; i<fCardOnCFGService.size(); i++) {
    //   fCardOnCFGService.at(i)->ReadbackProcess(inbuf);
    //   fCardOnCFGService.at(i)->Update();
    // }
    
    for( i = 0; i< fConfigMonitor.size(); i++ )
        fConfigMonitor.at(i)->ReadbackProcess(inbuf);
    
}


// load flash command
// uses fixed port 6007 to send the load flash command
// the simple sorin's fw requires same sender and receiver ports
void TSru::LoadFlash(){
    
    int i;
    vector<uint32_t> outbuf;
    TSocket *socket;
    
    socket = new TSocket();
    
    // setup socket
    sprintf( socket->fHostname, "%s", fHostname );
    socket->fPort = 6007;
    socket->fPortLocal = 6007;
    
    // prepare the command
    outbuf.push_back( 0x80000000 );
    outbuf.push_back( 0x00000000 );
    outbuf.push_back( 0xAAAAFFFF );
    outbuf.push_back( 0x00000000 );
    outbuf.push_back( 0xFFFFFFFF );
    outbuf.push_back( 0xFFFF8000 );
    
    i = 0;
    // connect the socket
    while( socket->Connect() && i < 20 ){
        sleep( 1 );
        i++;
    }
    
    if( i < 20 ){
        // send commands
        socket->Commands( &outbuf );
        
        // close socket
        socket->Close();
    }
    
    delete socket;
    
    // let the SRU boot
    sleep( 10 );
}


// power up/down devices command build
void TSru::PowerChangeBuildSequence( vector<TSequencerCommand*> *sequence, int forceh, int forcel ){
    
    int i, powerh, powerl, seth, setl;
    TSequencerCommand *command;
    
    // find difference in between readback and set values and process
    powerl = fPowerL->GetReadback();
    powerh = fPowerH->GetReadback();
    if( forcel == -1 )
        setl = fPowerL->GetValue();
    else
        setl = forcel;
    if( forceh == -1 )
        seth = fPowerH->GetValue();
    else
        seth = forceh;
    
    // process the low power register
    for( i = 0; i < 20; i++ ){
        if(( powerl & ( 1 << i )) != ( setl & ( 1 << i ))){
            // toggle the bit
            powerl ^= 1 << i;
            
            // write command
            command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x6, powerl, 1 );
            sequence->push_back( command );
            // read command
            command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x80000006, 0, 0 );
            sequence->push_back( command );
        }
    }
    // process the high power register
    for( i = 0; i < 20; i++ ){
        if(( powerh & ( 1 << i )) != ( seth & ( 1 << i ))){
            // toggle the bit
            powerh ^= 1 << i;
            
            // write command
            command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x7, powerh, 1 );
            sequence->push_back( command );
            // read command
            command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x80000007, 0, 0 );
            sequence->push_back( command );
        }
    }
    
    // update ON masks for registers
    UpdateRegisterMasks();
    
    // sleep some time to allow to stabilize
    sleep( 7 );
}

// sets clock and resets SRU
void TSru::InitClockResetBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    TSequencerCommand *command;
    //ttc, use TTC clock when it is ready
    
    if(fNumber==14) //SRU M4-3 does not have any busy input. 0x29 is for disabling busy input.
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x20, 0x80000029, 2 );
    
    else // Other SRUs have busy input from anther SRU. 0x69 is for enabling busy input (px18).
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x20, 0x80000069, 2 );
    
    sequence->push_back( command );
    
    // send the reset command and 10s delay
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x10, 0, 10 );
    sequence->push_back( command );
    
}

// DTC alignment command
void TSru::DTCAlignBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    int mod;
    int part;
    
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
    
    char filename[255];
    TSequencerCommand *command;
    
    sprintf(filename,"M%d-%d/SRUparam.dat",mod,part);
    ifstream f;
    
    f.open(filename,ios::in);
    printf("reading file : %s\n",filename);
    
    char tmp[255];
    int DTCrefport;
    
    while(!f.eof())
        f>>tmp>>DTCrefport;
    
    int xDTCrefport = (DTCrefport<<24) + 0xD2;
    printf("+++++SRU parameters of M%d-%d in decimal+++++\n",mod,part);
    printf("DTC reference port for Phase Scan = %d and Phase Scan command = 0x%x\n",DTCrefport,xDTCrefport);
    
    // Bit 24 select the DTC reference port for phase scan
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x11, xDTCrefport, 2);
    sequence->push_back( command );
    
    //DTC word alignment
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x12, 0x0, 1);
    sequence->push_back( command );
    
    //DTC Error Test
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x45, 0x0, 1 );
    sequence->push_back( command );
    
}

// Config the readout and trigger registers
void TSru::RdoTrgConfigBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    unsigned int i;
    TSequencerCommand *command;
    TRegister *reg;
    int regs[6] = { 0xa4, 0x8, 0x9, 0x12, 0x8e, 0x8f };
    
    // re-send the config
    for( i = 0; i < 6; i++ ){
        if( (*fRegister)[regs[i]] != 0 ){
            reg = (*fRegister)[regs[i]];
            
            // send the command
            command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, regs[i], reg->GetValue(), 0 );
            sequence->push_back( command );
        }
    }
}

// Reset fee
void TSru::FeeResetBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    TSequencerCommand *command;
    
    for(int i=2; i<=14; i++) {
        command = new TSequencerCommand( 2, i, -1, -1, 0x3, i, 0 );
        sequence->push_back( command );
    }
    
    command = new TSequencerCommand( 2, 15, -1, -1, 0x3, 0x1, 0 );
    sequence->push_back( command );
    
    for(int i=21; i<=34; i++) {
        command = new TSequencerCommand( 2, i, -1, -1, 0x3, i-20+16, 0 );
        sequence->push_back( command );
    }
    
    
}

void TSru::AltroConfigBuildSequence( vector<TSequencerCommand*> *sequence, int number ){
    
    TSequencerCommand *command;
    
    int zs;
    int off, thr, nsampl, npresampl, lgsup, altroclock, pedrun;
    
    ifstream f;
    char name[80];
    
    if(number==0) {
        f.open("common/FEEparam.dat",ios::in);
    }
    else if(number==1) {
        f.open("common/FEEparam_physics.dat",ios::in);
    }
    else if(number==2) {
        f.open("common/FEEparam_led.dat",ios::in);
    }
    else if(number==3) {
        f.open("common/FEEparam_pedestal.dat",ios::in);
    }
    else if(number==4) {
        f.open("common/FEEparam_standalone.dat",ios::in);
    }
    
    while(!f.eof())
        f>>name>>zs>>name>>thr>>name>>off>>name>>nsampl>>name>>npresampl>>name>>lgsup>>name>>altroclock>>name>>pedrun;
    
    printf("ZSenable         %d\n",zs);
    printf("ZSthreshold      %d\n",thr);
    printf("ZSoffset         %d\n",off);
    printf("ALTROnSamples    %d\n",nsampl);
    printf("ALTROnPreSamples %d\n",npresampl);
    printf("LGsuppression    %d\n",lgsup);
    printf("ALTROclock       %d\n",altroclock);
    printf("PedRefRun        %d\n",pedrun);
    
    fPedRefRun = pedrun;
    
    //ALTROCFG1
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x27, (zs<<15) + (off<<10) + thr, 0);
    sequence->push_back( command );
    
    //ALTROCFG2
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x28, (npresampl<<20) + (nsampl<<10) + (altroclock<<5), 0);
    sequence->push_back( command );
    
    //time window for L1 trigger
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x29, 0x012C0078, 0);
    sequence->push_back( command );
    
    //time window for L2 trigger
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x2a, 0x4e2000C8, 0);
    sequence->push_back( command );
    
    //Syncronize ALTRO sampling clock with respect to LHC clock
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x41, 0xE4, 0);
    sequence->push_back( command );
    
    for(int i=2; i<=15; i++) {
        writeALTROregs(i,sequence,zs,off,thr,nsampl,npresampl,lgsup);
    }
    
    for(int i=21; i<=34; i++) {
        writeALTROregs(i,sequence,zs,off,thr,nsampl,npresampl,lgsup);
    }
}

void TSru::writeALTROregs(int dtc, vector<TSequencerCommand*> *sequence, bool zs, int offset, int threshold, int nsampl, int npresampl, int lgsup)
{
    
    int FEC =  (int)dtc%20;
    int branch = (int)dtc/20;
    int GTLaddr = FEC + branch*16;
    
    int altro[] = {0,2,3,4};
    int a,b;
    
    char sreg[255];
    int addr[255];
    
    TSequencerCommand *command;
    
    for(int i=0; i<4; i++) {
        a = altro[i];
        b = a*2; // ALTRO shifted up 1 bit
        
        // ALTRO_ZTHR
        int zsthr = (offset<<10) + threshold;
        sprintf(sreg,"400%02x%01x08",GTLaddr,b);
        charToHex(sreg,addr);
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], zsthr, 0 );
        sequence->push_back( command );
        
        // ALTRO_TRCFG
        sprintf(sreg,"400%02x%01x0A",GTLaddr,b);
        charToHex(sreg,addr);
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], nsampl, 0 );
        sequence->push_back( command );
        
        // ALTRO_DPCFG
        sprintf(sreg,"400%02x%01x0B",GTLaddr,b);
        charToHex(sreg,addr);
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], 0xa7000, 0 ); // ZS on, MINSEQ=3, din-fpd
        sequence->push_back( command );
        
        // ALTRO_BFNPT
        sprintf(sreg,"400%02x%01x0C",GTLaddr,b);
        charToHex(sreg,addr);
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], npresampl, 0 );
        sequence->push_back( command );
    }
    
    //LG suppression enable/disable
    command = new TSequencerCommand( 2, dtc, -1, -1, 0x2, lgsup, 0 );
    sequence->push_back( command );
}

// FEE config
void TSru::FeeConfigBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    unsigned int i;
    TTru *tru;
    
    // Switch on FEE cards, mask dead DTC links
    ReadPowerMask( sequence );
    
    // SRU config
    InitClockResetBuildSequence( sequence );
    DTCAlignBuildSequence( sequence );
    
    // ALTRO config
    AltroConfigBuildSequence( sequence, 1);
    
    // FEE reset, set phys. addresses to FEE cards
    FeeResetBuildSequence( sequence );
    
    // load pedestals
    PedestalBuildSequence( sequence );
    
    // Apply readout mask
    ReadoutMaskBuildSequence( sequence);
    
    // Apply CSP masks
    CSPmaskBuildSequence( sequence);
    
    // TRU configuration
    for( i = 0; i < fTru->size(); i++ ){
        
        tru = fTru->at( i );
        if( tru == 0 ) continue;
        
        // if( IsOn( tru->GetNumber() ))
        tru->ConfigBuildSequence( sequence );
    }
    
    // Re-read SRU monitoring configuration if config files were changed
    for( i = 0; i < fConfigMonitor.size(); i++ ){
        fConfigMonitor.at(i)->Init();
    }
    
    // Re-read monitoring configuration for devices
    for( i = 0; i < fDevice->size(); i++ ){
        if(fDevice->at(i)) fDevice->at(i)->Init();
    }
}

// Configure FEE cards only, don't touch TRUs
void TSru::FeeConfigBuildSequence2( vector<TSequencerCommand*> *sequence ){
    
    // Switch on FEE cards, mask dead DTC links
    ReadPowerMask( sequence );
    
    // SRU config
    InitClockResetBuildSequence( sequence );
    DTCAlignBuildSequence( sequence );
    
    // ALTRO config
    AltroConfigBuildSequence( sequence, 1);
    
    // FEE reset, set phys. addresses to FEE cards
    FeeResetBuildSequence( sequence );
    
    // load pedestals
    PedestalBuildSequence( sequence );
    
    // Apply readout mask
    ReadoutMaskBuildSequence( sequence);
    
    // Apply CSP masks
    CSPmaskBuildSequence( sequence);
    
    // Re-read monitoring configuration if config files were changed
    for( unsigned int i = 0; i < fConfigMonitor.size(); i++ ){
        fConfigMonitor.at(i)->Init();
    }
    
}

void TSru::HvLoadAllBuildSequence( vector<TSequencerCommand*> *sequence, int number ){
    // Apply HV values to DACs.
    
    // Read APD settings from files in the local directories
    // like APDsettings_310, where 310 is the number of APD configuration.
    // This number should be given to the HVLOAD service.
    
    // If 0 is given to HVLOAD service, or file does not exists,
    // read "default" settings from directory APDsettings.
    
    TSequencerCommand *command;
    
    int addr,bias;
    int mod,part;
    
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
    
    char prefix[80],filename[80],line[1000];
    FILE* fp;
    
    if(number) sprintf(prefix,"APDsettings_%d",number);
    else sprintf(prefix,"APDsettings");
    
    for(int i=2; i<=15; i++) {
        
        sprintf(filename,"%s/APD_M%d-%d_FEE%.2d.dat",prefix,mod,part,i);
        fp = fopen(filename,"r");
        
        if(!fp) {
            sprintf(filename,"APDsettings/APD_M%d-%d_FEE%.2d.dat",mod,part,i);
            fp = fopen(filename,"r");
        }
        
        while(fgets(line,sizeof(line),fp)){
            
            if (*line == '#') continue;
            sscanf(line,"%x %x",&addr,&bias);
            
            printf("Address: %x, bias: %x\n",addr,bias);
            command = new TSequencerCommand( 2, i, -1, -1, addr, bias, 100000 );
            sequence->push_back( command );
        }
        
        //Apply changes with 1 sec delay per card!
        command = new TSequencerCommand( 2, i, -1, -1, 0x1e, 0x0, 100000 );
        sequence->push_back( command );
    }
    
    for(int i=21; i<=34; i++) {
        
        sprintf(filename,"%s/APD_M%d-%d_FEE%.2d.dat",prefix,mod,part,i);
        fp = fopen(filename,"r");
        
        if(!fp) {
            sprintf(filename,"APDsettings/APD_M%d-%d_FEE%.2d.dat",mod,part,i);
            fp = fopen(filename,"r");
        }
        
        while(fgets(line,sizeof(line),fp)){
            
            if (*line == '#') continue;
            sscanf(line,"%x %x",&addr,&bias);
            
            printf("Address: %x, bias: %x\n",addr,bias);
            command = new TSequencerCommand( 2, i, -1, -1, addr, bias, 100000 );
            sequence->push_back( command );
        }
        
        //Apply changes with 1 sec delay per card!
        command = new TSequencerCommand( 2, i, -1, -1, 0x1e, 0x0, 100000 );
        sequence->push_back( command );
    }
    
    // sprintf(fHvCFG,"PHYS");
    // fHvCFGService->updateService();
}

void TSru::PedestalBuildSequence( vector<TSequencerCommand*> *sequence ){
    // Load pedestals
    
    int mod;
    int part;
    
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
    
    char filename[255];
    TSequencerCommand *command;
    
    int  addr[255];
    char sreg[255],tmp[255];
    
    int   altro,channel,ped;
    float pd;
    
    for(int i=2; i<=15; i++) {
        
        sprintf(filename,"Pedestal/%d/Pedestal_%d_SRUM%d-%d_DTC%d.txt",fPedRefRun,fPedRefRun,mod,part,i);
        ifstream f;
        
        f.open(filename,ios::in);
        printf("Reading from %s\n",filename);
        
        while(!f.eof()){
            
            f>>tmp>>altro>>tmp>>channel>>tmp>>pd;
            ped = (int)pd;
            printf("ALTRO: %d CHANNEL: %d PED: %d\n",altro,channel,ped);
            
            int FEC =  (int)i%20;
            int branch = (int)i/20;
            
            if(i==15) {
                FEC =  (int)1%20;
                branch = (int)1/20;
            }
            
            int GTLaddr = FEC + branch*16;
            int b = altro*2; // # ALTRO shifted up 1 bit
            int d = channel*2 + b*16; // channnel shifted up 1 bit + ALTRO-bit info
            
            sprintf(sreg,"400%02x%02x6",GTLaddr,d);
            charToHex(sreg,addr);
            
            command = new TSequencerCommand( 2, i, -1, -1, addr[0], ped, 1000 );
            sequence->push_back( command );
        }
    }
    
    
    for(int i=21; i<=34; i++) {
        
        sprintf(filename,"Pedestal/%d/Pedestal_%d_SRUM%d-%d_DTC%d.txt",fPedRefRun,fPedRefRun,mod,part,i);
        ifstream f;
        
        f.open(filename,ios::in);
        printf("Reading from %s\n",filename);
        
        while(!f.eof()){
            
            f>>tmp>>altro>>tmp>>channel>>tmp>>pd;
            ped = (int)pd;
            printf("ALTRO: %d CHANNEL: %d PED: %d\n",altro,channel,ped);
            
            int FEC =  (int)i%20;
            int branch = (int)i/20;
            int GTLaddr = FEC + branch*16;
            
            int b = altro*2; // # ALTRO shifted up 1 bit
            int d = channel*2 + b*16; // channnel shifted up 1 bit + ALTRO-bit info
            
            sprintf(sreg,"400%02x%02x6",GTLaddr,d);
            charToHex(sreg,addr);
            
            command = new TSequencerCommand( 2, i, -1, -1, addr[0], ped, 1000 );
            sequence->push_back( command );
        }
    }
    
    //sprintf(fPedCFG,"1");
    //fPedCFGService->updateService();
}

void TSru::FeeOffBuildSequence( vector<TSequencerCommand*> *sequence )
{
    TSequencerCommand *command;
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x21, 0x0, 0 );
    sequence->push_back( command );
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x22, 0x0, 0 );
    sequence->push_back( command );
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x23, 0xfffff, 0 );
    sequence->push_back( command );
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x24, 0xfffff, 0 );
    sequence->push_back( command );
    
    ResetRegisters();
}

void TSru::CSPmaskBuildSequence( vector<TSequencerCommand*> *sequence )
{
    
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
    if(!file.good()) return;
    
    while(std::getline(file,feechnls)) {
        
        int csp,dtc;
        std::stringstream stream(feechnls);
        
        stream >> dtc; // FEE card number
        
        int CSPmask6 = 0x0, CSPmask7 = 0x0;
        int mask1chan;
        
        while (stream >> csp) {  // CSP to mask
            
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
        TSequencerCommand *command;
        
        command = new TSequencerCommand( DCS_DIM_FEE_TYPE, dtc, -1, -1, 0x6, CSPmask6, 0 );
        sequence->push_back( command );
        
        command = new TSequencerCommand( DCS_DIM_FEE_TYPE, dtc, -1, -1, 0x7, CSPmask7, 0 );
        sequence->push_back( command );
    }
    
}

// TRU SOR build sequence
void TSru::TruSorBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    int trumaskh, trumaskl;
    TSequencerCommand *command;
    
    GetTruMasks( &trumaskh, &trumaskl );
    
    // broadcast to all ON TRUs
    // TRU SoR reset
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, -1, trumaskh, trumaskl, 0x4f, 0x100, 1 );
    sequence->push_back( command );
    // TRU err clear
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, -1, trumaskh, trumaskl, 0x4f, 0x1000, 0 );
    sequence->push_back( command );
}

// FEE config command
void TSru::ZeroPedestalBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    int feemaskh, feemaskl;
    int address;
    TSequencerCommand *command;
    
    GetFeeMasks( &feemaskh, &feemaskl );
    
    // setup ALTRO broadcast address for pedestals
    address = 0x40000000; // altro
    address |= 0x40000;   // bcast
    address |= 0x6;       // pedestal registry
    address |= 0x80000;   // even parity
    
    // broadcast 0 pedestal to all ALTROs
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, -1, feemaskh, feemaskl, address, 0, 0 );
    sequence->push_back( command );
}

// delay scan sequence
void TSru::DelayScanBuildSequence( vector<TSequencerCommand*> *sequence ){
    
    int i;
    TSequencerCommand *command;
    
    // power on all DTC lines
    PowerChangeBuildSequence( sequence, 0xfffff, 0xfffff );
    
    // send the reset command and 10s delay
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0xb7, 0, 10 );
    sequence->push_back( command );
    
    // clock to manual shift
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x87, 0x1, 0 );
    sequence->push_back( command );
    
    // clock step size
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x86, 0x14, 1 );
    sequence->push_back( command );
    
    // shift steps
    for( i = 0; i < 81; i++ ){
        
        // send clock shift
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x85, 0x1, 1 );
        sequence->push_back( command );
        
        // phase align
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x9f, 0xffffff, 0 );
        sequence->push_back( command );
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0xa0, 0xffffff, 1 );
        sequence->push_back( command );
        
        // FSM error testing bit
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0xa1, 0x1, 1 );
        sequence->push_back( command );
        
        // readback the DTC regs
        ReadbackDtcLineRegsBuildSequence( sequence );
    }
    
    // return power
    PowerChangeBuildSequence( sequence );
}

// SoR done flag
void TSru::SorDone(){
    
    // update service with time flag
    time_t tnow;
    
    time( &tnow );
    
    fSorDone->UpdateReadback( tnow );
    
}


void TSru::ReadPowerMask(vector<TSequencerCommand*> *sequence){
    
    int maskL; // DTC 0-19
    int maskH; // DTC 20-39
    
    // Read from files like M2-0/FEEmask.dat
    bool ok = CreatePowerMask(maskL, maskH);
    
    if(!ok) {
        
        char filename[80];
        sprintf(filename,"Power/mask_SRU%d.txt",fNumber);
        
        ifstream f;
        f.open(filename,ios::in);
        
        while(!f.eof()){
            f>>hex>>maskL>>hex>>maskH;
            printf("Reading from %s: maskL %x, maskH %x\n",filename,maskL,maskH);
        }
    }
    
    TSequencerCommand *command;
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x21, maskL, 0);
    sequence->push_back( command );
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x22, maskH, 0);
    sequence->push_back( command );
    
    // command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x23, maskL ^ 0xfffff, 0);
    // sequence->push_back( command ); // Readout mask of DTC0-19
    
    // command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x24, maskH ^ 0xfffff, 0);
    // sequence->push_back( command );  // Readout mask of DTC20-39
}

void TSru::ReadoutMaskBuildSequence(vector<TSequencerCommand*> *sequence) {
    
    int maskL; // DTC 0-19
    int maskH; // DTC 20-39
    
    // Read from files like M2-0/FEEmask.dat
    CreateReadoutMask(maskL, maskH);
    
    // Exclude bad TRU from readout
    ExcludeBadTruFromReadout(maskL, maskH);
    
    char filename[80];
    sprintf(filename,"Readout/mask_SRU%d.txt",fNumber);
    
    ifstream f;
    f.open(filename,ios::in);
    
    if(f.good()) {
        while(!f.eof())
            f>>hex>>maskL>>hex>>maskH;
    }
    
    TSequencerCommand *command;
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x23, maskL, 0);
    sequence->push_back( command ); // Readout mask of DTC0-19
    
    command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x24, maskH, 0);
    sequence->push_back( command );  // Readout mask of DTC20-39
    
}

void TSru::charToHex(char *buffer,int *buf) {
    int i;
    int num = 0, tot = 0, base = 1;
    
    for (i=7;i>=0;i--){
        buffer[i] = toupper(buffer[i]);
        if (buffer[i] < '0' || buffer[i] > '9'){
            if (buffer[i] > 'F') buffer[i] = 'F';
            num = buffer[i] - 'A';
            num += 10;
        }
        else num = buffer[i] - '0';
        
        tot += num*base;
        base *= 16;
    }
    *buf = tot;
}

void TSru::ProcessZSOn( int value, vector<TSequencerCommand*> *sequence )
{
    // value=1 => ZS on
    // value=0 => ZS off
    
    for(int i=1; i<=14; i++) {
        writeZSOn(i,sequence,value);
    }
    
    for(int i=21; i<=34; i++) {
        writeZSOn(i,sequence,value);
    }
    
}

void TSru::writeZSOn(int dtc, vector<TSequencerCommand*> *sequence, int value)
{
    int FEC =  (int)dtc%20;
    int branch = (int)dtc/20;
    int GTLaddr = FEC + branch*16;
    
    int altro[] = {0,2,3,4};
    int a,b;
    
    char sreg[255];
    int addr[255];
    
    TSequencerCommand *command;
    
    for(int i=0; i<4; i++) {
        
        a = altro[i];
        b = a*2; // ALTRO shifted up 1 bit
        
        sprintf(sreg,"400%02x%01x0B",GTLaddr,b); // ALTRO_DPCFG
        charToHex(sreg,addr);
        
        if(value) command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], 0xa4000, 0 ); // ZS on
        else   command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], 0x0, 0 );     // ZS off
        
        sequence->push_back( command );
    }
    
}

void TSru::ProcessZSOffset( int value, vector<TSequencerCommand*> *sequence )
{
    
    fZSOffset = value;
    
    for(int i=1; i<=14; i++) {
        writeZSOffset(i,sequence,value);
    }
    
    for(int i=21; i<=34; i++) {
        writeZSOffset(i,sequence,value);
    }
    
}

void TSru::writeZSOffset(int dtc, vector<TSequencerCommand*> *sequence, int value)
{
    
    int FEC =  (int)dtc%20;
    int branch = (int)dtc/20;
    int GTLaddr = FEC + branch*16;
    
    int altro[] = {0,2,3,4};
    int a,b;
    
    char sreg[255];
    int addr[255];
    
    TSequencerCommand *command;
    int zsthr = (fZSOffset<<10) + fZSThr;
    
    for(int i=0; i<4; i++) {
        
        a = altro[i];
        b = a*2;
        
        sprintf(sreg,"400%02x%01x08",GTLaddr,b); // ALTRO_ZSTHR
        charToHex(sreg,addr);
        
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], zsthr, 0 );
        sequence->push_back( command );
    }
    
}

void TSru::ProcessZSThr( int value, vector<TSequencerCommand*> *sequence )
{
    
    fZSThr = value;
    
    for(int i=1; i<=14; i++) {
        writeZSOffset(i,sequence,value);
    }
    
    for(int i=21; i<=34; i++) {
        writeZSOffset(i,sequence,value);
    }
    
}

void TSru::ProcessNSamples( int value, vector<TSequencerCommand*> *sequence )
{
    
    for(int i=1; i<=14; i++) {
        writeNSamples(i,sequence,value);
    }
    
    for(int i=21; i<=34; i++) {
        writeNSamples(i,sequence,value);
    }
    
}

void TSru::writeNSamples(int dtc, vector<TSequencerCommand*> *sequence, int value)
{
    
    int FEC =  (int)dtc%20;
    int branch = (int)dtc/20;
    int GTLaddr = FEC + branch*16;
    
    int altro[] = {0,2,3,4};
    int a,b;
    
    char sreg[255];
    int addr[255];
    
    TSequencerCommand *command;
    
    for(int i=0; i<4; i++) {
        
        a = altro[i];
        b = a*2;
        
        sprintf(sreg,"400%02x%01x0A",GTLaddr,b); // ALTRO_TRCFG
        charToHex(sreg,addr);
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], value, 0 );
        sequence->push_back( command );
    }
    
}

void TSru::ProcessNPreSamples( int value, vector<TSequencerCommand*> *sequence )
{
    
    for(int i=1; i<=14; i++) {
        writeNPreSamples(i,sequence,value);
    }
    
    for(int i=21; i<=34; i++) {
        writeNPreSamples(i,sequence,value);
    }
    
}

void TSru::writeNPreSamples(int dtc, vector<TSequencerCommand*> *sequence, int value)
{
    
    int FEC =  (int)dtc%20;
    int branch = (int)dtc/20;
    int GTLaddr = FEC + branch*16;
    
    int altro[] = {0,2,3,4};
    int a,b;
    
    char sreg[255];
    int addr[255];
    
    TSequencerCommand *command;
    
    for(int i=0; i<4; i++) {
        
        a = altro[i];
        b = a*2;
        
        sprintf(sreg,"400%02x%01x0C",GTLaddr,b);  // ALTRO_BFNPT
        charToHex(sreg,addr);
        command = new TSequencerCommand( 2, dtc, -1, -1, addr[0], value, 0 );
        sequence->push_back( command );
    }
    
}

void TSru::ResetRegisters() 
{
    unsigned int i;
    
    int readback = 0;
    int force = 1;
    
    // sprintf(fHvCFG,"");
    // fHvCFGService->updateService();
    
    //sprintf(fPedCFG,"");
    //fPedCFGService->updateService();
    
    for (i=0;  i<fConfigMonitor.size(); i++) {
        fConfigMonitor.at(i)->Reset();
    }
    
    // for (i=0;  i<fCardOnCFGService.size(); i++) {
    //   fCardOnCFGService.at(i)->Reset();
    // }
    
    for (i=0;  i<fConfig->size(); i++) {
        fConfig->at(i)->UpdateReadback(readback,force);
    }
    
    for (i=0;  i<fStatus->size(); i++) {
        fStatus->at(i)->UpdateReadback(readback,force);
    }
    
    // for (i=0;  i<fRegister->size(); i++) {
    //   fRegister->at(i)->UpdateReadback(readback,force);
    // }
    
    for (i=0;  i<fAltro->size(); i++) {
        fAltro->at(i)->UpdateReadback(readback,force);
    }
    
    for(i=0; i<fFee->size(); i++) {
        fFee->at(i)->ResetRegisters();
    }
    
    for(i=0; i<fTru->size(); i++) {
        fTru->at(i)->ResetRegisters();
    }
}

void TSru::ReadConfiguration()
{
    
    int zs, off, thr, nsampl, npresampl, lgsup, altroclock, pedrun;
    char name[80];
    
    ifstream f;
    f.open("common/FEEparam_physics.dat",ios::in);
    
    while(!f.eof())
        f>>name>>zs>>name>>thr>>name>>off>>name>>nsampl>>name>>npresampl>>name>>lgsup>>name>>altroclock>>name>>pedrun;
    
    fPedRefRun = pedrun;
}

bool TSru::CreatePowerMask(int &maskL, int &maskH)
{
    
    int a,fee,mask1fee;
    int mod,part;
    
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
    sprintf(filename,"M%d-%d/FEEmask.dat",mod,part);
    
    fstream file(filename);
    if(!file.good()) return false;
    
    int pwrmaskl = 0x0;
    int pwrmaskh = 0x0;
    
    while( file >> a) {
        
        if(a>19) fee = a-20;
        else fee = a;
        
        mask1fee = 1<<fee;
        
        if(a>19) pwrmaskh = pwrmaskh + mask1fee;
        else pwrmaskl = pwrmaskl + mask1fee;
        
    }
    
    maskL = pwrmaskl;
    maskH = pwrmaskh;
    
    return true;
}

bool TSru::CreateReadoutMask(int &maskL, int &maskH)
{
    
    int pmaskL, pmaskH;
    
    bool powerOK = CreatePowerMask(pmaskL,pmaskH);
    if(!powerOK) return false;
    
    maskL = pmaskL ^ 0xfffff;
    maskH = pmaskH ^ 0xfffff;
    
    // Suppress high and low ports 0 from readout mask (these are TRU)
    
    // maskTRU="1<<0"
    // rdomaskHi=$(($maskTRU ^ $((rdomaskHi))))
    // rdomaskLo=$(($maskTRU ^ $((rdomaskLo))))
    
    // maskL = (1<<0) ^ maskL;
    // maskH = (1<<0) ^ maskH;
    
    return true;
}

void TSru::CreateTRU(char* buf, int position)
{
    // Create TRU at the "logical" position 0 or 20.
    
    int mod,part,dtc;
    char filename[80];
    std::string tmp,bad;
    TTru *tru;
    
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
    
    fstream f;
    
    sprintf(filename,"M%d-%d/TRUparam_TRU%02d.dat",mod,part,position);
    printf("Reading TRU ports configuration from file %s\n",filename);
    f.open(filename);
    
    f>>tmp>>dtc>>tmp>>bad;
    printf("dtc: %d, TRUreadout %s\n",dtc,bad.c_str());
    
    tru = new TTru( buf, dtc, fNumber );
    if(!strcmp(bad.c_str(),"bad")) tru->SetBad();
    
    (*fDevice)[dtc] = tru;
    fTru->push_back( tru );
    
    char buf2[300];
    sprintf(buf2,"%s/%s%02d/%s", buf, DCS_DIM_TRU, tru->GetPosition(), "ISON" );
    
    TCardOnCFG* cc = new TCardOnCFG(buf2,dtc,fSequence);
    fConfigMonitor.push_back(cc);
    // fCardOnCFGService.push_back(cc);
}

void TSru::UpdateTruReadoutStatus(TTru *tru)
{
    
    int mod,part,dtc;
    char filename[80];
    
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
    
    fstream f;
    
    sprintf(filename,"M%d-%d/TRUparam_TRU%02d.dat",mod,part,tru->GetPosition());
    printf("Reading TRU ports configuration from file %s\n",filename);
    f.open(filename);
    
    std::string tmp,bad;
    f>>tmp>>dtc>>tmp>>bad;
    
    printf("dtc: %d, TRUreadout %s\n",dtc,bad.c_str());
    if(!strcmp(bad.c_str(),"bad")) tru->SetBad();
    else tru->SetGood();
    
}

void TSru::ExcludeBadTruFromReadout(int &maskL, int &maskH)
{
    printf("maskL=0x%x, maskH=0x%x before\n",maskL,maskH);
    for( unsigned int i = 0; i < fTru->size(); i++ ){
        
        TTru* tru = fTru->at( i );
        
        if( tru == 0 )    continue;
        UpdateTruReadoutStatus( tru);
        
        if(tru->GetPosition() == 0) {
            if(tru->IsBad())
                maskL |= 1<<(tru->GetNumber()); // set 1 to DTC port tru->GetNumber()
            else
                maskL &= ~(1<<(tru->GetNumber()));  // set 0 to DTC port tru->GetNumber()
            
            printf("tru->GetNumber()=%d, maskL=0x%x after\n",tru->GetNumber(),maskL);
        }
        
        if(tru->GetPosition() == 20) {
            if(tru->IsBad())
                maskH |= 1<<(tru->GetNumber()-20) ;
            else
                maskH &= ~(1<<(tru->GetNumber()-20));
            
            printf("tru->GetNumber()=%d, maskH=0x%x after\n",tru->GetNumber(),maskH);
        }
    }
}

void TSru::UpdateReadoutMask(int tru_position)
{
    // TRU "position": 0 or 20
    
    int maskL = GetReadoutMaskL();
    int maskH = GetReadoutMaskH();
    
    ExcludeBadTruFromReadout(maskL,maskH);
    TSequencerCommand *command;
    
    if(tru_position == 0) // Readout mask of DTC0-19
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x23, maskL, 0);
    
    if(tru_position == 20) // Readout mask of DTC20-39
        command = new TSequencerCommand( DCS_DIM_INTERPRETED_TYPE, 40, -1, -1, 0x24, maskH, 0);
    
    fSequence->push_back( command );
    
}
