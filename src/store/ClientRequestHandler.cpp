/**
 *******************************************************************************
 * ClientRequestHandler.cpp                                                    *
 *                                                                             *
 * Client request handler                                                      *
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

#include "ClientRequestHandler.h"
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

ClientRequestHandler::ClientRequestHandler(io_service & io,
                                           ConnectionManager& mgr) :
   StoreRequestHandler(io, mgr)
{
}

ClientRequestHandler::~ClientRequestHandler()
{
}

int ClientRequestHandler::handle_create_request(CreatRequestMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    return 0;
}

int ClientRequestHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    return 0;
}

