#include "mvlar.h"

#include <navdata_client.h>
#include <video_client.h>

#include <thread.h>
#include <fstream>

#include "tclap/CmdLine.h"



using namespace std;

// RecordVideoStreamBuffer permet d'enregister les données recu de la transmission pour effectuer des test
// avec un espace entre chaque trame
void recordVideoStreamBuffer(int bufferSize, int nbrTrame, const char* fileName) {
	boost::asio::io_service ios;
	tcp::socket socket(ios);
	tcp::endpoint remote = tcp::endpoint(boost::asio::ip::address::from_string(WIFI_ARDRONE_IP), VIDEO_PORT);
	std::vector<uint8_t> buffer(bufferSize);

	ofstream of(fileName, ofstream::binary | ofstream::out);

	size_t seekOutFile = 0;

	try {
		boost::system::error_code ex = boost::asio::error::host_not_found;
		socket.connect(remote, ex);
		if (ex) {
			throw boost::system::system_error(ex);
		}
		for (int i = 0; i < nbrTrame;i++) {
				size_t len = socket.read_some(boost::asio::buffer(buffer), ex);
				BOOST_LOG_TRIVIAL(info) << "Length of data read " << len;
				of.seekp(seekOutFile);
				of.write((char*)buffer.data(),len);
				seekOutFile += len + 100;
				buffer.clear();
		}
		of.close();
		socket.close();
	}
	catch (const std::exception& e) {
		BOOST_LOG_TRIVIAL(error) << e.what();
	}
}


void parseVideoStreamDump(const char* file) {

}

int main()
{
	cout << "Hello CMake." << endl;

	recordVideoStreamBuffer(8096,4,"data.bin");

	return 0;
}
