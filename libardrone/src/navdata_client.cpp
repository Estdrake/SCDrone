
#include "navdata_client.h"
#include "config.h"
#include "navdata_common.h"
#include <thread>
#include "at_cmd.h"

NavDataClient::NavDataClient(NAVQueue* queue) : local_endpoint(udp::v4(),NAVDATA_PORT),socket(ios,local_endpoint) {
	this->queue = queue;
	this->remote_endpoint = udp::endpoint(ip::address::from_string(WIFI_ARDRONE_IP), NAVDATA_PORT);
	this->recv_buffer = new uint8_t[4086];
}

NavDataClient::~NavDataClient()
{
}


void NavDataClient::init_communication()
{
	uint8_t flag = 1, len = sizeof(flag);
	qDebug() << "Sending init NAVDATA";
	socket.send_to(buffer(&flag, 1), remote_endpoint);
}


void NavDataClient::run_service() {

	init_communication();

	while (!stopRequested())
	{
		try {
			socket.receive(buffer(recv_buffer, 4086));
		} catch(asio::system_error& ex)
		{
			continue;
		}


	}
}
