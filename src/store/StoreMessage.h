/**
 *******************************************************************************
 * StoreMessage.h                                                              *
 *                                                                             *
 * Store message commons                                                       *
 *******************************************************************************
 */

#ifndef _STORE_MSG_COMMON_H_
#define _STORE_MSG_COMMON_H_

/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <string>
#include <stdexcept>
#include <memory>

#include "stdinclude.h"
#include "messages.h"
#include "plexcept.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Forward declaraction                                                        *
 *******************************************************************************
 */
class Connection;

/**
 *******************************************************************************
 * Class declaraction                                                          *
 *******************************************************************************
 */

enum class MessageOriginator : int {
    PLUTO_FIRST = 0,
    Client = PLUTO_FIRST,
    Server,
    PLUTO_LAST
};

inline std::string get_originator_desc(MessageOriginator orig) {
    switch(orig) {
    case MessageOriginator::Client: return "Client";
    case MessageOriginator::Server: return "Server";
    default: return "Unknown: " + std::to_string(static_cast<int>(orig));
    }
    return "Bad";
}

class StoreMessage : public Message {
public:
    StoreMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
        Message(buf, sz, managebuf), 
        m_txid(-1),
        m_replica_type(-1),
        m_originator(MessageOriginator::Client),
        m_pconn(nullptr) {
    };
  
    StoreMessage(MsgType type, 
                 MessageOriginator originator,
                 int64 txid,
                 int version=PLUTO_CURRENT_VERSION, 
                 int magic=PLUTO_MSG_MAGIC) :
       Message(type, version, magic), 
       m_txid(txid),
       m_replica_type(-1),
       m_originator(originator),
       m_pconn(nullptr)
    {
    }

    virtual ~StoreMessage() {
        m_pconn.reset();
    }

    void set_originator(MessageOriginator originator) {
        m_originator = originator;
    }

    MessageOriginator get_originator() const {
        return m_originator;
    }

    void set_txid(int64 txid) {
        m_txid = txid;
    }

    int64 get_txid() const {
        return m_txid;
    }

    int set_replica_type(int32 tp) {
        if ((tp<0) || (tp>=PLUTO_NODE_REPLICAS_NUM)) {
            return -1;
        }
        m_replica_type = tp;
        return 0;
    }

    int32 get_replica_type() const {
        return m_replica_type;
    }

    void set_connection(std::shared_ptr<Connection > pconn) {
        m_pconn = pconn;
    }

    std::shared_ptr<Connection > get_connection() const {
        return m_pconn;
    }

    boost::asio::ip::tcp::endpoint get_dest_endpoint() const {
        std::pair<boost::asio::ip::address, unsigned short> addr = get_destination();
        return boost::asio::ip::tcp::endpoint(addr.first, addr.second);
    }

    int build_msg_body(unsigned char* buf, size_t sz) {
        // format: int64 -- txid
        //         int32 -- originator
        //         int32 -- reserved 
        if (buf == nullptr) {
            getlog()->sendlog(LogLevel::ERROR, "Store message, build body nullptr received\n");
            return -1;
        }

        if (sz < get_bodysize()) {
            getlog()->sendlog(LogLevel::ERROR, "Store message, build body no enough buffer, size=%d, required %d\n",
                                                sz, get_bodysize());
            return -1;
        }

        network_write_int64(buf, m_txid);
        buf += sizeof(int64);

        int32 ival = static_cast<int32>(m_originator);
        ival = htonl(ival);
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        ival = htonl(m_replica_type);
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        return build_storemsg_body(buf, sz - get_storemsg_hdrsize());
    }

    virtual int build_storemsg_body(unsigned char* buf, size_t sz) = 0;

    void parse_msg_body(unsigned char* buf, size_t sz) throw (parse_error) {
        if (buf==nullptr) {
            throw parse_error("StoreMessage: parse null buffer");
        }

        if (sz < get_storemsg_hdrsize()) {
            throw parse_error("StoreMessage: size error");
        }

        m_txid = network_read_int64(buf);
        buf += sizeof(int64);

        int32 ival;
        memcpy(&ival, buf, sizeof(int32));
        ival = ntohl(ival);
        buf += sizeof(int32);

        int hi, low;
        low = static_cast<int>(MessageOriginator::PLUTO_FIRST);
        hi  = static_cast<int>(MessageOriginator::PLUTO_LAST);
        if ((ival < low) || (ival >= hi)) {
            throw parse_error("StoreMessage: invalid originator " + std::to_string(ival));
        }
        m_originator = static_cast<MessageOriginator>(ival);
        // reserved
        memcpy(&ival, buf, sizeof(int32));
        ival = ntohl(ival);
        buf += sizeof(int32);

        m_replica_type = ival;

        parse_storemsg_body(buf, sz - get_storemsg_hdrsize());
    }

    virtual void parse_storemsg_body(const unsigned char* buf, const size_t sz)
       throw (parse_error) = 0;

    size_t get_bodysize() const {
        return get_storemsg_hdrsize() + get_storemsg_bodysize();
    }

    size_t get_storemsg_hdrsize() const {
        return sizeof(int64) + sizeof(int32)*2;
    }

    virtual size_t get_storemsg_bodysize() const = 0;

    void dump_body(int (*output)(const char*, ...)=printf,
                   bool verbose=false) const {
        output("Originator   : '%s'", get_originator_desc(m_originator).c_str());
        output("TransactionId: '%ld'", m_txid);
        dump_storemsg_body(output, verbose);
    }
    virtual void dump_storemsg_body(int (*output)(const char*, ...)=printf,
                                    bool verbose=false) const = 0;
protected:
    void network_write_int64(unsigned char* buf, int64 val) {
        int32 ival;

        ival = val >> 32 & 0xFFFFFFFF;
        ival = htonl(ival);
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);

        ival = val & 0xFFFFFFFF;
        ival = htonl(ival);
        memcpy(buf, &ival, sizeof(int32));
        buf += sizeof(int32);
    }

    int64 network_read_int64(const unsigned char* buf) {
        int32 ival1, ival2;

        memcpy(&ival1, buf, sizeof(int32));
        ival1 = ntohl(ival1);
        buf += sizeof(int32);
        memcpy(&ival2, buf, sizeof(int32));
        ival2 = ntohl(ival2);

        int64 lval = ival1;
        lval = lval<<32 | ival2;
        return lval;
    }
private:
    int64               m_txid;
    int32               m_replica_type;
    MessageOriginator   m_originator;
    std::shared_ptr<Connection > m_pconn;
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _STORE_MSG_COMMON_H_

