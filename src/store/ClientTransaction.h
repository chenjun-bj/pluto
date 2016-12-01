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
       m_rplycnt(0) {
    }
    ~ClientTransaction() {
    }

    ClientTransaction(const ClientTransaction& other) {
        m_pmsg = other.m_pmsg;
        m_txid = other.m_txid;
        m_type = other.m_type;
        m_crttm = other.m_crttm;
        m_nodes = other.m_nodes;
    }

    ClientTransaction& operator=(const ClientTransaction& other) {
        if (this != &other) {
            m_pmsg = other.m_pmsg;
            m_txid = other.m_txid;
            m_type = other.m_type;
            m_crttm = other.m_crttm;
            m_nodes = other.m_nodes;
        }
        return *this;
    }

    long long get_txid() const {
        return m_txid;
    }

    std::chrono::system_clock::time_point get_creat_time() const {
        return m_crttm;
    }

    int get_reply_count() const {
        return m_rplycnt;
    }

    REQUEST_TYPE get_type() const {
        return m_type;
    }

    StoreMessage* get_msg() const {
        return m_pmsg;
    }

    int add_reply(StoreMessage* pmsg) {
        if (pmsg==nullptr) return -1;
        switch(m_type) {
        case REQUEST_TYPE::CREAT: {
            CreatResponseMessage * presp = 
                dynamic_cast<CreatResponseMessage*>(pmsg);
            if (presp == nullptr || presp->get_status() != MsgStatus::OK) {
                return -1;
            }
            break;
        }
        case REQUEST_TYPE::READ: {
            ReadResponseMessage * presp = 
                dynamic_cast<ReadResponseMessage*>(pmsg);
            if (presp == nullptr || presp->get_status() != MsgStatus::OK) {
                return -1;
            }
            break;
        }
        case REQUEST_TYPE::UPDATE: {
            UpdateResponseMessage * presp = 
                dynamic_cast<UpdateResponseMessage*>(pmsg);
            if (presp == nullptr || presp->get_status() != MsgStatus::OK) {
                return -1;
            }
            break;
        }
        case REQUEST_TYPE::DELETE: {
            DeleteResponseMessage * presp = 
                dynamic_cast<DeleteResponseMessage*>(pmsg);
            if (presp == nullptr || presp->get_status() != MsgStatus::OK) {
                return -1;
            }
            break;
        }
        default: return -1;
        }

        m_rplycnt++;
        return 0;
    }

    const std::vector<struct MemberEntry >& get_nodes() const {
        return m_nodes;
    }

private:
    StoreMessage* m_pmsg;
    long long    m_txid;
    REQUEST_TYPE m_type;
    std::chrono::system_clock::time_point m_crttm;
    int          m_rplycnt;
    std::vector<struct MemberEntry > m_nodes;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _CLIENT_TRANSACTION_H_

