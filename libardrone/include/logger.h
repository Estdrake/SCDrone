#ifndef LOGGER_H
#define LOGGER_H
#include <vector>
#include <mutex>

#include "log.h"


inline ARLogger logger;


#define AR_LOG_INFO(code,msg) { \
	logger.Append(LINFO,__FILENAME__,__LINE__,code,msg);}
#define AR_LOG_WARNING(code,msg) { \
	logger.Append(LWARNING,__FILENAME__,__LINE__,code,msg);}
#define AR_LOG_ERROR(code,msg) { \
	logger.Append(LERROR,__FILENAME__,__LINE__,code,msg);}

#endif 