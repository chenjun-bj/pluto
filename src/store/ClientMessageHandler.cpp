/**
 *******************************************************************************
 * ClientMessageHandler.cpp                                                    *
 *                                                                             *
 * Client message handler                                                      *
 *   - Perform CRUD request from client                                        *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "CreatMessage.h"
#include "ReadMessage.h"
#include "UpdateMessage.h"
#include "DeleteMessage.h"

#include "StoreClient.h"
#include "ClientMessageHandler.h"
#include "ConnectionManager.h"

#include <chrono>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;

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

ClientMessageHandler::ClientMessageHandler(io_service & io,
                                           ConnectionManager& mgr,
                                           StoreManager& store,
                                           ConfigPortal * pcfg):
   StoreMessageHandler(io, mgr, store, pcfg),
   m_strand(io),
   m_pending_tran()
{
}

ClientMessageHandler::~ClientMessageHandler()
{
    for (auto && t: m_pending_tran) {
        if (t.second!=nullptr) {
            delete t.second;
        }
    }
    m_pending_tran.clear();
}

void ClientMessageHandler::handle_time_event()
{
    m_strand.post([this]() {
        using namespace std::chrono;

        system_clock::time_point now = system_clock::now();
        duration<int, std::milli> expires(m_pconfig->get_message_timeout());
        for (auto it = m_pending_tran.begin(); it != m_pending_tran.end(); ) {
            if (it->second->get_creat_time() + expires < now) {
                // Expires 
                if (!it->second->is_client_response()) {
                    StoreMessage * resp = construct_client_resp_msg(it->second, MsgStatus::ERROR);
                    if (resp != nullptr) {
                        send_message(resp);
                    }
                }
                delete it->second;
                it = m_pending_tran.erase(it);
            }
            else {
                it++;
            }
        }
    });
}

int ClientMessageHandler::handle_create_request(CreatRequestMessage* pmsg)
{
    // Find who is in charge of the key, and then send server request message to
    // these nodes.
    if (pmsg == nullptr) return -1;
    async_find_nodes(pmsg->get_key(), [this, pmsg](const std::vector<MemberEntry > &v) {
            std::vector<unsigned char> val;
            std::map<ip::tcp::endpoint, StoreClient* > node_clt;
            unsigned long long txid = reinterpret_cast<unsigned long long>(pmsg);

            pmsg->get_value(val);

            ClientTransaction * pclt_tran = new ClientTransaction(pmsg, 
                                            ClientTransaction::REQUEST_TYPE::CREAT, 
                                            v);
            // TODO: handle errors
            prepare_node_tran(v, pclt_tran, node_clt);
            add_pending_tran(pmsg, pclt_tran);

            int replica_type = -1;
            for (auto&& n : v) {
                boost::asio::ip::tcp::endpoint ep = get_node_endpoint(n);
                replica_type++;
                if (node_clt.find(ep) == node_clt.end()) {
                    getlog()->sendlog(LogLevel::ERROR, "Can't find client for node '%s:%d'\n",
                                     ep.address().to_string().c_str(), ep.port());
                    continue;
                }
                StoreClient * pclt = node_clt[ep];
                if (pclt == nullptr) {
                    m_store.async_creat(pmsg->get_key(), replica_type, 
                                        val.data(), val.size(), 
                                        [this, txid](int rc) {
                                            handle_store_cud_complete(rc, txid);
                                        });
                }
                else {
                    // Construct server request messages
                    // Send to node n
                    CreatRequestMessage * preq = new CreatRequestMessage(MessageOriginator::Server,
                                                                         reinterpret_cast<int64>(pmsg));
                    preq->set_key(pmsg->get_key());
                    preq->set_value(val.data(), val.size());
                    preq->set_replica_type(replica_type);
                    preq->set_dest_endpoint(ep);
                    pclt->async_call(ep, preq, 
                                     [this, txid](const ClientErrorCode& e,
                                                  StoreMessage* presp) {
                                         handle_clt_crud_complete(static_cast<int>(e), presp, txid);
                                     });
                }
            }
        }); 
    return 0;
}

int ClientMessageHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    // TODO: not support
    return 0;
}

int ClientMessageHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    async_find_nodes(pmsg->get_key(), [this, pmsg](const std::vector<MemberEntry > &v) {
            std::map<ip::tcp::endpoint, StoreClient* > node_clt;
            unsigned long long txid = reinterpret_cast<unsigned long long>(pmsg);

            ClientTransaction * pclt_tran = new ClientTransaction(pmsg, 
                                            ClientTransaction::REQUEST_TYPE::READ, 
                                            v);
            // TODO: handle errors
            prepare_node_tran(v, pclt_tran, node_clt);
            add_pending_tran(pmsg, pclt_tran);

            int replica_type = -1;
            for (auto&& n : v) {
                boost::asio::ip::tcp::endpoint ep = get_node_endpoint(n);
                replica_type++;
                if (node_clt.find(ep) == node_clt.end()) {
                    getlog()->sendlog(LogLevel::ERROR, "Can't find client for node '%s:%d'\n",
                                     ep.address().to_string().c_str(), ep.port());
                    continue;
                }
                StoreClient * pclt = node_clt[ep];
                if (pclt == nullptr) {
                    m_store.async_read(pmsg->get_key(), replica_type, 
                              [this, txid](int rc, unsigned char* val, size_t sz) {
                                  handle_store_r_complete(rc, txid, val, sz);
                              });
                }
                else {
                    // Construct server request messages
                    // Send to node n
                    ReadRequestMessage * preq = new ReadRequestMessage(MessageOriginator::Server,
                                                                       reinterpret_cast<int64>(pmsg));
                    preq->set_key(pmsg->get_key());
                    preq->set_replica_type(replica_type);
                    preq->set_dest_endpoint(ep);
                    pclt->async_call(ep, preq, 
                                     [this, txid](const ClientErrorCode& e,
                                                  StoreMessage* presp) {
                                         handle_clt_crud_complete(static_cast<int>(e), presp, txid);
                                     });
                }
            }
        }); 
    return 0;
}

int ClientMessageHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    async_find_nodes(pmsg->get_key(), [this, pmsg](const std::vector<MemberEntry > &v) {
            std::vector<unsigned char> val;
            std::map<ip::tcp::endpoint, StoreClient* > node_clt;
            unsigned long long txid = reinterpret_cast<unsigned long long>(pmsg);

            pmsg->get_value(val);

            ClientTransaction * pclt_tran = new ClientTransaction(pmsg, 
                                            ClientTransaction::REQUEST_TYPE::UPDATE, 
                                            v);
            // TODO: handle errors
            prepare_node_tran(v, pclt_tran, node_clt);
            add_pending_tran(pmsg, pclt_tran);

            int replica_type = -1;
            for (auto&& n : v) {
                boost::asio::ip::tcp::endpoint ep = get_node_endpoint(n);
                replica_type++;
                if (node_clt.find(ep) == node_clt.end()) {
                    getlog()->sendlog(LogLevel::ERROR, "Can't find client for node '%s:%d'\n",
                                     ep.address().to_string().c_str(), ep.port());
                    continue;
                }
                StoreClient * pclt = node_clt[ep];
                if (pclt == nullptr) {
                    m_store.async_update(pmsg->get_key(), replica_type, 
                                        val.data(), val.size(), 
                                        [this, txid](int rc) {
                                            handle_store_cud_complete(rc, txid);
                                        });
                }
                else {
                    // Construct server request messages
                    // Send to node n
                    UpdateRequestMessage * preq = new UpdateRequestMessage(MessageOriginator::Server,
                                                                         reinterpret_cast<int64>(pmsg));
                    preq->set_key(pmsg->get_key());
                    preq->set_value(val.data(), val.size());
                    preq->set_replica_type(replica_type);
                    preq->set_dest_endpoint(ep);
                    pclt->async_call(ep, preq, 
                                     [this, txid](const ClientErrorCode& e,
                                                  StoreMessage* presp) {
                                         handle_clt_crud_complete(static_cast<int>(e), presp, txid);
                                     });
                }
            }
        }); 
    return 0;
}

int ClientMessageHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    async_find_nodes(pmsg->get_key(), [this, pmsg](const std::vector<MemberEntry > &v) {
            std::map<ip::tcp::endpoint, StoreClient* > node_clt;
            unsigned long long txid = reinterpret_cast<unsigned long long>(pmsg);

            ClientTransaction * pclt_tran = new ClientTransaction(pmsg, 
                                            ClientTransaction::REQUEST_TYPE::DELETE, 
                                            v);
            // TODO: handle errors
            prepare_node_tran(v, pclt_tran, node_clt);
            add_pending_tran(pmsg, pclt_tran);

            int replica_type = -1;
            for (auto&& n : v) {
                boost::asio::ip::tcp::endpoint ep = get_node_endpoint(n);
                replica_type++;
                if (node_clt.find(ep) == node_clt.end()) {
                    getlog()->sendlog(LogLevel::ERROR, "Can't find client for node '%s:%d'\n",
                                     ep.address().to_string().c_str(), ep.port());
                    continue;
                }
                StoreClient * pclt = node_clt[ep];
                if (pclt == nullptr) {
                    m_store.async_delete(pmsg->get_key(), replica_type, 
                                        [this, txid](int rc) {
                                            handle_store_cud_complete(rc, txid);
                                        });
                }
                else {
                    // Construct server request messages
                    // Send to node n
                    DeleteRequestMessage * preq = new DeleteRequestMessage(MessageOriginator::Server,
                                                                         reinterpret_cast<int64>(pmsg));
                    preq->set_key(pmsg->get_key());
                    preq->set_replica_type(replica_type);
                    preq->set_dest_endpoint(ep);
                    pclt->async_call(ep, preq, 
                                     [this, txid](const ClientErrorCode& e,
                                                  StoreMessage* presp) {
                                         handle_clt_crud_complete(static_cast<int>(e), presp, txid);
                                     });
                }
            }
        }); 
    return 0;
}

int ClientMessageHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

ip::tcp::endpoint ClientMessageHandler::get_node_endpoint(const struct MemberEntry& e)
{
     return ip::tcp::endpoint(rawip2address(e.af, e.address), e.portnumber);
}

ip::tcp::endpoint ClientMessageHandler::get_self_endpoint() const
{
     return ip::tcp::endpoint(m_self_addr, m_self_port);
}

int ClientMessageHandler::prepare_node_tran(
                          const std::vector<struct MemberEntry > & nodes,
                          ClientTransaction * pclt_tran,
                          std::map<ip::tcp::endpoint, StoreClient* > & node_clt)
{
    if (pclt_tran==nullptr) return -1;
    node_clt.clear();
    for (auto&& n : nodes) {
        boost::asio::ip::tcp::endpoint ep = get_node_endpoint(n);
        StoreClient * pclt = nullptr;
        if (!is_self(n)) {
            pclt = new StoreClient(m_pconfig, m_io);
        }
        pclt_tran->start_wait_reply(ep, pclt);
        node_clt[ep] = pclt;
    }
    return 0;
}

void ClientMessageHandler::handle_store_cud_complete(int rc, unsigned long long txid)
{
    m_strand.post([this, rc, txid]() {
        auto it = m_pending_tran.find(txid);
        if (it != m_pending_tran.end()) {
            it->second->add_reply(get_self_endpoint(), 
                                  rc);
            handle_node_reply(it);
        }
    });
}

void ClientMessageHandler::handle_store_r_complete(int rc, unsigned long long txid,
                                                   unsigned char* data, size_t sz)
{
    m_strand.post([=]() {
        auto it = m_pending_tran.find(txid);
        if (it != m_pending_tran.end()) {
            it->second->add_reply(get_self_endpoint(), 
                                  rc, data, sz);
            handle_node_reply(it);
        }
    });
}

void ClientMessageHandler::handle_clt_crud_complete(int rc, 
                                                    StoreMessage * pmsg,
                                                    unsigned long long txid)
{
    m_strand.post([this, rc, pmsg, txid]() {
        auto it = m_pending_tran.find(txid);
        if (it != m_pending_tran.end()) {
            it->second->add_reply(pmsg);
            handle_node_reply(it);
        }
    });
}

void ClientMessageHandler::handle_node_reply(std::map<unsigned long long, ClientTransaction* >::iterator& it)
{
    CheckOperation op = check_clnt_tran(it->second);
    if (op==CheckOperation::SEND_RESP) {
        // construct response message and send back
        StoreMessage * resp = construct_client_resp_msg(it->second, MsgStatus::OK);
        if (resp != nullptr) {
            send_message(resp);
            it->second->mark_send_clnt_resp();
        }
    }
    if (op==CheckOperation::DELETE) {
        if (!it->second->is_client_response()) {
            // failed
            StoreMessage * resp = construct_client_resp_msg(it->second, MsgStatus::ERROR);
            if (resp != nullptr) {
                send_message(resp);
            }
        }
        delete it->second;
        m_pending_tran.erase(it);
    }
}

ClientMessageHandler::CheckOperation ClientMessageHandler::check_clnt_tran(ClientTransaction* pclt_trn)
{
    int total_reply;
    int succ_reply;
    std::tie(total_reply, succ_reply) = pclt_trn->get_reply_count();
    if (succ_reply >= m_pconfig->get_quorum_num()) {
        if (!pclt_trn->is_client_response()) {
            return CheckOperation::SEND_RESP;
        }
    }
    if (total_reply >= pclt_trn->get_wait_count()) {
        return CheckOperation::DELETE;
    }
    return CheckOperation::NOP;
}

void ClientMessageHandler::add_pending_tran(StoreMessage* pmsg, 
                                            ClientTransaction * pclt_trn)
{
    if ((pmsg == nullptr) || (pclt_trn==nullptr)) {
        getlog()->sendlog(LogLevel::FATAL, "add pending transaction, nullptr\n");
        return ;
    }
    m_strand.post([this, pmsg, pclt_trn]() {
            if (m_pending_tran.find(reinterpret_cast<unsigned long long>(pmsg)) != m_pending_tran.end()) {
                return;
            }
            m_pending_tran.insert(std::make_pair(reinterpret_cast<unsigned long long>(pmsg), pclt_trn));
        });

}

StoreMessage* ClientMessageHandler::construct_client_resp_msg(ClientTransaction* pclt_trn,
                                                              MsgStatus status)
{
    if (pclt_trn == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Consturct client response message got nullptr\n");
        return nullptr;
    }

    StoreMessage * presp = nullptr;
    switch(pclt_trn->get_type()) {
    case ClientTransaction::REQUEST_TYPE::CREAT:
        presp = new CreatResponseMessage(MessageOriginator::Client,
                                         pclt_trn->get_txid(), status);
        break;
    case ClientTransaction::REQUEST_TYPE::READ:
    {
        presp = new ReadResponseMessage(MessageOriginator::Client,
                                        pclt_trn->get_txid(), status);
        std::vector<unsigned char> v;
        pclt_trn->get_read_value(v);
        dynamic_cast<ReadResponseMessage*>(presp)->set_value(v);
        break;
    }
    case ClientTransaction::REQUEST_TYPE::UPDATE:
        //presp = construct_update_response(pclt_trn->get_txid(), status);
        break;
    case ClientTransaction::REQUEST_TYPE::DELETE:
        //presp = construct_delete_response(pclt_trn->get_txid(), status);
        break;
    default:
        break;
    }

    if (presp != nullptr) {
        presp->set_connection(pclt_trn->get_msg()->get_connection());
    }

    return presp;
}

/* eof */

