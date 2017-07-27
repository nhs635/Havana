// PulseOverlapDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Havana.h"
#include "HavanaDlg.h"
#include "PulseOverlapDlg.h"
#include "afxdialogex.h"

#include <Tab View\VisStream.h>

// FLIMAnalysisDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(PulseOverlapDlg, CDialogEx)

PulseOverlapDlg::PulseOverlapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PULSE_OVERLAP, pParent),
	m_CheckSeeSequence(false),
	m_CheckShowIRF(true), m_CheckShowCH1(true), m_CheckShowCH2(true), m_CheckShowCH3(true),
	m_CheckShowMeanDelay(false), m_CheckShowWin(false),
	m_CheckSoftwareBroadening(false), m_CheckSplineInterp(true), m_CheckNormalize(false),
	m_CheckMask(false), m_ToggleMaskModification(false),
	m_GridPen(PS_SOLID, 1, RGB(0, 64, 0)),
	m_IrfPen(PS_SOLID, 1, RGB(255, 255, 255)), m_Ch1Pen(PS_SOLID, 1, RGB(200, 5, 240)),
	m_Ch2Pen(PS_SOLID, 1, RGB(0, 255, 255)), m_Ch3Pen(PS_SOLID, 1, RGB(0, 255, 0)),
	m_MeanDelayPen(PS_SOLID, 1, RGB(255, 0, 0)), m_WindowPen(PS_SOLID, 1, RGB(255, 255, 0)),
	m_MaskPen(PS_SOLID, 1, RGB(0, 255, 0)), m_MaskSelectPen(PS_SOLID, 3, RGB(0, 150, 0))
{
}

PulseOverlapDlg::~PulseOverlapDlg()
{
}

void PulseOverlapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PULSE_GRAPH, m_StaticPulseGraph);
	DDX_Check(pDX, IDC_CHECK_SHOW_IRF, m_CheckShowIRF);
	DDX_Check(pDX, IDC_CHECK_SHOW_CH1, m_CheckShowCH1);
	DDX_Check(pDX, IDC_CHECK_SHOW_CH2, m_CheckShowCH2);
	DDX_Check(pDX, IDC_CHECK_SHOW_CH3, m_CheckShowCH3);
	DDX_Check(pDX, IDC_CHECK_SHOW_MEAN_DELAY, m_CheckShowMeanDelay);
	DDX_Check(pDX, IDC_CHECK_SHOW_WIN, m_CheckShowWin);
	DDX_Check(pDX, IDC_CHECK_SPLINE_INTERP, m_CheckSplineInterp);
	DDX_Check(pDX, IDC_CHECK_NORMALIZE, m_CheckNormalize);
	DDX_Check(pDX, IDC_CHECK_BROADENING, m_CheckSoftwareBroadening);
	DDX_Check(pDX, IDC_CHECK_SEE_SEQUENCE, m_CheckSeeSequence);
	DDX_Check(pDX, IDC_CHECK_SHOW_MASK, m_CheckMask);
	DDX_Check(pDX, IDC_TOGGLE_MODIFY_MASK, m_ToggleMaskModification);
}


BEGIN_MESSAGE_MAP(PulseOverlapDlg, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_SHOW_IRF, &PulseOverlapDlg::OnBnClickedCheckShowIrf)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CH1, &PulseOverlapDlg::OnBnClickedCheckShowCh1)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CH2, &PulseOverlapDlg::OnBnClickedCheckShowCh2)
	ON_BN_CLICKED(IDC_CHECK_SHOW_CH3, &PulseOverlapDlg::OnBnClickedCheckShowCh3)
	ON_BN_CLICKED(IDC_CHECK_SHOW_MEAN_DELAY, &PulseOverlapDlg::OnBnClickedCheckShowMeanDelay)
	ON_BN_CLICKED(IDC_CHECK_SHOW_WIN, &PulseOverlapDlg::OnBnClickedCheckShowWin)
	ON_BN_CLICKED(IDC_CHECK_SPLINE_INTERP, &PulseOverlapDlg::OnBnClickedCheckSplineInterp)
	ON_BN_CLICKED(IDC_CHECK_NORMALIZE, &PulseOverlapDlg::OnBnClickedCheckNormalize)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_CHECK_BROADENING, &PulseOverlapDlg::OnBnClickedCheckBroadening)
	ON_BN_CLICKED(IDC_CHECK_SEE_SEQUENCE, &PulseOverlapDlg::OnBnClickedCheckSeeSequence)
	ON_BN_CLICKED(IDC_CHECK_SHOW_MASK, &PulseOverlapDlg::OnBnClickedCheckShowMask)
	ON_BN_CLICKED(IDC_TOGGLE_MODIFY_MASK, &PulseOverlapDlg::OnBnClickedCheckModifyMask)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_ADD, &PulseOverlapDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &PulseOverlapDlg::OnBnClickedButtonRemove)
END_MESSAGE_MAP()


// FLIMAnalysisDlg �޽��� ó�����Դϴ�.
void PulseOverlapDlg::SetMainDlg(CHavanaDlg * pMainDlg)
{
	m_pMainDlg = pMainDlg;
	m_pVisStream = &(m_pMainDlg->m_VisStream);
}


BOOL PulseOverlapDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	CWnd *pWnd = this->GetDlgItem(IDC_STATIC_PULSE_GRAPH);
	pWnd->GetWindowRect(&m_RectPulseGraphWnd);
	this->ScreenToClient(&m_RectPulseGraphWnd);
	pt = { -m_RectPulseGraphWnd.left, -m_RectPulseGraphWnd.top };
	m_RectPulseGraph = m_RectPulseGraphWnd;
	m_RectPulseGraph.OffsetRect(pt);

	// Set Range
	CString str;
	str.Format(_T("%d"), (int)MAX_VAL);
	GetDlgItem(IDC_STATIC_MAX_VAL)->SetWindowTextW(str);
	str.Format(_T("%d"), (int)MIN_VAL);
	GetDlgItem(IDC_STATIC_MIN_VAL)->SetWindowTextW(str);

	// For setting color
	SetWindowTheme(GetDlgItem(IDC_CHECK_SEE_SEQUENCE)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SHOW_IRF)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SHOW_CH1)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SHOW_CH2)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SHOW_CH3)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SHOW_MEAN_DELAY)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SHOW_WIN)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_BROADENING)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_SPLINE_INTERP)->m_hWnd, L"", L"");
	SetWindowTheme(GetDlgItem(IDC_CHECK_NORMALIZE)->m_hWnd, L"", L"");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}


void PulseOverlapDlg::PostNcDestroy()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (m_pVisStream->m_pPulseOverlapDlg != nullptr)
		m_pVisStream->m_pPulseOverlapDlg = nullptr;
	delete this;
}


void PulseOverlapDlg::OnCancel()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	DestroyWindow();
}


void PulseOverlapDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
					   // �׸��� �޽����� ���ؼ��� CDialogEx::OnPaint()��(��) ȣ������ ���ʽÿ�.

	CPaintDC dcScope(&m_StaticPulseGraph);
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
	dcMem.MoveTo(0                       , int(m_RectPulseGraph.Height() * MAX_VAL / (MAX_VAL - MIN_VAL)));
	dcMem.LineTo(m_RectPulseGraph.Width(), int(m_RectPulseGraph.Height() * MAX_VAL / (MAX_VAL - MIN_VAL)));

	// Draw graphs
	RESIZE* pResize = &m_pMainDlg->m_pFLIM->_resize;
	LIFETIME* pLifetime = &m_pMainDlg->m_pFLIM->_lifetime;
	FLIM_PARAMS* pParams = &m_pVisStream->m_Params_FLIM;	
	CString str;

	if (m_CheckSeeSequence)
	{		
		int xLen = (m_CheckSplineInterp) ? pResize->nsite : pResize->nx;

		float xInc = float(m_RectPulseGraph.Width()) / float(xLen - 1);
		float yInc = float(m_RectPulseGraph.Height()) / float(MAX_VAL - MIN_VAL);

		str.Format(_T("%d"), xLen);
		GetDlgItem(IDC_STATIC_NUM_SAMPLES)->SetWindowTextW(str);
		
		int offset = m_pVisStream->m_SliderCurAline / 4;
		float* data = (m_CheckSplineInterp) ? 
			(m_CheckSoftwareBroadening ? &pResize->filt_src(0, offset) : &pResize->ext_src(0, offset)) : &pResize->crop_src(0, offset);
		float maxval;

		if (data)
		{			
			dcMem.SelectObject(&m_IrfPen);
			ippsMax_32f(data, xLen, &maxval);
			maxval = m_CheckNormalize ? MAX_VAL / maxval : 1;
									
			for (int i = 0; i < xLen - 1; i++)
			{
				if (m_CheckMask)
				{
					if (pResize->pMask[i])
						dcMem.SelectObject(&m_IrfPen);
					else
						dcMem.SelectObject(&m_MaskPen);
				}

				dcMem.MoveTo(int(xInc * (i)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data[i]) - MIN_VAL)));
				dcMem.LineTo(int(xInc * (i + 1)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data[i + 1]) - MIN_VAL)));
			}

			if (m_ToggleMaskModification)
			{
				dcMem.SelectObject(&m_MaskSelectPen);

				if (m_start < m_end)
				{
					for (int i = m_start; i < m_end; i++)
					{
						dcMem.MoveTo(int(xInc * (i)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data[i]) - MIN_VAL)));
						dcMem.LineTo(int(xInc * (i + 1)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data[i + 1]) - MIN_VAL)));
					}
				}
				else
				{
					for (int i = m_end; i < m_start; i++)
					{
						dcMem.MoveTo(int(xInc * (i)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data[i]) - MIN_VAL)));
						dcMem.LineTo(int(xInc * (i + 1)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data[i + 1]) - MIN_VAL)));
					}
				}
			}
						
			int* ch_ind = (m_CheckSplineInterp) ? pResize->ch_start_ind1 : pParams->ch_start_ind;
			if (m_CheckShowWin) 
			{
				dcMem.SelectObject(&m_WindowPen);
				for (int i = 0; i < 4; i++) {
					dcMem.MoveTo(int(xInc * (ch_ind[i] - ch_ind[0])), 0);
					dcMem.LineTo(int(xInc * (ch_ind[i] - ch_ind[0])), m_RectPulseGraph.Height());
				}
			}

			if (m_CheckShowMeanDelay)
			{
				dcMem.SelectObject(&m_MeanDelayPen);
				float* scan_mean_delay = m_pMainDlg->m_FluMeanDelay.raw_ptr();
				float scale = (m_CheckSplineInterp) ? pResize->ActualFactor : 1.0f;
				for (int i = 0; i < 4; i++) {
					dcMem.MoveTo(int(xInc * (scale * scan_mean_delay[offset + i * m_pMainDlg->m_Edit_nAlines / 4] - ch_ind[0])), 0);
					dcMem.LineTo(int(xInc * (scale * scan_mean_delay[offset + i * m_pMainDlg->m_Edit_nAlines / 4] - ch_ind[0])), m_RectPulseGraph.Height());
				}
			}
		}
	}
	else
	{
		int xLen_temp = (int)round((float)pResize->pulse_roi_length / (float)pResize->upSampleFactor);
		int xLen = (m_CheckSplineInterp) ? xLen_temp * pResize->upSampleFactor : xLen_temp;

		float xIncTemp = float(m_RectPulseGraph.Width()) / float(xLen_temp - 1);
		float xInc = (m_CheckSplineInterp) ? xIncTemp / (float)pResize->ActualFactor : xIncTemp;
		float yInc = float(m_RectPulseGraph.Height()) / float(MAX_VAL - MIN_VAL);

		str.Format(_T("%d"), xLen);
		GetDlgItem(IDC_STATIC_NUM_SAMPLES)->SetWindowTextW(str);

		float* data = (m_CheckSplineInterp) ? (m_CheckSoftwareBroadening ?
			&pResize->filt_src(0, m_pVisStream->m_SliderCurAline / 4) : &pResize->ext_src(0, m_pVisStream->m_SliderCurAline / 4))
			: &pResize->crop_src(0, m_pVisStream->m_SliderCurAline / 4);
		float maxval;

		if (data)
		{
			if (m_CheckShowIRF)
			{
				dcMem.SelectObject(&m_IrfPen);
				float* data1 = data;
				ippsMax_32f(data1, xLen, &maxval);
				maxval = m_CheckNormalize ? MAX_VAL / maxval : 1;

				for (int i = 0; i < xLen - 1; i++)
				{
					dcMem.MoveTo(int(xInc * (i)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i]) - MIN_VAL)));
					dcMem.LineTo(int(xInc * (i + 1)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i + 1]) - MIN_VAL)));
				}
			}

			if (m_CheckShowCH1)
			{
				dcMem.SelectObject(&m_Ch1Pen);
				int index_offset = (m_CheckSplineInterp) ? (pResize->ch_start_ind1[1] - pResize->ch_start_ind1[0]) : (pParams->ch_start_ind[1] - pParams->ch_start_ind[0]);
				float* data1 = data + index_offset;
				float time_offset = (m_CheckSplineInterp) ? pParams->delay_offset[0] / (pParams->samp_intv / pResize->ActualFactor) - index_offset :
					pParams->delay_offset[0] / pParams->samp_intv - index_offset;

				ippsMax_32f(data1, xLen, &maxval);
				maxval = m_CheckNormalize ? MAX_VAL / maxval : 1;

				for (int i = 0; i < xLen - 1; i++)
				{
					dcMem.MoveTo(int(xInc * (i - time_offset)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i]) - MIN_VAL)));
					dcMem.LineTo(int(xInc * (i + 1 - time_offset)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i + 1]) - MIN_VAL)));
				}
			}

			if (m_CheckShowCH2)
			{
				dcMem.SelectObject(&m_Ch2Pen);
				int index_offset = (m_CheckSplineInterp) ? (pResize->ch_start_ind1[2] - pResize->ch_start_ind1[0]) : (pParams->ch_start_ind[2] - pParams->ch_start_ind[0]);
				float* data1 = data + index_offset;
				float time_offset = (m_CheckSplineInterp) ? pParams->delay_offset[1] / (pParams->samp_intv / pResize->ActualFactor) - index_offset :
					pParams->delay_offset[1] / pParams->samp_intv - index_offset;

				ippsMax_32f(data1, xLen, &maxval);
				maxval = m_CheckNormalize ? MAX_VAL / maxval : 1;

				for (int i = 0; i < xLen - 1; i++)
				{
					dcMem.MoveTo(int(xInc * (i - time_offset)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i]) - MIN_VAL)));
					dcMem.LineTo(int(xInc * (i + 1 - time_offset)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i + 1]) - MIN_VAL)));
				}
			}

			if (m_CheckShowCH3)
			{
				dcMem.SelectObject(&m_Ch3Pen);
				int index_offset = (m_CheckSplineInterp) ? (pResize->ch_start_ind1[3] - pResize->ch_start_ind1[0]) : (pParams->ch_start_ind[3] - pParams->ch_start_ind[0]);
				float* data1 = data + index_offset;
				float time_offset = (m_CheckSplineInterp) ? pParams->delay_offset[2] / (pParams->samp_intv / pResize->ActualFactor) - index_offset :
					pParams->delay_offset[2] / pParams->samp_intv - index_offset;

				ippsMax_32f(data1, xLen, &maxval);
				maxval = m_CheckNormalize ? MAX_VAL / maxval : 1;

				for (int i = 0; i < xLen - 1; i++)
				{
					dcMem.MoveTo(int(xInc * (i - time_offset)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i]) - MIN_VAL)));
					dcMem.LineTo(int(xInc * (i + 1 - time_offset)), m_RectPulseGraph.Height() - int(yInc * (maxval*(data1[i + 1]) - MIN_VAL)));
				}
			}
		}
	}

	// Update window size
	str.Format(_T("[%3d %3d %3d %3d]"), pLifetime->roiWidth[0], pLifetime->roiWidth[1], pLifetime->roiWidth[2], pLifetime->roiWidth[3]);
	GetDlgItem(IDC_STATIC_WIN_SIZE)->SetWindowTextW(str);
	str.Format(_T("[%2.1f %2.1f %2.1f %2.1f] ns"), 
		(float)pLifetime->roiWidth[0] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR, 
		(float)pLifetime->roiWidth[1] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR,
		(float)pLifetime->roiWidth[2] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR,
		(float)pLifetime->roiWidth[3] * pParams->samp_intv / (float)FLIM_SPLINE_FACTOR);
	GetDlgItem(IDC_STATIC_WIN_SEC)->SetWindowTextW(str);

	// Transfering
	dcScope.BitBlt(0, 0, m_RectPulseGraph.Width(), m_RectPulseGraph.Height(), &dcMem, 0, 0, SRCCOPY);
}


void PulseOverlapDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (m_ToggleMaskModification)
	{
		CPoint pt1 = point, pt2;
		pt1.x = pt1.x + pt.x;
		pt1.y = pt1.y + pt.y;

		if (m_RectPulseGraph.PtInRect(pt1))
		{
			m_LButtonClicked = TRUE;
			RESIZE* pResize = &m_pMainDlg->m_pFLIM->_resize;
			pt2 = pt1;
			pt2.x = (LONG)floor(double(pResize->nx * pt2.x) / double(m_RectPulseGraph.Width()));

			m_start = pt2.x;
			printf("[%d %d]\n", m_start, 0);
		}
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void PulseOverlapDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (m_ToggleMaskModification)
	{
		CPoint pt1 = point, pt2;
		pt1.x = pt1.x + pt.x;
		pt1.y = pt1.y + pt.y;

		if (m_RectPulseGraph.PtInRect(pt1))
		{
			if (m_LButtonClicked)
			{
				RESIZE* pResize = &m_pMainDlg->m_pFLIM->_resize;
				pt2 = pt1;
				pt2.x = (LONG)floor(double(pResize->nx * pt2.x) / double(m_RectPulseGraph.Width()));
								
				m_end = pt2.x;
				Invalidate(FALSE);
				printf("[%d %d]\n", m_start, m_end);
			}
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void PulseOverlapDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (m_ToggleMaskModification)
	{
		CPoint pt1 = point, pt2;
		pt1.x = pt1.x + pt.x;
		pt1.y = pt1.y + pt.y;

		if (m_RectPulseGraph.PtInRect(pt1))
		{
			m_LButtonClicked = FALSE;
			RESIZE* pResize = &m_pMainDlg->m_pFLIM->_resize;
			pt2 = pt1;
			pt2.x = (LONG)floor(double(pResize->nx * pt2.x) / double(m_RectPulseGraph.Width()));

			m_end = pt2.x;
			Invalidate(FALSE);
			printf("[%d %d]\n", m_start, m_end);
		}
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


void PulseOverlapDlg::OnBnClickedCheckSeeSequence()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	GetDlgItem(IDC_CHECK_SHOW_IRF)->EnableWindow(!m_CheckSeeSequence);
	GetDlgItem(IDC_CHECK_SHOW_CH1)->EnableWindow(!m_CheckSeeSequence);
	GetDlgItem(IDC_CHECK_SHOW_CH2)->EnableWindow(!m_CheckSeeSequence);
	GetDlgItem(IDC_CHECK_SHOW_CH3)->EnableWindow(!m_CheckSeeSequence);

	GetDlgItem(IDC_CHECK_SHOW_MEAN_DELAY)->EnableWindow(m_CheckSeeSequence);
	GetDlgItem(IDC_CHECK_SHOW_WIN)->EnableWindow(m_CheckSeeSequence);

	if (m_CheckSeeSequence && !m_CheckSplineInterp && !m_CheckSoftwareBroadening)
	{
		GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_TOGGLE_MODIFY_MASK)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_TOGGLE_MODIFY_MASK)->EnableWindow(FALSE);
	}

	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowIrf()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowCh1()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowCh2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowCh3()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowMeanDelay()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowWin()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckBroadening()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if (m_CheckSeeSequence && !m_CheckSplineInterp && !m_CheckSoftwareBroadening)
	{
		GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_TOGGLE_MODIFY_MASK)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_TOGGLE_MODIFY_MASK)->EnableWindow(FALSE);
	}

	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckSplineInterp()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if (m_CheckSeeSequence && !m_CheckSplineInterp && !m_CheckSoftwareBroadening)
	{
		GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(TRUE);
		GetDlgItem(IDC_TOGGLE_MODIFY_MASK)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(FALSE);
		GetDlgItem(IDC_TOGGLE_MODIFY_MASK)->EnableWindow(FALSE);
	}

	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckNormalize()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckShowMask()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedCheckModifyMask()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	GetDlgItem(IDC_CHECK_SEE_SEQUENCE)->EnableWindow(!m_ToggleMaskModification);

	GetDlgItem(IDC_CHECK_SHOW_IRF)->EnableWindow(!m_ToggleMaskModification);
	GetDlgItem(IDC_CHECK_SHOW_CH1)->EnableWindow(!m_ToggleMaskModification);
	GetDlgItem(IDC_CHECK_SHOW_CH2)->EnableWindow(!m_ToggleMaskModification);
	GetDlgItem(IDC_CHECK_SHOW_CH3)->EnableWindow(!m_ToggleMaskModification);

	GetDlgItem(IDC_CHECK_BROADENING)->EnableWindow(!m_ToggleMaskModification);
	GetDlgItem(IDC_CHECK_SPLINE_INTERP)->EnableWindow(!m_ToggleMaskModification);
	GetDlgItem(IDC_CHECK_NORMALIZE)->EnableWindow(!m_ToggleMaskModification);

	GetDlgItem(IDC_CHECK_SHOW_MEAN_DELAY)->EnableWindow(!m_ToggleMaskModification);
	GetDlgItem(IDC_CHECK_SHOW_WIN)->EnableWindow(!m_ToggleMaskModification);

	GetDlgItem(IDC_CHECK_SHOW_MASK)->EnableWindow(!m_ToggleMaskModification);
	
	m_CheckMask = m_ToggleMaskModification;	
	UpdateData(FALSE);

	GetDlgItem(IDC_BUTTON_ADD)->EnableWindow(m_ToggleMaskModification);
	GetDlgItem(IDC_BUTTON_REMOVE)->EnableWindow(m_ToggleMaskModification);

	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedButtonAdd()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	RESIZE* pResize = &m_pMainDlg->m_pFLIM->_resize;
	
	if (m_start < m_end)
	{
		for (int i = m_start; i < m_end; i++)
			pResize->pMask[i] = 0.0f;
	}
	else
	{
		for (int i = m_end; i < m_start; i++)
			pResize->pMask[i] = 0.0f;
	}

	memset(pResize->start_ind, 0, sizeof(int) * 4);
	memset(pResize->end_ind, 0, sizeof(int) * 4);

	int start_count = 0, end_count = 0;
	for (int i = 0; i < pResize->nx - 1; i++)
	{
		if (pResize->pMask[i + 1] - pResize->pMask[i] == -1)
		{
			start_count++;
			if (start_count < 5)
				pResize->start_ind[start_count - 1] = i - 1;
		}
		if (pResize->pMask[i + 1] - pResize->pMask[i] == 1)
		{
			end_count++;
			if (end_count < 5)
				pResize->end_ind[end_count - 1] = i;
		}
	}
	
	for (int i = 0; i < 4; i++)
		printf("mask %d: [%d %d]\n", i + 1, pResize->start_ind[i], pResize->end_ind[i]);

	if ((start_count == 4) && (end_count == 4))
		printf("Proper mask is selected!!");
	else
		printf("Improper mask: please modify the mask!");

	m_start = 0; m_end = 0;
	Invalidate(FALSE);
}


void PulseOverlapDlg::OnBnClickedButtonRemove()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	RESIZE* pResize = &m_pMainDlg->m_pFLIM->_resize;

	if (m_start < m_end)
	{
		for (int i = m_start; i < m_end; i++)
			pResize->pMask[i] = 1.0f;
	}
	else
	{
		for (int i = m_end; i < m_start; i++)
			pResize->pMask[i] = 1.0f;
	}

	memset(pResize->start_ind, 0, sizeof(int) * 4);
	memset(pResize->end_ind, 0, sizeof(int) * 4);

	int start_count = 0, end_count = 0;
	for (int i = 0; i < pResize->nx - 1; i++)
	{
		if (pResize->pMask[i + 1] - pResize->pMask[i] == -1)
		{
			start_count++;
			if (start_count < 5)
				pResize->start_ind[start_count - 1] = i - 1;
		}
		if (pResize->pMask[i + 1] - pResize->pMask[i] == 1)
		{
			end_count++;
			if (end_count < 5)
				pResize->end_ind[end_count - 1] = i;
		}
	}

	for (int i = 0; i < 4; i++)
		printf("mask %d: [%d %d]\n", i + 1, pResize->start_ind[i], pResize->end_ind[i]);

	if ((start_count == 4) && (end_count == 4))
		printf("Proper mask is selected!!\n");
	else
		printf("Improper mask: please modify the mask!\n");

	m_start = 0; m_end = 0;
	Invalidate(FALSE);
}


HBRUSH PulseOverlapDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  ���⼭ DC�� Ư���� �����մϴ�.
	if (pWnd->GetDlgCtrlID() == IDC_CHECK_SHOW_IRF) {
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkColor(RGB(0, 0, 0));
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	if (pWnd->GetDlgCtrlID() == IDC_CHECK_SHOW_CH1) {
		pDC->SetTextColor(RGB(200, 5, 240));
		pDC->SetBkColor(RGB(0, 0, 0));
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	if (pWnd->GetDlgCtrlID() == IDC_CHECK_SHOW_CH2) {
		pDC->SetTextColor(RGB(0, 255, 255));
		pDC->SetBkColor(RGB(0, 0, 0));
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	if (pWnd->GetDlgCtrlID() == IDC_CHECK_SHOW_CH3) {
		pDC->SetTextColor(RGB(0, 255, 0));
		pDC->SetBkColor(RGB(0, 0, 0));
		return (HBRUSH)::GetStockObject(NULL_BRUSH);
	}
	
	return hbr;
}



