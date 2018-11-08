#include "video_common.h"
#include <cstring>
#include <cassert>


const unsigned char c_magicWord[] = { 'P', 'a', 'V', 'E' };



bool getOffsetMagicWord(const char* data) {
	return (data[0] == 0x50 && data[1] == 0x61 && data[2] == 0x56 && data[3] == 0x45);
}
