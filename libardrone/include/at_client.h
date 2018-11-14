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

typedef ConcurrentQueue<std::string> ATQueue;

class ATClient : public QObject
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

	std::thread start();

	void set_ref(ref_flags f);


public slots:
	void run_service();

private slots:
	void on_read_ready();
};


#endif
