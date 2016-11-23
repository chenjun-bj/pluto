/**
 *******************************************************************************
 * ServerMessageHandler.cpp                                                    *
 *                                                                             *
 * Server request handler                                                      *
 *   - Perform CRUD request from server                                        *
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

#include "ServerMessageHandler.h"
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

ServerMessageHandler::ServerMessageHandler(io_service & io,
                                           ConnectionManager& conn_mgr,
                                           StoreManager& store,
                                           ConfigPortal * pcfg) :
   StoreMessageHandler(io, conn_mgr, store, pcfg)
{
}

ServerMessageHandler::~ServerMessageHandler()
{
}


int ServerMessageHandler::handle_create_request(CreatRequestMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    return 0;
}

