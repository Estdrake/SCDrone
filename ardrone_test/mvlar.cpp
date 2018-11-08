#include "mvlar.h"

#include <video_client.h>

#include <thread.h>
#include <fstream>


using namespace std;


#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudacodec.hpp>


namespace fs = std::filesystem;


class TestTcp : public QObject {
	//Q_OBJECT

private:
	QTcpSocket*	socket;
public:
	TestTcp(QObject *parent = 0) {

	}

	void Connect(int nbrIteration,std::string folderName) {
		QByteArray ba;
		socket = new QTcpSocket(this);

		fs::path folder = folderName;
		if (!fs::exists(folder)) {
			qDebug() << "Dossier inexistant " << folder.c_str();
			return;
		}

		socket->connectToHost("192.168.1.1", 5555);
		if (socket->waitForConnected(3000)) {
			qDebug() << "Connecter a l'host";
			for (int i = 0; i < nbrIteration;) {
				if (socket->waitForReadyRead()) {
					qDebug() << socket->bytesAvailable();
					ba = socket->readAll();
					fs::path fileName = std::to_string(i) + ".bin";
					fileName = folderName / fileName;
					qDebug() << "Length of data receive " << ba.length() << " writting to " << fileName.c_str();
					ofstream of(fileName, ofstream::binary | ofstream::out);
					of.seekp(0);
					of.write(ba.data(), ba.length());
					of.close();
					i++;
				}
			}
		}
		else {
			qDebug() << "Could not connect to host";
		}
		socket->close();
	}

};


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


int streamIndex = 0;
unsigned char* stream = new unsigned char[1024000];

void parseVideoStreamDump(const char* folderName, int nbrFile) {

	video_encapsulation_t ve;
	ve.frame_number = 0;

	int currentSize;
	unsigned char* vfBuffer;

	int length;
	char* buffer;

	fs::path folder = folderName;

	if (!fs::exists(folder)) {
		std::cerr << "Folder dont exists " << folder << std::endl;
		return;
	}

	for (int i = 0; i < nbrFile; i++) {
		fs::path fileName = std::to_string(i) + ".bin";
		fs::path file = folder / fileName;
		ifstream ifs(file, ofstream::binary);

		// Get la longeur du fichier pour savoir le buffer a prendre
		ifs.seekg(0, ios::end);
		length = ifs.tellg();
		ifs.seekg(0, ios::beg);

		if (length == 0) {
			std::cerr << "No data in file " << file << std::endl;
		}

		buffer = new char[length];

		std::cout << "File " << file << " length is " << length << std::endl;

		ifs.read(buffer, length);

		if (!ifs) {
			std::cerr << "Error only " << ifs.gcount() << " could be read" << std::endl;
			return;
		}

		// Trouve l'offset de PaVE

		if (!getOffsetMagicWord(buffer)) {
			if (ve.frame_number != 0) {
				bool endFrame = false;
				std::cout << "Got extra " << length << std::endl;
				// get le nombre de byte qu'on veut prendre ce cette array
				int nbrByte = length;
				if (length + currentSize >= ve.payload_size) {
					nbrByte = ve.payload_size - currentSize;
					endFrame = true;
				}
				memcpy(vfBuffer + currentSize, buffer, nbrByte);
				if (endFrame) {
					memcpy(stream + streamIndex, vfBuffer, ve.payload_size);
					streamIndex += ve.payload_size;
					std::cout << "Frame is rebuilt stream index is now " << streamIndex  << std::endl;
					return;
				}
			}
		}
		else {
			memcpy_s(&ve, sizeof(video_encapsulation_t), buffer, sizeof(video_encapsulation_t));
			printVideoStreamDump(&ve);

			std::cout << "Payload size is " << ve.payload_size << std::endl;
			// Crée mon buffer pour recevoir le payload
			vfBuffer = new unsigned char[ve.payload_size];
			// Copie les données qui sont apres le header
			int sizeData = length - ve.header_size;
			std::cout << "Header size is " << ve.header_size << " and sizeof(video_encapsulation_t) is " << sizeof(video_encapsulation_t) << " Size data after header : " << sizeData << std::endl;
			std::cout << "Missing " << ve.payload_size - sizeData << std::endl;
			currentSize = sizeData;
			memcpy(vfBuffer, buffer+ve.header_size, sizeData);

			if (ve.payload_size - sizeData == 0) {
				memcpy(stream + streamIndex, vfBuffer, ve.payload_size);
				streamIndex += ve.payload_size;
				std::cout << "Frame is rebuilt stream index is now " << streamIndex << std::endl;
			}
		}
		ifs.close();
		delete[] buffer;
	}

	fs::path f = "stream.264";
	// Écrit le stream vers un fichier
	ofstream of(folder / f, ofstream::binary | ofstream::out);
	of.seekp(0);
	of.write((char*)stream, streamIndex);
	of.close();
	delete[] stream;
}

class ARDroneRawVideoSource : public cv::cudacodec::RawVideoSource {

public:
	
	ARDroneRawVideoSource::ARDroneRawVideoSource() {

	}

	ARDroneRawVideoSource::~ARDroneRawVideoSource() {

	}
	cv::cudacodec::FormatInfo ARDroneRawVideoSource::format() const {
		cv::cudacodec::FormatInfo fi;
		fi.chromaFormat = cv::cudacodec::YUV420;
		fi.codec = cv::cudacodec::MPEG4;
		fi.width = 360;
		fi.height = 640;
		return fi;
	}
	bool ARDroneRawVideoSource::getNextPacket(unsigned char** data, int* size, bool* endOfFile) {
		*endOfFile = false;
		*size = streamIndex;
		data = &stream;
		return true;
	}
};


extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include <libavutil/avutil.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}


void processStream() {
	// Initiliaze la library ffmpeg avcodec
	av_register_all();
	av_log_set_level(AV_LOG_DEBUG);

}



int main()
{
	cout << "Hello CMake." << endl;

	cv::cuda::printShortCudaDeviceInfo(cv::cuda::getDevice());

	#if defined(HAVE_OPENCV_CUDACODEC)
	cout << "You got it" << endl;
	#endif
	
	//std::cout << cv::getBuildInformation() << std::endl;

	//TestTcp t;
	//t.Connect(150,"D:\\l\\data");
	parseVideoStreamDump("D:\\l\\data",50);

	// Crée notre videoreader avec cuda et le shit envoie tout le temps la meme affaire
	cv::Ptr<cv::cudacodec::RawVideoSource> raw = new ARDroneRawVideoSource();
	try {
		cv::Ptr<cv::cudacodec::VideoReader> vr = cv::cudacodec::createVideoReader(raw);
		cv::cuda::GpuMat frame;
		if (!vr->nextFrame(frame)) {
			std::cerr << "Failed to get next frame" << std::endl;
		}
		else {
			cv::imshow("GPU", frame);
			cv::waitKey(0);
		}

	}
	catch (cv::Exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}


	
	cin.get();
	delete[] stream;
	return 0;
}
