#ifndef STAGING_TEST_H
#define STAGING_TEST_H

#include "video_common.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/opt.h>
	#include <libavutil/common.h>
	#include <libavutil/avutil.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
}

AVFormatContext* fc = 0;
int vi = -1; // vi veut dire video index

inline void process_stream() {
}

// enfaite pense pas qu'on aille a valider le nal,
inline int get_nal_type(const char* data, int len)
{
	// Data pas assez long pour avoir un NAL
	if (!data || 5 >= len)
		return -1;

	// Trouve la valeur du premier NAL
	
	return 0;
}

inline cv::Mat avframe_to_mat(const AVFrame* avframe)
{
	AVFrame dst;
	cv::Mat m;

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

	return m;
	
}
AVCodecContext* codec_context = nullptr;

inline bool init_stream(const unsigned char* data, int len)
{

	const char* file = "test.avi";

	const AVCodecID codec_id = AV_CODEC_ID_H264;
	AVCodec* codec = avcodec_find_encoder(codec_id);

	int br = 1000000;
	int w = 640;
	int h = 360;

	int fps = 24;

	// Crée le container pour le stream
	//AVOutputFormat* of = av_guess_format(0, file, 0);
	fc = avformat_alloc_context();

	// ajoute un stream video
	AVStream* pst = avformat_new_stream(fc, codec); // Pourquoi je passe pas le codec ici ?
	vi = pst->index;

	codec_context = avcodec_alloc_context3(codec);

	codec_context->bit_rate = br;
	codec_context->width = w;
	codec_context->height = h;
	codec_context->time_base = {1,fps};
	codec_context->gop_size = 10; // Emit one intra frame every ten frames
	codec_context->max_b_frames = 1;
	codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

	// Vu quon n'est en h264
	av_opt_set(codec_context->priv_data, "preset", "slow", 0);


	// Ouvre notre codec	
	if(avcodec_open2(codec_context, codec,nullptr) < 0)
	{
		cerr << "Impossible d'ouvrir le codec" << endl;
		return false;
	}

	return true;
}

// https://ffmpeg.org/doxygen/trunk/doc_2examples_2decoding__encoding_8c-example.html
// https://stackoverflow.com/questions/5964142/raw-h264-frames-in-mpegts-container-using-libavcodec
// https://stackoverflow.com/questions/44852117/libav-avframe-to-opencv-mat-to-avpacket-conversion

inline void append_stream(const unsigned char* data, int len)
{
	if( 0 > vi)
	{
		cerr << "video index is less than 0" << endl;
		return;
	}
	AVStream* pst = fc->streams[vi];

	AVPacket pkt;
	AVFrame* frame;

	frame = av_frame_alloc();

	if (!frame)
	{
		cerr << "Could not allocate video frame" << endl;
		return;
	}

	frame->format = codec_context->pix_fmt;
	frame->width = codec_context->width;
	frame->height = codec_context->height;

	// Alloue la mémoire de la frame
	int ret = av_image_alloc(frame->data, frame->linesize, codec_context->width, codec_context->height, codec_context->pix_fmt, 32);

	// Init un nouveau packet
	av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;

}


inline void execute_staging_test(const fs::path& folder, int nbr_trame)
{
	fs::path file_name = folder / "stream.bin";
	if(!fs::exists(file_name))
	{
		cerr << "The file " << file_name.string() << " does not exists" << endl;
		return;
	}

	av_log_set_level(AV_LOG_DEBUG);

	av_register_all();

	int length = 0;
	char* buffer;

	for(int i = 0; i < nbr_trame;i++)
	{
		fs::path file = std::to_string(i) + ".raw";
		file = folder / file;
		cout << "Got frame on " << file.string() << endl;
		ifstream ifs(file, ofstream::binary);

		// Get la longeur du fichier pour savoir le buffer a prendre
		ifs.seekg(0, ios::end);
		length = ifs.tellg();
		ifs.seekg(0, ios::beg);

		if (length == 0) {
			std::cerr << "No data in file " << file << std::endl;
			return;
		}

		buffer = new char[length];

		std::cout << "File " << file << " length is " << length << std::endl;

		ifs.read(buffer, length);

		if(!fc)
		{
			if(!init_stream((unsigned char*)buffer, length))
			{
				return;
			}
		}
		if(fc)
		{
			append_stream((unsigned char*)buffer, length);
		}	
	}
}

#endif
