#include "stdafx.h"
#include "DataAcquisition.h"

#include "HavanaDlg.h"
#include "Tab View\VisStream.h"
#include "ImageProcess\OCTProcess\OCTProcess.h"
#include "ImageProcess\FLIMProcess\FLIMProcess.h"

#include "DataAcquisition\SignatecDAQ\SignatecDAQ.h"

#if NI_ENABLE
#include "NI DAQ\GainControl.h"
#include "NI DAQ\SyncFLIM.h"
#include "NI DAQ\GalvoScan.h"
#endif

#include "ZaberStage\ZaberStage.h"
#include "RS232\FaulhaberMotor\FaulhaberMotor.h"
#include "RS232\ElforlightLaser\ElforlightLaser.h"

#include <resource.h>

using namespace std;

DataAcquisition::DataAcquisition(CHavanaDlg* pMainDlg) : pMainDlg(pMainDlg)
{
	pStream = &(pMainDlg->m_VisStream);
	pOCT = pMainDlg->m_pOCT;
	pFLIM = pMainDlg->m_pFLIM;

	pDAQ = new SignatecDAQ;
#if NI_ENABLE
	pGainControl = new GainControl;
	pSyncFLIM = new SyncFLIM;
	pGalvoScan = new GalvoScan;
#endif
	pZaberStage = new ZaberStage;
	pFaulhaberMotor = new FaulhaberMotor;
	pElforlightLaser = new ElforlightLaser;
}


DataAcquisition::~DataAcquisition()
{
	if (pDAQ) delete pDAQ;
#if NI_ENABLE
	if (pGainControl) delete pGainControl;
	if (pSyncFLIM) delete pSyncFLIM;
	if (pGalvoScan) delete pGalvoScan;
#endif
	if (pZaberStage) delete pZaberStage;
	if (pFaulhaberMotor) delete pFaulhaberMotor;
	if (pElforlightLaser) delete pElforlightLaser;
}


void DataAcquisition::SetLambdas()
{
	// Callback function definition by Lambda Function for acquisition update ///////////////////////////////////////////////////////////////////////
	pDAQ->DidAcquireData += [&](int frame_count, const np::Array<uint16_t, 2> &frame)
	{	
		// Data Halving
		int frame_length = pMainDlg->m_Edit_nScans * pMainDlg->m_Edit_nAlines;
		//int step = (pMainDlg->m_Combo_ActiveChannel + 1) * pMainDlg->m_Edit_nScans;
		//np::Array<uint16_t, 2> frame_odd(step, pMainDlg->m_Edit_nAlines);

		//int n = 4;
		//tbb::parallel_for(tbb::blocked_range<size_t>(0, (size_t)n),
		//	[&](const tbb::blocked_range<size_t>& r) {
		//	for (size_t i = r.begin(); i != r.end(); ++i)
		//		ippiCopy_16u_C1R(&frame(0, (int)(2 * i * pMainDlg->m_Edit_nAlines / n)), sizeof(uint16_t) * 2 * step, 
		//			&frame_odd(0, (int)(i * pMainDlg->m_Edit_nAlines / n)), sizeof(uint16_t) * step, { step, pMainDlg->m_Edit_nAlines / n });
		//});

		// Data transfer		
		int refresh_count = FRAME_INTERVAL / (N_ALINES / 1024);
		if (!(frame_count % refresh_count))
		{
			const uint16_t* frame_ptr = frame.raw_ptr();

			uint16_t* fringe_ptr = nullptr; // pMainDlg->m_FrameFringe.raw_ptr(); // nullptr;
			uint16_t* pulse_ptr = nullptr; // pMainDlg->m_FrameFLIM.raw_ptr();
			{
				std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_OCT_proc);

				if (!pMainDlg->m_queue_fringe.empty())
				{
					fringe_ptr = pMainDlg->m_queue_fringe.front();
					pMainDlg->m_queue_fringe.pop();
				}
			}
			{
				std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_FLIM_proc);

				if (!pMainDlg->m_queue_pulse.empty())
				{
					pulse_ptr = pMainDlg->m_queue_pulse.front();
					pMainDlg->m_queue_pulse.pop();
				}
			}

			if ((fringe_ptr != nullptr) && (pulse_ptr != nullptr))
			{
				// Data deinterleaving
				ippsCplxToReal_16sc((Ipp16sc *)frame_ptr, (Ipp16s *)fringe_ptr, (Ipp16s *)pulse_ptr, frame_length);
				memcpy(pMainDlg->m_FrameFringe, fringe_ptr, sizeof(uint16_t) * frame_length); // for visualization
				memcpy(pMainDlg->m_FramePulse, pulse_ptr, sizeof(uint16_t) * frame_length); // for visualization

				// 작업 동기화 Queue에 넣기 (push)
				pMainDlg->m_Queue_OCT_proc.push(fringe_ptr);
				pMainDlg->m_Queue_FLIM_proc.push(pulse_ptr);
			}
		}

		// Buffering (When recording)
		if (pMainDlg->m_pDataRecording->isRecording)
		{
			// Push to the copy queue for copying transfered data in copy thread
			pMainDlg->m_Queue_Copy.push(frame.raw_ptr());
			pMainDlg->m_pDataRecording->nRecordedFrames++;
			//pMainDlg->m_pDataRecording->nFramesToCopy++;

			// Finish recording when the buffer is full
			if (pMainDlg->m_pDataRecording->nRecordedFrames == NUM_BUFFER_FRAME)
				pMainDlg->m_pDataRecording->StopRecording();
		}
	};

	// Callback function definition by Lambda Function for thread stop ////////////////////////////////////////////////////////////////////////////////////
	pDAQ->DidStopData += [&]()
	{
		pDAQ->_running = false;
		pMainDlg->m_Queue_OCT_proc.push(nullptr);
		pMainDlg->m_Queue_FLIM_proc.push(nullptr);
	};

	// Callback function definition by Lambda Function for OCT image process update ///////////////////////////////////////////////////////////////////////
	pOCT->DidAcquireData += [&](int frame_count) //, const np::Array<uint16_t, 2> &frame)
	{
		// Transfer thread에서 데이터 받기
		uint16_t* fringe_data = pMainDlg->m_Queue_OCT_proc.pop();
		if (fringe_data != nullptr)
		{
			// OCT image queue에서 결과 저장할 데이터 받기
			float* image_data = nullptr;
			{
				std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_OCT_vis);
				if (!pMainDlg->m_queue_image.empty())
				{
					image_data = pMainDlg->m_queue_image.front();
					pMainDlg->m_queue_image.pop();

				}
			}

			if (image_data)
			{
				// Body
				(*pOCT)(image_data, fringe_data);
				memcpy(pMainDlg->m_FrameOCT.raw_ptr(), image_data, sizeof(float) * pMainDlg->m_FrameOCT.length());

				// Visualization thread에 데이터 보내기
				pMainDlg->m_Queue_OCT_vis.push(image_data);

				// OCT fringe queue에 데이터 돌려주기
				{
					std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_OCT_proc);
					pMainDlg->m_queue_fringe.push(fringe_data);
				}
			}
		}
		else
			pOCT->_running = false;
	};

	// Callback function definition by Lambda Function for thread stop ////////////////////////////////////////////////////////////////////////////////
	pOCT->DidStopData += [&]()
	{
		pMainDlg->m_Queue_OCT_vis.push(nullptr);
	};

	// Callback function definition by Lambda Function for OCT image process update ///////////////////////////////////////////////////////////////////////
	pFLIM->DidAcquireData += [&](int frame_count) //, const np::Array<uint16_t, 2> &frame)
	{
		// Transfer thread에서 데이터 받기
		uint16_t* pulse_data = pMainDlg->m_Queue_FLIM_proc.pop();
		if (pulse_data != nullptr)
		{
			// FLIM result queue에서 결과 저장할 데이터 받기
			float* flim_data = nullptr;
			{
				std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_FLIM_vis);
				if (!pMainDlg->m_queue_flim.empty())
				{
					flim_data = pMainDlg->m_queue_flim.front();
					pMainDlg->m_queue_flim.pop();
				}
			}

			if (flim_data)
			{
				// Body (FLIM Process)
				if (pMainDlg->m_CheckFLIMSync)
					(*pFLIM)(pMainDlg->m_FluIntensity, pMainDlg->m_FluMeanDelay, pMainDlg->m_FluLifetime,
						pStream->m_Params_FLIM, pMainDlg->m_Edit_PreTrig, pMainDlg->m_FramePulse);

				// Visualization thread에 데이터 보내기
				pMainDlg->m_Queue_FLIM_vis.push(flim_data);

				// OCT fringe queue에 데이터 돌려주기
				{
					std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_FLIM_proc);
					pMainDlg->m_queue_pulse.push(pulse_data);
				}
			}
		}
		else
			pFLIM->_running = false;
	};

	// Callback function definition by Lambda Function for thread stop ////////////////////////////////////////////////////////////////////////////////
	pFLIM->DidStopData += [&]()
	{
		pMainDlg->m_Queue_FLIM_vis.push(nullptr);
	};

	// Callback function definition by Lambda Function for visualization update ///////////////////////////////////////////////////////////////////////
	pStream->DidAcquireData += [&](int frame_count) //, const np::Array<uint16_t, 2> &frame)
	{
		{
			std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_Invalidate);

			// FLIM process thread와 OCT process thread에서 데이터 받기
			float* image_data = pMainDlg->m_Queue_OCT_vis.pop();
			float* flim_data = pMainDlg->m_Queue_FLIM_vis.pop();
			if ((image_data != nullptr)  && (flim_data != nullptr))
			{
				if (pStream->_running)
				{
					// Body				
					pStream->InvalidateRect(pStream->m_VisRegionRectWnd, FALSE);

					if (pMainDlg->m_CheckFLIMSync)
					{
						// FLIM Parameters Update (Normalized Intensity)	
						CString str;
						str.Format(_T("%.3f"), pMainDlg->m_FluIntensity(pStream->m_SliderCurAline / 4, 1));
						pStream->SetDlgItemTextW(IDC_EDIT_INTENSITY_CH1, str);
						str.Format(_T("%.3f"), pMainDlg->m_FluIntensity(pStream->m_SliderCurAline / 4, 2));
						pStream->SetDlgItemTextW(IDC_EDIT_INTENSITY_CH2, str);
						str.Format(_T("%.3f"), pMainDlg->m_FluIntensity(pStream->m_SliderCurAline / 4, 3));
						pStream->SetDlgItemTextW(IDC_EDIT_INTENSITY_CH3, str);

#if defined(_DEBUG)
						// FLIM Parameters Update (Mean Delay)
						str.Format(_T("%.3f"), pStream->m_Params_FLIM.samp_intv * pMainDlg->m_FluMeanDelay(pStream->m_SliderCurAline / 4, 0));
						pStream->SetDlgItemTextW(IDC_EDIT_MEAN_DELAY_CH0, str);
						str.Format(_T("%.3f"), pStream->m_Params_FLIM.samp_intv * pMainDlg->m_FluMeanDelay(pStream->m_SliderCurAline / 4, 1));
						pStream->SetDlgItemTextW(IDC_EDIT_MEAN_DELAY_CH1, str);
						str.Format(_T("%.3f"), pStream->m_Params_FLIM.samp_intv * pMainDlg->m_FluMeanDelay(pStream->m_SliderCurAline / 4, 2));
						pStream->SetDlgItemTextW(IDC_EDIT_MEAN_DELAY_CH2, str);
						str.Format(_T("%.3f"), pStream->m_Params_FLIM.samp_intv * pMainDlg->m_FluMeanDelay(pStream->m_SliderCurAline / 4, 3));
						pStream->SetDlgItemTextW(IDC_EDIT_MEAN_DELAY_CH3, str);
#endif

						// FLIM Parameters Update (Lifetime)
						str.Format(_T("%.3f"), pMainDlg->m_FluLifetime(pStream->m_SliderCurAline / 4, 0));
						pStream->SetDlgItemTextW(IDC_EDIT_LIFETIME_CH1, str);
						str.Format(_T("%.3f"), pMainDlg->m_FluLifetime(pStream->m_SliderCurAline / 4, 1));
						pStream->SetDlgItemTextW(IDC_EDIT_LIFETIME_CH2, str);
						str.Format(_T("%.3f"), pMainDlg->m_FluLifetime(pStream->m_SliderCurAline / 4, 2));
						pStream->SetDlgItemTextW(IDC_EDIT_LIFETIME_CH3, str);
					}

					// OCT image queue와 FLIM res queue에 데이터 돌려주기 		
					{
						std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_OCT_vis);
						pMainDlg->m_queue_image.push(image_data);
					}
					{
						std::unique_lock<std::mutex> lock(pMainDlg->m_mtx_FLIM_vis);
						pMainDlg->m_queue_flim.push(flim_data);
					}
				}
			}
			else
			{
				if (image_data != nullptr)
				{
					pMainDlg->m_Queue_OCT_vis.pop();
					pMainDlg->m_queue_image.push(image_data);
				}
				else if (flim_data != nullptr)
				{
					pMainDlg->m_Queue_FLIM_vis.pop();
					pMainDlg->m_queue_flim.push(flim_data);
				}

				pStream->_running = false;
			}
		}
	};

	// Callback function definition by Lambda Function for message
	pDAQ->SendStatusMessage += [this](CString msg) { wprintf(_T("%s\n"), (LPCWSTR)msg); };
}


bool DataAcquisition::InitializeAcquisition()
{
	// Parameter settings for DAQ
	pMainDlg->UpdateData(TRUE);
	pDAQ->nChannels = (!(pMainDlg->m_Combo_ActiveChannel)) ? 1 : 2;
	pDAQ->nScans = pMainDlg->GetDlgItemInt(IDC_EDIT_N_SCANS);
	pDAQ->nAlines = pMainDlg->GetDlgItemInt(IDC_EDIT_N_ALINES);
	pDAQ->VoltRange1 = pMainDlg->m_Combo_OCTInputRange + 1;
	pDAQ->VoltRange2 = pMainDlg->m_Combo_FLIMInputRange + 1;
	pDAQ->AcqRate = pMainDlg->m_Edit_AdcRate; // MHz
	pDAQ->PreTrigger = pMainDlg->m_Edit_PreTrig;
	pDAQ->TriggerDelay = pMainDlg->m_Edit_TrigDelay;

	// Initialization for DAQ
	if (!(pDAQ->set_init()))
	{
		StopAcquisition();
		return false;
	}

	return true;
}


bool DataAcquisition::StartAcquisition()
{
	// Parameter settings for DAQ
	pMainDlg->UpdateData(TRUE);
	pDAQ->VoltRange1 = pMainDlg->m_Combo_OCTInputRange + 1;
	pDAQ->VoltRange2 = pMainDlg->m_Combo_FLIMInputRange + 1;
	pDAQ->PreTrigger = pMainDlg->m_Edit_PreTrig;
	pDAQ->TriggerDelay = pMainDlg->m_Edit_TrigDelay;

	// Start acquisition
	if (!(pDAQ->startAcquisition()))
	{
		StopAcquisition();
		return false;
	}

	// Buffer 생성 (For raw data) ////////////////////////////////////////////////////////////////////////////
	if (!pMainDlg->m_bIsBufferAllocated)
	{
		std::thread buffer_creation([&]()
		{
			pMainDlg->GetDlgItem(IDC_TOGGLE_ACQUISITION)->EnableWindow(FALSE);
			int nChannels = (!(pMainDlg->m_Combo_ActiveChannel)) ? 1 : 2;
			int nTotal = nChannels * pMainDlg->m_Edit_nScans * pMainDlg->m_Edit_nAlines;
			for (int i = 0; i < NUM_BUFFER_FRAME; i++)
			{
				uint16_t* buffer = new uint16_t[nTotal];
				memset(buffer, 0, nTotal * sizeof(uint16_t));
				pMainDlg->m_awBufferQueue.push(buffer);
				printf("\rAllocating recording buffers... [%d / %d]", i + 1, NUM_BUFFER_FRAME);
			}
			printf("\nRecording buffers are successfully allocated. [Number of buffers: %d]\n", NUM_BUFFER_FRAME);
			printf("Now, recording process is available!\n");
			pMainDlg->m_bIsBufferAllocated = true;
			pMainDlg->GetDlgItem(IDC_TOGGLE_ACQUISITION)->EnableWindow(TRUE);
			pMainDlg->GetDlgItem(IDC_TOGGLE_RECORDING)->EnableWindow(TRUE);

			CString str; str.Format(_T("Current Buffer Address: 0x%x"), pMainDlg->m_awBufferQueue.front());
			pMainDlg->SetDlgItemText(IDC_STATIC_BUFFER_ADDRESS, str);
		});

		buffer_creation.detach();
	}

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_TOGGLE_ACQUISITION, _T("Stop Acquisition"));
	if (pMainDlg->m_bIsBufferAllocated) pMainDlg->GetDlgItem(IDC_TOGGLE_RECORDING)->EnableWindow(TRUE);
	//pMainDlg->GetDlgItem(IDC_TOGGLE_FLIM_SYNC)->EnableWindow(FALSE);
	//pMainDlg->GetDlgItem(IDC_CHECK_FLIM_SYNC)->EnableWindow(FALSE);
	//pMainDlg->GetDlgItem(IDC_CHECK_GALVO_SCAN)->EnableWindow(FALSE);
	//pMainDlg->GetDlgItem(IDC_TOGGLE_GALVO_SCAN)->EnableWindow(FALSE);
	// GetDlgItem(IDC_COMBO_ACTIVE_CH)->EnableWindow(FALSE);
	// pMainDlg->GetDlgItem(IDC_EDIT_N_ALINES)->EnableWindow(FALSE);
	// pMainDlg->GetDlgItem(IDC_EDIT_N_SCANS)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_PRE_TRIG_SAMPS)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_TRIG_DELAY_SAMPS)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_COMBO_FLIM_INPUT_RANGE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_COMBO_OCT_INPUT_RANGE)->EnableWindow(FALSE);
	//pMainDlg->GetDlgItem(IDC_EDIT_ADC_RATE)->EnableWindow(FALSE);

	// SetCursor(LoadCursor(NULL, IDC_ARROW));

	return true;
}


void DataAcquisition::StopAcquisition()
{
	// stop thread
	pDAQ->stopAcquisition();

#if NI_ENABLE
	// Stop hardware operation
	StopSyncFLIM();
	StopGalvoScan();
	StopGainControl();
#endif

	// Status change	
	pMainDlg->m_ToggleAcquisition.SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_TOGGLE_ACQUISITION, _T("Start Acquisition"));
	pMainDlg->GetDlgItem(IDC_TOGGLE_RECORDING)->EnableWindow(FALSE);
	//pMainDlg->GetDlgItem(IDC_TOGGLE_FLIM_SYNC)->EnableWindow(TRUE);
	//pMainDlg->GetDlgItem(IDC_CHECK_FLIM_SYNC)->EnableWindow(TRUE);
	//pMainDlg->GetDlgItem(IDC_TOGGLE_GALVO_SCAN)->EnableWindow(TRUE);
	//pMainDlg->GetDlgItem(IDC_CHECK_GALVO_SCAN)->EnableWindow(TRUE);
	//GetDlgItem(IDC_COMBO_ACTIVE_CH)->EnableWindow(TRUE);
	//pMainDlg->GetDlgItem(IDC_EDIT_N_ALINES)->EnableWindow(TRUE);
	//pMainDlg->GetDlgItem(IDC_EDIT_N_SCANS)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_PRE_TRIG_SAMPS)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_TRIG_DELAY_SAMPS)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_COMBO_FLIM_INPUT_RANGE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_COMBO_OCT_INPUT_RANGE)->EnableWindow(TRUE);
	// pMainDlg->GetDlgItem(IDC_EDIT_ADC_RATE)->EnableWindow(TRUE);
}


#if NI_ENABLE

bool DataAcquisition::InitializeGainControl()
{
	// For PMT Gain Control Initialization	
	pMainDlg->UpdateData(TRUE);

	pGainControl->voltage = pMainDlg->m_Edit_PmtGain;
	pGainControl->physicalChannel = "Dev1/ao2";

	if (pGainControl->voltage > 1.0)
	{
		AfxMessageBox(_T(">1.0V Gain cannot be assigned!"));
		return false;
	}

	if (!(pGainControl->initialize()))
		return false;

	return true;
}


void DataAcquisition::StartGainControl()
{
	// Start PMT Gain Control
	pGainControl->start();

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_CHECK_PMT_GAIN, _T("Disable PMT Gain Control"));
	pMainDlg->GetDlgItem(IDC_EDIT_PMT_GAIN)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_PMT_GAIN)->EnableWindow(FALSE);
}


void DataAcquisition::StopGainControl()
{
	// Stop PMT Gain Control
	pGainControl->stop();

	// Status change
	pMainDlg->m_CheckPmtGainControl = false; ((CButton *)pMainDlg->GetDlgItem(IDC_CHECK_PMT_GAIN))->SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_CHECK_PMT_GAIN, _T("Enable PMT Gain Control"));
	pMainDlg->GetDlgItem(IDC_EDIT_PMT_GAIN)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_PMT_GAIN)->EnableWindow(TRUE);
}


bool DataAcquisition::InitializeSyncFLIM()
{
	// For FLIM Synchronization Initialization	
	pSyncFLIM->counterChannel = "Dev1/ctr0";
	pSyncFLIM->sourceTerminal = (pMainDlg->m_CheckInternalSync) ? "20MHzTimeBase" : "PFI13";
	pSyncFLIM->slow = (pMainDlg->m_CheckInternalSync) ? 20000 / (pMainDlg->m_Edit_RepetitionRate) : 4;

	if (!(pSyncFLIM->initialize()))
		return false;

	return true;
}


void DataAcquisition::StartSyncFLIM()
{
	// Start FLIM Sync
	pSyncFLIM->start();

	// Find FLIM channel automatically
	std::thread find_flim_ch([&]() {

		printf("\nFinding FLIM channel automatically....\n");
		std::this_thread::sleep_for(1s);

		int i = 0;
		for (i = 0; i < 4; i++)
		{
			uint16_t* pSrc = pMainDlg->m_FramePulse.raw_ptr() + i * pMainDlg->m_Edit_nScans + pMainDlg->m_Edit_PreTrig;

			int len = pMainDlg->m_VisStream.m_nScansFLIM;
			int ind1, ind2;
			Ipp16u max, min;
			ippsMinMaxIndx_16u(pSrc, len, &min, &ind1, &max, &ind2);
				
			if (max > 35000)
			{
				pMainDlg->m_VisStream.m_Params_FLIM.act_ch = i;
				pMainDlg->m_VisStream.SetDlgItemInt(IDC_EDIT_FLIM_CHANNEL, i);
				printf("Active Channel: CH %d\n", i + 1);
				break;
			}
		}

		if (i == 4)
			printf("Fail to find FLIM channel...\n");

	});
	find_flim_ch.detach();

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_CHECK_FLIM_SYNC, _T("Disable FLIM Laser Sync"));
	pMainDlg->GetDlgItem(IDC_CHECK_INTERNAL_SYNC)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_REPETITION)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_REPETITION)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_REPETITION_KHZ)->EnableWindow(FALSE);
}


void DataAcquisition::StopSyncFLIM()
{
	// Stop FLIM Sync
	pSyncFLIM->stop();

	// Status change
	pMainDlg->m_CheckFLIMSync = false; ((CButton *)pMainDlg->GetDlgItem(IDC_CHECK_FLIM_SYNC))->SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_CHECK_FLIM_SYNC, _T("Enable FLIM Laser Sync"));
	pMainDlg->GetDlgItem(IDC_CHECK_INTERNAL_SYNC)->EnableWindow(TRUE);

	pMainDlg->UpdateData(TRUE);
	if (pMainDlg->m_CheckInternalSync)
	{
		pMainDlg->GetDlgItem(IDC_EDIT_REPETITION)->EnableWindow(TRUE);
		pMainDlg->GetDlgItem(IDC_STATIC_REPETITION)->EnableWindow(TRUE);
		pMainDlg->GetDlgItem(IDC_STATIC_REPETITION_KHZ)->EnableWindow(TRUE);
	}
}


bool DataAcquisition::InitializeGalvoScan()
{
	// For Galvo Scanner Initialization
	pMainDlg->UpdateData(TRUE);

	pGalvoScan->nAlines = pMainDlg->m_Edit_nAlines;
	pGalvoScan->pp_voltage = pMainDlg->m_Edit_GalvoVoltage;
	pGalvoScan->offset = pMainDlg->m_Edit_GalvoOffset;
	pGalvoScan->max_rate = 500000.0;
	pGalvoScan->physicalChannel = "Dev1/ao1";
	pGalvoScan->sourceTerminal = "/Dev1/PFI13";

	if (!(pGalvoScan->initialize()))
		return false;

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_CHECK_GALVO_SCAN, _T("Disable Galvo Mirror Scan"));
	pMainDlg->GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_GALVO_PP_VOLTAGE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_GALVO_OFFSET)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO2)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO3)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO4)->EnableWindow(FALSE);

	return true;
}


void DataAcquisition::StartGalvoScan()
{
	// Start Galvo Scan
	pGalvoScan->start();
}


void DataAcquisition::StopGalvoScan()
{
	// Stop Galvo Scan
	pGalvoScan->stop();

	// Status change
	pMainDlg->m_CheckGalvoScan = false; ((CButton *)pMainDlg->GetDlgItem(IDC_CHECK_GALVO_SCAN))->SetCheck(0);
	pMainDlg->GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO)->EnableWindow(FALSE);
	pMainDlg->SetDlgItemTextW(IDC_CHECK_GALVO_SCAN, _T("Enable Galvo Mirror Scan"));
	pMainDlg->GetDlgItem(IDC_SCROLLBAR_ADJUST_GALVO)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_GALVO_PP_VOLTAGE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_GALVO_OFFSET)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO2)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO3)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_GALVO4)->EnableWindow(TRUE);
}

#endif

bool DataAcquisition::EnableZaberStage()
{
	// For Zaber Stage Initialization
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_WAIT));

	// Set stage parameter
	pZaberStage->conversion_factor = BENCHTOP_MODE ? 1.0 / 9.375 : 1.6384;
	pZaberStage->max_micro_resolution = BENCHTOP_MODE ? 128 : 64;
	pZaberStage->micro_resolution = 32;

	// Connect stage
	if (!(pZaberStage->ConnectStage()))
	{
		DisableZaberStage();
		return false;
	}

	// Set target speed first
	pMainDlg->UpdateData(TRUE);
	pZaberStage->SetTargetSpeed(pMainDlg->m_Edit_TargetSpeed);

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_CHECK_ZABER_STAGE, _T("Disable Zaber Stage Control"));
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_HOME)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_STOP)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_MOVE_ABSOLUTE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_SET_TARGET_SPEED)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_ZABER_MOVE_ABSOLUTE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_ZABER_SET_TARGET_SPEED)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_ZABER1)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_ZABER2)->EnableWindow(TRUE);

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	return true;
}


void DataAcquisition::DisableZaberStage()
{
	// Stop Wait Thread
	pZaberStage->StopWaitThread();

	// Disconnect The Stage
	pZaberStage->DisconnectStage();

	// Status change
	pMainDlg->m_CheckZaberStage = false; ((CButton *)pMainDlg->GetDlgItem(IDC_CHECK_ZABER_STAGE))->SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_CHECK_ZABER_STAGE, _T("Enable Zaber Stage Control"));
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_HOME)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_STOP)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_MOVE_ABSOLUTE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ZABER_SET_TARGET_SPEED)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_ZABER_MOVE_ABSOLUTE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_ZABER_SET_TARGET_SPEED)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_ZABER1)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_ZABER2)->EnableWindow(FALSE);

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}


void DataAcquisition::OperateZaberStage(int type)
{
	switch (type)
	{
	case 0:
		pZaberStage->Home();
		break;
	case 1:
		pZaberStage->Stop();
		break;
	case 2:
		pMainDlg->UpdateData(TRUE);
		pZaberStage->MoveAbsoulte(pMainDlg->m_Edit_MoveAbs);
		break;
	case 3:
		pMainDlg->UpdateData(TRUE);
		if (BENCHTOP_MODE)
		{
			if (pMainDlg->m_Edit_TargetSpeed <= 8)
				pZaberStage->SetTargetSpeed(pMainDlg->m_Edit_TargetSpeed);
			else
				printf("ZABER: Invalid target speed.\n");
		}
		else
		{
			if (pMainDlg->m_Edit_TargetSpeed <= 50)
				pZaberStage->SetTargetSpeed(pMainDlg->m_Edit_TargetSpeed);
			else
				printf("ZABER: Invalid target speed.\n");
		}
		break;
	default:
		break;
	}
}


bool DataAcquisition::EnableFaulhaberMotor()
{
	// For Elforlight laser Initialization
	if (!(pFaulhaberMotor->ConnectDevice(pMainDlg->m_hWnd)))
	{
		DisableFaulhaberMotor();
		return false;
	}

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_CHECK_FAULHABER_MOTOR, _T("Disable Faulhaber Motor Control"));
	pMainDlg->GetDlgItem(IDC_TOGGLE_FAULHABER_ROTATE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_FAULHABER_RPM)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_STATIC_FAULHABER_RPM)->EnableWindow(TRUE);

	return true;
}


void DataAcquisition::DisableFaulhaberMotor()
{
	// Disconnect device
	pFaulhaberMotor->DisconnectDevice();

	// Status change
	pMainDlg->m_CheckFaulhaberMotor = false; ((CButton *)pMainDlg->GetDlgItem(IDC_CHECK_FAULHABER_MOTOR))->SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_CHECK_FAULHABER_MOTOR, _T("Enable Faulhaber Motor Control"));
	pMainDlg->GetDlgItem(IDC_TOGGLE_FAULHABER_ROTATE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_FAULHABER_RPM)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_STATIC_FAULHABER_RPM)->EnableWindow(FALSE);
}


void DataAcquisition::OperateFaulhaberMotor(int type)
{
	switch (type)
	{
	case 0: // Rotate the motor by the given RPM value
		pMainDlg->UpdateData(TRUE);
		pFaulhaberMotor->RotateMotorRPM(pMainDlg->m_Edit_RPM);
		pMainDlg->SetDlgItemTextW(IDC_TOGGLE_FAULHABER_ROTATE, _T("Stop Motor"));
		pMainDlg->GetDlgItem(IDC_CHECK_FAULHABER_MOTOR)->EnableWindow(FALSE);
		break;
	case 1: // Stop the motor
		pFaulhaberMotor->StopMotor();
		pMainDlg->SetDlgItemTextW(IDC_TOGGLE_FAULHABER_ROTATE, _T("Rotate Motor"));
		pMainDlg->GetDlgItem(IDC_CHECK_FAULHABER_MOTOR)->EnableWindow(TRUE);
		break;
	}
}


bool DataAcquisition::EnableElforlightLaser()
{
	// For Elforlight laser Initialization
	if (!(pElforlightLaser->ConnectDevice(pMainDlg->m_hWnd)))
	{
		DisableElforlightLaser();
		return false;
	}

	// Status change
	pMainDlg->SetDlgItemTextW(IDC_CHECK_ELFORLIGHT_LASER, _T("Disable Elforlight Laser Control"));
	pMainDlg->GetDlgItem(IDC_BUTTON_ELFORLIGHT_INCREASE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ELFORLIGHT_DECREASE)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_EDIT_ELFORLIGHT_LASER_VALUE)->EnableWindow(TRUE);

	return true;
}


void DataAcquisition::DisableElforlightLaser()
{
	// Disconnect device
	pElforlightLaser->DisconnectDevice();

	// Status change
	pMainDlg->m_CheckElforlightLaser = false; ((CButton *)pMainDlg->GetDlgItem(IDC_CHECK_ELFORLIGHT_LASER))->SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_CHECK_ELFORLIGHT_LASER, _T("Enable Elforlight Laser Control"));
	pMainDlg->GetDlgItem(IDC_BUTTON_ELFORLIGHT_INCREASE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_BUTTON_ELFORLIGHT_DECREASE)->EnableWindow(FALSE);
	pMainDlg->GetDlgItem(IDC_EDIT_ELFORLIGHT_LASER_VALUE)->EnableWindow(FALSE);
}


void DataAcquisition::OperateElforlightLaser(int type)
{
	switch (type)
	{
	case 0: // increase power by a step
		pElforlightLaser->IncreasePower();
		break;
	case 1: // decrease power by a step
		pElforlightLaser->DecreasePower();
		break;
	}
}
