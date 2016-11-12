/**
 *******************************************************************************
 * log.h                                                                       *
 *                                                                             *
 * log interface                                                               *
 *******************************************************************************
 */

#ifndef _LOG_H_
#define _LOG_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <stdio.h>
#include <stdarg.h>

/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

// Log level macros

enum class LogLevel : unsigned int {
    TRACE = 0,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

class Log;
Log* getlog();

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */
enum log_level { trace, debug, info, warning, error, fatal };

class Log {
private:
    Log(); 
public:
    ~Log() ;
    void set_log_file(const char* fn);
    void sendlog(LogLevel level, const char* fmt, ...);

    bool is_level_allowed(LogLevel level) {
        // TODO: implement in future
        return true;
    }

    typedef int (*PRINT)(const char*, ...);
    PRINT get_print_handle() const ;

    static Log* get_instance();

    friend int pl_log_write(const char*, ...);
protected:
    void sendlog(const char* fmt, va_list);
private:
    static Log* s_inst;
    FILE* fd;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _LOG_H_

