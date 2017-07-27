#ifndef _ELFORLIGHT_LASER_H_
#define _ELFORLIGHT_LASER_H_

#include "RS232\CommThread\CommThread.h"

#include <iostream>
#include <thread>

class ElforlightLaser
{
public:
	ElforlightLaser();
	~ElforlightLaser();

public:
	bool ConnectDevice(HWND hWnd);
	void DisconnectDevice();
	void IncreasePower();
	void DecreasePower();

private:
	CCommThread m_Comm;	//�ø��� ��� ��ü
	const char* port_name;
};

#endif