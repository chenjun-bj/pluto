/**
 *******************************************************************************
 * PeerLeave.h                                                                 *
 *                                                                             *
 * Peer leave message                                                          *
 *******************************************************************************
 */

#ifndef _PEERLEAVE_MESG_H_
#define _PEERLEAVE_MESG_H_

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

class PeerLeaveMessage : public Message{
public:
    enum LeaveReason : int {
        PLUTO_FIRST= 0,
        NORMAL_SHUTDOWN=PLUTO_FIRST,
        ERROR_DOWN,
        PLUTO_LAST
    };
public:
    PeerLeaveMessage(unsigned char* msg, size_t sz, bool managebuf = true);
    PeerLeaveMessage(LeaveReason reason, 
                     int af, unsigned short port, unsigned char* ip);
    ~PeerLeaveMessage();

    PeerLeaveMessage(const PeerLeaveMessage& other) = delete;
    PeerLeaveMessage& operator=(const PeerLeaveMessage& other) = delete;

    PeerLeaveMessage( PeerLeaveMessage&& other);
    PeerLeaveMessage& operator=(PeerLeaveMessage&& other);

    virtual size_t get_bodysize() const;

    virtual int build_msg_body(unsigned char* buf, size_t size);

    virtual void parse_msg_body(unsigned char* buf, size_t size) 
                              throw(parse_error);

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const;

    /* IPv4: 4 bytes, IPv6: 16 bytes */
    void set_ip_addr(int af, unsigned char* ip);
    void set_port(unsigned short port);
    void set_leave_reason(LeaveReason reason) {
        m_reason = reason;
    }

    const unsigned char* get_ip_addr(int * af) const;
    unsigned short get_port() const;
    LeaveReason get_leave_reason() const {
        return m_reason;
    }

private:
    LeaveReason m_reason;
    Address     m_addr;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _PEERLEAVE_MESG_H_

