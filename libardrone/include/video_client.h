#ifndef _VIDEO_CLIENT_H_
#define _VIDEO_CLIENT_H_

#include "video_common.h"
#include "thread.h"

#include <thread>

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

typedef ConcurrentQueue<VideoFrame> VFQueue;

class VideoClient : QObject {
	
public:
	VideoClient(ConcurrentQueue<VideoFrame>* queue);
	~VideoClient();

	std::thread start();

private:
	ConcurrentQueue<VideoFrame>*		queue;
	QTcpSocket* socket;

	QByteArray	bufferFrame;
	
	// Indique si on n'est courrament connecter async avec le drone
	bool		isConected;

	
	void run_service();
};

void print_video_stream_dump(video_encapsulation_t* PaVE);


#endif