/*-------------------------------------------------------------------------- - */
/**
 * @file   ap_client.h
 * @brief  Définit la classe qui s'occupe de la gestion de l'envoi des commandes
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

typedef ConcurrentQueue<std::string> ATQueue;

class ATClient : public QObject, public Runnable
{
	ATQueue* queue;
	//ATQueue* config_queue;

	QUdpSocket* socket;
	QHostAddress* sender;
	quint16 port;

	std::atomic<ref_flags> ref_mode;

	int sequence_nbr;

public:
	ATClient(ATQueue* queue,QObject* parent = 0);
	~ATClient();

	void set_ref(ref_flags f);


public slots:
	void run_service();

private slots:
	void on_read_ready();
};

enum x_direction
{
	LEFT,
	RIGHT
};

enum y_direction
{
	HIGHER,
	LOWER
};

enum z_direction
{
	FORWARD,
	BACKWARD
};

class DroneControl
{
	ATQueue* queue;
	std::promise<void>			exitSignal;
	std::shared_future<void>	futureObj;
	std::shared_future<void>	futureCurrent;
	std::atomic<bool>			started;

	milliseconds		interval_msg{};

public:
	DroneControl(ATQueue* queue);

	void rotate(x_direction, float speed);
	void move_x(x_direction, float speed);
	void move_y(y_direction, float speed);
	void move_z(z_direction, float speed);
	void move_xy(x_direction, y_direction, float speedX, float speedY);

	void set_interval(milliseconds ms) const;

	bool wait_for(milliseconds ms) const;
	void stop();
};

#endif
