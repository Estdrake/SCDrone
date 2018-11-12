#ifndef THREAD_TEST_H
#define THREAD_TEST_H

#include "video_common.h"
#include "thread.h"
#include <iostream>


inline void thread_out(ConcurrentQueue<VideoFrame>* cq)
{
	VideoFrame vf {};
	for(int i = 0; i < 10;i++)
	{
		vf = cq->pop();
		std::cout << "Receive data " << vf.Got << std::endl;

		std::this_thread::sleep_for(500ms);
	}
	std::cout << "Done receiving" << std::endl;
	
}

inline void thread_int(ConcurrentQueue<VideoFrame>* cq)
{
	VideoFrame vf{};
	for(int i = 0; i < 10;i++)
	{
		vf.Got = 1140;
		vf.Data = new unsigned char[vf.Got];
		cq->push(vf);
		std::this_thread::sleep_for(300ms);
	}
	std::cout << "Done pushing" << std::endl;
}


inline void execute_thread_test()
{
	ConcurrentQueue<VideoFrame> cq;
	std::cout << "Start thread test" << std::endl;
	std::thread t_out = std::thread(thread_out, &cq);
	std::this_thread::sleep_for(1s);
	std::thread t_in = std::thread(thread_int, &cq);
	t_in.join();
	t_out.join();

}




#endif