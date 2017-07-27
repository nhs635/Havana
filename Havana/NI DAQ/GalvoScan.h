#ifndef GALVO_SCAN_H_
#define GALVO_SCAN_H_

#include <iostream>

typedef void *TaskHandle;

class GalvoScan
{
public:
	GalvoScan();
	~GalvoScan();
	
	int nAlines;

	double pp_voltage;
	double offset;
	double max_rate;

	char* physicalChannel;
	char* sourceTerminal;

	bool initialize();
	void start();
	void stop();
		
private:
	TaskHandle _taskHandle;

	int N;
	double* data;

	void dumpError(int res, LPCSTR pPreamble);
};

#endif // GALVO_SCAN_H_