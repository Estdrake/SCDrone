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

cv::Mat SimpleObjectTracker::getBestThreshOutput(bool& is_ready,obj_info& obj)
{
	if (!chrono.isOver())
	{
		is_ready = false;
		return cv::Mat();
	}
	is_ready = true;
	cv::Mat out;
	if(best_image.empty())
	{
		is_ready = false;
		return out;
	}
	if (get_interval_between)
		traitementThreshold(best_image, out);
	else
		traitementThresholdExterieur(best_image, out);
	// Approximate contours to polygons + get bounding rects and circles
	dilate(out, out, Mat(), Point(-1, -1), 20);
	erode(out, out, Mat(), Point(-1, -1), 15);

	if (tryFoundObject(out)) {
		obj = last_info;
	}

	chrono.reset();
	current_noise = 3000;
	return out;
}



void SimpleObjectTracker::traitementThresholdExterieur(const cv::Mat& frame, cv::Mat& retour)
{
	if (frame.empty())
		return;
	Mat redOnly1;
	Mat redOnly2;

	frame.convertTo(retour, CV_8UC1, 1.5, -15);

	cvtColor(retour, retour, COLOR_BGR2HSV);

	inRange(retour, low_thresh_1, high_thresh_1, redOnly1);
	inRange(retour, low_thresh_2, low_thresh_2, redOnly2);

	retour = redOnly1 + redOnly2;
}

void SimpleObjectTracker::traitementThreshold(const cv::Mat& frame, cv::Mat& retour)
{
	cv::medianBlur(frame, retour, 5);
	frame.convertTo(retour, CV_8UC1, 1.5, -15);

	cvtColor(retour, retour, COLOR_BGR2HSV);

	inRange(retour, low_thresh_1, high_thresh_1, retour);
}

bool SimpleObjectTracker::tryFoundObject(const cv::Mat& img)
{
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, Point(0, 0));
	vector<Point> biggest;
	if (contours.empty()) // ou qu'il a trop de contour
	{
		return false;
	}
	for (const auto& vp : contours)
	{
		if (vp.size() > biggest.size())
			biggest = vp;
	}
	Moments m = moments(biggest);
	obj_info o;
	o.pixel_area = m.m00;
	o.at_time = HRClock::now();
	o.position = { (float)(m.m10 / m.m00), (float)(m.m01 / m.m00) };
	//convertion des coordonné ver NDC
	o.position[0] = ((o.position[0] / 640.0f) * 2.0f) - 1.0f;
	o.position[1] = (((-(o.position[1] - 360.0f)) * 2.0f) / 360.0f) - 1.0f;
	last_info = o;
	return true;
}
