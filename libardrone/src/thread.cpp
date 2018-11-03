#include "thread.h"

// CONSTRUCTEUR

ARDroneWorkers::ARDroneWorkers(int pool_size) : _work_io_service(_io_service), _thread_free(pool_size) {
	if (pool_size > 0) {
		// Si on donne pas de grosseur pool de thread on utilise le double du nombre de coeur du cpu
		pool_size = boost::thread::hardware_concurrency() * 2;
	}
}

ARDroneWorkers::~ARDroneWorkers() {
	_io_service.stop();
	try {
		_thread_group.join_all();
	} catch(const boost::system::system_error& ex) {

	}
}

// FONCTION PUBLIC

void ARDroneWorkers::run_service(boost::function<void()> f) {
	_thread_group.create_thread(f);
}


// FONCTION PRIVER
