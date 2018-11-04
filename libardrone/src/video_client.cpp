#include "config.h"
#include "video_client.h"

#include <boost/log/trivial.hpp>

VideoClient::VideoClient(boost::asio::io_service& ios) : socket(ios) {
	// Configure le remote endpoit du serveur TCP
	remote = tcp::endpoint(boost::asio::ip::address::from_string(WIFI_ARDRONE_IP), VIDEO_PORT);

}

VideoClient::~VideoClient() {

}

boost::thread* VideoClient::start_service() {
	return new boost::thread(boost::bind(&VideoClient::run_service, this));
}

void VideoClient::run_service() {
	BOOST_LOG_TRIVIAL(info) << "Starting video worker on " << remote;
	isStopped = false;
	try {
		boost::system::error_code ex = boost::asio::error::host_not_found;
		socket.connect(remote, ex);
		if (ex) {
			// A l'ideal on enverrait un signal a la main thread dire qu'on n'a une erreur
			isStopped = true;
			throw boost::system::system_error(ex);
		}
		for (;;) {
			size_t len = socket.read_some(boost::asio::buffer(buffer), ex);
			BOOST_LOG_TRIVIAL(info) << "Length of data read " << len;

			// Regarde si le premier buffer du stream est plein
			

			
			
		}
	}
	catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << e.what();
	}
}
