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
	}
	ATQueue queue;
	ATClient atclient(&queue);
	std::thread t = atclient.start();

	atclient.set_ref(EMERGENCY_FLAG);
	atclient.set_ref(TAKEOFF_FLAG);

	std::this_thread::sleep_for(3s);

	for(int i = 0; i < 40;i++)
	{
		queue.push(at_format_pcmd(HOVERING, 0, 0, 0, 0.5));
		std::this_thread::sleep_for(60ms);
	}
	std::cout << "Stop spinning";
	std::this_thread::sleep_for(2s);
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(1s);
	queue.push("@");
	t.join();
}

#endif