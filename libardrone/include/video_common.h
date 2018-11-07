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



struct VideoPacket
{
	long Timestamp;
	long Duration;
	unsigned int FrameNumber;
	unsigned short Height;
	unsigned short Width;
	frame_type_t FrameType;
	ARBuffer Buffer;

	VideoPacket(
		long timestamp,
		long duration,
		unsigned int frameNumber,
		unsigned short height,
		unsigned short width,
		frame_type_t frameType,
		uint32_t bufferLength) :
		Timestamp(timestamp),
		Duration(Duration),
		FrameNumber(frameNumber),
		Height(height),
		Width(width),
		FrameType(frameType),
		Buffer(bufferLength)
	{
	}
};


class ARStream {
private:
	bool						isFirstFrame; // 
	unsigned int				firstFrameTimeStamp;
	unsigned int				lastFrameTimeStamp;

	uint32_t					offset;
	uint32_t					bufferSize;

	bool						isFirstFilled;
	bool						isSecondFilled;

	std::shared_ptr<ARBuffer>	first;
	std::shared_ptr<ARBuffer>	second;

	// Me faire une queue pour envoyé le shared_ptr d'un packet video assembler

	void advance1Buffer();
	bool findMagicWord();
	
	unsigned char at(uint32_t i) const;
	//void newDataReceive(unsigned char* data, uint32_t count);
	void copyTo(unsigned char* pOut, uint32_t count);

public:
	ARStream(uint32_t bufferSize);


};


#endif