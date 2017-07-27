#ifndef _HAVANA_DLG_H_
#define _HAVANA_DLG_H_

#include "afxcmn.h"
#include "afxwin.h"

#include "Tab View\VisStream.h"
#include "Tab View\VisResult.h"
#include "DataAcquisition\DataAcquisition.h"
#include "DataRecording\DataRecording.h"
#include "ImageProcess\OCTProcess\OCTProcess.h"
#include "ImageProcess\FLIMProcess\FLIMProcess.h"
#include "Configuration.h"

#include "Miscel\objcpp\Queue.h"
#include <iostream>
#include <complex>
#include <thread>
#include <mutex>
#include <queue>

// CHavanaDlg 대화 상자
class CHavanaDlg : public CDialogEx
{
// 생성입니다.
public:
	CHavanaDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

	// For Tab View
	CWnd* m_pWndShow; 
	CVisStream m_VisStream;
	CVisResult m_VisResult;

	DataAcquisition* m_pDataAcquisition;
	DataRecording* m_pDataRecording;
	OCTProcess* m_pOCT;
	FLIMProcess* m_pFLIM;
	
	// Control Variables
	CStatusBarCtrl m_StatusBar;
	CTabCtrl m_Tab;
	CButton m_ToggleAcquisition;
	CButton m_ToggleRecording;
	CButton m_ToggleSaving;
	CProgressCtrl m_ProgressBar;
	BOOL m_CheckPmtGainControl;
	BOOL m_CheckFLIMSync;
	BOOL m_CheckInternalSync;
	BOOL m_CheckGalvoScan;
	BOOL m_CheckZaberStage;
	BOOL m_CheckFaulhaberMotor;
	BOOL m_ToggleRotateMotor;
	BOOL m_CheckElforlightLaser;
	
	double m_Edit_PmtGain;
	int m_Edit_RepetitionRate;
	int m_ScrollAdjustGalvo;
	double m_Edit_GalvoVoltage, m_Edit_GalvoOffset;
	int m_Combo_ActiveChannel, m_Combo_FLIMInputRange, m_Combo_OCTInputRange, m_Combo_TrigSource;
	int m_Edit_nAlines, m_Edit_nScans, m_Edit_nFFT;
	int m_Edit_PreTrig, m_Edit_TrigDelay, m_Edit_AdcRate;
	float m_adInputVol[25] = { 0.220f, 0.247f, 0.277f, 0.311f, 0.349f, 0.391f, 0.439f, 0.493f, 0.553f, 0.620f,
								0.696f, 0.781f, 0.876f, 0.983f, 1.103f, 1.237f, 1.388f, 1.557f, 1.748f, 1.961f,
								2.200f, 2.468f, 2.770f, 3.108f, 3.487f }; // Voltage
	
	double m_Edit_MoveAbs, m_Edit_TargetSpeed;
	int m_Edit_RPM;
	
	// Buffers for threading operations
	std::queue<uint16_t*> m_queue_fringe;
	std::queue<uint16_t*> m_queue_pulse;
	std::queue<float*> m_queue_image;
	std::queue<float*> m_queue_flim;

	// Synchronization objects for threading operations
	Queue<const uint16_t*> m_Queue_Copy;
	Queue<uint16_t*> m_Queue_OCT_proc;
	Queue<uint16_t*> m_Queue_FLIM_proc;
	Queue<float*> m_Queue_OCT_vis;
	Queue<float*> m_Queue_FLIM_vis;

	std::mutex m_mtx_OCT_proc;
	std::mutex m_mtx_OCT_vis;
	std::mutex m_mtx_FLIM_proc;
	std::mutex m_mtx_FLIM_vis;
	std::mutex m_mtx_Invalidate;

	// Buffer allocations
	std::queue<uint16_t*> m_awBufferQueue;
	BOOL m_bIsBufferAllocated;

	np::Array<uint16_t, 2> m_FrameFringe;
	np::Array<float, 2> m_FrameOCT, m_FrameOCT_tp;

	np::Array<uint16_t, 2> m_FramePulse;
	np::Array<float, 2> m_FluIntensity, m_FluMeanDelay, m_FluLifetime;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HAVANA_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	virtual void PostNcDestroy();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public: // About Ini File
	CString m_strIniPath;
	void SetIniFile();
	void GetIniFile();
	int temp_discom;

public:			
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedToggleAcquisition();
	afx_msg void OnBnClickedToggleRecording();
	afx_msg void OnBnClickedToggleSaving();
	
	afx_msg void OnBnClickedCheckPmtGain();
	afx_msg void OnBnClickedCheckFlimSync();
	afx_msg void OnBnClickedCheckInternalSync();
	
	afx_msg void OnEnChangeEditGalvoPpVoltage();
	afx_msg void OnEnChangeEditGalvoOffset();
	afx_msg void OnBnClickedCheckGalvoScan();

	afx_msg void OnBnClickedCheckZaberStage();
	afx_msg void OnBnClickedButtonZaberHome();
	afx_msg void OnBnClickedButtonZaberStop();
	afx_msg void OnBnClickedButtonMoveAbsolute();
	afx_msg void OnBnClickedButtonSetTargetSpeed();
	afx_msg void OnEnChangeEditZaberSetTargetSpeed();

	afx_msg void OnBnClickedCheckFaulhaberMotor();
	afx_msg void OnBnClickedToggleFaulhaberRotateMotor();

	afx_msg void OnBnClickedCheckElforlightLaserControl();
	afx_msg void OnBnClickedButtonElforlightIncrease();
	afx_msg void OnBnClickedButtonElforlightDecrease();

	afx_msg void OnEnChangeEditAdcRate();
	afx_msg void OnCbnSelchangeComboFlimInputRange();	
	afx_msg void OnCbnSelchangeComboOctInputRange();
	
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	
	afx_msg void OnBnClickedButton1();	
	
};

#endif