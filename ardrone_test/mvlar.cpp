#include "mvlar.h"
#include <navdata_client.h>
#include <video_client.h>
#include <thread.h>

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	ARDroneWorkers workers(4);
	VideoClient vc(workers.getIoService());
	boost::thread* t =  vc.start_service();
	t->join();
	
	return 0;
}
