#ifndef _FLIM_ANALYSIS_DLG_H_
#define _FLIM_ANALYSIS_DLG_H_

#include "afxwin.h"
#include "afxcmn.h"

class CHavanaDlg;
class CVisStream;

struct Histogram
{
public:
	Histogram() : pHistObj(nullptr), pBuffer(nullptr), pLevels(nullptr), lowerLevel(0), upperLevel(0)
	{
	}

	~Histogram()
	{
		if (pHistObj) ippsFree(pHistObj);
		if (pBuffer) ippsFree(pBuffer);
		if (pLevels) ippsFree(pLevels);
	}

public:
	void operator() (const Ipp32f* pSrc, Ipp32u* pHist, Ipp32f* _lowerLevel, Ipp32f* _upperLevel)
	{
		if ((lowerLevel != *_lowerLevel) || (upperLevel != *_upperLevel))
		{
			// set vars
			lowerLevel = *_lowerLevel;
			upperLevel = *_upperLevel;

			// initialize spec
			ippiHistogramUniformInit(ipp32f, _lowerLevel, _upperLevel, &nLevels, 1, pHistObj);

			// check levels of bins
			ippiHistogramGetLevels(pHistObj, &pLevels);
		}

		// calculate histogram
		ippiHistogram_32f_C1R(pSrc, roiSize.width * sizeof(Ipp32f), roiSize, pHist, pHistObj, pBuffer);
	}

public:
	void initialize(int _nBins, int _length)
	{
		// init vars
		roiSize = { _length, 1 };
		nBins = _nBins; nLevels = nBins + 1;
		pLevels = ippsMalloc_32f(nLevels);

		// get sizes for spec and buffer
		ippiHistogramGetBufferSize(ipp32f, roiSize, &nLevels, 1/*nChan*/, 1/*uniform*/, &sizeHistObj, &sizeBuffer);

		pHistObj = (IppiHistogramSpec*)ippsMalloc_8u(sizeHistObj);
		pBuffer = (Ipp8u*)ippsMalloc_8u(sizeBuffer);		
	}

private:
	IppiSize roiSize;
	int nBins, nLevels;
	int sizeHistObj, sizeBuffer;
	Ipp32f lowerLevel, upperLevel;
	IppiHistogramSpec* pHistObj;
	Ipp8u* pBuffer;
	Ipp32f* pLevels;
};


// FLIMAnalysisDlg 대화 상자입니다.

class FLIMAnalysisDlg : public CDialogEx
{
	DECLARE_DYNAMIC(FLIMAnalysisDlg)

public:
	FLIMAnalysisDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~FLIMAnalysisDlg();

private:
	CHavanaDlg* m_pMainDlg;
	CVisStream* m_pVisStream;

	Histogram histogram_intensity;
	Histogram histogram_lifetime;

private:
	CPen m_DataPen;
	CStatic m_GraphIntensity, m_GraphLifetime;
	CStatic m_ColorbarIntensity, m_ColorbarLifetime;

public:
	CRect m_RectGraphIntensity, m_RectGraphLifetime;
	CRect m_RectColorbarIntensity, m_RectColorbarLifetime;
	CRect m_VisRegionRectWnd;

private:
	int m_nBins;
	float m_IncrementX, m_IncrementY;
	unsigned int *m_pHistogramIntensity, *m_pHistogramLifetime;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_FLIM_ANALYSIS };
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

	void Draw(float* ScanIntensity, float* ScanLifetime);

};

#endif