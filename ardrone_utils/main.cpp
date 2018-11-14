
#include "control.h"
#include <thread>
#include <chrono>

#include <QThread>
#include <QThreadPool>

int main(int argc, char* argv[])
{

	QCoreApplication a(argc, argv);
	ATQueue queue;
	ATClient client(&queue,&a);
	std::thread t = client.start();
	GamepadMonitor monitor(&queue,&a,&client);
	//QThread* thread = new QThread();
	//QObject::connect(thread, &QThread::started, &client, &ATClient::run_service);
	//thread->start();


	a.exec();

	return 0;
}