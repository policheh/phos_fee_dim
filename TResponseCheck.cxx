#include "TBaseCFG.hxx"
#include "TDevice.hxx"
#include "TResponseCheck.hxx"

using namespace std;

TResponseCheck::TResponseCheck(char* name, vector<TSequencerCommand*> *sequence, TDevice* dev) :
TBaseCFG(name,sequence),fDevice(0x0)
{
    fDevice = dev;
}

void TResponseCheck::CalculateStatus()
{
    fStatus = fDevice->IsResponsive();
}
