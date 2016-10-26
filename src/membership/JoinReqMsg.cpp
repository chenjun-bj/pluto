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
#include <utility>
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

JoinRequestMessage::JoinRequestMessage(unsigned char* msg, size_t sz, 
                                       bool managebuf) : 
   Message(msg, sz, managebuf),
   m_addr(msgbodyptr(), msgbodysize())
{
   //parse_msg();
}

JoinRequestMessage::JoinRequestMessage(int af, unsigned short port, 
                                       unsigned char* ip) :
   Message(MsgType::JOINREQ),
   m_addr(af, SOCK_STREAM, ip, port,
          af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN)
{
}

JoinRequestMessage::~JoinRequestMessage()
{
}

JoinRequestMessage::JoinRequestMessage(JoinRequestMessage&& other) :
   Message(std::move(other)),
   m_addr(std::move(other.m_addr))
{
}


JoinRequestMessage& JoinRequestMessage::operator=(JoinRequestMessage&& other)
{
    Message::operator=(std::move(other));
    if (this != &other) {
        m_addr = std::move(other.m_addr);
    }
    return *this;
}

size_t JoinRequestMessage::get_bodysize() const
{
    return m_addr.get_required_buf_len();
}

int JoinRequestMessage::build_msg_body(unsigned char* buf, size_t size)
{
    m_addr.build(buf, size);
    return 0;
}

void JoinRequestMessage::parse_msg_body(unsigned char* buf, size_t size)
                       throw(parse_error)
{
    m_addr.parse(buf, size);
}

void JoinRequestMessage:: dump_body(int (*output)(const char*, ...),
                                    bool verbose) const
{
    m_addr.dump(output);
}

void JoinRequestMessage::set_ip_addr(int af, unsigned char* ip)
{
    m_addr.set_ip_addr(af, ip, 
                       af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN);
}

void JoinRequestMessage::set_portnumber(unsigned short port)
{
    m_addr.set_port(port);
}

const unsigned char* JoinRequestMessage::get_ip_addr(int * af) const
{
    return m_addr.get_ip_addr(af);
}

unsigned short JoinRequestMessage::get_portnumber() const
{
    return m_addr.get_port();
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


