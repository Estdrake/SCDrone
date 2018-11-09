#include "headers.h"

#include <video_client.h>



class TestTcp : public QObject {
	//Q_OBJECT

private:
	QTcpSocket*	socket;
public:
	TestTcp(QObject *parent = 0) {

	}

	void Connect(int nbrIteration, fs::path folder) {
		QByteArray ba;
		socket = new QTcpSocket(this);

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

inline void parseVideoStreamDump(fs::path folder, int nbrFile) {

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
					std::cout << "Frame is rebuilt stream index is now " << streamIndex << std::endl;
				}
			}
		}
		else {
			memcpy(&ve, buffer, sizeof(video_encapsulation_t));
			printVideoStreamDump(&ve);
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

				memcpy(stream + streamIndex, vfBuffer, ve.payload_size);
				streamIndex += ve.payload_size;
				std::cout << "Frame is rebuilt stream index is now " << streamIndex << std::endl;

			}
		}
		ifs.close();
		delete[] buffer;
	}

	fs::path f = "stream.bin";
	// Écrit le stream vers un fichier
	ofstream of(folder / f, ofstream::binary | ofstream::out);
	of.seekp(0);
	of.write((char*)stream, streamIndex);
	of.close();
}


inline void execute_video_test(fs::path folder, int nbrTrame, bool onlyParse = false ,int bufferSize = STREAM_SIZE) {

	stream = new unsigned char[bufferSize];
	if (!onlyParse) {
		TestTcp t;
		t.Connect(nbrTrame, folder);
	}
	parseVideoStreamDump(folder, nbrTrame);
	delete[] stream;
}