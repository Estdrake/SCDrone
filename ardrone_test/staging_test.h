#ifndef STAGING_TEST_H
#define STAGING_TEST_H

#include "video_common.h"
#include "video_staging.h"
#include "at_client.h"



inline void execute_staging_test(const fs::path& folder, int nbr_trame = 40887)
{
	VFQueue queue;
	MQueue mqueue;
	ATQueue atqueue;
	VideoStaging vs(&queue, &mqueue);
	ATClient atclient(&atqueue);
	VideoClient vc(&queue);
	if (vs.init() > 0)
	{
		return;
	}
	std::thread st = vs.start();
	std::thread vt = vc.start();
	std::thread at = atclient.start();

	//cv::VideoWriter video("output.avi", CV_FOURCC('M', 'J', 'P', 'G'), 10, cv::Size(640, 360));
	atclient.set_ref(EMERGENCY_FLAG);
	atclient.set_ref(TAKEOFF_FLAG);
	for (;;)
	{
		cv::Mat m = mqueue.pop2();
		cv::imshow("Drone video stream", m);

		char c = static_cast<char>(cv::waitKey(1));
		
		if (c == 27)
			break;
		switch(c)
		{
		case 'w': // vers l'avant
			break; 
		case 'a': // vers gauche
			break;;
		case 's': // vers arrière
			break;
		case 'd': // vers droit
			break;;
		case 'k': // moins gaz
			break;
		case 'l': // plus gaz
			break;
		default:
			break;
		}
	}
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(1s);
	atclient.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(1s);
	vc.stop();
	vs.stop();
	//video.release();
	at.join();
	vt.join();
	st.join();
}
#endif
