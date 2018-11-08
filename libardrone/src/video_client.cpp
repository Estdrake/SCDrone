#include "config.h"
#include "video_client.h"

#include <thread>


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


VideoClient::VideoClient() : isConected(false) {
	// Démarre la thread de video
}

VideoClient::~VideoClient() {

}

std::thread VideoClient::spawn() {
	return std::thread([this] { this->run_service(); });
}


void VideoClient::run_service() {
	QByteArray readBuffer;

	VideoFrame vf;
	vf.Header.frame_number = 0;
	vf.Got = 0;

	socket = new QTcpSocket(this);

	socket->connectToHost(WIFI_ARDRONE_IP, VIDEO_PORT);
	if (socket->waitForConnected(3000)) {
		for (;;) {
			if (socket->waitForReadyRead()) {
				readBuffer = socket->readAll();
				// Regarde si le buffer commence par PaVE se qui indique qu'on na une nouvelle trame
				if (getOffsetMagicWord(readBuffer)) {
					// Copie le header de la frame dans la structure
					memcpy(&vf.Header, readBuffer, sizeof(video_encapsulation_t));
					// Devrait peux etre faire une validation ici

					vf.Data = new unsigned char[vf.Header.payload_size];
					int length = readBuffer.length();
					int sizeData = length - vf.Header.header_size;
					int missing = vf.Header.payload_size - sizeData;

					memcpy(vf.Data, readBuffer.data() + vf.Header.header_size, sizeData);

					if (missing == 0) {
						// Notre VideoFrame est complete l'envoie a la queue 

					}
				}
			}
			else {
				qDebug() << "Fail to get data within 3 second";
			}
		}
	}
	else {
		// Fail de la connection doit envoyer un signal a quelqun ou restart plusieurs fois en boucle
		qDebug() << "Fail to connect to video port of drone";
	}
}
