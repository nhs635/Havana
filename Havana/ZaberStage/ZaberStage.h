#ifndef _ZABER_STAGE_H_
#define _ZABER_STAGE_H_

#include "zb_serial.h"
#include <thread>

class ZaberStage
{
public:
	ZaberStage();
	~ZaberStage();

private:
	void SetCommandData(int cmd_data, uint8_t* msg);
	void GetReplyData(uint8_t* msg, int &reply_val);

public:
	bool ConnectStage();
	void DisconnectStage();
	void StopWaitThread();

	void Home();
	void Stop();
	void MoveAbsoulte(double position);
	void SetTargetSpeed(double speed);	

public:
	const char* device_name;
	double conversion_factor;
	int max_micro_resolution;
	int micro_resolution;
	
private:
	z_port port;
	double microstep_size;
	uint8_t home[6], stop[6], move_absolute[6], target_speed[6], received_msg[6];

private:
	std::thread t_wait;
	bool _running;

};

#endif