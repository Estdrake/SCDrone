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

		float speedXZ = 0.1f;
		float speedYR = 0.4f;

		bool has_image = false;

		for (;;)
		{
			m = mat_queue.pop2_wait(200ms,&has_image);
			if (has_image)
				cv::imshow(wname, m);

			char c = static_cast<char>(cv::waitKey(30));
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
			case 32:
			case 9: // Tab: arrete deplacement 
				std::cout << "Stopping" << std::endl;
				control.stop();
				break;
			case 'w': // vers l'avant
				control.move_z(FORWARD, speedXZ);
				break;
			case 'a': // vers gauche
				control.move_x(LEFT, speedXZ);
				break;;
			case 's': // vers arrière
				control.move_z(BACKWARD, speedXZ);
				break;
			case 'd': // vers droit
				control.move_x(RIGHT, speedXZ);
				break;;
			case 'k': // moins gaz
				control.move_y(LOWER, speedYR);
				break;
			case 'l': // plus gaz
				control.move_y(HIGHER, speedYR);
				break;
			case 'i':
				control.rotate(LEFT, speedYR);
				break;
			case 'o':
				control.rotate(RIGHT, speedYR);
				break;
			case 'q':
				control.move_xy(LEFT, HIGHER, speedXZ,speedYR);
				break;
			case 'e':
				control.move_xy(RIGHT, HIGHER, speedXZ, speedYR);
				break;
			case 'z':
				control.move_xy(LEFT, LOWER, speedXZ, speedYR);
				break;
			case 'c':
				control.move_xy(RIGHT, LOWER, speedXZ, speedYR);
				break;
			case '1': // descend la vitesse XY
				if (speedXZ > 0.1f)
					speedXZ -= 0.1f;
				std::cout << "SpeedXZ is " << speedXZ << std::endl;
				break;
			case '2': // augmente la vitesse XY
				if (speedXZ < 1.0f)
					speedXZ += 0.1f;
				std::cout << "SpeedXZ is " << speedXZ << std::endl;
				break;
			case '3': // descend la vitesse Z
				if (speedYR < 1.0f)
					speedYR += 0.1f;
				std::cout << "SpeedYR is " << speedYR << std::endl;
				break; 
			case '4': // augmente la vitesse Z
				if (speedYR > 0.1f)
					speedYR += 0.1f;
				std::cout << "SpeedYR is " << speedYR << std::endl;
				break;
			case '0': // Ground calibration
				at_queue.push(at_format_ftrim());
				break;
			case '9': // Fly calibration
				at_queue.push(at_format_calib(0));
				break;
			case '8': // Reset to allow flying again
				at_client.set_ref(EMERGENCY_FLAG);
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