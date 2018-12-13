/*-------------------------------------------------------------------------- - */
/**
 * @file   ap_client.h
 * @brief  D�finit la classe qui s'occupe de la gestion de l'envoi des commandes
 *			at vers le drone
 *
 ******************************************************************************/

#ifndef AP_CLIENT_H
#define AP_CLIENT_H
#include "thread.h"
#include "at_cmd.h"
#include <QObject>
#include <QUdpSocket>
#include <navdata_common.h>
#include <chrono>
#include <future>

typedef ConcurrentQueue<std::string> ATQueue;

enum x_direction
{
	LEFT,
	RIGHT,
	NONE_X
};

enum y_direction
{
	HIGHER,
	LOWER,
	NONE_Y
};

enum z_direction
{
	FORWARD,
	BACKWARD,
	NONE_Z
};

struct speed
{
	float x;
	float y;
	float z;
	float r;
};


class ATClient : public QObject, public Runnable
{
	ATQueue* queue;

	QUdpSocket* socket;
	QHostAddress* sender;
	quint16 port;

	std::atomic<ref_flags>			ref_mode;
	std::atomic<progressive_flags>	prog_flag;
	std::atomic<speed>				speed_drone;

	int sequence_nbr;

public:
	ATClient(ATQueue* queue,QObject* parent = 0);
	~ATClient();
	
	void setVector2D(float x, float y);

	void setSpeedX(x_direction d, float x);
	void setSpeedY(y_direction d, float y);
	void setSpeedZ(z_direction d, float z);
	void setSpeedR(x_direction d, float r);

	void hover();

	void setProgressiveFlag(progressive_flags f)
	{
		prog_flag = f;
	}

	speed getSpeed() const
	{
		return speed_drone;
	}
	progressive_flags getProgressiveFlag() const
	{
		return prog_flag;
	}

	void set_ref(ref_flags f);

	const char* get_ref() const
	{
		ref_flags f = ref_mode;
		switch(f)
		{
		case EMERGENCY_FLAG:
			return "EMERGENCY";
		case TAKEOFF_FLAG:
			return "FLYING";
		default:
			return "LANDED";
		}
		
	}


public slots:
	void run_service();

private slots:
	void on_read_ready();
};

class DroneControl
{
	ATQueue* queue;
	std::promise<void>			exitSignal;
	std::shared_future<void>	futureObj;
	std::shared_future<void>	futureCurrent;
	std::atomic<bool>			started;

	std::chrono::milliseconds		interval_msg{};

public:
	DroneControl(ATQueue* queue);

	void rotate(x_direction, float speed);
	void move_x(x_direction, float speed);
	void move_y(y_direction, float speed);
	void move_z(z_direction, float speed);
	void move_xy(x_direction, y_direction, float speedX, float speedY);

	void set_interval(std::chrono::milliseconds ms) const;

	bool wait_for(std::chrono::milliseconds ms) const;
	void stop();
};

#endif
