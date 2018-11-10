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

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

/*
AVFormatContext* fc = 0;
int vi = -1; // vi veut dire video index


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

inline bool init_stream(unsigned char* data, int len)
{

	const char* file = "test.avi";

	const AVCodecID codec_id = AV_CODEC_ID_H264;
	AVCodec* codec = avcodec_find_encoder(codec_id);
	// Crée le container pour le stream
	fc = avformat_alloc_context();

	AVOutputFormat *of = av_guess_format(0, file, 0);
	fc = avformat_alloc_context();
	fc->oformat = of;
	strcpy(fc->filename, file);

	int br = 1000000;
	int w = 640;
	int h = 360;

	int fps = 24;

;

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

	if (!(fc->oformat->flags & AVFMT_NOFILE))
		avio_open(&fc->pb, fc->filename,0);

	avformat_write_header(fc,nullptr);


	return true;
}

// https://ffmpeg.org/doxygen/trunk/doc_2examples_2decoding__encoding_8c-example.html
// https://stackoverflow.com/questions/5964142/raw-h264-frames-in-mpegts-container-using-libavcodec
// https://stackoverflow.com/questions/44852117/libav-avframe-to-opencv-mat-to-avpacket-conversion

inline void append_stream(uint8_t* data, int len)
{
	if( 0 > vi)
	{
		cerr << "video index is less than 0" << endl;
		return;
	}
	AVStream* pst = fc->streams[vi];

	AVPacket pkt;


	// Init un nouveau packet
	av_init_packet(&pkt);
	pkt.flags |= AV_PKT_FLAG_KEY;
	pkt.data = data;
	pkt.stream_index = pst->index;
	pkt.size = len;

	pkt.dts = AV_NOPTS_VALUE;
	pkt.pts = AV_NOPTS_VALUE;

	av_interleaved_write_frame(fc, &pkt);


}


inline void execute_staging_test(const fs::path& folder, int nbr_trame)
{
	fs::path file_name = folder / "stream.bin";
	if(!fs::exists(file_name))
	{
		cerr << "The file " << file_name.string() << " does not exists" << endl;
		return;
	}
;

	avcodec_register_all();

	av_log_set_level(AV_LOG_DEBUG);
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

		cv::VideoWriter vw;
		int codec = cv::VideoWriter::fourcc('X', '2', '6', '4');
		

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
*/

class video_decoder
{
private:
	AVPixelFormat		format;
	int					width;
	int					height;
	int					bit_rate;
	int					fps;

	unsigned int		num_picture_decoded = 0;

	AVCodec*			codec;
	AVCodecContext*		codec_ctx;

	AVFrame*			frame;
	AVFrame*			frame_output;

	uint8_t**			buffer_array;
	uint8_t*			buffer;

	SwsContext*			img_convert_ctx;
public:
	video_decoder();
	~video_decoder();

};

inline video_decoder::video_decoder()
{
	avcodec_register_all();
	av_log_set_level(AV_LOG_DEBUG);

	const AVCodecID codec_id = AV_CODEC_ID_H264;
	codec = avcodec_find_encoder(codec_id);
	if(nullptr == codec)
	{
		// echer de recuperation du codec devrait pas arriver
	}

	format = AV_PIX_FMT_BGR24;
	bit_rate = 1000000;
	width = 640;
	height = 360;
	fps = 24;

	codec_ctx = avcodec_alloc_context3(codec);

	codec_ctx->bit_rate = bit_rate;
	codec_ctx->width = width;
	codec_ctx->height = height;
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
	//av_opt_set(codec_context->priv_data, "preset", "slow", 0);

	// Ouvre notre codec	
	if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
	{
		cerr << "Impossible d'ouvrir le codec" << endl;
		return;
	}

	frame_output = av_frame_alloc();
	frame = av_frame_alloc();

	buffer_array = (uint8_t**)malloc(sizeof(uint8_t*));
	buffer = nullptr;
	img_convert_ctx = nullptr;
}

inline video_decoder::~video_decoder()
{
}
inline void execute_staging_test(const fs::path& folder, int nbr_trame)
{
	video_decoder vd;
}
#endif
