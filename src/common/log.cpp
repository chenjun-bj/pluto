/**
 *******************************************************************************
 * log.cpp                                                                     *
 *                                                                             *
 * log interface                                                               *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <stdio.h>

#include "log.h"

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */
Log* Log::s_inst = NULL;

Log* Log::get_instance()
{
    if (s_inst==NULL) {
        // TODO: use mutex to protect
        s_inst = new Log();
    }
    return s_inst;
}

Log::Log(): fd(NULL)
{
}

Log::~Log()
{
    if (fd) {
        fclose(fd);
    }
}

void Log::set_log_file(char* fn)
{
    if (fn) {
        fd = fopen(fn, "a");
    }
}

void Log::sendlog(int level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (fd) {
        vfprintf(fd, fmt, args);
    }
    else {
        vprintf(fmt, args);
    }
    va_end(args);
}

Log* getlog()
{
    return Log::get_instance();
}

