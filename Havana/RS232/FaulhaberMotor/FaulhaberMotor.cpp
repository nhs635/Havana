#include "stdafx.h"
#include "FaulhaberMotor.h"

#include "Configuration.h"

FaulhaberMotor::FaulhaberMotor() :
	port_name(FAULHABER_PORT)
{
}


FaulhaberMotor::~FaulhaberMotor()
{
	if (m_Comm.m_bConnected)
	{
		BYTE* Buff1 = (BYTE*)"v0\n";
		printf("FAULHABER: Send: %s", Buff1);
		m_Comm.WriteComm(Buff1, (DWORD)strnlen_s((const char*)Buff1, 100));

		BYTE* Buff2 = (BYTE*)"di\n";
		printf("FAULHABER: Send: %s", Buff2);
		m_Comm.WriteComm(Buff2, (DWORD)strnlen_s((const char*)Buff2, 100));

		Sleep(100);

		m_Comm.ClosePort();
		printf("FAULHABER: Success to disconnect to %s.\n", port_name);
	}
}


bool FaulhaberMotor::ConnectDevice(HWND hWnd)
{
	m_Comm.hCommWnd = hWnd;

	// 통신 포트를 열기
	if (!m_Comm.m_bConnected)
	{
		CString port; port.Format(_T("\\\\.\\%s"), (CString)port_name);
		if (m_Comm.OpenPort(port, _T("9600"), _T("8"), _T("NO"), _T("NO"), _T("1")))
		{
			printf("FAULHABER: Success to connect to %s.\n", port_name);

			m_Comm.m_Receive = [&]() {
				BYTE aByte;
				m_Comm.m_bReserveMsg = false;

				int size = (m_Comm.m_QueueRead).GetSize();
				if (size == 0) return;

				for (int i = 0; i < size; i++)
				{
					(m_Comm.m_QueueRead).GetByte(&aByte);
					printf("%c", aByte);
				}
			};

			BYTE* Buff = (BYTE*)"en\n";
			printf("FAULHABER: Send: %s", Buff);
			m_Comm.WriteComm(Buff, (DWORD)strnlen_s((const char*)Buff, 100));
		}
		else
		{
			printf("FAULHABER: Fail to connect to %s.\n", port_name);
			return false;
		}
	}

	return true;
}


void FaulhaberMotor::DisconnectDevice()
{
	if (m_Comm.m_bConnected)
	{
		BYTE* Buff = (BYTE*)"di\n";
		printf("FAULHABER: Send: %s", Buff);
		m_Comm.WriteComm(Buff, (DWORD)strnlen_s((const char*)Buff, 100));

		Sleep(100);

		m_Comm.ClosePort();
		printf("FAULHABER: Success to disconnect to %s.\n", port_name);
	}
}


void FaulhaberMotor::RotateMotorRPM(int RPM)
{
	BYTE Buff[100];
	sprintf_s((char*)Buff, sizeof(Buff), "v%d\n", -RPM);
	
	printf("FAULHABER: Send: %s", Buff);
	m_Comm.WriteComm(Buff, (DWORD)strnlen_s((const char*)Buff, 100));
}


void FaulhaberMotor::StopMotor()
{
	BYTE* Buff = (BYTE*)"v0\n";
	printf("FAULHABER: Send: %s", Buff);
	m_Comm.WriteComm(Buff, (DWORD)strnlen_s((const char*)Buff, 100));
}