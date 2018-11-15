#ifndef _ATCMD_H_
#define _ATCMD_H_

#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <regex>
#include "at_client.h"
#include "at_cmd.h"

using namespace  std;
using namespace std::chrono;

bool rotateForTime(ATQueue* queue,std::future<void> future, float speed) {

	while (future.wait_for(1ms) == std::future_status::timeout) {
		queue->push(at_format_pcmd(HOVERING, 0, 0, 0, 0.5));
		std::this_thread::sleep_for(60ms);
	}

	queue->push(at_format_pcmd(HOVERING, 0, 0, 0, -0.5));
	queue->push(at_format_pcmd(HOVERING, 0, 0, 0, -0.5));
	queue->push(at_format_pcmd(HOVERING, 0, 0, 0, -0.5));
	queue->push(at_format_pcmd(HOVERING, 0, 0, 0, -0.5));
	return true;
}


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

	queue.push(at_format_config("general:navdata_demo", "TRUE"));
	queue.push(at_format_controlinit());
	atclient.set_ref(EMERGENCY_FLAG);
	//atclient.set_ref(TAKEOFF_FLAG);

	std::this_thread::sleep_for(4s);
	std::promise<void> exitSignal;
	std::future<void> futureObj = exitSignal.get_future();
	std::async(std::launch::async, rotateForTime, &queue, std::move(futureObj),0.5f);
	std::cout << "starting async spinning" << std::endl;
	std::this_thread::sleep_for(3s);
	exitSignal.set_value();
	std::cout << "done async spinning" << std::endl;

	std::this_thread::sleep_for(2s);
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);
	queue.push("@");
	t.join();
}

#endif