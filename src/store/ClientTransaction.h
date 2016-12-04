/**
 *******************************************************************************
 * ClientTransaction.h                                                         *
 *                                                                             *
 * Client transaction:                                                         *
 *   - client transactions, tracks a client request                            *
 *******************************************************************************
 */

#ifndef _CLIENT_TRANSACTION_H_
#define _CLIENT_TRANSACTION_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"
#include "memberlist.h"
#include "StoreClient.h"

#include <tuple>
#include <utility>
#include <map>
#include <chrono>
#include <boost/asio.hpp>

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

class ClientTransaction {
public:
    enum REQUEST_TYPE {
        CREAT = 0,
        READ,
        UPDATE,
        DELETE
    };

    ClientTransaction(StoreMessage * pmsg,
                      REQUEST_TYPE   tp,
                      const std::vector<struct MemberEntry > & nodes) :
       m_pmsg(pmsg),
       m_txid(reinterpret_cast<long long>(pmsg)),
       m_type(tp),
       m_crttm(std::chrono::system_clock::now()),
       m_waitcnt(0),
       m_rplycnt(0),
       m_succcnt(0),
       m_rplyst(REPLY_STATE::INITIAL),
       m_nodes(nodes),
       m_replys() {
    }
    ~ClientTransaction() {
        for (auto& r : m_replys) {
            StoreClient * pclt = std::get<1>(r.second);
            if (pclt != nullptr) {
                delete pclt;
            }
        }
        m_replys.clear();
    }

    ClientTransaction(const ClientTransaction& other) = delete;
    ClientTransaction(const ClientTransaction&& other) = delete;

    ClientTransaction& operator=(const ClientTransaction& other) = delete;
    ClientTransaction& operator=(const ClientTransaction&& other) = delete;

    long long get_txid() const {
        return m_txid;
    }

    std::chrono::system_clock::time_point get_creat_time() const {
        return m_crttm;
    }

    // total_reply, success_reply
    std::tuple<int, int > get_reply_count() const {
        return std::make_tuple(m_rplycnt, m_succcnt);
    }

    int get_wait_count() const {
        return m_waitcnt;
    }

    REQUEST_TYPE get_type() const {
        return m_type;
    }

    StoreMessage* get_msg() const {
        return m_pmsg;
    }

    int get_read_value(std::vector<unsigned char>& v) const {
        // TODO: check if read data is same or not
        std::map<boost::asio::ip::tcp::endpoint, NODE_TYPE >::const_iterator it =
            m_replys.begin();
        if (it != m_replys.end()) {
            v = std::get<3>(it->second);
            return 0;
        }
        return -1;
    }

    bool is_client_response() const {
        return m_rplyst == REPLY_STATE::REPLYED;
    }

    void mark_send_clnt_resp() {
        // Now can delete the transaction
        m_rplyst = REPLY_STATE::REPLYED;
    }

    void start_wait_reply(const boost::asio::ip::tcp::endpoint& ep, StoreClient * pclt) {
        m_waitcnt++;
        if (m_rplyst==REPLY_STATE::INITIAL) {
            m_rplyst = REPLY_STATE::WAITING;
        }
        m_replys[ep] = std::make_tuple(REPLY_STATE::WAITING, pclt, -1, std::vector<unsigned char>() );
    }

    int add_reply(StoreMessage* pmsg) {
        if (pmsg==nullptr) return -1;
        int status = 0;
        switch(m_type) {
        case REQUEST_TYPE::CREAT: {
            CreatResponseMessage * presp = 
                dynamic_cast<CreatResponseMessage*>(pmsg);
            if (presp == nullptr) {
                return -1;
            }
            status = static_cast<int>(presp->get_status());
            break;
        }
        case REQUEST_TYPE::READ: {
            ReadResponseMessage * presp = 
                dynamic_cast<ReadResponseMessage*>(pmsg);
            if (presp == nullptr) {
                return -1;
            }
            status = static_cast<int>(presp->get_status());

            std::vector<unsigned char> val;
            presp->get_value(val);
            return add_reply(pmsg->get_dest_endpoint(), status,
                             val.data(), val.size());
        }
        case REQUEST_TYPE::UPDATE: {
            UpdateResponseMessage * presp = 
                dynamic_cast<UpdateResponseMessage*>(pmsg);
            if (presp == nullptr) {
                return -1;
            }
            status = static_cast<int>(presp->get_status());
            break;
        }
        case REQUEST_TYPE::DELETE: {
            DeleteResponseMessage * presp = 
                dynamic_cast<DeleteResponseMessage*>(pmsg);
            if (presp == nullptr) {
                return -1;
            }
            status = static_cast<int>(presp->get_status());
            break;
        }
        default: return -1;
        }

        return add_reply(pmsg->get_dest_endpoint(), status);
    }

    int add_reply(const boost::asio::ip::tcp::endpoint& ep, int status, 
                  const unsigned char* data=nullptr, size_t sz=0) {
        std::map<boost::asio::ip::tcp::endpoint, NODE_TYPE >::iterator it =
             m_replys.find(ep);
        if (it == m_replys.end()) {
            // Not found
            return -1;
        }
        std::get<0>(it->second) = REPLY_STATE::REPLYED;
        std::get<2>(it->second) = status;

        if (m_type == REQUEST_TYPE::READ) {
            std::get<3>(it->second).resize(sz);
            memcpy(std::get<3>(it->second).data(), data, sz);
        }

        if (status == 0) {
            // success
            m_succcnt++;
        }
        m_rplycnt++;
        return 0;
    }

    const std::vector<struct MemberEntry >& get_nodes() const {
        return m_nodes;
    }

private:
    enum class REPLY_STATE : int{
        INITIAL = 0,
        WAITING = 1,
        REPLYED = 2,
    };

    typedef std::vector<unsigned char> RD_REPLAY;
    typedef std::tuple<REPLY_STATE, StoreClient*, int, RD_REPLAY > NODE_TYPE;

private:
    StoreMessage* m_pmsg;
    long long    m_txid;
    REQUEST_TYPE m_type;
    std::chrono::system_clock::time_point m_crttm;
    int          m_waitcnt;
    int          m_rplycnt;
    int          m_succcnt;
    REPLY_STATE  m_rplyst;
    std::vector<struct MemberEntry > m_nodes;
    std::map<boost::asio::ip::tcp::endpoint, NODE_TYPE > m_replys;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _CLIENT_TRANSACTION_H_

