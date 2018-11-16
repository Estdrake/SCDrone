#include "drone_client.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


class DroneKB : public DroneClient
{
public:
	virtual ~DroneKB() = default;

private:
	void mainLoop() override
	{
		cv::Mat m;

		const char* wname = "Drone video stream";
		cv::namedWindow(wname);

		float speed = 0.4f;

		for (;;)
		{
			m = mat_queue.pop2();
			cv::imshow(wname, m);

			char c = static_cast<char>(cv::waitKey(0));
			std::cout << c << std::endl;
			if (c == 27)
				break;
			switch (c)
			{
			case 13: // Enter: decoller
				std::cout << "Take-off" << std::endl;
				at_client.set_ref(TAKEOFF_FLAG);
				break;
			case 8: // Backspace: fait atterir 
				std::cout << "Landing" << std::endl;
				at_client.set_ref(LAND_FLAG);
				break;
			case 9: // Tab: arrete deplacement 
				std::cout << "Stopping" << std::endl;
				control.stop();
				break;
			case 'w': // vers l'avant
				control.move_z(FORWARD, speed);
				break;
			case 'a': // vers gauche
				control.move_x(LEFT, speed);
				break;;
			case 's': // vers arrière
				control.move_z(BACKWARD, speed);
				break;
			case 'd': // vers droit
				control.move_x(RIGHT, speed);
				break;;
			case 'k': // moins gaz
				control.move_y(LOWER, speed);
				break;
			case 'l': // plus gaz
				control.move_y(HIGHER, speed);
				break;
			case '1': // descend la vitesse
				if (speed > 0)
					speed -= 0.1f;
				break;
			case '2': // augmente la vitess
				if (speed < 1.0f)
					speed += 0.1f;
				break;
			default:
				break;
			}
		}
		cv::destroyAllWindows();
	}
};


int main(int argc,char* argv[])
{
	DroneKB kb;
	return kb.Start();
}