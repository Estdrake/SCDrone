#ifndef CONFIG_SERVER_H
#define CONFIG_SERVER_H

#include <asio.hpp>
#include "thread.h"

using namespace asio;
using ip::tcp;

class ConfigServer : public Runnable
{
public:
	void run_service() override;
private:
	tcp::socket socket;
	std::string msg;

public:
	ConfigServer(io_service& ios) : socket(ios)
	{
		
	}
	
};


#endif // CONFIG_SERVER_H