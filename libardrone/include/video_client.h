#ifndef _VIDEO_CLIENT_H_
#define _VIDEO_CLIENT_H_
#include "video_common.h"

#include <thread>

#include <QtCore/QObject>
#include <QtCore/QDebug>
#include <QtNetwork/QTcpSocket>

class VideoClient : QObject {
	
public:
	VideoClient();
	~VideoClient();

	std::thread spawn();
	
private:

	QTcpSocket* socket;

	QByteArray	bufferFrame;
	
	// Indique si on n'est courrament connecter async avec le drone
	bool	isConected;

	void run_service();
	
};

void printVideoStreamDump(video_encapsulation_t* PaVE);


#endif