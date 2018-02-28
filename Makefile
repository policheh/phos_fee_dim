bobjects = PhsFeeDim udp_server srucmd dim_command countd feed client2 read1 setPedestals setHV setRun ResetFEE ResetTRU ResetFEE2 ResetHV CSPmask ConfigureAllFEE buildPedestals buildBadMaps

CC = gcc
CXX = g++

CFLAGS = -g -Wall -I./dim -I./
CXXFLAGS = -g -Wall
CXXFLAGS += -I./dim -I./dim/dim -I./ 

all : $(bobjects)

PhsFeeDim : TSru.o TDevice.o TFee.o TTru.o TSocket.o TSequencer.o TSequencerCommand.o TRegister.o TRegisterCommand.o TRegisterCalibrated.o TMassConfig.o TAltroCFG.o THvCFG.o TApdSettingsCheck.o TResponseCheck.o TCardOnCFG.o TBusyFeeCFG.o TBusyTruCFG.o TBusySruCFG.o TPedCFG.o TBaseCFG.o TPing.o TDimCounter.o test.cxx
	$(CXX) $(CXXFLAGS) -L./dim/ -ldim -lpthread -o $@ TSru.cxx TDevice.cxx TFee.cxx TTru.cxx TSocket.cxx TSequencer.cxx TSequencerCommand.cxx TRegister.cxx TRegisterCommand.cxx TRegisterCalibrated.cxx TMassConfig.o TAltroCFG.o THvCFG.o TApdSettingsCheck.o TResponseCheck.o TCardOnCFG.o TBusyFeeCFG.o TBusyTruCFG.o TBusySruCFG.o TPedCFG.o TBaseCFG.o TPing.cxx TDimCounter.cxx test.cxx ./dim/libdim.a

udp_server : 
	$(CXX) $(CXXFLAGS) -o $@ udp_server.cxx

dim_command : 
	$(CXX) $(CXXFLAGS) -lpthread -o $@ dim_command.cxx ./dim/libdim.a

countd : countd.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ countd.cxx ./dim/libdim.a

feed : feed.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ feed.cxx ./dim/libdim.a

client2 : client2.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ client2.cxx ./dim/libdim.a

read1 : read1.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ read1.cxx ./dim/libdim.a

dimGui: dimGui.cxx
	$(CXX) $(CXXFLAGS) `root-config --cflags --glibs` -L./dim -ldim -lpthread -o $@ dimGui.cxx ./dim/libdim.a

setPedestals : setPedestals.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ setPedestals.cxx ./dim/libdim.a

setHV: setHV.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ setHV.cxx ./dim/libdim.a

setRun: setRun.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ setRun.cxx ./dim/libdim.a

ResetFEE: ResetFEE.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ ResetFEE.cxx ./dim/libdim.a

ResetFEE2: ResetFEE2.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ ResetFEE2.cxx ./dim/libdim.a

ResetTRU: ResetTRU.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ ResetTRU.cxx ./dim/libdim.a

ResetHV: ResetHV.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ ResetHV.cxx ./dim/libdim.a

CSPmask: CSPmask.cxx
	$(CXX) $(CXXFLAGS) -lpthread -o $@ CSPmask.cxx

ConfigureAllFEE : ConfigureAllFEE.cxx
	$(CXX) $(CXXFLAGS) -L./dim -ldim -lpthread -o $@ ConfigureAllFEE.cxx ./dim/libdim.a

AliPHOSFEEMapRun2.o : AliPHOSFEEMapRun2.cxx AliPHOSFEEMapRun2.h
	$(CXX) $(CXXFLAGS) -c AliPHOSFEEMapRun2.cxx

buildPedestals : buildPedestals.cxx AliPHOSFEEMapRun2.o CreatePedestalTable.C
	$(CXX) -L$(ROOTLIB) -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lMathCore -lThread -pthread -lm -ldl -rdynamic $(CXXFLAGS) -I/usr/include/root -I$(ROOTSYS)/include -L./dim -ldim -lpthread -o $@ buildPedestals.cxx AliPHOSFEEMapRun2.o ./dim/libdim.a

buildBadMaps : buildBadMaps.cxx AliPHOSFEEMapRun2.o
	$(CXX) -L$(ROOTLIB) -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lMathCore -lThread -pthread -lm -ldl -rdynamic $(CXXFLAGS) -I/usr/include/root -I$(ROOTSYS)/include -lpthread -o $@ buildBadMaps.cxx AliPHOSFEEMapRun2.o

srucmd : TSocket.o
	$(CXX) $(CXXFLAGS) -o $@ TSocket.cxx srucmd.cxx

TSru.o : TSru.cxx TSru.hxx
	$(CXX) $(CFLAGS) -c TSru.cxx

TDevice.o : TDevice.cxx TDevice.hxx
	$(CXX) $(CFLAGS) -c TDevice.cxx

TFee.o : TFee.cxx
	$(CXX) $(CFLAGS) -c TFee.cxx

TTru.o : TTru.cxx TTru.hxx
	$(CXX) $(CFLAGS) -c TTru.cxx

TSocket.o : TSocket.cxx
	$(CXX) $(CFLAGS) -c TSocket.cxx

TSequencer.o : TSequencer.cxx
	$(CXX) $(CFLAGS) -c TSequencer.cxx

TSequencerCommand.o : TSequencerCommand.cxx
	$(CXX) $(CFLAGS) -c TSequencerCommand.cxx

TRegister.o : TRegister.cxx
	$(CXX) $(CFLAGS) -c TRegister.cxx

TRegisterCommand.o : TRegisterCommand.cxx
	$(CXX) $(CFLAGS) -c TRegisterCommand.cxx

TRegisterCalibrated.o : TRegisterCalibrated.cxx TRegisterCalibrated.hxx
	$(CXX) $(CFLAGS) -c TRegisterCalibrated.cxx

TMassConfig.o : TMassConfig.cxx
	$(CXX) $(CFLAGS) -c TMassConfig.cxx

TAltroCFG.o : TAltroCFG.cxx TAltroCFG.hxx
	$(CXX) $(CFLAGS) -std=gnu++0x -c TAltroCFG.cxx

THvCFG.o : THvCFG.cxx THvCFG.hxx
	$(CXX) $(CFLAGS) -c THvCFG.cxx

TApdSettingsCheck.o : TApdSettingsCheck.cxx TApdSettingsCheck.hxx
	$(CXX) $(CFLAGS) -c TApdSettingsCheck.cxx

TResponseCheck.o : TResponseCheck.cxx TResponseCheck.hxx 
	$(CXX) $(CFLAGS) -c TResponseCheck.cxx

TCardOnCFG.o : TCardOnCFG.cxx TCardOnCFG.hxx
	$(CXX) $(CFLAGS) -c TCardOnCFG.cxx

TBusyFeeCFG.o : TBusyFeeCFG.cxx TBusyFeeCFG.hxx
	$(CXX) $(CFLAGS) -c TBusyFeeCFG.cxx

TBusyTruCFG.o : TBusyTruCFG.cxx TBusyTruCFG.hxx
	$(CXX) $(CFLAGS) -c TBusyTruCFG.cxx

TBusySruCFG.o : TBusySruCFG.cxx TBusySruCFG.hxx
	$(CXX) $(CFLAGS) -c TBusySruCFG.cxx

TPedCFG.o : TPedCFG.cxx TPedCFG.hxx
	$(CXX) $(CFLAGS) -c TPedCFG.cxx

TBaseCFG.o : TBaseCFG.cxx TBaseCFG.hxx
	$(CXX) $(CFLAGS) -c TBaseCFG.cxx

TPing.o : TPing.cxx
	$(CXX) $(CFLAGS) -c TPing.cxx

TDimCounter.o : TDimCounter.cxx
	$(CXX) $(CFLAGS) -c TDimCounter.cxx

uninstall: clean
	( cd $(IBINPATH) ; rm -f $(bobjects) )

clean: 
	rm -f $(bobjects)
	rm -f *.o
	rm -f M*-* APDsettings Pedestal common

distclean: clean uninstall
	rm -f ./*~

install: all
	ln -s $(HOME)/PHOSControl_5/M*-* ./
	ln -s $(HOME)/PHOSControl_5/APDsettings ./
	ln -s $(HOME)/PHOSControl_5/Pedestal ./
	ln -s $(HOME)/PHOSControl_5/common ./