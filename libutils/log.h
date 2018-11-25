//
// Created by wq on 18-11-25.
//

#ifndef LOG_H
#define LOG_H

#include "definition.h"

#include <mutex>
#include <vector>


#define FOREACH_LOGLEVEL(LOGLEVEL) LOGLEVEL(LERROR) LOGLEVEL(LINFO) LOGLEVEL(LWARNING)

enum LOGLEVEL_ENUM { FOREACH_LOGLEVEL(GENERATE_ENUM) };

static const char* LOGLEVEL_STRING[] = {
        FOREACH_LOGLEVEL(GENERATE_STRINGS)
};


class ARLogger
{
    std::mutex						mutex;
    std::vector<std::string>		logs;

public:
    void Append(LOGLEVEL_ENUM ll, const char* file, int line, int code, const char* msg)
    {
        char buf[150];
        sprintf(buf, "[%s][%s:%d] %d %s", LOGLEVEL_STRING[ll], file, line, code, msg);
        std::unique_lock<std::mutex> lk(mutex);
        logs.emplace_back(buf);
        lk.unlock();
    }

    std::vector<std::string> Consume()
    {
        std::unique_lock<std::mutex> lk(mutex);
        std::vector <std::string> v = logs;
        logs.clear();
        return v;
    }
};
#endif //LOG_H
