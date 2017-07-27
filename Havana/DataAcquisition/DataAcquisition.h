#ifndef _DATA_ACQUISITION_H_
#define _DATA_ACQUISITION_H_

#include "afxwin.h"
#include "afxcmn.h"

#include <Configuration.h>

class CHavanaDlg;
class CVisStream;
class OCTProcess;
class FLIMProcess;

class SignatecDAQ;
#if NI_ENABLE
class GainControl;
class SyncFLIM;
class GalvoScan;
#endif
class ZaberStage;
class FaulhaberMotor;
class ElforlightLaser;

class DataAcquisition
{
public:
	DataAcquisition(CHavanaDlg* pMainDlg);
	virtual ~DataAcquisition();
	void SetLambdas();

	// Data Acquisition
	bool InitializeAcquisition();
	bool StartAcquisition();
	void StopAcquisition();

#if NI_ENABLE
	// PMT Gain Control
	bool InitializeGainControl();
	void StartGainControl();
	void StopGainControl();

	// FLIM Synchronization TTL signal generation
	bool InitializeSyncFLIM();
	void StartSyncFLIM();
	void StopSyncFLIM();

	// Galvo Scanning
	bool InitializeGalvoScan();
	void StartGalvoScan();
	void StopGalvoScan();
#endif

	// Zaber Stage
	bool EnableZaberStage();
	void DisableZaberStage();
	void OperateZaberStage(int type);

	// Faulhaber Motor
	bool EnableFaulhaberMotor();
	void DisableFaulhaberMotor();
	void OperateFaulhaberMotor(int type);

	// Elforlight Laser
	bool EnableElforlightLaser();
	void DisableElforlightLaser();
	void OperateElforlightLaser(int type);

private:
	CHavanaDlg* pMainDlg;
	CVisStream* pStream;
	OCTProcess* pOCT;
	FLIMProcess* pFLIM;

	SignatecDAQ* pDAQ;
#if NI_ENABLE
	GainControl* pGainControl;
	SyncFLIM* pSyncFLIM;
	GalvoScan* pGalvoScan;
#endif
	ZaberStage* pZaberStage;
	FaulhaberMotor* pFaulhaberMotor;
	ElforlightLaser* pElforlightLaser;
};

#endif 