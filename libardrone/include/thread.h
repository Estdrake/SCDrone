#ifndef _THREAD_H_
#define _THREAD_H_

#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <chrono>
#include <condition_variable>
#include <qstring.h>
#include "video_common.h"
#include <iostream>


enum ar_drone_workers {
	VIDEO_WORKER,
	NAVDATA_WORKER,
	AT_WORKER,
	CONFIG_WORKER
};

template<typename dataType>
class ConcurrentQueue {
private:
	std::queue<dataType>	queue;
	std::mutex				mutex;
	
	std::condition_variable cv;

	std::atomic<bool>		forceExit = false;

public:

	dataType& pop()
	{
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait(lk, [this] { return !queue.empty(); }); // Attend que la queue ne soit plus vide et utiliser
		dataType& f = queue.front();
		queue.pop();
		return f;
	}

	void push(dataType const& data) {
		forceExit.store(false);
		std::unique_lock<std::mutex> lk(mutex);
		queue.push(data);
		lk.unlock();
		cv.notify_one();
	}

	bool isEmpty() {
		std::unique_lock<std::mutex> lk(mutex);
		return queue.empty();
	}

};


#endif // _THREAD_H_