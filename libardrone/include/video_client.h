#ifndef _VIDEO_CLIENT_H_
#define _VIDEO_CLIENT_H_
#include "video_common.h"

//#define FRAME_BUFFER_SIZE 0x1000000;
//#define NETWORK_STREAM_SIZE 0x10000;


class VideoClient {
	
public:
	VideoClient();
	~VideoClient();

private:


	// Variable sur le flux video
	bool	isStopped;
	
	int		offset;
	int		frameStart;
	int		frameEnd;


	bool hasVideoPacket;


	void run_service();

};


#endif