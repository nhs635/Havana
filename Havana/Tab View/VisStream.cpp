// VisStream.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisStream.h"
#include "afxdialogex.h"

#include "Havana.h"
#include "HavanaDlg.h"
#include "ImageProcess\MyBitmap.h"
#include "ImageProcess\OCTProcess\CalibrationDlg\CalibrationDlg.h"
#include "ImageProcess\FLIMProcess\FLIMAnalysisDlg\FLIMAnalysisDlg.h"
#include "ImageProcess\FLIMProcess\PulseOverlappedDlg\PulseOverlapDlg.h"


// CVisStream 대화 상자입니다.

IMPLEMENT_DYNAMIC(CVisStream, CDialogEx)

CVisStream::CVisStream(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TAB_STREAM, pParent), _running(false),
	m_pBitmapOCT(nullptr), m_pBitmapCircOCT(nullptr),
	m_pBitmapIntensity(nullptr), m_pBitmapLifetime(nullptr),
	m_pColorbarIntensity(nullptr), m_pColorbarLifetime(nullptr), 
	m_pCalibrationDlg(nullptr), m_pFLIMAnalysisDlg(nullptr), m_pPulseOverlapDlg(nullptr),
	m_dGridCount(20.0f), m_dCountPerGrid(4.0f), 
	m_Max_FLIM(MAX_INTENSITY), m_Min_FLIM(MIN_INTENSITY), m_Max_FLIM_life(MAX_LIFETIME), m_Min_FLIM_life(MIN_LIFETIME),
	m_ToggleViewMode(true), m_CheckFLIMShowWindow(false), m_CheckFLIMSoftwareBroadening(false), m_CheckCircularize(false),
	m_ComboFLIMShow(0), m_SliderCurAline(0), m_EditCircCenter(0)
{
	m_MainGridPen.CreatePen(PS_SOLID, 1, RGB(0, 160, 0));
	m_SubGridPen.CreatePen(PS_SOLID, 1, RGB(0, 64, 0));
	m_DataPen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	m_WindowPen.CreatePen(PS_SOLID, 2, RGB(255, 255, 0));
	m_MeanDelayPen.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
}

CVisStream::~CVisStream()
{
}

void CVisStream::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRAPH_REGION1, m_StaticGraphRegion[0]);
	DDX_Control(pDX, IDC_GRAPH_REGION2, m_StaticGraphRegion[1]);
	DDX_Control(pDX, IDC_GRAPH_REGION3, m_StaticGraphRegion[2]);
	DDX_Control(pDX, IDC_IMAGE_REGION, m_StaticImageRegion);
	DDX_Control(pDX, IDC_STATIC_COLORBAR_INTENSITY, m_StaticCBarRegion1);
	DDX_Control(pDX, IDC_STATIC_COLORBAR_LIFETIME, m_StaticCBarRegion2);

	DDX_Check(pDX, IDC_TOGGLE_VIEW_MODE, m_ToggleViewMode);
	DDX_Slider(pDX, IDC_SLIDER_CUR_ALINE, m_SliderCurAline);

	DDX_Check(pDX, IDC_CHECK_FLIM_SHOW_WINDOW, m_CheckFLIMShowWindow);
	DDX_Check(pDX, IDC_CHECK_FLIM_SOFTWARE_BROADENING, m_CheckFLIMSoftwareBroadening);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_START_CH0, m_Params_FLIM.ch_start[0]);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_START_CH1, m_Params_FLIM.ch_start[1]);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_START_CH2, m_Params_FLIM.ch_start[2]);
	DDX_Text(pDX, IDC_EDIT_CHANNEL_START_CH3, m_Params_FLIM.ch_start[3]);
	DDX_Text(pDX, IDC_EDIT_DELAY_OFFSET_CH1, m_Params_FLIM.delay_offset[0]);
	DDX_Text(pDX, IDC_EDIT_DELAY_OFFSET_CH2, m_Params_FLIM.delay_offset[1]);
	DDX_Text(pDX, IDC_EDIT_DELAY_OFFSET_CH3, m_Params_FLIM.delay_offset[2]);

	DDX_Text(pDX, IDC_EDIT_VIS_FLIM_SAMPS, m_nScansFLIM);
	DDX_Text(pDX, IDC_EDIT_FLIM_BACKGROUND, m_Params_FLIM.bg);
	DDX_Text(pDX, IDC_EDIT_FLIM_WIDTH, m_Params_FLIM.width_factor);
	DDX_Text(pDX, IDC_EDIT_FLIM_CHANNEL, m_Params_FLIM.act_ch);
	DDX_CBIndex(pDX, IDC_COMBO_FLIM_SHOW, m_ComboFLIMShow);

	//DDX_Text(pDX, IDC_EDIT_FLIM_MAX, m_Max_FLIM);
	//DDX_Text(pDX, IDC_EDIT_FLIM_MIN, m_Min_FLIM);

	DDX_Check(pDX, IDC_CHECK_CIRCULARIZE, m_CheckCircularize);
	DDX_Check(pDX, IDC_CHECK_BG_REMOVED_FRINGE, m_CheckBgRemovedFringe);
	DDX_CBIndex(pDX, IDC_COMBO_LUT, m_ComboLUT);

	DDX_Text(pDX, IDC_EDIT_ALINE_MAX_DB, m_Max_dB);
	DDX_Text(pDX, IDC_EDIT_ALINE_MIN_DB, m_Min_dB);
	DDX_Text(pDX, IDC_EDIT_CIRC_CENTER, m_EditCircCenter);
}

BEGIN_MESSAGE_MAP(CVisStream, CDialogEx)
	ON_WM_PAINT() 
	ON_BN_CLICKED(IDC_TOGGLE_VIEW_MODE, &CVisStream::OnBnClickedToggleViewMode)

	ON_BN_CLICKED(IDC_CHECK_FLIM_SHOW_WINDOW, &CVisStream::OnBnClickedCheckFlimShowWindow)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CHANNEL_START_CH0, &CVisStream::OnDeltaposSpinChannelStartCh0)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CHANNEL_START_CH1, &CVisStream::OnDeltaposSpinChannelStartCh1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CHANNEL_START_CH2, &CVisStream::OnDeltaposSpinChannelStartCh2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_CHANNEL_START_CH3, &CVisStream::OnDeltaposSpinChannelStartCh3)
	ON_EN_CHANGE(IDC_EDIT_DELAY_OFFSET_CH1, &CVisStream::OnEnChangeEditDelayOffsetCh)
	ON_EN_CHANGE(IDC_EDIT_DELAY_OFFSET_CH2, &CVisStream::OnEnChangeEditDelayOffsetCh)
	ON_EN_CHANGE(IDC_EDIT_DELAY_OFFSET_CH3, &CVisStream::OnEnChangeEditDelayOffsetCh)

	ON_EN_CHANGE(IDC_EDIT_VIS_FLIM_SAMPS, &CVisStream::OnEnChangeEditVisFlimSamps)
	ON_EN_CHANGE(IDC_EDIT_FLIM_BACKGROUND, &CVisStream::OnEnChangeEditFlimBackground)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_FLIM_BACKGROUND, &CVisStream::OnBnClickedButtonCaptureFlimBackground)
	ON_EN_CHANGE(IDC_EDIT_FLIM_WIDTH, &CVisStream::OnEnChangeEditFlimWidth)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_FLIM_CHANNEL, &CVisStream::OnDeltaposSpinFlimChannel)
	ON_CBN_SELCHANGE(IDC_COMBO_FLIM_SHOW, &CVisStream::OnCbnSelchangeComboFlimShow)
	ON_BN_CLICKED(IDC_BUTTON_FLIM_ANALYSIS, &CVisStream::OnBnClickedButtonFlimAnalysis)

	ON_EN_CHANGE(IDC_EDIT_FLIM_MAX, &CVisStream::OnEnChangeEditFlimMax)
	ON_EN_CHANGE(IDC_EDIT_FLIM_MIN, &CVisStream::OnEnChangeEditFlimMin)
	ON_EN_CHANGE(IDC_EDIT_FLIM_MAX_LIFE, &CVisStream::OnEnChangeEditFlimMaxLife)
	ON_EN_CHANGE(IDC_EDIT_FLIM_MIN_LIFE, &CVisStream::OnEnChangeEditFlimMinLife)

	ON_BN_CLICKED(IDC_CHECK_BG_REMOVED_FRINGE, &CVisStream::OnBnClickedCheckBgRemovedFringe)
	ON_BN_CLICKED(IDC_CHECK_CIRCULARIZE, &CVisStream::OnBnClickedCheckCircularize)
	ON_BN_CLICKED(IDC_BUTTON_CALIBRATION, &CVisStream::OnBnClickedButtonCalibration)
	ON_CBN_SELCHANGE(IDC_COMBO_LUT, &CVisStream::OnCbnSelchangeComboLut)

	ON_EN_CHANGE(IDC_EDIT_ALINE_MAX_DB, &CVisStream::OnEnChangeEditAlineMaxDb)
	ON_EN_CHANGE(IDC_EDIT_ALINE_MIN_DB, &CVisStream::OnEnChangeEditAlineMinDb)
	ON_EN_CHANGE(IDC_EDIT_CIRC_CENTER, &CVisStream::OnEnChangeEditCircCenter)

	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()

	ON_BN_CLICKED(IDC_BUTTON_FLIM_PULSE_OVERLAPPED_VIEW, &CVisStream::OnBnClickedButtonFlimPulseOverlappedView)
	ON_BN_CLICKED(IDC_CHECK_FLIM_SOFTWARE_BROADENING, &CVisStream::OnBnClickedCheckFlimSoftwareBroadening)
END_MESSAGE_MAP()


// CVisStream 메시지 처리기입니다.

void CVisStream::SetMainDlg(CHavanaDlg* pMainDlg)
{
	m_pMainDlg = pMainDlg;
		
	// Size variables initialization
	m_nScans = m_pMainDlg->m_Edit_nScans;
	m_nScansFFT = m_pMainDlg->m_Edit_nFFT;
	m_nAlines = m_pMainDlg->m_Edit_nAlines;

	m_nSizeFrame = m_nScans * m_nAlines;
	m_nSizeImageOCT = m_nScansFFT * m_nAlines;

	circ_OCT = circularize(m_nScans / 2, m_nAlines);
}


BOOL CVisStream::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	// Scope 부분 초기화 ////////////////////////////////////////////////////////////////////////////
	CWnd *pWnd = this->GetDlgItem(IDC_GRAPH_REGION1);
	pWnd->GetWindowRect(&m_GraphRegionRect[0]);
	this->ScreenToClient(&m_GraphRegionRect[0]);

	pWnd = this->GetDlgItem(IDC_GRAPH_REGION2);
	pWnd->GetWindowRect(&m_GraphRegionRect[1]);
	this->ScreenToClient(&m_GraphRegionRect[1]);

	pWnd = this->GetDlgItem(IDC_GRAPH_REGION3);
	pWnd->GetWindowRect(&m_GraphRegionRect[2]);
	this->ScreenToClient(&m_GraphRegionRect[2]);

	pWnd = this->GetDlgItem(IDC_IMAGE_REGION);
	pWnd->GetWindowRect(&m_ImageRegionRect);
	this->ScreenToClient(&m_ImageRegionRect);

	pWnd = this->GetDlgItem(IDC_STATIC_COLORBAR_INTENSITY);
	pWnd->GetWindowRect(&m_CBarRegionRect1);
	this->ScreenToClient(&m_CBarRegionRect1);

	pWnd = this->GetDlgItem(IDC_STATIC_COLORBAR_LIFETIME);
	pWnd->GetWindowRect(&m_CBarRegionRect2);
	this->ScreenToClient(&m_CBarRegionRect2);

	m_VisRegionRectWnd = CRect(m_GraphRegionRect[0].left, m_GraphRegionRect[0].top,
								m_ImageRegionRect.right, m_ImageRegionRect.bottom);

	pt = { -m_GraphRegionRect[0].left, -m_GraphRegionRect[0].top };
	m_GraphRegionRect[0].OffsetRect(pt);
	m_GraphRegionRect[1].OffsetRect(pt);
	m_GraphRegionRect[2].OffsetRect(pt);
	m_ImageRegionRect.OffsetRect(pt);
	m_CBarRegionRect1.OffsetRect(pt);
	m_CBarRegionRect2.OffsetRect(pt);

	// Control Initialization
	m_Params_FLIM.act_ch = 0;
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_ALINE))->SetRange(0, m_nAlines - 1);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_ALINE))->SetLineSize(1);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_ALINE))->SetPageSize(50);
	SetDlgItemInt(IDC_EDIT_FLIM_MAX, (int)m_Max_FLIM);
	SetDlgItemInt(IDC_EDIT_FLIM_MIN, (int)m_Min_FLIM);
	SetDlgItemInt(IDC_EDIT_FLIM_MAX_LIFE, (int)m_Max_FLIM_life);
	SetDlgItemInt(IDC_EDIT_FLIM_MIN_LIFE, (int)m_Min_FLIM_life);
	CString str;
	str.Format(_T("Current A-line :    1 / %d"), m_nAlines);
	GetDlgItem(IDC_STATIC_CUR_ALINE)->SetWindowTextW(str);
		
	// Grid Visualization //////////////////////////////////////////////
	m_dStep = m_GraphRegionRect[0].Width() / m_dGridCount;
	m_dDataStep = m_dStep / m_dCountPerGrid;
	m_dIncrementX_OCT = m_GraphRegionRect[0].Width() / float(m_nScans - 1);
	m_dIncrementX_FFT = m_GraphRegionRect[0].Width() / float(m_nScansFFT / 2 - 1);
	m_dIncrementY = m_GraphRegionRect[0].Height() / float(POWER_2_16 - 1);
	half_16bit = np::Array<float>(m_nScans);
	for (int i = 0; i < m_nScans; i++)
		half_16bit(i) = POWER_2_15;

	// Visualization Bitmap File
	m_pTemp = np::Array<uint8_t, 2>(m_nScansFFT / 2, m_nAlines);
	m_pBitmapOCT = new MyBitmap;
	m_pBitmapCircOCT = new MyBitmap;
	m_pBitmapIntensity = new MyBitmap;

#if INTENSITY_MAP_FIRE
	int lut = LUT_FIRE;
#else
	int lut = LUT_PURPLE;
#endif

	m_pBitmapIntensity->SetBitmap(1, m_nAlines / 4, 1, lut); // purple
	m_pBitmapLifetime = new MyBitmap;
	m_pBitmapLifetime->SetBitmap(1, m_nAlines / 4, 1, LUT_HSV); // jet

	// FLIM Colorbar Initialization
	m_pColorbarIntensity = new MyBitmap;
	m_pColorbarIntensity->SetBitmap(1, 256, 1, lut);
	BYTE bar[256];
	for (int i = 0; i < 256; i++)
		bar[i] = (BYTE)i;
	memcpy(m_pColorbarIntensity->GetPtr(), bar, 256 * sizeof(BYTE));

	m_pColorbarLifetime = new MyBitmap;
	m_pColorbarLifetime->SetBitmap(1, 256, 1, LUT_HSV);
	memcpy(m_pColorbarLifetime->GetPtr(), bar, 256 * sizeof(BYTE));
	
	OnCbnSelchangeComboFlimShow();

	UpdateData(FALSE);	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CVisStream::PostNcDestroy()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	delete m_pBitmapOCT;
	delete m_pBitmapIntensity;
	delete m_pBitmapLifetime;
	delete m_pColorbarIntensity;
	delete m_pColorbarLifetime;

	CDialogEx::PostNcDestroy();
}


void CVisStream::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (!m_CheckCircularize)
	{
		bool isIn = false;
		CPoint pt1 = point, pt2;
		pt1.x = pt1.x + pt.x;
		pt1.y = pt1.y + pt.y;

		if (m_ToggleViewMode)
		{
			if (m_ImageRegionRect.PtInRect(pt1))
			{
				pt2 = { pt1.x - m_ImageRegionRect.left, pt1.y - m_ImageRegionRect.top };
				isIn = true;

				pt2.x = (LONG)floor(double(m_nAlines * pt2.x) / double(m_ImageRegionRect.Width()));
				pt2.y = (LONG)floor(double(m_nScansFFT / 2 * pt2.y) / double(m_ImageRegionRect.Height() * 0.92));
			}
		}
		else
		{
			CRect region = { 0, 0, m_GraphRegionRect[0].Width(), m_VisRegionRectWnd.Height() };
			if (region.PtInRect(pt1))
			{
				pt2 = { pt1.x - region.left, pt1.y - region.top };
				isIn = true;

				pt2.x = (LONG)floor(double(m_nAlines * pt2.x) / double(region.Width()));
				pt2.y = (LONG)floor(double(m_nScansFFT / 2 * pt2.y) / double(region.Height() * 0.92));
			}
		}

		if (isIn)
		{
			if (pt2.y < m_nScansFFT / 2)
			{
				CString str;
				str.Format(_T("(%4d, %4d) %3d (%.1f dB) / Ch. %d / %.3f / %.3f nsec"),
					pt2.x, pt2.y, *m_pBitmapOCT->GetPtr(m_nScansFFT / 2 - 1 - pt2.y, pt2.x), m_pMainDlg->m_FrameOCT(pt2.y, pt2.x),
					m_ComboFLIMShow + 1, m_pMainDlg->m_FluIntensity(pt2.x / 4, m_ComboFLIMShow + 1), m_pMainDlg->m_FluLifetime(pt2.x / 4, m_ComboFLIMShow));
				m_pMainDlg->m_StatusBar.SetText(str, 1, 0);
			}
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


BOOL CVisStream::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CVisStream::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	//CDialogEx::OnOK();
}


void CVisStream::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	//CDialogEx::OnCancel();
}


void CVisStream::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.

	CPaintDC dcGraph0(&m_StaticGraphRegion[0]);
	CPaintDC dcGraph1(&m_StaticGraphRegion[1]);
	CPaintDC dcGraph2(&m_StaticGraphRegion[2]);
	CPaintDC dcImage(&m_StaticImageRegion);
	CPaintDC dcCBar1(&m_StaticCBarRegion1);
	CPaintDC dcCBar2(&m_StaticCBarRegion2);
	CDC dcMem;
	
	// Assigning memory DC
	dcMem.CreateCompatibleDC(&dc);
	CBitmap btmp;
	btmp.CreateCompatibleBitmap(&dc, m_VisRegionRectWnd.Width(), m_VisRegionRectWnd.Height());
	dcMem.SelectObject(&btmp);
						
	// Fluorescence Information
	float* ScanIntensity = &m_pMainDlg->m_FluIntensity(0, 1 + m_ComboFLIMShow); // : &m_pMainDlg->m_FluLifetime(0, m_ComboFLIMShow % 3);
	float* ScanLifetime = &m_pMainDlg->m_FluLifetime(0, m_ComboFLIMShow);

	if (m_ToggleViewMode)
	{
		// Drawing black bg to memory DC
		dcMem.FillSolidRect(CRect(0, 0, m_GraphRegionRect[0].Width(), m_VisRegionRectWnd.Height()), RGB(0, 0, 0));

		// Drawing grid to memory DC	
		for (int i = 0; int(i*m_dDataStep) < m_GraphRegionRect[0].Width(); i++) {
			if (!(i % 10))
				dcMem.SelectObject(&m_MainGridPen);
			else
				dcMem.SelectObject(&m_SubGridPen);

			for (int j = 0; j < 3; j++)
			{
				dcMem.MoveTo(int(i*m_dDataStep), m_GraphRegionRect[j].top);
				dcMem.LineTo(int(i*m_dDataStep), m_GraphRegionRect[j].bottom);
			}
		}
		dcMem.SelectObject(&m_MainGridPen);
		for (int j = 0; j < 2; j++)
		{
			dcMem.MoveTo(m_GraphRegionRect[j].left, m_GraphRegionRect[j].bottom - m_GraphRegionRect[j].Height() / 2);
			dcMem.LineTo(m_GraphRegionRect[j].right, m_GraphRegionRect[j].bottom - m_GraphRegionRect[j].Height() / 2);
		}

		// Drawing FLIM pulse data
		dcMem.SelectObject(&m_DataPen);		
		uint16_t* ScanFramePulse = m_pMainDlg->m_FramePulse.raw_ptr() + m_Params_FLIM.act_ch * m_nScans 
			+ int((m_SliderCurAline + m_pMainDlg->m_ScrollAdjustGalvo) % m_nAlines / 4) * 4 * m_nScans;
		for (int i = 0; i < m_nScansFLIM - 1; i++) {
			dcMem.MoveTo(int(m_dIncrementX_FLIM* i),      int(m_dIncrementY*(UINT16_MAX - ScanFramePulse[m_pMainDlg->m_Edit_PreTrig + i])));
			dcMem.LineTo(int(m_dIncrementX_FLIM*(i + 1)), int(m_dIncrementY*(UINT16_MAX - ScanFramePulse[m_pMainDlg->m_Edit_PreTrig + i + 1])));
		}

		// Drawing OCT fringe data
		float* FringeBG = (m_CheckBgRemovedFringe) ? m_pMainDlg->m_pOCT->GetBg() : half_16bit.raw_ptr();
		uint16_t* ScanFrameFringe = m_pMainDlg->m_FrameFringe.raw_ptr() + (m_SliderCurAline + m_pMainDlg->m_ScrollAdjustGalvo) % m_nAlines * m_nScans;
		for (int i = 0; i < m_nScans - 1; i++) {
			dcMem.MoveTo(int(m_dIncrementX_OCT* i),      m_GraphRegionRect[1].top + int(m_dIncrementY * (POWER_2_15 - (float)ScanFrameFringe[i    ] + FringeBG[i])));
			dcMem.LineTo(int(m_dIncrementX_OCT*(i + 1)), m_GraphRegionRect[1].top + int(m_dIncrementY * (POWER_2_15 - (float)ScanFrameFringe[i + 1] + FringeBG[i])));
		}

		// Drawing OCT A-line data
		float* ScanFrameOCT = m_pMainDlg->m_FrameOCT.raw_ptr() + (m_SliderCurAline + m_pMainDlg->m_ScrollAdjustGalvo) % m_nAlines * m_nScansFFT / 2;
		for (int i = 0; i < m_nScansFFT/2 - 1; i++) {

			int y0 = (ScanFrameOCT[i] <= m_Max_dB) * int(m_dIncrementY_FFT*((m_Max_dB - ScanFrameOCT[i]))); // : 0;
			int y1 = (ScanFrameOCT[i + 1] <= m_Max_dB) * int(m_dIncrementY_FFT*((m_Max_dB - ScanFrameOCT[i + 1]))); // : 0;

			y0 += m_GraphRegionRect[2].top;
			y1 += m_GraphRegionRect[2].top;

			dcMem.MoveTo(int(m_dIncrementX_FFT* (i)), y0);
			dcMem.LineTo(int(m_dIncrementX_FFT* (i + 1)), y1);
		}

		// Drawing window for FLIM (if necessary)
		float* ScanMeanDelay = m_pMainDlg->m_FluMeanDelay.raw_ptr();
		int offset = int(m_SliderCurAline / 4); // 256 x 4
		if (m_CheckFLIMShowWindow) {
			dcMem.SelectObject(&m_WindowPen);
			for (int i = 0; i < 4; i++) {
				dcMem.MoveTo(int(m_dIncrementX_FLIM * m_Params_FLIM.ch_start_ind[i]), 0);
				dcMem.LineTo(int(m_dIncrementX_FLIM * m_Params_FLIM.ch_start_ind[i]), m_GraphRegionRect[0].Height());
			}
			dcMem.MoveTo(int(m_dIncrementX_FLIM * (m_Params_FLIM.ch_start_ind[3] + FLIM_CH_START_5)), 0);
			dcMem.LineTo(int(m_dIncrementX_FLIM * (m_Params_FLIM.ch_start_ind[3] + FLIM_CH_START_5)), m_GraphRegionRect[0].Height());

			dcMem.SelectObject(&m_MeanDelayPen);
			for (int i = 0; i < 4; i++) {
				dcMem.MoveTo(int(m_dIncrementX_FLIM * ScanMeanDelay[offset + i * m_nAlines / 4]), 0);
				dcMem.LineTo(int(m_dIncrementX_FLIM * ScanMeanDelay[offset + i * m_nAlines / 4]), m_GraphRegionRect[0].Height());
			}
		}

		// Scaling & Drawing OCT image 
		ScanFrameOCT = m_pMainDlg->m_FrameOCT.raw_ptr();

		SetStretchBltMode(dcMem.m_hDC, COLORONCOLOR);

		IppiSize roi_oct = { m_nScansFFT / 2, m_nAlines };
		ippiScale_32f8u_C1R(ScanFrameOCT, roi_oct.width * sizeof(float),
			m_pTemp.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct, (float)m_Min_dB, (float)m_Max_dB);

		if (!m_CheckCircularize)
		{
			ippiMirror_8u_C1IR(m_pTemp.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct, ippAxsVertical);
			ippiTranspose_8u_C1R(m_pTemp.raw_ptr(), roi_oct.width * sizeof(uint8_t), m_pBitmapOCT->GetPtr(), roi_oct.height * sizeof(uint8_t), roi_oct);

			for (int i = 0; i < m_nScansFFT / 2; i++)
			{
				uint8_t* pImg = m_pBitmapOCT->GetPtr() + i * m_nAlines;
				std::rotate(pImg, pImg + m_pMainDlg->m_ScrollAdjustGalvo, pImg + m_nAlines);
			}

			StretchDIBits(dcMem.m_hDC,
				m_ImageRegionRect.left, m_ImageRegionRect.top, m_ImageRegionRect.Width(), int(m_ImageRegionRect.Height() * 0.92),
				0, 0, m_pBitmapOCT->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapOCT->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapOCT->GetPtr(), m_pBitmapOCT->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

			// Scaling FLIM image
			IppiSize roi_flim = { m_nAlines / 4, 1 };
			ippiScale_32f8u_C1R(ScanIntensity, roi_flim.width * sizeof(float),
				m_pBitmapIntensity->GetPtr(), roi_flim.width * sizeof(BYTE), roi_flim, m_Min_FLIM, m_Max_FLIM);
			ippiScale_32f8u_C1R(ScanLifetime, roi_flim.width * sizeof(float),
				m_pBitmapLifetime->GetPtr(), roi_flim.width * sizeof(BYTE), roi_flim, m_Min_FLIM_life, m_Max_FLIM_life);
			std::rotate(m_pBitmapIntensity->GetPtr(), m_pBitmapIntensity->GetPtr() + m_pMainDlg->m_ScrollAdjustGalvo / 4, m_pBitmapIntensity->GetPtr() + roi_flim.width);
			std::rotate(m_pBitmapLifetime->GetPtr(), m_pBitmapLifetime->GetPtr() + m_pMainDlg->m_ScrollAdjustGalvo / 4, m_pBitmapLifetime->GetPtr() + roi_flim.width);

			StretchDIBits(dcMem.m_hDC,
				m_ImageRegionRect.left, m_ImageRegionRect.top + int(m_ImageRegionRect.Height() * 0.92), m_ImageRegionRect.Width(), int(m_ImageRegionRect.Height() * 0.04) + 1,
				0, 0, m_pBitmapIntensity->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapIntensity->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapIntensity->GetPtr(), m_pBitmapIntensity->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
			StretchDIBits(dcMem.m_hDC,
				m_ImageRegionRect.left, m_ImageRegionRect.top + int(m_ImageRegionRect.Height() * 0.96), m_ImageRegionRect.Width(), int(m_ImageRegionRect.Height() * 0.04) + 1,
				0, 0, m_pBitmapLifetime->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapLifetime->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapLifetime->GetPtr(), m_pBitmapLifetime->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
		}
		else
		{
			circ_OCT(m_pTemp, m_pBitmapCircOCT->GetPtr(), m_EditCircCenter);

			StretchDIBits(dcMem.m_hDC,
				m_ImageRegionRect.left, m_ImageRegionRect.top, m_ImageRegionRect.Width(), m_ImageRegionRect.Width(),
				0, 0, m_pBitmapCircOCT->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapCircOCT->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapCircOCT->GetPtr(), m_pBitmapCircOCT->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
		}

		// Memory DC transfer
		dcGraph0.BitBlt(0, 0, m_GraphRegionRect[0].Width(), m_GraphRegionRect[0].Height(), &dcMem, 0, 0, SRCCOPY);
		dcGraph1.BitBlt(0, 0, m_GraphRegionRect[0].Width(), m_GraphRegionRect[0].Height(), &dcMem, 0, m_GraphRegionRect[1].top, SRCCOPY);
		dcGraph2.BitBlt(0, 0, m_GraphRegionRect[0].Width(), m_GraphRegionRect[0].Height(), &dcMem, 0, m_GraphRegionRect[2].top, SRCCOPY);
		int im_height = (!m_CheckCircularize) ? m_ImageRegionRect.Height() : m_ImageRegionRect.Width();
		dcImage.BitBlt(0, 0, m_ImageRegionRect.Width(), im_height, &dcMem, m_ImageRegionRect.left, m_ImageRegionRect.top, SRCCOPY);
	}
	else
	{
		// Scaling & Drawing OCT image 
		float* ScanFrameOCT = m_pMainDlg->m_FrameOCT.raw_ptr();

		SetStretchBltMode(dcMem.m_hDC, COLORONCOLOR);

		IppiSize roi_oct = { m_nScansFFT / 2, m_nAlines };
		ippiScale_32f8u_C1R(ScanFrameOCT, roi_oct.width * sizeof(float),
			m_pTemp.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct, (float)m_Min_dB, (float)m_Max_dB);

		if (!m_CheckCircularize)
		{
			ippiMirror_8u_C1IR(m_pTemp.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct, ippAxsVertical);
			ippiTranspose_8u_C1R(m_pTemp.raw_ptr(), roi_oct.width * sizeof(uint8_t), m_pBitmapOCT->GetPtr(), roi_oct.height * sizeof(uint8_t), roi_oct);

			for (int i = 0; i < m_nScansFFT / 2; i++)
			{
				uint8_t* pImg = m_pBitmapOCT->GetPtr() + i * m_nAlines;
				std::rotate(pImg, pImg + m_pMainDlg->m_ScrollAdjustGalvo, pImg + m_nAlines);
			}

			StretchDIBits(dcMem.m_hDC,
				m_GraphRegionRect[0].left, m_GraphRegionRect[0].top, m_GraphRegionRect[0].Width(), int(m_VisRegionRectWnd.Height() * 0.92),
				0, 0, m_pBitmapOCT->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapOCT->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapOCT->GetPtr(), m_pBitmapOCT->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

			// Scaling FLIM image
			IppiSize roi_flim = { m_nAlines / 4, 1 };
			ippiScale_32f8u_C1R(ScanIntensity, roi_flim.width * sizeof(float),
				m_pBitmapIntensity->GetPtr(), roi_flim.width * sizeof(BYTE), roi_flim, m_Min_FLIM, m_Max_FLIM);
			ippiScale_32f8u_C1R(ScanLifetime, roi_flim.width * sizeof(float),
				m_pBitmapLifetime->GetPtr(), roi_flim.width * sizeof(BYTE), roi_flim, m_Min_FLIM_life, m_Max_FLIM_life);
			std::rotate(m_pBitmapIntensity->GetPtr(), m_pBitmapIntensity->GetPtr() + m_pMainDlg->m_ScrollAdjustGalvo / 4, m_pBitmapIntensity->GetPtr() + roi_flim.width);
			std::rotate(m_pBitmapLifetime->GetPtr(), m_pBitmapLifetime->GetPtr() + m_pMainDlg->m_ScrollAdjustGalvo / 4, m_pBitmapLifetime->GetPtr() + roi_flim.width);

			StretchDIBits(dcMem.m_hDC,
				m_GraphRegionRect[0].left, m_GraphRegionRect[0].top + int(m_VisRegionRectWnd.Height() * 0.92), m_GraphRegionRect[0].Width(), int(m_VisRegionRectWnd.Height() * 0.04) + 1,
				0, 0, m_pBitmapIntensity->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapIntensity->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapIntensity->GetPtr(), m_pBitmapIntensity->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
			StretchDIBits(dcMem.m_hDC,
				m_GraphRegionRect[0].left, m_GraphRegionRect[0].top + int(m_VisRegionRectWnd.Height() * 0.96), m_GraphRegionRect[0].Width(), int(m_VisRegionRectWnd.Height() * 0.04) + 1,
				0, 0, m_pBitmapLifetime->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapLifetime->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapLifetime->GetPtr(), m_pBitmapLifetime->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
		}
		else
		{
			circ_OCT(m_pTemp, m_pBitmapCircOCT->GetPtr(), m_EditCircCenter);

			StretchDIBits(dcMem.m_hDC,
				m_GraphRegionRect[0].left, m_GraphRegionRect[0].top, m_GraphRegionRect[0].Width(), m_GraphRegionRect[0].Width(),
				0, 0, m_pBitmapCircOCT->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapCircOCT->GetLpBmpInfo()->bmiHeader.biHeight,
				m_pBitmapCircOCT->GetPtr(), m_pBitmapCircOCT->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
		}

		// Memory DC transfer
		int im_height = (!m_CheckCircularize) ? m_VisRegionRectWnd.Height() : m_GraphRegionRect[0].Width();
		dc.StretchBlt(m_VisRegionRectWnd.left, m_VisRegionRectWnd.top, m_GraphRegionRect[0].Width(), im_height,
			&dcMem, m_GraphRegionRect[0].left, m_GraphRegionRect[0].top, m_GraphRegionRect[0].Width(), im_height, SRCCOPY);
	}

	// Drawing colorbars	
	StretchDIBits(dcMem.m_hDC,
		m_CBarRegionRect1.left, m_CBarRegionRect1.top, m_CBarRegionRect1.Width(), m_CBarRegionRect1.Height(),
		0, 0, m_pColorbarIntensity->GetLpBmpInfo()->bmiHeader.biWidth, m_pColorbarIntensity->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pColorbarIntensity->GetPtr(), m_pColorbarIntensity->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
	StretchDIBits(dcMem.m_hDC,
		m_CBarRegionRect2.left, m_CBarRegionRect2.top, m_CBarRegionRect2.Width(), m_CBarRegionRect2.Height(),
		0, 0, m_pColorbarLifetime->GetLpBmpInfo()->bmiHeader.biWidth, m_pColorbarLifetime->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pColorbarLifetime->GetPtr(), m_pColorbarLifetime->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

	// Memory DC transfer
	dcCBar1.BitBlt(0, 0, m_CBarRegionRect1.Width(), m_CBarRegionRect1.Height(), &dcMem, m_CBarRegionRect1.left, m_CBarRegionRect1.top, SRCCOPY);
	dcCBar2.BitBlt(0, 0, m_CBarRegionRect2.Width(), m_CBarRegionRect2.Height(), &dcMem, m_CBarRegionRect2.left, m_CBarRegionRect2.top, SRCCOPY);	

	// FLIM Analysis Dialog
	if (m_pFLIMAnalysisDlg)
		m_pFLIMAnalysisDlg->Draw(ScanIntensity, ScanLifetime);

	// FLIM Pulse Overlapped View Dialog
	if (m_pPulseOverlapDlg)
		m_pPulseOverlapDlg->InvalidateRect(m_pPulseOverlapDlg->m_RectPulseGraphWnd, FALSE);
}

void CVisStream::run()
{
	unsigned int frameIndex = 0;

	_running = true;
	while (_running)
		DidAcquireData(frameIndex++);
}

bool CVisStream::startVisualization()
{
	if (_thread.joinable())
	{
		printf("ERROR: Visualization thread is already running.\n");
		return false;
	}

	_thread = std::thread(&CVisStream::run, this);

	printf("Visualization thread is started.\n");

	return true;
}

void CVisStream::stopVisualization()
{
	std::thread stop([&] () 
	{
		std::unique_lock<std::mutex> lock(m_pMainDlg->m_mtx_Invalidate);

		if (_thread.joinable())
		{
			_running = false;
			_thread.join();
		}

		printf("Visualization thread is finished normally.\n");
	});

	stop.detach();
}

void CVisStream::OnBnClickedToggleViewMode()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	for (int i = 0; i < 7; i++)
		GetDlgItem(IDC_STATIC_TIME1 + i)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_HIGH_VOL)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_HIGH_VOL2)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_LOW_VOL)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_LOW_VOL2)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_HIGH_DB)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_LOW_DB)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_ZERO_VOL)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_ZERO_VOL2)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_STATIC_CUR_ALINE)->ShowWindow(m_ToggleViewMode);
	GetDlgItem(IDC_SLIDER_CUR_ALINE)->ShowWindow(m_ToggleViewMode);

	if (m_ToggleViewMode)
		SetDlgItemTextW(IDC_TOGGLE_VIEW_MODE, _T("Diagnosis Mode"));
	else
		SetDlgItemTextW(IDC_TOGGLE_VIEW_MODE, _T("Preview Mode"));

	InvalidateRect(m_VisRegionRectWnd, TRUE);
}


void CVisStream::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_ALINE);

	if (pScrollBar == (CScrollBar*)pSlider)
	{
		UpdateData(TRUE);
		
		CString str;
		str.Format(_T("Current A-line : %4d / %4d"), m_SliderCurAline + 1, m_nAlines);
		SetDlgItemText(IDC_STATIC_CUR_ALINE, str);
		InvalidateRect(m_VisRegionRectWnd, FALSE);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CVisStream::OnBnClickedCheckFlimShowWindow()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnBnClickedCheckFlimSoftwareBroadening()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	m_Params_FLIM.is_broadening = m_CheckFLIMSoftwareBroadening;
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnDeltaposSpinChannelStartCh0(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int ch = 0;
	if (pNMUpDown->iDelta < 0)
	{
		if (m_Params_FLIM.ch_start[ch] > m_nScansFLIM * m_Params_FLIM.samp_intv)
			return;
		m_Params_FLIM.ch_start[ch] += m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	else
	{
		if (m_Params_FLIM.ch_start[ch] < 0)
			return;
		m_Params_FLIM.ch_start[ch] -= m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	UpdateData(FALSE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
	if (m_pPulseOverlapDlg) m_pPulseOverlapDlg->Invalidate(FALSE);
	*pResult = 0;
}


void CVisStream::OnDeltaposSpinChannelStartCh1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int ch = 1;
	if (pNMUpDown->iDelta < 0)
	{
		if (m_Params_FLIM.ch_start[ch] > m_nScansFLIM * m_Params_FLIM.samp_intv)
			return;
		m_Params_FLIM.ch_start[ch] += m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	else
	{
		if (m_Params_FLIM.ch_start[ch] < 0)
			return;
		m_Params_FLIM.ch_start[ch] -= m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	UpdateData(FALSE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
	if (m_pPulseOverlapDlg) m_pPulseOverlapDlg->Invalidate(FALSE);
	*pResult = 0;
}


void CVisStream::OnDeltaposSpinChannelStartCh2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int ch = 2;
	if (pNMUpDown->iDelta < 0)
	{
		if (m_Params_FLIM.ch_start[ch] > m_nScansFLIM * m_Params_FLIM.samp_intv)
			return;
		m_Params_FLIM.ch_start[ch] += m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	else
	{
		if (m_Params_FLIM.ch_start[ch] < 0)
			return;
		m_Params_FLIM.ch_start[ch] -= m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	UpdateData(FALSE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
	if (m_pPulseOverlapDlg) m_pPulseOverlapDlg->Invalidate(FALSE);
	*pResult = 0;
}


void CVisStream::OnDeltaposSpinChannelStartCh3(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int ch = 3;
	if (pNMUpDown->iDelta < 0)
	{
		if (m_Params_FLIM.ch_start[ch] > m_nScansFLIM * m_Params_FLIM.samp_intv)
			return;
		m_Params_FLIM.ch_start[ch] += m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	else
	{
		if (m_Params_FLIM.ch_start[ch] < 0)
			return;
		m_Params_FLIM.ch_start[ch] -= m_Params_FLIM.samp_intv;
		m_Params_FLIM.ch_start_ind[ch] = int(m_Params_FLIM.ch_start[ch] / m_Params_FLIM.samp_intv);
	}
	UpdateData(FALSE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
	if (m_pPulseOverlapDlg) m_pPulseOverlapDlg->Invalidate(FALSE);
	*pResult = 0;
}


void CVisStream::OnEnChangeEditDelayOffsetCh()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if (m_pPulseOverlapDlg) m_pPulseOverlapDlg->Invalidate(FALSE);
}


void CVisStream::OnEnChangeEditVisFlimSamps()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다..	
	CString str;
	UpdateData(TRUE);

	if (m_nScansFLIM > m_nScans)
	{
		m_nScansFLIM = m_nScans;
		UpdateData(FALSE);
	}

	m_dIncrementX_FLIM = (float)m_GraphRegionRect[0].Width() / float(m_nScansFLIM - 1);
	for (int i = 0; i < 7; i++) {
		str.Format(_T("%2.1fns"), ((double)m_nScansFLIM - 1) / 8 * m_Params_FLIM.samp_intv * (i + 1));
		SetDlgItemTextW(IDC_STATIC_TIME1 + i, str);
	}

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditFlimBackground()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CVisStream::OnBnClickedButtonCaptureFlimBackground()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	Ipp16u* flim_bg = m_pMainDlg->m_FramePulse.raw_ptr();
	Ipp32f* flim_bg_32f = ippsMalloc_32f(m_nScans);
	ippsConvert_16u32f(flim_bg, flim_bg_32f, m_nScans);
	ippsMean_32f(flim_bg_32f, m_nScans, &m_Params_FLIM.bg, ippAlgHintFast);
	UpdateData(FALSE);
}


void CVisStream::OnEnChangeEditFlimWidth()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CVisStream::OnDeltaposSpinFlimChannel(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pNMUpDown->iDelta < 0)
	{
		if (m_Params_FLIM.act_ch >= 3)
			return;
		m_Params_FLIM.act_ch++;
	}
	else
	{
		if (m_Params_FLIM.act_ch <= 0)
			return;
		m_Params_FLIM.act_ch--;
	}
	UpdateData(FALSE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
	if (m_pPulseOverlapDlg) m_pPulseOverlapDlg->Invalidate(FALSE);
	*pResult = 0;
}


void CVisStream::OnEnChangeEditFlimMax()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_FLIM_MAX, str);
	m_Max_FLIM = (float)_wtof(str);
	
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditFlimMin()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_FLIM_MIN, str);
	m_Min_FLIM = (float)_wtof(str);

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditFlimMaxLife()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_FLIM_MAX_LIFE, str);
	m_Max_FLIM_life = (float)_wtof(str);

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditFlimMinLife()
{	
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_FLIM_MIN_LIFE, str);
	m_Min_FLIM_life = (float)_wtof(str);

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnCbnSelchangeComboFlimShow()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	int ch;
	switch (m_ComboFLIMShow)
	{
	case 0:
		ch = LUT_PURPLE; // purple (ch1 intensity)
		break;
	case 1:
		ch = LUT_BLUE; // blue (ch2 intensity)		
		break;
	case 2:
		ch = LUT_GREEN; // green (ch3 intensity)
		break;
	}
	
#if INTENSITY_MAP_FIRE
	ch = LUT_FIRE;
#endif

	m_pBitmapIntensity->SetColormap(ch); 
	m_pColorbarIntensity->SetColormap(ch);
			
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnBnClickedButtonFlimAnalysis()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (!m_pFLIMAnalysisDlg)
	{
		m_pFLIMAnalysisDlg = new FLIMAnalysisDlg;
		m_pFLIMAnalysisDlg->SetMainDlg(m_pMainDlg);
		m_pFLIMAnalysisDlg->Create(IDD_DIALOG_FLIM_ANALYSIS);
		m_pFLIMAnalysisDlg->ShowWindow(SW_SHOW);
	}
	else
		m_pFLIMAnalysisDlg->SetFocus();

	m_pFLIMAnalysisDlg->MoveWindow(CRect(0, 815, 469, 1079));
}


void CVisStream::OnBnClickedButtonFlimPulseOverlappedView()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (!m_pPulseOverlapDlg)
	{
		m_pPulseOverlapDlg = new PulseOverlapDlg;
		m_pPulseOverlapDlg->SetMainDlg(m_pMainDlg);
		m_pPulseOverlapDlg->Create(IDD_DIALOG_PULSE_OVERLAP);
		m_pPulseOverlapDlg->ShowWindow(SW_SHOW);
	}
	else
		m_pPulseOverlapDlg->SetFocus();

	//m_pPulseOverlapDlg->MoveWindow(CRect(775, 78, 1432, 366));
}


void CVisStream::OnBnClickedCheckBgRemovedFringe()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnBnClickedCheckCircularize()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE); 

	if (m_ToggleViewMode)
	{
		CRect ImageRegionRect1 = m_ImageRegionRect;
		POINT pt = { m_VisRegionRectWnd.left, m_VisRegionRectWnd.top };
		ImageRegionRect1.OffsetRect(pt);
		InvalidateRect(ImageRegionRect1, TRUE);
	}
	else
	{
		CRect ImageRegionRect1(m_GraphRegionRect[0].left, m_GraphRegionRect[0].top, m_GraphRegionRect[0].right, m_ImageRegionRect.bottom);
		POINT pt = { m_VisRegionRectWnd.left, m_VisRegionRectWnd.top };
		ImageRegionRect1.OffsetRect(pt);
		InvalidateRect(ImageRegionRect1, TRUE);
	}
}


void CVisStream::OnBnClickedButtonCalibration()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (!m_pCalibrationDlg)
	{
		m_pCalibrationDlg = new CalibrationDlg;
		m_pCalibrationDlg->SetMainDlg(m_pMainDlg);
		m_pCalibrationDlg->Create(IDD_DIALOG_CALIBRATION);
		m_pCalibrationDlg->ShowWindow(SW_SHOW);		
	}
	else
		m_pCalibrationDlg->SetFocus();
	
	m_pCalibrationDlg->MoveWindow(CRect(0, 392, 395, 814));
}


void CVisStream::OnCbnSelchangeComboLut()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	m_pBitmapOCT->SetColormap(m_ComboLUT);
	m_pBitmapCircOCT->SetColormap(m_ComboLUT);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditAlineMaxDb()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_Max_dB < m_Min_dB)	
		return;
	
	SetDlgItemInt(IDC_STATIC_HIGH_DB, m_Max_dB);

	m_dIncrementY_FFT = (float)m_GraphRegionRect[0].Height() / (float)(m_Max_dB - m_Min_dB - 1);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditAlineMinDb()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_Max_dB < m_Min_dB)
		return;

	SetDlgItemInt(IDC_STATIC_LOW_DB, m_Min_dB);

	m_dIncrementY_FFT = (float)m_GraphRegionRect[0].Height() / (float)(m_Max_dB - m_Min_dB - 1);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisStream::OnEnChangeEditCircCenter()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if (m_EditCircCenter > (m_nScansFFT / 2 - 1300))
	{
		m_EditCircCenter = m_nScansFFT / 2 - 1300 - 1;
		UpdateData(FALSE);
	}
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


