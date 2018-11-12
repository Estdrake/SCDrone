#ifndef STAGING_TEST_H
#define STAGING_TEST_H

#include "video_common.h"
#include "video_staging.h"


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/common.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

inline void execute_staging_test(const fs::path& folder, int nbr_trame = 40887)
{
	VFQueue queue;
	VideoStaging vs(&queue);
	VideoClient vc(&queue);
	if(vs.init() > 0)
	{
		return;
	}
	std::thread st = vs.start();
	std::thread vt = vc.start();

	vt.join();
	st.join();
	fs::path file = std::to_string(nbr_trame) + ".bin";
	file = folder / file;
	cout << "Got frame on " << file.string() << endl;
	ifstream ifs(file, ofstream::binary);

	// Get la longeur du fichier pour savoir le buffer a prendre
	ifs.seekg(0, ios::end);
	int length = ifs.tellg();
	ifs.seekg(0, ios::beg);

	if (length == 0) {
		std::cerr << "No data in file " << file << std::endl;
		return;
	}

	char* buffer = new char[length];

	// Cast le buffer
	ifs.read(buffer, length);
}
#endif
