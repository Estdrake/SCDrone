// mvlar.cpp : Defines the entry point for the application.
//

#include "tracking.h"
#include "filter.h"

using namespace std;



bool SimpleObjectTracker::addImage(cv::Mat& miam)
{
	int n = get_image_noise_level(miam);
	if(n < current_noise)
	{
		best_image = miam;
		current_noise = n;
	}
	return false;
}

cv::Mat SimpleObjectTracker::getBestThreshOutput()
{
	cv::Mat m;
	return m;
}

void traitementImage(const cv::Mat& frame,Mat &retour)
{
	Mat redOnly1;
	Mat redOnly2;

	frame.convertTo(frame, CV_8UC1, 1.5, -15);

	cvtColor(frame, frame, COLOR_BGR2HSV);

	inRange(frame, Scalar(0, 134, 91), Scalar(12, 255, 255), redOnly1);
	inRange(frame, Scalar(168, 134, 91), Scalar(179, 255, 255), redOnly2);

	retour = redOnly1+redOnly2 ;
}
