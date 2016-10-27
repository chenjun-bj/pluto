/**
 *******************************************************************************
 * messages.h                                                                  *
 *                                                                             *
 * Message types:                                                              *
 *******************************************************************************
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_


/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <string>
#include <new>

#include <stdio.h>

#include "plexcept.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */
enum class MsgType : int {
    PLUTO_FIRST=0,
    JOINREQ,
    JOINRESP,
    HEARTBEAT,
    PEERLEAVE,
    CREATREQ,
    CREATRESP,
    UPDATEREQ,
    UPDATERESP,
    READREQ,
    READRESP,
    DELETEREQ,
    DELETERESP,
    PLUTO_LAST,
    INVTYPE=PLUTO_LAST
};

enum class MsgStatus : int {
    PLUTO_FIRST=0,
    OK=PLUTO_FIRST,
    ERROR,
    PLUTO_LAST,
};

#define PLUTO_MSG_MAGIC   0X504C5401

typedef struct {
    int magic;
    int version;
    int type;
    int reserved;
} MsgCommonHdr;

/**
 *******************************************************************************
 * Class declaraction                                                          *
 *******************************************************************************
 */
inline std::string get_desc_msgtype(MsgType type) 
{
    switch(type) {
    case MsgType::JOINREQ:    return "JOINREQ";
    case MsgType::JOINRESP:   return "JOINRESP";
    case MsgType::HEARTBEAT:  return "HEARTBEAT";
    case MsgType::PEERLEAVE:  return "PEERLEAVE";
    case MsgType::CREATREQ:   return "CREATREQ";
    case MsgType::CREATRESP:  return "CREATRESP";
    case MsgType::UPDATEREQ:  return "UPDATEREQ";
    case MsgType::UPDATERESP: return "UPDATERESP";
    case MsgType::READREQ:    return "READREQ";
    case MsgType::READRESP:   return "READRESP";
    case MsgType::DELETEREQ:  return "DELETEREQ";
    case MsgType::DELETERESP: return "DELETERESP";
    default: return "Unknown";
    }
}

class Message {
public:
    Message(unsigned char* msg, size_t sz, bool managebuf = true) :
       m_pbuf(msg), m_msgsize(sz), m_managebuf(managebuf) {
        m_hdr = { 0 };
    }

    Message(MsgType type, int version=PLUTO_CURRENT_VERSION, int magic=PLUTO_MSG_MAGIC): m_pbuf(nullptr), m_managebuf(false)  {
        m_hdr.magic   = magic;
        m_hdr.version = version;
        m_hdr.type    = static_cast<int>(type);
        m_hdr.reserved= 0;
    }

    virtual ~Message() {
        if (m_pbuf && m_managebuf) {
            delete [] m_pbuf;
        }
    }

    Message(const Message& other) = delete;
    Message& operator=(const Message& other) = delete;

    Message(Message&& other) : m_pbuf(other.m_pbuf), 
                               m_msgsize(0), 
                               m_managebuf(other.m_managebuf),
                               m_hdr(other.m_hdr) {
        other.m_pbuf = nullptr;
    }

    Message& operator=(Message&& other) {
        if (this != &other) {
            if (m_pbuf && m_managebuf) delete [] m_pbuf;
            m_pbuf = other.m_pbuf;
            m_managebuf  = other.m_managebuf;
            other.m_pbuf = nullptr;
            m_hdr = other.m_hdr;
            m_msgsize  = other.m_msgsize;
        }
        return *this;
    }

    unsigned char* msgbodyptr() {
        if (m_pbuf) {
            return m_pbuf + sizeof(MsgCommonHdr);
        }
        return nullptr;
    }

    size_t msgbodysize() {
        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        if (m_msgsize>MSGHDRSZ) {
            return m_msgsize - MSGHDRSZ;
        }
        return 0;
    }

    int build_msg() {
        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        m_msgsize = get_bodysize() + MSGHDRSZ;
        if (m_msgsize < MSGHDRSZ) {
            return -1;
        }
        if (m_pbuf && m_managebuf) {
            delete [] m_pbuf;
        }
        m_pbuf = new(std::nothrow) unsigned char[m_msgsize];
        if (m_pbuf == nullptr) {
            return -1;
        } 
        m_managebuf = true;

        int rc = build_msg_hdr();
        if (rc != 0) {
            return -1;
        }

        rc = build_msg_body(m_pbuf+MSGHDRSZ, m_msgsize-MSGHDRSZ);
        if (rc != 0) {
            return -1;
        }

        return 0;
    }

    int build_msg_hdr() {
        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        if (m_pbuf) {
            memcpy(m_pbuf, &m_hdr, MSGHDRSZ);
            return 0;
        }
        return -1;
    }

    virtual int build_msg_body(unsigned char* buf, size_t size) = 0;
    
    virtual size_t get_bodysize() const = 0;

    virtual void parse_msg() throw (parse_error) {
        if (m_pbuf) {
            const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
            parse_msg_hdr();
            parse_msg_body(m_pbuf+MSGHDRSZ, m_msgsize-MSGHDRSZ);
        } else {
            throw parse_error("null buffer");
        }
    }

    void parse_msg_hdr() throw(parse_error) {
        if (!m_pbuf) {
            throw parse_error("null buffer");
        }

        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        if (m_msgsize < MSGHDRSZ) {
            throw parse_error("in-complete header received");
        }

        MsgCommonHdr *pHdr = reinterpret_cast<MsgCommonHdr*>(m_pbuf);
        if (pHdr == nullptr) {
            throw parse_error("internal error, cast failed");
        }
        m_hdr.magic   = pHdr->magic;
        m_hdr.version = pHdr->version;

        int low, hi;
        low = static_cast<int>(MsgType::PLUTO_FIRST);
        hi  = static_cast<int>(MsgType::PLUTO_LAST);
        if ((pHdr->type <= low) || (pHdr->type >= hi)) {
            std::string errmsg="invalid type value " + std::to_string(pHdr->type);
            throw parse_error(errmsg.c_str());
        }
        m_hdr.type = pHdr->type;
    }

    virtual void parse_msg_body(unsigned char*, size_t) 
                     throw(parse_error) = 0;

    void dump(int (*output)(const char*, ...)=printf,
              bool verbose=false) const {
        if (m_pbuf) {
            if (verbose) {
                dump_memory(NULL, (const char*)m_pbuf, m_msgsize);
            }
            dump_hdr(output, verbose);
            dump_body(output, verbose);
            
        }
    }

    void dump_hdr(int (*output)(const char*, ...)=printf, 
                  bool verbose=false) const {
        output("Magic   : %0X\n", get_magic());
        output("Version : %0X\n", get_version());
        output("MsgType : %s\n", get_desc_msgtype(get_msgtype()).c_str());
    }

    virtual void dump_body(int (*output)(const char*, ...)=printf,
                           bool verbose=false) const = 0;
    
    int get_magic() const {
        return m_hdr.magic;
    }
    int get_version() const {
        return m_hdr.version;
    }
    MsgType get_msgtype() const {
        return static_cast<MsgType>(m_hdr.type);
    }

    unsigned char* get_raw() const {
        return m_pbuf;
    }
private:
    unsigned char* m_pbuf;
    size_t         m_msgsize;
    bool           m_managebuf;
    MsgCommonHdr   m_hdr;
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _MESSAGES_H_

