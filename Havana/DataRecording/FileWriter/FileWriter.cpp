#include "stdafx.h"
#include "FileWriter.h"

using namespace std;

FileWriter::FileWriter() :
	nChannels(2),
	nScans(2600),
	nAlines(1024),
	FilePath(_T("F:\\temp.data")),
	//pFile(nullptr),
	_running(false),
	is_saved(false)
{	
}


FileWriter::~FileWriter()
{
	if (_thread.joinable())
	{
		_running = false;
		_thread.join();
	}

	//if (pFile)
	//	fclose(pFile);
}


void FileWriter::write(std::queue<UINT16*> &queue, int Nbuffer)
{		
	_thread = std::thread(recording_lambda, std::ref(queue), Nbuffer);

	printf("Data saving thread is started.\n");
}


void FileWriter::stop()
{
	// This fuction is only executed by user's interrupt.
	if (_thread.joinable())
	{
		_running = false;
		_thread.join();
	}
	
	//if (pFile)
	//	fclose(pFile);

	printf("Data saving thread is halted by user.\n");
}