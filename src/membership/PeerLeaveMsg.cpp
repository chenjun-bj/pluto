/**
 *******************************************************************************
 * PeerLeaveMsg.cpp                                                            *
 *                                                                             *
 * Peer leave message definition                                               *
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

#include "PeerLeaveMsg.h"

/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */
#define PEERLEAVE_REASON_OFFSET  sizeof(int32)

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

PeerLeaveMessage::PeerLeaveMessage(unsigned char* msg, size_t sz, 
                                   bool managebuf) : 
   Message(msg, sz, managebuf),
   m_reason(LeaveReason::NORMAL_SHUTDOWN),
   m_addr(msgbodyptr()+PEERLEAVE_REASON_OFFSET, 
          msgbodysize()-PEERLEAVE_REASON_OFFSET)
{
   //parse_msg();
}

PeerLeaveMessage::PeerLeaveMessage(LeaveReason reason,
                                   int af, unsigned short port, 
                                   unsigned char* ip) :
   Message(MsgType::PEERLEAVE),
   m_reason(reason),
   m_addr(af, SOCK_STREAM, ip, port,
          af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN)
{
}

PeerLeaveMessage::~PeerLeaveMessage()
{
}

PeerLeaveMessage::PeerLeaveMessage(PeerLeaveMessage&& other) :
   Message(std::move(other)),
   m_reason(other.m_reason),
   m_addr(std::move(other.m_addr))
{
}


PeerLeaveMessage& PeerLeaveMessage::operator=(PeerLeaveMessage&& other)
{
    Message::operator=(std::move(other));
    if (this != &other) {
        m_reason= other.m_reason;
        m_addr  = std::move(other.m_addr);
    }
    return *this;
}

size_t PeerLeaveMessage::get_bodysize() const
{
    return m_addr.get_required_buf_len() + PEERLEAVE_REASON_OFFSET;
}

int PeerLeaveMessage::build_msg_body(unsigned char* buf, size_t size)
{
    /*
     * Format:
     * int32   -- reason code
     * Address -- 
     */
    if (buf == nullptr) {
        throw std::invalid_argument("nullptr");
    }

    if (size<get_bodysize()) {
        throw std::length_error("no enough buf");
    }

    int32 ival = static_cast<int32>(m_reason);
    ival = htonl(ival);
    memcpy(buf, &ival, PEERLEAVE_REASON_OFFSET);
    buf += PEERLEAVE_REASON_OFFSET;
    size -= PEERLEAVE_REASON_OFFSET;

    m_addr.build(buf, size);
    return 0;
}

void PeerLeaveMessage::parse_msg_body(unsigned char* buf, size_t size)
                       throw(parse_error)
{
    if (buf==nullptr) {
        throw parse_error("nullptr");
    }
    if (size < get_bodysize()) {
        throw parse_error("size error");
    }
    int32 ival;
    memcpy(&ival, buf, PEERLEAVE_REASON_OFFSET);
    ival = ntohl(ival);

    buf += PEERLEAVE_REASON_OFFSET;
    size -= PEERLEAVE_REASON_OFFSET;

    int low, hi;
    low = static_cast<int>(LeaveReason::PLUTO_FIRST);
    hi  = static_cast<int>(LeaveReason::PLUTO_LAST);
    if ((ival < low) || (ival >=hi)) {
        throw parse_error("invalid reason: " + std::to_string(ival));
    }
    m_reason = static_cast<LeaveReason>(ival);

    m_addr.parse(buf, size);
}

void PeerLeaveMessage:: dump_body(int (*output)(const char*, ...),
                                    bool verbose) const
{
    output("Leave Reason: %d\n", m_reason);
    m_addr.dump(output);
}

void PeerLeaveMessage::set_ip_addr(int af, unsigned char* ip)
{
    m_addr.set_ip_addr(af, ip, 
                       af == AF_INET ? PL_IPv4_ADDR_LEN : PL_IPv6_ADDR_LEN);
}

void PeerLeaveMessage::set_port(unsigned short port)
{
    m_addr.set_port(port);
}

const unsigned char* PeerLeaveMessage::get_ip_addr(int * af) const
{
    return m_addr.get_ip_addr(af);
}

unsigned short PeerLeaveMessage::get_port() const
{
    return m_addr.get_port();
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


