#ifndef DEMO_1_H
#define DEMO_1_H

#include <opencv2/highgui.hpp>
#include <thread>
#include <chrono>
#include "at_client.h"
#include "at_cmd.h"
#include "video_client.h"
#include "video_staging.h"
using namespace std;

void execute_demo_1()
{
	const char* fenetre = "Drone video stream";
	cv::namedWindow(fenetre);

	// La queue pour les frames vidéos entre VideoClient et VideoStaging
	VFQueue queue;
	// La queue pour les images Mat entre VideoStaging et ici
	MQueue mqueue;
	// La queue pour envoyer des commandes at vers le drone
	ATQueue atqueue;
	
	VideoStaging vs(&queue, &mqueue);
	VideoClient vc(&queue);
	if (int ret = vs.init(); ret > 0)
	{
		std::cerr << "Error during video staging init " << ret << endl;
		return;
	}
	ATClient atc(&atqueue);
	std::thread at = atc.start();
	std::thread st = vs.start();

	std::thread vt = vc.start();
	//atqueue.push(at_format_config("general:navdata_demo", "TRUE"));
	atc.set_ref(EMERGENCY_FLAG);
	atc.set_ref(TAKEOFF_FLAG);
	std::cout << "Set takeoff flag" << std::endl;
	int state = 0;
	int v = 0;
	for(;;)
	{
		cv::Mat m = mqueue.pop2();
		cv::imshow("Drone video stream", m);
		char c = (char)cv::waitKey(1);
		if (c == 27)
			break;
		if(state == 0)
		{
			atqueue.push(at_format_pcmd(HOVERING, 0, 0, 0, 0));
		} else if (state == 1)
		{
			atqueue.push(at_format_pcmd(HOVERING, 0, 0, 0, 0.5));
		}
		v++;
		if (v == 3000000)
		{
			if(state == 1)
			{
				break;
			}
			state = 1;
			v = 0;
		}
			return;
	}

	std::this_thread::sleep_for(3s);
	std::cout << "Stop spinning";
	std::this_thread::sleep_for(2s);
	atc.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);
	atc.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);
	atqueue.push("@");
	at.join();



}


#endif