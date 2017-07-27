#ifndef FILE_WRITER_H_
#define FILE_WRITER_H

#include <Miscel\objcpp\signal.h>

#include <iostream>
#include <queue>
#include <thread>

typedef std::function<void(std::queue<UINT16*> &, int)> MyRecording;

class FileWriter
{
public:
	// variables
	int nScans, nAlines, nChannels;	
	bool _running;
	bool is_saved;

	// file
	CString FilePath;
//	FILE* pFile;

	// signal
	MyRecording recording_lambda;
	signal<CString> SendStatusMessage;
	
	// constructor and destructor
	FileWriter();
	virtual ~FileWriter();

	// operation
	void write(std::queue<UINT16*> &queue, int Nbuffer);
	void stop();

private:
	// thread
	std::thread _thread;	

};

#endif