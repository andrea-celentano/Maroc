// for testing TCalib class

int testcalib()
{
	TCalib * mycal = new TCalib();
	
	mycal->Import("testpedestaltoimport.map",0);
	
	//mycal->Plot(0);	
	//mycal->Plot(1);	
	//mycal->Plot(2);
	//mycal->Plot(3,"AP");	
	mycal->Plot(4,"AP");	

	//mycal->Print(1);
	//mycal->Print(2);
	//mycal->Print(3);
	
	//mycal->Reset();

	
	//mycal->Export("testPedestal.map",0);
	//mycal->Export("testGain.map",1);
	
	delete mycal;
	
	return 0;
};
