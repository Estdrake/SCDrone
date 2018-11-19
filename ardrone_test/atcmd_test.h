#ifndef _ATCMD_H_
#define _ATCMD_H_

#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <regex>
#include "at_client.h"
#include "at_cmd.h"
#include "navdata_client.h"

using namespace  std;
using namespace std::chrono;

void execute_atcmd_test(bool printAllTrame = true)
{
	ATQueue at;
	NAVQueue queue;
	ATClient atclient(&at);
	NavDataClient client(&queue);
	std::thread t = client.start();
	std::thread tt = atclient.start();
	std::this_thread::sleep_for(500ms);
	std::string navconf = at_format_config("general:navdata_demo", "TRUE");
	std::string ack = at_format_ack();
	//at.push(navconf);
	//at.push(ack);
	navconf = at_format_config("general:navdata_demo", "TRUE");
	navconf.append(at_format_ack());
	at.push(navconf);
	t.join();
}

#endif