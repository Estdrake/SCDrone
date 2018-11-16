#ifndef _THREAD_H_
#define _THREAD_H_

#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <chrono>
#include <future>
#include <condition_variable>
#include <qstring.h>
#include "video_common.h"
#include <iostream>

using namespace std;
using namespace std::chrono;


enum ar_drone_workers {
	VIDEO_WORKER,
	NAVDATA_WORKER,
	AT_WORKER,
	CONFIG_WORKER
};

class Runnable {
	std::promise<void> exitSignal;
	std::future<void> futureObject;

public:
	Runnable() :futureObject(exitSignal.get_future()) {}
	Runnable(Runnable && obj) : exitSignal(std::move(obj.exitSignal)), futureObject(std::move(obj.futureObject)) {
		std::cout << "Move Constructor called" << std::endl;
	}
	Runnable & operator=(Runnable && obj) {
		std::cout << "Move assignment is called" << std::endl;
		exitSignal = std::move(obj.exitSignal);
		futureObject = std::move(obj.futureObject);
		return *this;
	}

	// les classes enfants doivent donné une définition
	virtual void run_service() = 0;

	// La fonction thread a executer
	std::thread start() {
		return std::thread([this] { this->run_service(); });
	}

	bool stopRequested() const
	{
		return !(futureObject.wait_for(0ms) == std::future_status::timeout);
	}

	void stop() {
		exitSignal.set_value();
	}

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

	dataType pop2() {
		std::unique_lock<std::mutex> lk(mutex);
		cv.wait(lk, [this] { return !queue.empty(); }); // Attend que la queue ne soit plus vide et utiliser
		dataType f = queue.front();
		queue.pop();
		return f;
	}

	dataType pop_wait(std::chrono::milliseconds timeout,bool* has_data)
	{
		std::unique_lock<std::mutex> lk(mutex);
		if(!cv.wait_for(lk, timeout, [this] { return !queue.empty(); }))
		{
			*has_data = false;
			return dataType();
		}
		*has_data = true;
		dataType f = queue.front();
		queue.pop();
		return f;
	}

	dataType* pop_all_wait(std::chrono::milliseconds timeout,int* nbrElement)
	{
		std::unique_lock<std::mutex> lk(mutex);
		if (!cv.wait_for(lk, timeout, [this] { return !queue.empty(); }))
		{
			*nbrElement = 0;
			return nullptr;
		}
		*nbrElement = queue.size();
		dataType* d = new dataType[*nbrElement];
		for(int i =0; i<*nbrElement;i++)
		{
			d[i] = queue.front();
			queue.pop();
		}
		return d;

	}

	void push(dataType const& data) {
		forceExit.store(false);
		std::unique_lock<std::mutex> lk(mutex);
		queue.push(data);
		lk.unlock();
		cv.notify_one();
	}

	void empty()
	{
		std::unique_lock<std::mutex> lk(mutex);
		for(int i = 0; i < queue.size(); i++)
		{
			queue.pop();
		}
		lk.unlock();
	}

	bool isEmpty() {
		std::unique_lock<std::mutex> lk(mutex);
		return queue.empty();
	}

};


#endif // _THREAD_H_