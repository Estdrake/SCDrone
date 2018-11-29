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

cv::Mat SimpleObjectTracker::getBestThreshOutput(bool& is_ready)
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
	// Approximate contours to polygons + get bounding rects and circles
	dilate(retour, retour, Mat(), Point(-1, -1), 20);
	erode(retour, retour, Mat(), Point(-1, -1), 15);
	
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
	o.position = { (int)(m.m10 / m.m00), (int)(m.m01 / m.m00) };
	object_info_list.emplace_back(o);
	return true;
}

void SimpleObjectTracker ::followObject (ATClient * at_client,cv::Point objectPosition, speed drone_speed)
{
	//convertion des coordonné ver NDC
	float x = ((objectPosition.x / 640) * 2) - 1;
	float y = (((-(objectPosition.y - 360)) * 2) / 360) - 1;

	if (x > 0)
	{
		at_client->setProgressiveFlag(PROGRESSIVE);
		at_client->setSpeedX(RIGHT, drone_speed.x);
	}
	if (x < 0)
	{
		at_client->setProgressiveFlag(PROGRESSIVE);
		at_client->setSpeedX(LEFT, drone_speed.x);
	}
	if (x > -0.02 && x < 0.02)
	{
		
		at_client->setSpeedX(NONE_X, 0.0f);
	}
	if (y > 0)
	{
		at_client->setProgressiveFlag(COMBINED_YAW);
		at_client->setSpeedY(HIGHER, drone_speed.y);
	}
	if (y < 0)
	{
		at_client->setProgressiveFlag(COMBINED_YAW);
		at_client->setSpeedY(LOWER, drone_speed.y);
	}
	if (y > -0.02 && y < 0.02)
	{
		
		at_client->setSpeedY(NONE_Y, 0.0f);
	}

	if ((y > -0.02 && y < 0.02) && (x > -0.02 && x < 0.02))
	{
		at_client->hover();
	}


}
