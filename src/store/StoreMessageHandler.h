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

#include <string>
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

    std::vector< struct MemberEntry > find_nodes(const std::string& key);
protected:
    bool is_self(const struct MemberEntry& e);
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
private:
    StoreMessageHandler * m_phdler_client;
    StoreMessageHandler * m_phdler_server;

    ConfigPortal        * m_pconfig;

    // self address                                                                      
    boost::asio::ip::address m_self_addr;                                                
    int            m_self_af;                                                            
    unsigned char  m_self_rawip[PL_IPv6_ADDR_LEN];                                       
    unsigned short m_self_port; 
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_REQUEST_HANDLER_H_

