#include "drone_client.h"

DroneClient::DroneClient() :
	video_staging(&vf_queue,&mat_queue),
	video_client(&vf_queue),
	at_client(&at_queue),
	control(&at_queue)
{
}

int DroneClient::Start()
{
	if(int ret = init(); ret > 0)
		return ret;
	mainLoop();
	return stop();
}

int DroneClient::init()
{
	if (int ret = video_staging.init(); ret > 0)
	{
		std::cerr << "Error during video staging init " << ret << endl;
		return 1;
	}
	at_thread = at_client.start();
	vc_thread = video_client.start();
	vs_thread = video_staging.start();
	std::cout << "All thread are started" << std::endl;
	return 0;
}

int DroneClient::stop()
{
	at_client.set_ref(LAND_FLAG);
	std::this_thread::sleep_for(100ms);

	std::cout << "Stopping all thread and joining theme" << std::endl;
	video_staging.stop();
	video_client.stop();
	at_client.stop();

	vs_thread.join();
	vc_thread.join();
	at_thread.join();
	return 0;
}
