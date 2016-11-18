/**
 *******************************************************************************
 * KVMessage.h                                                                 *
 *                                                                             *
 * Key value message commons                                                   *
 *******************************************************************************
 */

#ifndef _KEY_VALUE_MSG_H_
#define _KEY_VALUE_MSG_H_

/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <string>
#include <vector>

#include "stdinclude.h"
#include "StoreMessage.h"
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

class KVReqMessage : public StoreMessage {
public:
    KVReqMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       StoreMessage(buf, sz, managebuf),
       m_key(),
       m_value() {
    };
  
    KVReqMessage(MsgType type,
                 MessageOriginator originator,
                 int64 txid) :
       StoreMessage(type, originator, txid),
       m_key(),
       m_value() {
    }

    virtual ~KVReqMessage() {
    }

    void set_key(const std::string& key) {
        m_key = key;
    }

    const std::string& get_key() const {
        return m_key;
    }

    void set_value(const unsigned char* val, size_t sz) {
        m_value.clear();
        if (val && sz>0) {
            m_value.resize(sz);
            unsigned char* buf = m_value.data();
            memcpy(buf, val, sz);
        }
    }

    int get_value(unsigned char* val, size_t & sz) {
        if (val==nullptr) return -1;
        if (sz < m_value.size()) return -1;
        sz = m_value.size();
        memcpy(val, m_value.data(), sz);
        return 0;
    }

    int build_storemsg_body(unsigned char* buf, size_t sz) {
        /**
         * Format:
         *   uint64 -- key size
         *   uint64 -- value size
         *   unsigned char array -- key
         *   unsigned char array -- value
         *   pad to 4 bytes
         */
        if (buf == nullptr) {
            getlog()->sendlog(LogLevel::ERROR, "Create message, build body nullptr received\n");
            return -1;
        }

        if (sz < get_storemsg_bodysize()) {
            getlog()->sendlog(LogLevel::ERROR, "Create message, build body no enough buffer, size=%d, required %d\n",
                                                sz, get_storemsg_bodysize());
            return -1;
        }

        network_write_int64(buf, m_key.size());
        buf += sizeof(int64);

        network_write_int64(buf, m_value.size());
        buf += sizeof(int64);

        memcpy(buf, m_key.c_str(), m_key.size());
        buf += m_key.size();

        memcpy(buf, m_value.data(), m_value.size());
        buf += m_value.size();

        size_t wrtlen = 2*sizeof(int64) + m_key.size() + m_value.size();
        if (get_storemsg_bodysize() > wrtlen) {
            memset(buf, 0, (get_storemsg_bodysize() - wrtlen) * sizeof(char));
        }

        return 0;
    }

    void parse_storemsg_body(const unsigned char* buf, const size_t sz)
       throw (parse_error) {
        if (buf == nullptr) {
            throw parse_error("KVReqMessage: parse got null ptr!");
        }

        if (sz < 2*sizeof(int64)) {
            throw parse_error("KVReqMessage: invalid length expected: " + std::to_string(2*sizeof(int64)));
        }

        int64 keylen, vallen;
        keylen = network_read_int64(buf);
        buf += sizeof(int64);

        vallen = network_read_int64(buf);
        buf += sizeof(int64);

        if (sz < keylen + vallen + 2*sizeof(int64)) {
            throw parse_error("KVReqMessage: in-complete message, received length: " + std::to_string(sz));
        }

        std::string mstr((const char*)buf, keylen);
        buf += keylen;

        m_key = std::move(mstr);

        m_value.clear();
        m_value.resize(vallen);

        memcpy(m_value.data(), buf, vallen);
        buf += vallen;

        // no need to read padding
    }

    size_t get_storemsg_bodysize() const {
        size_t sz = m_key.size() + m_value.size();
        sz = (sz + 3) / 4 * 4;
        return sz + 2 * sizeof(uint64);
    }

    void dump_storemsg_body(int (*output)(const char*, ...)=printf,
                            bool verbose=false) const {
        output("Key  : '%s'\n", m_key.c_str());
        output("Value:\n");
        for (auto v : m_value) {
            output("%02X ", v);
        }
        if (verbose) {
            dump_memory("VALUE", (const char*)m_value.data(), m_value.size(), output);
        }
    }
private:
    std::string                m_key;
    std::vector<unsigned char> m_value;
};

class KVRespMessage : public StoreMessage {
public:
    KVRespMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       StoreMessage(buf, sz, managebuf),
       m_status(MsgStatus::OK) {
    }
  
    KVRespMessage(MsgType type,
                  MessageOriginator originator,
                  int64 txid,
                  MsgStatus status) : 
       StoreMessage(type, originator, txid),
       m_status(status) {
    }

    virtual ~KVRespMessage() {
    }

    void set_status(MsgStatus status) {
        m_status = status;
    }

    MsgStatus get_status() const {
        return m_status;
    }

    int build_storemsg_body(unsigned char* buf, size_t sz) {
        /**
         * Format:
         *  int32 -- status
         */
        if (buf == nullptr) {
            getlog()->sendlog(LogLevel::ERROR, "Key value response message, build body nullptr received\n");
            return -1;
        }

        if (sz < get_storemsg_bodysize()) {
            getlog()->sendlog(LogLevel::ERROR, "Key value response message, build body no enough buffer, size=%d, required %d\n",
                                                sz, get_storemsg_bodysize());
            return -1;
        }
        int32 ival = static_cast<int32>(m_status);
        ival = htonl(ival);
        memcpy(buf, &ival, sizeof(ival));

        return 0;
    }

    void parse_storemsg_body(const unsigned char* buf, const size_t sz)
       throw (parse_error) {
        if (buf == nullptr) {
            throw parse_error("KVReqMessage: parse got null ptr!");
        }

        if (sz < sizeof(int32)) {
            throw parse_error("KVRespMessage: invalid length expected: " + std::to_string(2*sizeof(int64)));
        }

        int32 ival;
        memcpy(&ival, buf, sizeof(int32));
        ival = ntohl(ival);
        buf += sizeof(int32);

        int hi, low;
        low = static_cast<int>(MsgStatus::PLUTO_FIRST);
        hi  = static_cast<int>(MsgStatus::PLUTO_LAST);
        if (ival<=low || ival>hi) {
            throw parse_error("KVRespMessage: invalid status message");
        }

        m_status = static_cast<MsgStatus>(ival);

    }

    size_t get_storemsg_bodysize() const {
        return sizeof(int32);
    }

    void dump_storemsg_body(int (*output)(const char*, ...)=printf,
                            bool verbose=false) const {
        output("Status: '%d'\n", m_status);
    }
private:
    MsgStatus m_status;
};

class KeyReqMessage : public StoreMessage {
public:
    KeyReqMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       StoreMessage(buf, sz, managebuf),
       m_key() {
    };
  
    KeyReqMessage(MsgType type,
                  MessageOriginator originator,
                  int64 txid) :
       StoreMessage(type, originator, txid),
       m_key() {
    }

    virtual ~KeyReqMessage() {
    }

    void set_key(const std::string& key) {
        m_key = key;
    }

    const std::string& get_key() const {
        return m_key;
    }

    int build_storemsg_body(unsigned char* buf, size_t sz) {
        /**
         * Format:
         *  int64 -- key length
         *  char array -- keys
         *  pad -- to 4 bytes
         */
        if (buf == nullptr) {
            getlog()->sendlog(LogLevel::ERROR, "Key request message, build body nullptr received\n");
            return -1;
        }

        if (sz < get_storemsg_bodysize()) {
            getlog()->sendlog(LogLevel::ERROR, "Key request message, build body no enough buffer, size=%d, required %d\n",
                                                sz, get_storemsg_bodysize());
            return -1;
        }

        network_write_int64(buf, m_key.size());
        buf += sizeof(int64);

        memcpy(buf, m_key.c_str(), m_key.size());
        buf += m_key.size();

        if (get_storemsg_bodysize() > sizeof(int64) + m_key.size()) {
            memset(buf, 0, get_storemsg_bodysize() - sizeof(int64) - m_key.size());
        }
        return 0;
    }

    void parse_storemsg_body(const unsigned char* buf, const size_t sz)
       throw (parse_error) {
        if (buf == nullptr) {
            throw parse_error("KeyReqMessage: parse got null ptr!");
        }

        if (sz < sizeof(int64)) {
            throw parse_error("KeyReqMessage: invalid length expected: " + std::to_string(sizeof(int64)));
        }

        int64 lval = network_read_int64(buf);
        buf += sizeof(int64);

        if (sz < lval + sizeof(int64)) {
            throw parse_error("KeyReqMessage: in-complete message, recevied size: " + std::to_string(sz));
        }
        std::string mstr((const char*)buf, lval);
        m_key = mstr;

        // No need to read paddings
    }

    size_t get_storemsg_bodysize() const {
        return sizeof(int64) + (m_key.size()+3)/4*4;
    }

    void dump_storemsg_body(int (*output)(const char*, ...)=printf,
                            bool verbose=false) const {
        output("Key  : '%s'\n", m_key.c_str());
    }
private:
    std::string m_key;
};

class KeyRespMessage : public StoreMessage {
public:
    KeyRespMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       StoreMessage(buf, sz, managebuf),
       m_value(),
       m_status(MsgStatus::OK) {
    };
  
    KeyRespMessage(MsgType type,
                   MessageOriginator originator,
                   int64 txid,
                   MsgStatus status) :
       StoreMessage(type, originator, txid),
       m_value(),
       m_status(status) {
    }

    virtual ~KeyRespMessage() {
    }

    void set_value(const unsigned char* val, size_t sz) {
        m_value.clear();
        if (val && sz>0) {
            m_value.resize(sz);
            unsigned char* buf = m_value.data();
            memcpy(buf, val, sz);
        }
    }

    int get_value(unsigned char* val, size_t & sz) {
        if (val==nullptr) return -1;
        if (sz < m_value.size()) return -1;
        sz = m_value.size();
        memcpy(val, m_value.data(), sz);
        return 0;
    }

    void set_status(MsgStatus status) {
        m_status = status;
    }

    MsgStatus get_status() const {
        return m_status;
    }

    int build_storemsg_body(unsigned char* buf, size_t sz) {
        /**
         * Format:
         *  int32 -- status
         *  int32 -- reserved
         *  int64 -- value length
         *  char array -- keys
         *  pad -- to 4 bytes
         */
        if (buf == nullptr) {
            getlog()->sendlog(LogLevel::ERROR, "Key response message, build body nullptr received\n");
            return -1;
        }

        if (sz < get_storemsg_bodysize()) {
            getlog()->sendlog(LogLevel::ERROR, "Key response message, build body no enough buffer, size=%d, required %d\n",
                                                sz, get_storemsg_bodysize());
            return -1;
        }

        int32 ival = htonl(static_cast<int32>(m_status));
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        ival = 0; 
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        network_write_int64(buf, m_value.size());
        buf += sizeof(int64);

        memcpy(buf, m_value.data(), m_value.size());
        buf += m_value.size();

        if (get_storemsg_bodysize() > sizeof(int64) + m_value.size()) {
            memset(buf, 0, get_storemsg_bodysize() - sizeof(int64) - m_value.size());
        }
        return 0;
    }

    void parse_storemsg_body(const unsigned char* buf, const size_t sz)
       throw (parse_error) {
        if (buf == nullptr) {
            throw parse_error("KeyRespMessage: parse got null ptr!");
        }

        if (sz < sizeof(int64)) {
            throw parse_error("KeyRespMessage: invalid length expected: " + std::to_string(sizeof(int64)));
        }

        int32 ival;
        memcpy(&ival, buf, sizeof(int32));
        buf += sizeof(int32);
        ival = ntohl(ival);

        int hi, low;
        low = static_cast<int>(MsgStatus::PLUTO_FIRST);
        hi  = static_cast<int>(MsgStatus::PLUTO_LAST);
        if (ival<=low || ival>hi) {
            throw parse_error("KVRespMessage: invalid status message");
        }
        m_status = static_cast<MsgStatus>(ival);
       
        // The reserved field 
        memcpy(&ival, buf, sizeof(int32));
        buf += sizeof(int32);

        int64 lval = network_read_int64(buf);
        buf += sizeof(int64);

        if (sz < lval + sizeof(int64)) {
            throw parse_error("KeyRespMessage: in-complete message, recevied size: " + std::to_string(sz));
        }

        m_value.clear();
        m_value.resize(lval);

        memcpy(m_value.data(), buf, lval);
        buf += lval;
        // No need to read paddings
    }

    size_t get_storemsg_bodysize() const {
        return 2*sizeof(int32) + sizeof(int64) + (m_value.size()+3)/4*4;
    }

    void dump_storemsg_body(int (*output)(const char*, ...)=printf,
                            bool verbose=false) const {
        output("Value:");
        for (auto v : m_value) {
            output("%02X", v);
        }
        if (verbose) {
            dump_memory("VALUE", (const char*)m_value.data(), m_value.size(), output);
        }
    }
private:
    std::vector<unsigned char> m_value;
    MsgStatus                  m_status;
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _KEY_VALUE_MSG_H_

