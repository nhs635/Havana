#ifndef DATA_RECORDING_H_
#define DATA_RECORDING_H_

#include "afxwin.h"
#include "afxcmn.h"

#include <iostream>
#include <thread>

class CHavanaDlg;
class FileWriter;

class DataRecording
{
public:
	DataRecording(CHavanaDlg* pMainDlg);
	virtual ~DataRecording();

	bool isRecording;
	int nRecordedFrames;
//	int nFramesToCopy;

	// Data Recording
	void StartRecording();
	void StopRecording();

	// Data Saving
	void StartSaving();
	void StopSaving();
	
private:
	CHavanaDlg* pMainDlg;
	FileWriter* pWriter;

};

#endif 