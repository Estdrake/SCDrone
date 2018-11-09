#include "headers.h"


#if defined(HAVE_OPENCV_CUDACODEC)

#include <opencv2/cudacodec.hpp>


inline void read_video_dump_with_cuda(const char* stream_file) {

	cv::VideoCapture cap(stream_file);
	if (!cap.isOpened()) {
		std::cerr << "Failed to read my dump";
		return;
	}

	cv::Mat t;
	cv::cuda::GpuMat gt;
	cap >> t;
	if (t.empty()) {
		std::cerr << "Failed to get first frame" << std::endl;
		return;
	}

	gt.upload(t);

	cv::Mat d;

	gt.download(d);

	cv::imshow("First Frame", d);
	cv::waitKey(0);


	cap.release();
}


inline void execute_cuda_test(const fs::path& stream_file) {
	cv::cuda::printShortCudaDeviceInfo(cv::cuda::getDevice());
	read_video_dump_with_cuda(stream_file.string().c_str());
}

#else
inline void execute_cuda_test(const fs::path& streamFile) {
	cerr << "Cuda not supported by this OpenCV build" << endl;
	cout << cv::getBuildInformation() << endl;
}
#endif