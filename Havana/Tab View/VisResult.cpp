// VisOCT.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "VisResult.h"
#include "afxdialogex.h"

#include "Havana.h"
#include "HavanaDlg.h"

#include "DataRecording\DataRecording.h"
#include "ImageProcess\MyBitmap.h"
#include "ImageProcess\OCTProcess\OCTProcess.h"
#include "ImageProcess\FLIMProcess\FLIMProcess.h"

#include "ImageProcess\FLIMProcess\PulseReviewDlg\PulseReviewDlg.h"

#include <ippcc.h>

#include <iostream>
#include <thread>
#include <chrono>

// CVisOCT 대화 상자입니다.

IMPLEMENT_DYNAMIC(CVisResult, CDialogEx)

CVisResult::CVisResult(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TAB_RESULT, pParent),
	m_GreenPen(PS_SOLID, 1, RGB(0, 255, 0)), m_WhitePen(PS_SOLID, 1, RGB(255, 255, 255)), m_RedPen(PS_SOLID, 1, RGB(200, 0, 0)),
	m_pPulseReviewDlg(nullptr), m_pFLIMforExt(nullptr), m_SliderCurFrame(0), m_RadioSelectData(0),
	m_ComboResLut(0), m_ComboResFlimShow(0), 
	m_CheckRGBWriting(false), m_CheckShowGuideLine(false), m_CheckCircularize(false), m_CheckHSVEnhancing(false), m_CheckResNormalization(false),
	m_ToggleMeasureDistance(false), n_click(0),
	m_EditMaxIntensity(MAX_INTENSITY), m_EditMinIntensity(MIN_INTENSITY),
	m_EditMaxLifetime(MAX_LIFETIME), m_EditMinLifetime(MIN_LIFETIME),
	m_nFrame(1), m_EditCircCenter(0), m_AdjustGalvo(0),
	m_pBitmapOCT(nullptr), m_pBitmapCircOCT(nullptr), 
	m_pBitmapProjection(nullptr), m_pBitmapIntensityMap(nullptr), m_pBitmapLifetimeMap(nullptr), m_pBitmapEnhancedMap(nullptr),
	m_pColorbarProjection(nullptr), m_pColorbarIntensity(nullptr), m_pColorbarLifetime(nullptr),
	m_IsExternData(false), m_IsSaved(false), m_path(_T(""))
	
{
}

CVisResult::~CVisResult()
{
}

void CVisResult::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Slider(pDX, IDC_SLIDER_CUR_FRAME, m_SliderCurFrame);
	DDX_Radio(pDX, IDC_RADIO_IN_BUFFER, (int&)m_RadioSelectData);

	DDX_Text(pDX, IDC_EDIT_RES_ALINE_MAX_DB, m_EditResMaxDb);
	DDX_Text(pDX, IDC_EDIT_RES_ALINE_MIN_DB, m_EditResMinDb);
	DDX_Text(pDX, IDC_EDIT_RES_CIRC_CENTER, m_EditCircCenter);

	DDX_Check(pDX, IDC_CHECK_RES_CIRCULARIZE, m_CheckCircularize);
	DDX_CBIndex(pDX, IDC_COMBO_RES_LUT, m_ComboResLut);
	DDX_CBIndex(pDX, IDC_COMBO_RES_FLIM_SHOW, m_ComboResFlimShow);

	DDX_Control(pDX, IDC_STATIC_RES_IMAGE, m_StaticResImage);
	DDX_Control(pDX, IDC_STATIC_RES_PROJECTION, m_StaticResProjection);
	DDX_Control(pDX, IDC_STATIC_RES_COLORBAR_PROJECTION, m_StaticResColorbarProjection);
	DDX_Control(pDX, IDC_STATIC_INTENSITY_MAP, m_StaticResIntensityMap);
	DDX_Control(pDX, IDC_STATIC_RES_COLORBAR_INTENSITY, m_StaticResColorbarIntensity);
	DDX_Control(pDX, IDC_STATIC_LIFETIME_MAP, m_StaticResLifetimeMap);
	DDX_Control(pDX, IDC_STATIC_RES_COLORBAR_LIFETIME, m_StaticResColorbarLifetime);

	DDX_Control(pDX, IDC_PROGRESS_RES, m_ProgressRes);
	DDX_Check(pDX, IDC_CHECK_RES_HSV_ENHANCING, m_CheckHSVEnhancing);
	DDX_Check(pDX, IDC_CHECK_RES_SAVE_AS_RGB, m_CheckRGBWriting);
	DDX_Check(pDX, IDC_CHECK_RES_SHOW_GUIDELINE, m_CheckShowGuideLine);
	DDX_Check(pDX, IDC_CHECK_RES_MEASURE_DISTANCE, m_ToggleMeasureDistance);
	DDX_Check(pDX, IDC_CHECK_RES_NORMALIZATION, m_CheckResNormalization);
}

BEGIN_MESSAGE_MAP(CVisResult, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_IN_BUFFER, IDC_RADIO_EXTERNAL, CVisResult::OnSetRadioSelectData)
	ON_BN_CLICKED(IDC_CHECK_RES_CIRCULARIZE, &CVisResult::OnBnClickedCheckCircularize)
	ON_CBN_SELCHANGE(IDC_COMBO_RES_LUT, &CVisResult::OnCbnSelchangeComboResLut)
	ON_CBN_SELCHANGE(IDC_COMBO_RES_FLIM_SHOW, &CVisResult::OnCbnSelchangeComboResFlimShow)
	ON_BN_CLICKED(IDC_BUTTON_IMAGE_PROCESSING, &CVisResult::OnBnClickedButtonImageProcessing)
	ON_EN_CHANGE(IDC_EDIT_RES_ALINE_MAX_DB, &CVisResult::OnEnChangeEditResAlineMaxDb)
	ON_EN_CHANGE(IDC_EDIT_RES_ALINE_MIN_DB, &CVisResult::OnEnChangeEditResAlineMinDb)
	ON_EN_CHANGE(IDC_EDIT_RES_INTENSITY_MAX, &CVisResult::OnEnChangeEditResIntensityMax)
	ON_EN_CHANGE(IDC_EDIT_RES_INTENSITY_MIN, &CVisResult::OnEnChangeEditResIntensityMin)
	ON_EN_CHANGE(IDC_EDIT_RES_LIFETIME_MAX, &CVisResult::OnEnChangeEditResLifetimeMax)
	ON_EN_CHANGE(IDC_EDIT_RES_LIFETIME_MIN, &CVisResult::OnEnChangeEditResLifetimeMin)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_EN_CHANGE(IDC_EDIT_RES_CIRC_CENTER, &CVisResult::OnEnChangeEditResCircCenter)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_RESULT, &CVisResult::OnBnClickedButtonSaveResult)
	ON_BN_CLICKED(IDC_CHECK_RES_HSV_ENHANCING, &CVisResult::OnBnClickedCheckResHsvEnhancing)
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_CHECK_RES_SAVE_AS_RGB, &CVisResult::OnBnClickedCheckResSaveAsRgb)
	ON_BN_CLICKED(IDC_CHECK_RES_SHOW_GUIDELINE, &CVisResult::OnBnClickedCheckResShowGuideline)
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_CHECK_RES_MEASURE_DISTANCE, &CVisResult::OnBnClickedCheckResMeasureDistance)
	ON_BN_CLICKED(IDC_CHECK_RES_NORMALIZATION, &CVisResult::OnBnClickedCheckResNormalization)
END_MESSAGE_MAP()


// CVisOCT 메시지 처리기입니다.

void CVisResult::SetMainDlg(CHavanaDlg* pMainDlg)
{
	m_pMainDlg = pMainDlg;

	// Size variables initialization	
	SetCircs(m_pMainDlg->m_Edit_nScans, m_pMainDlg->m_Edit_nAlines);
}


BOOL CVisResult::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	// Image View 부분 초기화 ////////////////////////////////////////////////////////////////////////////
	CWnd *pWnd = this->GetDlgItem(IDC_STATIC_RES_IMAGE);
	pWnd->GetWindowRect(&m_ResImageRect);
	this->ScreenToClient(&m_ResImageRect);

	pWnd = this->GetDlgItem(IDC_STATIC_RES_PROJECTION);
	pWnd->GetWindowRect(&m_ResProjectionRect);
	this->ScreenToClient(&m_ResProjectionRect);

	pWnd = this->GetDlgItem(IDC_STATIC_INTENSITY_MAP);
	pWnd->GetWindowRect(&m_ResIntensityMapRect);
	this->ScreenToClient(&m_ResIntensityMapRect);

	pWnd = this->GetDlgItem(IDC_STATIC_LIFETIME_MAP);
	pWnd->GetWindowRect(&m_ResLifetimeMapRect);
	this->ScreenToClient(&m_ResLifetimeMapRect);

	pWnd = this->GetDlgItem(IDC_STATIC_RES_COLORBAR_INTENSITY);
	pWnd->GetWindowRect(&m_CBarRegionRect1);
	this->ScreenToClient(&m_CBarRegionRect1);

	pWnd = this->GetDlgItem(IDC_STATIC_RES_COLORBAR_LIFETIME);
	pWnd->GetWindowRect(&m_CBarRegionRect2);
	this->ScreenToClient(&m_CBarRegionRect2);

	pWnd = this->GetDlgItem(IDC_STATIC_RES_COLORBAR_PROJECTION);
	pWnd->GetWindowRect(&m_CBarRegionRect3);
	this->ScreenToClient(&m_CBarRegionRect3);

	m_VisRegionRectWnd = CRect(m_ResImageRect.left, m_ResImageRect.top,
		m_CBarRegionRect2.right, m_ResImageRect.bottom);

	pt = { -m_ResImageRect.left, -m_ResImageRect.top };
	m_ResImageRect.OffsetRect(pt);
	m_ResProjectionRect.OffsetRect(pt);
	m_ResIntensityMapRect.OffsetRect(pt);
	m_ResLifetimeMapRect.OffsetRect(pt);
	m_CBarRegionRect1.OffsetRect(pt);
	m_CBarRegionRect2.OffsetRect(pt);
	m_CBarRegionRect3.OffsetRect(pt);

	// Control Initialization
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetRange(0, m_nFrame - 1);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetLineSize(1);
	((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetPageSize(20);
	SetDlgItemInt(IDC_EDIT_RES_INTENSITY_MAX, (int)m_EditMaxIntensity);
	SetDlgItemInt(IDC_EDIT_RES_INTENSITY_MIN, (int)m_EditMinIntensity);
	SetDlgItemInt(IDC_EDIT_RES_LIFETIME_MAX, (int)m_EditMaxLifetime);
	SetDlgItemInt(IDC_EDIT_RES_LIFETIME_MIN, (int)m_EditMinLifetime);

	// Visualization Bitmap File
	SetBitmaps();

	// Buffers Setting
	SetBuffers(m_nFrame);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CVisResult::SetCircs(int scans, int alines)
{
	m_nScans = scans;
	m_nScansFFT = GET_NEAR_2_POWER(scans);

	m_nAlines = alines;
	m4_nAlines = m_nAlines / 4;
	m_nAlines4 = (m_nAlines + 3) & ~3;

	m_nSizeFrame = m_nScans * m_nAlines;
	m_nSizeImageOCT = m_nScansFFT * m_nAlines;
	
	circ_OCT = circularize(CIRC_RADIUS, m_nAlines, false);
	circ_writing_OCT = circularize(CIRC_RADIUS, m_nAlines, true);
	rect_medfilt = medfilt(m_nScansFFT / 2, m_nAlines, 3, 3); // , "ipp8u");
}


void CVisResult::SetBitmaps()
{
	if (m_pBitmapOCT) { delete m_pBitmapOCT; m_pBitmapOCT = nullptr; }
	if (m_pBitmapCircOCT) { delete m_pBitmapCircOCT; m_pBitmapCircOCT = nullptr; }
	
	if (m_pBitmapProjection) { delete m_pBitmapProjection; m_pBitmapProjection = nullptr; }
	if (m_pBitmapIntensityMap) { delete m_pBitmapIntensityMap; m_pBitmapIntensityMap = nullptr; }
	if (m_pBitmapLifetimeMap) { delete m_pBitmapLifetimeMap; m_pBitmapLifetimeMap = nullptr; }
	if (m_pBitmapEnhancedMap) { delete m_pBitmapEnhancedMap; m_pBitmapEnhancedMap = nullptr; }

	if (m_pColorbarProjection) { delete m_pColorbarProjection; m_pColorbarProjection = nullptr; }
	if (m_pColorbarIntensity) { delete m_pColorbarIntensity; m_pColorbarIntensity = nullptr; }
	if (m_pColorbarLifetime) { delete m_pColorbarLifetime; m_pColorbarLifetime = nullptr; }

	m_pBitmapOCT = new MyBitmap;
	m_pBitmapOCT->SetBitmap(m_nScansFFT / 2, m_nAlines, 3); // inv gray
	m_pBitmapCircOCT = new MyBitmap;
	m_pBitmapCircOCT->SetBitmap(2 * CIRC_RADIUS, 2 * CIRC_RADIUS, 3);

	m_TempBitmapOCT = np::Array<BYTE, 2>(m_nAlines, m_nScansFFT / 2);
	m_TempBitmapCircOCT = np::Array<BYTE, 2>(2 * CIRC_RADIUS, 2 * CIRC_RADIUS);

	m_RingIntensity = np::Array<BYTE>(m_nAlines);
	m_RingLifetime = np::Array<BYTE>(m_nAlines);
	m_RingEnhanced = np::Array<BYTE>(m_nAlines * 3);

#if INTENSITY_MAP_FIRE
	int lut = LUT_FIRE;
#else
	int lut = m_ComboResFlimShow + LUT_PURPLE;
#endif
	m_pBitmapProjection = new MyBitmap;
	m_pBitmapProjection->SetBitmap(m_nFrame, m_nAlines, 1, m_ComboResLut); // inv gray
	m_pBitmapIntensityMap = new MyBitmap;
	m_pBitmapIntensityMap->SetBitmap(m_nFrame, m_nAlines, 1, lut);
	m_pBitmapLifetimeMap = new MyBitmap;
	m_pBitmapLifetimeMap->SetBitmap(m_nFrame, m_nAlines, 1, LUT_HSV); // hsv
	m_pBitmapEnhancedMap = new MyBitmap;
	m_pBitmapEnhancedMap->SetBitmap(m_nFrame, m_nAlines, 3); // RGB space

	// Colorbar Initialization
	m_pColorbarProjection = new MyBitmap;
	m_pColorbarProjection->SetBitmap(256, 4, 1, LUT_INV_GRAY); // inv gray
	m_pColorbarIntensity = new MyBitmap;
	m_pColorbarIntensity->SetBitmap(256, 4, 1, lut);
	m_pColorbarLifetime = new MyBitmap;
	m_pColorbarLifetime->SetBitmap(256, 4, 1, LUT_HSV);

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			m_pColorbarProjection->SetPtr((BYTE)i, i, j);
			m_pColorbarIntensity->SetPtr((BYTE)i, i, j);
			m_pColorbarLifetime->SetPtr((BYTE)i, i, j);
		}
	}

	// Medfilt for projection
	proj_medfilt = medfilt(m_nAlines, m_nFrame, 5, 3);
	intensity4_medfilt = medfilt(m4_nAlines, m_nFrame, 5, 3);
	lifetime4_medfilt = medfilt(m4_nAlines, m_nFrame,  11, 7); //7, 5);
}


void CVisResult::SetBuffers(int nFrame)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Clear Previous Buffers
	std::vector<np::Array<float, 2>> clear_vector1;
	clear_vector1.swap(m_OCT_Image32f);

	std::vector<np::Array<float, 2>> clear_vector2;
	clear_vector2.swap(m_IntensityMap32f);

	std::vector<np::Array<float, 2>> clear_vector3;
	clear_vector3.swap(m_LifetimeMap32f);

	std::vector<np::Array<float, 2>> clear_vector4;
	clear_vector4.swap(m_PulseCrop);

	std::vector<np::Array<float, 2>> clear_vector5;
	clear_vector5.swap(m_PulseMask);

//	std::vector<np::Array<float, 2>> clear_vector6;
//	clear_vector6.swap(m_PulseSpline);

	// Data Buffers
	for (int i = 0; i < nFrame; i++)
	{
		np::Array<float, 2> buffer = np::Array<float, 2>(m_nScansFFT / 2, m_nAlines);
		m_OCT_Image32f.push_back(buffer);		
	}
	for (int i = 0; i < 3; i++)
	{
		np::Array<float, 2> intensity = np::Array<float, 2>(m4_nAlines, nFrame);
		np::Array<float, 2> lifetime = np::Array<float, 2>(m4_nAlines, nFrame);
		m_IntensityMap32f.push_back(intensity);
		m_LifetimeMap32f.push_back(lifetime);
	}
	m_Projection32f = np::Array<float, 2>(m_nAlines, nFrame);

	// Visualization Buffers
	m_OCT_Image8u = np::Array<BYTE, 2>(m_nScansFFT / 2, m_nAlines);
}


void CVisResult::PostNcDestroy()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	std::vector<np::Array<float, 2>>().swap(m_OCT_Image32f);
	std::vector<np::Array<float, 2>>().swap(m_IntensityMap32f);
	std::vector<np::Array<float, 2>>().swap(m_LifetimeMap32f);
	std::vector<np::Array<float, 2>>().swap(m_PulseCrop);
	std::vector<np::Array<float, 2>>().swap(m_PulseMask);
//	std::vector<np::Array<float, 2>>().swap(m_PulseSpline);

	if (m_pFLIMforExt) delete m_pFLIMforExt;

	if (m_pBitmapOCT) delete m_pBitmapOCT;
	if (m_pBitmapCircOCT) delete m_pBitmapCircOCT;

	if (m_pBitmapProjection) delete m_pBitmapProjection;
	if (m_pBitmapIntensityMap) delete m_pBitmapIntensityMap;
	if (m_pBitmapLifetimeMap) delete m_pBitmapLifetimeMap;
	if (m_pBitmapEnhancedMap) delete m_pBitmapEnhancedMap;

	if (m_pColorbarProjection) delete m_pColorbarProjection;
	if (m_pColorbarIntensity) delete m_pColorbarIntensity;
	if (m_pColorbarLifetime) delete m_pColorbarLifetime;

	CDialogEx::PostNcDestroy();
}


void CVisResult::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.	
	CPoint pt1 = point, pt2;
	pt1.x = pt1.x + pt.x;
	pt1.y = pt1.y + pt.y;

	if (m_nFrame != 1)
	{
		if (m_ResImageRect.PtInRect(pt1) && (!m_CheckCircularize))
		{
			pt2 = { pt1.x - m_ResImageRect.left, pt1.y - m_ResImageRect.top };

			pt2.x = (LONG)floor(double(m_nAlines * pt2.x) / double(m_ResImageRect.Width()));
			pt2.y = (LONG)floor(double(m_nScansFFT / 2 * pt2.y) / double(m_ResImageRect.Height() * 0.92));

			if (pt2.y < m_nScansFFT / 2)
			{
				CString str;
				str.Format(_T("(%4d, %4d) %3d (%.1f dB) / Ch. %d / %.3f / %.3f nsec"),
					pt2.x, pt2.y, *m_pBitmapOCT->GetPtr(m_nScansFFT / 2 - 1 - pt2.y, pt2.x), m_OCT_Image32f.at(m_SliderCurFrame)(pt2.y, pt2.x),
					m_ComboResFlimShow + 1, m_IntensityMap32f.at(m_ComboResFlimShow)(pt2.x / 4, m_SliderCurFrame), m_LifetimeMap32f.at(m_ComboResFlimShow)(pt2.x / 4, m_SliderCurFrame));
				m_pMainDlg->m_StatusBar.SetText(str, 1, 0);
			}
		}
		else
		{
			bool isIn = false;
			if (m_ResProjectionRect.PtInRect(pt1))
			{
				pt2 = { pt1.x - m_ResProjectionRect.left, pt1.y - m_ResProjectionRect.top };
				isIn = true;
			}
			else if (m_ResIntensityMapRect.PtInRect(pt1))
			{
				pt2 = { pt1.x - m_ResIntensityMapRect.left, pt1.y - m_ResIntensityMapRect.top };
				isIn = true;
			}
			else if (m_ResLifetimeMapRect.PtInRect(pt1))
			{
				pt2 = { pt1.x - m_ResLifetimeMapRect.left, pt1.y - m_ResLifetimeMapRect.top };
				isIn = true;
			}

			if (isIn)
			{
				pt2.x = (LONG)floor(double(m4_nAlines * pt2.x) / double(m_ResProjectionRect.Width()));
				pt2.y = (LONG)floor(double(m_nFrame * pt2.y) / double(m_ResProjectionRect.Height()));
				pt2.y = m_nFrame - pt2.y - 1;

				CString str;
				str.Format(_T("(%4d, %4d) %3d (%.1f dB) / Ch. %d / %.3f / %.3f nsec"),
					4 * pt2.x, pt2.y, *m_pBitmapProjection->GetPtr(pt2.y, pt2.x), m_Projection32f(pt2.x, pt2.y),
					m_ComboResFlimShow + 1, m_IntensityMap32f.at(m_ComboResFlimShow)(pt2.x, pt2.y), m_LifetimeMap32f.at(m_ComboResFlimShow)(pt2.x, pt2.y));
				m_pMainDlg->m_StatusBar.SetText(str, 1, 0);
			}
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void CVisResult::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CPoint pt1 = point;
	pt1.x = pt1.x + pt.x;
	pt1.y = pt1.y + pt.y;

	if (m_CheckCircularize)
	{
		if (m_pPulseReviewDlg && m_ResImageRect.PtInRect(pt1))
		{
			pt1 = { pt1.x - m_ResImageRect.left, pt1.y - m_ResImageRect.top };

			int circ_center = m_ResImageRect.Width() / 2;
			CPoint pt2 = pt1; pt2.Offset(-circ_center, -circ_center); 

			double len = sqrt(pt2.x * pt2.x + pt2.y * pt2.y);
			double theta = acos(pt2.x / len) * 360.0 / IPP_2PI;
			if (pt2.y > 0)
				theta = 360 - theta;

			int aline4_pos = (int)(theta / 360.0 * (double)m4_nAlines);		
			
			m_pPulseReviewDlg->m_SliderCurAline = aline4_pos;
			m_pPulseReviewDlg->UpdateData(FALSE);

			CString str;
			str.Format(_T("Current A-line : %4d / %d"), 4 * (aline4_pos + 1), m_nAlines);
			m_pPulseReviewDlg->SetDlgItemText(IDC_STATIC_REVIEW_CUR_ALINE, str);

			m_pPulseReviewDlg->InvalidateRect(m_pPulseReviewDlg->m_RectPulseGraphWnd, FALSE);

			if (m_CheckShowGuideLine)
				InvalidateRect(m_VisRegionRectWnd, FALSE);
		}

		if (m_ResImageRect.PtInRect(pt1) && m_ToggleMeasureDistance)
		{
			pt_dist1[n_click] = { point.x, point.y };
			pt1 = { pt1.x - m_ResImageRect.left, pt1.y - m_ResImageRect.top };

			pt_dist2[n_click].x = (LONG)floor(double(2 * CIRC_RADIUS * pt1.x) / double(m_ResImageRect.Width()));
			pt_dist2[n_click].y = (LONG)floor(double(2 * CIRC_RADIUS * pt1.y) / double(m_ResImageRect.Width()));

			if (pt_dist2[n_click].y < 2 * CIRC_RADIUS)
			{
				CClientDC dc(this);
				CBrush brush; brush.CreateSolidBrush(RGB(255, 0, 255));
				dc.SelectStockObject(NULL_PEN);
				dc.SelectObject(&brush);
				dc.Ellipse(pt_dist1[n_click].x - 2, pt_dist1[n_click].y - 2, pt_dist1[n_click].x + 2, pt_dist1[n_click].y + 2);

				n_click++;

				if (n_click == 2)
				{
					CPen pen(PS_SOLID, 1, RGB(255, 0, 255));
					dc.SelectObject(&pen);
					dc.MoveTo(pt_dist1[0].x, pt_dist1[0].y);
					dc.LineTo(pt_dist1[1].x, pt_dist1[1].y);

					UpdateData(TRUE);
					double dist = PIXEL_SIZE / 1000 * sqrt((pt_dist2[0].x - pt_dist2[1].x) * (pt_dist2[0].x - pt_dist2[1].x) + (pt_dist2[0].y - pt_dist2[1].y) * (pt_dist2[0].y - pt_dist2[1].y));
					CString str;
					str.Format(_T("%.3f mm"), dist);

					dc.SetTextColor(RGB(255, 0, 255));
					dc.TextOutW((pt_dist1[0].x + pt_dist1[1].x) / 2, (pt_dist1[0].y + pt_dist1[1].y) / 2, str);

					m_ToggleMeasureDistance = FALSE;
					UpdateData(FALSE);
				}
			}
		}
		else
		{
			if (!m_ToggleMeasureDistance)
				InvalidateRect(m_VisRegionRectWnd, FALSE);
		}		
	}
	else
	{
		if (m_pPulseReviewDlg && m_ResImageRect.PtInRect(pt1))
		{						
			pt1 = { pt1.x - m_ResImageRect.left, pt1.y - m_ResImageRect.top };
			int aline4_pos = (int)floor(double(m4_nAlines * pt1.x) / double(m_ResImageRect.Width()));

			m_pPulseReviewDlg->m_SliderCurAline = aline4_pos;
			m_pPulseReviewDlg->UpdateData(FALSE);

			CString str;
			str.Format(_T("Current A-line : %4d / %d"), 4 * (aline4_pos + 1), m_nAlines);
			m_pPulseReviewDlg->SetDlgItemText(IDC_STATIC_REVIEW_CUR_ALINE, str);

			m_pPulseReviewDlg->InvalidateRect(m_pPulseReviewDlg->m_RectPulseGraphWnd, FALSE);

			if (m_CheckShowGuideLine)
				InvalidateRect(m_VisRegionRectWnd, FALSE);			
		}
	}

	if (m_ResProjectionRect.PtInRect(pt1) || m_ResIntensityMapRect.PtInRect(pt1) || m_ResLifetimeMapRect.PtInRect(pt1))
	{
		if (m_ResProjectionRect.PtInRect(pt1))
			pt1 = { pt1.x - m_ResProjectionRect.left, pt1.y - m_ResProjectionRect.top };
		else if (m_ResIntensityMapRect.PtInRect(pt1))
			pt1 = { pt1.x - m_ResIntensityMapRect.left, pt1.y - m_ResIntensityMapRect.top };
		else if (m_ResLifetimeMapRect.PtInRect(pt1))
			pt1 = { pt1.x - m_ResLifetimeMapRect.left, pt1.y - m_ResLifetimeMapRect.top };
		
		int frame_pos = (int)floor(double(m_nFrame * (m_ResProjectionRect.Height() - pt1.y)) / double(m_ResProjectionRect.Height()));

		m_SliderCurFrame = frame_pos;
		UpdateData(FALSE);

		CString str;
		str.Format(_T("Current Frame : %4d / %d"), m_SliderCurFrame + 1, m_nFrame);
		SetDlgItemText(IDC_STATIC_CUR_FRAME, str);

		InvalidateRect(m_VisRegionRectWnd, FALSE);

		if (m_pPulseReviewDlg)
			m_pPulseReviewDlg->InvalidateRect(m_pPulseReviewDlg->m_RectPulseGraphWnd, FALSE);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CVisResult::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_nFrame > 1)
	{
		CPoint pt1 = point;
		pt1.x = pt1.x + pt.x;
		pt1.y = pt1.y + pt.y;

		if (m_ResImageRect.PtInRect(pt1))
		{
			if (!m_pPulseReviewDlg)
			{
				m_pPulseReviewDlg = new PulseReviewDlg;
				m_pPulseReviewDlg->SetMainDlg(m_pMainDlg);
				m_pPulseReviewDlg->Create(IDD_DIALOG_PULSE_REVIEW);
				m_pPulseReviewDlg->ShowWindow(SW_SHOW);
				m_pPulseReviewDlg->MoveWindow(CRect(0, 0, 626, 359));
			}
			else
				m_pPulseReviewDlg->SetFocus();

			m_CheckShowGuideLine = TRUE;
			UpdateData(FALSE);
			OnBnClickedCheckResShowGuideline();
		}
	}

	CDialogEx::OnLButtonDblClk(nFlags, point);
}


BOOL CVisResult::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CVisResult::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

}


void CVisResult::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

}


void CVisResult::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.

	CPaintDC dcResImage(&m_StaticResImage);
	CPaintDC dcProjection(&m_StaticResProjection);
	CPaintDC dcIntensityMap(&m_StaticResIntensityMap);
	CPaintDC dcLifetimeMap(&m_StaticResLifetimeMap);
	CPaintDC dcCBar1(&m_StaticResColorbarIntensity);
	CPaintDC dcCBar2(&m_StaticResColorbarLifetime);
	CPaintDC dcCBar3(&m_StaticResColorbarProjection);
	CDC dcMem;

	// Assigning memory DC
	dcMem.CreateCompatibleDC(&dc);
	CBitmap btmp;
	btmp.CreateCompatibleBitmap(&dc, m_VisRegionRectWnd.Width(), m_VisRegionRectWnd.Height());
	dcMem.SelectObject(&btmp);

	// Drawing OCT Projection & FLIM en face map
	SetStretchBltMode(dcMem.m_hDC, COLORONCOLOR);

	MyBitmap* pSelectedLifetimeMap = (!m_CheckHSVEnhancing) ? m_pBitmapLifetimeMap : m_pBitmapEnhancedMap;

	StretchDIBits(dcMem.m_hDC,
		m_ResProjectionRect.left, m_ResProjectionRect.top, m_ResProjectionRect.Width(), m_ResProjectionRect.Height(),
		0, 0, m_pBitmapProjection->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapProjection->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pBitmapProjection->GetPtr(), m_pBitmapProjection->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
	StretchDIBits(dcMem.m_hDC,
		m_ResIntensityMapRect.left, m_ResIntensityMapRect.top, m_ResIntensityMapRect.Width(), m_ResIntensityMapRect.Height(),
		0, 0, m_pBitmapIntensityMap->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapIntensityMap->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pBitmapIntensityMap->GetPtr(), m_pBitmapIntensityMap->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
	StretchDIBits(dcMem.m_hDC,
		m_ResLifetimeMapRect.left, m_ResLifetimeMapRect.top, m_ResLifetimeMapRect.Width(), m_ResLifetimeMapRect.Height(),
		0, 0, pSelectedLifetimeMap->GetLpBmpInfo()->bmiHeader.biWidth, pSelectedLifetimeMap->GetLpBmpInfo()->bmiHeader.biHeight,
		pSelectedLifetimeMap->GetPtr(), pSelectedLifetimeMap->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

	int cur_frame = int(double((m_nFrame - m_SliderCurFrame) * m_ResProjectionRect.Height()) / (double)(m_nFrame));
	dcMem.SelectObject(&m_GreenPen);
	dcMem.MoveTo(m_ResProjectionRect.left, m_ResProjectionRect.top + cur_frame); dcMem.LineTo(m_ResProjectionRect.right, m_ResProjectionRect.top + cur_frame);
	dcMem.SelectObject(&m_GreenPen);
	dcMem.MoveTo(m_ResIntensityMapRect.left, m_ResIntensityMapRect.top + cur_frame); dcMem.LineTo(m_ResIntensityMapRect.right, m_ResIntensityMapRect.top + cur_frame);
	dcMem.SelectObject(&m_WhitePen);
	dcMem.MoveTo(m_ResLifetimeMapRect.left, m_ResLifetimeMapRect.top + cur_frame); dcMem.LineTo(m_ResLifetimeMapRect.right, m_ResLifetimeMapRect.top + cur_frame);

	// Scaling OCT image
	float* OCT_image = m_OCT_Image32f.at(m_SliderCurFrame);
	ScaleRect(OCT_image, m_OCT_Image8u);
	TransposeRect(m_OCT_Image8u, m_TempBitmapOCT);
	m_pBitmapOCT->IndToRGB(m_TempBitmapOCT.raw_ptr(), m_ComboResLut);

	// Select FLIM Information
	memcpy(m_RingIntensity.raw_ptr(), m_pBitmapIntensityMap->GetPtr() + m_nAlines * m_SliderCurFrame, sizeof(BYTE) * m_nAlines);
	np::Array<BYTE>* pRing = (!m_CheckHSVEnhancing) ? &m_RingLifetime : &m_RingEnhanced;
	memcpy(pRing->raw_ptr(), pSelectedLifetimeMap->GetPtr() + pSelectedLifetimeMap->GetChannel() * m_nAlines * m_SliderCurFrame,
		sizeof(BYTE) * pSelectedLifetimeMap->GetChannel() * m_nAlines);
	
	// Rectangular Image Drawing
	if (!m_CheckCircularize)
	{
		// Draw FLIM Rings
		int ring_thickness = 80;
		for (int i = 0; i < ring_thickness; i++)
		{
			if (!m_CheckHSVEnhancing)
				m_pBitmapOCT->IndToRGB_line(pRing->raw_ptr(), i, LUT_HSV);
			else
				memcpy(m_pBitmapOCT->GetPtr(i), pRing->raw_ptr(), sizeof(BYTE) * pRing->length());

			m_pBitmapOCT->IndToRGB_line(m_RingIntensity, ring_thickness + i, LUT_FIRE);
		}

		StretchDIBits(dcMem.m_hDC,
			m_ResImageRect.left, m_ResImageRect.top, m_ResImageRect.Width(), m_ResImageRect.Height(),
			0, 0, m_pBitmapOCT->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapOCT->GetLpBmpInfo()->bmiHeader.biHeight,
			m_pBitmapOCT->GetPtr(), m_pBitmapOCT->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

		// Draw Guide Lines
		if (m_CheckShowGuideLine)
		{
			int circ_center = int(double(m_EditCircCenter * m_ResImageRect.Height()) / (double)(m_nScansFFT / 2));
			int proj_offset = int(double((m_EditCircCenter + PROJECTION_OFFSET) * m_ResImageRect.Height()) / (double)(m_nScansFFT / 2));
			int circ_end = int(double((m_EditCircCenter + CIRC_RADIUS) * m_ResImageRect.Height()) / (double)(m_nScansFFT / 2));
			dcMem.SelectObject(&m_RedPen);
			dcMem.MoveTo(m_ResImageRect.left, m_ResImageRect.top + circ_center); dcMem.LineTo(m_ResImageRect.right, m_ResImageRect.top + circ_center);
			dcMem.MoveTo(m_ResImageRect.left, m_ResImageRect.top + proj_offset); dcMem.LineTo(m_ResImageRect.right, m_ResImageRect.top + proj_offset);
			dcMem.MoveTo(m_ResImageRect.left, m_ResImageRect.top + circ_end); dcMem.LineTo(m_ResImageRect.right, m_ResImageRect.top + circ_end);

			if (m_pPulseReviewDlg)
			{
				int cur_aline = int(double(4 * m_pPulseReviewDlg->m_SliderCurAline * m_ResImageRect.Width()) / (double)(m_nAlines));
				dcMem.MoveTo(m_ResImageRect.left + cur_aline, m_ResImageRect.top); 
				dcMem.LineTo(m_ResImageRect.left + cur_aline, m_ResImageRect.bottom);
			}
		}

		dcResImage.BitBlt(0, 0, m_ResImageRect.Width(), m_ResImageRect.Height(), &dcMem, m_ResImageRect.left, m_ResImageRect.top, SRCCOPY);
	}
	// Circular Image Drawing
	else
	{
		// Draw FLIM Rings
		int ring_thickness = 80;
		for (int i = 0; i < ring_thickness; i++)
		{
			int circ_offset = m_nScansFFT / 2 - 1 - m_EditCircCenter - CIRC_RADIUS + i;

			if (!m_CheckHSVEnhancing)
				m_pBitmapOCT->IndToRGB_line(pRing->raw_ptr(), circ_offset, LUT_HSV);
			else
				memcpy(m_pBitmapOCT->GetPtr(circ_offset), pRing->raw_ptr(), sizeof(BYTE) * pRing->length());

			m_pBitmapOCT->IndToRGB_line(m_RingIntensity, ring_thickness + circ_offset, LUT_FIRE);
		}

		// Circularize OCT Image
		IppiSize roi_oct = { m_nScansFFT / 2, m_nAlines };
		IppiSize roi_oct1 = { m_nAlines, m_nScansFFT / 2 };
		for (int i = 0; i < 3; i++)
		{
			np::Array<BYTE, 2> temp_rect(m_pBitmapOCT->GetChannelImg(i), m_nAlines, m_nScansFFT / 2);
			np::Array<BYTE, 2> temp_rect1(m_nScansFFT / 2, m_nAlines);

			ippiTranspose_8u_C1R(temp_rect.raw_ptr(), roi_oct1.width * sizeof(uint8_t), temp_rect1.raw_ptr(), roi_oct1.height * sizeof(uint8_t), roi_oct1);
			ippiMirror_8u_C1IR(temp_rect1.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct, ippAxsVertical);			
			
			circ_OCT(temp_rect1, m_TempBitmapCircOCT, m_EditCircCenter);
			m_pBitmapCircOCT->PutChannelImg(m_TempBitmapCircOCT, i);
		}

		StretchDIBits(dcMem.m_hDC,
			m_ResImageRect.left, m_ResImageRect.top, m_ResImageRect.Width(), m_ResImageRect.Width(),
			0, 0, m_pBitmapCircOCT->GetLpBmpInfo()->bmiHeader.biWidth, m_pBitmapCircOCT->GetLpBmpInfo()->bmiHeader.biHeight,
			m_pBitmapCircOCT->GetPtr(), m_pBitmapCircOCT->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

		// Draw Guide Lines
		if (m_CheckShowGuideLine)
		{
			if (m_pPulseReviewDlg)
			{
				int circ_center = m_ResImageRect.Width() / 2; // int(double(m_EditCircCenter * m_ResImageRect.Width()) / (double)(m_nScansFFT / 2));

				int circ_x = (int)(circ_center + (double)circ_center * cos((double)m_pPulseReviewDlg->m_SliderCurAline / (double)m4_nAlines * IPP_2PI));
				int circ_y = (int)(circ_center - (double)circ_center * sin((double)m_pPulseReviewDlg->m_SliderCurAline / (double)m4_nAlines * IPP_2PI));

				dcMem.SelectObject(&m_RedPen);
				dcMem.MoveTo(circ_center, circ_center); dcMem.LineTo(circ_x, circ_y);
			}
		}

		dcResImage.BitBlt(0, 0, m_ResImageRect.Width(), m_ResImageRect.Width(), &dcMem, m_ResImageRect.left, m_ResImageRect.top, SRCCOPY);
	}

	// Drawing Colorbars
	StretchDIBits(dcMem.m_hDC,
		m_CBarRegionRect1.left, m_CBarRegionRect1.top, m_CBarRegionRect1.Width(), m_CBarRegionRect1.Height(),
		0, 0, m_pColorbarIntensity->GetLpBmpInfo()->bmiHeader.biWidth, m_pColorbarIntensity->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pColorbarIntensity->GetPtr(), m_pColorbarIntensity->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
	StretchDIBits(dcMem.m_hDC,
		m_CBarRegionRect2.left, m_CBarRegionRect2.top, m_CBarRegionRect2.Width(), m_CBarRegionRect2.Height(),
		0, 0, m_pColorbarLifetime->GetLpBmpInfo()->bmiHeader.biWidth, m_pColorbarLifetime->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pColorbarLifetime->GetPtr(), m_pColorbarLifetime->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);
	StretchDIBits(dcMem.m_hDC,
		m_CBarRegionRect3.left, m_CBarRegionRect3.top, m_CBarRegionRect3.Width(), m_CBarRegionRect3.Height(),
		0, 0, m_pColorbarProjection->GetLpBmpInfo()->bmiHeader.biWidth, m_pColorbarProjection->GetLpBmpInfo()->bmiHeader.biHeight,
		m_pColorbarProjection->GetPtr(), m_pColorbarProjection->GetLpBmpInfo(), DIB_RGB_COLORS, SRCCOPY);

	// Memory DC transfer	
	dcProjection.BitBlt(0, 0, m_ResProjectionRect.Width(), m_ResProjectionRect.Height(), &dcMem, m_ResProjectionRect.left, m_ResProjectionRect.top, SRCCOPY);
	dcIntensityMap.BitBlt(0, 0, m_ResIntensityMapRect.Width(), m_ResIntensityMapRect.Height(), &dcMem, m_ResIntensityMapRect.left, m_ResIntensityMapRect.top, SRCCOPY);
	dcLifetimeMap.BitBlt(0, 0, m_ResLifetimeMapRect.Width(), m_ResLifetimeMapRect.Height(), &dcMem, m_ResLifetimeMapRect.left, m_ResLifetimeMapRect.top, SRCCOPY);
	dcCBar1.BitBlt(0, 0, m_CBarRegionRect1.Width(), m_CBarRegionRect1.Height(), &dcMem, m_CBarRegionRect1.left, m_CBarRegionRect1.top, SRCCOPY);
	dcCBar2.BitBlt(0, 0, m_CBarRegionRect2.Width(), m_CBarRegionRect2.Height(), &dcMem, m_CBarRegionRect2.left, m_CBarRegionRect2.top, SRCCOPY);
	dcCBar3.BitBlt(0, 0, m_CBarRegionRect3.Width(), m_CBarRegionRect3.Height(), &dcMem, m_CBarRegionRect3.left, m_CBarRegionRect3.top, SRCCOPY);

}


void CVisResult::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME);

	if (pScrollBar == (CScrollBar*)pSlider)
	{
		UpdateData(TRUE);

		CString str;
		str.Format(_T("Current Frame : %3d / %3d"), m_SliderCurFrame + 1, m_nFrame);
		SetDlgItemText(IDC_STATIC_CUR_FRAME, str);
		InvalidateRect(m_VisRegionRectWnd, FALSE);

		if (m_pPulseReviewDlg)
			m_pPulseReviewDlg->InvalidateRect(m_pPulseReviewDlg->m_RectPulseGraphWnd, FALSE);
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CVisResult::OnSetRadioSelectData(UINT value)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	switch (m_RadioSelectData)
	{
	case 0:
		break;
	case 1:
		GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
		break;
	}
	UpdateData(FALSE);
}


void CVisResult::OnCbnSelchangeComboResLut()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	m_pBitmapOCT->SetColormap(m_ComboResLut);
	m_pBitmapCircOCT->SetColormap(m_ComboResLut);
	m_pBitmapProjection->SetColormap(m_ComboResLut);
	m_pColorbarProjection->SetColormap(m_ComboResLut);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnCbnSelchangeComboResFlimShow()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	int ch;
	switch (m_ComboResFlimShow)
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

	m_pBitmapIntensityMap->SetColormap(ch);
	m_pColorbarIntensity->SetColormap(ch);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnBnClickedCheckResHsvEnhancing()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	SetDlgItemText(IDC_STATIC_LIFETIME_TITLE, m_CheckHSVEnhancing ? _T("Lifetime (HSV Enhancing)") : _T("Lifetime"));

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnBnClickedCheckResNormalization()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnBnClickedCheckResMeasureDistance()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_ToggleMeasureDistance)
	{
		n_click = 0;
		InvalidateRect(m_VisRegionRectWnd, FALSE);
	}
}


void CVisResult::OnBnClickedButtonImageProcessing()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (!m_RadioSelectData)
		ImageProcessingInBuffer();
	else
		ImageProcessingExternal();
}


void CVisResult::OnEnChangeEditResAlineMaxDb()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	SetDlgItemInt(IDC_STATIC_RES_PROJECTION_MAX, m_EditResMaxDb);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnEnChangeEditResAlineMinDb()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	SetDlgItemInt(IDC_STATIC_RES_PROJECTION_MIN, m_EditResMinDb);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnEnChangeEditResCircCenter()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	if (m_EditCircCenter > (m_nScansFFT / 2 - CIRC_RADIUS))
	{
		m_EditCircCenter = m_nScansFFT / 2 - CIRC_RADIUS - 1;
		UpdateData(FALSE);
	}
	if (m_nFrame != 1)
	{
		m_Projection32f = np::Array<float, 2>(m_nAlines, m_nFrame);
		GetProjection(m_OCT_Image32f, m_Projection32f, m_EditCircCenter);
	}

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnEnChangeEditResIntensityMax()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_RES_INTENSITY_MAX, str);
	m_EditMaxIntensity = (float)_wtof(str);

	SetDlgItemText(IDC_STATIC_RES_COLORBAR_MAX, str);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnEnChangeEditResIntensityMin()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_RES_INTENSITY_MIN, str);
	m_EditMinIntensity = (float)_wtof(str);

	SetDlgItemText(IDC_STATIC_RES_COLORBAR_MIN, str);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnEnChangeEditResLifetimeMax()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_RES_LIFETIME_MAX, str);
	m_EditMaxLifetime = (float)_wtof(str);

	SetDlgItemText(IDC_STATIC_RES_COLORBAR_MAX1, str);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnEnChangeEditResLifetimeMin()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	GetDlgItemText(IDC_EDIT_RES_LIFETIME_MIN, str);
	m_EditMinLifetime = (float)_wtof(str);

	SetDlgItemText(IDC_STATIC_RES_COLORBAR_MIN1, str);

	// Make En Face Map
	MaskEnFaceMap();

	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnBnClickedCheckResShowGuideline()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	InvalidateRect(m_VisRegionRectWnd, FALSE);
}


void CVisResult::OnBnClickedCheckCircularize()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	GetDlgItem(IDC_CHECK_RES_MEASURE_DISTANCE)->EnableWindow(m_CheckCircularize);
	InvalidateRect(m_VisRegionRectWnd, TRUE);
}


void CVisResult::OnBnClickedCheckResSaveAsRgb()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CVisResult::ImageProcessingInBuffer()
{
	std::thread im_proc([&]() {

		// Initialization
		SetCircs(m_pMainDlg->m_Edit_nScans, m_pMainDlg->m_Edit_nAlines);

		// Get Range
		m_nFrame = 1;
		int nFrame = m_pMainDlg->m_pDataRecording->nRecordedFrames;
		printf("Start in-buffer image processing... (Total nFrame: %d)\n", nFrame);

		m_ProgressRes.SetRange(0, nFrame);
		m_ProgressRes.SetPos(0);

		CString str;
		str.Format(_T("Current Frame : %3d / %3d"), 1, nFrame);
		SetDlgItemText(IDC_STATIC_CUR_FRAME, str);

		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetPos(0);
		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(FALSE);

		// Set Buffers
		SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Setting buffers..."));
		SetBuffers(nFrame);

		// Set OCT FLIM Object //////////////////////////////////////////////////////////////////////////////////	
		OCTProcess* pOCT = m_pMainDlg->m_pOCT;
		m_pFLIMforExt = m_pMainDlg->m_pFLIM;

		// Image Processing	Buffers /////////////////////////////////////////////////////////////////////////////
		std::queue<uint16_t*> fringe, pulse;
		int N_buffer = nFrame / 5; // (nFrame / 2 < NUM_BUFFER_FRAME) ? nFrame / 2 : NUM_BUFFER_FRAME;
		for (int i = 0; i < N_buffer; i++)
		{
			uint16_t* buffer1 = new uint16_t[2 * m_nScans * m_nAlines];
			memset(buffer1, 0, 2 * m_nScans * m_nAlines * sizeof(uint16_t));
			fringe.push(buffer1);

			uint16_t* buffer2 = new uint16_t[m_nScans * m_nAlines];
			memset(buffer2, 0, m_nScans * m_nAlines * sizeof(uint16_t));
			pulse.push(buffer2);
		}

		np::Array<float, 2> itn(m4_nAlines, 4); // temp intensity
		np::Array<float, 2> md(m4_nAlines, 4); // temp meandelay
		np::Array<float, 2> ltm(m4_nAlines, 3); // temp lifetime

		// Synchronization Objects ///////////////////////////////////////////////////////////////////////////////
		Queue<uint16_t*> Queue_oct, Queue_flim;
		std::mutex mutex_oct, mutex_flim;

		// Data DeInterleaving //////////////////////////////////////////////////////////////////////////////////
		SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Image processing..."));
		std::thread deinterleave([&]() {
			int frame_count = 0;

			for (int i = 0; i < NUM_BUFFER_FRAME - nFrame; i++)
			{
				uint16_t* temp = m_pMainDlg->m_awBufferQueue.front();
				m_pMainDlg->m_awBufferQueue.pop();
				m_pMainDlg->m_awBufferQueue.push(temp);
			}

			while (frame_count < nFrame)
			{
				// Get a frame buffer from raw data queue
				uint16_t* fringe_data = nullptr;
				{
					std::unique_lock<std::mutex> lock(mutex_oct);
					if (!fringe.empty())
					{
						fringe_data = fringe.front();
						fringe.pop();
					}
				}

				if (fringe_data)
				{
					// Data deinterleaving
					uint16_t* frame = m_pMainDlg->m_awBufferQueue.front();
					m_pMainDlg->m_awBufferQueue.pop();
					ippsCplxToReal_16sc((Ipp16sc *)frame, (Ipp16s *)fringe_data, (Ipp16s *)(fringe_data + m_nSizeFrame), m_nSizeFrame);
					frame_count++;

					// Recycle data
					m_pMainDlg->m_awBufferQueue.push(frame);

					// Push to sync Queue
					Queue_oct.push(fringe_data);
				}
			}

			CString str; str.Format(_T("Current Buffer Address: 0x%x"), m_pMainDlg->m_awBufferQueue.front());
			m_pMainDlg->SetDlgItemText(IDC_STATIC_BUFFER_ADDRESS, str);
		});

		// OCT Process /////////////////////////////////////////////////////////////////////////////
		std::thread oct_proc([&]() {
			int frame_count = 0;
			while (frame_count < nFrame)
			{
				// Get data from synch Queue
				uint16_t* pulse_data = nullptr;
				{
					std::unique_lock<std::mutex> lock(mutex_flim);
					if (!pulse.empty())
					{
						pulse_data = pulse.front();
						pulse.pop();
					}
				}

				if (pulse_data)
				{
					// Get data from synch Queue
					uint16_t* fringe_data = Queue_oct.pop();

					// OCT Process
					(*pOCT)(m_OCT_Image32f.at(frame_count++), fringe_data);
					memcpy(pulse_data, fringe_data + m_nScans * m_nAlines, sizeof(uint16_t) * m_nScans * m_nAlines);

					// Push to sync Queue
					Queue_flim.push(pulse_data);

					// Return fringe buffer to original queue
					{
						std::unique_lock<std::mutex> lock(mutex_oct);
						fringe.push(fringe_data);
					}
				}
			}
		});

		// FLIM Process /////////////////////////////////////////////////////////////////////////////
		std::thread flim_proc([&]() {
			int frame_count = 0;
			while (frame_count < nFrame)
			{
				// Get data from synch Queue
				uint16_t* pulse_data = Queue_flim.pop();

				// FLIM Process
				np::Array<uint16_t, 2> temp(pulse_data, m_nScans * 4, m4_nAlines);

				(*m_pFLIMforExt)(itn, md, ltm, m_pMainDlg->m_VisStream.m_Params_FLIM, m_pMainDlg->m_Edit_PreTrig, temp);
				memcpy(&m_IntensityMap32f.at(0)(0, frame_count), &itn(0, 1), sizeof(float) * m4_nAlines);
				memcpy(&m_IntensityMap32f.at(1)(0, frame_count), &itn(0, 2), sizeof(float) * m4_nAlines);
				memcpy(&m_IntensityMap32f.at(2)(0, frame_count), &itn(0, 3), sizeof(float) * m4_nAlines);
				memcpy(&m_LifetimeMap32f.at(0)(0, frame_count), &ltm(0, 0), sizeof(float) * m4_nAlines);
				memcpy(&m_LifetimeMap32f.at(1)(0, frame_count), &ltm(0, 1), sizeof(float) * m4_nAlines);
				memcpy(&m_LifetimeMap32f.at(2)(0, frame_count++), &ltm(0, 2), sizeof(float) * m4_nAlines);

				// Copy for Pulse Review
				np::Array<float, 2> crop(m_pFLIMforExt->_resize.nx, m_pFLIMforExt->_resize.ny);
				np::Array<float, 2> mask(m_pFLIMforExt->_resize.nx, m_pFLIMforExt->_resize.ny);

				memcpy(crop, m_pFLIMforExt->_resize.crop_src, crop.length() * sizeof(float));
				memcpy(mask, m_pFLIMforExt->_resize.mask_src, mask.length() * sizeof(float));

				m_PulseCrop.push_back(crop);
				m_PulseMask.push_back(mask);

				// Return fringe buffer to original queue
				{
					std::unique_lock<std::mutex> lock(mutex_flim);
					pulse.push(pulse_data);
				}

				// Status update
				m_ProgressRes.SetPos(frame_count);
			}
		});

		// Wait for threads end
		deinterleave.join();
		oct_proc.join();
		flim_proc.join();

		// Delete fringe queue
		for (int i = 0; i < N_buffer; i++)
		{
			uint16_t* buffer1 = fringe.front();
			fringe.pop();
			delete[] buffer1;

			uint16_t* buffer2 = pulse.front();
			pulse.pop();
			delete[] buffer2;
		}

		// Get OCT Projection
		GetProjection(m_OCT_Image32f, m_Projection32f, m_EditCircCenter);

		// Status Update
		m_nFrame = nFrame;
		m_AdjustGalvo = m_pMainDlg->m_ScrollAdjustGalvo;
		m_IsExternData = false;

		// Set Bitmap 
		SetBitmaps();

		// Status Update
		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetRange(0, m_nFrame - 1);
		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetPos(0);
		((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->EnableWindow(TRUE);
		m_SliderCurFrame = 0;
		UpdateData(FALSE);

		GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(TRUE);

		SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Ready"));
		printf("In-buffer image processing is successfully completed!\n");

		// Make En Face Map
		MaskEnFaceMap();

		InvalidateRect(m_VisRegionRectWnd, FALSE);
		if (m_pPulseReviewDlg) m_pPulseReviewDlg->InvalidateRect(m_pPulseReviewDlg->m_RectPulseGraphWnd, FALSE);
	});

	im_proc.detach();
}


void CVisResult::ImageProcessingExternal()
{
	std::thread im_proc([&]() {

		CString str = _T("OCT FLIM interleaved raw data (*.data) | *.data; | All Files (*.*) | *.*||");
		CFileDialog dlg(TRUE, _T("data"), NULL, OFN_HIDEREADONLY, str);

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		if (IDOK == dlg.DoModal())
		{
			CString fullpath = dlg.GetPathName();
			CString name = dlg.GetFileName();
			CString title = dlg.GetFileTitle();
			CString path = fullpath.Left(fullpath.GetLength() - name.GetLength());
			CString bg_path = path + title + (CString)_T(".background");
			CString calib_path = path + title + (CString)_T(".calibration");
			CString ini_path = path + title + (CString)_T(".ini");
			CString mask_path = path + title + (CString)_T(".flim_mask");
			m_path = path;

			HANDLE hFile = INVALID_HANDLE_VALUE;
			hFile = CreateFile(fullpath, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
				printf("Invalid external data!\n");
			else
			{
				// Read Ini File & Initialization ////////////////////////////////////////////////////////////////////////
				FLIM_PARAMS flim_params;

				TCHAR szBuf[MAX_PATH] = { 0, };
				CString strSection, strKey, strValue, strValue1;

				strSection = _T("configuration");

				strKey = _T("nScans");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue1.Format(_T("%s"), szBuf);

				strKey = _T("nAlines");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				SetCircs(_wtoi(strValue1), _wtoi(strValue));

				strKey = _T("GalvoAdjShift");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				m_AdjustGalvo = _wtoi(strValue);
				CScrollBar* pScroll = (CScrollBar*)m_pMainDlg->GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO);
				pScroll->SetScrollPos(m_AdjustGalvo);
				m_pMainDlg->m_ScrollAdjustGalvo = m_AdjustGalvo;

				strKey = _T("flimBg");
				//strKey = _T("FLIMBg");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				flim_params.bg = (float)_wtof(strValue);

				strKey = _T("flimCh");
				//strKey = _T("FLIMCh");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				flim_params.act_ch = _wtoi(strValue);

				strKey = _T("flimWidthFactor");
				//strKey = _T("FLIMwidth_factor");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				flim_params.width_factor = (float)_wtof(strValue);

				flim_params.samp_intv = 1000.0f / (float)ADC_RATE;

				strKey = _T("preTrigSamps");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				int pre_trig = _wtoi(strValue);

				for (int i = 0; i < 4; i++)
				{
					strKey.Format(_T("flimChStartInd_%d"), i);
					//strKey.Format(_T("ChannelStart_%d"), i);
					GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
					strValue.Format(_T("%s"), szBuf);
					//flim_params.ch_start[i] = (float)_wtof(strValue);
					//flim_params.ch_start_ind[i] = int(flim_params.ch_start[i] / flim_params.samp_intv);
					flim_params.ch_start_ind[i] = _wtoi(strValue);
					//flim_params.ch_start[i] = int(flim_params.ch_start[i] / flim_params.samp_intv);
				}

				for (int i = 1; i < 4; i++)
				{					
					strKey.Format(_T("flimDelayOffset_%d"), i);
					//strKey.Format(_T("DelayTimeOffset_%d"), i);
					GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
					strValue.Format(_T("%s"), szBuf);
					flim_params.delay_offset[i - 1] = (float)_wtof(strValue);
				}

				strKey = _T("DiscomValue");
				GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, ini_path);
				strValue.Format(_T("%s"), szBuf);
				int discom_val = _wtoi(strValue);

				// Set Range (다시 확인 필요) /////////////////////////////////////////////////////////////////////
				DWORD FileSize[2];
				FileSize[0] = GetFileSize(hFile, &FileSize[1]);
				QWORD TotalFileSize;

				TotalFileSize = FileSize[1];
				TotalFileSize <<= 32;
				TotalFileSize |= FileSize[0];

				m_nFrame = 1;
				int nFrame = int(TotalFileSize / sizeof(uint16_t) / m_nScans / m_nAlines / 2);

				printf("Start external image processing... (Total nFrame: %d)\n", nFrame);

				m_ProgressRes.SetRange(0, nFrame);
				m_ProgressRes.SetPos(0);

				CString str;
				str.Format(_T("Current Frame : %3d / %3d"), 1, nFrame);
				SetDlgItemText(IDC_STATIC_CUR_FRAME, str);

				((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetPos(0);
				((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(FALSE);

				// Set Buffers ////////////////////////////////////////////////////////////////////////////////////
				SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Setting buffers..."));
				SetBuffers(nFrame);

				// Set OCT FLIM Object ///////////////////////////////////////////////////////////////////////////
				m_pMainDlg->m_Edit_nAlines = m_nAlines;
				OCTProcess* pOCT = new OCTProcess(m_pMainDlg); // new OCT object..
				pOCT->LoadCalibration(discom_val, bg_path, calib_path);

				if (m_pFLIMforExt) if (m_pFLIMforExt != m_pMainDlg->m_pFLIM) delete m_pFLIMforExt;
				m_pFLIMforExt = new FLIMProcess;
				m_pFLIMforExt->_resize.initialize(flim_params,
					flim_params.ch_start_ind[3] - flim_params.ch_start_ind[0] + FLIM_CH_START_5, FLIM_SPLINE_FACTOR, m4_nAlines);
				m_pFLIMforExt->LoadMaskData(mask_path);
								
				// Image Processing	Buffers ///////////////////////////////////////////////////////////////////////////////	
				std::queue<uint16_t*> frame;
				std::queue<uint16_t*> fringe, pulse;
				int N_buffer = nFrame / 5;
				for (int i = 0; i < N_buffer; i++)
				{
					uint16_t* buffer1 = new uint16_t[2 * m_nScans * m_nAlines];
					memset(buffer1, 0, 2 * m_nScans * m_nAlines * sizeof(uint16_t));
					frame.push(buffer1);

					uint16_t* buffer2 = new uint16_t[2 * m_nScans * m_nAlines];
					memset(buffer2, 0, 2 * m_nScans * m_nAlines * sizeof(uint16_t));
					fringe.push(buffer2);

					uint16_t* buffer3 = new uint16_t[m_nScans * m_nAlines];
					memset(buffer3, 0, m_nScans * m_nAlines * sizeof(uint16_t));
					pulse.push(buffer3);
				}

				np::Array<float, 2> itn(m4_nAlines, 4); // temp intensity
				np::Array<float, 2> md(m4_nAlines, 4); // temp meandelay
				np::Array<float, 2> ltm(m4_nAlines, 3); // temp lifetime

				// Synchronization Objects ////////////////////////////////////////////////////
				Queue<uint16_t*> Queue_raw, Queue_oct, Queue_flim;
				std::mutex mutex_raw, mutex_oct, mutex_flim;

				// Get external data /////////////////////////////////////////////////////////////////////////////
				DWORD dwRead;
				SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Image processing..."));
				std::thread load_data([&]() {
					int frame_count = 0;
					while (frame_count < nFrame)
					{
						// Get a frame buffer from raw data queue
						uint16_t* frame_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_raw);
							if (!frame.empty())
							{
								frame_data = frame.front();
								frame.pop();
							}
						}

						if (frame_data)
						{
							// Read data from the external data 
							ReadFile(hFile, frame_data, sizeof(uint16_t) * 2 * m_nScans * m_nAlines, &dwRead, NULL);
							frame_count++;

							// Push to sync Queue
							Queue_raw.push(frame_data);
						}
					}
				});

				// Data DeInterleaving & FLIM Process //////////////////////////////////////////////////////////////////
				std::thread deinterleave([&]() {
					int frame_count = 0;
					while (frame_count < nFrame)
					{
						// Get data from synch Queue
						uint16_t* fringe_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_oct);
							if (!fringe.empty())
							{
								fringe_data = fringe.front();
								fringe.pop();
							}
						}

						if (fringe_data)
						{
							// Get data from synch Queue
							uint16_t* frame_data = Queue_raw.pop();

							// Data deinterleaving
							ippsCplxToReal_16sc((Ipp16sc *)frame_data, (Ipp16s *)fringe_data, (Ipp16s *)(fringe_data + m_nSizeFrame), m_nSizeFrame);
							frame_count++;

							// Push to sync Queue
							Queue_oct.push(fringe_data);

							// Return frame buffer to original queue
							{
								std::unique_lock<std::mutex> lock(mutex_raw);
								frame.push(frame_data);
							}
						}
					}
				});

				// OCT Process /////////////////////////////////////////////////////////////////////////////
				std::thread oct_proc([&]() {
					int frame_count = 0;
					while (frame_count < nFrame)
					{
						// Get data from synch Queue
						uint16_t* pulse_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_flim);
							if (!pulse.empty())
							{
								pulse_data = pulse.front();
								pulse.pop();
							}
						}

						if (pulse_data)
						{
							// Get data from synch Queue
							uint16_t* fringe_data = Queue_oct.pop();

							// OCT Process
							//frame_count++;
							(*pOCT)(m_OCT_Image32f.at(frame_count++), fringe_data);
							memcpy(pulse_data, fringe_data + m_nScans * m_nAlines, sizeof(uint16_t) * m_nScans * m_nAlines);

							// Push to sync Queue
							Queue_flim.push(pulse_data);

							// Return fringe buffer to original queue
							{
								std::unique_lock<std::mutex> lock(mutex_oct);
								fringe.push(fringe_data);
							}
						}
					}
				});

				// FLIM Process /////////////////////////////////////////////////////////////////////////////
				std::thread flim_proc([&]() {
					int frame_count = 0;
					while (frame_count < nFrame)
					{
						// Get data from synch Queue
						uint16_t* pulse_data = Queue_flim.pop();

						// FLIM Process
						np::Array<uint16_t, 2> temp(pulse_data, m_nScans * 4, m4_nAlines);
						(*m_pFLIMforExt)(itn, md, ltm, flim_params, pre_trig, temp);
						
						// Copy for Pulse Review
						np::Array<float, 2> crop(m_pFLIMforExt->_resize.nx, m_pFLIMforExt->_resize.ny);						
						np::Array<float, 2> mask(m_pFLIMforExt->_resize.nx, m_pFLIMforExt->_resize.ny);
						np::Array<float, 2> spline(m_pFLIMforExt->_resize.nsite, m_pFLIMforExt->_resize.ny);

						memcpy(crop, m_pFLIMforExt->_resize.crop_src, crop.length() * sizeof(float));
						memcpy(mask, m_pFLIMforExt->_resize.mask_src, mask.length() * sizeof(float));
						memcpy(spline, m_pFLIMforExt->_resize.ext_src, spline.length() * sizeof(float));

						m_PulseCrop.push_back(crop);
						m_PulseMask.push_back(mask);
						//m_PulseSpline.push_back(spline);

						// Copy for Intensity & Lifetime
						memcpy(&m_IntensityMap32f.at(0)(0, frame_count), &itn(0, 1), sizeof(float) * m4_nAlines);
						memcpy(&m_IntensityMap32f.at(1)(0, frame_count), &itn(0, 2), sizeof(float) * m4_nAlines);
						memcpy(&m_IntensityMap32f.at(2)(0, frame_count), &itn(0, 3), sizeof(float) * m4_nAlines);
						memcpy(&m_LifetimeMap32f.at(0)(0, frame_count), &ltm(0, 0), sizeof(float) * m4_nAlines);
						memcpy(&m_LifetimeMap32f.at(1)(0, frame_count), &ltm(0, 1), sizeof(float) * m4_nAlines);
						memcpy(&m_LifetimeMap32f.at(2)(0, frame_count++), &ltm(0, 2), sizeof(float) * m4_nAlines);

						// Return fringe buffer to original queue
						{
							std::unique_lock<std::mutex> lock(mutex_flim);
							pulse.push(pulse_data);
						}

						// Status update
						m_ProgressRes.SetPos(frame_count);
					}
				});

				// Wait for threads end
				load_data.join();
				deinterleave.join();
				oct_proc.join();
				flim_proc.join();

				// Delete OCT Process object & fringe queue
				delete pOCT;
				for (int i = 0; i < N_buffer; i++)
				{
					uint16_t* buffer1 = frame.front();
					frame.pop();
					delete[] buffer1;

					uint16_t* buffer2 = fringe.front();
					fringe.pop();
					delete[] buffer2;

					uint16_t* buffer3 = pulse.front();
					pulse.pop();
					delete[] buffer3;
				}

				// Get OCT Projection ////////////////////////////////////////////////////////////////////////					
				GetProjection(m_OCT_Image32f, m_Projection32f, m_EditCircCenter);

				// Status Update /////////////////////////////////////////////////////////////////////////////
				m_nFrame = nFrame;
				m_IsExternData = true;
				SetBitmaps();

				m_pMainDlg->m_Edit_nAlines = N_ALINES;
				((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetRange(0, nFrame - 1);
				((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->SetPos(0);
				((CSliderCtrl*)GetDlgItem(IDC_SLIDER_CUR_FRAME))->EnableWindow(TRUE);
				m_SliderCurFrame = 0;
				UpdateData(FALSE);

				GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(TRUE);
				GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(TRUE);
				GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
				GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(TRUE);

				SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Ready"));
				printf("External image processing is successfully completed!\n");

				// Make En Face Map //////////////////////////////////////////////////////////////////////////
				MaskEnFaceMap();


				InvalidateRect(m_VisRegionRectWnd, FALSE);
				if (m_pPulseReviewDlg) m_pPulseReviewDlg->InvalidateRect(m_pPulseReviewDlg->m_RectPulseGraphWnd, FALSE);
			}

			CloseHandle(hFile);
		}

		std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
		printf("elapsed time : %.2f sec\n", elapsed.count());
	});

	im_proc.detach();
}


void CVisResult::OnBnClickedButtonSaveResult()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (!m_CheckRGBWriting)
		SaveResults();
	else
	{
		if (!IS_TRIPLE_RING)
			SaveResultsMerged();
		else
			SaveResultsMerged3();
	}
}


void CVisResult::SaveResults()
{
	std::thread im_writing([&]() {

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		if (m_IsExternData || ((!m_IsExternData) && m_IsSaved))
		{
			// Set CONTROLS enable
			GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(FALSE);

			// Create directory to save images //////////////////////////////////////////////////////////////////
			CString rect_path = m_path + (CString)_T("rect_image");
			CString circ_path = m_path + (CString)_T("circ_image");
			CString rect_name, circ_name;

			CreateDirectory(rect_path, NULL);
			CreateDirectory(circ_path, NULL);

			// Image Processing	Buffers Allocation ////////////////////////////////////////////////////////////////			
			std::queue<uint8_t*> rect, circ;

			UpdateData(TRUE);
			int BufferSize = 100; int m_radius = CIRC_RADIUS;
			for (int i = 0; i < BufferSize; i++)
			{
				uint8_t* rect_buffer = new uint8_t[m_nScansFFT / 2 * m_nAlines4];
				uint8_t* circ_buffer = new uint8_t[4 * m_radius * m_radius];

				memset(rect_buffer, 0, m_nScansFFT / 2 * m_nAlines4 * sizeof(uint8_t));
				memset(circ_buffer, 0, 4 * m_radius * m_radius * sizeof(uint8_t));

				rect.push(rect_buffer);
				circ.push(circ_buffer);
			}

			// Bitmap Objects ///////////////////////////////////////////////////////////////////////////////////
			MyBitmap rect_bitmap; rect_bitmap.SetBitmap(m_nScansFFT / 2, m_nAlines, 1, m_ComboResLut);
			MyBitmap circ_bitmap; circ_bitmap.SetBitmap(2 * m_radius, 2 * m_radius, 1, m_ComboResLut);

			// Synchronization Objects //////////////////////////////////////////////////////////////////////////
			Queue<uint8_t*> Queue_rect, Queue_circ;
			std::mutex mutex_rect, mutex_circ;

			// Circ Image Writing Processing ///////////////////////////////////////////////////////////////////////
			SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Writing images..."));

			std::thread circ_writing_proc([&]() {
				int frame_count = 0;
				while (frame_count < m_nFrame)
				{
					// Get data from synch Queue
					uint8_t* circ_data = Queue_circ.pop();

					// Writing images
					circ_name.Format(_T("\\circ_1pullback_%03d.bmp"), ++frame_count);
					memcpy(circ_bitmap.GetPtr(), circ_data, sizeof(uint8_t) * 4 * m_radius * m_radius);
					circ_bitmap.WriteImage(circ_path + circ_name);

					m_ProgressRes.SetPos(frame_count);

					// Return circ buffer to original queue
					{
						std::unique_lock<std::mutex> lock(mutex_circ);
						circ.push(circ_data);
					}
				}
			});

			// Circ Image Processing //////////////////////////////////////////////////////////////////////////////			
			std::thread circ_proc([&]() {
				int frame_count = 0;
				while (frame_count < m_nFrame)
				{
					// Get data from synch Queue
					uint8_t* circ_data = nullptr;
					{
						std::unique_lock<std::mutex> lock(mutex_circ);
						if (!circ.empty())
						{
							circ_data = circ.front();
							circ.pop();
						}
					}

					if (circ_data)
					{
						// Get data from synch Queue
						uint8_t* rect_data = Queue_rect.pop();

						// Circularizing rect images
						np::Array<uint8_t, 2> rect_temp(rect_data, m_nScansFFT / 2, m_nAlines4);
						circ_OCT(rect_temp, circ_data, m_EditCircCenter);
						frame_count++;

						// Return buffer to original queue
						{
							std::unique_lock<std::mutex> lock(mutex_rect);
							rect.push(rect_data);
						}

						// Push to sync Queue
						Queue_circ.push(circ_data);
					}
				}
			});

			// Rect Image Processing //////////////////////////////////////////////////////////////////////////////
			std::thread rect_proc([&]() {
				int frame_count = 0;
				while (frame_count < m_nFrame)
				{
					// Get data from synch Queue
					uint8_t* rect_data = nullptr;
					{
						std::unique_lock<std::mutex> lock(mutex_rect);
						if (!rect.empty())
						{
							rect_data = rect.front();
							rect.pop();
						}
					}

					if (rect_data)
					{
						// Image conversion (32f -> 8u)
						ScaleRect(m_OCT_Image32f.at(frame_count++), rect_data);

						// Transpose Image
						TransposeRect(rect_data, rect_bitmap.GetPtr());

						// Write Image
						rect_name.Format(_T("\\rect_1pullback_%03d.bmp"), frame_count);
						rect_bitmap.WriteImage(rect_path + rect_name);

						// Push to sync Queue
						Queue_rect.push(rect_data);
					}
				}
			});

			// Wait for threads end ////////////////////////////////////////////////////////////////////////////			
			rect_proc.join();
			circ_proc.join();
			circ_writing_proc.join();

			// Delete buffer queues /////////////////////////////////////////////////////////////////////////			
			for (int i = 0; i < BufferSize; i++)
			{
				uint8_t* rect_buffer = rect.front(); rect.pop(); delete[] rect_buffer;
				uint8_t* circ_buffer = circ.front(); circ.pop(); delete[] circ_buffer;
			}

			// Write Fluorescence map into a binary file ////////////////////////////////////////////////////////
			SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Writing fluorescence map..."));

			CString flim_res_path = m_path + (CString)_T("FLIMres");
			CString file_res_name;
			CreateDirectory(flim_res_path, NULL);

			HANDLE hFile = INVALID_HANDLE_VALUE;
			DWORD dwWritten;

			for (int i = 0; i < 3; i++)
			{
				auto intensity_map(m_IntensityMap32f.at(i));
				auto lifetime_map(m_LifetimeMap32f.at(i));
				for (int i = 0; i < m_nFrame; i++)
				{
					float* pIntensity = intensity_map.raw_ptr() + i * m4_nAlines;
					float* pLifetime = lifetime_map.raw_ptr() + i * m4_nAlines;
					std::rotate(pIntensity, pIntensity + m_AdjustGalvo / 4, pIntensity + m4_nAlines);
					std::rotate(pLifetime, pLifetime + m_AdjustGalvo / 4, pLifetime + m4_nAlines);
				}

				file_res_name.Format(_T("\\intensity_ch%d.flimres"), i + 1);
				hFile = CreateFile(flim_res_path + file_res_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				WriteFile(hFile, intensity_map, sizeof(float) * intensity_map.length(), &dwWritten, NULL);
				CloseHandle(hFile);

				file_res_name.Format(_T("\\lifetime_ch%d.flimres"), i + 1);
				hFile = CreateFile(flim_res_path + file_res_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				WriteFile(hFile, lifetime_map, sizeof(float) * lifetime_map.length(), &dwWritten, NULL);
				CloseHandle(hFile);

				for (int i = 0; i < m_nFrame; i++)
				{
					float* pIntensity = intensity_map.raw_ptr() + i * m4_nAlines;
					float* pLifetime = lifetime_map.raw_ptr() + i * m4_nAlines;
					std::rotate(pIntensity, pIntensity + (m4_nAlines - m_AdjustGalvo / 4), pIntensity + m4_nAlines);
					std::rotate(pLifetime, pLifetime + (m4_nAlines - m_AdjustGalvo / 4), pLifetime + m4_nAlines);
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////

			// Set CONTROLS enable
			GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(TRUE);

			// Status Update ///////////////////////////////////////////////////////////////////////////////////
			SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Ready"));
			printf("Writing images is successfully completed!\n");

			CopyFile(_T("res_analysis.m"), flim_res_path + (CString)_T("\\res_analysis.m"), FALSE);

			std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
			printf("elapsed time : %.2f sec\n", elapsed.count());
		}
		else
			AfxMessageBox(_T("In case of in-buffer processing, you should save the binary file first!"), MB_ICONERROR);

	});

	im_writing.detach();
}


void CVisResult::SaveResultsMerged()
{
	std::thread im_writing([&]() {

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		if (m_IsExternData || ((!m_IsExternData) && m_IsSaved))
		{			
			// Set Controls Disable //////////////////////////////////////////////////////////////////////////////
			GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(FALSE);

			// Image Processing	Buffers Allocation ////////////////////////////////////////////////////////////////			
			std::queue<uint8_t*> rect, circ;

			UpdateData(TRUE);
			int BufferSize = 100; int m_radius = CIRC_RADIUS / 2;
			for (int i = 0; i < BufferSize; i++)
			{
				uint8_t* rect_buffer = new uint8_t[m_nScansFFT / 2 * m_nAlines4 * 3];
				uint8_t* circ_buffer = new uint8_t[4 * m_radius * m_radius * 3];

				memset(rect_buffer, 0, m_nScansFFT / 2 * m_nAlines4 * 3 * sizeof(uint8_t));
				memset(circ_buffer, 0, 4 * m_radius * m_radius * 3 * sizeof(uint8_t));

				rect.push(rect_buffer);
				circ.push(circ_buffer);
			}
			
			// Synchronization Objects //////////////////////////////////////////////////////////////////////////
			Queue<uint8_t*> Queue_rect, Queue_circ;
			std::mutex mutex_rect, mutex_circ;

			// Set FLIM Range /////////////////////////////////////////////////////////////////////////////////////
			CString str;
			GetDlgItemText(IDC_EDIT_RES_INTENSITY_MAX, str); float max_intensity = (float)_wtof(str);
			GetDlgItemText(IDC_EDIT_RES_INTENSITY_MIN, str); float min_intensity = (float)_wtof(str);
			GetDlgItemText(IDC_EDIT_RES_LIFETIME_MAX, str); int max_lifetime = _wtoi(str);
			GetDlgItemText(IDC_EDIT_RES_LIFETIME_MIN, str); int min_lifetime = _wtoi(str);		

			CString flim_res_path = m_path + (CString)_T("FLIMres");
			if (m_CheckResNormalization) flim_res_path += (CString)_T("_norm");
			CString file_res_name;
			CreateDirectory(flim_res_path, NULL);

			// Start Processing /////////////////////////////////////////////////////////////////////////////////////						
			for (int ch = 0; ch < 3; ch++)
			{
				CString msg; msg.Format(_T("Writing images [ch %d]..."), ch + 1);
				SetDlgItemText(IDC_STATIC_PROG_MSG, msg);

				// Select FLIM Channel ///////////////////////////////////////////////////////////////////////////////
				m_ComboResFlimShow = ch; UpdateData(FALSE);
				MaskEnFaceMap();

				file_res_name.Format(_T("\\intensity_ch%d.bmp"), ch + 1);
				m_pBitmapIntensityMap->WriteImage(flim_res_path + file_res_name);

				file_res_name.Format(_T("\\lifetime_ch%d.bmp"), ch + 1);
				m_pBitmapLifetimeMap->WriteImage(flim_res_path + file_res_name);

				if (m_CheckHSVEnhancing)
				{
					file_res_name.Format(_T("\\FLIMmap_ch%d.bmp"), ch + 1);
					m_pBitmapEnhancedMap->WriteImage(flim_res_path + file_res_name);
				}

				// Create directory to save images //////////////////////////////////////////////////////////////////
				CString foldername;
				//foldername.Format(_T("rect_merged_ch%d"), ch + 1);
				//CString rect_path = m_path + foldername;
				if (!m_CheckResNormalization)
					foldername.Format(_T("circ_merged_ch%d"), ch + 1);
				else
					foldername.Format(_T("circ_merged_norm_ch%d"), ch + 1);
				CString circ_path = m_path + foldername;
				CString rect_name, circ_name;

				//CreateDirectory(rect_path, NULL);
				CreateDirectory(circ_path, NULL);

				// Circ Image Writing Processing ///////////////////////////////////////////////////////////////////////
				std::thread circ_writing_proc([&]() {
					MyBitmap circ_bitmap; circ_bitmap.SetBitmap(2 * m_radius, 2 * m_radius, 3);
					int frame_count = 0;
					while (frame_count < m_nFrame)
					{
						// Get data from synch Queue
						uint8_t* circ_data = Queue_circ.pop();
						
						// Copy Data
						memcpy(circ_bitmap, circ_data, circ_bitmap.GetTotalLength() * sizeof(BYTE));

						// Writing images
						circ_name.Format(_T("\\circ_1pullback_ch%d_i[%.1f %.1f]t[%d %d]_%03d.bmp"),
							ch + 1, min_intensity, max_intensity, min_lifetime, max_lifetime, ++frame_count);
						circ_bitmap.WriteImage(circ_path + circ_name);

						m_ProgressRes.SetPos(frame_count);

						// Return circ buffer to original queue
						{
							std::unique_lock<std::mutex> lock(mutex_circ);
							circ.push(circ_data);
						}
					}
				});

				// Circ Image Processing //////////////////////////////////////////////////////////////////////////////			
				std::thread circ_proc([&]() {
					int frame_count = 0;
					MyBitmap rect_bitmap; rect_bitmap.SetBitmap(m_nScansFFT / 2, m_nAlines, 3);
					MyBitmap circ_bitmap; circ_bitmap.SetBitmap(2 * m_radius, 2 * m_radius, 3);

					int ring_thickness = 80;
					while (frame_count < m_nFrame)
					{
						// Get data from synch Queue
						uint8_t* circ_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_circ);
							if (!circ.empty())
							{
								circ_data = circ.front();
								circ.pop();
							}
						}

						if (circ_data)
						{
							// Get data from synch Queue
							uint8_t* rect_data = Queue_rect.pop();

							// Copy Data
							memcpy(rect_bitmap, rect_data, rect_bitmap.GetTotalLength() * sizeof(BYTE));
													
							// Circularize OCT Image
							IppiSize roi_oct = { m_nScansFFT / 2, m_nAlines };
							IppiSize roi_oct1 = { m_nAlines, m_nScansFFT / 2 };
							tbb::parallel_for(tbb::blocked_range<size_t>(0, (size_t)3),
								[&](const tbb::blocked_range<size_t>& r) {
								for (size_t i = r.begin(); i != r.end(); ++i)
								{
									np::Array<BYTE, 2> temp_rect(rect_bitmap.GetChannelImg((int)i), m_nAlines, m_nScansFFT / 2);
									np::Array<BYTE, 2> temp_rect1(m_nScansFFT / 2, m_nAlines);

									ippiTranspose_8u_C1R(temp_rect.raw_ptr(), roi_oct1.width * sizeof(uint8_t), temp_rect1.raw_ptr(), roi_oct1.height * sizeof(uint8_t), roi_oct1);
									ippiMirror_8u_C1IR(temp_rect1.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct1, ippAxsVertical);

									np::Array<BYTE, 2> circ_temp1(2 * m_radius, 2 * m_radius);
									circ_writing_OCT(temp_rect1, circ_temp1, m_EditCircCenter);
									circ_bitmap.PutChannelImg(circ_temp1, (int)i);
								}
							});
							memcpy(circ_data, circ_bitmap, circ_bitmap.GetTotalLength() * sizeof(BYTE));
							frame_count++;

							// Return buffer to original queue
							{
								std::unique_lock<std::mutex> lock(mutex_rect);
								rect.push(rect_data);
							}

							// Push to sync Queue
							Queue_circ.push(circ_data);
						}
					}
				});

				// Rect Image Processing //////////////////////////////////////////////////////////////////////////////
				std::thread rect_proc([&]() {
					int frame_count = 0;
					MyBitmap rect_bitmap; rect_bitmap.SetBitmap(m_nScansFFT / 2, m_nAlines, 3);
					np::Array<BYTE, 2> temp_rect_data(m_nScansFFT / 2, m_nAlines);
					np::Array<BYTE, 2> transpose_data(m_nAlines, m_nScansFFT / 2);

					int ring_thickness = 80;
					while (frame_count < m_nFrame)
					{
						// Get data from synch Queue
						uint8_t* rect_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_rect);
							if (!rect.empty())
							{
								rect_data = rect.front();
								rect.pop();
							}
						}

						if (rect_data)
						{
							// Image conversion (32f -> 8u)
							ScaleRect(m_OCT_Image32f.at(frame_count), temp_rect_data);

							// Transpose Image
							TransposeRect(temp_rect_data, transpose_data);

							// RGB Conversion
							rect_bitmap.IndToRGB(transpose_data, m_ComboResLut);

							// Circularizing rect images
							for (int i = 0; i < ring_thickness; i++)
							{
								int circ_offset = m_nScansFFT / 2 - 1 - m_EditCircCenter - CIRC_RADIUS + i;

								if (!m_CheckHSVEnhancing)
									rect_bitmap.IndToRGB_line(m_pBitmapLifetimeMap->GetPtr(frame_count), circ_offset, LUT_HSV);
								else
									memcpy(rect_bitmap.GetPtr(circ_offset), m_pBitmapEnhancedMap->GetPtr(frame_count), sizeof(BYTE) * m_pBitmapEnhancedMap->GetWStep());

								rect_bitmap.IndToRGB_line(m_pBitmapIntensityMap->GetPtr(frame_count), ring_thickness + circ_offset, LUT_FIRE);
							}

							memcpy(rect_data, rect_bitmap, rect_bitmap.GetTotalLength() * sizeof(BYTE));
							frame_count++;

							//// Copy FLIM Information						
							//for (int i = 0; i < ring_thickness; i++)
							//{
							//	if (!m_CheckHSVEnhancing)
							//		rect_bitmap.IndToRGB_line(m_pBitmapLifetimeMap->GetPtr(frame_count), i, LUT_HSV);
							//	else
							//		memcpy(rect_bitmap.GetPtr(i), m_pBitmapEnhancedMap->GetPtr(frame_count), sizeof(BYTE) * m_pBitmapEnhancedMap->GetWStep());

							//	rect_bitmap.IndToRGB_line(m_pBitmapIntensityMap->GetPtr(frame_count), ring_thickness + i, LUT_FIRE);
							//}

							//// Write Image
							//rect_name.Format(_T("\\rect_1pullback_ch%d_i[%.1f %.1f]t[%d %d]_%03d.bmp"),
							//	ch + 1, min_intensity, max_intensity, min_lifetime, max_lifetime, ++frame_count);
							//rect_bitmap.WriteImage(rect_path + rect_name);

							// Push to sync Queue
							Queue_rect.push(rect_data);
						}
					}
				});

				// Wait for threads end ////////////////////////////////////////////////////////////////////////////			
				rect_proc.join();
				circ_proc.join();
				circ_writing_proc.join();
			}

			// Delete buffer queues /////////////////////////////////////////////////////////////////////////			
			for (int i = 0; i < BufferSize; i++)
			{
				uint8_t* rect_buffer = rect.front(); rect.pop(); delete[] rect_buffer;
				uint8_t* circ_buffer = circ.front(); circ.pop(); delete[] circ_buffer;
			}				
			
			// Set Controls Enable ///////////////////////////////////////////////////////////////////////////////
			GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(TRUE);
			GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(TRUE);

			// Status Update ///////////////////////////////////////////////////////////////////////////////////
			SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Ready"));
			printf("Writing images is successfully completed!\n");

			//CopyFile(_T("res_analysis.m"), flim_res_path + (CString)_T("\\res_analysis.m"), FALSE);

			std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
			printf("elapsed time : %.2f sec\n", elapsed.count());
		}
		else
			AfxMessageBox(_T("In case of in-buffer processing, you should save the binary file first!"), MB_ICONERROR);

	});

	im_writing.detach();
}


void CVisResult::SaveResultsMerged3()
{
	std::thread im_writing([&]() {

		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

		if (m_CheckHSVEnhancing)
		{
			if (m_IsExternData || ((!m_IsExternData) && m_IsSaved))
			{
				// Set Controls Disable //////////////////////////////////////////////////////////////////////////////
				GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(FALSE);
				GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(FALSE);
				GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(FALSE);
				GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(FALSE);

				// Image Processing	Buffers Allocation ////////////////////////////////////////////////////////////////			
				std::queue<uint8_t*> rect, circ;

				UpdateData(TRUE);
				int BufferSize = 100; int m_radius = CIRC_RADIUS / 2;
				for (int i = 0; i < BufferSize; i++)
				{
					uint8_t* rect_buffer = new uint8_t[m_nScansFFT / 2 * m_nAlines4 * 3];
					uint8_t* circ_buffer = new uint8_t[4 * m_radius * m_radius * 3];

					memset(rect_buffer, 0, m_nScansFFT / 2 * m_nAlines4 * 3 * sizeof(uint8_t));
					memset(circ_buffer, 0, 4 * m_radius * m_radius * 3 * sizeof(uint8_t));
					
					rect.push(rect_buffer);
					circ.push(circ_buffer);
				}

				// Synchronization Objects //////////////////////////////////////////////////////////////////////////
				Queue<uint8_t*> Queue_rect, Queue_circ;
				std::mutex mutex_rect, mutex_circ;

				// Set FLIM Range /////////////////////////////////////////////////////////////////////////////////////
				CString str;
				GetDlgItemText(IDC_EDIT_RES_INTENSITY_MAX, str); float max_intensity = (float)_wtof(str);
				GetDlgItemText(IDC_EDIT_RES_INTENSITY_MIN, str); float min_intensity = (float)_wtof(str);
				GetDlgItemText(IDC_EDIT_RES_LIFETIME_MAX, str); int max_lifetime = _wtoi(str);
				GetDlgItemText(IDC_EDIT_RES_LIFETIME_MIN, str); int min_lifetime = _wtoi(str);

				CString flim_res_path = m_path + (CString)_T("FLIMres_total");
				if (m_CheckResNormalization) flim_res_path += (CString)_T("_norm");
				CString file_res_name;
				CreateDirectory(flim_res_path, NULL);

				// Start Processing /////////////////////////////////////////////////////////////////////////////////////					
				CString msg; msg.Format(_T("Writing images with triple rings..."));
				SetDlgItemText(IDC_STATIC_PROG_MSG, msg);
				
				// Select FLIM Channel ///////////////////////////////////////////////////////////////////////////////
				MyBitmap* pBitmapEnhancedMap[3];
				for (int ch = 0; ch < 3; ch++)
				{
					m_ComboResFlimShow = ch; UpdateData(FALSE);
					if (ch == 2)
					{
						m_EditMinIntensity = 0.3f;
						UpdateData(FALSE);
					}
		
					MaskEnFaceMap();

					pBitmapEnhancedMap[ch] = new MyBitmap;
					pBitmapEnhancedMap[ch]->SetBitmap(m_nFrame, m_nAlines, 3);
					
					memcpy(pBitmapEnhancedMap[ch]->GetPtr(), m_pBitmapEnhancedMap->GetPtr(), 3 * m_nFrame * m_nAlines);

					file_res_name.Format(_T("\\intensity_ch%d.bmp"), ch + 1);
					m_pBitmapIntensityMap->WriteImage(flim_res_path + file_res_name);

					file_res_name.Format(_T("\\lifetime_ch%d.bmp"), ch + 1);
					m_pBitmapLifetimeMap->WriteImage(flim_res_path + file_res_name);

					if (m_CheckHSVEnhancing)
					{
						file_res_name.Format(_T("\\FLIMmap_ch%d.bmp"), ch + 1);
						m_pBitmapEnhancedMap->WriteImage(flim_res_path + file_res_name);
					}
				}

				// Create directory to save images //////////////////////////////////////////////////////////////////
				CString foldername;
				foldername.Format(_T("rect_merged_total"));
				CString rect_path = m_path + foldername;

				if (!m_CheckResNormalization)
					foldername.Format(_T("circ_merged_total"));
				else
					foldername.Format(_T("circ_merged_total_norm"));
				CString circ_path = m_path + foldername;
				CString rect_name, circ_name;

				if (IS_RECT_WRITE) CreateDirectory(rect_path, NULL);
				CreateDirectory(circ_path, NULL);

				// Circ Image Writing Processing ///////////////////////////////////////////////////////////////////////
				std::thread circ_writing_proc([&]() {
					MyBitmap circ_bitmap; circ_bitmap.SetBitmap(2 * m_radius, 2 * m_radius, 3);
					int frame_count = 0;
					while (frame_count < m_nFrame)
					{
						// Get data from synch Queue
						uint8_t* circ_data = Queue_circ.pop();

						// Copy Data
						memcpy(circ_bitmap, circ_data, circ_bitmap.GetTotalLength() * sizeof(BYTE));

						// Writing images
						circ_name.Format(_T("\\circ_1pullback_triple_i[%.1f %.1f]t[%d %d]_%03d.bmp"),
							min_intensity, max_intensity, min_lifetime, max_lifetime, ++frame_count);
						if (!IS_RECT_WRITE) circ_bitmap.WriteImage(circ_path + circ_name);

						m_ProgressRes.SetPos(frame_count);

						// Return circ buffer to original queue
						{
							std::unique_lock<std::mutex> lock(mutex_circ);
							circ.push(circ_data);
						}
					}
				});

				// Circ Image Processing //////////////////////////////////////////////////////////////////////////////			
				std::thread circ_proc([&]() {
					int frame_count = 0;
					MyBitmap rect_bitmap; rect_bitmap.SetBitmap(m_nScansFFT / 2, m_nAlines, 3);
					MyBitmap circ_bitmap; circ_bitmap.SetBitmap(2 * m_radius, 2 * m_radius, 3);

					int ring_thickness = 80;
					while (frame_count < m_nFrame)
					{
						// Get data from synch Queue
						uint8_t* circ_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_circ);
							if (!circ.empty())
							{
								circ_data = circ.front();
								circ.pop();
							}
						}

						if (circ_data)
						{
							// Get data from synch Queue
							uint8_t* rect_data = Queue_rect.pop();

							if (!IS_RECT_WRITE)
							{
								// Copy Data
								memcpy(rect_bitmap, rect_data, rect_bitmap.GetTotalLength() * sizeof(BYTE));

								// Circularize OCT Image
								IppiSize roi_oct = { m_nScansFFT / 2, m_nAlines };
								IppiSize roi_oct1 = { m_nAlines, m_nScansFFT / 2 };
								tbb::parallel_for(tbb::blocked_range<size_t>(0, (size_t)3),
									[&](const tbb::blocked_range<size_t>& r) {
									for (size_t i = r.begin(); i != r.end(); ++i)
									{
										np::Array<BYTE, 2> temp_rect(rect_bitmap.GetChannelImg((int)i), m_nAlines, m_nScansFFT / 2);
										np::Array<BYTE, 2> temp_rect1(m_nScansFFT / 2, m_nAlines);

										ippiTranspose_8u_C1R(temp_rect.raw_ptr(), roi_oct1.width * sizeof(uint8_t), temp_rect1.raw_ptr(), roi_oct1.height * sizeof(uint8_t), roi_oct1);
										ippiMirror_8u_C1IR(temp_rect1.raw_ptr(), roi_oct.width * sizeof(uint8_t), roi_oct1, ippAxsVertical);

										np::Array<BYTE, 2> circ_temp1(2 * m_radius, 2 * m_radius);
										circ_writing_OCT(temp_rect1, circ_temp1, m_EditCircCenter);
										circ_bitmap.PutChannelImg(circ_temp1, (int)i);
									}
								});
								memcpy(circ_data, circ_bitmap, circ_bitmap.GetTotalLength() * sizeof(BYTE));
							}
							frame_count++;

							// Return buffer to original queue
							{
								std::unique_lock<std::mutex> lock(mutex_rect);
								rect.push(rect_data);
							}

							// Push to sync Queue
							Queue_circ.push(circ_data);
						}
					}
				});

				// Rect Image Processing //////////////////////////////////////////////////////////////////////////////
				std::thread rect_proc([&]() {
					int frame_count = 0;
					MyBitmap rect_bitmap; rect_bitmap.SetBitmap(m_nScansFFT / 2, m_nAlines, 3);
					np::Array<BYTE, 2> temp_rect_data(m_nScansFFT / 2, m_nAlines);
					np::Array<BYTE, 2> transpose_data(m_nAlines, m_nScansFFT / 2);

					int ring_thickness = 80;
					while (frame_count < m_nFrame)
					{
						// Get data from synch Queue
						uint8_t* rect_data = nullptr;
						{
							std::unique_lock<std::mutex> lock(mutex_rect);
							if (!rect.empty())
							{
								rect_data = rect.front();
								rect.pop();
							}
						}

						if (rect_data)
						{
							// Image conversion (32f -> 8u)
							ScaleRect(m_OCT_Image32f.at(frame_count), temp_rect_data);

							// Transpose Image
							TransposeRect(temp_rect_data, transpose_data);

							// RGB Conversion
							rect_bitmap.IndToRGB(transpose_data, m_ComboResLut);

							// Circularizing rect images
							for (int i = 0; i < ring_thickness; i++)
							{
								int circ_offset = m_nScansFFT / 2 - 1 - m_EditCircCenter - CIRC_RADIUS + i;
															
								for (int ch = 0; ch < 3; ch++)
									memcpy(rect_bitmap.GetPtr(circ_offset + (2-ch) * ring_thickness), pBitmapEnhancedMap[ch]->GetPtr(frame_count), sizeof(BYTE) * pBitmapEnhancedMap[ch]->GetWStep());
							}

							memcpy(rect_data, rect_bitmap, rect_bitmap.GetTotalLength() * sizeof(BYTE));
							frame_count++;

							//// Copy FLIM Information						
							//for (int i = 0; i < ring_thickness; i++)
							//{
							//	if (!m_CheckHSVEnhancing)
							//		rect_bitmap.IndToRGB_line(m_pBitmapLifetimeMap->GetPtr(frame_count), i, LUT_HSV);
							//	else
							//		memcpy(rect_bitmap.GetPtr(i), m_pBitmapEnhancedMap->GetPtr(frame_count), sizeof(BYTE) * m_pBitmapEnhancedMap->GetWStep());

							//	rect_bitmap.IndToRGB_line(m_pBitmapIntensityMap->GetPtr(frame_count), ring_thickness + i, LUT_FIRE);
							//}

							// Write Image
							if (IS_RECT_WRITE)
							{
								rect_name.Format(_T("\\rect_1pullback_triple_i[%.1f %.1f]t[%d %d]_%03d.bmp"),
									min_intensity, max_intensity, min_lifetime, max_lifetime, frame_count);
								rect_bitmap.WriteImage(rect_path + rect_name);
							}

							// Push to sync Queue
							Queue_rect.push(rect_data);
						}
					}
				});

				// Wait for threads end ////////////////////////////////////////////////////////////////////////////			
				rect_proc.join();
				circ_proc.join();
				circ_writing_proc.join();

				// Delete buffer queues /////////////////////////////////////////////////////////////////////////			
				for (int i = 0; i < BufferSize; i++)
				{
					uint8_t* rect_buffer = rect.front(); rect.pop(); delete[] rect_buffer;
					uint8_t* circ_buffer = circ.front(); circ.pop(); delete[] circ_buffer;
				}

				// Free Bitmap Objects ////////////////////////////////////////////////////////////////////////////
				for (int ch = 0; ch < 3; ch++)
					delete pBitmapEnhancedMap[ch];

				// Set Controls Enable ///////////////////////////////////////////////////////////////////////////////
				GetDlgItem(IDC_EDIT_RES_ALINE_MAX_DB)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_ALINE_MIN_DB)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_CIRC_CENTER)->EnableWindow(TRUE);
				GetDlgItem(IDC_COMBO_RES_LUT)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_SAVE_AS_RGB)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_SHOW_GUIDELINE)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_CIRCULARIZE)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_HSV_ENHANCING)->EnableWindow(TRUE);
				GetDlgItem(IDC_CHECK_RES_NORMALIZATION)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_INTENSITY_MAX)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_INTENSITY_MIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_LIFETIME_MAX)->EnableWindow(TRUE);
				GetDlgItem(IDC_EDIT_RES_LIFETIME_MIN)->EnableWindow(TRUE);
				GetDlgItem(IDC_COMBO_RES_FLIM_SHOW)->EnableWindow(TRUE);
				GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
				GetDlgItem(IDC_BUTTON_SAVE_RESULT)->EnableWindow(TRUE);

				// Status Update ///////////////////////////////////////////////////////////////////////////////////
				SetDlgItemText(IDC_STATIC_PROG_MSG, _T("Ready"));
				printf("Writing images is successfully completed!\n");

				//CopyFile(_T("res_analysis.m"), flim_res_path + (CString)_T("\\res_analysis.m"), FALSE);

				std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
				printf("elapsed time : %.2f sec\n", elapsed.count());
			}
			else
				AfxMessageBox(_T("In case of in-buffer processing, you should save the binary file first!"), MB_ICONERROR);
		}
		else
			printf("This operation is only possible when HSV Mapping Mode is enabled.");

	});

	im_writing.detach();
}


//#include <chrono>
void CVisResult::GetProjection(std::vector<np::Array<float, 2>>& OCT_vector, np::Array<float, 2>& Proj, int offset)
{
	//std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();
	int len = CIRC_RADIUS;
	for (int i = 0; i < (int)OCT_vector.size(); i++)
	{
		float max_val;
		for (int j = 0; j < m_nAlines; j++)
		{
	//		ippsMean_32f(&OCT_vector.at(i)(offset + 300, j), 400, &max_val, ippAlgHintFast); // lee mean woo (max woo)
			ippsMax_32f(&OCT_vector.at(i)(offset + PROJECTION_OFFSET, j), len, &max_val);
			Proj(j, i) = max_val;
		}
	}
	//std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();
	//std::chrono::system_clock::time_point end1 = std::chrono::system_clock::now();

	//std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
	//printf("elapsed time : %d msec\n", (int)elapsed.count());
}


void CVisResult::ScaleRect(const float* pSrc, uint8_t* pDst)
{
	IppiSize roi_oct = { m_nScansFFT / 2, m_nAlines };
	IppiSize roi_oct4 = { m_nScansFFT / 2, m_nAlines4 };

	ippiScale_32f8u_C1R(pSrc, roi_oct.width * sizeof(float),
		pDst, roi_oct.width * sizeof(uint8_t), roi_oct, (float)m_EditResMinDb, (float)m_EditResMaxDb);
	rect_medfilt(pDst);

	//if (m_CheckFilipHorizontal) ippiMirror_8u_C1IR(pDst, roi_oct4.width * sizeof(uint8_t), roi_oct4, ippAxsVertical);
}


void CVisResult::TransposeRect(const uint8_t* pSrc, uint8_t* pDst)
{
	IppiSize roi_oct4 = { m_nScansFFT / 2, m_nAlines4 };
	IppiSize roi_oct4t = { m_nAlines4, m_nScansFFT / 2 };

	ippiTranspose_8u_C1R(pSrc, roi_oct4.width * sizeof(uint8_t), pDst, roi_oct4.height * sizeof(uint8_t), roi_oct4);
	ippiMirror_8u_C1IR(pDst, roi_oct4t.width * sizeof(uint8_t), roi_oct4t, ippAxsHorizontal);

	for (int i = 0; i < m_nScansFFT / 2; i++)
	{
		uint8_t* pImg = pDst + i * m_nAlines;
		std::rotate(pImg, pImg + m_AdjustGalvo, pImg + m_nAlines);
	}
}


void CVisResult::MaskEnFaceMap()
{
	if (m_nFrame != 1)
	{
		UpdateData(TRUE);
		
		// OCT Projection
		np::Array<float, 2> OCT_projection(m_nAlines, m_nFrame);		
		memcpy(OCT_projection, m_Projection32f, sizeof(float) * OCT_projection.length());
		proj_medfilt(OCT_projection);

		// Intensity En Face map
		np::Array<float, 2> intensity_map4(m4_nAlines, m_nFrame), intensity_map(m_nAlines, m_nFrame);				
		memcpy(intensity_map4, m_IntensityMap32f.at(m_ComboResFlimShow), sizeof(float) * intensity_map4.length());			
		intensity4_medfilt(intensity_map4);	

		np::Array<float, 2> sum_map(m4_nAlines, m_nFrame);
		memset(sum_map, 0, sizeof(float) * sum_map.length());
		if (m_CheckResNormalization)
		{
			for (int i = 0; i < 3; i++)
			{
				np::Array<float, 2> temp_map(m4_nAlines, m_nFrame);
				memcpy(temp_map, m_IntensityMap32f.at(i), sizeof(float) * temp_map.length());
				intensity4_medfilt(temp_map);
				ippsAdd_32f_I(temp_map, sum_map, sum_map.length());
			}

			ippsDiv_32f_I(sum_map, intensity_map4, intensity_map4.length());
		}

		// Lifetime En Face Map
		np::Array<float, 2> lifetime_map4(m4_nAlines, m_nFrame), lifetime_map(m_nAlines, m_nFrame);
		memcpy(lifetime_map4, m_LifetimeMap32f.at(m_ComboResFlimShow), sizeof(float) * lifetime_map4.length());
		lifetime4_medfilt(lifetime_map4);

		// En Face Map Resize
		np::Array<float> x_old(m4_nAlines); ippsVectorSlope_32f(x_old, m4_nAlines, 0, ((float)m_nAlines - 1)/((float)m4_nAlines - 1));
		np::Array<float> x_new(m_nAlines); ippsVectorSlope_32f(x_new, m_nAlines, 0, 1);
		for (int i = 0; i < m_nFrame; i++)
		{
			Interpolation_32f(x_old, &intensity_map4(0, i), x_new, m4_nAlines, m_nAlines, &intensity_map(0, i));
			Interpolation_32f(x_old, &lifetime_map4(0, i), x_new, m4_nAlines, m_nAlines, &lifetime_map(0, i));
		}

		// Scaling
		IppiSize roi_proj = { m_nAlines, m_nFrame };		
		ippiScale_32f8u_C1R(OCT_projection, roi_proj.width * sizeof(float),
			m_pBitmapProjection->GetPtr(), roi_proj.width * sizeof(uint8_t), roi_proj, (float)m_EditResMinDb, (float)m_EditResMaxDb);
		ippiScale_32f8u_C1R(intensity_map, roi_proj.width * sizeof(float),
			m_pBitmapIntensityMap->GetPtr(), roi_proj.width * sizeof(BYTE), roi_proj, m_EditMinIntensity, m_EditMaxIntensity);
		ippiScale_32f8u_C1R(lifetime_map, roi_proj.width * sizeof(float),
			m_pBitmapLifetimeMap->GetPtr(), roi_proj.width * sizeof(BYTE), roi_proj, m_EditMinLifetime, m_EditMaxLifetime);

		// Circ Shifting
		for (int i = 0; i < m_nFrame; i++)
		{
			uint8_t* pProjection = m_pBitmapProjection->GetPtr() + i * m_nAlines;
			uint8_t* pIntensity = m_pBitmapIntensityMap->GetPtr() + i * m_nAlines;
			uint8_t* pLifetime = m_pBitmapLifetimeMap->GetPtr() + i * m_nAlines;
			std::rotate(pProjection, pProjection + m_AdjustGalvo, pProjection + m_nAlines);
			std::rotate(pIntensity, pIntensity + m_AdjustGalvo, pIntensity + m_nAlines);
			std::rotate(pLifetime, pLifetime + m_AdjustGalvo, pLifetime + m_nAlines);
		}

		// Generating HSV Map
		if (m_CheckHSVEnhancing)
		{
			MyBitmap temp_hsv; temp_hsv.SetBitmap(m_nFrame, m_nAlines, 3);
			temp_hsv.PutChannelImg(m_pBitmapLifetimeMap->GetPtr(), 0);
			np::Array<uint8_t, 2> one(m_nAlines, m_nFrame); ippsSet_8u(255, one, one.length());
			temp_hsv.PutChannelImg(one, 1);

			np::Array<BYTE, 2> temp_intensity(m_nAlines, m_nFrame);
			memcpy(temp_intensity, m_pBitmapIntensityMap->GetPtr(), temp_intensity.length() * sizeof(BYTE));
			if (!m_CheckResNormalization)
				ippsMulC_8u_ISfs(HSV_MULTIFICATION_FACTOR, temp_intensity, temp_intensity.length(), 0);

			temp_hsv.PutChannelImg(temp_intensity, 2);

			ippiHSVToRGB_8u_C3R(temp_hsv.GetPtr(), 3 * m_nAlines, m_pBitmapEnhancedMap->GetPtr(), 3 * m_nAlines, roi_proj);
			m_pBitmapEnhancedMap->SwapBlueRed();
		}
	}
}




void CVisResult::Interpolation_32f(const Ipp32f* srcX, const Ipp32f* srcY, Ipp32f* dstX, int srcLen, int dstLen, Ipp32f* dstY)
{
	// This function is only valid when diff(srcX) is always larger than 0...
	// Also diff(dstX) should be either >0 or <0.

	Ipp32f srcX_0 = 0, srcX_1 = 0;
	Ipp32f srcY_0 = 0, srcY_1 = 0;
	int ii;
	Ipp32f w;

	BOOL isIncre = (dstX[1] > dstX[0]) ? TRUE : FALSE;
	BOOL isExist;

	for (int i = 0; i < dstLen; i++)
	{
		for (int j = 0; j < srcLen - 1; j++)
		{
			isExist = isIncre ? ((dstX[i] >= srcX[j]) && (dstX[i] <= srcX[j + 1])) : ((dstX[i] <= srcX[j]) && (dstX[i] >= srcX[j + 1]));
			if (isExist)
			{
				srcX_0 = srcX[j]; srcX_1 = srcX[j + 1];
				srcY_0 = srcY[j]; srcY_1 = srcY[j + 1];
				ii = j;
				break;
			}
		}

		w = (dstX[i] - srcX_0) / (srcX_1 - srcX_0);
		dstY[i] = (1 - w) * srcY_0 + w * srcY_1;
	}
}



