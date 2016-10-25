/**
 *******************************************************************************
 * MsgCommon.h                                                                 *
 *                                                                             *
 * Membership message commons                                                  *
 *******************************************************************************
 */

#ifndef _MSG_COMMON_H_
#define _MSG_COMMON_H_

/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <string>
#include <stdexcept>

#include "stdinclude.h"
#include "pladdress.h"
#include "plexcept.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Class declaraction                                                          *
 *******************************************************************************
 */

class HeartMsgStruct {
public:
    HeartMsgStruct(const unsigned char* buf, const size_t size) : m_addr(buf, size) {
        parse(buf, size);
    };
  
    HeartMsgStruct(int64 hb, Address& addr) : m_addr(addr) {
        set_heartbeat(hb);
    }

    /*HeartMsgStruct(const HeartMsgStruct& other):
       m_addr(other.m_addr),
       m_heartbeat(other.m_heartbeat) {
    }

    HeartMsgStruct& operator=(const HeartMsgStruct& other) {
        if (this != &other) {
            m_addr      = other.m_addr;
            m_heartbeat = other.m_heartbeat;
        }
        return *this;
    }

    HeartMsgStruct(HeartMsgStruct&& other):
       m_addr(other.m_addr),
       m_heartbeat(other.m_heartbeat) {
    }

    HeartMsgStruct& operator=(HeartMsgStruct&& other) {
        if (this != &other) {
            m_addr      = other.m_addr;
            m_heartbeat = other.m_heartbeat;
        }
        return *this;
    }*/

    void parse(const unsigned char*buf, const size_t size) throw (parse_error) 
    {
        /* Format 
         * Address - refer pladdress.h
         * int64   - heartbeat
         */
        if (buf==nullptr) {
            throw parse_error("null buffer");
        }

        unsigned int len = get_required_buf_len();
        if (size < len) {
            throw parse_error("size error");
        }

        buf += m_addr.get_required_buf_len();

        int64  llval;
        int32  lval1, lval2;
        memcpy(&lval1, buf, sizeof(int32));
        buf += sizeof(int32);
        memcpy(&lval2, buf, sizeof(int32));
        buf += sizeof(int32);

        llval = ntohl(lval1);
        lval2 = ntohl(lval2);
        llval = llval << 32 | lval2;
        m_heartbeat = llval;
    }

    // serialize 
    void build(unsigned char* buf, const size_t size)
    {
        unsigned req_len = get_required_buf_len();
        
        if (req_len == 0) {
            throw std::length_error("internal size error");
        }

        if (size < req_len) {
            throw std::length_error("no enough size");
        }

        if (buf == nullptr) {
            throw std::invalid_argument("nullptr");
        }

        m_addr.build(buf, size);
        buf += m_addr.get_required_buf_len();

        int64  llval = m_heartbeat;
        int32  lval1, lval2;

        lval1 = llval >> 32 & 0xFFFFFFFF;
        lval1 = htonl(lval1);
        lval2 = llval & 0xFFFFFFFF;
        lval2 = htonl(lval2);
        memcpy(buf, &lval1, sizeof(int32));
        buf += sizeof(int32);
        memcpy(buf, &lval2, sizeof(int32));
        buf += sizeof(int32);
    }

    size_t get_required_buf_len() const  {
        return m_addr.get_required_buf_len() + sizeof(int64);
    }

    void set_address(const Address& addr) {
        m_addr = addr;
    }

    void set_heartbeat(int64 hb) {
        m_heartbeat = hb;
    }

    Address get_address() const {
        return m_addr;
    }

    int64 get_heartbeat() const {
        return m_heartbeat;
    }

    void dump(int (*output)(const char*,...)=printf) const {
        m_addr.dump(output);
        output("Heartbeat: %ld\n", m_heartbeat);
    }
private:
    Address   m_addr;
    int64     m_heartbeat;
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _MSG_COMMON_H_

