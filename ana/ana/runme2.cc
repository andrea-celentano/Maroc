
// This script compiles and loads all the libraries
//
// .x runme2.cc
//
// 5 seconds to compile having modified just TAnalysis (May 7th 2013 on macbook)
//

void runme2() {
		
//	gSystem->Load("/usr/local/lib/libconfig++"); // on my macbook (matteo)
	gSystem->Load("/usr/lib64/libconfig++.so"); // on pclb7 at ISS 	
	
	// ++g for debug, ++O for optimization
	gROOT->ProcessLine(".L src/General.h+O"); 
	gROOT->ProcessLine(".L src/TMaroc.cpp+O"); 
	
	gROOT->ProcessLine(".L src/TVME.cpp+O"); 
	
	gROOT->ProcessLine(".L src/TH8500.cpp+O"); 
	gROOT->ProcessLine(".L src/TH9500.cpp+O"); 
	gROOT->ProcessLine(".L src/TR8900.cpp+O"); 
	
	gROOT->ProcessLine(".L src/TSiPM.cpp+O"); // new 2014, Nov 25th
	
	gROOT->ProcessLine(".L src/TConnectH8500.cpp+O"); 
	gROOT->ProcessLine(".L src/TConnectH9500.cpp+O"); 
	gROOT->ProcessLine(".L src/TConnectR8900.cpp+O"); 

	//gROOT->ProcessLine(".L ./src/TMFE.cpp+O");  // old configuration manager
	
	gROOT->ProcessLine(".L src/CNSAnalysis.cpp+O"); // new 2013 Giugno 13, Matteo Turisini
	
	
	gROOT->ProcessLine(".L src/TEvent.cpp+O"); 
	gROOT->ProcessLine(".L src/TFileRaw.cpp+O");
	gROOT->ProcessLine(".L src/TCalib.cpp+O");
	gROOT->ProcessLine(".L src/TBlock.cpp+O");
	gROOT->ProcessLine(".L src/MRCConfigurationManager.cpp+O");
	gROOT->ProcessLine(".L src/MRCSetup.cpp+O");
	gROOT->ProcessLine(".L src/TAnalysis.cpp+O");
	gROOT->ProcessLine(".L src/TMRCScanner.cpp+O");
	
	gROOT->ProcessLine(".!rm ./src/*.d");
	
	// creates directory
	//gSystem->mkdir("./mydir/mydir/mydur",true);
}

// gROOT->ProcessLine(".cd C:/data/sw_analisi/Modularizzazione/BinDisplay");


