#ifndef _VIDEO_CLIENT_H_
#define _VIDEO_CLIENT_H_

#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "video_common.h"


#define FRAME_BUFFER_SIZE 0x1000000;
#define NETWORK_STREAM_SIZE 0x10000;

using boost::asio::ip::tcp;

class VideoClient {
	
public:
	VideoClient(boost::asio::io_service& client);
	~VideoClient();

	boost::thread* start_service();

private:
	// Variable du socket
	tcp::socket		socket;
	tcp::endpoint	remote;

	// Variable sur le flux video
	bool	isStopped;
	
	int		offset;
	int		frameStart;
	int		frameEnd;

	std::vector<uint8_t>		packetData;
	std::vector<uint8_t>		buffer;
	std::unique_ptr<ARStream>	arStream;

	bool hasVideoPacket;


	void run_service();

	VideoClient(VideoClient&);
};


#endif