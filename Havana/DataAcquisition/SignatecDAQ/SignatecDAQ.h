#ifndef _SIGNATEC_DAQ_H_
#define _SIGNATEC_DAQ_H_

// #include <objcpp/property.h>
#include <Miscel\numcpp\array.h>
#include <Miscel\objcpp\signal.h>

//#include <Configuration.h>

#include <iostream>
#include <thread>

typedef struct _px14hs_* HPX14;

class SignatecDAQ
{
public:
	//void accept(property_visitor &visit);
	//property<bool> UseVirtualDevice, UseInternalTrigger; // for debugging purpose

	int nChannels, nScans, nAlines;
	int VoltRange1, VoltRange2;
	int AcqRate;
	int PreTrigger, TriggerDelay;
	bool UseVirtualDevice, UseInternalTrigger;
	
	SignatecDAQ();
	virtual ~SignatecDAQ();

	// operations void Start(), Stop(), SwitchOn(), SwitchOff();
	// signals (callback?)
	signal2<int, const np::Array<uint16_t, 2> &> DidAcquireData; 
	signal<void> DidStopData;
	signal<CString> SendStatusMessage; 

	bool doneAcquire;

	bool initialize();
	bool set_init();
	bool startAcquisition();
	void stopAcquisition();

	bool _running;

private:
	//void switchOn();
	//void switchOff();

    bool _dirty;

	// thread
	std::thread _thread;
	void run();

private:
	// PX14400 board driver handle
	HPX14 _board;

	// DMA buffer pointer to acquire data
	//std::array<unsigned short*, 8> dma_bufp;
	unsigned short *dma_bufp;

	// Data buffer size (2 channel, half size. merge two buffers to build a frame) 
	int getDataBufferSize() { return nChannels * nScans * nAlines / 4; } 

	// Dump a PX14400 library error
	void dumpError(int res, LPCSTR pPreamble);
	void dumpErrorSystem(int res, LPCSTR pPreamble);
};

#endif // SIGNATEC_DAQ_H_