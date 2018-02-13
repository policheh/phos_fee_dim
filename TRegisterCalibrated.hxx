#ifndef TREGISTERCALIBRATED_HH
#define TREGISTERCALIBRATED_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TRegister.hxx"

using namespace std;

class TRegisterCalibrated : public TRegister {

public:

  // contructor
  TRegisterCalibrated( char *dimaddress, int address, int type, int number, vector<TSequencerCommand*> *sequence = 0, int *clean = 0, float calib = 1., float c = 0);

  void UpdateReadback( int readback, int force = 0);
  void SetBitwiseANDCalibrationFactor(int factor) { fCalibBitwiseAND = factor; }

private:

  float fCalib; // calibration factor
  float fConst; // additive constant: y = (x & fCalibBitwiseAND)*fCalib + fConst
  int   fCalibBitwiseAND;

};

#endif
