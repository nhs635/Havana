#ifndef GAIN_CONTROL_H_
#define GAIN_CONTROL_H_

#include <iostream>

typedef void *TaskHandle;

class GainControl
{
public:
	GainControl();
	~GainControl();

	double voltage;
	
	char* physicalChannel;
	char* sourceTerminal;

	bool initialize();
	void start();
	void stop();
		
private:
	TaskHandle _taskHandle;
	
	void dumpError(int res, LPCSTR pPreamble);
};

#endif // GAIN_CONTROL_H_