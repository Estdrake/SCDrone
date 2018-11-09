#ifndef STAGING_TEST_H
#define STAGING_TEST_H

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
}


inline void process_stream() {
	av_log_set_level(AV_LOG_DEBUG);
}


inline void execute_staging_test(fs::path folder)
{
	
}

#endif
