#ifndef _THREAD_H_
#define _THREAD_H_


#include <queue>
#include <list>


enum ar_drone_workers {
	VIDEO_WORKER,
	NAVDATA_WORKER,
	AT_WORKER,
	CONFIG_WORKER
};

#endif // _THREAD_H_