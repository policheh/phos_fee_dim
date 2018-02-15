#ifndef TSRU_HH
#define TSRU_HH

#include <iostream>

#include "TDevice.hxx"
#include "TFee.hxx"
#include "TTru.hxx"
#include "TSequencerCommand.hxx"
#include "THvCFG.hxx"
#include "TPedCFG.hxx"
#include "TCardOnCFG.hxx"
#include "TBaseCFG.hxx"
#include "TSocket.hxx"

using namespace std;

typedef struct {
	int lock[2];
	int data[40];
	int errs[40];
} SDtcLineRegs;

class TSru {
	protected:

		vector<TSequencerCommand*> *fSequence;
		int fCleanFlag;

		// registry arrays
		vector<TRegister *> *fConfig;
		vector<TRegister *> *fStatus;
		vector<TRegister *> *fCommand;
		vector<TRegister *> *fAltro;
		vector<TRegister *> *fRegister;
		
		TRegister *fPowerL;
		TRegister *fPowerH;
  
                TRegister *fReadoutL;
		TRegister *fReadoutH;

		TRegister *fSorDone;
		
		int fNumber;  // SRU number
		int fDTCAligned; // if DTC allignment happened
		char fHostname[300]; // IP or host name
		
		vector<TTru *> *fTru;
		vector<TFee *> *fFee;
		vector<TDevice *> *fDevice;
		
		// special registers
		SDtcLineRegs fDtcLineRegs;
		DimService *fDtcLineRegsService;

                // ZS offset and threshold
                int fZSOffset,fZSThr; 
  
                // HV configuration check
                // char fHvCFG[80];
                // DimService *fHvCFGService;

                // Pedestal configuration check
                // char fPedCFG[80];             
                // DimService *fPedCFGService;
  
                // vector<TCardOnCFG*> fCardOnCFGService;  
                int fPedRefRun;

                // Socket
                TSocket* fSocket;

                // monitoring services
                vector<TBaseCFG *> fConfigMonitor;

	public:
                TSru( const char *serverRoot, int sruNum, const char *hostname, TSocket* socket);
		~TSru();
		
                int  GetNumber() { return fNumber; }
		void SetDTCAligned( int aligned = 1 ){ fDTCAligned = aligned; }
		
		TDevice* GetDevice( int number ){ return fDevice->at( number ); }

                int GetReadoutMaskL() { return fReadoutL->GetReadback(); }
                int GetReadoutMaskH() { return fReadoutH->GetReadback(); }

                int GetPowerMaskL() { return fPowerL->GetReadback(); }
                int GetPowerMaskH() { return fPowerH->GetReadback(); }
		
		int IsOn( int device );
                int IsResponsive( int device );

		void GetFeeMasks( int *maskh, int *maskl );
		void GetTruMasks( int *maskh, int *maskl );
		void UpdateRegisterMasks();
                void UpdateReadoutMask(int tru_position);
                void UpdateTruReadoutStatus(TTru *tru);
                void ExcludeBadTruFromReadout(int &maskL, int &maskH);
  
		void GetSequence( vector<TSequencerCommand*> *sequence );
		void ClearSequence();

		void ReadbackBuildSequence( int force = 0 );
		void ReadbackDtcLineRegsBuildSequence( vector<TSequencerCommand*> *sequence );
		void ReadbackList( vector<int> *rbnum ); // returns list of enabled HW
		void ReadbackProcess( int num, vector<uint32_t> *inbuf ); // processes the readback
		
		void LoadFlash();
                void ReadPowerMask(vector<TSequencerCommand*> *sequence);
                bool CreatePowerMask(int &maskL, int &maskH);
		void PowerChangeBuildSequence( vector<TSequencerCommand*> *sequence, int forceh = -1, int forcel = -1 );
		void InitClockResetBuildSequence( vector<TSequencerCommand*> *sequence );
		void DTCAlignBuildSequence( vector<TSequencerCommand*> *sequence );
		void RdoTrgConfigBuildSequence( vector<TSequencerCommand*> *sequence );
		void FeeResetBuildSequence( vector<TSequencerCommand*> *sequence );
		void FeeOffBuildSequence( vector<TSequencerCommand*> *sequence );
                void AltroConfigBuildSequence( vector<TSequencerCommand*> *sequence, int number );
		void FeeConfigBuildSequence( vector<TSequencerCommand*> *sequence );
		void FeeConfigBuildSequence2( vector<TSequencerCommand*> *sequence );
                void HvLoadAllBuildSequence( vector<TSequencerCommand*> *sequence, int number );
                void PedestalBuildSequence( vector<TSequencerCommand*> *sequence );
                void ReadoutMaskBuildSequence(vector<TSequencerCommand*> *sequence);
                void CSPmaskBuildSequence( vector<TSequencerCommand*> *sequence );
                bool CreateReadoutMask(int &maskL, int &maskH);

                void ProcessZSOn( int value, vector<TSequencerCommand*> *sequence );
                void ProcessZSOffset( int value, vector<TSequencerCommand*> *sequence );
                void ProcessZSThr( int value, vector<TSequencerCommand*> *sequence );
                void ProcessNSamples( int value, vector<TSequencerCommand*> *sequence );
                void ProcessNPreSamples( int value, vector<TSequencerCommand*> *sequence );

		void TruSorBuildSequence( vector<TSequencerCommand*> *sequence );
		
		void ZeroPedestalBuildSequence( vector<TSequencerCommand*> *sequence );
		
		void DelayScanBuildSequence( vector<TSequencerCommand*> *sequence );
		
		void GetCommands( vector<TSequencerCommand*> *sequence );
		void CleanCommands();
		
                void ReadConfiguration();
                void ResetRegisters();

		void SorDone();
  
private:
  void charToHex(char *buffer,int *buf);
  void writeALTROregs(int dtc, vector<TSequencerCommand*> *sequence, bool zs=true, int offset=1, int threshold=3, int nsampl=32, int npresampl=5, int lgsup=1);
  void writeZSOn(int dtc, vector<TSequencerCommand*> *sequence, int value);
  void writeZSOffset(int dtc, vector<TSequencerCommand*> *sequence, int value);
  void writeNSamples(int dtc, vector<TSequencerCommand*> *sequence, int value);
  void writeNPreSamples(int dtc, vector<TSequencerCommand*> *sequence, int value);
  void CreateTRU(char* buf, int position);
};

#endif
