#include "TRegisterCalibrated.hxx"

TRegisterCalibrated::TRegisterCalibrated( char *dimaddress, int address, int type, int number, vector<TSequencerCommand*> *sequence, int *clean, float calib, float c) : TRegister(dimaddress,address,type,number,sequence,clean),fCalib(calib),fConst(c),fCalibBitwiseAND(0)
{}


void TRegisterCalibrated::UpdateReadback( int readback, int force ) 
{
  int readbackNew;

  if(fCalibBitwiseAND)
    readbackNew = (int)( (readback & fCalibBitwiseAND)*fCalib+fConst );
  else
    readbackNew = (int)(readback*fCalib+fConst);
  
  TRegister::UpdateReadback(readbackNew,force ); 
}

