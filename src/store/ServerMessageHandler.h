/**
 *******************************************************************************
 * ServerMessageHandler.h                                                      *
 *                                                                             *
 * Server message handler:                                                     *
 *   - Handle CRUD request from server                                         *
 *******************************************************************************
 */

#ifndef _SERVER_REQUEST_HANDLER_H_
#define _SERVER_REQUEST_HANDLER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"
#include "StoreMessageHandler.h"

#include <boost/asio.hpp>

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

class ServerMessageHandler : public StoreMessageHandler {
public:
    ServerMessageHandler(boost::asio::io_service& io,
                         ConnectionManager& mgr,
                         StoreManager & store,
                         ConfigPortal * pcfg);
    ~ServerMessageHandler();

protected:
    virtual int handle_create_request(CreatRequestMessage* pmsg);
    virtual int handle_create_response(CreatResponseMessage* pmsg);

    virtual int handle_read_request(ReadRequestMessage* pmsg);
    virtual int handle_read_response(ReadResponseMessage* pmsg);

    virtual int handle_update_request(UpdateRequestMessage* pmsg);
    virtual int handle_update_response(UpdateResponseMessage* pmsg);

    virtual int handle_delete_request(DeleteRequestMessage* pmsg);
    virtual int handle_delete_response(DeleteResponseMessage* pmsg);

private:
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _SERVER_REQUEST_HANDLER_H_

