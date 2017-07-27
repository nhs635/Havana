#ifndef _VIS_STREAM_H_
#define _VIS_STREAM_H_

#include "afxwin.h"
#include "afxcmn.h"
#include <thread>
#include <Miscel\objcpp\signal.h>

#include "ImageProcess\FLIMProcess\FLIMProcess.h"
#include "ImageProcess\circularize.h"

class CHavanaDlg;
class MyBitmap;
class CalibrationDlg;
class FLIMAnalysisDlg;
class PulseOverlapDlg;

// CVisStream 대화 상자입니다.
class CVisStream : public CDialogEx
{
	DECLARE_DYNAMIC(CVisStream)

public:
	CVisStream(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CVisStream();

private:
	CHavanaDlg* m_pMainDlg;

	CStatic m_StaticGraphRegion[3];
	CStatic m_StaticImageRegion;
	CStatic m_StaticCBarRegion1;
	CStatic m_StaticCBarRegion2;

	CPen m_MainGridPen, m_SubGridPen, m_DataPen, m_WindowPen, m_MeanDelayPen;

public:
	MyBitmap* m_pBitmapOCT;
	MyBitmap* m_pBitmapCircOCT;
	MyBitmap* m_pBitmapIntensity;
	MyBitmap* m_pBitmapLifetime;
	MyBitmap* m_pColorbarIntensity;
	MyBitmap* m_pColorbarLifetime;
	np::Array<uint8_t, 2> m_pTemp;

public:
	CRect m_GraphRegionRect[3], m_ImageRegionRect, m_CBarRegionRect1, m_CBarRegionRect2, m_VisRegionRectWnd;
	POINT pt;

public:
	CalibrationDlg* m_pCalibrationDlg;
	FLIMAnalysisDlg* m_pFLIMAnalysisDlg;
	PulseOverlapDlg* m_pPulseOverlapDlg;
	
private:
	np::Array<float> half_16bit;

	float m_dGridCount, m_dCountPerGrid;
	float m_dStep, m_dDataStep;	

	float m_dIncrementX_FLIM, m_dIncrementX_OCT, m_dIncrementY;
	float m_dIncrementX_FFT, m_dIncrementY_FFT;
	
public:
	int m_Max_dB, m_Min_dB;
	int m_EditCircCenter;
	float m_Max_FLIM, m_Min_FLIM;
	float m_Max_FLIM_life, m_Min_FLIM_life;

	int m_nScans, m_nScansFLIM, m_nScansFFT, m_nAlines;
	int m_nSizeFrame, m_nSizeImageOCT;

	FLIM_PARAMS m_Params_FLIM;	
	
public:
	// thread operation	
	signal<int> DidAcquireData; // signal	
	signal<void> DidStopData;

private:
	void run(); // run visualization (thread)	
	std::thread _thread; // thread object

public:
	bool _running;
	bool startVisualization();
	void stopVisualization();

public:
	circularize circ_OCT;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_STREAM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	
public:	
	BOOL m_ToggleViewMode;
	BOOL m_CheckFLIMShowWindow;
	BOOL m_CheckFLIMSoftwareBroadening;
	BOOL m_CheckBgRemovedFringe;
	BOOL m_CheckCircularize;
	int m_ComboLUT;
	int m_ComboFLIMShow;
	int m_SliderCurAline;

	void SetMainDlg(CHavanaDlg* pMainDlg);
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnBnClickedToggleViewMode();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

	// FLIM Operation
	afx_msg void OnBnClickedCheckFlimShowWindow();
	afx_msg void OnBnClickedCheckFlimSoftwareBroadening();
	afx_msg void OnDeltaposSpinChannelStartCh0(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinChannelStartCh1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinChannelStartCh2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpinChannelStartCh3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditDelayOffsetCh();
	//afx_msg void OnEnChangeEditDelayOffsetCh2();
	//afx_msg void OnEnChangeEditDelayOffsetCh3();

	afx_msg void OnEnChangeEditVisFlimSamps();
	afx_msg void OnEnChangeEditFlimBackground();
	afx_msg void OnBnClickedButtonCaptureFlimBackground();
	afx_msg void OnDeltaposSpinFlimChannel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeComboFlimShow();
	afx_msg void OnBnClickedButtonFlimAnalysis();
	afx_msg void OnBnClickedButtonFlimPulseOverlappedView();

	afx_msg void OnEnChangeEditFlimMax();
	afx_msg void OnEnChangeEditFlimMin();
	afx_msg void OnEnChangeEditFlimMaxLife();
	afx_msg void OnEnChangeEditFlimMinLife();

	// OCT Operation
	afx_msg void OnBnClickedCheckBgRemovedFringe();
	afx_msg void OnBnClickedCheckCircularize();
	afx_msg void OnBnClickedButtonCalibration();
	afx_msg void OnCbnSelchangeComboLut();

	afx_msg void OnEnChangeEditAlineMaxDb();
	afx_msg void OnEnChangeEditAlineMinDb();
	afx_msg void OnEnChangeEditCircCenter();
	afx_msg void OnEnChangeEditFlimWidth();
};

#endif