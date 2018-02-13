#include <stdio.h>
#include <iostream>
#include <vector>
#include <fstream>

#include "dim/dis.hxx"

#include "TTru.hxx"
#include "TBusyFeeCFG.hxx"
#include "TSequencerCommand.hxx"
#include "TRegisterCalibrated.hxx"
#include "globals.hxx"

using namespace std;

TTru::TTru( const char *serverRoot, int truNum, int sru ) : TDevice( truNum, sru ){
  
  char buf[300];
  unsigned int i;
  TRegister *reg;
  
  // set device type to TRU
  fType = 1;

  // By default TRU is good for readout
  fBad = 0;
  
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "IS_READOUT_BAD" );
  fBadService = new DimService(buf,fBad);

  // Phys. address
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "PHYADDR" );
  reg = new TRegister( buf, 0x3, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );
  
  // L0 and L1 sample phase
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "PHL0" );
  reg = new TRegister( buf, 0x24, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  // L0 delay
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "DLL0" );
  reg = new TRegister( buf, 0x2f, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  //  L0-L0confirm rollback
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "RLBL0a" );
  reg = new TRegister( buf, 0x3a, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "RLBL0b" );
  reg = new TRegister( buf, 0x3b, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );
  
  // L0Threshold
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "L0Threshold" );
  reg = new TRegister( buf, 0x2e, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  // TRU system clock phase
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "ClockPhase" );
  reg = new TRegister( buf, 0x33, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_000_015" );
  reg = new TRegister( buf, 0x50, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_016_031" );
  reg = new TRegister( buf, 0x51, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_032_047" );
  reg = new TRegister( buf, 0x52, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_048_063" );
  reg = new TRegister( buf, 0x53, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );
  
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_064_079" );
  reg = new TRegister( buf, 0x54, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_080_095" );
  reg = new TRegister( buf, 0x55, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );

  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUmask_096_111" );
  reg = new TRegister( buf, 0x56, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddService(2);
  fConfig->push_back( reg );
  
  // 0 if number of L0 resets < 1000, Nresets/1000 otherwise.
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUnoisy" );
  reg = new TRegisterCalibrated( buf, 0x49, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag, 1./1000 );
  reg->AddService( 5 );
  fConfig->push_back( reg );
  
  // TRU temperature
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TEMP1" );
  reg = new TRegisterCalibrated( buf, 0x34, DCS_DIM_TRU_TYPE, fNumber, fSequence, &fCleanFlag, 503./64./1024., -273. );
  reg->AddService( 30 );
  fConfig->push_back( reg );
  
  // BUSY service
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "BUSY" );
  TBusyFeeCFG* busyCFG = new TBusyFeeCFG( buf, fNumber, fSequence ); busyCFG->SetRefresh(30);
  fConfigMonitor.push_back( busyCFG );
  
  // TRU config command
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRUCONFIG" );
  reg = new TRegister( buf, 0x30, DCS_DIM_SPECIAL_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddCommand( DCS_DIM_IMMEDIATE, 0);
  fCommand->push_back( reg );
  
  // TRU full reset command
  sprintf( buf, "%s/%s%02d/%s", serverRoot, DCS_DIM_TRU, GetPosition(), "TRU_RESET" );
  reg = new TRegister( buf, 0x31, DCS_DIM_SPECIAL_TYPE, fNumber, fSequence, &fCleanFlag );
  reg->AddCommand( DCS_DIM_IMMEDIATE, 0);
  fCommand->push_back( reg );
  
  for( i = 0; i < fConfig->size(); i++ ){
    reg = fConfig->at( i );
    (*fRegister)[reg->GetAddress()] = reg;
  }
}

TTru::~TTru(){
  delete fBadService;
}

void TTru::ResetBuildSequence( vector<TSequencerCommand*> *sequence)
{
  // Full reset
  TSequencerCommand *command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x4f, 0x10, 0 );
  sequence->push_back( command );
}

void TTru::ConfigBuildSequence( vector<TSequencerCommand*> *sequence){
  
  TSequencerCommand *command;
  
  // Mask noisy channels
  MaskNoisyChannels( sequence);
  
  // Set parameters common for all TRUs
  SetCommonParameters( sequence);
  
  // FEE address: 0x0 in branch 0 and 0x10 in branch 1
  if(fNumber>19) command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x3, 0x10, 0 );
  else command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x3, 0x0,  0 );
  sequence->push_back( command );
  
  // Clock selector
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x22, 0x111, 0 );
  sequence->push_back( command );

  // L0 dead time
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x30, 0x0, 0 );
  sequence->push_back( command );
  
  // Set parameters specific for given TRU
  SetTruParameters( sequence);
}

void TTru::SetTruParameters(vector<TSequencerCommand*> *sequence)
{
  
  int mod,part,position;
  char filename[80],tmp[255],bad[4];
  
  // if(sru==1) { mod=1; part=2; } // M1-2
  // if(sru==2) { mod=1; part=3; } // M1-3
  // if(sru==3) { mod=2; part=0; } // M2-0
  // if(sru==4) { mod=2; part=1; } // M2-1
  // if(sru==5) { mod=2; part=2; } // M2-2
  // if(sru==6) { mod=2; part=3; } // M2-3
  // if(sru==7) { mod=3; part=0; } // M3-0
  // if(sru==8) { mod=3; part=1; } // M3-1
  // if(sru==9) { mod=3; part=2; } // M3-2
  // if(sru==10){ mod=3; part=3; } // M3-3
  // if(sru==11){ mod=4; part=0; } // M4-0
  // if(sru==12){ mod=4; part=1; } // M4-1
  // if(sru==13){ mod=4; part=2; } // M4-2
  // if(sru==14){ mod=4; part=3; } // M4-3

  GetModuleAndPartition(mod,part);

  if(fNumber>19) position = 20; // "TRU20"
  else position = 0;            // "TRU00"
  
  fstream f;
  TSequencerCommand *command;
  
  sprintf(filename,"M%d-%d/TRUparam_TRU%02d.dat",mod,part,position);
  f.open(filename);
  
  int dtc,L0delay,L0rollback,ClockPhase;
  f>>tmp>>dtc>>tmp>>bad>>tmp>>hex>>L0delay>>tmp>>L0rollback>>tmp>>dec>>ClockPhase;
  
  if(!strcmp(bad,"bad"))
    fBad = 1;
  else
    fBad = 0;
  
  fBadService->updateService();
  
  // L0 delay
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x2f, L0delay, 0 );
  sequence->push_back( command );

  // L0-L0confirm rollback
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x3a, L0rollback+8, 0 );
  sequence->push_back( command );

  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x3b, L0rollback, 0 );
  sequence->push_back( command );
  
  // Set system clock phase
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x33, ClockPhase, 0 );
  sequence->push_back( command );

  // Clock reset (also (counters reset, algorithm reset, ADC resync ... 
  // as if STU clock goes off. Sleep 1 sec.
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x4f, 0x1, 1000000 );
  sequence->push_back( command );
  
  // Clear error registries
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x4f, 0x1000, 200000 );
  sequence->push_back( command );
  
  // SOR reset (SR clock domain = fake ALTRO, counters)
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x4f, 0x100, 0 );
  sequence->push_back( command );
  
}

void TTru::MaskNoisyChannels(vector<TSequencerCommand*> *sequence)
{

  int TRUmask_000_015=0x0;
  int TRUmask_016_031=0x0;
  int TRUmask_032_047=0x0;
  int TRUmask_048_063=0x0;
  int TRUmask_064_079=0x0;
  int TRUmask_080_095=0x0;
  int TRUmask_096_111=0x0;
  
  int chan, mask1chan;
  char filename[80];
  
  int mod,part;
  GetModuleAndPartition(mod,part);
  
  sprintf(filename,"M%d-%d/TRUmask_TRU%02d.dat",mod,part,GetPosition());
  printf("Reading TRU mask from file %s..\n",filename);
  fstream file(filename);

  while( file >> chan) {
    mask1chan = 1<<(chan%16);
    
    if(chan<16)
      TRUmask_000_015=TRUmask_000_015+mask1chan;
    else if(chan<32)
      TRUmask_016_031=TRUmask_016_031+mask1chan;
    else if(chan<48)
      TRUmask_032_047=TRUmask_032_047+mask1chan;
    else if(chan<64)
      TRUmask_048_063=TRUmask_048_063+mask1chan;
    else if(chan<80)
      TRUmask_064_079=TRUmask_064_079+mask1chan;
    else if(chan<96)
      TRUmask_080_095=TRUmask_080_095+mask1chan;
    else if(chan<112)
      TRUmask_096_111=TRUmask_096_111+mask1chan;
  }

  printf("TRUmask_000_015 = 0x%x\n",TRUmask_000_015);
  printf("TRUmask_016_031 = 0x%x\n",TRUmask_016_031);
  printf("TRUmask_032_047 = 0x%x\n",TRUmask_032_047);
  printf("TRUmask_048_063 = 0x%x\n",TRUmask_048_063);
  printf("TRUmask_064_079 = 0x%x\n",TRUmask_064_079);
  printf("TRUmask_080_095 = 0x%x\n",TRUmask_080_095);
  printf("TRUmask_096_111 = 0x%x\n",TRUmask_096_111);

  TSequencerCommand *command;
  int dl = 200000; // 0.2 sec delay
  
  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x50, TRUmask_000_015, dl);
  sequence->push_back( command );

  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x51, TRUmask_016_031, dl);
  sequence->push_back( command );

  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x52, TRUmask_032_047, dl);
  sequence->push_back( command );

  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x53, TRUmask_048_063, dl);
  sequence->push_back( command );

  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x54, TRUmask_064_079, dl);
  sequence->push_back( command );

  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x55, TRUmask_080_095, dl);
  sequence->push_back( command );

  command = new TSequencerCommand(DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x56, TRUmask_096_111, dl);
  sequence->push_back( command );
  
}

void TTru::SetCommonParameters(vector<TSequencerCommand*> *sequence) 
{
  // Set parameters common for all TRUs.
  
  TSequencerCommand *command;
  
  char tmp[255];
  int SamplePhase,L0Algorithm,L0Threshold,DTCReturn,DataSelector,TRUZSThreshold;
  
  ifstream f;
  f.open("common/TRUparam.dat");
  
  f>>tmp>>hex>>SamplePhase>>tmp>>hex>>L0Algorithm>>tmp>>dec>>L0Threshold>>tmp>>DTCReturn>>tmp>>hex>>DataSelector>>tmp>>hex>>TRUZSThreshold;
  
  // L0 and L1 sample phase (0x1)
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x24, SamplePhase, 0 );
  sequence->push_back( command );
  
  // L0 algorithm selector: normal 4x4 trigger (0x1)
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x2c, L0Algorithm, 0 );
  sequence->push_back( command );

  // L0 threshold for cosmic 4x4 trigger in ADC counts
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x2e, L0Threshold*52.8, 0 );
  sequence->push_back( command );

  // DTC return: 0x0001
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x38, DTCReturn, 0 );
  sequence->push_back( command );
  
  // Transmit data selector: timesum data (0x5)
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x3c, DataSelector, 0 );
  sequence->push_back( command );

  // Zero Suppression threshold: 0x00f
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x3d, TRUZSThreshold, 0 );
  sequence->push_back( command );
  
  // L0 reset enable
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x46, 1, 0 );
  sequence->push_back( command );
  
  // L0 reset limit 1000 Hz
  command = new TSequencerCommand( DCS_DIM_TRU_TYPE, fNumber, -1, -1, 0x47, 1000, 0 );
  sequence->push_back( command );

}

int TTru::GetPosition()
{
  // Return "logical DTC port" of this TRU.
  // In fact, this port is configurable in files TRUparam_TRU00.dat and TRUparam_TRU20.dat.
  
  int position;
  
  if(fNumber>19) position = 20; // "TRU20"
  else position = 0;            // "TRU00"
  
  return position;
}
