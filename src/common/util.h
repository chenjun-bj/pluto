/**
 *******************************************************************************
 * util.h                                                                      *
 *                                                                             *
 * Utility declaraction:                                                       *
 *   - daemonize                                                               *
 *******************************************************************************
 */

#ifndef _UTIL_H_
#define _UTIL_H_

/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */

#include <cstdio>
#include <string>
#include <boost/asio.hpp>

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

void daemonize(const char *cmd);
int register_signal(int signo, void (*handler)(int), int * errcode);

void dump_memory(const char* cap, const char* data, unsigned long sz,
                 int (*output)(const char*, ...)=std::printf);

boost::asio::ip::udp::endpoint ip2udpend(const std::string& ip, unsigned short port);
boost::asio::ip::tcp::endpoint ip2tcpend(const std::string& ip, unsigned short port);

#endif // _UTIL_H_

