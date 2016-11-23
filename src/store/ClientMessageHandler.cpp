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

#include "ClientMessageHandler.h"
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

ClientMessageHandler::ClientMessageHandler(io_service & io,
                                           ConnectionManager& mgr,
                                           StoreManager& store,
                                           ConfigPortal * pcfg) :
   StoreMessageHandler(io, mgr, store, pcfg)
{
}

ClientMessageHandler::~ClientMessageHandler()
{
}

int ClientMessageHandler::handle_create_request(CreatRequestMessage* pmsg)
{
    // Find who is in charge of the key, and then send server request message to
    // these nodes.

    return 0;
}

int ClientMessageHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    return 0;
}

