#include "config.h"
#include "video_client.h"
#include "logger.h"
#include <thread>


void print_video_stream_dump(video_encapsulation_t* PaVE) {
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


VideoClient::VideoClient(ConcurrentQueue<VideoFrame>* queue)
{
	this->socket = new QTcpSocket(this);
	socket->connectToHost(WIFI_ARDRONE_IP, VIDEO_PORT);
	this->queue = queue;
}

VideoClient::VideoClient(VideoStaging* vs) {
	this->socket = new QTcpSocket(this);
	socket->connectToHost(WIFI_ARDRONE_IP, VIDEO_PORT);
	this->video_staging = vs;
	direct_staging = true;
	
}

VideoClient::~VideoClient() = default;

void VideoClient::run_service() {

	QByteArray readBuffer;

	VideoFrame vf{};
	vf.Header.frame_number = 0;
	vf.Got = 0;

	state = true;

	if (socket->waitForConnected(3000)) {
		while (stopRequested() == false) {
			if (socket->waitForReadyRead()) {
				if (socket->bytesAvailable() > 0) {
						readBuffer = socket->readAll();
						if (readBuffer.length() < socket->bytesAvailable()) {
							
							continue;
						}
				}
				else {
					continue;
				}
				int length = readBuffer.length();
				bool endFrame = false;
				// Regarde si le buffer commence par PaVE se qui indique qu'on na une nouvelle trame
				if (getOffsetMagicWord(readBuffer)) {
					// Copie le header de la frame dans la structure
					memcpy(&vf.Header, readBuffer, sizeof(video_encapsulation_t));
#ifdef DEBUG_VIDEO_CLIENT
					print_video_stream_dump(&vf.Header);
#endif
					// Devrait peux etre faire une validation ici
					vf.Data = new unsigned char[vf.Header.payload_size];
					vf.Got = length - vf.Header.header_size;
					int missing = vf.Header.payload_size - vf.Got;
					if (missing < 0) {
						vf.Got = vf.Header.payload_size;
						missing = 0;
					}

					memcpy(vf.Data, readBuffer.data() + vf.Header.header_size, vf.Got);

					if (missing == 0) {
						endFrame = true;
					}
#ifdef DEBUG_VIDEO_CLIENT
					qDebug() << "Missing " << missing << " bytes from frame " << vf.Header.frame_number;
#endif
				} else
				{
					// Il doit s'agir du reste d'une trame manquante
					// valide qu'on n'a deja des data
					if(vf.Got == 0)
					{
#ifdef DEBUG_VIDEO_CLIENT
						qDebug() << "Receive data but no previous PaVE";
#endif
						// mauvais data on n'a jamais recu de PaVE
						continue;
					}
#ifdef DEBUG_VIDEO_CLIENT
					qDebug() << "Receive extra data to fill buffer " << length;
#endif
					if(length + vf.Got >= vf.Header.payload_size)
					{
						// ya trop de data pour ce qu'on n'a besoin
						length = vf.Header.payload_size - vf.Got;
						endFrame = true;
					}
					memcpy(vf.Data + vf.Got, readBuffer.data(), length);
					vf.Got += length;
				}

				if (endFrame)
				{
#ifdef DEBUG_VIDEO_CLIENT
					qDebug() << "Pushing frame " << vf.Header.frame_number;
#endif
					if (direct_staging)
						video_staging->onNewVideoFrame(vf);
					else
						queue->push(vf);
					vf.Got = 0;
				}
			}
			else {
				//stop();
			}
		}
		AR_LOG_INFO(1, "Stopping video client\n");
		qDebug() << "Stopping video client";
	}
	else {
		AR_LOG_INFO(1, "Fail to connect to video port of drone\n");
		qDebug() << "Fail to connect to video port of drone";
	}

	state = false;
}
