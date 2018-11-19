
#include "control.h"
#include "demo_1.h"
#include <thread>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

int main(int argc, char* argv[])
{
	execute_demo_1();
	return 0;
	/*
	QCoreApplication a(argc, argv);
	ATQueue queue;
	ATClient client(&queue,&a);
	std::thread t = client.start();
	GamepadMonitor monitor(&queue,&a,&client);
	
	bool run = true;

	ref_flags f = DEFAULT_FLAG;
	float gaz = 0.0f, roll = 0.0f, yaw = 0.0f, pitch = 0.0f;
	std::cout << "Start" << std::endl;
	while (run) {
		int v = cv::waitKey(0);
		switch ((char)v) {
		case 'p':
			if (f == DEFAULT_FLAG) {
				std::cout << "take off" << std::endl;
				f = TAKEOFF_FLAG;
			}
			else {
				f = DEFAULT_FLAG;
				run = false;
			}
			client.set_ref(f);
			break;
		case 'w':
			if (gaz < 1.0f) {
				gaz += 0.1f;
			}
			break;
		case 's':
			if (gaz > -1.0f) {
				gaz -= 0.1f;
			}
			break;
		case 'a':
			if (yaw < 1.0f) {
				yaw += 0.1f;
			}
			break;
		case 'd':
			if (yaw > -1.0f)
				yaw -= 0.1f;
			break;
		}
		//queue.push(at_format_pcmd(PROGRESSIVE, roll, pitch, gaz, yaw));
	}
	*/
	return 0;
}