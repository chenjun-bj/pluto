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

int pl_log_write(const char* fmt, ...);

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

void Log::set_log_file(const char* fn)
{
    if (fn) {
        fd = fopen(fn, "a");
    }
}

void Log::sendlog(LogLevel level, const char* fmt, ...)
{
    if (is_level_allowed(level)) {
        va_list args;
        va_start(args, fmt);
        sendlog(fmt, args);
        va_end(args);
    }
}

void Log::sendlog(const char* fmt, va_list args)
{
    if (fd) {
        vfprintf(fd, fmt, args);
    }
    else {
        vprintf(fmt, args);
    }
}

Log::PRINT Log::get_print_handle() const 
{
    return pl_log_write;
}

Log* getlog()
{
    return Log::get_instance();
}

int pl_log_write(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    getlog()->sendlog(fmt, args);
    va_end(args);

    return 0;
}

