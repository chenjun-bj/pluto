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

    virtual void handle_time_event();
protected:
    bool is_self(const struct MemberEntry& e);

    void set_resp_info_from_req(StoreMessage* presp, const StoreMessage * preq);

    void send_message(const boost::asio::ip::tcp::endpoint & endpoint,
                      StoreMessage * pmsg);
    // Send messages to endpoint indicated by message
    void send_message(StoreMessage * pmsg);

protected:
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

    boost::asio::io_service & m_io;

    ConfigPortal        * m_pconfig;

    // self address                                                                      
    boost::asio::ip::address m_self_addr;                                                
    int            m_self_af;                                                            
    unsigned char  m_self_rawip[PL_IPv6_ADDR_LEN];                                       
    unsigned short m_self_port; 

private:
    StoreMessageHandler * m_phdler_client;
    StoreMessageHandler * m_phdler_server;

    //std::map<unsigned long long, ClientTransaction* > m_pending_tran;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_REQUEST_HANDLER_H_

