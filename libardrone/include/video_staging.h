#ifndef VIDEO_STAGING_H
#define VIDEO_STAGING_H

#include "video_common.h"
#include "thread.h"
#include "video_client.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/opt.h>
	#include <libavutil/common.h>
	#include <libavutil/avutil.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
}

#include <opencv2/core/mat.hpp>

class VideoStaging
{
	video_encapsulation_t prev_encapsulation_ = {}; // Dernier header PaVE reçu

	AVPixelFormat		format; // Format pixel du flux video
	int					display_width; // Largeur du flu
	int					display_height;
	int					bit_rate;
	int					fps;

	unsigned int		num_picture_decoded = 0;

	AVCodec*			codec;
	AVCodecContext*		codec_ctx;

	AVFormatContext*	format_ctx;

	AVFrame*			frame;
	AVFrame*			frame_output;
	AVPacket*			packet;
	uint8_t**			buffer_array;
	uint8_t* buffer;
	int					buffer_size;

	int					line_size;

	SwsContext*			img_convert_ctx;

	int					first_frame;
	int					last_frame;

	bool				have_received;
	bool				only_idr = true;
	bool				record_to_file = false;
	const char*			record_file;

	VFQueue*			queue;

public:
	VideoStaging(VFQueue* queue);
	~VideoStaging();

	int	init() const;

	std::thread start();

	void on_new_frame(VideoFrame vf);

private:

	void run_service();
	bool have_frame_changed(const VideoFrame& vf);
	void init_or_frame_changed(const VideoFrame& vf,bool init = false);

};

void frame_to_mat(const AVFrame* avframe, cv::Mat& m);
#endif