#include "filter.h"

int get_image_noise_level(const cv::Mat& o)
{
	cv::Mat g;
	cv::Mat lap;
	cv::cvtColor(o, g, cv::COLOR_BGR2GRAY);
	cv::Laplacian(g, lap, CV_64F);
	Scalar mean, stddev;
	meanStdDev(lap, mean, stddev, Mat());
	return static_cast<int>(stddev.val[0] * stddev.val[0]);
}