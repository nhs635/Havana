
// HavanaDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "Havana.h"
#include "HavanaDlg.h"
#include "afxdialogex.h"

#include "ImageProcess\MyBitmap.h"

#include "ImageProcess\OCTProcess\CalibrationDlg\CalibrationDlg.h"
#include "ImageProcess\FLIMProcess\FLIMAnalysisDlg\FLIMAnalysisDlg.h"
#include "ImageProcess\FLIMProcess\PulseOverlappedDlg\PulseOverlapDlg.h"
#include "ImageProcess\FLIMProcess\PulseReviewDlg\PulseReviewDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHavanaDlg 대화 상자

CHavanaDlg::CHavanaDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_HAVANA_DIALOG, pParent),
	m_pWndShow(NULL), m_pDataAcquisition(nullptr), m_pDataRecording(nullptr),
	m_pOCT(nullptr), m_pFLIM(nullptr),
	m_CheckPmtGainControl(false), m_CheckFLIMSync(false), m_CheckInternalSync(false),
	m_CheckGalvoScan(false), m_CheckZaberStage(false),
	m_CheckFaulhaberMotor(false), m_ToggleRotateMotor(false), m_CheckElforlightLaser(false),
	m_bIsBufferAllocated(false),
	m_Edit_AdcRate(ADC_RATE), m_Edit_nAlines(N_ALINES), m_Edit_nScans(2600), m_Edit_nFFT(GET_NEAR_2_POWER(m_Edit_nScans)),
	m_Combo_ActiveChannel(1), m_Combo_TrigSource(1), m_Edit_PmtGain(0.5), m_Edit_RepetitionRate(30), m_ScrollAdjustGalvo(0), temp_discom(0),
	m_Edit_GalvoVoltage(2.0), m_Edit_GalvoOffset(-2.5), m_Edit_MoveAbs(20.0), m_Edit_TargetSpeed(5.0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_HAVANA);
}

void CHavanaDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_Tab);
	DDX_Control(pDX, IDC_TOGGLE_ACQUISITION, m_ToggleAcquisition);
	DDX_Control(pDX, IDC_TOGGLE_RECORDING, m_ToggleRecording);
	DDX_Control(pDX, IDC_TOGGLE_SAVING, m_ToggleSaving);

	DDX_Text(pDX, IDC_EDIT_N_ALINES, m_Edit_nAlines);
	DDX_Text(pDX, IDC_EDIT_N_SCANS, m_Edit_nScans);
	DDX_Text(pDX, IDC_EDIT_PRE_TRIG_SAMPS, m_Edit_PreTrig);
	DDX_Text(pDX, IDC_EDIT_TRIG_DELAY_SAMPS, m_Edit_TrigDelay);
	DDX_Text(pDX, IDC_EDIT_ADC_RATE, m_Edit_AdcRate);

	DDX_CBIndex(pDX, IDC_COMBO_FLIM_INPUT_RANGE, m_Combo_FLIMInputRange);
	DDX_CBIndex(pDX, IDC_COMBO_OCT_INPUT_RANGE, m_Combo_OCTInputRange);
	DDX_CBIndex(pDX, IDC_COMBO_ACTIVE_CH, m_Combo_ActiveChannel);
	DDX_CBIndex(pDX, IDC_COMBO_TRIG_SOURCE, m_Combo_TrigSource);

	DDX_Control(pDX, IDC_PROGRESS, m_ProgressBar);

	DDX_Check(pDX, IDC_CHECK_PMT_GAIN, m_CheckPmtGainControl);
	DDX_Text(pDX, IDC_EDIT_PMT_GAIN, m_Edit_PmtGain);
	DDX_Check(pDX, IDC_CHECK_FLIM_SYNC, m_CheckFLIMSync);
	DDX_Check(pDX, IDC_CHECK_INTERNAL_SYNC, m_CheckInternalSync);
	DDX_Text(pDX, IDC_EDIT_REPETITION, m_Edit_RepetitionRate);
	DDX_Check(pDX, IDC_CHECK_GALVO_SCAN, m_CheckGalvoScan);
	DDX_Text(pDX, IDC_EDIT_GALVO_PP_VOLTAGE, m_Edit_GalvoVoltage);
	DDX_Text(pDX, IDC_EDIT_GALVO_OFFSET, m_Edit_GalvoOffset);
	DDX_Scroll(pDX, IDC_SCROLLBAR_ADJUST_GALVO, m_ScrollAdjustGalvo);
	DDX_Check(pDX, IDC_CHECK_ZABER_STAGE, m_CheckZaberStage);
	DDX_Text(pDX, IDC_EDIT_ZABER_MOVE_ABSOLUTE, m_Edit_MoveAbs);
	DDX_Text(pDX, IDC_EDIT_ZABER_SET_TARGET_SPEED, m_Edit_TargetSpeed);
	DDX_Check(pDX, IDC_CHECK_FAULHABER_MOTOR, m_CheckFaulhaberMotor);
	DDX_Text(pDX, IDC_EDIT_FAULHABER_RPM, m_Edit_RPM);
	DDX_Check(pDX, IDC_TOGGLE_FAULHABER_ROTATE, m_ToggleRotateMotor);
	DDX_Check(pDX, IDC_CHECK_ELFORLIGHT_LASER, m_CheckElforlightLaser);
}

BEGIN_MESSAGE_MAP(CHavanaDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CHavanaDlg::OnTcnSelchangeTab)
	ON_BN_CLICKED(IDC_TOGGLE_ACQUISITION, &CHavanaDlg::OnBnClickedToggleAcquisition)
	ON_BN_CLICKED(IDC_TOGGLE_RECORDING, &CHavanaDlg::OnBnClickedToggleRecording)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_TOGGLE_SAVING, &CHavanaDlg::OnBnClickedToggleSaving)
	ON_EN_CHANGE(IDC_EDIT_ADC_RATE, &CHavanaDlg::OnEnChangeEditAdcRate)
	ON_CBN_SELCHANGE(IDC_COMBO_FLIM_INPUT_RANGE, &CHavanaDlg::OnCbnSelchangeComboFlimInputRange)
	ON_CBN_SELCHANGE(IDC_COMBO_OCT_INPUT_RANGE, &CHavanaDlg::OnCbnSelchangeComboOctInputRange)
	ON_BN_CLICKED(IDC_BUTTON1, &CHavanaDlg::OnBnClickedButton1)
	
	ON_BN_CLICKED(IDC_CHECK_INTERNAL_SYNC, &CHavanaDlg::OnBnClickedCheckInternalSync)
	ON_BN_CLICKED(IDC_CHECK_PMT_GAIN, &CHavanaDlg::OnBnClickedCheckPmtGain)
	ON_BN_CLICKED(IDC_CHECK_FLIM_SYNC, &CHavanaDlg::OnBnClickedCheckFlimSync)

	ON_BN_CLICKED(IDC_CHECK_GALVO_SCAN, &CHavanaDlg::OnBnClickedCheckGalvoScan)

	ON_BN_CLICKED(IDC_CHECK_ZABER_STAGE, &CHavanaDlg::OnBnClickedCheckZaberStage)
	ON_BN_CLICKED(IDC_BUTTON_ZABER_HOME, &CHavanaDlg::OnBnClickedButtonZaberHome)
	ON_BN_CLICKED(IDC_BUTTON_ZABER_STOP, &CHavanaDlg::OnBnClickedButtonZaberStop)
	ON_BN_CLICKED(IDC_BUTTON_ZABER_MOVE_ABSOLUTE, &CHavanaDlg::OnBnClickedButtonMoveAbsolute)
	ON_BN_CLICKED(IDC_BUTTON_ZABER_SET_TARGET_SPEED, &CHavanaDlg::OnBnClickedButtonSetTargetSpeed)	
	ON_EN_CHANGE(IDC_EDIT_ZABER_SET_TARGET_SPEED, &CHavanaDlg::OnEnChangeEditZaberSetTargetSpeed)

	ON_BN_CLICKED(IDC_CHECK_FAULHABER_MOTOR, &CHavanaDlg::OnBnClickedCheckFaulhaberMotor)
	ON_BN_CLICKED(IDC_TOGGLE_FAULHABER_ROTATE, &CHavanaDlg::OnBnClickedToggleFaulhaberRotateMotor)

	ON_BN_CLICKED(IDC_CHECK_ELFORLIGHT_LASER, &CHavanaDlg::OnBnClickedCheckElforlightLaserControl)
	ON_BN_CLICKED(IDC_BUTTON_ELFORLIGHT_INCREASE, &CHavanaDlg::OnBnClickedButtonElforlightIncrease)
	ON_BN_CLICKED(IDC_BUTTON_ELFORLIGHT_DECREASE, &CHavanaDlg::OnBnClickedButtonElforlightDecrease)
	
	ON_EN_CHANGE(IDC_EDIT_GALVO_PP_VOLTAGE, &CHavanaDlg::OnEnChangeEditGalvoPpVoltage)
	ON_EN_CHANGE(IDC_EDIT_GALVO_OFFSET, &CHavanaDlg::OnEnChangeEditGalvoOffset)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CHavanaDlg 메시지 처리기

BOOL CHavanaDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Status bar initialization
	m_StatusBar.Create(WS_CHILD | WS_VISIBLE | SBT_OWNERDRAW, CRect(0, 0, 0, 0), this, 0);
	int strPartDim[2] = { 1100, 1500 }; // , 300, 450 - 1};
	m_StatusBar.SetParts(2, strPartDim);
	m_StatusBar.SetText(_T("OCT FLIM Integrated System Operation Interface Software"), 0, 0);
	m_StatusBar.SetText(_T("(000, 000) 000 (00.0 dB) / Ch. 0 / 0.000 / 0.000 nsec"), 1, 0);

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.
	SetWindowTextW(_T("Havana OCT-FLIM v1.3.3"));

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// Tab 생성 /////////////////////////////////////////////////////////////////////////////////////////////
	m_Tab.InsertItem(0, _T("STREAMING"));
	m_Tab.InsertItem(1, _T("RESULT"));

	CRect Rect;
	m_Tab.GetClientRect(&Rect);
	
	m_VisStream.SetMainDlg(this);
	m_VisStream.Create(IDD_TAB_STREAM, &m_Tab);
	m_VisStream.SetWindowPos(NULL, 2, 22, Rect.Width() - 5, Rect.Height() - 24, SWP_SHOWWINDOW | SWP_NOZORDER);	
	m_pWndShow = &m_VisStream;

	m_VisResult.SetMainDlg(this);
	m_VisResult.Create(IDD_TAB_RESULT, &m_Tab);
	m_VisResult.SetWindowPos(NULL, 2, 22, Rect.Width() - 5, Rect.Height() - 24, SWP_NOZORDER);	

	// Control 초기화 ////////////////////////////////////////////////////////////////////////////////////////
	SetDlgItemInt(IDC_EDIT_ADC_RATE, ADC_RATE);
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO))->SetScrollRange(0, m_Edit_nAlines - 1, FALSE);
	((CScrollBar*)GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO))->SetScrollPos(0);

	// Get Ini File //////////////////////////////////////////////////////////////////////////////////////////
	TCHAR path[_MAX_PATH];
	GetCurrentDirectory(_MAX_PATH, path);
	CString strPath = path;
	m_strIniPath = strPath + (CString)_T("\\Havana.ini");
	GetIniFile();

	// Data Processing Class 초기화 //////////////////////////////////////////////////////////////////////////	
	m_pOCT = new OCTProcess(this); m_pOCT->LoadCalibration(temp_discom); // m_pOCT->Initialize();// OCT Process
	m_pFLIM = new FLIMProcess; // FLIM Process
	m_pDataAcquisition = new DataAcquisition(this); m_pDataAcquisition->SetLambdas();
	m_pDataRecording = new DataRecording(this);

	// Buffer 생성 (For visualization data) //////////////////////////////////////////////////////////////////			
	m_FrameFringe = np::Array<uint16_t, 2>(m_Edit_nScans, m_Edit_nAlines); // 2600 x 1024
	m_FrameOCT    = np::Array<float, 2>(m_Edit_nFFT / 2, m_Edit_nAlines); // 2048 x 1024

	m_FramePulse = np::Array<uint16_t, 2>(m_Edit_nScans * 4, m_Edit_nAlines / 4); // 10400 x 256
	m_FluIntensity = np::Array<float, 2>(m_Edit_nAlines / 4, 4); // 256 x 4 
	m_FluMeanDelay = np::Array<float, 2>(m_Edit_nAlines / 4, 4); // 256 x 4 
	m_FluLifetime  = np::Array<float, 2>(m_Edit_nAlines / 4, 3); // 256 x 3 
		
	float* bg = m_pOCT->GetBg();
	for (int i = 0; i < m_Edit_nAlines; i++)
	{
		int offsetIdx = m_Edit_nScans * i;
		for (int j = 0; j < m_Edit_nScans; j++)
		{			
			m_FrameFringe(offsetIdx + j) = (uint16_t)(bg[j]); // / 2 * sin(IPP_2PI * j * i / 4000) + POWER_2_15);
			m_FramePulse(offsetIdx + j) = (uint16_t)(m_VisStream.m_Params_FLIM.bg); // / 2 * sin(IPP_2PI * j * j / 10000) + POWER_2_15 - 1);
		}
	}	
	(*m_pOCT)(m_FrameOCT.raw_ptr(), m_FrameFringe.raw_ptr());
	(*m_pFLIM)(m_FluIntensity, m_FluMeanDelay, m_FluLifetime, m_VisStream.m_Params_FLIM, m_Edit_PreTrig, m_FramePulse);
	m_pFLIM->LoadMaskData();

	for (int i = 0; i < NUM_BUFFER_THREAD; i++)
	{
		uint16_t* fringe = new uint16_t[2 * m_Edit_nScans * m_Edit_nAlines];
		memset(fringe, 0, 2 * m_Edit_nScans * m_Edit_nAlines * sizeof(uint16_t));
		m_queue_fringe.push(fringe);

		uint16_t* pulse = new uint16_t[m_Edit_nScans * m_Edit_nAlines];
		memset(pulse, 0, m_Edit_nScans * m_Edit_nAlines * sizeof(uint16_t));
		m_queue_pulse.push(pulse);

		float* oct_im = new float[m_Edit_nFFT / 2 * m_Edit_nAlines];
		memset(oct_im, 0, m_Edit_nFFT / 2 * m_Edit_nAlines * sizeof(float));
		m_queue_image.push(oct_im);

		float* flim_res = new float[m_Edit_nScans / 4 * 11];
		memset(flim_res, 0, m_Edit_nScans / 4 * 11 * sizeof(float));
		m_queue_flim.push(flim_res);
	}
	printf("Threading operation buffers are successfully allocated.\n");
					
	// Window 위치 변경
	this->SetWindowPos(NULL, 315, 5, 0, 0, SWP_NOSIZE);

	// Set Timer
	SetTimer(10, 5*60*1000, NULL); // 5분마다 ini 저장

	printf("\nReady to start...\n\n");

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}


BOOL CHavanaDlg::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	return CDialogEx::DestroyWindow();
}


void CHavanaDlg::PostNcDestroy()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	for (int i = 0; i < NUM_BUFFER_THREAD; i++)
	{
		uint16_t* fringe = m_queue_fringe.front();
		m_queue_fringe.pop();
		delete[] fringe;

		uint16_t* pulse = m_queue_pulse.front();
		m_queue_pulse.pop();
		delete[] pulse;

		float* oct_im = m_queue_image.front();
		m_queue_image.pop();
		delete[] oct_im;

		float* flim_res = m_queue_flim .front();
		m_queue_flim.pop();
		delete[] flim_res;
	}
	for (int i = 0; i < NUM_BUFFER_FRAME; i++)
	{
		if (m_awBufferQueue.size())
		{
			uint16_t* buffer = m_awBufferQueue.front();
			if (buffer)
			{
				m_awBufferQueue.pop();
				delete[] buffer;
			}
		}
	}

	KillTimer(10);
	SetIniFile();

	if (m_pDataAcquisition) delete m_pDataAcquisition;
	if (m_pDataRecording) delete m_pDataRecording;
	if (m_pOCT) delete m_pOCT;
	if (m_pFLIM) delete m_pFLIM;

	CDialogEx::PostNcDestroy();
}


void CHavanaDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CHavanaDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pWndShow != NULL)
	{
		m_pWndShow->ShowWindow(SW_HIDE);
		m_pWndShow = NULL;
	}

	int nIndex = m_Tab.GetCurSel();
	switch (nIndex)
	{
	case 0:
		m_VisStream.ShowWindow(SW_SHOW);
		m_pWndShow = &m_VisStream;		

		if (m_VisResult.m_pPulseReviewDlg != nullptr)
		{
			m_VisResult.m_pPulseReviewDlg->DestroyWindow();
			delete m_VisResult.m_pPulseReviewDlg;
		}
		
		GetDlgItem(IDC_TOGGLE_ACQUISITION)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_GALVO_SCAN)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_PMT_GAIN)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_GALVO)->EnableWindow(FALSE);
		GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO)->EnableWindow(FALSE);
		break;
	case 1:
		m_VisResult.ShowWindow(SW_SHOW);
		m_pWndShow = &m_VisResult;
		if (m_ToggleAcquisition.GetCheck())
		{
#if NI_ENABLE
			m_pDataAcquisition->StopSyncFLIM();
			m_pDataAcquisition->StopGalvoScan();
			m_pDataAcquisition->StopGainControl();
#endif
			m_pDataAcquisition->StopAcquisition();
			m_pOCT->stopOCTprocess();
			m_pFLIM->stopFLIMprocess();
			m_VisStream.stopVisualization();			
		}

		if (m_VisStream.m_pCalibrationDlg != nullptr)
		{
			m_VisStream.m_pCalibrationDlg->DestroyWindow();
			delete m_VisStream.m_pCalibrationDlg;
		}
		if (m_VisStream.m_pPulseOverlapDlg != nullptr)
		{
			m_VisStream.m_pPulseOverlapDlg->DestroyWindow();
			delete m_VisStream.m_pPulseOverlapDlg;
		}
		if (m_VisStream.m_pFLIMAnalysisDlg != nullptr)
		{
			m_VisStream.m_pFLIMAnalysisDlg->DestroyWindow();
			delete m_VisStream.m_pFLIMAnalysisDlg;
		}

		GetDlgItem(IDC_TOGGLE_ACQUISITION)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_GALVO_SCAN)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_PMT_GAIN)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_GALVO)->EnableWindow(TRUE);
		GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO)->EnableWindow(TRUE);
		m_VisResult.m_AdjustGalvo = m_ScrollAdjustGalvo;
		break;
	}

	*pResult = 0;
}


void CHavanaDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CScrollBar* pScroll = (CScrollBar*)GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO);

	int line_shift = 1;
	int page_shift = 50;

	if (pScrollBar == (CScrollBar*)pScroll)
	{		
		switch (nSBCode)
		{
		case SB_LINELEFT:
			pScroll->SetScrollPos(pScrollBar->GetScrollPos() - line_shift);
			break;
		case SB_PAGELEFT:
			pScroll->SetScrollPos(pScrollBar->GetScrollPos() - page_shift);
			break;
		case SB_THUMBTRACK:
			pScroll->SetScrollPos(nPos);
			break;
		case SB_PAGERIGHT:
			pScroll->SetScrollPos(pScrollBar->GetScrollPos() + page_shift);
			break;
		case SB_LINERIGHT:
			pScroll->SetScrollPos(pScrollBar->GetScrollPos() + line_shift);
			break;
		}
		m_ScrollAdjustGalvo = pScroll->GetScrollPos();

		if (m_Tab.GetCurSel() == 1)
		{
			m_VisResult.m_AdjustGalvo = m_ScrollAdjustGalvo;
			m_VisResult.MaskEnFaceMap();
			m_VisResult.InvalidateRect(m_VisResult.m_VisRegionRectWnd, FALSE);
		}

		printf("Galvo Adjustment: %d \n", m_ScrollAdjustGalvo);
	}

	//CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CHavanaDlg::OnBnClickedToggleAcquisition()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.	
	UpdateData(TRUE);

	if (m_ToggleAcquisition.GetCheck())
	{		
		//bool isInitSync = (m_CheckFLIMSync) ? m_pDataAcquisition->InitializeSyncFLIM() : false;
		//bool isInitGalvo = (m_CheckGalvoScan) ? m_pDataAcquisition->InitializeGalvoScan() : false;
		
		if (m_pDataAcquisition->InitializeAcquisition()) 
		{
			m_VisStream.startVisualization(); // Result Visualization Thread
			m_pFLIM->startFLIMprocess(); // FLIM Image Processing Thread
			m_pOCT->startOCTprocess(); // OCT Image Processing Thread
			m_pDataAcquisition->StartAcquisition(); // Data Transfer Thread

			//if (isInitGalvo)
			//	m_pDataAcquisition->StartGalvoScan();
			//else
			//	m_pDataAcquisition->StopGalvoScan();

			//if (isInitSync)
			//	m_pDataAcquisition->StartSyncFLIM();
			//else
			//	m_pDataAcquisition->StopSyncFLIM();
		}
	}
	else
	{
		//m_pDataAcquisition->StopSyncFLIM();
		//m_pDataAcquisition->StopGalvoScan();
		m_pDataAcquisition->StopAcquisition();
		m_pOCT->stopOCTprocess();
		m_pFLIM->stopFLIMprocess();
		m_VisStream.stopVisualization();		
	}
}


void CHavanaDlg::OnBnClickedToggleRecording()
{
	if (m_ToggleRecording.GetCheck())
		m_pDataRecording->StartRecording();
	else
		m_pDataRecording->StopRecording();
}


void CHavanaDlg::OnBnClickedToggleSaving()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_ToggleSaving.GetCheck())
		m_pDataRecording->StartSaving();
	else
		m_pDataRecording->StopSaving(); // halted by user
}




void CHavanaDlg::OnBnClickedCheckPmtGain()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

#if NI_ENABLE
	if (m_CheckPmtGainControl)
	{
		if (m_pDataAcquisition->InitializeGainControl())
			m_pDataAcquisition->StartGainControl();
		else
			m_pDataAcquisition->StopGainControl();
	}
	else
		m_pDataAcquisition->StopGainControl();
#endif
}


void CHavanaDlg::OnBnClickedCheckFlimSync()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

#if NI_ENABLE
	if (m_CheckFLIMSync)
	{
		if (m_pDataAcquisition->InitializeSyncFLIM())
			m_pDataAcquisition->StartSyncFLIM();
		else
			m_pDataAcquisition->StopSyncFLIM();
	}
	else
	{
		m_pDataAcquisition->StopSyncFLIM();
	}
#endif
}


void CHavanaDlg::OnBnClickedCheckInternalSync()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_CheckInternalSync)	
		SetDlgItemText(IDC_CHECK_INTERNAL_SYNC, _T("Use External Sync"));
	else
		SetDlgItemText(IDC_CHECK_INTERNAL_SYNC, _T("Use Internal Sync"));

	GetDlgItem(IDC_STATIC_REPETITION)->EnableWindow(m_CheckInternalSync);
	GetDlgItem(IDC_STATIC_REPETITION_KHZ)->EnableWindow(m_CheckInternalSync);
	GetDlgItem(IDC_EDIT_REPETITION)->EnableWindow(m_CheckInternalSync);
}


void CHavanaDlg::OnEnChangeEditGalvoPpVoltage()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CHavanaDlg::OnEnChangeEditGalvoOffset()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CHavanaDlg::OnBnClickedCheckGalvoScan()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

#if NI_ENABLE
	if (m_CheckGalvoScan)
	{
		if (m_pDataAcquisition->InitializeGalvoScan())
			m_pDataAcquisition->StartGalvoScan();
		else
			m_pDataAcquisition->StopGalvoScan();
	}
	else
		m_pDataAcquisition->StopGalvoScan();
#endif
}


void CHavanaDlg::OnBnClickedCheckZaberStage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_CheckZaberStage)
	{
		if (!(m_pDataAcquisition->EnableZaberStage()))
		{
			m_pDataAcquisition->DisableZaberStage();
			return;
		}
		//m_pDataAcquisition->OperateZaberStage(0);
	}
	else
		m_pDataAcquisition->DisableZaberStage();
}


void CHavanaDlg::OnBnClickedButtonZaberHome()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDataAcquisition->OperateZaberStage(0);
}


void CHavanaDlg::OnBnClickedButtonZaberStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDataAcquisition->OperateZaberStage(1);
}


void CHavanaDlg::OnBnClickedButtonMoveAbsolute()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDataAcquisition->OperateZaberStage(2);
}


void CHavanaDlg::OnBnClickedButtonSetTargetSpeed()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.	
}


void CHavanaDlg::OnEnChangeEditZaberSetTargetSpeed()
{	
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDataAcquisition->OperateZaberStage(3);
}


void CHavanaDlg::OnBnClickedCheckFaulhaberMotor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_CheckFaulhaberMotor)
	{
		if (!(m_pDataAcquisition->EnableFaulhaberMotor()))
			m_pDataAcquisition->DisableFaulhaberMotor();
	}
	else
		m_pDataAcquisition->DisableFaulhaberMotor();
}


void CHavanaDlg::OnBnClickedToggleFaulhaberRotateMotor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_ToggleRotateMotor)
	{
		m_pDataAcquisition->OperateFaulhaberMotor(0);
	}
	else
		m_pDataAcquisition->OperateFaulhaberMotor(1);

}


void CHavanaDlg::OnBnClickedCheckElforlightLaserControl()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if (m_CheckElforlightLaser)
	{
		if (!(m_pDataAcquisition->EnableElforlightLaser()))
			m_pDataAcquisition->DisableElforlightLaser();
	}
	else
		m_pDataAcquisition->DisableElforlightLaser();
}


void CHavanaDlg::OnBnClickedButtonElforlightIncrease()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDataAcquisition->OperateElforlightLaser(0);
}


void CHavanaDlg::OnBnClickedButtonElforlightDecrease()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pDataAcquisition->OperateElforlightLaser(1);
}


void CHavanaDlg::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	//CDialogEx::OnOK();
}


void CHavanaDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.	
	if (m_ToggleAcquisition.GetCheck())
		AfxMessageBox(_T("Stop acquisition first!"), MB_ICONERROR);
	else
		DestroyWindow();
}


BOOL CHavanaDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CHavanaDlg::OnEnChangeEditAdcRate()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;

	UpdateData(TRUE);
	m_VisStream.m_Params_FLIM.samp_intv = 1000.0f / (float)m_Edit_AdcRate;
	for (int i = 0; i < 7; i++) {
		str.Format(_T("%2.1fns"), (m_VisStream.m_nScansFLIM - 1) / 8 * m_VisStream.m_Params_FLIM.samp_intv * (i + 1));
		m_VisStream.SetDlgItemTextW(IDC_STATIC_TIME1 + i, str);
	}
}


void CHavanaDlg::OnCbnSelchangeComboFlimInputRange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;

	UpdateData(TRUE);
	float volt = m_adInputVol[m_Combo_FLIMInputRange];
	str.Format(_T("%1.2fV"), volt / 2);
	m_VisStream.SetDlgItemTextW(IDC_STATIC_HIGH_VOL, str);
	str.Format(_T("%1.2fV"), -volt / 2);
	m_VisStream.SetDlgItemTextW(IDC_STATIC_LOW_VOL, str);
}


void CHavanaDlg::OnCbnSelchangeComboOctInputRange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;

	UpdateData(TRUE);
	float volt = m_adInputVol[m_Combo_OCTInputRange];
	str.Format(_T("%1.2fV"), volt / 2);
	m_VisStream.SetDlgItemTextW(IDC_STATIC_HIGH_VOL2, str);
	str.Format(_T("%1.2fV"), -volt / 2);
	m_VisStream.SetDlgItemTextW(IDC_STATIC_LOW_VOL2, str);
}


void CHavanaDlg::SetIniFile()
{	
	CString strSection, strKey, strValue;

	strSection = _T("configuration");
	 
	UpdateData(TRUE);

	strKey = _T("nAlines");
	strValue.Format(_T("%d"), m_Edit_nAlines);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("nScans");
	strValue.Format(_T("%d"), m_Edit_nScans);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("PreTrigSamps");
	strValue.Format(_T("%d"), m_Edit_PreTrig);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("TrigDelaySamps");
	strValue.Format(_T("%d"), m_Edit_TrigDelay);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("FLIMVoltRange");
	strValue.Format(_T("%d"), m_Combo_FLIMInputRange);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("OCTVoltRange"); 
	strValue.Format(_T("%d"), m_Combo_OCTInputRange);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	//strKey = _T("IsFLIMSync");
	//strValue.Format(_T("%d"), m_CheckFLIMSync);
	//WritePrivateProfileString(strSection,	strKey, strValue, m_strIniPath);
	
	strKey = _T("NumVisFLIMSamps");
	strValue.Format(_T("%d"), m_VisStream.m_nScansFLIM);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("FLIMBg");
	strValue.Format(_T("%.2f"), m_VisStream.m_Params_FLIM.bg);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("FLIMwidth_factor");
	strValue.Format(_T("%.2f"), m_VisStream.m_Params_FLIM.width_factor);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("Colormap");
	strValue.Format(_T("%d"), m_VisStream.m_ComboLUT);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("IsBgRemoved");
	strValue.Format(_T("%d"), m_VisStream.m_CheckBgRemovedFringe);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("Max_dB");
	strValue.Format(_T("%d"), m_VisStream.m_Max_dB);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("Min_dB");
	strValue.Format(_T("%d"), m_VisStream.m_Min_dB);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	for (int i = 0; i < 4; i++)
	{
		strKey.Format(_T("ChannelStart_%d"), i);
		strValue.Format(_T("%.3f"), m_VisStream.m_Params_FLIM.ch_start[i]);
		WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);
	}

	for (int i = 1; i < 4; i++)
	{
		strKey.Format(_T("DelayTimeOffset_%d"), i);
		strValue.Format(_T("%.3f"), m_VisStream.m_Params_FLIM.delay_offset[i - 1]);
		WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);
	}

	strKey = _T("CircCenter");
	strValue.Format(_T("%d"), m_VisStream.m_EditCircCenter);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("GalvoAdjShift");
	strValue.Format(_T("%d"), m_ScrollAdjustGalvo);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("FLIMCh");
	strValue.Format(_T("%d"), m_VisStream.m_Params_FLIM.act_ch);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("FaulhaberRPM");
	strValue.Format(_T("%d"), m_Edit_RPM);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);
	
	strKey = _T("DiscomValue");
	strValue.Format(_T("%d"), m_pOCT->discom_value);
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	strKey = _T("Time");
	CTime time = CTime::GetCurrentTime(); 
	strValue.Format(_T("%04d-%02d-%02d %02d-%02d-%02d"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());
	WritePrivateProfileString(strSection, strKey, strValue, m_strIniPath);

	m_pFLIM->SaveMaskData();
}


void CHavanaDlg::GetIniFile()
{
	TCHAR szBuf[MAX_PATH] = { 0, };
	CString strSection, strKey, strValue;
		
	strSection = _T("configuration");
	
	strKey = _T("PreTrigSamps");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	SetDlgItemInt(IDC_EDIT_PRE_TRIG_SAMPS, _wtoi(strValue));
	
	strKey = _T("TrigDelaySamps");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	SetDlgItemInt(IDC_EDIT_TRIG_DELAY_SAMPS, _wtoi(strValue));

	strKey = _T("FLIMVoltRange");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	((CComboBox *)GetDlgItem(IDC_COMBO_FLIM_INPUT_RANGE))->SetCurSel(_wtoi(strValue));
	OnCbnSelchangeComboFlimInputRange();

	strKey = _T("OCTVoltRange");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	((CComboBox *)GetDlgItem(IDC_COMBO_OCT_INPUT_RANGE))->SetCurSel(_wtoi(strValue));
	OnCbnSelchangeComboOctInputRange();

	//strKey = _T("IsFLIMSync");
	//GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	//strValue.Format(_T("%s"), szBuf);
	//((CButton *)GetDlgItem(IDC_CHECK_FLIM_SYNC))->SetCheck(_wtoi(strValue) == 1);
	
	strKey = _T("NumVisFLIMSamps");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	m_VisStream.SetDlgItemInt(IDC_EDIT_VIS_FLIM_SAMPS, _wtoi(strValue));	
	m_VisStream.OnEnChangeEditVisFlimSamps();

	strKey = _T("FLIMBg");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	m_VisStream.SetDlgItemText(IDC_EDIT_FLIM_BACKGROUND, strValue);
	m_VisStream.OnEnChangeEditFlimBackground();

	strKey = _T("FLIMwidth_factor");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	m_VisStream.SetDlgItemText(IDC_EDIT_FLIM_WIDTH, strValue);
	m_VisStream.OnEnChangeEditFlimWidth();

	strKey = _T("Colormap");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	((CComboBox *)m_VisStream.GetDlgItem(IDC_COMBO_LUT))->SetCurSel(_wtoi(strValue));
	m_VisStream.m_pBitmapOCT->SetBitmap(m_Edit_nFFT / 2, m_Edit_nAlines, 1, _wtoi(strValue));
	m_VisStream.m_pBitmapCircOCT->SetBitmap(m_Edit_nScans / 2, m_Edit_nScans / 2, 1, _wtoi(strValue));

	strKey = _T("IsBgRemoved");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	((CButton *)m_VisStream.GetDlgItem(IDC_CHECK_BG_REMOVED_FRINGE))->SetCheck(_wtoi(strValue) == 1);

	strKey = _T("Max_dB");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	m_VisStream.SetDlgItemInt(IDC_EDIT_ALINE_MAX_DB, _wtoi(strValue));
	m_VisResult.SetDlgItemInt(IDC_EDIT_RES_ALINE_MAX_DB, _wtoi(strValue));

	strKey = _T("Min_dB");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	m_VisStream.SetDlgItemInt(IDC_EDIT_ALINE_MIN_DB, _wtoi(strValue));
	m_VisResult.SetDlgItemInt(IDC_EDIT_RES_ALINE_MIN_DB, _wtoi(strValue));

	strKey = _T("CircCenter");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	m_VisStream.SetDlgItemInt(IDC_EDIT_CIRC_CENTER, _wtoi(strValue));
	m_VisResult.SetDlgItemInt(IDC_EDIT_RES_CIRC_CENTER, _wtoi(strValue));

	for (int i = 0; i < 4; i++)
	{
		strKey.Format(_T("ChannelStart_%d"), i);
		GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
		strValue.Format(_T("%s"), szBuf); 
		m_VisStream.m_Params_FLIM.ch_start[i] = (float)_wtof(strValue);
		m_VisStream.m_Params_FLIM.ch_start_ind[i] = int(round(m_VisStream.m_Params_FLIM.ch_start[i] / m_VisStream.m_Params_FLIM.samp_intv));
		m_VisStream.SetDlgItemText(IDC_EDIT_CHANNEL_START_CH0 + i, strValue);
	}

	for (int i = 1; i < 4; i++)
	{
		strKey.Format(_T("DelayTimeOffset_%d"), i);
		GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
		strValue.Format(_T("%s"), szBuf);
		m_VisStream.m_Params_FLIM.delay_offset[i - 1] = (float)_wtof(strValue);
		m_VisStream.SetDlgItemText(IDC_EDIT_DELAY_OFFSET_CH1 + 2 * (i - 1), strValue);
	}
	
	strKey = _T("FaulhaberRPM");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	SetDlgItemInt(IDC_EDIT_FAULHABER_RPM, _wtoi(strValue));

	strKey = _T("DiscomValue");
	GetPrivateProfileString(strSection, strKey, _T(""), szBuf, MAX_PATH, m_strIniPath);
	strValue.Format(_T("%s"), szBuf);
	temp_discom = _wtoi(strValue);

	UpdateData(TRUE);	
}


void CHavanaDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 10)
	{
		SetIniFile();
		printf("Saved INI file!\n");
	}

	CDialogEx::OnTimer(nIDEvent);
}


////#include "ImageProcess\circularize.h"
////np::Array<uint8_t, 2> rect_img1, rect_img2, rect_tp;
////np::Array<uint8_t, 2> circ_img;
//
//
//
////#include <mkl_df.h>
//#include <chrono>
//
////#define NX 120                     /* Size of partition, number of breakpoints */
////#define NY 256
////#define NSITE 12000                 /* Number of interpolation sites */
//
//typedef enum _DayOfWeek {
//	Sunday = 0,
//	Monday,
//	Tuesday,
//	Wednesday,
//	Thursday,
//	Friday,
//	Saturday
//} DayOfWeek;
//
//union Test {
//	uint64_t i1;
//	uint32_t i2;
//	uint16_t i3;
//	uint8_t  i4;
//};
//
//#include <assert.h>

void CHavanaDlg::OnBnClickedButton1() // Test Purpose
{
//	DayOfWeek week1;
//	week1 = Tuesday;
//
//	union Test t;
//	
//	int i = 0;
//	printf("%p\n", &i);
//
//	printf("%#p 0x%P 0x%p 0x%P\n", &t.i1, &t.i2, &t.i3, &t.i4);
//
//	printf("%s\n", __FILE__);
//
//	assert(&i == nullptr);
//
//#if defined(NDEBUG)
//	printf("release mode\n");
//#endif

	//np::Array<uint16_t, 2> test(2048, 2 * 2600);
	//np::Array<uint16_t, 2> buff(2048, 2 * 2600);

	//int size = 2048 * 2 * 2600;
	//int N = 1;
	//int size1 = size / N;

	//std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();
	//memcpy(buff, test, sizeof(uint16_t) * size);
	//std::chrono::system_clock::time_point end1 = std::chrono::system_clock::now();
	//std::chrono::milliseconds mil1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
	//printf("elapsed time : %d msec\n", (int)mil1.count());

	//std::chrono::system_clock::time_point start2 = std::chrono::system_clock::now();
	//for (int i = 0; i < N; i++)
	//	memcpy(buff + i * size1, test + i * size1, sizeof(uint16_t) * size1);
	//std::chrono::system_clock::time_point end2 = std::chrono::system_clock::now();
	//std::chrono::milliseconds mil2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2);
	//printf("elapsed time : %d msec\n", (int)mil2.count());
	
	//unsigned char ap, dui;
	//for (ap = 0xca; ap != 0xfe; ++ap)
	//	for (dui = 0xa1; dui != 0xff; ++dui) 
	//		printf("%c%c ", ap, dui);

}

/*
	CString str = _T("OCT FLIM interleaved raw data (*.data) | *.data; | All Files (*.*) | *.*||");
	CFileDialog dlg(TRUE, _T("data"), NULL, OFN_HIDEREADONLY, str);

	if (IDOK == dlg.DoModal())
	{
		CString fullpath = dlg.GetPathName();
		CString name = dlg.GetFileName();
		CString title = dlg.GetFileTitle();
		CString path = fullpath.Left(fullpath.GetLength() - name.GetLength());
		CString bg_path = path + title + (CString)_T(".background");
		CString calib_path = path + title + (CString)_T(".calibration");
		CString ini_path = path + title + (CString)_T(".ini");

		HANDLE hFile = INVALID_HANDLE_VALUE;
		hFile = CreateFile(fullpath, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

		np::Array<uint16_t, 2> frame_data(2 * m_Edit_nScans, m_Edit_nAlines);
		DWORD dwRead;

		int N = 1;
		for (int i = 0; i < N; i++)
			ReadFile(hFile, frame_data, sizeof(uint16_t) * 2 * m_Edit_nScans * m_Edit_nAlines, &dwRead, NULL);

		CloseHandle(hFile);

		uint16_t* fringe_ptr = m_FrameFringe.raw_ptr();
		uint16_t* flim_ptr = m_FramePulse.raw_ptr();

		// Data deinterleaving
		ippsCplxToReal_16sc((Ipp16sc *)frame_data.raw_ptr(), (Ipp16s *)fringe_ptr, (Ipp16s *)flim_ptr, m_Edit_nScans * m_Edit_nAlines);
		
		m_VisStream.m_Params_FLIM.act_ch = 1;
		//std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();

		(*m_pOCT)(m_FrameOCT.raw_ptr(), m_FrameFringe.raw_ptr());
		(*m_pFLIM)(m_FluIntensity, m_FluMeanDelay, m_FluLifetime, m_VisStream.m_Params_FLIM, m_Edit_PreTrig, m_FramePulse);
		//std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();
//std::chrono::system_clock::time_point end1 = std::chrono::system_clock::now();
		//std::chrono::milliseconds mil1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);
		//printf("elapsed time : %d msec\n", (int)mil1.count());

		//scope(&m_FluLifetime(0, 1), 256, 0, 10); // &m_pFLIM->_resize.ext_src(0, 1), m_pFLIM->_resize.upSampleLength, 0.0f, 65535.0f);

		np::Array<float, 2> temp(m_pFLIM->_resize.nx, m_pFLIM->_resize.ny);



		//CScope scope;
		//scope(temp.raw_ptr(), m_pFLIM->_resize.nx, -32768, 32768);
		//scope(&m_pFLIM->_resize.crop_src(0, 0), m_pFLIM->_resize.nx, -32678, 32678);

		//printf("%f %f\n", m_FluMeanDelay(0, 0), m_FluMeanDelay(0, 2));
		
		
		Ipp32f mean, std;
		ippsMeanStdDev_32f(&m_FluIntensity(0, 2), 256, &mean, &std, ippAlgHintFast);
		printf("Intensity: %.4f ± %.4f nsec\n", mean, std);
		ippsMeanStdDev_32f(&m_FluLifetime(0, 1), 256, &mean, &std, ippAlgHintFast);
		printf("Lifetime: %.4f ± %.4f AU\n", mean, std);

		//ippsMeanStdDev_32f(&m_FluMeanDelay(0, 0), 256, &mean, &std, ippAlgHintFast);
		//printf("Mean Delay (ch0): %.4f ± %.4f AU\n", mean, std);
		//ippsMeanStdDev_32f(&m_FluMeanDelay(0, 1), 256, &mean, &std, ippAlgHintFast);
		//printf("Mean Delay (ch1): %.4f ± %.4f AU\n", mean, std);
		//ippsMeanStdDev_32f(&m_FluMeanDelay(0, 2), 256, &mean, &std, ippAlgHintFast);
		//printf("Mean Delay (ch2): %.4f ± %.4f AU\n", mean, std);
		
		
	}
}


*/


	/*
	int radi = 100, alines = 256;
	circularize circ(radi, alines, false);

	rect_img1 = np::Array<uint8_t, 2>(alines, radi);
	rect_img2 = np::Array<uint8_t, 2>(alines, radi);
	rect_tp = np::Array<uint8_t, 2>(radi, alines);

	memset(rect_img1.raw_ptr(), 0, sizeof(uint8_t) * rect_img1.length());
	memset(rect_img2.raw_ptr(), 0, sizeof(uint8_t) * rect_img2.length());
	for (int i = 92; i < 96; i++)
		memcpy(rect_img1 + i * alines, m_VisResult.m_pBitmapIntensityMap->GetPtr() + m_VisResult.m4_nAlines * m_VisResult.m_SliderCurFrame, alines);

	//	memcpy(rect_img + i * alines, m_VisResult.m_pBitmapIntensityMap->GetPtr() + m_VisResult.m4_nAlines * m_VisResult.m_SliderCurFrame, alines);
	
	//for (int i = 96; i < 100; i++)
	//	memset(rect_img2 + i * alines, 0, alines);
	//	memcpy(rect_img + i * alines, m_VisResult.m_pBitmapLifetimeMap->GetPtr() + m_VisResult.m4_nAlines * m_VisResult.m_SliderCurFrame, alines);
	
	ippiTranspose_8u_C1R(rect_img1.raw_ptr(), alines, rect_tp.raw_ptr(), radi, { alines, radi });

	circ_img = np::Array<uint8_t, 2>(2*radi, 2*radi);	
	memset(circ_img.raw_ptr(), 0, sizeof(uint8_t) * circ_img.length());
	
	circ(rect_tp, circ_img);
			
	Imshow imshow;
	imshow(circ_img, circ_img.size(0), circ_img.size(1)); // , 0.0f, 100.0f);

	//ippiTranspose_8u_C1R(rect_img2.raw_ptr(), alines, rect_tp.raw_ptr(), radi, { alines, radi });

	//circ(rect_tp, circ_img);

	//imshow(circ_img, circ_img.size(0), circ_img.size(1)); // , 0.0f, 100.0f);
}

*/
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//Ipp16u* frame = ippsMalloc_16u(2 * m_Edit_nScans * m_Edit_nAlines);
	//Ipp16u* oct = ippsMalloc_16u(m_Edit_nScans * m_Edit_nAlines);
	//Ipp16u* flim = ippsMalloc_16u(m_Edit_nScans * m_Edit_nAlines);
	//std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();	
	//ippsCplxToReal_16sc((Ipp16sc *)frame, (Ipp16s *)oct, (Ipp16s *)flim, m_Edit_nScans * m_Edit_nAlines); // DeInterleave
	//std::chrono::system_clock::time_point end1 = std::chrono::system_clock::now();
	//	printf("elapsed time : %d msec\n", (int)mil1.count());	

//	std::chrono::system_clock::time_point start1 = std::chrono::system_clock::now();
//	(*m_pOCT)(m_FrameOCT, m_FrameFringe);
	//(*m_pFLIM)(m_FluIntensity, m_FluMeanDelay, m_FluLifetime, m_VisStream.m_Params_FLIM, m_Edit_PreTrig, m_FrameFLIM);
//	std::chrono::system_clock::time_point end1 = std::chrono::system_clock::now();

//	std::chrono::milliseconds mil1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1);			
//	printf("elapsed time : %d msec\n", (int)mil1.count());
	


