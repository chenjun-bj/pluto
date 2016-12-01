/**
 *******************************************************************************
 * StoreMessageHandler.cpp                                                     *
 *                                                                             *
 * Store request handler:                                                      *
 *   - CRUD handle commons                                                     *
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

#include "StoreMessageHandler.h"
#include "ClientMessageHandler.h"
#include "ServerMessageHandler.h"
#include "ConnectionManager.h"

#include <cstring>

#include <string>
#include <vector>
#include <chrono>
#include <boost/asio.hpp>

using namespace std;
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

StoreMessageHandler::StoreMessageHandler(io_service & io,
                                         ConnectionManager& conn_mgr,
                                         StoreManager & store,
                                         ConfigPortal * pcfg,
                                         bool creat_child) :
   m_conn_mgr(conn_mgr),
   m_store(store),
   m_phdler_client(nullptr),
   m_phdler_server(nullptr),
   m_pconfig(pcfg),
   m_strand(io)
{
    if (creat_child) {
        m_phdler_client = new ClientMessageHandler(io, conn_mgr, store, pcfg, this);
        m_phdler_server = new ServerMessageHandler(io, conn_mgr, store, pcfg, this);
    }

    string self_ip(m_pconfig->get_bindip());                                   
 
    m_self_port = m_pconfig->get_bindport();                                             
    m_self_addr = ip::address::from_string(self_ip);                                     

    memset(m_self_rawip, '\0', PL_IPv6_ADDR_LEN); 
    if (m_self_addr.is_v6()) {                                                           
        ip::address_v6::bytes_type rawaddr = m_self_addr.to_v6().to_bytes();             
        m_self_af = AF_INET6;
        memcpy(m_self_rawip, rawaddr.data(), PL_IPv6_ADDR_LEN);                          
    }                                                                                    
    else {                                                                               
        ip::address_v4::bytes_type rawaddr = m_self_addr.to_v4().to_bytes();             
        m_self_af = AF_INET;              
        memcpy(m_self_rawip, rawaddr.data(), PL_IPv6_ADDR_LEN);                          
    }
 
}

StoreMessageHandler::~StoreMessageHandler()
{
    if (m_phdler_client != nullptr) {
        delete m_phdler_client;
    }
    if (m_phdler_server != nullptr) {
        delete m_phdler_server;
    }
}

int StoreMessageHandler :: handle_message(StoreMessage * pmsg)
{
    getlog()->sendlog(LogLevel::INFO, "Handle request, msg='%x'\n", pmsg);

    if (pmsg == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "StoreMessageHandler: got null message\n");
        return -1;
    }

    StoreMessageHandler * phdler = nullptr;
    if (pmsg->get_originator() == MessageOriginator::Client) {
        phdler = m_phdler_client;
    }
    else if (pmsg->get_originator() == MessageOriginator::Server) {
        phdler = m_phdler_client;
    }
    else {
        getlog()->sendlog(LogLevel::ERROR, "StoreMessageHandler: invalid message originator");
        return -1;
    }

    if (phdler == nullptr) {
        getlog()->sendlog(LogLevel::FATAL, "StoreMessageHandler: no handler\n");
        return -1;
    }

    int ret = 0;

    MsgType msgtype=pmsg->get_msgtype();
    switch(pmsg->get_msgtype()) {
    case MsgType::CREATREQ:
        ret = phdler->handle_create_request(dynamic_cast<CreatRequestMessage*>(pmsg));
        break;
    case MsgType::CREATRESP:
        ret = phdler->handle_create_response(dynamic_cast<CreatResponseMessage*>(pmsg));
        break;
    case MsgType::READREQ:
        ret = phdler->handle_read_request(dynamic_cast<ReadRequestMessage*>(pmsg));
        break;
    case MsgType::READRESP:
        ret = phdler->handle_read_response(dynamic_cast<ReadResponseMessage*>(pmsg));
        break;
    case MsgType::UPDATEREQ:
        ret = phdler->handle_update_request(dynamic_cast<UpdateRequestMessage*>(pmsg));
        break;
    case MsgType::UPDATERESP:
        ret = phdler->handle_update_response(dynamic_cast<UpdateResponseMessage*>(pmsg));
        break;
    case MsgType::DELETEREQ:
        ret = phdler->handle_delete_request(dynamic_cast<DeleteRequestMessage*>(pmsg));
        break;
    case MsgType::DELETERESP:
        ret = phdler->handle_delete_response(dynamic_cast<DeleteResponseMessage*>(pmsg));
        break;
    default:
        // Invalid message received
        getlog()->sendlog(LogLevel::ERROR, "Message type not support '%s'\n", get_desc_msgtype(msgtype).c_str());
        return -1;
    }

    return ret;
}

/*
std::vector<struct MemberEntry > StoreMessageHandler::find_nodes(const std::string & key)
{
    return m_store.get_nodes(key);
}
*/

void StoreMessageHandler::handle_time_event()
{
    m_strand.post([this]() {
            using namespace std::chrono;

            system_clock::time_point now = system_clock::now();
            duration<int, std::milli> expires(m_pconfig->get_message_timeout());
            for (auto it = m_pending_tran.begin();it != m_pending_tran.end();) {
                if (it->second.get_creat_time() + expires < now) {
                    // expires, erase
                    // TODO: send failed response to client
                    it = m_pending_tran.erase(it);
                }
                else {
                    it++;
                }
            }
        });
}

bool StoreMessageHandler::is_self(const struct MemberEntry& e)
{
    if (e.af != m_self_af) return false;                                                   
    if (e.portnumber != m_self_port) return false;                                               
    int cmplen = PL_IPv6_ADDR_LEN;                                                       
    if (e.af == AF_INET) { 
        cmplen = PL_IPv4_ADDR_LEN;                                                       
    } 
    if (memcmp(m_self_rawip, e.address, cmplen) != 0) { 
        return false;
    }

    return true;
    
}

void StoreMessageHandler::set_resp_info_from_req(StoreMessage* presp, const StoreMessage * preq)
{
    if ((presp==nullptr) || (preq==nullptr)) {
        return;
    }
    presp->set_replica_type(preq->get_replica_type());
    presp->set_connection(preq->get_connection());
    std::pair<boost::asio::ip::address, unsigned short> srcaddr = preq->get_source();
    presp->set_destination(srcaddr.first, srcaddr.second);
}

void StoreMessageHandler::send_message(const boost::asio::ip::tcp::endpoint& endpoint,
                                       StoreMessage* pmsg)
{
    if (pmsg==nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Error, send null message\n");
        return ;
    }
    m_conn_mgr.send_message(endpoint, pmsg);
}

void StoreMessageHandler::send_message(StoreMessage* pmsg)
{
    if (pmsg==nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Error, send null message\n");
        return ;
    }
    if (pmsg->get_connection().get() != nullptr) {
        pmsg->get_connection().get()->do_write(pmsg);
    }
    else {
        send_message(pmsg->get_dest_endpoint(), pmsg);
    }
}

void StoreMessageHandler::add_pending_tran(StoreMessage * pmsg, const ClientTransaction& clt_trn)
{
    if (pmsg == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "add pending transaction, nullptr\n");
        return ;
    }
    m_strand.post([this, pmsg, clt_trn]() {
            if (m_pending_tran.find(reinterpret_cast<unsigned long long>(pmsg)) != m_pending_tran.end()) {
                return;
            }
            m_pending_tran.insert(make_pair(reinterpret_cast<unsigned long long>(pmsg), clt_trn));
        });
}

void StoreMessageHandler::add_pending_tran_reply(StoreMessage * pmsg)
{
    if (pmsg == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "add pending transaction reply, nullptr\n");
        return ;
    }
    m_strand.post([this, pmsg]() {
            map<unsigned long long, ClientTransaction >::iterator it;
            it = m_pending_tran.find(reinterpret_cast<unsigned long long>(pmsg));
            if (it != m_pending_tran.end()) {
                it->second.add_reply(pmsg);
                if (it->second.get_reply_count() >= m_pconfig->get_quorum_num()) {
                    handle_client_tran_complete(it->second);
                    // TODO: remove after receive all replies???
                    m_pending_tran.erase(it);
                }
            }
        });
}

void StoreMessageHandler::handle_client_tran_complete(const ClientTransaction& clt_trn)
{
    // Construct response message
    StoreMessage * pclntresp = nullptr;
    switch(clt_trn.get_type()) {
    case ClientTransaction::REQUEST_TYPE::CREAT:
        pclntresp = construct_creat_response(clt_trn.get_txid(), MsgStatus::OK);
        break;
    case ClientTransaction::REQUEST_TYPE::READ:
        pclntresp = construct_read_response(clt_trn.get_txid(), MsgStatus::OK,
                                            nullptr, 0);
        break;
    case ClientTransaction::REQUEST_TYPE::UPDATE:
        pclntresp = construct_update_response(clt_trn.get_txid(), MsgStatus::OK);
        break;
    case ClientTransaction::REQUEST_TYPE::DELETE:
        pclntresp = construct_delete_response(clt_trn.get_txid(), MsgStatus::OK);
        break;
    default:
        break;
    }

    if (pclntresp != nullptr) {
        pclntresp->set_connection(clt_trn.get_msg()->get_connection());
        send_message(pclntresp);
    }
}

int StoreMessageHandler::handle_create_request(CreatRequestMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

int StoreMessageHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    getlog()->sendlog(LogLevel::FATAL, "Fatal error, store message handler got called\n");
    return PLERROR;
}

/* eof */
