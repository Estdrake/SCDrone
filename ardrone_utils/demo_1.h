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
	ATClient atc(&atqueue);
	DroneControl control(&atqueue);
	if (int ret = vs.init(); ret > 0)
	{
		std::cerr << "Error during video staging init " << ret << endl;
		return;
	}
	std::thread at = atc.start();
	std::thread st = vs.start();
	std::thread vt = vc.start();
	std::cout << "All thread are started" << std::endl;

	cv::Mat m = cv::imread("image4.png");
	const char* wname = "Drone video stream";
	cv::namedWindow(wname);

	float speed = 0.4f;

	for (;;)
	{
		m = mqueue.pop2();
		cv::imshow(wname, m);

		char c = static_cast<char>(cv::waitKey(10));
		if (c == 27)
			break;
		switch (c)
		{
		case 13: // Enter: decoller
			std::cout << "Take-off" << std::endl;
			atc.set_ref(TAKEOFF_FLAG);
			break;
		case 8: // Backspace: fait atterir 
			std::cout << "Landing" << std::endl;
			atc.set_ref(LAND_FLAG);
			break;
		case 9: // Tab: arrete deplacement 
			std::cout << "Stopping" << std::endl;
			control.stop();
			break;
		case 'w': // vers l'avant
			std::cout << "Moving forward" << std::endl;
			control.move_z(FORWARD, speed);
			break;
		case 'a': // vers gauche
			std::cout << "Moving left" << std::endl;
			control.move_x(LEFT, speed);
			break;;
		case 's': // vers arrière
			std::cout << "Moving backward" << std::endl;
			control.move_z(BACKWARD, speed);
			break;
		case 'd': // vers droit
			std::cout << "Moving right" << std::endl;
			control.move_x(RIGHT, speed);
			break;;
		case 'k': // moins gaz
			std::cout << "Moving lower" << std::endl;
			control.move_y(LOWER, speed);
			break;
		case 'l': // plus gaz
			std::cout << "Moving higher" << std::endl;
			control.move_y(HIGHER, speed);
			break;
		case '1': // descend la vitesse
			if (speed > 0)
				speed -= 0.1f;
			std::cout << "Speed is now " << speed << std::endl;
			break;
		case '2': // augmente la vitess
			if (speed < 1.0f)
				speed += 0.1f;
			std::cout << "Speed is now " << speed << std::endl;
			break;
		default:
			break;
		}
	}
	cv::destroyAllWindows();
	atc.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);

	vs.stop();
	vc.stop();
	atc.stop();

	st.join();
	vt.join();
	at.join();

}


#endif