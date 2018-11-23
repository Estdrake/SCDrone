// mvlar.cpp : Defines the entry point for the application.
//

#include "tracking.h"

using namespace std;


void DoStuff() {
	cout << "Doing stuff" << endl;
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
