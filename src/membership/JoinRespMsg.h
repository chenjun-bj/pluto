/**
 *******************************************************************************
 * JoinRespMsg.h                                                               *
 *                                                                             *
 * Join response message                                                       *
 *******************************************************************************
 */

#ifndef _JOIN_RESP_MESG_H_
#define _JOIN_RESP_MESG_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "messages.h"
#include "pladdress.h"
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

class JoinResponseMessage : public Message{
public:
    JoinResponseMessage(unsigned char* msg, size_t sz);
    JoinResponseMessage(MsgStatus status, std::vector< HeartMsgStruct > & ms);
    ~JoinResponseMessage();

    JoinResponseMessage(const JoinResponseMessage& other) = delete;
    JoinResponseMessage& operator=(const JoinResponseMessage& other) = delete;

    JoinResponseMessage( JoinResponseMessage&& other);
    JoinResponseMessage& operator=(JoinResponseMessage&& other);

    virtual size_t get_bodysize() const;

    virtual int build_msg_body(unsigned char* buf, size_t size);

    virtual void parse_msg_body(unsigned char* buf, size_t size) 
                              throw(parse_error);

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const;

    int set_status(MsgStatus status) {
        if ((status < MsgStatus::PLUTO_FIRST) || (status >= MsgStatus::PLUTO_LAST)) {
            return -1;
        }
        m_status = status;
        return 0;
    }

    int set_members(std::vector< HeartMsgStruct > & ms);

    int add_members(HeartMsgStruct & hb);

    MsgStatus  get_status() const {
        return m_status;
    }

    std::vector< HeartMsgStruct > get_members();
private:
    MsgStatus m_status;
    std::vector< HeartMsgStruct > m_members;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _JOIN_RESP_MSG_H_

