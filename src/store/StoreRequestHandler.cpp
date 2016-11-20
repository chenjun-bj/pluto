/**
 *******************************************************************************
 * StoreRequestHandler.cpp                                                     *
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

#include "StoreRequestHandler.h"
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

StoreRequestHandler::StoreRequestHandler(io_service & io,
                                         ConnectionManager& mgr) 
{
}

StoreRequestHandler::~StoreRequestHandler()
{
}

int StoreRequestHandler :: handle_message(StoreMessage * pmsg)
{
    getlog()->sendlog(LogLevel::INFO, "Handle request, msg='%x'\n", pmsg);

    if (pmsg == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "StoreRequestHandler: got null message\n");
        return -1;
    }

    int ret = 0;

    MsgType msgtype=pmsg->get_msgtype();
    switch(pmsg->get_msgtype()) {
    case MsgType::CREATREQ:
        ret = handle_create_request(dynamic_cast<CreatRequestMessage*>(pmsg));
        break;
    case MsgType::CREATRESP:
        ret = handle_create_response(dynamic_cast<CreatResponseMessage*>(pmsg));
        break;
    case MsgType::READREQ:
        ret = handle_read_request(dynamic_cast<ReadRequestMessage*>(pmsg));
        break;
    case MsgType::READRESP:
        ret = handle_read_response(dynamic_cast<ReadResponseMessage*>(pmsg));
        break;
    case MsgType::UPDATEREQ:
        ret = handle_update_request(dynamic_cast<UpdateRequestMessage*>(pmsg));
        break;
    case MsgType::UPDATERESP:
        ret = handle_update_response(dynamic_cast<UpdateResponseMessage*>(pmsg));
        break;
    case MsgType::DELETEREQ:
        ret = handle_delete_request(dynamic_cast<DeleteRequestMessage*>(pmsg));
        break;
    case MsgType::DELETERESP:
        ret = handle_delete_response(dynamic_cast<DeleteResponseMessage*>(pmsg));
        break;
    default:
        // Invalid message received
        getlog()->sendlog(LogLevel::ERROR, "Message type not support '%s'\n", get_desc_msgtype(msgtype).c_str());
        return -1;
    }

    return ret;
}

