#include "drone_client.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_other.h"

#include "filter.h"
#include "rgb_player.h"
#include "imgui_internal.h"

#include "logger.h"
#include "tracking.h"
#include "chrono.h"


static cv::Scalar toU8C3(const float* data)
{
	return { data[2] * 255.0f,data[1] * 255.0f,data[0] * 255.0f };
}


static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error " << error << " : " << description << std::endl;
}
class DroneKB : public DroneClient
{
public:
	virtual ~DroneKB() = default;

	DroneKB() : DroneClient(true),last_mat(640,360,CV_8UC3,cv::Scalar(0,0,0)), presentation_mat(last_mat.clone())
	{
		nd = {};
		speedXZ = 0.1f;
		speedYR = 0.4f;
	}

private:

	bool					are_thread_running = true;

	cv::Mat					last_mat;
	cv::Mat					presentation_mat;

	navdata_demo_t			nd;

	float					speedXZ;
	float					speedYR;

	RGBGL_Player			player;
	GLFWwindow*				window;

	SimpleObjectTracker		obj_tracker;


	bool					run_main_loop = true;

	bool					show_nd = true;
	bool					show_vs_info = false;
	bool					show_log = false;
	bool					show_basic_cmd_drone = true;
	bool					show_color_obj_tracking = true;

	bool					show_info_overlay = true;


	bool					show_manual_control = true;
	bool					enable_manual_control = false;
	bool					enable_tracking_video = false;
	bool					enable_autopilot_xy = false;
	speed					speed_drone = { 0.2f,0.4f,0.2f,0.4f };

	void show_log_window()
	{
		if(show_log)
		{
			static ExampleAppLog log;

			for(auto v : logger.Consume())
			{
				log.AddLog(v.c_str());
			}
			log.Draw("Logger AR Drone",&show_log);
		}
	}

	void show_metric_overlay(bool* p_open) {
		const float DISTANCE = 10.0f;
		static int corner = 3;
		ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
		ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
		if (corner != -1)
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
		if (ImGui::Begin("Information Application", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGui::Text("Version : %d:%d:%d \n" "Framerate %.2f DeltaTime %.2f",0,0,1,io.Framerate,io.DeltaTime);
			if (ImGui::BeginPopupContextWindow())
			{
				if (p_open && ImGui::MenuItem("Close")) *p_open = false;
				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	void show_app_menu_bar()
	{
		
		if(ImGui::BeginMainMenuBar())
		{
			if(ImGui::BeginMenu("Fichier"))
			{
				ImGui::Separator();
				if(ImGui::MenuItem("Quitter", "Alt+F4"))
				{
					run_main_loop = false;
				}
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Fenetre"))
			{
				ImGui::Checkbox("Navdata", &show_nd);
				ImGui::Checkbox("Video Staging", &show_vs_info);
				ImGui::Checkbox("Logger", &show_log);

				ImGui::Checkbox("Commande", &show_basic_cmd_drone);
				ImGui::Checkbox("Controle", &show_manual_control);
				ImGui::Checkbox("Tracking", &show_color_obj_tracking);

				
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		
	}


	void show_color_obj_tracking_window()
	{
		if(show_color_obj_tracking)
		{
			static float 	low_color[4] {0.0f,134.0f,91.0f};
			static float 	high_color[4] {12.0f,255.0f,255.0f};

			static float	low_color2[4] {168.0f,134.0f,91.0f};
			static float	high_color2[4] {179.0f,255.0f,255.0f};

			static int		interval_time = 750;

			static int 		size_obj[2]{ 20 , 20 };

			static bool		external_interval = false;

			static bool		is_started = false;
			

			int misc_flags = ImGuiColorEditFlags_HSV;


			ImGui::Begin("Tracking d'object colorer",&show_color_obj_tracking);

			ImGui::TextColored({ 255,255,0,1 }, "Configuration de l'object");
			ImGui::Separator();
			ImGui::DragInt2("Dimension (CM)", size_obj);

			ImGui::Checkbox("Interval Exterieur",&external_interval);

			ImGui::Text("Min");
			if(ImGui::DragFloat3("Min##1", (float*)low_color,1,0,255))
			{
				obj_tracker.setLowTresh(cv::Scalar(low_color[0], low_color[1], low_color[2]));
			}

			ImGui::Text("Max");
			if(ImGui::DragFloat3("Max##2", (float*)high_color,1,0,255))
			{
				obj_tracker.setHighTresh(cv::Scalar(high_color[0], high_color[1], high_color[2]));
			}

			if(external_interval)
			{
				ImGui::Text("Min 2");
				if(ImGui::DragFloat3("Min##3", (float*)low_color2,1,0,255))
				{
					obj_tracker.setLowTresh2(cv::Scalar(low_color2[0],low_color2[1],low_color2[2]));
				}
				ImGui::Text("Max 2");
				if(ImGui::DragFloat3("Max##4", (float*)high_color2,1,0,255))
				{
					obj_tracker.setHighTresh2(cv::Scalar(high_color2[0], high_color2[1], high_color2[2]));
				}
			}

			if(ImGui::DragInt("Interval entre détection (MS)",&interval_time,0.05,0,500))
			{
				obj_tracker.setIntervalMS(interval_time);
			}

			ImGui::Separator();

			if(is_started) {

				if (ImGui::Button("Arreter", { 200,20 })) {
					is_started = false;
					enable_tracking_video = false;
					player.disable2();
				}
				ImGui::SameLine();//esteban est rendu la
				if (ImGui::Button("Demarrer", { 200,20 })) {
					enable_autopilot_xy = true;

				}
				ImGui::Separator();
				auto i = obj_tracker.getLastObjectInfo();
				ImGui::VSliderInt("Y", ImVec2(18, 160), &i.position.y,360,0);
				ImGui::SameLine();
				ImGui::SliderInt("X", &i.position.x, 0, 640);
				ImGui::SameLine();
				ImGui::Text("Air : %.2f", i.pixel_area);

			} else {
				if (ImGui::Button("Demarrer", {200, 20})) {
					is_started = true;
					obj_tracker.setModeThreeshold(!external_interval);
					obj_tracker.reset();
					enable_tracking_video = true;
				}
			}
			

			


			ImGui::End();
			
		}
	}

	void show_drone_basic_command_window()
	{
		if(show_basic_cmd_drone)
		{
			ImGui::Begin("Commande de Base",&show_basic_cmd_drone);
			if (ImGui::Button("Urgence",{150,50}))
			{
				at_client.set_ref(EMERGENCY_FLAG);
			}
			ImGui::SameLine();
			if (ImGui::Button("Calibrage au sol",{150,50}))
			{
				at_queue.push(at_format_ftrim());

			}
			ImGui::SameLine();
			if (ImGui::Button("Calibrage en vol",{150,50}))
			{
				if (at_client.get_ref() == "FLYING") {
					at_queue.push(at_format_calib(0));
				}
				else
				{
					AR_LOG_ERROR(0, "Le drone doit etre en vol\n");
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Changer camera", { 150,50 }))
			{
				// TODO permettre changer de caméra 
			}

			ImGui::End();
		}
		
	}

	void show_nav_data_window()
	{
		if (show_nd) {
			static int range_building[1]{ 0 };
			ImGui::Begin("Information Navdata",&show_nd);

			ImGui::TextColored({ 255,255,0,1 }, "Information general");
			ImGui::Text("Pourcentage Batterie : %d", nd.vbat_flying_percentage);
			ImGui::Text("Etat actuelle : %s", nd.ctrl_state & ARDRONE_FLY_MASK ? "FLYING" : "LANDED");
			ImGui::Separator();
			ImGui::TextColored({ 255,255,0,1 }, "Information de vol");

			ImGui::Text("Altitude : %d", nd.altitude);
			ImGui::Text("Phi : %.2f  Theta : %.2f  Psi : %.2f", nd.phi / 1000.0f, nd.theta / 1000.0f, nd.psi / 1000.0f);
			ImGui::Text("Velociter X : %.2f Y : %.2f", nd.vx, nd.vy);

		
			ImGui::End();
		}
	}

	void show_video_staging_info_window()
	{
		if (show_vs_info) {
			static char record_folder[100];
			static bool record_raw = false;
			ImGui::Begin("Video Staging",&show_vs_info);

			video_staging_info vsi = video_staging.getInfo();
			strcpy(record_folder, vsi.file_name);

			ImGui::TextColored({ 255,255,0,1 }, "Codec");
			ImGui::Separator();
			ImGui::Text("Codec : %s", (vsi.codec == CODEC_MPEG4_AVC) ? "MPEG4 AVC" : "");
			ImGui::Text("Bit rate : %d", vsi.bit_rate);
			ImGui::Text("Grosseur : %d x %d", vsi.d_width, vsi.d_width, vsi.d_height);
			ImGui::Separator();
			ImGui::TextColored({ 255,255,0,1 }, "Performance");
			ImGui::Separator();
			ImGui::Text("Interval de %d frames Temps moyens : %g ms Frames perdues : %d", vsi.nbr_frame_gap, vsi.average_decoding_time,vsi.nbr_frame_missing);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::TextColored({ 255,255,0,1 }, "Enregistrement");
			ImGui::Separator();
			ImGui::Checkbox("Enregister raw", &record_raw);
			if (record_raw)
			{
				// D�marre l'enregistrement
			}
			if (ImGui::InputText("Dossier enregistrement", record_folder, 100))
			{
			}


			ImGui::End();
		}
	}

	void show_manual_control_window()
	{
		if (show_manual_control) {
			ImGui::Begin("Control manuel",&show_manual_control);

			ImGui::TextColored({ 255,255,0,1 }, "Parametre");

			ImGui::DragFloat("Vitesse X ", &speed_drone.x, 0.01, 0, 1);
			ImGui::DragFloat("Vitesse Y ", &speed_drone.y, 0.01, 0, 1);
			ImGui::DragFloat("Vitesse Z ", &speed_drone.z, 0.01, 0, 1);
			ImGui::DragFloat("Vitesse R ", &speed_drone.r, 0.01, 0, 1);

			ImGui::Separator();
			if(ImGui::Checkbox("Demarrer ",&enable_manual_control))
			{
				if(!enable_manual_control)
				{
					// quand on l'arrete on s'assure que le drone atterisse
					at_client.set_ref(LAND_FLAG);
				}
			}
			ImGui::End();
		}
	}

	void show_widgets()
	{
		show_app_menu_bar();
		show_video_staging_info_window();
		show_nav_data_window();
		show_log_window();
		show_drone_basic_command_window();
		show_color_obj_tracking_window();
		show_metric_overlay(&show_info_overlay);
		show_manual_control_window();
	}

	void manual_control_keyboard()
	{
		static bool is_pressed[8];
		ImGuiIO& io = ImGui::GetIO();
		if (io.KeysDown[GLFW_KEY_TAB])
		{
			at_client.hover();
		}
		if (io.KeysDown[GLFW_KEY_ENTER])
		{
			at_client.set_ref(TAKEOFF_FLAG);
		}
		if (io.KeysDown[GLFW_KEY_BACKSPACE])
		{
			at_client.set_ref(LAND_FLAG);
		}
		if (io.KeysDown[GLFW_KEY_A]) {
			if(!is_pressed[0])
			{
				at_client.setProgressiveFlag(PROGRESSIVE);
				at_client.setSpeedX(LEFT, speed_drone.x);
				is_pressed[0] = true;
			}
		} else
		{
			if(is_pressed[0])
			{
				at_client.setSpeedX(NONE_X, 0.0f);
				is_pressed[0] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_W]) {
			if(!is_pressed[1])
			{
				at_client.setProgressiveFlag(PROGRESSIVE);
				at_client.setSpeedZ(FORWARD, speed_drone.z);
				is_pressed[1] = true;
			}
		} else
		{
			if(is_pressed[1])
			{
				at_client.setSpeedZ(NONE_Z, 0.0f);
				is_pressed[1] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_S]) {
			if(!is_pressed[2])
			{
				at_client.setProgressiveFlag(PROGRESSIVE);
				at_client.setSpeedZ(BACKWARD, speed_drone.z);
				is_pressed[2] = true;
			}
		} else
		{
			if(is_pressed[2])
			{
				at_client.setSpeedZ(NONE_Z, 0.0f);
				is_pressed[2] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_D]) {
			if(!is_pressed[3])
			{
				at_client.setProgressiveFlag(PROGRESSIVE);
				at_client.setSpeedX(RIGHT, speed_drone.x);
				is_pressed[3] = true;
			}
		} else
		{
			if(is_pressed[3])
			{
				at_client.setSpeedX(NONE_X, 0.0f);
				is_pressed[3] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_Q])
		{
			if(!is_pressed[4])
			{
				at_client.setProgressiveFlag(COMBINED_YAW);
				at_client.setSpeedR(LEFT, speed_drone.z);
				is_pressed[4] = true;
			}
		} else
		{
			if(is_pressed[4])
			{
				at_client.setSpeedR(NONE_X, 0.0f);
				is_pressed[4] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_E])
		{
			if (!is_pressed[5])
			{
				at_client.setProgressiveFlag(COMBINED_YAW);
				at_client.setSpeedR(RIGHT, speed_drone.z);
				is_pressed[5] = true;
			}
		} else
		{
			if (is_pressed[5])
			{
				at_client.setSpeedR(NONE_X, 0.0f);
				is_pressed[5] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_O])
		{
			if (!is_pressed[6])
			{
				at_client.setProgressiveFlag(COMBINED_YAW);
				at_client.setSpeedY(LOWER, speed_drone.y);
				is_pressed[5] = true;
			}
		}
		else
		{
			if (is_pressed[6])
			{
				at_client.setSpeedY(NONE_Y, 0.0f);
				is_pressed[6] = false;
			}
		}
		if (io.KeysDown[GLFW_KEY_P])
		{
			if (!is_pressed[7])
			{
				at_client.setProgressiveFlag(COMBINED_YAW);
				at_client.setSpeedY(HIGHER, speed_drone.y);
				is_pressed[7] = true;
			}
		}
		else
		{
			if (is_pressed[7])
			{
				at_client.setSpeedY(NONE_Y, 0.0f);
				is_pressed[7] = false;
			}
		}
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

		while (!glfwWindowShouldClose(window) && run_main_loop)
		{
			m = mat_queue.pop2_wait(10ms,&has_image);
			nd = nd_client.get_last_nd_demo();
			if (has_image) {
				last_mat = m;
				player.setPixels(last_mat);
				if(enable_tracking_video)
				{
					obj_tracker.addImage(last_mat);
				}
			}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			if (enable_tracking_video) {
				//int v = get_image_noise_level(last_mat);
				static bool		is_gap_over;
				cv::Mat b = obj_tracker.getBestThreshOutput(is_gap_over);
				if(is_gap_over && !b.empty())
				{
					if(obj_tracker.tryFoundObject(b)) {
						auto i = obj_tracker.getLastObjectInfo();
					}
					cv::cvtColor(b, b, cv::COLOR_GRAY2BGR);
					player.setPixels2(b);
				} 
			}

			player.draw();


			show_widgets();
			
			if (enable_manual_control)
				manual_control_keyboard();
			
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

		glfwTerminate();
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


		if(!player.setup(640,360,1600,900))
		{
			AR_LOG_ERROR(0, "Echec de la configuration du lecteur video");
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