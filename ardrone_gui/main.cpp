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

#include "logger.h"

static void glfw_error_callback(int error, const char* description)
{
	std::cerr << "GLFW Error " << error << " : " << description << std::endl;
}

struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset
	bool                ScrollToBottom;

	void    Clear() { Buf.clear(); LineOffsets.clear(); }

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size);
		ScrollToBottom = true;
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}
		if (ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if (copy) ImGui::LogToClipboard();

		if (Filter.IsActive())
		{
			const char* buf_begin = Buf.begin();
			const char* line = buf_begin;
			for (int line_no = 0; line != NULL; line_no++)
			{
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : NULL;
				if (Filter.PassFilter(line, line_end))
					ImGui::TextUnformatted(line, line_end);
				line = line_end && line_end[1] ? line_end + 1 : NULL;
			}
		}
		else
		{
			ImGui::TextUnformatted(Buf.begin());
		}

		if (ScrollToBottom)
			ImGui::SetScrollHere(1.0f);
		ScrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}
};


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

	bool					are_thread_running = true;

	cv::Mat					last_mat;
	cv::Mat					presentation_mat;

	navdata_demo_t			nd;

	float					speedXZ;
	float					speedYR;

	RGBGL_Player			player;
	GLFWwindow*				window;


	bool					show_nd = true;
	bool					show_vs_info = false;
	bool					show_log = false;
	bool					show_basic_cmd_drone = true;

	void show_log_window()
	{
		if(show_log)
		{
			static ExampleAppLog log;

			for(auto v : logger.Consume())
			{
				log.AddLog(v.c_str());
			}
			log.Draw("Logger AR Drone");
		}
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
					std::cout << "Quitting" << std::endl;
				}
				ImGui::EndMenu();
			}

			if(ImGui::BeginMenu("Fenetre"))
			{
				ImGui::Checkbox("Navdata", &show_nd);
				ImGui::Checkbox("Video Staging", &show_vs_info);
				ImGui::Checkbox("Logger", &show_log);
				
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
		
	}

	void show_drone_basic_command_window()
	{
		if(show_basic_cmd_drone)
		{
			ImGui::Begin("Commande de Base");


			ImGui::End();
		}
		
	}

	void show_nav_data_window()
	{
		if (show_nd) {
			static int range_building[1]{ 0 };
			ImGui::Begin("Information Navdata");

			ImGui::TextColored({ 255,255,0,1 }, "Information general");
			ImGui::Text("Pourcentage Batterie : %d", nd.vbat_flying_percentage);
			ImGui::Text("Etat actuelle : %s", nd.ctrl_state & ARDRONE_FLY_MASK ? "FLYING" : "LANDED");
			ImGui::Separator();
			ImGui::TextColored({ 255,255,0,1 }, "Information de vol");

			ImGui::Text("Altitude : %d", nd.altitude);
			ImGui::Text("Phi : %.2f  Theta : %.2f  Psi : %.2f", nd.phi / 1000.0f, nd.theta / 1000.0f, nd.psi / 1000.0f);
			ImGui::Text("Velociter X : %.2f Y : %.2f", nd.vx, nd.vy);

			if (ImGui::Button("Urgence"))
			{
				at_client.set_ref(EMERGENCY_FLAG);
			}
			ImGui::SameLine();
			if (ImGui::Button("Calibrage au sol"))
			{
				at_queue.push(at_format_ftrim());

			}
			ImGui::SameLine();
			if (ImGui::Button("Calibrage en vol"))
			{
				if (at_client.get_ref() == "FLYING") {
					at_queue.push(at_format_calib(0));
				}
				else
				{
					AR_LOG_ERROR(0, "Le drone doit etre en vol\n");
				}
			}

			ImGui::End();
		}
	}

	void show_video_staging_info_window()
	{
		if (show_vs_info) {
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
			ImGui::Text("Interval de %d frames Temps moyens : %g ms Frames perdues : %d", vsi.nbr_frame_gap, vsi.average_decoding_time,vsi.nbr_frame_missing);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::TextColored({ 255,255,0,1 }, "Enregistrement");
			ImGui::Separator();
			ImGui::Checkbox("Enregister raw", &record_raw);
			if (record_raw)
			{
				// Démarre l'enregistrement
			}
			if (ImGui::InputText("Dossier enregistrement", record_folder, 100))
			{
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

			show_widgets();

			presentation_mat = last_mat.clone();

			player.draw();

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