#include "drone_client.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "filter.h"
#include "tracking.h"


class DroneKB : public DroneClient
{
public:
	virtual ~DroneKB() = default;

	DroneKB() : last_mat(640,360,CV_8UC3,cv::Scalar(0,0,0)), presentation_mat(last_mat.clone())
	{
		nd = {};
		speedXZ = 0.1f;
		speedYR = 0.4f;
	}

private:

	cv::Mat					last_mat;
	cv::Mat					presentation_mat;

	navdata_demo_t			nd;

	float					speedXZ;
	float					speedYR;

	void displayCurrentInfo(int noise)
	{
		std::stringstream ss;
		ss << "Battery " << nd.vbat_flying_percentage << "% " << "Altitude " << nd.altitude << " Phi " << nd.phi/1000 << " Theta " << nd.theta/1000 << " Psi " << nd.psi/1000;
		cv::putText(presentation_mat, ss.str(), cv::Point(30, 30), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
		ss = std::stringstream();
		ss << "Velocity X " << nd.vx << " Y " << nd.vy << " Z " << nd.vz;
		cv::putText(presentation_mat, ss.str(), cv::Point(30, 60), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
		ss = std::stringstream();
		ss << "Image noise " << noise;
		cv::putText(presentation_mat, ss.str(), cv::Point(30, 90), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
		ss = std::stringstream();
		ss << "Speed XZ " << speedXZ << " Speed YR " << speedYR;
		cv::putText(presentation_mat, ss.str(), cv::Point(30, 330), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
	}

	void mainLoop() override
	{
		cv::Mat m;
		const char* wname = "Drone video stream";
		cv::namedWindow(wname);

		bool has_image = false;
		bool has_navdata = false;

		string navconf = at_format_config("general:navdata_demo", "TRUE");
		navconf.append(at_format_ack());
		at_queue.push(navconf);

		for (;;)
		{
			m = mat_queue.pop2_wait(10ms,&has_image);
			//nd = nav_queue.pop_wait(50ms, &has_navdata);
			nd = nd_client.get_last_nd_demo();
			if (has_image)
				last_mat = m;

			presentation_mat = last_mat.clone();

			int v = get_image_noise_level(last_mat);
			cv::imshow("ref", presentation_mat);
			traitementImage(presentation_mat, presentation_mat);

			displayCurrentInfo(v);

			
				
			cv::imshow(wname, presentation_mat);


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
				if (speedYR > 0.1f)
					speedYR -= 0.1f;
				std::cout << "SpeedYR is " << speedYR << std::endl;
				break; 
			case '4': // augmente la vitesse Z
				if (speedYR < 1.0f)
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