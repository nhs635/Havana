#define _CRT_SECURE_NO_WARNING

#include "stdafx.h"
#include "DataRecording.h"

#include <HavanaDlg.h>
#include <DataRecording\FileWriter\FileWriter.h>

#include <iostream>
#include <queue>

#include <QtCore\QFile>
#include <QtCore\QString>

#include <resource.h>

using namespace std;

DataRecording::DataRecording(CHavanaDlg* pMainDlg)
	: pMainDlg(pMainDlg), pWriter(nullptr), isRecording(false), nRecordedFrames(0)
{
}


DataRecording::~DataRecording()
{
	if (pWriter)
		delete pWriter;
}


void DataRecording::StartRecording()
{
	// Create FileWriter object
	do
	{
		if (pWriter)
		{
			if (!(pWriter->is_saved))
			{
				int select = AfxMessageBox(_T("The recorded data is not saved yet. Do you want to proceed?"), MB_YESNO | MB_ICONQUESTION);
				if (select == IDNO)
				{
					pMainDlg->m_ToggleRecording.SetCheck(0);
					break;
				}
			}
			delete pWriter; pWriter = nullptr;
		}

		pMainDlg->UpdateData(TRUE);

		pWriter = new FileWriter;
		pWriter->nChannels = (!(pMainDlg->m_Combo_ActiveChannel)) ? 1 : 2;
		pWriter->nScans = pMainDlg->m_Edit_nScans;
		pWriter->nAlines = pMainDlg->m_Edit_nAlines;

		// Lambda function for data saving
		pWriter->recording_lambda = [&](std::queue<UINT16*> &queue, int Nbuffer) {

			int i;
			size_t res; bool halted = false;

			DWORD dwSamplesToWrite = pWriter->nChannels * pWriter->nScans * pWriter->nAlines / 4;// *sizeof(uint16_t);
			pWriter->_running = true;

			// Saving /////////////////////////////////////////////////////////////
			UINT16* buffer = nullptr;		
			for (i = 0; i < NUM_BUFFER_FRAME - nRecordedFrames; i++)
			{
				buffer = queue.front();
				queue.pop();
				queue.push(buffer);				
			}

			printf("Starting Buffer Address: 0x%x\n", pMainDlg->m_awBufferQueue.front());

			/*FILE* pFile = nullptr;
			res = fopen_s(&pFile, "F:\\temp.data", "wb");

			for (i = 0; i < nRecordedFrames; i++)
			{
				buffer = queue.front();
				queue.pop();
				
				for (int j = 0; j < 8; j++)
				{
					res = fwrite(buffer + j * dwSamplesToWrite, sizeof(UINT16), dwSamplesToWrite, pFile);
					if (!(res == dwSamplesToWrite)) { halted = true; break; printf("error occurred while writing...\n"); }
				}

				if (!(i % 10)) printf("\r%dth frame is wrote... [%3.2f%%]", i + 1, 100 * (double)(i + 1) / (double)nRecordedFrames);

				queue.push(buffer);

				if (!(pWriter->_running))
				{
					printf("\n");
					return;
				}
			}

			printf("\n");
			fclose(pFile);
			pFile = nullptr;*/

			QString path((CStringA)pWriter->FilePath);
			QFile file(path);
			if (file.open(QIODevice::WriteOnly))
			{			
				for (int i = 0; i < nRecordedFrames; i++)
				{
					buffer = queue.front();
					queue.pop();

					for (int j = 0; j < 4; j++)
					{
						res = file.write((char *)(buffer + j * dwSamplesToWrite), sizeof(uint16_t) * dwSamplesToWrite);
						if (!(res == sizeof(uint16_t) * dwSamplesToWrite)) { halted = true; break; printf("error occurred while writing...\n"); }
					}

					printf("\r%dth frame is wrote... [%3.2f%%]", i + 1, 100 * (double)(i + 1) / (double)nRecordedFrames);

					queue.push(buffer);

					if (!(pWriter->_running))
					{
						printf("\n");
						return;
					}					
				}

				file.close();
			}
			else
			{
				printf("Error occurred during writing process.\n");
				return;
			}

			// Move File //////////////////////////////////////////////////////////
			//MoveFile(_T("E:\\temp.data"), pWriter->FilePath);

			// Status Update //////////////////////////////////////////////////////
			pWriter->is_saved = true;
			pMainDlg->m_ToggleSaving.SetCheck(0);
			pMainDlg->SetDlgItemTextW(IDC_TOGGLE_SAVING, _T("Save Recorded Data"));
			pMainDlg->GetDlgItem(IDC_TOGGLE_SAVING)->EnableWindow(FALSE);
			if (pMainDlg->m_ToggleAcquisition.GetCheck())
				pMainDlg->GetDlgItem(IDC_TOGGLE_RECORDING)->EnableWindow(TRUE);

			CString str; str.Format(_T("Current Buffer Address: 0x%x"), pMainDlg->m_awBufferQueue.front());
			pMainDlg->SetDlgItemText(IDC_STATIC_BUFFER_ADDRESS, str);

			printf("Data saving thread is finished normally. (Saved frames: %d frames)\n", nRecordedFrames);
			wprintf(_T("[%s]\n"), pWriter->FilePath.GetBuffer(0));
		};

		// Lambda for status notification
		//pWriter->SendStatusMessage += [this](CString msg) { wprintf(_T("%s\n"), (LPCWSTR)msg); }; // this->pMainDlg->m_StatusBar.SetText(msg, 0, 0);};

		// Start recording
		printf("Data recording is started.\n");
		if (pMainDlg->m_CheckZaberStage)
			pMainDlg->OnBnClickedButtonMoveAbsolute();
		nRecordedFrames = 0;
//		nFramesToCopy = 0;
		isRecording = true;
		pMainDlg->m_VisResult.m_IsSaved = false;

		// Thread for copying transfered data (memcpy)
		std::thread	thread_copy_data = std::thread([&]() {

			printf("Data copying thread is started.\n");

			int frame_size = pWriter->nChannels * pWriter->nScans * pWriter->nAlines;
			//int div_N = 1; // 4 * N_ALINES / 1024;
			//int frame_size1 = frame_size / div_N;

			while (1) //isRecording || (nFramesToCopy != 0))
			{
				const uint16_t* frame = pMainDlg->m_Queue_Copy.pop();
				if (frame)
				{
					uint16_t* buffer = pMainDlg->m_awBufferQueue.front();
					pMainDlg->m_awBufferQueue.pop();
					memcpy(buffer, frame, sizeof(uint16_t) * frame_size);
					//for (int i = 0; i < div_N; i++)
					//	memcpy(buffer + i * frame_size1, frame + i * frame_size1, sizeof(uint16_t) * frame_size1);
					pMainDlg->m_awBufferQueue.push(buffer);
				}
				else
					break;
	
//				printf("Writing... %d for %d\r", nFramesToCopy, nRecordedFrames);
//				nFramesToCopy--;
			}

			CString str; str.Format(_T("Current Buffer Address: 0x%x"), pMainDlg->m_awBufferQueue.front());
			pMainDlg->SetDlgItemText(IDC_STATIC_BUFFER_ADDRESS, str);

			printf("Data copying thread is finished.\n");
		});

		thread_copy_data.detach();
		
		// UI update
		pMainDlg->SetDlgItemTextW(IDC_TOGGLE_RECORDING, _T("Stop Recording"));
		pMainDlg->GetDlgItem(IDC_TOGGLE_ACQUISITION)->EnableWindow(FALSE);
		pMainDlg->GetDlgItem(IDC_TOGGLE_SAVING)->EnableWindow(FALSE);

	} while (0);
}


void DataRecording::StopRecording()
{
	// Stop recording
	isRecording = false;

	// Push nullptr to Copy Queue
	pMainDlg->m_Queue_Copy.push(nullptr);

	// UI update
	pMainDlg->m_ToggleRecording.SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_TOGGLE_RECORDING, _T("Start Recording"));
	pMainDlg->GetDlgItem(IDC_TOGGLE_ACQUISITION)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_TOGGLE_SAVING)->EnableWindow(TRUE);
	pMainDlg->m_VisResult.GetDlgItem(IDC_RADIO_IN_BUFFER)->EnableWindow(TRUE);
	pMainDlg->m_VisResult.GetDlgItem(IDC_BUTTON_IMAGE_PROCESSING)->EnableWindow(TRUE);
	//pMainDlg->SetDlgItemInt(IDC_STATIC_REC_FRAMES, nRecordedFrames);

	// Status update
	QWORD total_size = ((QWORD)nRecordedFrames * (QWORD)(pWriter->nChannels * pWriter->nScans * pWriter->nAlines * sizeof(uint16_t)) / 1024);
	printf("\nData recording is finished normally. \n(Recorded frames: %d frames (%1.3f GB)\n", nRecordedFrames, (double)total_size / 1024.0 / 1024.0);
}


void DataRecording::StartSaving()
{
	// Create dialog to set directory for saving
	CString str = _T("OCT FLIM interleaved raw data (*.data) | *.data; | All Files (*.*) | *.*||");
	CString fullpath, path, name, title;

	CFileDialog dlg(FALSE, _T("data"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, str);
	if (IDOK == dlg.DoModal())
	{
		fullpath = dlg.GetPathName();
		name = dlg.GetFileName();
		title = dlg.GetFileTitle();

		path = fullpath.Left(fullpath.GetLength() - name.GetLength());
		pMainDlg->m_VisResult.m_path = path;
		pMainDlg->m_VisResult.m_IsSaved = true;

		pWriter->FilePath = fullpath;

		// Create file saving thread 
		//nRecordedFrames = NUM_BUFFER_FRAME;
		pWriter->write(std::ref(pMainDlg->m_awBufferQueue), nRecordedFrames); // pWriter->FilePath);

		// Calibration files
		CopyFile(_T("calibration.dat"), path + title + (CString)_T(".calibration"), FALSE);
		CopyFile(_T("bg.bin"), path + title + (CString)_T(".background"), FALSE);
		CopyFile(_T("d1.bin"), path + (CString)_T("\\d1.bin"), FALSE);
		CopyFile(_T("d2.bin"), path + (CString)_T("\\d2.bin"), FALSE);
		CopyFile(_T("Lumen_IP_octflim.m"), path + (CString)_T("\\Lumen_IP_octflim.m"), FALSE);

		pMainDlg->SetIniFile();
		CopyFile(_T("Havana.ini"), path + title + (CString)_T(".ini"), FALSE);
		CopyFile(_T("flim_mask.dat"), path + title + (CString)_T(".flim_mask"), FALSE);

		// Status update
		pMainDlg->m_ProgressBar.SetRange(0, nRecordedFrames);
		pMainDlg->m_ProgressBar.SetPos(0);
		pMainDlg->SetDlgItemTextW(IDC_TOGGLE_SAVING, _T("Saving... (Click to halt)"));
		//pMainDlg->GetDlgItem(IDC_TOGGLE_RECORDING)->EnableWindow(FALSE);
		pMainDlg->GetDlgItem(IDC_TOGGLE_SAVING)->EnableWindow(TRUE);
	}
	else
		pMainDlg->m_ToggleSaving.SetCheck(0);
}


void DataRecording::StopSaving()
{
	// This fuction is only executed by user's interrupt.
	pWriter->stop();

	pMainDlg->m_ToggleSaving.SetCheck(0);
	pMainDlg->SetDlgItemTextW(IDC_TOGGLE_SAVING, _T("Save Recorded Data"));
	pMainDlg->GetDlgItem(IDC_TOGGLE_RECORDING)->EnableWindow(TRUE);
	pMainDlg->GetDlgItem(IDC_TOGGLE_SAVING)->EnableWindow(TRUE);
	//pMainDlg->SetDlgItemInt(IDC_STATIC_REC_FRAMES, nRecordedFrames);
}

