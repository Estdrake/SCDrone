#include "video_staging.h"
#include <iostream>

VideoStaging::VideoStaging(VFQueue* queue)
{
	this->queue = queue;
	this->have_received = false;

	avcodec_register_all();
	av_log_set_level(AV_LOG_DEBUG);

	const AVCodecID codec_id = AV_CODEC_ID_H264;
	codec = avcodec_find_encoder(codec_id);
	if (nullptr == codec)
	{
		// echer de recuperation du codec devrait pas arriver
	}

	format = AV_PIX_FMT_BGR24;
	bit_rate = 1000000;
	display_width = 640;
	display_height = 360;
	fps = 24;

	codec_ctx = avcodec_alloc_context3(codec);

	codec_ctx->bit_rate = bit_rate;
	codec_ctx->width = display_width;
	codec_ctx->height = display_height;
	codec_ctx->time_base = { 1,fps };
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->skip_frame = AVDISCARD_DEFAULT;
//	codec_ctx->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
//	codec_ctx->skip_loop_filter = AVDISCARD_DEFAULT;
	codec_ctx->workaround_bugs = FF_BUG_AUTODETECT;
	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->codec_id = AV_CODEC_ID_H264;
//	codec_ctx->skip_idct = AVDISCARD_DEFAULT;

	// Enfaire devrait peux etre mettre l'option fast si on n'a besoin de vitesse
	//av_opt_set(codec_context->priv_data, "preset", "slow", 0);

	frame_output = av_frame_alloc();
	frame = av_frame_alloc();

	buffer_array = (uint8_t**)malloc(sizeof(uint8_t*));
	buffer = nullptr;
	img_convert_ctx = nullptr;
}

VideoStaging::~VideoStaging()
{
}

int VideoStaging::init() const
{
	// Ouvre notre codec	
	if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
	{
		std::cerr << "Impossible d'ouvrir le codec" << std::endl;
		return 1;
	}

	return 0;

}

std::thread VideoStaging::start()
{
	return std::thread([this] { this->run_service(); });
}

void VideoStaging::run_service()
{
	VideoFrame vf {};
	for(;;)
	{
		vf = queue->pop();
		if(!have_received)
		{
			init_or_frame_changed(vf, true);
			have_received = true;
		}
		on_new_frame(vf);
	}
}

void VideoStaging::on_new_frame(VideoFrame vf)
{
	int frameFinish;
	AVPacket packet;
	if(only_idr && (vf.Header.frame_type == FRAME_TYPE_IDR_FRAME || vf.Header.frame_type == FRAME_TYPE_I_FRAME ))
	{
		std::cout << "Im a " + std::to_string(vf.Header.frame_type) << std::endl;
		packet.data = vf.Data;
		packet.size = vf.Header.payload_size;

		int i = avcodec_send_packet(codec_ctx, &packet);
		if(i < 0)
		{
			char* d = new char[1024];
			d = av_make_error_string(d, 1024, i);
			std::cout << i << endl;
			delete[] d;
		}

	} 

}

bool VideoStaging::have_frame_changed(const VideoFrame& vf)
{

	return false;
}

void VideoStaging::init_or_frame_changed(const VideoFrame& vf,bool init)
{
	// Update la grandeur
	codec_ctx->width = vf.Header.encoded_stream_width;
	codec_ctx->width = vf.Header.encoded_stream_height;
	this->display_height = vf.Header.display_height;
	this->display_width = vf.Header.display_width;

	this->format = codec_ctx->pix_fmt;
	this->buffer_size = avpicture_get_size(this->format, this->display_width, this->display_height);
	this->buffer = (uint8_t*)av_realloc(this->buffer, this->buffer_size * sizeof(uint8_t));

	avpicture_fill((AVPicture*)frame, this->buffer, this->format, this->display_width, this->display_height);

//	this->img_convert_ctx = sws_getCachedContext(img_convert_ctx, this->display_width, this->display_height,
//		this->format, this->display_width, this->display_height,this->format, SWS_FAST_BILINEAR, nullptr,nullptr, nullptr);
	if(init)
	{
		first_frame = vf.Header.frame_number;
		last_frame = first_frame;
	}
}

void frame_to_mat(const AVFrame* avframe, cv::Mat& m)
{
	AVFrame dst;

	memset(&dst, 0, sizeof(dst));

	int w = avframe->width, h = avframe->height;

	m = cv::Mat(h, w, CV_8UC3);
	dst.data[0] = (uint8_t*)m.data;

	avpicture_fill((AVPicture*)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);

	struct SwsContext *convert_ctx = NULL;

	enum AVPixelFormat src_pixfmt = AV_PIX_FMT_BGR24;
	enum AVPixelFormat dst_pixfmt = AV_PIX_FMT_BGR24;

	convert_ctx = sws_getContext(w, h, src_pixfmt, w, h, dst_pixfmt, SWS_FAST_BILINEAR, NULL, NULL, NULL);

	sws_scale(convert_ctx, avframe->data, avframe->linesize, 0, h, dst.data, dst.linesize);
	sws_freeContext(convert_ctx);
}
