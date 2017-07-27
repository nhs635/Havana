#include "stdafx.h"
#include "FLIMProcess.h"

FLIMProcess::FLIMProcess() : _running(false)
{
}

FLIMProcess::~FLIMProcess()
{
	if (_thread.joinable())
	{
		_running = false;
		_thread.join();
	}
}

void FLIMProcess::operator() (np::Array<float, 2>& intensity, np::Array<float, 2>& mean_delay, np::Array<float, 2>& lifetime, 
	FLIM_PARAMS& pParams, int PreTrig, np::Array<uint16_t, 2>& pulse)
{
	// 1. Crop and resize pulse data
	_resize(pulse, FLIM_SPLINE_FACTOR, PreTrig, pParams);
	
	// 2. Get intensity
	_intensity(_resize, pParams, intensity);

	// 3. Get lifetime
	_lifetime(_resize, pParams, intensity, mean_delay, lifetime);
}


void FLIMProcess::SaveMaskData(CString path)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile)
	{
		DWORD dwWritten;
		WriteFile(hFile, _resize.pMask, sizeof(float) * _resize.nx, &dwWritten, NULL);
		CloseHandle(hFile);
	}
}


void FLIMProcess::LoadMaskData(CString path)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);	
	if (hFile)
	{
		DWORD dwRead;
		ReadFile(hFile, _resize.pMask, sizeof(float) * _resize.nx, &dwRead, NULL);
		CloseHandle(hFile);

		int start_count = 0, end_count = 0;
		for (int i = 0; i < _resize.nx - 1; i++)
		{
			if (_resize.pMask[i + 1] - _resize.pMask[i] == -1)
			{
				start_count++;
				if (start_count < 5)
					_resize.start_ind[start_count - 1] = i + 1;
			}
			if (_resize.pMask[i + 1] - _resize.pMask[i] == 1)
			{
				end_count++;
				if (end_count < 5)
					_resize.end_ind[end_count - 1] = i;
			}
		}

		for (int i = 0; i < 4; i++)
			printf("mask %d: [%d %d]\n", i + 1, _resize.start_ind[i], _resize.end_ind[i]);

		if ((start_count == 4) && (end_count == 4))
			printf("Proper mask is selected!!\n");
		else
			printf("Improper mask: please modify the mask!\n");
	}
}


void FLIMProcess::run()
{
	unsigned int frameIndex = 0;

	_running = true;
	while (_running)
		DidAcquireData(frameIndex++);
}


bool FLIMProcess::startFLIMprocess()
{
	if (_thread.joinable())
	{
		dumpErrorSystem(::GetLastError(), "ERROR: FLIM processing thread is already running: ");
		return false;
	}

	_thread = std::thread(&FLIMProcess::run, this);

	printf("FLIM image processing thread is started.\n");

	return true;
}


void FLIMProcess::stopFLIMprocess()
{
	if (_thread.joinable())
	{
		DidStopData(); //_running = false;
		_thread.join();
	}

	printf("FLIM image processing thread is finished normally.\n");
}


void FLIMProcess::dumpErrorSystem(int res, LPCSTR pPreamble)
{
	char *pErr = nullptr;

	CString str; str.Format(_T("Error code (%d)"), res);
	AfxMessageBox(str);

	//EndBufferedPciAcquisitionPX14(_board);
	//if (dma_bufp) FreeDmaBufferPX14(_board, dma_bufp);
	//DisconnectFromDevicePX14(_board);
}