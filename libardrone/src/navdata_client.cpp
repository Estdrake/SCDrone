
#include "navdata_client.h"
#include "config.h"
#include "navdata_common.h"
#include <thread>

NavDataClient::NavDataClient(ConcurrentQueue< _navdata_option_t>* queue)  {
}

NavDataClient::~NavDataClient() {

}

std::thread NavDataClient::start()
{
	socket->connectToHost(WIFI_ARDRONE_IP, NAVDATA_PORT);
	return std::thread([this] {this->run_service(); });
}


void NavDataClient::run_service() {
	QByteArray readBuffer;
	if (socket->waitForConnected(3000))
	{
		while (!stopRequested())
		{
			if (socket->bytesAvailable() > 0)
			{
				readBuffer = socket->readAll();
			}
			else {
				continue;
			}
			int length = readBuffer.length();
			bool endData = false;




		}
	}
}