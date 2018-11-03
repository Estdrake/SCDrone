#ifndef _THREAD_H_
#define _THREAD_H_

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <queue>
#include <list>


enum ar_drone_workers {
	VIDEO_WORKER,
	NAVDATA_WORKER,
	AT_WORKER,
	CONFIG_WORKER
};

// Classe qui me permet de generer des nouvelles thread et l'ensemble des threads d�marrer
class ARDroneWorkers {

	private:
		boost::asio::io_service			_io_service;
		boost::asio::io_service::work	_work_io_service;

		boost::thread_group				_thread_group;

		boost::mutex					_mx;
		unsigned short					_thread_free;


	public:
		ARDroneWorkers(int pool_size);
		~ARDroneWorkers();

		void stop_service();
		void run_service(boost::function<void()> f);

		boost::asio::io_service&	getIoService() {
			return _io_service;
		}

	private:

};

// class de base pour nos worker ( thread )
class base_worker {


protected:
	boost::thread		_thread;

protected:
	

};

// locked_queue pour les messages a trait� par nos thread
template<typename _T> class locked_queue
{
private:
    boost::mutex mutex;
    std::queue<_T> queue;
public:
    void push(_T value) 
    {
        boost::mutex::scoped_lock lock(mutex);
        queue.push(value);
    };

    _T pop()
    {
        boost::mutex::scoped_lock lock(mutex);
        _T value;
        std::swap(value,queue.front());
        queue.pop();
        return value;
    };

    bool empty() {
        boost::mutex::scoped_lock lock(mutex);
        return queue.empty();
    }
};

#endif // _THREAD_H_