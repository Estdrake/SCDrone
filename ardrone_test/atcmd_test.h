#ifndef _ATCMD_H_
#define _ATCMD_H_

#include <iostream>
#include <regex>
#include "at_client.h"
#include "at_cmd.h"

using namespace  std;

void execute_atcmd_test(bool printAllTrame = true)
{
	if (printAllTrame)
	{
		string cmd = at_format_ref(TAKEOFF_FLAG);
		cmd = std::regex_replace(cmd, std::regex("\\$seq"), std::to_string(1));
		cout << cmd << endl;

		cmd = at_format_config("general:navdata_demo", "TRUE");
		cmd = std::regex_replace(cmd, std::regex("\\$seq"), std::to_string(2));
		cout << cmd << endl;
	}
	ATQueue queue;
	ATClient atclient(&queue);
	std::thread t = atclient.start();

	//queue.push(at_format_config("general:navdata_demo", "TRUE"));
	atclient.set_ref(EMERGENCY_FLAG);
	atclient.set_ref(TAKEOFF_FLAG);

	std::this_thread::sleep_for(8s);

	for(int i = 0; i < 40;i++)
	{
		queue.push(at_format_pcmd(HOVERING, 0, 0, 0, 0.5));
		std::this_thread::sleep_for(60ms);
	}
	std::this_thread::sleep_for(3s);
	std::cout << "Stop spinning";
	std::this_thread::sleep_for(2s);
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);
	queue.push("@");
	t.join();
}

#endif