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
#include <fstream>
#include <iostream>
#include <filesystem>

#include <opencv2/core/mat.hpp>

#define H264_INBUF_SIZE 50000 // Grosseur de mon buffer que j'utilise pour les trames que je recoit

typedef ConcurrentQueue<cv::Mat> MQueue;

class VideoStaging
{
	video_encapsulation_t		prev_encapsulation_ = {}; // Dernier header PaVE reçu

	AVPixelFormat				format_in;					// Format pixel du flux video
	AVPixelFormat				format_out;					// Format pixel de l'image de sortit pour OpenCV
	int							display_width;				// Largeur de l'image dans le stream
	int							display_height;				// Hauteur de l'image dans le stream
	int							bit_rate;					// Bit-rate du stream reçu
	int							fps;						// FPS du stream reçu

	unsigned int				num_picture_decoded = 0;	// Nombre d'image decoder depuis le debut du stream
	long						average_time = 0.0l;		// Temps moyen pour faire le traitement d'une image

	AVCodec*					codec;						// Contient les informations du codec qu'on utilise (H264)
	AVCodecContext*				codec_ctx;					// Contient le context de notre codec ( information sur comment decoder le stream )
	AVCodecParserContext*		codec_parser;				// Contient le context sur comment parser les frame de notre stream


	AVFrame*					frame;						// Contient la dernière frame qu'on na reçu a reconstruire de notre stream
	AVFrame*					frame_output;				// Contient la dernière frame convertit dans le format pour l'envoyer a OpenCV
	AVPacket*					packet;

	uint8_t**					buffer_array;				// Contient la liste des buffers que nous avons reçu depuis le début
	uint8_t*					buffer;						// Contient le buffer en attendant qu'on n'aille une frame complete
	int							indice_buffer;				// Contient l'indice courrant dans le buffer
	int							buffer_size;				// Contient la grosseur du buffer en ce moment

	int							line_size;

	SwsContext*					img_convert_ctx;			// Contient le context pour effectuer la conversion entre notre image YUV420 et BGR pour OpenCV

	int							first_frame;				// Contient le numéro de la première frame reçu
	int							last_frame;					// Contient le numéro de la dernière frame reçu

	bool						have_received;				// Indique si on n'a déjà commencer a recevoir des trames
	bool						only_idr = true;			// Indique si on souhaite selon parser les frames IDR et de skipper les frames P
	bool						record_to_file_raw = false; // Indique si on souhaite sauvegarder le stream dans un fichier
	const char*					record_file = nullptr;		// Indique le chemin du fichier a sauvegarder le stream
	std::ofstream				of;

	VFQueue*					queue;						// La queue dans laquelle les frames reçu par TCP arrivent
	MQueue*						mqueue;

public:
	VideoStaging(VFQueue* queue, MQueue* mqueue);
	//VideoStaging(VFQueue* queue, const char* filepath);
	~VideoStaging();

	int	init() const;

	std::thread start();


private:

	void run_service();
	bool have_frame_changed(const VideoFrame& vf);
	bool add_frame_buffer(const VideoFrame& vf);
	void init_or_frame_changed(const VideoFrame& vf,bool init = false);
	bool frame_to_mat(const AVFrame* avframe, cv::Mat& m);
	void append_file(const VideoFrame& vf);
};


#endif