#ifndef _NAVDATA_CLIENT_H_
#define _NAVDATA_CLIENT_H_

#include "navdata_client.h"
#include "config.h"
#include "navdata_common.h"
#include "thread.h"

#include <iostream>
#include <thread>

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

typedef ConcurrentQueue<_navdata_option_t> NAVQueue;

class NavDataClient{

    public:
        NavDataClient(NAVQueue* queue);
        ~NavDataClient();

		std::thread start();

    private:
		NAVQueue*  queue;
		QTcpSocket* socket;
		QByteArray bufferNavData;
		bool isConected;
		void run_service();
};

#endif // _NAVDATA_CLIENT_H_
