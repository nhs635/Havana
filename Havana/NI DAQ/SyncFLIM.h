#ifndef SYNC_FLIM_H_
#define SYNC_FLIM_H_

#include <iostream>

typedef void *TaskHandle;

class SyncFLIM
{
public:
	SyncFLIM();
	~SyncFLIM();
	
	int slow;
	char* counterChannel;
	char* sourceTerminal;

	bool initialize();
	void start();
	void stop();
		
private:
	TaskHandle _taskHandle;
	void dumpError(int res, LPCSTR pPreamble);
};

#endif // SYNC_FLIM_H_