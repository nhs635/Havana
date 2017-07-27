메세지를 받을 클래스의 헤더파일에 아래 코드들을 추가한다.

#include "CommThread.h"

클래스 내부 인스턴스 할당
private: //필요에 따라. public도 상관없음
	LONG OnReceive(UINT port,LONG lParam);	//메세지 처리기
	CCommThread m_Comm;	//시리얼 통신 객체

--------------------------------------------------------------

메세지를 받을 클래스의 cpp 파일에 아래 코드들을 추가한다.

//OnInitDialog() 에 추가.
	m_Comm.hCommWnd = this->m_hWnd;

// 통신 포트를 열기
	if (!m_Comm.m_bConnected)
	{
		if(m_Comm.OpenPort("\\\\.\\COM1", "9600", "8","NO","NO","1"))
		{
			//접속 성공
		}
		else
		{
			//접속 실패
		}
	}

//통신 포트 닫기
	if (m_Comm.m_bConnected) m_Comm.ClosePort();

//데이터 전송
	BYTE Buff[6] = "hello";	//with NULL 5 char+ 1NULL = 6BYTE
	m_Comm.WriteComm(Buff,5);

//메세기 처리기
//BEGIN_MESSAGE_MAP((클래스), CDialogEx)
	ON_MESSAGE(WM_COMM_READ, OnReceive)    // Communication Message Handleer
//END_MESSAGE_MAP()

//데이터 수신(메세지 처리기)
LONG (클래스)::OnReceive(UINT port, LONG lParam)
{
	BYTE aByte;
	m_Comm.m_bReserveMsg = false;

	int size = (m_Comm.m_QueueRead).GetSize();
	if (size == 0) return 0;

	for (int i=0;i<size;i++)
	{
		(m_Comm.m_QueueRead).GetByte(&aByte);
		//aByte 이용 여기서 데이터 처리
	}
return 0;
}

//열려있는 포트 이름 얻기
	CStringArray* ComPorts = m_Comm.GetPorts();
-----------------------------------------------------------------