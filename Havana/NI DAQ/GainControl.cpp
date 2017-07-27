#include "stdafx.h"
#include "GainControl.h"

#include "NIDAQmx.h"

using namespace std;

GainControl::GainControl() :
	_taskHandle(nullptr),
	voltage(0),
	physicalChannel("Dev1/ao0")
{
}

GainControl::~GainControl()
{
	if (_taskHandle) 
		DAQmxClearTask(_taskHandle);
}


bool GainControl::initialize()
{
	printf("Initializing NI Analog Output for PMT gain control...\n");

	int res;
	double data[1] = { voltage };

	/*********************************************/
	// Voltage Generator
	/*********************************************/
	if ((res = DAQmxCreateTask("", &_taskHandle)) != 0)
	{
		dumpError(res, "ERROR: Failed to set Gain Control: ");
		return false;
	}
	if ((res = DAQmxCreateAOVoltageChan(_taskHandle, physicalChannel, "", -10.0, 10.0, DAQmx_Val_Volts, NULL)) != 0)
	{
		dumpError(res, "ERROR: Failed to set Gain Control: ");
		return false;
	}	
	if ((res = DAQmxWriteAnalogF64(_taskHandle, 1, TRUE, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, data, NULL, NULL)) != 0)
	{
		dumpError(res, "ERROR: Failed to set Gain Control: ");
		return false;
	}		

	printf("NI Analog Output for PMT gain control is successfully initialized.\n");	

	return true;
}


void GainControl::start()
{
	if (_taskHandle)
	{
		printf("PMT gain control generates a voltage...\n");
		DAQmxStartTask(_taskHandle);
	}
}


void GainControl::stop()
{
	if (_taskHandle)
	{
		double data[1] = { 0.0 };
		DAQmxWriteAnalogF64(_taskHandle, 1, TRUE, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, data, NULL, NULL);

		printf("NI Analog Output is stopped.\n");
		DAQmxStopTask(_taskHandle);
		DAQmxClearTask(_taskHandle);
		
		_taskHandle = nullptr;
	}
}


void GainControl::dumpError(int res, LPCSTR pPreamble)
{	
	char errBuff[2048];
	if (res < 0)
		DAQmxGetErrorString(res, errBuff, 2048);

	AfxMessageBox((CString)((LPCSTR)errBuff));
	printf("%s\n", errBuff);

	if (_taskHandle)
	{
		double data[1] = { 0.0 };
		DAQmxWriteAnalogF64(_taskHandle, 1, TRUE, DAQmx_Val_WaitInfinitely, DAQmx_Val_GroupByChannel, data, NULL, NULL);

		DAQmxStopTask(_taskHandle);
		DAQmxClearTask(_taskHandle);
		
		_taskHandle = nullptr;
	}
}
