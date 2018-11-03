#ifndef _VIDEO_COMMON_H_
#define _VIDEO_COMMON_H_

#include <cstdint>

enum frame_type {
	Unknow,
	I,
	P
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
	uint8 signature[4]; // <Ctype "c_unsigned int8 * 4">
	uint8 version;
	uint8 video_codec;

	unsigned short header_size;
	unsigned int payload_size;
	unsigned short encoded_stream_width;
	unsigned short encoded_stream_height;
	unsigned short display_width;
	unsigned short display_height;
	unsigned int frame_number;
	unsigned int timestamp;

	uint8 total_chuncks;
	uint8 chunck_index;
	uint8 frame_type;
	uint8 control;

	unsigned int stream_uint8_position_lw;
	unsigned int stream_uint8_position_uw;
	unsigned short stream_id;

	uint8 total_slices;
	uint8 slice_index;
	uint8 header1_size;
	uint8 header2_size;
	uint8 reserved2[2]; // <Ctype "c_unsigned int8 * 2">
	unsigned int advertised_size;
	uint8 reserved3[12]; // <Ctype "c_unsigned int8 * 12">
};




struct VideoPacket
{
	long Timestamp;
	long Duration;
	unsigned int FrameNumber;
	unsigned short Height;
	unsigned short Width;
	FrameType FrameTypev;
	ARBuffer Buffer;

	VideoPacket(
		long timestamp,
		long duration,
		unsigned int frameNumber,
		unsigned short height,
		unsigned short width,
		FrameType frameType,
		uint32_t bufferLength) :
	Timestamp(timestamp),
		Duration(Duration),
		FrameNumber(frameNumber),
		Height(height),
		Width(width),
		FrameTypev(frameType),
		Buffer(bufferLength)
	{
	}
};


class ARBuffer {
private:
	std::unique_ptr<unsigned char[]> _data;
	uint32_t						 _size;

	// Non copiable
	ARBuffer(const ARBuffer& other) { }

public:
	uint32_t getSize()const {
		return _size;
	}
	unsigned char* getData()const {
		return _data.get();
	}

	ARBuffer(unsigned char* dataToCpy, uint32_t size);
	ARBuffer(uint32_t size);
	ARBuffer(ARBuffer&& moved) :_data(std::move(moved._data)), _size(moved._size) {
		moved._size = 0;
	}

	~ARBuffer(void);
};



#endif