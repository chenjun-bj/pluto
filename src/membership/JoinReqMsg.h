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

#define PL_IPv4_ADDR_LEN 4
#define PL_IPv6_ADDR_LEN 16

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
    JoinRequestMessage(unsigned char* msg, unsigned long sz);
    JoinRequestMessage(int af, unsigned short port, unsigned char* ip);
    ~JoinRequestMessage();

    JoinRequestMessage(const JoinRequestMessage& other) = delete;
    JoinRequestMessage& operator=(const JoinRequestMessage& other) = delete;

    JoinRequestMessage( JoinRequestMessage&& other);
    JoinRequestMessage& operator=(JoinRequestMessage&& other);

    virtual unsigned long get_bodysize() const;

    virtual int build_msg_body(unsigned char* buf, unsigned long size);

    virtual void parse_msg_body(unsigned char* buf, unsigned long size) 
                              throw(parse_error);

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const;

    /* IPv4: 4 bytes, IPv6: 16 bytes */
    void set_ip_addr(int af, unsigned char* ip);
    void set_portnumber(unsigned short port);

    const unsigned char* get_ip_addr(int * af) const;
    unsigned short get_portnumber() const; 
private:
    unsigned char  m_ip[PL_IPv6_ADDR_LEN];
    int            m_af;
    int            m_type;  // Reserved
    unsigned short m_portnumber;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _JOIN_REQ_MSG_H_

