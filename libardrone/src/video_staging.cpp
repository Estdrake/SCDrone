#include "video_staging.h"
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>


typedef std::chrono::high_resolution_clock HRClock;
using namespace std::chrono;

uint8_t PADDING[AV_INPUT_BUFFER_PADDING_SIZE]{ 0 };


VideoStaging::VideoStaging(VFQueue* queue, MQueue* mqueue) : of(), Runnable()
{
#ifdef DEBUG_VIDEO_STAGING
	start_gap = HRClock::now();
	qDebug() << "Starting video staging";
#endif
	avcodec_register_all();
	this->queue = queue;
	this->mqueue = mqueue;
	this->have_received = false;
	this->buffer_size = H264_INBUF_SIZE;
	this->line_size = 0;
	this->first_frame = 0;
	this->last_frame = 0;
#ifdef DEBUG_VIDEO_STAGING
	av_log_set_level(AV_LOG_VERBOSE);
#else
	av_log_set_level(AV_LOG_FATAL);
#endif

	const AVCodecID codec_id = AV_CODEC_ID_H264;
	codec = avcodec_find_decoder(codec_id);
	if (nullptr == codec)
	{
		// echer de recuperation du codec devrait pas arriver
	}

	format_in = AV_PIX_FMT_YUV420P;
	format_out = AV_PIX_FMT_BGR24;
	bit_rate = 1000;
	display_width = 640;
	display_height = 360;
	fps = 14;

	codec_ctx = avcodec_alloc_context3(codec);
	if (!codec_ctx)
	{
		// devrait jamais arriver
		return;
	}
	avcodec_get_context_defaults3(codec_ctx, codec);

	if(codec->capabilities & AV_CODEC_CAP_TRUNCATED)
	{
		codec_ctx->flags |= AV_CODEC_FLAG_TRUNCATED;
	}

	codec_ctx->bit_rate = bit_rate;
	codec_ctx->width = display_width;
	codec_ctx->height = display_height;
	codec_ctx->time_base = { 1,fps };
	codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	codec_ctx->skip_frame = AVDISCARD_DEFAULT;
	codec_ctx->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
	codec_ctx->skip_loop_filter = AVDISCARD_DEFAULT;
	codec_ctx->workaround_bugs = FF_BUG_AUTODETECT;
	codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	codec_ctx->codec_id = AV_CODEC_ID_H264;
	codec_ctx->skip_idct = AVDISCARD_DEFAULT;

	// Enfaire devrait peux etre mettre l'option fast si on n'a besoin de vitesse
	//av_opt_set(codec_ctx->priv_data, "preset", "slow", 0);

	codec_parser = av_parser_init(codec_ctx->codec_id);
	if(codec_parser == nullptr)
	{
		// devrait jamais arriver
		return;
	}

	packet = av_packet_alloc();
	frame_output = av_frame_alloc();
	frame = av_frame_alloc();

	buffer = new uint8_t[H264_INBUF_SIZE];
	buffer_size = H264_INBUF_SIZE;
	indice_buffer = 0;
	buffer_array = (uint8_t**)malloc(sizeof(uint8_t*));
	img_convert_ctx = nullptr;


}


VideoStaging::~VideoStaging()
{
	if(codec_parser)
	{
		av_parser_close(codec_parser);
		codec_parser = nullptr;
	}
	if(codec_ctx)
	{
		avcodec_close(codec_ctx);
		codec_ctx = nullptr;
	}
	if(frame)
	{
		av_free(frame);
	}
	if(frame_output)
	{
		av_free(frame_output);
	}
	if (img_convert_ctx) {
		sws_freeContext(img_convert_ctx);
	}
	if(of)
	{
	}
	delete buffer_array;
	delete buffer;
}

int VideoStaging::init() const
{
	// Ouvre notre codec	
	if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
	{
		std::cerr << "Impossible d'ouvrir le codec" << std::endl;
		return 1;
	}
	// Ouvre le fichier 
	if(record_folder)
	{
	}
#ifdef DEBUG_VIDEO_STAGING
	auto end = HRClock::now();
	qDebug() << "End video staging initialization : " << duration_cast<milliseconds>(end-start_gap).count() << "ms";
#endif

	return 0;

}
void VideoStaging::run_service()
{
	VideoFrame vf {};
	indice_buffer = 0;
	bool has_data = false;
	while(stopRequested() == false)
	{
		vf = queue->pop_wait(100ms, &has_data);
		if (!has_data)
			continue;
#ifdef DEBUG_VIDEO_STAGING
		if (last_frame != 0 && last_frame + 1 != vf.Header.frame_number) { // on n'a manquer des frames
			qDebug() << "We have lose " << (vf.Header.frame_number - last_frame) << " frames on staging";
		}
		last_frame = vf.Header.frame_number;
		last_start = std::chrono::high_resolution_clock::now();
#endif
		if(!have_received)
		{
			init_or_frame_changed(vf, true);
			have_received = true;
		}
		add_frame_buffer(vf);
#ifdef DEBUG_VIDEO_STAGING
		last_end = HRClock::now();
		times.push_back((duration_cast<milliseconds>(last_end - last_start)).count());
		if (times.size() == 100) {
			auto v = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
			qDebug() << "Average time for 100 frame " << v << "ms";
			times.clear();
		}
#endif
		if(record_to_file_raw)
		{
			append_file(vf);
		}
	}
	std::cout << "Video stagging has been stop" << std::endl;
}

void VideoStaging::set_raw_recording(bool state)
{
	if(!state)
	{
		record_to_file_raw = false;
		// la thread ne devrait plus ecrire dans le fichier raw on le ferme
		of.close();
	} else
	{
		if(!record_to_file_raw)
		{
			time_t rawTime;
			tm * timeInfo;
			char buffer[80];
			time(&rawTime);
			timeInfo = localtime(&rawTime);
			size_t s = strftime(buffer, sizeof(buffer), "%d-%m %H:%M:%S", timeInfo);
			string filename(buffer);
			filename.append(".x264");
			of.open("recording.x264",ofstream::binary | ofstream::out);
			if(!of.is_open())
			{
				std::cout << "Could not start writting to " << filename << std::endl;
				return;
			}
			record_to_file_raw = true;
		}
	}
}


bool VideoStaging::have_frame_changed(const VideoFrame& vf)
{

	return false;
}

bool VideoStaging::add_frame_buffer(const VideoFrame& vf)
{
	if(vf.Header.payload_size == 0)
	{
		return false;
	}

	if(indice_buffer + vf.Got + AV_INPUT_BUFFER_PADDING_SIZE >= buffer_size)
	{
		// buffer pas assez gros pour contenir la nouvelle data flush
		indice_buffer = 0;
	}
	// Copy dans notre buffer le contenue de la trame reçu
	memcpy(buffer+indice_buffer, vf.Data, vf.Got);
	indice_buffer += vf.Got;



	int len = av_parser_parse2(codec_parser, codec_ctx, &packet->data, &packet->size, buffer, indice_buffer, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	if(len < 0)
	{
		// erreur de parsing pas bon
		return false;
	}
	if(packet->size)
	{
		if(avcodec_send_packet(codec_ctx, packet))
		{
			int i = avcodec_receive_frame(codec_ctx, frame);
			if(i == 0)
			{
				indice_buffer = 0;
				cv::Mat m;
				if (frame_to_mat(frame, m))
				{
					mqueue->push(m);
				}
				return true;
			}
		}
	} 
	
	return false;
}

void VideoStaging::init_or_frame_changed(const VideoFrame& vf,bool init)
{
	// Update la grandeur
	codec_ctx->width = vf.Header.encoded_stream_width;
	codec_ctx->width = vf.Header.encoded_stream_height;
	this->display_height = vf.Header.display_height;
	this->display_width = vf.Header.display_width;

	this->format_in = codec_ctx->pix_fmt;

	img_convert_ctx = sws_getContext(this->display_width, this->display_height, this->format_in, this->display_width, 
		this->display_height, this->format_out, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

	if(init)
	{
		first_frame = vf.Header.frame_number;
		last_frame = first_frame;
	}
}

bool VideoStaging::frame_to_mat(const AVFrame* avframe, cv::Mat& m)
{
	AVFrame dst;

	memset(&dst, 0, sizeof(dst));

	int w = avframe->width, h = avframe->height;

	m = cv::Mat(h, w, CV_8UC3);
	dst.data[0] = (uint8_t*)m.data;

	avpicture_fill((AVPicture*)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);

	int r = sws_scale(img_convert_ctx, avframe->data, avframe->linesize, 0, h, dst.data, dst.linesize);
	return true;
}

void VideoStaging::append_file(const VideoFrame& vf)
{
	of.write((char*)vf.Data, vf.Header.payload_size);
}
