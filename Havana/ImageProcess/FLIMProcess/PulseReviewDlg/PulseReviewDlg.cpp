// PulseReviewDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Havana.h"
#include "HavanaDlg.h"
#include "PulseReviewDlg.h"
#include "afxdialogex.h"

#include <Tab View\VisResult.h>
#include "ImageProcess\MyBitmap.h"

// PulseReviewDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(PulseReviewDlg, CDialogEx)

PulseReviewDlg::PulseReviewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PULSE_REVIEW, pParent),
	m_GridPen(PS_SOLID, 1, RGB(0, 64, 0)), 
	m_DataPen(PS_SOLID, 1, RGB(255, 255, 255)), m_WindowPen(PS_SOLID, 2, RGB(255, 255, 0)),
	m_ComboTypePulse(0), m_pSelectedVector(nullptr), m_SliderCurAline(0)
{
}

PulseReviewDlg::~PulseReviewDlg()
{
}

void PulseReviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PULSE_REVIEW_GRAPH, m_StaticPulseReviewGraph);
	DDX_CBIndex(pDX, IDC_COMBO_TYPE_PULSE, m_ComboTypePulse);
	DDX_Slider(pDX, IDC_SLIDER_REVIEW_CUR_ALINE, m_SliderCurAline);
}


BEGIN_MESSAGE_MAP(PulseReviewDlg, CDialogEx)
	ON_WM_PAINT()
	ON_CBN_SELCHANGE(IDC_COMBO_TYPE_PULSE, &PulseReviewDlg::OnCbnSelchangeComboTypePulse)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// PulseReviewDlg �޽��� ó�����Դϴ�.
void PulseReviewDlg::SetMainDlg(CHavanaDlg * pMainDlg)
{
	m_pMainDlg = pMainDlg;
	m_pVisResult = &(m_pMainDlg->m_VisResult);
}


BOOL PulseReviewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	CWnd *pWnd = this->GetDlgItem(IDC_STATIC_PULSE_REVIEW_GRAPH);
	pWnd->GetWindowRect(&m_RectPulseGraphWnd);
	this->ScreenToClient(&m_RectPulseGraphWnd);
	pt = { -m_RectPulseGraphWnd.left, -m_RectPulseGraphWnd.top };
	m_RectPulseGraph = m_RectPulseGraphWnd;
	m_RectPulseGraph.OffsetRect(pt);

	// Set Control
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_REVIEW_CUR_ALINE))->SetRange(0, m_pVisResult->m4_nAlines - 1);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_REVIEW_CUR_ALINE))->SetPos(0);

	// Set Range
	CString str;
	str.Format(_T("%d"), (int)MAX_VAL);
	GetDlgItem(IDC_STATIC_REVIEW_MAX_VAL)->SetWindowTextW(str);
	str.Format(_T("%d"), (int)MIN_VAL);
	GetDlgItem(IDC_STATIC_REVIEW_MIN_VAL)->SetWindowTextW(str);	
	str.Format(_T("Current A-line : %4d / %d"), 4, m_pVisResult->m_nAlines);
	GetDlgItem(IDC_STATIC_REVIEW_CUR_ALINE)->SetWindowTextW(str);

	// Set Data
	m_pSelectedVector = &m_pVisResult->m_PulseCrop;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void PulseReviewDlg::PostNcDestroy()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (m_pVisResult->m_pPulseReviewDlg != nullptr)
		m_pVisResult->m_pPulseReviewDlg = nullptr;

	delete this;
}


void PulseReviewDlg::OnCancel()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	DestroyWindow();
}


void PulseReviewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
					   // �׸��� �޽����� ���ؼ��� CDialogEx::OnPaint()��(��) ȣ������ ���ʽÿ�.

	CPaintDC dcScope(&m_StaticPulseReviewGraph);
	CDC dcMem;

	// Assigning memory DC
	dcMem.CreateCompatibleDC(&dc);
	CBitmap btmp;
	btmp.CreateCompatibleBitmap(&dc, m_RectPulseGraph.Width(), m_RectPulseGraph.Height());
	dcMem.SelectObject(&btmp);

	// Draw background & grid
	dcMem.FillSolidRect(m_RectPulseGraph, RGB(0, 0, 0));
	dcMem.SelectObject(&m_GridPen);
	for (int i = 0; i < 20; i++)
	{
		dcMem.MoveTo(m_RectPulseGraph.Width() * i / 20, 0);
		dcMem.LineTo(m_RectPulseGraph.Width() * i / 20, m_RectPulseGraph.Height());
	}
	dcMem.MoveTo(0, int(m_RectPulseGraph.Height() * MAX_VAL / (MAX_VAL - MIN_VAL)));
	dcMem.LineTo(m_RectPulseGraph.Width(), int(m_RectPulseGraph.Height() * MAX_VAL / (MAX_VAL - MIN_VAL)));

	// Set data
	int nFrame = m_pVisResult->m_SliderCurFrame;
	int nDataWidth = m_pSelectedVector->at(nFrame).size(0);

	PULSE_DATA Pulse(m_pSelectedVector->at(nFrame), nDataWidth, m_pVisResult->m4_nAlines);

	float xInc = float(m_RectPulseGraph.Width()) / float(nDataWidth - 1);
	float yInc = float(m_RectPulseGraph.Height()) / float(MAX_VAL - MIN_VAL);

	SetDlgItemInt(IDC_STATIC_REVIEW_NUM_SAMPLES, nDataWidth);

	// Draw Graphs
	dcMem.SelectObject(&m_DataPen);
	for (int i = 0; i < nDataWidth - 1; i++)
	{
		dcMem.MoveTo(int(xInc * (i)), m_RectPulseGraph.Height() - int(yInc * (Pulse(i, m_SliderCurAline) - MIN_VAL)));
		dcMem.LineTo(int(xInc * (i + 1)), m_RectPulseGraph.Height() - int(yInc * (Pulse(i + 1, m_SliderCurAline) - MIN_VAL)));
	}

	// Draw Windows
	int ch[4];
	memcpy(ch, m_pVisResult->m_pFLIMforExt->_resize.ch_start_ind1, 4 * sizeof(int));
	int ext_factor = (m_ComboTypePulse < 2) ? (int)m_pVisResult->m_pFLIMforExt->_resize.ActualFactor : 1;
	
	dcMem.SelectObject(&m_WindowPen);
	for (int i = 0; i < 4; i++)
	{
		dcMem.MoveTo(int(xInc * (ch[i] - ch[0]) / ext_factor), 0);
		dcMem.LineTo(int(xInc * (ch[i] - ch[0]) / ext_factor), m_RectPulseGraph.Height());
	}

	//// Update window size
	//str.Format(_T("[%3d %3d %3d %3d]"), pLifetime->roiWidth[0], pLifetime->roiWidth[1], pLifetime->roiWidth[2], pLifetime->roiWidth[3]);
	//GetDlgItem(IDC_STATIC_WIN_SIZE)->SetWindowTextW(str);
	//str.Format(_T("[%2.1f %2.1f %2.1f %2.1f] ns"),
	//	(float)pLifetime->roiWidth[0] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR,
	//	(float)pLifetime->roiWidth[1] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR,
	//	(float)pLifetime->roiWidth[2] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR,
	//	(float)pLifetime->roiWidth[3] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR);
	//GetDlgItem(IDC_STATIC_WIN_SEC)->SetWindowTextW(str);

	// Transfering
	dcScope.BitBlt(0, 0, m_RectPulseGraph.Width(), m_RectPulseGraph.Height(), &dcMem, 0, 0, SRCCOPY);
}


void PulseReviewDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_REVIEW_CUR_ALINE);

	if (pScrollBar == (CScrollBar*)pSlider)
	{
		UpdateData(TRUE);

		CString str;
		str.Format(_T("Current A-line : %4d / %d"), 4 * (m_SliderCurAline + 1), m_pVisResult->m_nAlines);
		SetDlgItemText(IDC_STATIC_REVIEW_CUR_ALINE, str);
		InvalidateRect(m_RectPulseGraphWnd, FALSE);

		if (m_pVisResult->m_CheckShowGuideLine)
			m_pVisResult->InvalidateRect(m_pVisResult->m_VisRegionRectWnd, FALSE);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void PulseReviewDlg::OnCbnSelchangeComboTypePulse()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	switch (m_ComboTypePulse)
	{
	case 0:
		m_pSelectedVector = &m_pVisResult->m_PulseCrop;
		break;
	case 1:
		m_pSelectedVector = &m_pVisResult->m_PulseMask;
		break;
	case 2:
//		m_pSelectedVector = &m_pVisResult->m_PulseSpline;
		break;
	}

	InvalidateRect(m_RectPulseGraphWnd, FALSE);
}