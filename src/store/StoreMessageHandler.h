/**
 *******************************************************************************
 * StoreMessageHandler.h                                                       *
 *                                                                             *
 * Store message handler:                                                      *
 *   - CRUD handle commons                                                     *
 *******************************************************************************
 */

#ifndef _STORE_MESSAGE_HANDLER_H_
#define _STORE_MESSAGE_HANDLER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"
#include "StoreManager.h"
#include "ClientTransaction.h"

#include <string>
#include <map>
#include <vector>
#include <boost/asio.hpp>

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */

class ConnectionManager;

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

class StoreMessageHandler {
public:
    StoreMessageHandler(boost::asio::io_service& io, 
                        ConnectionManager& conn_mgr,
                        StoreManager & store,
                        ConfigPortal * pcfg,
                        bool creat_child = false);
    virtual ~StoreMessageHandler();

    virtual int handle_message(StoreMessage* pmsg);

    template<typename H>
    void async_find_nodes(const std::string& key, H handler) {
        m_store.async_get_nodes<H>(key, handler);
    }

    void handle_time_event();
    //std::vector< struct MemberEntry > find_nodes(const std::string& key);
protected:
    bool is_self(const struct MemberEntry& e);

    void set_resp_info_from_req(StoreMessage* presp, const StoreMessage * preq);

    void send_message(const boost::asio::ip::tcp::endpoint & endpoint,
                      StoreMessage * pmsg);
    // Send messages to endpoint indicated by message
    void send_message(StoreMessage * pmsg);

    void add_pending_tran(StoreMessage * pmsg, const ClientTransaction& clt_trn);
    void add_pending_tran_reply(StoreMessage * pmsg);

    void handle_client_tran_complete(const ClientTransaction& clt_trn);
protected:
    virtual StoreMessage* construct_creat_response(long long txid, MsgStatus status) {
        return nullptr;
    }
    virtual StoreMessage* construct_read_response(long long txid, MsgStatus status,
                                                  const unsigned char* buf, size_t sz) {
        return nullptr;
    }
    virtual StoreMessage* construct_update_response(long long txid, MsgStatus status) {
        return nullptr;
    }
    virtual StoreMessage* construct_delete_response(long long txid, MsgStatus status) {
        return nullptr;
    }

    virtual int handle_create_request(CreatRequestMessage* pmsg);
    virtual int handle_create_response(CreatResponseMessage* pmsg);

    virtual int handle_read_request(ReadRequestMessage* pmsg);
    virtual int handle_read_response(ReadResponseMessage* pmsg);

    virtual int handle_update_request(UpdateRequestMessage* pmsg);
    virtual int handle_update_response(UpdateResponseMessage* pmsg);

    virtual int handle_delete_request(DeleteRequestMessage* pmsg);
    virtual int handle_delete_response(DeleteResponseMessage* pmsg);
protected:
    ConnectionManager&    m_conn_mgr;
    StoreManager &        m_store;
private:
    StoreMessageHandler * m_phdler_client;
    StoreMessageHandler * m_phdler_server;

    ConfigPortal        * m_pconfig;

    boost::asio::io_service::strand m_strand;

    // self address                                                                      
    boost::asio::ip::address m_self_addr;                                                
    int            m_self_af;                                                            
    unsigned char  m_self_rawip[PL_IPv6_ADDR_LEN];                                       
    unsigned short m_self_port; 

    std::map<unsigned long long, ClientTransaction > m_pending_tran;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_REQUEST_HANDLER_H_

