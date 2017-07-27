#ifndef _VIS_RESULT_H_
#define _VIS_RESULT_H_

#include "afxwin.h"
#include "afxcmn.h"

#include <iostream>
#include <vector>

#include "ImageProcess\circularize.h"
#include "ImageProcess\medfilt.h"

class CHavanaDlg;

class DataRecording;
class MyBitmap;
class OCTProcess;
class FLIMProcess;
class PulseReviewDlg;

// CVisResult 대화 상자입니다.
class CVisResult : public CDialogEx
{
	DECLARE_DYNAMIC(CVisResult)

public:
	CVisResult(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CVisResult();

private:
	CHavanaDlg* m_pMainDlg;

	CStatic m_StaticResImage;
	CStatic m_StaticResProjection;
	CStatic m_StaticResColorbarProjection;
	CStatic m_StaticResIntensityMap;
	CStatic m_StaticResColorbarIntensity;
	CStatic m_StaticResLifetimeMap;
	CStatic m_StaticResColorbarLifetime;

	CPen m_GreenPen, m_WhitePen, m_RedPen;

public:
	MyBitmap* m_pBitmapOCT, *m_pBitmapCircOCT;

	np::Array<BYTE, 2> m_TempBitmapOCT, m_TempBitmapCircOCT;
	np::Array<BYTE> m_RingIntensity, m_RingLifetime, m_RingEnhanced;
	
	MyBitmap* m_pBitmapProjection;
	MyBitmap* m_pBitmapIntensityMap;
	MyBitmap* m_pBitmapLifetimeMap;
	MyBitmap* m_pBitmapEnhancedMap; // HSV (HUE: lifetime & VALUE: Intensity)

	MyBitmap* m_pColorbarProjection;
	MyBitmap* m_pColorbarIntensity;
	MyBitmap* m_pColorbarLifetime;

	CRect m_ResImageRect, m_ResProjectionRect, m_ResIntensityMapRect, m_ResLifetimeMapRect, 
		m_CBarRegionRect1, m_CBarRegionRect2, m_CBarRegionRect3, m_VisRegionRectWnd;
	POINT pt;

public:
	PulseReviewDlg* m_pPulseReviewDlg;
	
public: // Buffers
	std::vector<np::Array<float, 2>> m_OCT_Image32f; // (2048 x 1024) x N
	std::vector<np::Array<float, 2>> m_IntensityMap32f; // (256 x N) x 3
	std::vector<np::Array<float, 2>> m_LifetimeMap32f; // (256 x N) x 3
	np::Array<float, 2> m_Projection32f; // (1024 x N)

	np::Array<BYTE, 2> m_OCT_Image8u; // (1024 x 2048)

	std::vector <np::Array<float, 2>> m_PulseCrop;
	std::vector <np::Array<float, 2>> m_PulseMask;
//	std::vector <np::Array<float, 2>> m_PulseSpline;

	circularize circ_OCT, circ_writing_OCT;
	medfilt rect_medfilt;
	medfilt proj_medfilt;
	medfilt intensity4_medfilt, lifetime4_medfilt;

	FLIMProcess* m_pFLIMforExt;
	BOOL m_IsExternData, m_IsSaved;
	CString m_path;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_RESULT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	void SetMainDlg(CHavanaDlg* pMainDlg);
	virtual BOOL OnInitDialog();
	void SetCircs(int scans, int alines);
	void SetBitmaps();
	void SetBuffers(int nFrame);
	virtual void PostNcDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();

public: // Control Data
	int m_SliderCurFrame;
	UINT m_RadioSelectData;
	int m_EditResMaxDb, m_EditResMinDb;	
	int m_EditCircCenter;
	int m_AdjustGalvo;
	float m_EditMaxIntensity, m_EditMinIntensity;
	float m_EditMaxLifetime, m_EditMinLifetime;
	int m_ComboResLut, m_ComboResFlimShow;
	CProgressCtrl m_ProgressRes;
	BOOL m_CheckRGBWriting, m_CheckShowGuideLine, m_CheckCircularize, m_CheckResNormalization, m_CheckHSVEnhancing;
	BOOL m_ToggleMeasureDistance;

public:
	int m_nScans, m_nScansFLIM, m_nScansFFT, m_nAlines, m4_nAlines, m_nAlines4, m_nFrame;
	int m_nSizeFrame, m_nSizeImageOCT;

public:
	CPoint pt_dist1[2], pt_dist2[2];
	int n_click;

public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSetRadioSelectData(UINT value);
	afx_msg void OnCbnSelchangeComboResLut();
	afx_msg void OnCbnSelchangeComboResFlimShow();
	afx_msg void OnBnClickedCheckResShowGuideline();
	afx_msg void OnBnClickedCheckResHsvEnhancing();
	afx_msg void OnBnClickedCheckResNormalization();
	afx_msg void OnBnClickedCheckResMeasureDistance();
	afx_msg void OnBnClickedButtonImageProcessing();
	afx_msg void OnEnChangeEditResAlineMaxDb();
	afx_msg void OnEnChangeEditResAlineMinDb();
	afx_msg void OnEnChangeEditResCircCenter();
	afx_msg void OnEnChangeEditResIntensityMax();
	afx_msg void OnEnChangeEditResIntensityMin();
	afx_msg void OnEnChangeEditResLifetimeMax();
	afx_msg void OnEnChangeEditResLifetimeMin();
	afx_msg void OnBnClickedCheckCircularize();
	afx_msg void OnBnClickedCheckResSaveAsRgb();
	afx_msg void OnBnClickedButtonSaveResult();

public:
	void ImageProcessingInBuffer();
	void ImageProcessingExternal();

	void SaveResults();
	void SaveResultsMerged();
	void SaveResultsMerged3();

	void GetProjection(std::vector<np::Array<float, 2>>& OCT_vector, np::Array<float, 2>& Proj, int offset = 0);
	void ScaleRect(const float* pSrc, uint8_t* pDst);
	void TransposeRect(const uint8_t* pSrc, uint8_t* pDst);
	void MaskEnFaceMap();	

private:
	void Interpolation_32f(const Ipp32f* srcX, const Ipp32f* srcY, Ipp32f* dstX, int srcLen, int dstLen, Ipp32f* dstY);

};

#endif