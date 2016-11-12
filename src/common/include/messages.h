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
#include <tuple>
#include <utility>

#include <boost/asio.hpp>
#include <boost/logic/tribool.hpp>

#include <cstdio>

#include "plexcept.h"
#include "pltypes.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */
enum class MsgType : uint32 {
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
    uint32 magic;
    uint32 version;
    uint32 type;
    uint32 size;  // Total message length
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
       m_pbuf(msg), m_bufsize(sz), m_managebuf(managebuf), m_hdr(),
       m_src_addr(), m_dest_addr(), m_src_port(0), m_dest_port(0)  {
        m_hdr = { 0 };
    }

    Message(MsgType type, int version=PLUTO_CURRENT_VERSION, int magic=PLUTO_MSG_MAGIC): 
       m_pbuf(nullptr), m_bufsize(0), m_managebuf(false), m_hdr(),  
       m_src_addr(), m_dest_addr(), m_src_port(0), m_dest_port(0)  {
        m_hdr.magic   = magic;
        m_hdr.version = version;
        m_hdr.type    = static_cast<uint32>(type);
        m_hdr.size    = 0;  // The size is computed at build stage
    }

    virtual ~Message() {
        if (m_pbuf && m_managebuf) {
            delete [] m_pbuf;
        }
    }

    Message(const Message& other) = delete;
    Message& operator=(const Message& other) = delete;

    Message(Message&& other) : m_pbuf(other.m_pbuf), 
                               m_bufsize(0), 
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
            m_bufsize  = other.m_bufsize;
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
        if (m_bufsize>MSGHDRSZ) {
            return m_bufsize - MSGHDRSZ;
        }
        return 0;
    }

    int build_msg() {
        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        m_bufsize = get_bodysize() + MSGHDRSZ;
        if (m_bufsize < MSGHDRSZ) {
            return -1;
        }
        if (m_pbuf && m_managebuf) {
            delete [] m_pbuf;
        }
        m_pbuf = new(std::nothrow) unsigned char[m_bufsize];
        if (m_pbuf == nullptr) {
            return -1;
        } 
        m_managebuf = true;

        int rc = build_msg_hdr();
        if (rc != 0) {
            return -1;
        }

        rc = build_msg_body(m_pbuf+MSGHDRSZ, m_bufsize-MSGHDRSZ);
        if (rc != 0) {
            return -1;
        }

        return 0;
    }

    int build_msg_hdr() {
        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        if (m_pbuf) {
            m_hdr.size = m_bufsize;
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
            if (m_hdr.size < m_bufsize) {
                throw parse_error("in-complete message");
            }
            parse_msg_body(m_pbuf+MSGHDRSZ, m_bufsize-MSGHDRSZ);
        } else {
            throw parse_error("null buffer");
        }
    }

    void parse_msg_hdr() throw(parse_error) {
        if (!m_pbuf) {
            throw parse_error("null buffer");
        }

        const unsigned MSGHDRSZ = sizeof(MsgCommonHdr);
        if (m_bufsize < MSGHDRSZ) {
            throw parse_error("in-complete header received");
        }

        MsgCommonHdr *pHdr = reinterpret_cast<MsgCommonHdr*>(m_pbuf);
        if (pHdr == nullptr) {
            throw parse_error("internal error, cast failed");
        }
        m_hdr.magic   = pHdr->magic;
        m_hdr.version = pHdr->version;

        uint32 low, hi;
        low = static_cast<uint32>(MsgType::PLUTO_FIRST);
        hi  = static_cast<uint32>(MsgType::PLUTO_LAST);
        if ((pHdr->type <= low) || (pHdr->type >= hi)) {
            std::string errmsg="invalid type value " + std::to_string(pHdr->type);
            throw parse_error(errmsg.c_str());
        }
        m_hdr.type = pHdr->type;
        m_hdr.size = pHdr->size;
    }

    virtual void parse_msg_body(unsigned char*, size_t) 
                     throw(parse_error) = 0;

    void dump(int (*output)(const char*, ...)=printf,
              bool verbose=false) const {
        if (m_pbuf) {
            dump_memory(NULL, (const char*)m_pbuf, m_bufsize);
            if (verbose) {
                dump_address(output, verbose);
                dump_hdr(output, verbose);
                dump_body(output, verbose);
            }
        }
    }

    void dump_hdr(int (*output)(const char*, ...)=std::printf, 
                  bool verbose=false) const {
        output("Magic   : %0X\n", get_magic());
        output("Version : %0X\n", get_version());
        output("MsgType : %s\n", get_desc_msgtype(get_msgtype()).c_str());
        output("Length  : %d\n", m_hdr.size);
    }

    void dump_address(int (*output)(const char*, ...)=std::printf, 
                      bool verbose=false) const {
        output("Source      : %s:%d\n", m_src_addr.to_string().c_str(), m_src_port);
        output("Destination : %s:%d\n", m_dest_addr.to_string().c_str(), m_dest_port);
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

    size_t get_size() const {
        return m_bufsize;
    }

    void set_source(const boost::asio::ip::address & addr, unsigned short port) {
        m_src_addr = addr;
        m_src_port = port;
    }

    std::pair<boost::asio::ip::address, unsigned short>  get_source() const {
        return std::make_pair(m_src_addr, m_src_port);
    }

    void set_destination(const boost::asio::ip::address & addr, unsigned short port) {
        m_dest_addr = addr;
        m_dest_port = port;
    }

    std::pair<boost::asio::ip::address, unsigned short>  get_destination() const {
        return std::make_pair(m_dest_addr, m_dest_port);
    }
    
private:
    unsigned char* m_pbuf;
    size_t         m_bufsize;
    bool           m_managebuf;
    MsgCommonHdr   m_hdr;

    boost::asio::ip::address m_src_addr;
    boost::asio::ip::address m_dest_addr;
    unsigned short m_src_port;
    unsigned short m_dest_port;
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

inline std::tuple<boost::tribool, size_t, MsgType>
try_parse(const unsigned char* buf, size_t size) 
{
    if (buf == nullptr) {
        return std::make_tuple( false, 0, MsgType::INVTYPE );
    }

    if (size < sizeof(MsgCommonHdr)) {
        return std::make_tuple( boost::indeterminate, 0, MsgType::INVTYPE );
    }

    const MsgCommonHdr *pHdr = reinterpret_cast<const MsgCommonHdr*>(buf);
    if (pHdr == nullptr) {
        return std::make_tuple( false, 0, MsgType::INVTYPE );
    }

    uint32 low, hi;
    low = static_cast<uint32>(MsgType::PLUTO_FIRST);
    hi  = static_cast<uint32>(MsgType::PLUTO_LAST);
    if ((pHdr->type <= low) || (pHdr->type >= hi)) {
        return std::make_tuple( false, 0, MsgType::INVTYPE );
    }

    if (size < pHdr->size) {
        return std::make_tuple( boost::indeterminate, pHdr->size, MsgType::INVTYPE );
    }

    return std::make_tuple( true, pHdr->size, static_cast<MsgType>(pHdr->type));
}

#endif // _MESSAGES_H_

