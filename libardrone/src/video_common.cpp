#include "video_common.h"
#include <cstring>
#include <cassert>


const unsigned char c_magicWord[] = { 'P', 'a', 'V', 'E' };

inline frame_type convert(byte frameType) {
	video_encapsulation_frametypes_t vFrameType = (video_encapsulation_frametypes_t)frameType;

	switch (vFrameType) {
	case FRAME_TYPE_IDR_FRAME:
	case FRAME_TYPE_I_FRAME:
		return I;
	case FRAME_TYPE_P_FRAME:
		return P;
	default:
		return Unknow;
}


ARBuffer::ARBuffer(unsigned char* dataToCpy, uint32_t size) : 
	data(new unsigned char[size], size(size) {
	std::memcpy(data.get(), size, dataToCpy, size);

}
ARBuffer::ARBuffer(uint32_t size) :
	data(new unsigned char[size], size(size) {

}

ARBuffer::~ARBuffer(void) {

}

ARStream::ARStream(uint32_t bufferSize) :
	offset(0),
	isFirstFrame(true),
	bufferSize(bufferSize),
	first(new ARBuffer(bufferSize)),
	second(new ARBuffer(bufferSize)),
	isFirstFilled(false),
	isSecondFilled(false)
{
	

}

ARStream::~ARBuffer(void) {

}



void ARStream::advance1Buffer() {
	assert(offset >= bufferSize);
	std::swap(first, second);
	offset -= bufferSize;
	isFirstFilled = isSecondFilled;
	isSecondFilled = false;
}

bool ARStream::findMagicWord() {
	for (uint32_t i = offset; i < 2 * bufferSize - sizeof(c_magicWord); i++) {
		if (at(i) == c_magicWord[0]
			&& at(i) == c_magicWord[1]
			&& at(i) == c_magicWord[2]
			&& at(i) == c_magicWord[3]) {
			offset = i;
			return true;
		}
	}
	offset = 2 * bufferSize - sizeof(c_magicWord);
	return false;
}

unsigned char ARStream::at(uint32_t i) const {
	if (i < _bufferSize) {
		return first->getData()[i];
	}
	else {
		return second->getData()[i - bufferSize];
	}
}

void ARStream::copyTo(unsigned char* pOut, uint32_t count) {
	uint32_t sizeInFirstBuffer = count;
	uint32_t sizeInSecondBuffer = 0;
	if (count + _offset > _bufferSize) {
		sizeInFirstBuffer = bufferSize - offset;
		sizeInSecondBuffer = count - sizeInFirstBuffer;
	}

	memcpy(pOut, count, first->getDate() + offset, sizeInFirstBuffer);
	if (sizeInSecondBuffer > 0) {
		memcpy(pOut + sizeInFirstBuffer, count - sizeInFirstBuffer, second->getDate(), sizeInSecondBuffer);
	}
}

void ARStream::newDataReceived(unsigned char* data, uint32_t count) {

}