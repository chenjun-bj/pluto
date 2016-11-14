/**
 *******************************************************************************
 * util.cpp                                                                    *
 *                                                                             *
 * Utility implementation                                                      *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "util.h"

#include <cstdarg>
#include <cstdlib>
#include <cctype>

#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

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

/*
 *******************************************************************************
 *  Class/Function definition                                                  *
 *******************************************************************************
 */

void err_quit(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

/* daemonize is from APUE */
void daemonize(const char *cmd)
{
    int               i, fd0, fd1, fd2;
    pid_t             pid;
    struct rlimit     rl;
    struct sigaction  sa;

    /*
     * Clear file creation mask.
     */
    umask(0);

    /*
     * Get maximum number of file descriptors.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        err_quit("%s: can't get file limit", cmd);
    }

    /*
     * Become a session leader to lose controlling TTY.
     */
    if ((pid = fork()) < 0) {
        err_quit("%s: can't fork", cmd);
    }
    else if (pid != 0) {
        exit(0);
    }
    setsid();

    /*
     * Ensure future opens won't allocate controlling TTYs.
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        err_quit("%s: can't ignore SIGHUP", cmd);
    }
    if ((pid = fork()) < 0) {
        err_quit("%s: can't fork", cmd);
    }
    else if (pid != 0) {
        exit(0);
    }

    /*
     * Change the current working directory to the root so
     * we won't prevent file systems from being unmounted.
     */
    if (chdir("/") < 0) {
        err_quit("%s: can't change directory to /", cmd);
    }

    /*
     * Close all open file descriptors.
     */
    if (rl.rlim_max == RLIM_INFINITY) {
        rl.rlim_max = 1024;
    }
    for (i = 0; i < rl.rlim_max; i++) {
        close(i);
    }

    /*
     * Attach file descriptors 0, 1, and 2 to /dev/null.
     */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);

    /*
     * Initialize the log file.
     */
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d",
          fd0, fd1, fd2);
        exit(1);
    }
}

int register_signal(int signo, void (*handler)(int), int * errcode) 
{
    struct sigaction sa = { 0 };
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(signo, &sa, NULL) < 0) {
        *errcode = errno;
        return -1;
    }
    return 0;
}

void dump_memory(const char* cap, const char* data, unsigned long sz,
                 int (*output)(const char*, ...))
{
    if (cap) {
        output("%s\n", cap);
    }

    unsigned int i = 0;
    unsigned int offset = 0;

    while (offset < sz)
    {
        output("%04X : ", offset);
        // DUMP Hexa
        for (i = offset; i < offset + 16; i++)
        {
            if (i < sz) {
                output("%02X ", (unsigned char)*(data + i));
            }
            else {
                output("   ");
            }
        }
        output("  ");
        // DUMP Ascii
        for (i = offset; i < offset + 16; i++)
        {
            if (i < sz)
            {
                if (isprint(*(data+i))) {
                    output("%c", *(data + i));
                }
                else {
                    output(".");
                }
            }
        }
        output("\n");
        offset += 16;
    }
}

boost::asio::ip::udp::endpoint ip2udpend(const string& ip, unsigned short port)
{
    using namespace boost::asio;

    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip, ec);
    boost::asio::detail::throw_error(ec, "ip");

    ip::udp::endpoint ep(addr, port);

    return ep;
}

boost::asio::ip::tcp::endpoint ip2tcpend(const string& ip, unsigned short port)
{
    using namespace boost::asio;

    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip, ec);
    boost::asio::detail::throw_error(ec, "ip");

    ip::tcp::endpoint ep(addr, port);

    return ep;
}

boost::asio::ip::address rawip2address(int af, const unsigned char *ip)
{
    using namespace boost::asio;

    if (af == AF_INET) {
        ip::address_v4::bytes_type rawip;

        for (unsigned int i=0; i<rawip.max_size(); i++) {
            rawip[i] = ip[i];
        }
        
        ip::address_v4 addr(rawip);
        return addr;
    }
    else if (af == AF_INET6) {
        ip::address_v6::bytes_type rawip;
        
        for (unsigned int i=0; i<rawip.max_size(); i++) {
            rawip[i] = ip[i];
        }

        ip::address_v6 addr(rawip);
        return addr;
    }

    return boost::asio::ip::address();
}
