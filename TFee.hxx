#ifndef TFEE_HH
#define TFEE_HH

#include <iostream>

#include "TSequencerCommand.hxx"
#include "TRegister.hxx"
#include "TDevice.hxx"
#include "TMassConfig.hxx"

using namespace std;

class TFee : public TDevice {
	private:
		TMassConfig *fHighVoltage;
		TMassConfig *fPedestal;
		
		TRegister *fAltroOK;
		
		vector<TRegister *> *fAltro;
		vector<int> fAltroCheck;
		
	public:
		
                TFee( const char *serverRoot, int truNum, int sruNum );
		~TFee();
		
		void SetAltro( vector<TRegister *> *altro );

 		void ConfigBuildSequence( int force = 0, vector<TSequencerCommand*> *sequence = 0 );
 		void ReadbackBuildSequence( int force = 0 );
		void ReadbackProcess( vector<uint32_t> *inbuf ); // processes the readback
		
		void CheckAltro( vector<uint32_t> *inbuf );
                void ResetRegisters();

};

#endif
