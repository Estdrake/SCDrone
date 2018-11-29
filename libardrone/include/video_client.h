#ifndef _VIDEO_CLIENT_H_
#define _VIDEO_CLIENT_H_

#include "video_common.h"
#include "video_staging.h"
#include "thread.h"

#include <thread>

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

class VideoClient : QObject, public Runnable {
	
public:
	VideoClient(ConcurrentQueue<VideoFrame>* queue);
	VideoClient(VideoStaging* vs);
	~VideoClient();

	void run_service();
private:
	bool								direct_staging = false;
	VideoStaging*						video_staging;
	ConcurrentQueue<VideoFrame>*		queue;
	QTcpSocket* socket;

	QByteArray	bufferFrame;
	
	// Indique si on n'est courrament connecter async avec le drone
	bool		isConected;
};

void print_video_stream_dump(video_encapsulation_t* PaVE);


#endif