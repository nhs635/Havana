#ifndef _PULSE_OVERLAP_DLG_H_
#define _PULSE_OVERLAP_DLG_H_

#pragma comment(lib, "UxTheme.lib")

#include "afxwin.h"
#include "afxcmn.h"

#include "Miscel\objcpp\signal.h"

#define MAX_VAL 32768.0f
#define MIN_VAL -4096.0f

class CHavanaDlg;
class CVisStream;

// PulseOverlapDlg 대화 상자입니다.

class PulseOverlapDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PulseOverlapDlg)

public:
	PulseOverlapDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~PulseOverlapDlg();

private:
	CHavanaDlg* m_pMainDlg;
	CVisStream* m_pVisStream;

private:
	CStatic m_StaticPulseGraph;

	CPen m_GridPen; 
	CPen m_IrfPen, m_Ch1Pen, m_Ch2Pen, m_Ch3Pen;
	CPen m_MeanDelayPen, m_WindowPen;
	CPen m_MaskPen, m_MaskSelectPen;

public:
	CRect m_RectPulseGraph;
	CRect m_RectPulseGraphWnd;
	POINT pt;
	
	BOOL m_CheckSeeSequence;
	BOOL m_CheckShowIRF;
	BOOL m_CheckShowCH1;
	BOOL m_CheckShowCH2;
	BOOL m_CheckShowCH3;
	BOOL m_CheckShowMeanDelay;
	BOOL m_CheckShowWin;
	BOOL m_CheckSoftwareBroadening;
	BOOL m_CheckSplineInterp;
	BOOL m_CheckNormalize;
	BOOL m_CheckMask;
	BOOL m_ToggleMaskModification;

public: 	
	BOOL m_LButtonClicked;
	int m_start, m_end;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PULSE_OVERLAP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	
public:
	void SetMainDlg(CHavanaDlg * pMainDlg);
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void OnCancel();

	afx_msg void OnPaint();

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnBnClickedCheckSeeSequence();
	afx_msg void OnBnClickedCheckShowIrf();
	afx_msg void OnBnClickedCheckShowCh1();
	afx_msg void OnBnClickedCheckShowCh2();
	afx_msg void OnBnClickedCheckShowCh3();
	afx_msg void OnBnClickedCheckShowMeanDelay();
	afx_msg void OnBnClickedCheckShowWin();
	afx_msg void OnBnClickedCheckBroadening();
	afx_msg void OnBnClickedCheckSplineInterp();
	afx_msg void OnBnClickedCheckNormalize();
	afx_msg void OnBnClickedCheckShowMask();
	afx_msg void OnBnClickedCheckModifyMask();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);	

	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonRemove();
};

#endif