#include "mvlar.h"

#include <navdata_client.h>
#include <video_client.h>

#include <thread.h>
#include <fstream>




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

void printVideoStreamDump(video_encapsulation_t* PaVE) {
	printf("Signature : \"%c%c%c%c\" [0x%02x][0x%02x][0x%02x][0x%02x]\n", PaVE->signature[0], PaVE->signature[1],
		PaVE->signature[2], PaVE->signature[3], PaVE->signature[0], PaVE->signature[1], PaVE->signature[2], PaVE->signature[3]);
	printf("Frame Type / Number : %s : %d : slice %d/%d\n",
		(PaVE->frame_type == FRAME_TYPE_P_FRAME) ? "P-Frame" : ((PaVE->frame_type == FRAME_TYPE_I_FRAME) ? "I-Frame" : "IDR-Frame"),
		PaVE->frame_number,
		PaVE->slice_index + 1,
		PaVE->total_slices);
	printf("Codec : %s\n", (PaVE->video_codec == CODEC_MPEG4_VISUAL) ? "MP4" : ((PaVE->video_codec == CODEC_MPEG4_AVC) ? "H264" : "Unknown"));
	printf("StreamID : %d \n", PaVE->stream_id);
	printf("Encoded dims : %d x %d\n", PaVE->encoded_stream_width, PaVE->encoded_stream_height);
	printf("Display dims : %d x %d\n", PaVE->display_width, PaVE->display_height);
	printf("Header size  : %d\n", PaVE->header_size);
	printf("Payload size : %d\n", PaVE->payload_size);
}

void parseVideoStreamDump(const char* file) {
	int length;
	char* buffer;

	ifstream ifs(file, ofstream::binary);
	// Get la longeur du fichier pour savoir le buffer a prendre
	ifs.seekg(0, ios::end);
	length = ifs.tellg();
	ifs.seekg(0, ios::beg);

	if (length == 0) {
		BOOST_LOG_TRIVIAL(error) << "No data in file " << file;
	}

	buffer = new char[length];

	BOOST_LOG_TRIVIAL(info) << "File " << file << " length is " << length;

	 ifs.read(buffer, length);

	 if (!ifs) {
		 BOOST_LOG_TRIVIAL(error) << "Error only " << ifs.gcount() << " could be read";
		 return;
	 }

	 video_encapsulation_t ve;
	 memcpy_s(&ve, sizeof(video_encapsulation_t), buffer, sizeof(video_encapsulation_t));
	 printVideoStreamDump(&ve);

	 BOOST_LOG_TRIVIAL(info) << "Payload size is " << ve.payload_size;

	 ifs.close();
	 delete[] buffer;
}

int main()
{
	cout << "Hello CMake." << endl;

	//recordVideoStreamBuffer(8096,4,"data.bin");
	parseVideoStreamDump("D:\\l\\data.bin");
	cin.get();
	return 0;
}
