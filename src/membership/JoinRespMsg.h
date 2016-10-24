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
    JoinResponseMessage(unsigned char* msg, unsigned long sz);
    JoinResponseMessage(int af, unsigned short port, unsigned char* ip);
    ~JoinResponseMessage();

    JoinResponseMessage(const JoinResponseMessage& other) = delete;
    JoinResponseMessage& operator=(const JoinResponseMessage& other) = delete;

    JoinResponseMessage( JoinResponseMessage&& other);
    JoinResponseMessage& operator=(JoinResponseMessage&& other);

    virtual unsigned long get_bodysize() const;

    virtual int build_msg_body(unsigned char* buf, unsigned long size);

    virtual void parse_msg_body(unsigned char* buf, unsigned long size) 
                              throw(parse_error);

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const;

    void set_status(int status);
    int  get_status() const;
private:
    int     m_status;
    Address m_addr;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _JOIN_REQ_MSG_H_

