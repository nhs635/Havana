#ifndef _FLIM_PROCESS_H_
#define _FLIM_PROCESS_H_

#include <thread>
#include <Miscel\objcpp\signal.h>

#include "tbb\parallel_for.h"
#include "tbb\blocked_range.h"

#include "Configuration.h"
#include "mkl_df.h"

struct FLIM_PARAMS
{
	float bg;
	int act_ch;
	float samp_intv;
	float width_factor;
	float ch_start[4];
	int ch_start_ind[4];
	float delay_offset[3];
	BOOL is_broadening;
};

struct FILTER // Gaussian Filtering
{
	FILTER() :
		pSpec(nullptr), pBuf(nullptr), pTaps(nullptr)
	{
	}

	~FILTER()
	{
		if (pSpec) { ippsFree(pSpec); pSpec = nullptr; }
		if (pBuf) { ippsFree(pBuf); pBuf = nullptr; }
		if (pTaps) { ippsFree(pTaps); pTaps = nullptr; }
	}

	void operator() (Ipp32f* pDst, Ipp32f* pSrc, int y)
	{
		ippsFIRSR_32f(pSrc, pDst, srcWidth, pSpec, NULL, NULL, &pBuf[srcWidth * y]);
	}

	void initialize(int _tapsLen, int _srcWidth, int ny)
	{
		tapsLen = _tapsLen;
		srcWidth = _srcWidth;

		pTaps = ippsMalloc_32f(tapsLen);
		float x;
		for (int i = 0; i < tapsLen; i++)
		{
			x = ((float)i - ((float)tapsLen - 1) / 2) / (float)GAUSSIAN_FILTER_STD;
			pTaps[i] = exp(-0.5f * x * x);
		}
		float sum; ippsSum_32f(pTaps, tapsLen, &sum, ippAlgHintFast);
		ippsDivC_32f_I(sum, pTaps, tapsLen);

		ippsFIRSRGetSize(tapsLen, ipp32f, &specSize, &bufSize);
		pSpec = (IppsFIRSpec_32f*)ippsMalloc_8u(specSize);
		pBuf = ippsMalloc_8u(ny * bufSize);
		ippsFIRSRInit_32f(pTaps, tapsLen, ippAlgDirect, pSpec);
	}

private:
	int tapsLen;
	int srcWidth;
	int specSize, bufSize;
	IppsFIRSpec_32f *pSpec;
	Ipp8u *pBuf;
	Ipp32f* pTaps;
};

struct RESIZE
{
public:
	RESIZE() : scoeff(nullptr), pSeq(nullptr), pMask(nullptr), nx(-1)
	{
		memset(start_ind, 0, sizeof(int) * 4);
		memset(end_ind, 0, sizeof(int) * 4);
	}

	~RESIZE()
	{
		if (scoeff) delete[] scoeff;
		if (pSeq) ippFree(pSeq);
		if (pMask) ippFree(pMask);
	}

	void operator() (const np::Array<uint16_t, 2> &src, int _upSampleFactor, int PreTrig, const FLIM_PARAMS& pParams)
	{
		// 0. Initialize
		int _nx = pParams.ch_start_ind[3] - pParams.ch_start_ind[0] + FLIM_CH_START_5; // roi_range: ch1 : (ch3+30 - 1)
		if (nx != _nx)
			initialize(pParams, _nx, _upSampleFactor, src.size(1));

		// 1. Crop ROI
		int offset = pParams.act_ch * src.size(0) / 4 + pParams.ch_start_ind[0] + PreTrig;
		ippiConvert_16u32f_C1R(&src(offset, 0), sizeof(uint16_t) * src.size(0),
			crop_src.raw_ptr(), sizeof(float) * crop_src.size(0), srcSize);

		// 2. Determine whether saturated
		ippsThreshold_32f(crop_src.raw_ptr(), sat_src.raw_ptr(), sat_src.length(), 65531, ippCmpLess);
		ippsSubC_32f_I(65531, sat_src.raw_ptr(), sat_src.length());
		int roi_len = (int)round(pulse_roi_length / ActualFactor);
		for (int i = 1; i < 4; i++)
		{
			for (int j = 0; j < ny; j++)
			{
				saturated(j, i) = 0;
				offset = pParams.ch_start_ind[i] - pParams.ch_start_ind[0];
				ippsSum_32f(&sat_src(offset, j), roi_len, &saturated(j, i), ippAlgHintFast);
			}
		}

		// 3. BG subtraction
		ippsSubC_32f_I(pParams.bg, crop_src.raw_ptr(), crop_src.length());

		// 4. Remove artifact manually (smart artifact removal method)
		int ch_ind4[5]; memcpy(ch_ind4, pParams.ch_start_ind, sizeof(int) * 4);
		ch_ind4[4] = ch_ind4[3] + FLIM_CH_START_5;
		int dc_determine_len = 5;

		memcpy(mask_src, crop_src, sizeof(float) * crop_src.length());
		tbb::parallel_for(tbb::blocked_range<size_t>(0, (size_t)ny),
			[&](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i != r.end(); ++i)
			{
				int end_ind4[4]; memcpy(end_ind4, ch_ind4 + 1, sizeof(int) * 4);
				ippsSubC_32s_ISfs(ch_ind4[0], end_ind4, 4, 0);
				
				for (int ch = 0; ch < 4; ch++)
				{
					if (start_ind[ch])
					{
						if (ch == 0)
							ippsMul_32f_I(pMask, &mask_src(0, (int)i), end_ind[ch]);
						else
						{
							float max_val; int max_ind;
							ippsMaxIndx_32f(&mask_src(start_ind[ch], (int)i), end_ind[ch] - start_ind[ch] + 1, &max_val, &max_ind);
							max_ind += start_ind[ch] - 1;
							end_ind4[ch - 1] = max_ind - 4;
							ippsSet_32f(0.0f, &mask_src(max_ind - 3, (int)i), 8);
						}
					}					
				}

				for (int ch = 1; ch < 4; ch++)
				{
					float dc_level;
					ippsMean_32f(&mask_src(end_ind4[ch] - dc_determine_len, (int)i), dc_determine_len, &dc_level, ippAlgHintFast);
					ippsSubC_32f_I(dc_level, &mask_src(ch_ind4[ch] - ch_ind4[0], (int)i), ch_ind4[ch + 1] - ch_ind4[ch] + 1);
				}

				// 5. Up-sampling by cubic natural spline interpolation
				DFTaskPtr task1 = nullptr;

				dfsNewTask1D(&task1, nx, x, DF_UNIFORM_PARTITION, 1, &mask_src(0, (int)i), DF_MATRIX_STORAGE_ROWS);
				dfsEditPPSpline1D(task1, DF_PP_CUBIC, DF_PP_NATURAL, DF_BC_NOT_A_KNOT, 0, DF_NO_IC, 0, scoeff + (int)i * (nx - 1) * DF_PP_CUBIC, DF_NO_HINT);
				dfsConstruct1D(task1, DF_PP_SPLINE, DF_METHOD_STD);
				dfsInterpolate1D(task1, DF_INTERP, DF_METHOD_PP, nsite, x, DF_UNIFORM_PARTITION, 1, &dorder,
					DF_NO_APRIORI_INFO, &ext_src(0, (int)i), DF_MATRIX_STORAGE_ROWS, NULL);
				dfDeleteTask(&task1);
				
				// 6. Software broadening by FIR Gaussian filtering
				_filter(&filt_src(0, (int)i), &ext_src(0, (int)i), (int)i);
			}
		});
	}

	void initialize(const FLIM_PARAMS& pParams, int _nx, int _upSampleFactor, int _alines)
	{
		/* Parameters */
		nx = _nx; ny = _alines;
		upSampleFactor = _upSampleFactor;
		nsite = nx * upSampleFactor;
		ActualFactor = (float)(nx * upSampleFactor - 1) / (float)(nx - 1);
		x[0] = 0.0f; x[1] = (float)nx - 1.0f;
		srcSize = { (int)nx, (int)ny };
		dorder = 1;

		/* Find pulse roi length for mean delay calculation */
		for (int i = 0; i < 4; i++)
			ch_start_ind1[i] = (int)round((float)pParams.ch_start_ind[i] * ActualFactor);

		int diff_ind[4];
		for (int i = 0; i < 3; i++)
			diff_ind[i] = ch_start_ind1[i + 1] - ch_start_ind1[i];
		diff_ind[3] = (int)round((float)FLIM_CH_START_5 * ActualFactor);

		ippsMin_32s(diff_ind, 4, &pulse_roi_length);

		/* sequence for mean delay caculation */
		if (pSeq) { ippFree(pSeq); pSeq = nullptr; }
		pSeq = ippsMalloc_32f(pulse_roi_length);
		ippsVectorSlope_32f(pSeq, pulse_roi_length, 0, 1);

		/* mask for removal of rotary junction artifacts */
		if (pMask) { ippFree(pMask); pMask = nullptr; }
		pMask = ippsMalloc_32f(nx);
		ippsSet_32f(1.0f, pMask, nx);

		printf("FLIM initializing... %d\n", pulse_roi_length);

		/* Array of spline coefficients */
		if (scoeff) { delete[] scoeff; scoeff = nullptr; }
		scoeff = new float[ny * (nx - 1) * DF_PP_CUBIC];

		/* data buffer allocation */
		crop_src = np::Array<float, 2>((int)nx, (int)ny);
		mask_src = np::Array<float, 2>((int)nx, (int)ny);
		sat_src = np::Array<float, 2>((int)nx, (int)ny);
		ext_src = np::Array<float, 2>((int)nsite, (int)ny);
		filt_src = np::Array<float, 2>((int)nsite, (int)ny);

		saturated = np::Array<float, 2>((int)ny, 4);
		memset(saturated, 0, sizeof(float) * saturated.length());

		/* filter coefficient allocation */
		_filter.initialize(GAUSSIAN_FILTER_WIDTH, nsite, ny);
	}

private:
	IppiSize srcSize;
	float* scoeff;
	float x[2];
	MKL_INT dorder;

public:
	MKL_INT nx, ny; // original data length, dimension
	MKL_INT nsite; // interpolated data length

	int ch_start_ind1[4];
	int upSampleFactor;
	Ipp32f ActualFactor;
	int pulse_roi_length;

	FILTER _filter;

	np::Array<float, 2> saturated;

	Ipp32f* pSeq;
	Ipp32f* pMask;
	int start_ind[4];
	int end_ind[4];

	np::Array<float, 2> crop_src;
	np::Array<float, 2> mask_src;
	np::Array<float, 2> sat_src;
	np::Array<float, 2> ext_src;
	np::Array<float, 2> filt_src;
};

struct INTENSITY
{
public:
	INTENSITY() {}
	~INTENSITY() {}

	void operator() (const RESIZE& resize, const FLIM_PARAMS& pParams, np::Array<float, 2>& intensity) // 2600 x 256 ==> 256 x 4
	{
		int offset;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < intensity.size(0); j++)
			{
				intensity(j, i) = 0;
				if (resize.saturated(j, i) < 4)
				{
					offset = resize.ch_start_ind1[i] - resize.ch_start_ind1[0];
					ippsSum_32f(&resize.ext_src(offset, j), resize.pulse_roi_length, &intensity(j, i), ippAlgHintFast);
				}
			}
		}

		for (int i = 0; i < 4; i++)
			ippsDiv_32f_I(&intensity(0, 0), &intensity(0, 3 - i), intensity.size(0));
	}
};

struct LIFETIME
{
public:
	LIFETIME() {};
	~LIFETIME() {};

	void operator() (const RESIZE& resize, const FLIM_PARAMS& pParams, np::Array<float, 2>& intensity,
		np::Array<float, 2>& mean_delay, np::Array<float, 2>& lifetime)
	{
		tbb::parallel_for(tbb::blocked_range<size_t>(0, (size_t)resize.ny),
			[&](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i != r.end(); ++i)
			{
				for (int j = 0; j < 4; j++)
				{
					int offset, width, left, maxIdx;
					offset = resize.ch_start_ind1[j] - resize.ch_start_ind1[0];

					// 1. Get IRF width
					const float* pulse = &resize.filt_src(offset, (int)i);
					//pParams.is_broadening ? &resize.filt_src(offset, i) : &resize.filt_src(offset, i);
					WidthIndex_32f(pulse, 0.5f, resize.pulse_roi_length, maxIdx, width);
					roiWidth[j] = (int)round(pParams.width_factor * width);
					left = (int)floor(roiWidth[j] / 2);

					// 2. Get mean delay of each channel (iterative process)
					MeanDelay_32f(resize, offset, (int)i, maxIdx, roiWidth[j], left, mean_delay((int)i, j));
					mean_delay((int)i, j) = (mean_delay((int)i, j) + (float)resize.ch_start_ind1[j]) / resize.ActualFactor;
				}

				// 3. Subtract mean delay of IRF to mean delay of each channel
				for (int j = 0; j < 3; j++)
				{
					if (intensity((int)i, j + 1) > INTENSITY_THRES)
						lifetime((int)i, j) = pParams.samp_intv * (mean_delay((int)i, j + 1) - mean_delay((int)i, 0)) - pParams.delay_offset[j];
					else
						lifetime((int)i, j) = 0.0f;
				}
			}
		});
	}

	void WidthIndex_32f(const Ipp32f* src, Ipp32f th, Ipp32s length, Ipp32s& maxIdx, Ipp32s& width)
	{
		Ipp32s left0 = 0, right0 = 0;
		Ipp32f maxVal;

		ippsMaxIndx_32f(src, length, &maxVal, &maxIdx);

		for (Ipp32s i = maxIdx; i >= 0; i--)
		{
			if (src[i] < maxVal * th)
			{
				left0 = i;
				break;
			}
		}
		for (Ipp32s i = maxIdx; i < length; i++)
		{
			if (src[i] < maxVal * th)
			{
				right0 = i;
				break;
			}
		}
		width = right0 - left0 + 1;
	}

	void MeanDelay_32f(const RESIZE& resize, int offset, int aline, Ipp32s maxIdx, Ipp32s width, Ipp32s left, Ipp32f &mean_delay)
	{
		Ipp32f sum, weight_sum;
		int start;

		mean_delay = (float)maxIdx;

		for (int i = 0; i < 10; i++)
		{
			start = (int)round(mean_delay) - left;

			if ((start < 0) || (start + width > resize.pulse_roi_length))
			{
				mean_delay = 0;
				break;
			}

			const float* pulse = &resize.filt_src(offset + start, aline);
			sum = 0; weight_sum = 0;

			if (resize.pSeq)
			{
				ippsDotProd_32f(pulse, &resize.pSeq[start], width, &weight_sum);
				ippsSum_32f(pulse, width, &sum, ippAlgHintFast);
			}

			if (sum)
				mean_delay = weight_sum / sum;
			else
			{
				mean_delay = 0;
				break;
			}

			if ((mean_delay > resize.pulse_roi_length) || (mean_delay < 0))
			{
				mean_delay = 0;
				break;
			}
		}
	}

public:
	int roiWidth[4];
};


class FLIMProcess
{
public:
	FLIMProcess();
	~FLIMProcess();

	// get fluorescence intensity & lifetime
	void operator() (np::Array<float, 2>& intensity, np::Array<float, 2>& mean_delay, np::Array<float, 2>& lifetime,
		FLIM_PARAMS& pParams, int PreTrig, np::Array<uint16_t, 2>& pulse);

	// save & load mask data
	void SaveMaskData(CString path = _T("flim_mask.dat"));
	void LoadMaskData(CString path = _T("flim_mask.dat"));

public:
	RESIZE _resize;
	INTENSITY _intensity;
	LIFETIME _lifetime;

public:
	// thread operation	
	signal<int> DidAcquireData; // signal	
	signal<void> DidStopData;

private:
	void run(); // run FLIM Process (thread)	
	std::thread _thread; // thread object

public:
	bool _running;
	bool startFLIMprocess();
	void stopFLIMprocess();

private:
	void dumpErrorSystem(int res, LPCSTR pPreamble);

};

#endif
