#ifndef TPING_HH
#define TPING_HH

#include <iostream>
#include <vector>
#include <stdint.h>

#include "TRegister.hxx"
#include "dim/dis.hxx"

using namespace std;

class TPing{
	private:
		DimService *fDimService;

		int fAlive;
		char fAddress[200];
		
	public:
		
		TPing( const char *dimroot, const char *ipaddress );
		~TPing();
		
		int PingSru();

		int GetAlive(){ return fAlive; }
};

#endif
