﻿#include "headers.h"

#include <tclap/CmdLine.h>
#include "build_info.h"
#include "thread_test.h"
#include "video_test.h"
#include "cuda_test.h"
#include "staging_test.h"
#include "wifi_test.h"
#include "atcmd_test.h"

using namespace std;




int main(int argv, char ** argc )
{

	bool run_video_test = false, run_cuda_test = false, run_staging_test = false, run_wifi_test = false, run_raw_convert = false, run_thread_test = false, run_atcmd_test = false;
	
	fs::path folder;

	int nbr_trame;

	try {

		TCLAP::CmdLine cmd("ardrone_test",' ',"0.0.1");

		
		TCLAP::ValueArg<std::string> folder_arg("d", "directory", "Directory use to search for wanted data", false, "./", "string");
		TCLAP::ValueArg<int> nbr_trame_arg("n", "number", "Number of trame to user", false, 150, "int");
		cmd.add(folder_arg);
		cmd.add(nbr_trame_arg);

		TCLAP::SwitchArg video_test_arg("v", "video_test", "Run the video test with drone", cmd, false);
		TCLAP::SwitchArg cuda_test_arg("c", "cuda_test", "Run test for cuda", cmd, false);
		TCLAP::SwitchArg staging_test_arg("s", "staging_test", "Run staging test", cmd, false);
		TCLAP::SwitchArg test_wifi_arg("w", "wifi_test", "Run wifi test with drone", cmd, false);
		TCLAP::SwitchArg convert_frame_arg("r", "raw", "Extract the saved frame to raw frame", cmd, false);
		TCLAP::SwitchArg thread_test_arg("t", "thread", "Effectue le test des threads",cmd,false);
		TCLAP::SwitchArg atcmd_test_arg("a", "atcmd", "Effectue le test des at cmd",cmd,false);


		cmd.parse(argv, argc);
		run_video_test = video_test_arg.getValue();
		run_cuda_test = cuda_test_arg.getValue();
		run_staging_test = staging_test_arg.getValue();
		run_wifi_test = test_wifi_arg.getValue();
		run_raw_convert = convert_frame_arg.getValue();
		run_thread_test = thread_test_arg.getValue();
		run_atcmd_test = atcmd_test_arg.getValue();
		folder = folder_arg.getValue();
		nbr_trame = nbr_trame_arg.getValue();

	}
	catch (TCLAP::ArgException &e) {
		std::cerr << "Erreur : " << e.error() << " pour l'argument " << e.argId() << std::endl;
		return 1;
	}

	if (!fs::exists(folder)) {
		cerr << "Folder don't exists " << folder << endl;
		return 1;
	}
	if (run_raw_convert)
		parse_video_packet_raw_file(folder, nbr_trame);
	if (run_video_test)
		execute_video_test(folder,nbr_trame);
	if (run_cuda_test)
		execute_cuda_test(folder);
	if (run_staging_test)
		execute_staging_test(folder,nbr_trame);
	if (run_wifi_test)
		execute_wifi_test();
	if (run_thread_test)
		execute_thread_test();
	if (run_atcmd_test)
		execute_atcmd_test();
		
	return 0;
}
