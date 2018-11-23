#include "drone_client.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp><

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "filter.h"
#include "rgb_player.h"
#include "imgui_internal.h"

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error " << error << " : " << description << std::endl;
}


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

	RGBGL_Player			player;
	GLFWwindow*				window;

	

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

	void generate_gui()
	{
		
	}

	void mainLoop() override
	{
		cv::Mat m;
		const char* wname = "Drone video stream";

		bool has_image = false;
		bool has_navdata = false;

		string navconf = at_format_config("general:navdata_demo", "TRUE");
		navconf.append(at_format_ack());
		at_queue.push(navconf);

		while (!glfwWindowShouldClose(window))
		{
			m = mat_queue.pop2_wait(10ms,&has_image);
			nd = nd_client.get_last_nd_demo();
			if (has_image) {
				last_mat = m;
				player.setPixels(last_mat);
			}


			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//###################
			// GUI Navdata
			//###################
			{
				static int range_building[1]{ 0 };
				ImGui::Begin("Information Navdata");
				
				ImGui::TextColored({ 255,255,0,1 }, "Information general");
				ImGui::Text("Pourcentage Batterie : %d", nd.vbat_flying_percentage);
				ImGui::Text("Etat actuelle : %s",nd.ctrl_state & ARDRONE_FLY_MASK ? "FLYING" : "LANDED");
				ImGui::Separator();
				ImGui::TextColored({255,255,0,1}, "Information de vol");

				ImGui::Text("Altitude : %d", nd.altitude);
				ImGui::Text("Phi : %.2f  Theta : %.2f  Psi : %.2f",nd.phi/1000.0f, nd.theta/1000.0f, nd.psi/1000.0f);

				ImGui::End();
			}
			//###################
			// GUI Video Staging
			//###################
			{
				static char record_folder[100];
				static bool record_raw = false;
				ImGui::Begin("Video Staging");

				video_staging_info vsi = video_staging.getInfo();
				strcpy_s(record_folder, vsi.file_name);

				ImGui::TextColored({ 255,255,0,1 }, "Codec");
				ImGui::Separator();
				ImGui::Text("Codec : %s", (vsi.codec == CODEC_MPEG4_AVC) ? "MPEG4 AVC" : "");
				ImGui::Text("Bit rate : %d", vsi.bit_rate);
				ImGui::Text("Grosseur : %d x %d", vsi.d_width, vsi.d_width, vsi.d_height);
				ImGui::Separator();
				ImGui::TextColored({ 255,255,0,1 }, "Performance");
				ImGui::Separator();
				ImGui::Text("Temps moyen decoder %d frames : %g ms", vsi.nbr_frame_gap, vsi.average_decoding_time);
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::TextColored({ 255,255,0,1 }, "Enregistrement");
				ImGui::Separator();
				ImGui::Checkbox("Enregister raw", &record_raw);
				if(record_raw)
				{
					// Démarre l'enregistrement
				}
				if(ImGui::InputText("Dossier enregistrement", record_folder, 100))
				{
					std::cout << "Text change " << record_folder << std::endl;
				}


				ImGui::End();
			}



			presentation_mat = last_mat.clone();

			player.draw(0, 0);

			ImGui::Render();
			int display_w, display_h;
			glfwMakeContextCurrent(window);
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


			glfwMakeContextCurrent(window);

			glfwSwapBuffers(window);

			glfwPollEvents();
		}
	}

public:
	bool init_context()
	{
		glfwSetErrorCallback(glfw_error_callback);
		if (!glfwInit())
		{
			std::cerr << "Erreur initialisation glfw" << std::endl;
			return false;
		}

		window = glfwCreateWindow(1600, 900, "ARDRONE GUI", nullptr, nullptr);
		if (!window)
		{
			glfwTerminate();
			std::cerr << "Erreur creation de la fenetre glfw" << std::endl;
			return false;
		}
		glfwMakeContextCurrent(window);
		glfwSwapInterval(1); // vsync

		int v = glewInit();


		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 430");

		// Setup Style
		ImGui::StyleColorsDark();


		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);


		if(!player.setup(640,360))
		{
			printf("Echec configuration du lecteur video opengl\n");
			return false;
		}

		return true;
	}

	void end_context()
	{
		glfwTerminate();
	}
};


int main(int argc,char* argv[])
{
	DroneKB kb;
	if(!kb.init_context())
	{
		printf("Impossible d'initialiser le context , fermeture");
		kb.end_context();
		return 1;
	}
	kb.Start();
	kb.end_context();
	return 0;
}