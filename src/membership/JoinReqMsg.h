/**
 *******************************************************************************
 * JoinReqMsg.h                                                                *
 *                                                                             *
 * Join request message                                                        *
 *******************************************************************************
 */

#ifndef _JOIN_REQ_MESG_H_
#define _JOIN_REQ_MESG_H_

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

class JoinRequestMessage : public Message{
public:
    JoinRequestMessage(unsigned char* msg, size_t sz);
    JoinRequestMessage(int af, unsigned short port, unsigned char* ip);
    ~JoinRequestMessage();

    JoinRequestMessage(const JoinRequestMessage& other) = delete;
    JoinRequestMessage& operator=(const JoinRequestMessage& other) = delete;

    JoinRequestMessage( JoinRequestMessage&& other);
    JoinRequestMessage& operator=(JoinRequestMessage&& other);

    virtual size_t get_bodysize() const;

    virtual int build_msg_body(unsigned char* buf, size_t size);

    virtual void parse_msg_body(unsigned char* buf, size_t size) 
                              throw(parse_error);

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const;

    /* IPv4: 4 bytes, IPv6: 16 bytes */
    void set_ip_addr(int af, unsigned char* ip);
    void set_portnumber(unsigned short port);

    const unsigned char* get_ip_addr(int * af) const;
    unsigned short get_portnumber() const; 
private:
    Address m_addr;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _JOIN_REQ_MSG_H_

