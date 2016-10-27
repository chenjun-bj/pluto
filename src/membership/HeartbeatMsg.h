/**
 *******************************************************************************
 * HeartbeatMsg.h                                                              *
 *                                                                             *
 * Heatbeat message                                                            *
 *******************************************************************************
 */

#ifndef _HEARTBEAT_MESG_H_
#define _HEARTBEAT_MESG_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "messages.h"
#include "MsgCommon.h"

#include <vector>

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

class HeartbeatMessage : public Message{
public:
    HeartbeatMessage(unsigned char* msg, size_t sz, bool managebuf = true);
    HeartbeatMessage();
    HeartbeatMessage(std::vector< HeartMsgStruct > & ms);
    ~HeartbeatMessage();

    HeartbeatMessage(const HeartbeatMessage& other) = delete;
    HeartbeatMessage& operator=(const HeartbeatMessage& other) = delete;

    HeartbeatMessage( HeartbeatMessage&& other);
    HeartbeatMessage& operator=(HeartbeatMessage&& other);

    virtual size_t get_bodysize() const;

    virtual int build_msg_body(unsigned char* buf, size_t size);

    virtual void parse_msg_body(unsigned char* buf, size_t size) 
                              throw(parse_error);

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const;

    int set_members(std::vector< HeartMsgStruct > & ms);

    int add_member(HeartMsgStruct & hb);

    std::vector< HeartMsgStruct > get_members() {
        return m_members;
    }
private:
    std::vector< HeartMsgStruct > m_members;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _HEARTBEAT_MSG_H_

