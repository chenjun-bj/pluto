/**
 *******************************************************************************
 * StoreHandler.h                                                              *
 *                                                                             *
 * Store handler:                                                              *
 *   - Perform CRUD operations                                                 *
 *******************************************************************************
 */

#ifndef _STORE_HANDLER_H_
#define _STORE_HANDLER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"
#include "ClientRequestHandler.h"
#include "ServerRequestHandler.h"

#include "KVStoreAccess.h"

#include <boost/asio.hpp>

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */
// Do not include ConnectionManager.h !
class ConnectionManager;

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

class StoreHandler {
public:
    StoreHandler(boost::asio::io_service & io,
                 ConnectionManager& mgr,
                 KVStoreAsyncAccessor& acc);
    ~StoreHandler();

    int handle_message(StoreMessage* pmsg);
private:
    int handle_client_message(StoreMessage* pmsg);
    int handle_server_message(StoreMessage* pmsg);
private:
    ConnectionManager&     m_mgr;
    KVStoreAsyncAccessor&  m_acc;
    ClientRequestHandler   m_client_handler;
    ServerRequestHandler   m_server_handler;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_HANDLER_H_

