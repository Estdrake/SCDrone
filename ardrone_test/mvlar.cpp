#include "mvlar.h"
#include <navdata_client.h>
#include <thread.h>

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;

	ARDroneWorkers workers(4);
	
	NavDataClient client(workers.getIoService());
	client.run_service();
	
	return 0;
}
