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

#include "JoinRespMsg.h"

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

JoinResponseMessage::JoinResponseMessage(unsigned char* msg, unsigned long sz) : 
   Message(msg, sz),
   m_addr(msgbodyptr(), msgbodysize())
{
   //parse_msg();
}

JoinResponseMessage::JoinResponseMessage(int af, unsigned short port, 
                                       unsigned char* ip) :
   Message(MsgType::JOINREQ),
   m_addr(af, SOCK_STREAM, ip, port,
          af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN)
{
}

JoinResponseMessage::~JoinResponseMessage()
{
}

JoinResponseMessage::JoinResponseMessage(JoinResponseMessage&& other) :
   Message(std::move(other)),
   m_addr(std::move(other.m_addr))
{
    m_status = other.m_status;
}


JoinResponseMessage& JoinResponseMessage::operator=(JoinResponseMessage&& other)
{
    Message::operator=(std::move(other));
    if (this != &other) {
        m_status = other.m_status;
    }
    return *this;
}

unsigned long JoinResponseMessage::get_bodysize() const
{
    return 0;
}

int JoinResponseMessage::build_msg_body(unsigned char* buf, unsigned long size)
{
    return 0;
}

void JoinResponseMessage::parse_msg_body(unsigned char* buf, unsigned long size)
                       throw(parse_error)
{
}

void JoinResponseMessage:: dump_body(int (*output)(const char*, ...),
                                    bool verbose) const
{
    m_addr.dump();
    output("Status: %d\n", m_status);
}


/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


