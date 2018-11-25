#ifndef _THREAD_H_
#define _THREAD_H_

#include <thread>
#include <chrono>
#include "thread_obj.h"

using namespace std;
using namespace std::chrono;


enum ar_drone_workers {
	VIDEO_WORKER,
	NAVDATA_WORKER,
	AT_WORKER,
	CONFIG_WORKER
};



#endif // _THREAD_H_