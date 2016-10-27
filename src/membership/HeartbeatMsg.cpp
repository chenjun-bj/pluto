/**
 *******************************************************************************
 * HeartbeatMessage.cpp                                                        *
 *                                                                             *
 * Heartbeat message definition                                                *
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

#include "HeartbeatMsg.h"

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

HeartbeatMessage::HeartbeatMessage(unsigned char* msg, size_t sz, 
                                   bool managebuf) : 
   Message(msg, sz, managebuf)
{
   //parse_msg();
}

HeartbeatMessage::HeartbeatMessage(std::vector< HeartMsgStruct > & ms) :
   Message(MsgType::HEARTBEAT),
   m_members(ms)
{
}

HeartbeatMessage::HeartbeatMessage() : 
   Message(MsgType::HEARTBEAT)
{
}

HeartbeatMessage::~HeartbeatMessage()
{
}

HeartbeatMessage::HeartbeatMessage(HeartbeatMessage&& other) :
   Message(std::move(other)),
   m_members(std::move(other.m_members))
{
}

HeartbeatMessage& HeartbeatMessage::operator=(HeartbeatMessage&& other)
{
    Message::operator=(std::move(other));
    if (this != &other) {
        m_members= std::move(other.m_members);
    }
    return *this;
}

size_t HeartbeatMessage::get_bodysize() const
{
    size_t sz = sizeof(int32);
    for (auto hb : m_members) {
        sz += hb.get_required_buf_len();
    }
    return sz;
}

int HeartbeatMessage::build_msg_body(unsigned char* buf, size_t size)
{
    /*
     * format
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

    int32 ival = m_members.size();
    ival = htonl(ival);
    memcpy(buf, &ival, sizeof(int32));
    buf += sizeof(int32);

    size_t remain = size - sizeof(int32);
    for (auto hb : m_members) {
        hb.build(buf, remain);
        size_t used = hb.get_required_buf_len();
        buf += used;
        remain -= used;
    }
 
    return 0;
}

void HeartbeatMessage::parse_msg_body(unsigned char* buf, size_t size)
                       throw(parse_error)
{
    if (buf==nullptr) {
        throw parse_error("nullptr");
    }

    size_t minlen = sizeof(int32);
    if (size < minlen) {
        throw parse_error("size error");
    }

    int32 ival;
    memcpy(&ival, buf, sizeof(int32));
    buf += sizeof(int32);
    ival = ntohl(ival);

    size_t remain = size - sizeof(int32);
    for (int32 i=0; i<ival; i++) {
        HeartMsgStruct hb(buf, remain);
        size_t used = hb.get_required_buf_len();
        remain -= used;
        buf    += used;
        m_members.push_back(hb);
    }
}

void HeartbeatMessage:: dump_body(int (*output)(const char*, ...),
                                    bool verbose) const
{
    output("Member count: %d\n", m_members.size());
    for (auto hb : m_members) {
        hb.dump(output);
    }
}

int HeartbeatMessage::set_members(std::vector< HeartMsgStruct > & ms)
{
    m_members = move(ms);
    return 0;
}

int HeartbeatMessage::add_member( HeartMsgStruct & hb)
{
    m_members.push_back(hb);
    return 0;
}


/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


