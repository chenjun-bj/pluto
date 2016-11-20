/**
 *******************************************************************************
 * StoreHandler.cpp                                                            *
 *                                                                             *
 * Store handler                                                               *
 *   - Perform CRUD operation                                                  *
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

#include "StoreHandler.h"
#include "ConnectionManager.h"

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

StoreHandler::StoreHandler(io_service & io,
                           ConnectionManager& mgr,
                           KVStoreAsyncAccessor& acc) :
   m_mgr(mgr),
   m_acc(acc),
   m_client_handler(io, mgr),
   m_server_handler(io, mgr)
{
}

StoreHandler::~StoreHandler()
{
}

int StoreHandler :: handle_message(StoreMessage * pmsg)
{
    getlog()->sendlog(LogLevel::INFO, "Handle message '%x'\n", pmsg);

    if (pmsg == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "StoreHandler: handle message got null message\n");
        return -1;
    }

    int ret = 0;
    if (pmsg->get_originator() == MessageOriginator::Client) {
        ret = handle_client_message(pmsg);
    } 
    else if (pmsg->get_originator() == MessageOriginator::Server) {
        ret = handle_server_message(pmsg);
    }
    else {
        getlog()->sendlog(LogLevel::ERROR, "StoreHandler: invalid message originator");
        return -1;
    }

    return ret;
}

int StoreHandler :: handle_client_message(StoreMessage * pmsg)
{
    return m_client_handler.handle_message(pmsg);
}

int StoreHandler :: handle_server_message(StoreMessage * pmsg)
{
    return m_server_handler.handle_message(pmsg);
}
