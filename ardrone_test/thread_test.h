#ifndef THREAD_TEST_H
#define THREAD_TEST_H

#include "video_common.h"
#include "thread.h"
#include <iostream>


inline void thread_out(ConcurrentQueue<VideoFrame>* cq)
{
	VideoFrame vf {};
	bool has_data = false;
	int nodata = 0;
	for(;;)
	{
		vf = cq->pop_wait(500ms,&has_data);
		if(!has_data)
		{
			std::cout << "timeout waiting" << endl;
			nodata++;
			if(nodata == 2)
			{
				std::cout << "No data in 1s exiting" << std::endl;
				break;
			}
			continue;
		}
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
	std::this_thread::sleep_for(800ms);
	std::thread t_in = std::thread(thread_int, &cq);
	t_in.join();
	t_out.join();
}




#endif