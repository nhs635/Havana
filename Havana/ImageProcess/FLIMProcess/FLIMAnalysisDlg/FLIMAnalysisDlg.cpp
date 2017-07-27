// FLIMAnalysisDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Havana.h"
#include "HavanaDlg.h"
#include "FLIMAnalysisDlg.h"
#include "afxdialogex.h"

#include <Tab View\VisStream.h>
#include "ImageProcess\MyBitmap.h"

// FLIMAnalysisDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(FLIMAnalysisDlg, CDialogEx)

FLIMAnalysisDlg::FLIMAnalysisDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_FLIM_ANALYSIS, pParent), m_nBins(50),
	m_pHistogramIntensity(nullptr), m_pHistogramLifetime(nullptr),
	m_DataPen(PS_SOLID, 1, RGB(255, 255, 0))
{
}

FLIMAnalysisDlg::~FLIMAnalysisDlg()
{
}

void FLIMAnalysisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRAPH_INTENSITY, m_GraphIntensity);
	DDX_Control(pDX, IDC_GRAPH_LIFETIME, m_GraphLifetime);
	DDX_Control(pDX, IDC_COLORBAR_INTENSITY, m_ColorbarIntensity);
	DDX_Control(pDX, IDC_COLORBAR_LIFETIME, m_ColorbarLifetime);
}


BEGIN_MESSAGE_MAP(FLIMAnalysisDlg, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// FLIMAnalysisDlg 메시지 처리기입니다.
void FLIMAnalysisDlg::SetMainDlg(CHavanaDlg * pMainDlg)
{
	m_pMainDlg = pMainDlg;
	m_pVisStream = &(m_pMainDlg->m_VisStream);
}


BOOL FLIMAnalysisDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	// Scope 부분 초기화 ////////////////////////////////////////////////////////////////////////////
	CWnd *pWnd = this->GetDlgItem(IDC_GRAPH_INTENSITY);
	pWnd->GetWindowRect(&m_RectGraphIntensity);
	this->ScreenToClient(&m_RectGraphIntensity);

	pWnd = this->GetDlgItem(IDC_GRAPH_LIFETIME);
	pWnd->GetWindowRect(&m_RectGraphLifetime);
	this->ScreenToClient(&m_RectGraphLifetime);

	pWnd = this->GetDlgItem(IDC_COLORBAR_INTENSITY);
	pWnd->GetWindowRect(&m_RectColorbarIntensity);
	this->ScreenToClient(&m_RectColorbarIntensity);

	pWnd = this->GetDlgItem(IDC_COLORBAR_LIFETIME);
	pWnd->GetWindowRect(&m_RectColorbarLifetime);
	this->ScreenToClient(&m_RectColorbarLifetime);

	m_VisRegionRectWnd = CRect(m_RectGraphIntensity.left, m_RectGraphIntensity.top,
		m_RectColorbarLifetime.right, m_RectColorbarLifetime.bottom);

	CPoint pt = { -m_RectGraphIntensity.left, -m_RectGraphIntensity.top };
	m_RectGraphIntensity.OffsetRect(pt);
	m_RectGraphLifetime.OffsetRect(pt);
	m_RectColorbarIntensity.OffsetRect(pt);
	m_RectColorbarLifetime.OffsetRect(pt);

	// Data Plotting
	m_IncrementX = (float)m_RectGraphIntensity.Width() / (float)(m_nBins - 1);
	m_IncrementY = (float)m_RectGraphIntensity.Height() / (float)(m_pVisStream->m_nAlines / 4 - 1);
	m_pHistogramIntensity = new unsigned int[m_nBins]; memset(m_pHistogramIntensity, 0, sizeof(int) * m_nBins);
	m_pHistogramLifetime = new unsigned int[m_nBins]; memset(m_pHistogramLifetime, 0, sizeof(int) * m_nBins);
	histogram_intensity.initialize(m_nBins, m_pVisStream->m_nAlines / 4);
	histogram_lifetime.initialize(m_nBins, m_pVisStream->m_nAlines / 4);
		
	// Static Initialization
	SetDlgItemInt(IDC_STATIC_MAX_INTENSITY, (int)m_pVisStream->m_Max_FLIM);
	SetDlgItemInt(IDC_STATIC_MIN_INTENSITY, (int)m_pVisStream->m_Min_FLIM);
	SetDlgItemInt(IDC_STATIC_MAX_LIFETIME, (int)m_pVisStream->m_Max_FLIM_life);
	SetDlgItemInt(IDC_STATIC_MIN_LIFETIME, (int)m_pVisStream->m_Min_FLIM_life);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void FLIMAnalysisDlg::PostNcDestroy()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (m_pHistogramIntensity) delete m_pHistogramIntensity;
	if (m_pHistogramLifetime) delete m_pHistogramLifetime;
	m_pVisStream->m_pFLIMAnalysisDlg = nullptr;
	delete this;
}


void FLIMAnalysisDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	DestroyWindow();
}


void FLIMAnalysisDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.

	CPaintDC dcGraph1(&m_GraphIntensity);
	CPaintDC dcGraph2(&m_GraphLifetime);
	CPaintDC dcCBar1(&m_ColorbarIntensity);
	CPaintDC dcCBar2(&m_ColorbarLifetime);
	CDC dcMem;

	// Assigning memory DC
	dcMem.CreateCompatibleDC(&dc);
	CBitmap btmp;
	btmp.CreateCompatibleBitmap(&dc, m_VisRegionRectWnd.Width(), m_VisRegionRectWnd.Height());
	dcMem.SelectObject(&btmp);

	// Drawing histograms
	dcMem.FillSolidRect(CRect(0, 0, m_VisRegionRectWnd.Width(), m_RectGraphIntensity.Height()), RGB(0, 0, 0));
	dcMem.SelectObject(m_DataPen);
	for (int i = 0; i < m_nBins - 1; i++) {
		dcMem.MoveTo(int(m_IncrementX * i     ), int(m_IncrementY * (m_pVisStream->m_nAlines / 4 - m_pHistogramIntensity[i]    )));
		dcMem.LineTo(int(m_IncrementX *(i + 1)), int(m_IncrementY * (m_pVisStream->m_nAlines / 4 - m_pHistogramIntensity[i + 1])));
	}
	for (int i = 0; i < m_nBins - 1; i++) {
		dcMem.MoveTo(int(m_RectGraphLifetime.left + m_IncrementX * i     ), int(m_IncrementY * (m_pVisStream->m_nAlines / 4 - m_pHistogramLifetime[i]    )));
		dcMem.LineTo(int(m_RectGraphLifetime.left + m_IncrementX *(i + 1)), int(m_IncrementY * (m_pVisStream->m_nAlines / 4 - m_pHistogramLifetime[i + 1])));
	}

	// Drawing colorbars	
	SetStretchBltMode(dcMem.m_hDC, COLORONCOLOR);
	MyBitmap* pColorbarIntensity = m_pVisStream->m_pColorbarIntensity;
	StretchDIBits(dcMem.m_hDC,
		m_RectColorbarIntensity.left, m_RectColorbarIntensity.top, m_RectColorbarIntensity.Width(), m_RectColorbarIntensity.Height(),
		0, 0, pColorbarIntensity->GetLpBmpInfo()->bmiHeader.biWidth, pColorbarIntensity->GetLpBmpInfo()->bmiHeader.biHeight,
		pColorbarIntensity->GetPtr(), pColorbarIntensity->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
	MyBitmap* pColorbarLifetime = m_pVisStream->m_pColorbarLifetime;
	StretchDIBits(dcMem.m_hDC,
		m_RectColorbarLifetime.left, m_RectColorbarLifetime.top, m_RectColorbarLifetime.Width(), m_RectColorbarLifetime.Height(),
		0, 0, pColorbarLifetime->GetLpBmpInfo()->bmiHeader.biWidth, pColorbarLifetime->GetLpBmpInfo()->bmiHeader.biHeight,
		pColorbarLifetime->GetPtr(), pColorbarLifetime->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

	// Memory DC transfer
	dcGraph1.BitBlt(0, 0, m_RectGraphIntensity.Width(), m_RectGraphIntensity.Height(), &dcMem, m_RectGraphIntensity.left, m_RectGraphIntensity.top, SRCCOPY);
	dcGraph2.BitBlt(0, 0, m_RectGraphLifetime.Width(), m_RectGraphLifetime.Height(), &dcMem, m_RectGraphLifetime.left, m_RectGraphLifetime.top, SRCCOPY);
	dcCBar1.BitBlt(0, 0, m_RectColorbarIntensity.Width(), m_RectColorbarIntensity.Height(), &dcMem, m_RectColorbarIntensity.left, m_RectColorbarIntensity.top, SRCCOPY);
	dcCBar2.BitBlt(0, 0, m_RectColorbarLifetime.Width(), m_RectColorbarLifetime.Height(), &dcMem, m_RectColorbarLifetime.left, m_RectColorbarLifetime.top, SRCCOPY);
}


void FLIMAnalysisDlg::Draw(float* ScanIntensity, float* ScanLifetime)
{
	CString str;

	SetDlgItemInt(IDC_STATIC_MAX_INTENSITY, (int)m_pVisStream->m_Max_FLIM);
	SetDlgItemInt(IDC_STATIC_MIN_INTENSITY, (int)m_pVisStream->m_Min_FLIM);
	SetDlgItemInt(IDC_STATIC_MAX_LIFETIME, (int)m_pVisStream->m_Max_FLIM_life);
	SetDlgItemInt(IDC_STATIC_MIN_LIFETIME, (int)m_pVisStream->m_Min_FLIM_life);

	histogram_intensity(ScanIntensity, m_pHistogramIntensity, &m_pVisStream->m_Min_FLIM, &m_pVisStream->m_Max_FLIM);
	histogram_lifetime(ScanLifetime, m_pHistogramLifetime, &m_pVisStream->m_Min_FLIM_life, &m_pVisStream->m_Max_FLIM_life);

	Ipp32f mean, stdev;
	ippsMeanStdDev_32f(ScanIntensity, m_pVisStream->m_nAlines / 4, &mean, &stdev, ippAlgHintFast);
	str.Format(_T("Mean: %.3f"), mean);
	SetDlgItemText(IDC_STATIC_MEAN_INTENSITY, str);
	str.Format(_T("Stdev: %.3f"), stdev);
	SetDlgItemText(IDC_STATIC_STDEV_INTENSITY, str);
	ippsMeanStdDev_32f(ScanLifetime, m_pVisStream->m_nAlines / 4, &mean, &stdev, ippAlgHintFast);
	str.Format(_T("Mean: %.3f"), mean);
	SetDlgItemText(IDC_STATIC_MEAN_LIFETIME, str);
	str.Format(_T("Stdev: %.3f"), stdev);
	SetDlgItemText(IDC_STATIC_STDEV_LIFETIME, str);

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}