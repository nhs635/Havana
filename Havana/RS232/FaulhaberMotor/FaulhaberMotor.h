#ifndef _FAULHABER_MOTOR_H_
#define _FAULHABER_MOTOR_H_

#include "RS232\CommThread\CommThread.h"

#include <iostream>
#include <thread>

class FaulhaberMotor
{
public:
	FaulhaberMotor();
	~FaulhaberMotor();

public:
	bool ConnectDevice(HWND hWnd);
	void DisconnectDevice();
	void RotateMotorRPM(int RPM);
	void StopMotor();

private:
	CCommThread m_Comm;	//�ø��� ��� ��ü
	const char* port_name;
};

#endif