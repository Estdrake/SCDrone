#include "config_server.h"
#include "config.h"

void ConfigServer::run_service()
{
	try {
		socket.bind(tcp::endpoint(tcp::v4(), CONTROL_PORT));
	} catch(asio::system_error& ex)
	{
		std::cout << "Exception on bind : " << ex.what() << std::endl;
	}
}
