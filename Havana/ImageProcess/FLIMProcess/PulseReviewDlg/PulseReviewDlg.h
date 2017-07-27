#ifndef _PULSE_REVIEW_DLG_H_
#define _PULSE_REVIEW_DLG_H_

#include "afxwin.h"
#include "afxcmn.h"

#include "Miscel\objcpp\signal.h"

#define MAX_VAL 32768.0f
#define MIN_VAL -4096.0f

class CHavanaDlg;
class CVisResult;

typedef np::Array<float, 2> PULSE_DATA;

// PulseReviewDlg 대화 상자입니다.

class PulseReviewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PulseReviewDlg)

public:
	PulseReviewDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~PulseReviewDlg();

private:
	CHavanaDlg* m_pMainDlg;
	CVisResult* m_pVisResult;

private:
	CStatic m_StaticPulseReviewGraph;
	CPen m_GridPen, m_DataPen, m_WindowPen;

public:
	CRect m_RectPulseGraph;
	CRect m_RectPulseGraphWnd;
	POINT pt;

	int m_SliderCurAline;
	int m_ComboTypePulse;
	
public:
	std::vector<PULSE_DATA>* m_pSelectedVector;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PULSE_REVIEW };
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
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCbnSelchangeComboTypePulse();
};

#endif