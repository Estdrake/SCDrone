#ifndef AP_CMD_H
#define AP_CMD_H
#include <string>

static const char* AT_REF = "AT*REF=$seq,%d\r";
static const char* AT_PMODE = "AT*PMODE=%d,%d\r";
static const char* AT_MISC = "AT*MISC=%d,%d,%d,%d,%d\r";
static const char* AT_GAIN = "AT*GAIN=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r";
static const char* AT_ANIM = "AT*ANI=%d,%d,%d\r";
static const char* AT_VISP = "AT*VISP=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r";
static const char* AT_VISO = "AT*VISO=%d,%d\r";
static const char* AT_CAP = "AT*CAP=%d,%d,%d\r";
static const char* AT_ZAP = "AT*ZAP=%d,%d\r";

static const char* AT_FTRIM = "AT*FTRIM=%d,\r"; // Tells the drone it is lying horizontally
static const char* AT_MTRIM = "AT*MTRIM=%d,%d,%d,%d\r";

static const char* AT_POLL = "AT*POLL=%d,%d,%d,%d,%d,%d\r";
static const char* AT_PCMD = "AT*PCMD=$seq,%d,%d,%d,%d,%d\r";
static const char* AT_PCMD_MAG = "AT*PCMD_MAG=%d,%d,%d,%d,%d,%d,%d,%d\r";
static const char* AT_CONFIG = "AT*CONFIG=$seq,\"%s\",\"%s\"\r";
static const char* AT_CONFIG_IDS = "AT*CONFIG_IDS=%d,\"%s\",\"%s\",\"%s\"\r";
static const char* AT_CTRL = "AT*CTRL=$seq,%d,%d\r";
static const char* AT_COMWDG = "AT*COMWDG=%d\r";
static const char* AT_PWM = "AT*PWM=%d,%d,%d,%d,%d\r";
static const char* AT_ATFLIGHT = "AT*ATFLIGHT=%d,%d\r";
static const char* AT_CALIB = "AT*CALIB=%d,%d\r";


enum ref_flags
{
	DEFAULT_FLAG = 1 << 18 | 1 << 20 | 1 << 22 | 1 << 24 | 1 << 28,
	LAND_FLAG = (0 << 9) | DEFAULT_FLAG,
	TAKEOFF_FLAG = (1 << 9)  | DEFAULT_FLAG,
	EMERGENCY_FLAG = (1 << 8) | DEFAULT_FLAG,
};

enum progressive_flags
{
	HOVERING = 0,
	PROGRESSIVE = 1 << 0,
	COMBINED_YAW = 1 << 1 | PROGRESSIVE,
	ABSOLUTE_CONTROL = 1 << 2 | PROGRESSIVE
};


template <typename ... Args>
std::string string_format(const char* format, Args ... args)
{
	const size_t size = snprintf(nullptr, 0, format, args ...) + 1;
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format, args ...);
	return std::string(buf.get(), buf.get() + size - 1);
}

inline std::string at_format_ref(ref_flags flag)
{
	return string_format(AT_REF, flag);
}

inline std::string at_format_pcmd(int flag, float roll, float pitch, float gaz, float yaw)
{
	assert(sizeof(int32_t) == sizeof(float));
	return string_format(AT_PCMD, flag, *(int32_t*)(&roll), *(int32_t*)(&pitch), *(int32_t*)(&gaz), *(int32_t*)(&yaw));
}

inline std::string at_format_controlinit()
{
	return string_format(AT_CTRL, 0);
}

inline std::string at_format_config(std::string key, std::string value) {
	return string_format(AT_CONFIG, key.c_str(), value.c_str());
}
#endif
