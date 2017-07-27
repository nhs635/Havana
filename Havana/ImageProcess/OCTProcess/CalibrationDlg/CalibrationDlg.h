#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include "afxwin.h"
#include "afxcmn.h"

#include "Miscel\objcpp\signal.h"

class CHavanaDlg;
class CVisStream;
class OCTProcess;

// Calibration 대화 상자입니다.
class CalibrationDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CalibrationDlg)

public:
	CalibrationDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CalibrationDlg();

private:
	CHavanaDlg* m_pMainDlg;
	CVisStream* m_pVisStream;
	OCTProcess* m_pOCT;

public:
	CStatic m_StaticResult;
	int m_Edit_DiscomVal;
	BOOL m_CheckResult;

private:
	CRect m_ScopeRect;
	CRect m_ScopeRectWnd; 
	POINT pt;
	CPen m_GridPen;
	CPen m_DataPen, m_DataPen1;

public:
	float* m_pScopeData;
	int m_DataNum;
	float m_Max, m_Min;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CALIBRATION };
#endif

public: // Callback	
	signal2<float*, CString> DrawGraph;
	signal2<int&, int&> WaitForRange;

	BOOL m_MouseClickAvailable;
	BOOL m_MouseClick;
	BOOL m_GoAhead;
	int m_start, m_end;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	void SetMainDlg(CHavanaDlg* pMainDlg);
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual void OnCancel();

	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	afx_msg void OnBnClickedButtonBackground();
	afx_msg void OnBnClickedButtonD1();
	afx_msg void OnBnClickedButtonD2();
	afx_msg void OnBnClickedButtonGenerateCalib();	

	afx_msg void OnEnChangeEdit1();	
	afx_msg void OnBnClickedButtonGoAhead();

	afx_msg void OnStnClickedStaticBackground();
	afx_msg void OnStnClickedStaticD1();
	afx_msg void OnStnClickedStaticD2();
	afx_msg void OnStnClickedStaticGenerateCalib();
};

#endif
