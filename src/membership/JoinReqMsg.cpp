/**
 *******************************************************************************
 * JoinReqMsg.cpp                                                              *
 *                                                                             *
 * Join request message definition                                             *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <stdexcept>
#include <arpa/inet.h>

#include "JoinReqMsg.h"

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

JoinRequestMessage::JoinRequestMessage(unsigned char* msg, unsigned long sz) : 
   Message(msg, sz),
   m_af(0),
   m_type(0),
   m_portnumber(0)
{
   memset(m_ip, 0, PL_IPv6_ADDR_LEN);
   //parse_msg();
}

JoinRequestMessage::JoinRequestMessage(int af, unsigned short port, 
                                       unsigned char* ip) :
   Message(MsgType::JOINREQ)
{
    if ((af != AF_INET) && (af != AF_INET6)) {
        throw std::invalid_argument("invalid parameter 'af'");
    }
    if (ip == nullptr) {
        throw std::invalid_argument("invalid parameter 'ip'");
    }
    m_af         = af;
    m_type       = 0; // Not support
    m_portnumber = port;
    unsigned int sz = af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN;
    memcpy(m_ip, ip, sz);
}

JoinRequestMessage::~JoinRequestMessage()
{
}

JoinRequestMessage::JoinRequestMessage(JoinRequestMessage&& other) :
   Message(std::move(other))
{
    unsigned int sz = other.m_af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN;
    memcpy(m_ip, other.m_ip, sz);
    m_af    = other.m_af;
    m_type  = other.m_type;
    m_portnumber = other.m_portnumber;
}


JoinRequestMessage& JoinRequestMessage::operator=(JoinRequestMessage&& other)
{
    if (this != &other) {
        unsigned int sz = other.m_af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN;
        memcpy(m_ip, other.m_ip, sz);
        m_af    = other.m_af;
        m_type  = other.m_type;
        m_portnumber = other.m_portnumber;
    }
    return *this;
}

unsigned long JoinRequestMessage::get_bodysize() const
{
    unsigned iplen = 0;
    if (m_af == AF_INET) {
        iplen = PL_IPv4_ADDR_LEN;
    }
    else if (m_af == AF_INET6) {
        iplen = PL_IPv6_ADDR_LEN; 
    }
    else {
        // Error
        return 0;
    }

    return iplen + sizeof(unsigned short) + 2 * sizeof(int);
}

int JoinRequestMessage::build_msg_body(unsigned char* buf, unsigned long size)
{
    /* Format 
     * int - Address Family: AF_INET, AF_INET6 
     * int - Type: SOCK_STREAM, SOCK_DGRAM
     * unsigned short - Self portnumber
     * unsigned char  - Self address
     */
    unsigned long bdy_len = get_bodysize();
    if (bdy_len == 0) {
        return -1;
    }
    if (bdy_len > size) {
        return -1;
    } 
    if (buf == nullptr) {
        return -1;
    }

    int ival;
    unsigned short sval;
    ival = htonl(m_af);
    memcpy(buf, &ival, sizeof(int));
    buf += sizeof(int);
    ival = htonl(m_type);
    memcpy(buf, &ival, sizeof(int));
    buf += sizeof(int);
    sval = htons(m_portnumber);
    memcpy(buf, &sval, sizeof(unsigned short));
    buf += sizeof(unsigned short);
    if (m_af == AF_INET) {
        memcpy(buf, m_ip, PL_IPv4_ADDR_LEN);
    }
    else if (m_af == AF_INET) {
        memcpy(buf, m_ip, PL_IPv6_ADDR_LEN);
    }
    else {
        return -1;
    }

    return 0;
}

void JoinRequestMessage::parse_msg_body(unsigned char* buf, unsigned long size)
                       throw(parse_error)
{
    if (buf==nullptr) {
        throw parse_error("null buffer");
    }

    unsigned int minlen = 2*sizeof(int) + sizeof(short) + PL_IPv4_ADDR_LEN;
    if (size < minlen) {
        throw parse_error("size error");
    }

    int val;
    memcpy(&val, buf, sizeof(int));
    buf += sizeof(int);

    val = ntohl(val);
    if ((val != AF_INET) && (val != AF_INET6)) {
        throw parse_error("invalid address family");
    }
    m_af = val;

    memcpy(&val, buf, sizeof(int));
    buf += sizeof(int);

    unsigned short sval;
    memcpy(&sval, buf, sizeof(unsigned short));
    buf += sizeof(unsigned short);
    m_portnumber = ntohs(sval);

    if (m_af == AF_INET) {
        memcpy(m_ip, buf, PL_IPv4_ADDR_LEN);
    } else {
        // IPv6
        memcpy(m_ip, buf, PL_IPv6_ADDR_LEN);
    }
}

void JoinRequestMessage:: dump_body(int (*output)(const char*, ...),
                                    bool verbose) const
{
    if (m_af == AF_INET) {
        output("IPv4: ");
        output("%0d.%0d.%0d.%0d\n", m_ip[0],
                                    m_ip[1],
                                    m_ip[2],
                                    m_ip[3]);
    }
    else if (m_af == AF_INET6) {
        output("IPv6: ");
        output("%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X.%0X\n", 
                                    m_ip[0], m_ip[1], m_ip[2], m_ip[3],
                                    m_ip[4], m_ip[5], m_ip[6], m_ip[7],
                                    m_ip[8], m_ip[9], m_ip[10], m_ip[11],
                                    m_ip[12], m_ip[13], m_ip[14], m_ip[15]);
    }

    output("Port: %d\n", m_portnumber);
}

void JoinRequestMessage::set_ip_addr(int af, unsigned char* ip)
{
    if (af == AF_INET) {
        m_af = af;
        memcpy(m_ip, ip, PL_IPv4_ADDR_LEN);
    }
    else if (af == AF_INET6) {
        m_af = af;
        memcpy(m_ip, ip, PL_IPv6_ADDR_LEN);
    }
    else {
        throw std::invalid_argument("invalid parameter 'af'");
    }
}

void JoinRequestMessage::set_portnumber(unsigned short port)
{
    m_portnumber = port;
}

const unsigned char* JoinRequestMessage::get_ip_addr(int * af) const
{
    *af = m_af;
    return m_ip;
}

unsigned short JoinRequestMessage::get_portnumber() const
{
    return m_portnumber;
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


