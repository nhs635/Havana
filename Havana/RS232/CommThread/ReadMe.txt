�޼����� ���� Ŭ������ ������Ͽ� �Ʒ� �ڵ���� �߰��Ѵ�.

#include "CommThread.h"

Ŭ���� ���� �ν��Ͻ� �Ҵ�
private: //�ʿ信 ����. public�� �������
	LONG OnReceive(UINT port,LONG lParam);	//�޼��� ó����
	CCommThread m_Comm;	//�ø��� ��� ��ü

--------------------------------------------------------------

�޼����� ���� Ŭ������ cpp ���Ͽ� �Ʒ� �ڵ���� �߰��Ѵ�.

//OnInitDialog() �� �߰�.
	m_Comm.hCommWnd = this->m_hWnd;

// ��� ��Ʈ�� ����
	if (!m_Comm.m_bConnected)
	{
		if(m_Comm.OpenPort("\\\\.\\COM1", "9600", "8","NO","NO","1"))
		{
			//���� ����
		}
		else
		{
			//���� ����
		}
	}

//��� ��Ʈ �ݱ�
	if (m_Comm.m_bConnected) m_Comm.ClosePort();

//������ ����
	BYTE Buff[6] = "hello";	//with NULL 5 char+ 1NULL = 6BYTE
	m_Comm.WriteComm(Buff,5);

//�޼��� ó����
//BEGIN_MESSAGE_MAP((Ŭ����), CDialogEx)
	ON_MESSAGE(WM_COMM_READ, OnReceive)    // Communication Message Handleer
//END_MESSAGE_MAP()

//������ ����(�޼��� ó����)
LONG (Ŭ����)::OnReceive(UINT port, LONG lParam)
{
	BYTE aByte;
	m_Comm.m_bReserveMsg = false;

	int size = (m_Comm.m_QueueRead).GetSize();
	if (size == 0) return 0;

	for (int i=0;i<size;i++)
	{
		(m_Comm.m_QueueRead).GetByte(&aByte);
		//aByte �̿� ���⼭ ������ ó��
	}
return 0;
}

//�����ִ� ��Ʈ �̸� ���
	CStringArray* ComPorts = m_Comm.GetPorts();
-----------------------------------------------------------------