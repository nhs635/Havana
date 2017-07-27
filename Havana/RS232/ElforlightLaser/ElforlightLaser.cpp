#include "stdafx.h"
#include "ElforlightLaser.h"

#include "Configuration.h"

ElforlightLaser::ElforlightLaser() :
	port_name(ELFORLIGHT_PORT)
{
}


ElforlightLaser::~ElforlightLaser()
{
	if (m_Comm.m_bConnected)
	{
		m_Comm.ClosePort();
		printf("ELFORLIGHT: Success to disconnect to %s.\n", port_name);
	}
}


bool ElforlightLaser::ConnectDevice(HWND hWnd)
{
	m_Comm.hCommWnd = hWnd;

	// 통신 포트를 열기
	if (!m_Comm.m_bConnected)
	{
		CString port; port.Format(_T("\\\\.\\%s"), (CString)port_name);
		if (m_Comm.OpenPort(port, _T("9600"), _T("8"), _T("NO"), _T("NO"), _T("1")))
		{
			printf("ELFORLIGHT: Success to connect to %s.\n", port_name);

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
		}
		else
		{
			printf("ELFORLIGHT: Fail to connect to %s.\n", port_name);
			return false;
		}
	}

	return true;
}


void ElforlightLaser::DisconnectDevice()
{
	if (m_Comm.m_bConnected)
	{
		m_Comm.ClosePort();
		printf("ELFORLIGHT: Success to disconnect to %s.\n", port_name);
	}
}


void ElforlightLaser::IncreasePower()
{
	BYTE Buff[2] = "+";

	Sleep(250);
	m_Comm.WriteComm(Buff, 2);
}


void ElforlightLaser::DecreasePower()
{
	BYTE Buff[2] = "-";

	Sleep(250);
	m_Comm.WriteComm(Buff, 2);
}
