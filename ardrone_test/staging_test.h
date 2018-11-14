#ifndef STAGING_TEST_H
#define STAGING_TEST_H

#include "video_common.h"
#include "video_staging.h"


inline void execute_staging_test(const fs::path& folder, int nbr_trame = 40887)
{
	VFQueue queue;
	MQueue mqueue;
	VideoStaging vs(&queue, &mqueue);
	VideoClient vc(&queue);
	if (vs.init() > 0)
	{
		return;
	}
	std::thread st = vs.start();
	std::thread vt = vc.start();

	//cv::namedWindow("Drone video stream");
	//for (;;)
	//{
	//	cv::Mat m = mqueue.pop();
	//	cv::imshow("Drone video stream",m);
	//}

	vt.join();
	st.join();
}
#endif
