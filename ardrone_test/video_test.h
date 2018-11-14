#include "headers.h"

#include <video_client.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>

class TestTcp : public QObject {
	//Q_OBJECT

private:
	QTcpSocket*	socket;
public:
	TestTcp(QObject *parent = 0) {
		socket = new QTcpSocket(this);
	}

	void Connect(int nbrIteration,const fs::path& folder) {
		QByteArray ba;

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
					fileName = folder / fileName;
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

#define STREAM_SIZE 1024000

int streamIndex = 0;
unsigned char* stream;

inline void parseVideoStreamDump(fs::path folder, int nbrFile, bool to_stream = true) {

	video_encapsulation_t ve;
	ve.frame_number = 0;

	int currentSize;
	unsigned char* vfBuffer;

	int length;
	char* buffer;


	if (!fs::exists(folder)) {
		std::cerr << "Folder dont exists " << folder << std::endl;
		return;
	}

	int nbr = 0;

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
					if (to_stream) {
						memcpy(stream + streamIndex, vfBuffer, ve.payload_size);
						streamIndex += ve.payload_size;
						std::cout << "Frame is rebuilt stream index is now " << streamIndex << std::endl;
					} else
					{
						fs::path f = std::to_string(nbr) + ".raw";
						nbr += 1;
						f = folder / f;
						// Écrit le stream vers un fichier
						ofstream of(f, ofstream::binary | ofstream::out);
						of.seekp(0);
						unsigned char* bb = new unsigned char[ve.payload_size + ve.header_size];
						memcpy(bb, &ve, sizeof(video_encapsulation_t));
						memcpy(bb + sizeof(video_encapsulation_t), vfBuffer, ve.payload_size);
						of.write((char*)bb, ve.payload_size + ve.header_size);
						of.close();
					}
				}
			}
		}
		else {
			memcpy(&ve, buffer, sizeof(video_encapsulation_t));
			print_video_stream_dump(&ve);
			std::cout << "Payload size is " << ve.payload_size << std::endl;
			// Crée mon buffer pour recevoir le payload
			vfBuffer = new unsigned char[ve.payload_size];
			// Copie les données qui sont apres le header
			int sizeData = length - ve.header_size;
			std::cout << "Header size is " << ve.header_size << " and sizeof(video_encapsulation_t) is " << sizeof(video_encapsulation_t) << " Size data after header : " << sizeData << std::endl;
			std::cout << "Missing " << ve.payload_size - sizeData << std::endl;
			if (sizeData > ve.payload_size) {
				sizeData = ve.payload_size;
			}
			currentSize = sizeData;
			memcpy(vfBuffer, buffer + ve.header_size, sizeData);

			if (ve.payload_size - sizeData == 0) {
				std::cout << "Frame is rebuilt stream index is now " << streamIndex << std::endl;
				if (to_stream) {
					memcpy(stream + streamIndex, vfBuffer, ve.payload_size);
					streamIndex += ve.payload_size;
				}
				else
				{
					fs::path f = std::to_string(nbr) + ".raw";
					nbr += 1;
					f = folder / f;
					// Écrit le stream vers un fichier
					ofstream of(f, ofstream::binary | ofstream::out);
					of.seekp(0);
					// Ma refaire un buffer pour le header + le full payload
					unsigned char* bb = new unsigned char[ve.payload_size + ve.header_size];
					memcpy(bb, &ve, sizeof(video_encapsulation_t));
					memcpy(bb + ve.header_size, vfBuffer, ve.payload_size);
					of.write((char*)bb, ve.payload_size+ve.header_size);
					of.close();
				}
			}
		}
		ifs.close();
		delete[] buffer;
	}
	if (to_stream) {
		fs::path f = "stream.bin";
		// Écrit le stream vers un fichier
		ofstream of(folder / f, ofstream::binary | ofstream::out);
		of.seekp(0);
		of.write((char*)stream, streamIndex);
		of.close();
	}
}

inline void parse_video_packet_raw_file(fs::path folder,int nbr_trame)
{
	parseVideoStreamDump(folder, nbr_trame, false);
}


inline void execute_video_test(fs::path folder, int nbrTrame, bool onlyParse = false ,int bufferSize = STREAM_SIZE) {

	//parseVideoStreamDump(folder, 4, false);

	VFQueue		queue;
	VideoClient client(&queue);

	VideoFrame	vf;


	thread t = client.start();

	fs::path f = folder / "stream.264";
	thread t2 = std::thread([&] {
		ofstream of(f, ofstream::binary | ofstream::out);
		int length = 0;
		std::cout << "Starting to wait for video frame" << std::endl;
		for (int i = 0; i < 1000; i++)
		{

			vf = queue.pop();
			std::cout << "Got frame id " << vf.Header.frame_number << std::endl;
			of.seekp(length);
			of.write((char*)vf.Data, vf.Header.payload_size);
			length += vf.Header.payload_size;
			delete[] vf.Data;
		}

		of.close();
	});
	t2.join();
	//t.join();
}