
/*
 * Test TAnalysis methods concerning Configuration DATA
 * Please pay attention that for each operation of Reading
 * it's good practice to call a Clean to free the memory allocated.
 *
 * Once the configuration IS loaded it is possible to Plot configuration data
 * and/or print them in a Single Channel Spectrum.
 *
 
 */


void testconfanalysis()
{
	TAnalysis* A = new TAnalysis();
	A->Read("../data/Calib_255_110_0_1.bin",10);
	A->ReadConfiguration();

	int channel = 2050;
	//A->ADC_Spectrum("./Parsed/Calib_255_110_0_1.root",channel);
	//A->HIT_Spectrum("./Parsed/Calib_255_110_0_1.root",channel);
	A->Channel_Spectrum("./Parsed/Calib_255_110_0_1.root",channel);
	
	
	// Configuration Representation getting the pointer...
	TConfigurationReader * myconf = A->GetConfigurationPointer();

	myconf->PlotGain();
	myconf->PlotHoldDelay();
	myconf->PlotTHR();
	myconf->PlotHitDelay();
	// in future is also possible to write a method that represent configuration in one canvas!
	
	delete A;
}

