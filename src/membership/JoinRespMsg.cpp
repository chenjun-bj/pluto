/**
 *******************************************************************************
 * JoinRespMsg.cpp                                                             *
 *                                                                             *
 * Join response message definition                                            *
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

JoinResponseMessage::JoinResponseMessage(unsigned char* msg, size_t sz, 
                                         bool managebuf) : 
   Message(msg, sz, managebuf),
   m_status(MsgStatus::OK)
{
   //parse_msg();
}

JoinResponseMessage::JoinResponseMessage(MsgStatus status,
                                         std::vector< HeartMsgStruct > & ms) :
   Message(MsgType::JOINRESP),
   m_members(ms)
{
    set_status(status);
}

JoinResponseMessage::JoinResponseMessage(MsgStatus status) : 
   Message(MsgType::JOINRESP)
{
    set_status(status);
}

JoinResponseMessage::~JoinResponseMessage()
{
}

JoinResponseMessage::JoinResponseMessage(JoinResponseMessage&& other) :
   Message(std::move(other)),
   m_members(std::move(other.m_members))
{
    m_status = other.m_status;
}


JoinResponseMessage& JoinResponseMessage::operator=(JoinResponseMessage&& other)
{
    Message::operator=(std::move(other));
    if (this != &other) {
        m_status = other.m_status;
        m_members= std::move(other.m_members);
    }
    return *this;
}

size_t JoinResponseMessage::get_bodysize() const
{
    size_t sz = 2*sizeof(int32);
    for (auto hb : m_members) {
        sz += hb.get_required_buf_len();
    }
    return sz;
}

int JoinResponseMessage::build_msg_body(unsigned char* buf, size_t size)
{
    /*
     * format
     * int32 - status
     * int32 - count of members
     * member list - refer MsgCommon.h
     */
    if (buf == nullptr) {
        throw std::invalid_argument("nullptr");
    }

    size_t sz = get_bodysize();
    if (sz > size) {
        throw std::length_error("no enough buf");
    }

    int32 ival = static_cast<int32>(m_status);
    ival = htonl(ival);
    memcpy(buf, &ival, sizeof(int32));
    buf += sizeof(int32);

    ival = m_members.size();
    ival = htonl(ival);
    memcpy(buf, &ival, sizeof(int32));
    buf += sizeof(int32);

    size_t remain = size - 2 * sizeof(int32);
    for (auto hb : m_members) {
        hb.build(buf, remain);
        size_t used = hb.get_required_buf_len();
        buf += used;
        remain -= used;
    }
 
    return 0;
}

void JoinResponseMessage::parse_msg_body(unsigned char* buf, size_t size)
                       throw(parse_error)
{
    if (buf==nullptr) {
        throw parse_error("nullptr");
    }

    size_t minlen = 2 * sizeof(int32);
    if (size < minlen) {
        throw parse_error("size error");
    }

    int32 ival;
    memcpy(&ival, buf, sizeof(int32));
    buf += sizeof(int32);
    ival = ntohl(ival);

    int low, hi;
    low = static_cast<int>(MsgStatus::PLUTO_FIRST);
    hi  = static_cast<int>(MsgStatus::PLUTO_LAST);
    if ((ival < low) || (ival >=hi)) {
        throw parse_error("invalid status");
    }

    m_status = static_cast<MsgStatus>(ival);

    memcpy(&ival, buf, sizeof(int32));
    buf += sizeof(int32);
    ival = ntohl(ival);

    size_t remain = size - 2 * sizeof(int32);
    for (int32 i=0; i<ival; i++) {
        HeartMsgStruct hb(buf, remain);
        size_t used = hb.get_required_buf_len();
        remain -= used;
        m_members.push_back(hb);
    }
}

void JoinResponseMessage:: dump_body(int (*output)(const char*, ...),
                                    bool verbose) const
{
    output("Status      : %d\n", m_status);
    output("Member count: %d\n", m_members.size());
    for (auto hb : m_members) {
        hb.dump(output);
    }
}

int JoinResponseMessage::set_members(std::vector< HeartMsgStruct > & ms)
{
    m_members = move(ms);
    return 0;
}

int JoinResponseMessage::add_member( HeartMsgStruct & hb)
{
    m_members.push_back(hb);
    return 0;
}


/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


