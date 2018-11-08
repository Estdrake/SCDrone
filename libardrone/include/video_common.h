#ifndef _VIDEO_COMMON_H_
#define _VIDEO_COMMON_H_

#include <cstdint>

enum frame_type_t {
	Unknow,
	I,
	P
};

enum video_encapsulation_codecs_t
{
	CODEC_UNKNOWN = 0,
	CODEC_VLIB,
	CODEC_P264,
	CODEC_MPEG4_VISUAL,
	CODEC_MPEG4_AVC
};


enum video_encapsulation_frametypes_t
{
	FRAME_TYPE_UNKNNOWN,
	FRAME_TYPE_IDR_FRAME,
	FRAME_TYPE_I_FRAME,
	FRAME_TYPE_P_FRAME,
	FRAME_TYPE_HEADERS,
};



struct video_encapsulation_t
{
	uint8_t signature[4]; // <Ctype "c_unsigned int8 * 4">
	uint8_t version;
	uint8_t video_codec;

	unsigned short header_size;
	unsigned int payload_size;
	unsigned short encoded_stream_width;
	unsigned short encoded_stream_height;
	unsigned short display_width;
	unsigned short display_height;
	unsigned int frame_number;
	unsigned int timestamp;

	uint8_t total_chuncks;
	uint8_t chunck_index;
	uint8_t frame_type;
	uint8_t control;

	unsigned int stream_uint8_position_lw;
	unsigned int stream_uint8_position_uw;
	unsigned short stream_id;

	uint8_t total_slices;
	uint8_t slice_index;
	uint8_t header1_size;
	uint8_t header2_size;
	uint8_t reserved2[2]; // <Ctype "c_unsigned int8 * 2">
	unsigned int advertised_size;
	uint8_t reserved3[12]; // <Ctype "c_unsigned int8 * 12">
};




struct VideoFrame {
	// Le header de la frame
	video_encapsulation_t	Header;

	// Ou on n'est rendu dans la frame
	int						Got;

	// Les données recu
	unsigned char*			Data;
};


bool getOffsetMagicWord(const char* data);

#endif