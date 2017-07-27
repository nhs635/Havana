// CalibrationDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Havana.h"
#include "HavanaDlg.h"
#include "CalibrationDlg.h"
#include "afxdialogex.h"

#include <Tab View\VisStream.h>
#include <ImageProcess\OCTProcess\OCTProcess.h>

// CalibrationDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CalibrationDlg, CDialogEx)

CalibrationDlg::CalibrationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_CALIBRATION, pParent),
	m_CheckResult(FALSE),
	m_Edit_DiscomVal(0),
	m_pScopeData(nullptr), 
	m_MouseClickAvailable(FALSE), m_MouseClick(FALSE), m_GoAhead(FALSE),
	m_GridPen(PS_SOLID, 1, RGB(0, 64, 0)),
	m_DataPen(PS_SOLID, 1, RGB(255, 255, 255)),
	m_DataPen1(PS_SOLID, 1, RGB(255, 0, 0))
{

}


CalibrationDlg::~CalibrationDlg()
{
}


void CalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_Edit_DiscomVal);
	DDX_Check(pDX, IDC_CHECK_RESULT, m_CheckResult);
	DDX_Control(pDX, IDC_STATIC_RESULT, m_StaticResult);
}


BEGIN_MESSAGE_MAP(CalibrationDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_BACKGROUND, &CalibrationDlg::OnBnClickedButtonBackground)
	ON_BN_CLICKED(IDC_BUTTON_D1, &CalibrationDlg::OnBnClickedButtonD1)
	ON_BN_CLICKED(IDC_BUTTON_D2, &CalibrationDlg::OnBnClickedButtonD2)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_CALIB, &CalibrationDlg::OnBnClickedButtonGenerateCalib)
	ON_EN_CHANGE(IDC_EDIT1, &CalibrationDlg::OnEnChangeEdit1)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON_GO_AHEAD, &CalibrationDlg::OnBnClickedButtonGoAhead)
	ON_STN_CLICKED(IDC_STATIC_BACKGROUND, &CalibrationDlg::OnStnClickedStaticBackground)
	ON_STN_CLICKED(IDC_STATIC_D1, &CalibrationDlg::OnStnClickedStaticD1)
	ON_STN_CLICKED(IDC_STATIC_D2, &CalibrationDlg::OnStnClickedStaticD2)
	ON_STN_CLICKED(IDC_STATIC_GENERATE_CALIB, &CalibrationDlg::OnStnClickedStaticGenerateCalib)
END_MESSAGE_MAP()


// CalibrationDlg 메시지 처리기입니다.
void CalibrationDlg::SetMainDlg(CHavanaDlg * pMainDlg)
{
	m_pMainDlg = pMainDlg;
	m_pVisStream = &(m_pMainDlg->m_VisStream);
	m_pOCT = m_pMainDlg->m_pOCT;
}


BOOL CalibrationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.	

	// Discom Value
	m_Edit_DiscomVal = m_pOCT->discom_value;
	UpdateData(FALSE);

	// Get Image Region Rect
	CWnd *pWnd = this->GetDlgItem(IDC_STATIC_RESULT);
	pWnd->GetWindowRect(&m_ScopeRectWnd);
	this->ScreenToClient(&m_ScopeRectWnd);
	pt = { -m_ScopeRectWnd.left, -m_ScopeRectWnd.top };
	m_ScopeRect = m_ScopeRectWnd;
	m_ScopeRect.OffsetRect(pt);

	// Get Last Update Information
	HANDLE hFile;
	FILETIME ftLastWrite, ftLocal;
	SYSTEMTIME st;
	if ((hFile = CreateFile(_T("bg.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
	{
		CString str;
		GetFileTime(hFile, NULL, NULL, &ftLastWrite);
		FileTimeToLocalFileTime(&ftLastWrite, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		str.Format(_T("Last Update: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetDlgItemTextW(IDC_STATIC_BACKGROUND, str);
		CloseHandle(hFile);

		GetDlgItem(IDC_STATIC_BACKGROUND)->EnableWindow(TRUE);
	}
	else
	{
		SetDlgItemTextW(IDC_STATIC_BACKGROUND, _T("Last Update: X"));
	}
	
	if ((hFile = CreateFile(_T("d1.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
	{
		CString str;
		GetFileTime(hFile, NULL, NULL, &ftLastWrite);
		FileTimeToLocalFileTime(&ftLastWrite, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		str.Format(_T("Last Update: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetDlgItemTextW(IDC_STATIC_D1, str);

		np::Array<uint16_t, 2> frame(m_pMainDlg->m_Edit_nScans, m_pMainDlg->m_Edit_nAlines);		
		DWORD dwRead;
		ReadFile(hFile, frame.raw_ptr(), sizeof(uint16_t) * frame.length(), &dwRead, NULL);
		for (int i = 0; i < frame.size(0); i++)
			m_pOCT->GetFringe(0)[i] = (float)frame(i, 0);

		CloseHandle(hFile);

		GetDlgItem(IDC_STATIC_D1)->EnableWindow(TRUE);
	}
	else
	{
		SetDlgItemTextW(IDC_STATIC_D1, _T("Last Update: X"));
	}

	if ((hFile = CreateFile(_T("d2.bin"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
	{
		CString str;
		GetFileTime(hFile, NULL, NULL, &ftLastWrite);
		FileTimeToLocalFileTime(&ftLastWrite, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		str.Format(_T("Last Update: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetDlgItemTextW(IDC_STATIC_D2, str);
		
		np::Array<uint16_t, 2> frame(m_pMainDlg->m_Edit_nScans, m_pMainDlg->m_Edit_nAlines);
		DWORD dwRead;
		ReadFile(hFile, frame.raw_ptr(), sizeof(uint16_t) * frame.length(), &dwRead, NULL);
		for (int i = 0; i < frame.size(0); i++)
			m_pOCT->GetFringe(1)[i] = (float)frame(i, 0);

		CloseHandle(hFile);

		GetDlgItem(IDC_STATIC_D2)->EnableWindow(TRUE);
	}
	else
	{
		SetDlgItemTextW(IDC_STATIC_D2, _T("Last Update: X"));
	}

	if ((hFile = CreateFile(_T("calibration.dat"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
	{
		CString str;
		GetFileTime(hFile, NULL, NULL, &ftLastWrite);
		FileTimeToLocalFileTime(&ftLastWrite, &ftLocal);
		FileTimeToSystemTime(&ftLocal, &st);
		str.Format(_T("Last Update: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		SetDlgItemTextW(IDC_STATIC_GENERATE_CALIB, str);
		CloseHandle(hFile);
	}
	else
	{
		SetDlgItemTextW(IDC_STATIC_GENERATE_CALIB, _T("Last Update: X"));
	}

	// Set Callback Signal Functor
	DrawGraph += [&](float* graph_data, CString title)
	{
		m_DataNum = m_pMainDlg->m_Edit_nFFT;
		m_Min = (float)m_pMainDlg->m_VisStream.m_Min_dB;
		m_Max = (float)m_pMainDlg->m_VisStream.m_Max_dB;
		if (m_pScopeData)
		{
			delete[] m_pScopeData;
			m_pScopeData = nullptr;
		}
		m_pScopeData = new float[m_DataNum];
		memcpy(m_pScopeData, graph_data, sizeof(float) * m_DataNum);
		SetDlgItemTextW(IDC_STATIC_TITLE, title);

		InvalidateRect(m_ScopeRectWnd, FALSE);
	};

	WaitForRange += [&](int& start, int& end)
	{
		m_MouseClickAvailable = TRUE;
		GetDlgItem(IDC_BUTTON_GO_AHEAD)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_GO_AHEAD)->EnableWindow(FALSE);

		GetDlgItem(IDC_BUTTON_BACKGROUND)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_D1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_D2)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_GENERATE_CALIB)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_BACKGROUND)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_D1)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_D2)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_GENERATE_CALIB)->EnableWindow(FALSE);

		m_start = 0; m_end = 0;

		while (!m_GoAhead)
		{
			start = m_start;
			end = m_end;
		}

		if (start > end)
		{
			int temp = start;
			start = end;
			end = temp;
		}

		m_start = 0; m_end = 0;
		m_MouseClickAvailable = FALSE;
		m_GoAhead = FALSE;
		GetDlgItem(IDC_BUTTON_GO_AHEAD)->ShowWindow(FALSE);

		GetDlgItem(IDC_BUTTON_BACKGROUND)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_D1)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_D2)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_GENERATE_CALIB)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_BACKGROUND)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_D1)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_D2)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_GENERATE_CALIB)->EnableWindow(TRUE);
	};

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CalibrationDlg::PostNcDestroy()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pVisStream->m_pCalibrationDlg = nullptr;
	delete this;
}


void CalibrationDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	DestroyWindow();
}


void CalibrationDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_MouseClickAvailable)
	{
		CPoint pt1 = point, pt2;
		pt1.x = pt1.x + pt.x;
		pt1.y = pt1.y + pt.y;

		if (m_ScopeRect.PtInRect(pt1))
		{
			m_MouseClick = TRUE;

			pt2 = { pt1.x - m_ScopeRect.left, pt1.y - m_ScopeRect.top };
			pt2.x = (LONG)floor(double(m_pMainDlg->m_Edit_nFFT * pt2.x) / double(m_ScopeRect.Width()));
			m_start = pt2.x;
			//printf("[%d %d]\n", m_start, 0);
		}
	}
	
	CDialogEx::OnLButtonDown(nFlags, point);
}


void CalibrationDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_MouseClickAvailable)
	{
		if (m_MouseClick)
		{
			CPoint pt1 = point, pt2;
			pt1.x = pt1.x + pt.x;
			pt1.y = pt1.y + pt.y;

			if (m_ScopeRect.PtInRect(pt1))
			{
				pt2 = { pt1.x - m_ScopeRect.left, pt1.y - m_ScopeRect.top };
				pt2.x = (LONG)floor(double(m_pMainDlg->m_Edit_nFFT * pt2.x) / double(m_ScopeRect.Width()));
				m_end = pt2.x;
				
				//printf("[%d %d]\n", m_start, m_end);
				InvalidateRect(m_ScopeRectWnd, FALSE);
			}
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void CalibrationDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_MouseClickAvailable)
	{
		if (m_MouseClick)
		{
			CPoint pt1 = point, pt2;
			pt1.x = pt1.x + pt.x;
			pt1.y = pt1.y + pt.y;

			if (m_ScopeRect.PtInRect(pt1))
			{
				m_MouseClick = FALSE;

				pt2 = { pt1.x - m_ScopeRect.left, pt1.y - m_ScopeRect.top };
				pt2.x = (LONG)floor(double(m_pMainDlg->m_Edit_nFFT * pt2.x) / double(m_ScopeRect.Width()));
				m_end = pt2.x;
				//printf("[%d %d]\n", m_start, m_end);
				InvalidateRect(m_ScopeRectWnd, FALSE);
				
				GetDlgItem(IDC_BUTTON_GO_AHEAD)->EnableWindow(TRUE);
			}
		}	
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CalibrationDlg::OnBnClickedButtonBackground()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pOCT->SetBg(m_pMainDlg->m_FrameFringe);

	CString str;
	SYSTEMTIME st;
	GetLocalTime(&st);
	str.Format(_T("Captured: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetDlgItemTextW(IDC_STATIC_BACKGROUND, str);

	m_DataNum = m_pMainDlg->m_Edit_nScans;
	m_Min = 0.0f; m_Max = 65535.0f;
	if (m_pScopeData)
	{
		delete[] m_pScopeData;
		m_pScopeData = nullptr;
	}
	m_pScopeData = new float[m_DataNum];
	memcpy(m_pScopeData, m_pOCT->GetBg(), sizeof(float) * m_DataNum);
	SetDlgItemTextW(IDC_STATIC_TITLE, _T("Background Signal"));

	InvalidateRect(m_ScopeRectWnd, FALSE);
}


void CalibrationDlg::OnBnClickedButtonD1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pOCT->SetFringe(m_pMainDlg->m_FrameFringe, 0);

	CString str;
	SYSTEMTIME st;
	GetLocalTime(&st);
	str.Format(_T("Captured: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetDlgItemTextW(IDC_STATIC_D1, str);
	
	m_DataNum = m_pMainDlg->m_Edit_nScans;
	m_Min = 0.0f; m_Max = 65535.0f;
	if (m_pScopeData)
	{
		delete[] m_pScopeData;
		m_pScopeData = nullptr;
	}
	m_pScopeData = new float[m_DataNum];
	memcpy(m_pScopeData, m_pOCT->GetFringe(0), sizeof(float) * m_DataNum);
	SetDlgItemTextW(IDC_STATIC_TITLE, _T("D1 Signal"));

	InvalidateRect(m_ScopeRectWnd, FALSE);
}


void CalibrationDlg::OnBnClickedButtonD2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pOCT->SetFringe(m_pMainDlg->m_FrameFringe, 1);

	CString str;
	SYSTEMTIME st;
	GetLocalTime(&st);
	str.Format(_T("Captured: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetDlgItemTextW(IDC_STATIC_D2, str);

	m_DataNum = m_pMainDlg->m_Edit_nScans;
	m_Min = 0.0f; m_Max = 65535.0f;
	if (m_pScopeData)
	{
		delete[] m_pScopeData;
		m_pScopeData = nullptr;
	}
	m_pScopeData = new float[m_DataNum];
	memcpy(m_pScopeData, m_pOCT->GetFringe(1), sizeof(float) * m_DataNum);
	SetDlgItemTextW(IDC_STATIC_TITLE, _T("D2 Signal"));

	InvalidateRect(m_ScopeRectWnd, FALSE);
}


void CalibrationDlg::OnBnClickedButtonGenerateCalib()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	m_pOCT->GenerateCalibration(m_Edit_DiscomVal, m_CheckResult);
	
	CString str;
	SYSTEMTIME st;
	GetLocalTime(&st);
	str.Format(_T("Calibrated: %4d-%02d-%02d %02d:%02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	SetDlgItemTextW(IDC_STATIC_GENERATE_CALIB, str);
}


void CalibrationDlg::OnEnChangeEdit1()
{
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
	m_pOCT->ChangeDiscomValue(m_Edit_DiscomVal);
	m_pOCT->discom_value = m_Edit_DiscomVal;
}


void CalibrationDlg::OnBnClickedButtonGoAhead()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_GoAhead = TRUE;
}


void CalibrationDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.

	CPaintDC dcScope(&m_StaticResult);
	CDC dcMem;

	// Assigning memory DC
	dcMem.CreateCompatibleDC(&dc);
	CBitmap btmp;
	btmp.CreateCompatibleBitmap(&dc, m_ScopeRect.Width(), m_ScopeRect.Height());
	dcMem.SelectObject(&btmp);

	// Draw graph
	dcMem.FillSolidRect(m_ScopeRect, RGB(0, 0, 0));

	dcMem.SelectObject(&m_GridPen);
	for (int i = 0; i < 20; i++)
	{
		dcMem.MoveTo(m_ScopeRect.Width() * i / 20, 0);
		dcMem.LineTo(m_ScopeRect.Width() * i / 20, m_ScopeRect.Height());
	}

	if (m_pScopeData)
	{
		float xInc = float(m_ScopeRect.Width()) / float(m_DataNum - 1);
		float yInc = float(m_ScopeRect.Height()) / float(m_Max - m_Min);	

		dcMem.SelectObject(&m_DataPen);
		for (int i = 0; i < m_DataNum - 1; i++)
		{
			dcMem.MoveTo(int(xInc * (i)), m_ScopeRect.Height() - int(yInc * (m_pScopeData[i] - m_Min)));
			dcMem.LineTo(int(xInc * (i + 1)), m_ScopeRect.Height() - int(yInc * (m_pScopeData[i + 1] - m_Min)));
		}

		if (m_MouseClickAvailable)
		{
			dcMem.SelectObject(&m_DataPen1);
			
			int start = m_start;
			int end = m_end;

			if (start > end)
			{
				int temp = start;
				start = end;
				end = temp;
			}

			for (int i = start; i < end; i++)
			{
				dcMem.MoveTo(int(xInc * (i)), m_ScopeRect.Height() - int(yInc * (m_pScopeData[i] - m_Min)));
				dcMem.LineTo(int(xInc * (i + 1)), m_ScopeRect.Height() - int(yInc * (m_pScopeData[i + 1] - m_Min)));
			}
		}		

		CString str;
		str.Format(_T("%d"), int(m_Max));
		GetDlgItem(IDC_STATIC_Y_MAX)->SetWindowTextW(str);
		str.Format(_T("%d"), int(m_Min));
		GetDlgItem(IDC_STATIC_Y_MIN)->SetWindowTextW(str);
		str.Format(_T("%d"), m_DataNum);
		GetDlgItem(IDC_STATIC_X_MAX)->SetWindowTextW(str);
	}	

	dcScope.BitBlt(0, 0, m_ScopeRect.Width(), m_ScopeRect.Height(), &dcMem, 0, 0, SRCCOPY);
}


void CalibrationDlg::OnStnClickedStaticBackground()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_DataNum = m_pMainDlg->m_Edit_nScans;
	m_Min = 0.0f; m_Max = 65535.0f;
	if (m_pScopeData)
	{
		delete[] m_pScopeData;
		m_pScopeData = nullptr;
	}
	m_pScopeData = new float[m_DataNum];
	memcpy(m_pScopeData, m_pOCT->GetBg(), sizeof(float) * m_DataNum);
	SetDlgItemTextW(IDC_STATIC_TITLE, _T("Stored Background Signal"));

	InvalidateRect(m_ScopeRectWnd, FALSE);
}


void CalibrationDlg::OnStnClickedStaticD1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_DataNum = m_pMainDlg->m_Edit_nScans;
	m_Min = 0.0f; m_Max = 65535.0f;
	if (m_pScopeData)
	{
		delete[] m_pScopeData;
		m_pScopeData = nullptr;
	}
	m_pScopeData = new float[m_DataNum];
	memcpy(m_pScopeData, m_pOCT->GetFringe(0), sizeof(float) * m_DataNum);
	SetDlgItemTextW(IDC_STATIC_TITLE, _T("Stored D1 Signal"));

	InvalidateRect(m_ScopeRectWnd, FALSE);
}


void CalibrationDlg::OnStnClickedStaticD2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_DataNum = m_pMainDlg->m_Edit_nScans;
	m_Min = 0.0f; m_Max = 65535.0f;
	if (m_pScopeData)
	{
		delete[] m_pScopeData;
		m_pScopeData = nullptr;
	}
	m_pScopeData = new float[m_DataNum];
	memcpy(m_pScopeData, m_pOCT->GetFringe(1), sizeof(float) * m_DataNum);
	SetDlgItemTextW(IDC_STATIC_TITLE, _T("Stored D2 Signal"));

	InvalidateRect(m_ScopeRectWnd, FALSE);
}


void CalibrationDlg::OnStnClickedStaticGenerateCalib()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
}
