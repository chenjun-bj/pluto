/**
 *******************************************************************************
 * StoreRequestHandler.h                                                       *
 *                                                                             *
 * Store request handler:                                                      *
 *   - CRUD handle commons                                                     *
 *******************************************************************************
 */

#ifndef _STORE_REQUEST_HANDLER_H_
#define _STORE_REQUEST_HANDLER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"

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

class StoreRequestHandler {
public:
    StoreRequestHandler(boost::asio::io_service& io, 
                        ConnectionManager& mgr);
    virtual ~StoreRequestHandler();

    virtual int handle_message(StoreMessage* pmsg);

protected:
    virtual int handle_create_request(CreatRequestMessage* pmsg) = 0;
    virtual int handle_create_response(CreatResponseMessage* pmsg) = 0;

    virtual int handle_read_request(ReadRequestMessage* pmsg) = 0;
    virtual int handle_read_response(ReadResponseMessage* pmsg) = 0;

    virtual int handle_update_request(UpdateRequestMessage* pmsg) = 0;
    virtual int handle_update_response(UpdateResponseMessage* pmsg) = 0;

    virtual int handle_delete_request(DeleteRequestMessage* pmsg) = 0;
    virtual int handle_delete_response(DeleteResponseMessage* pmsg) = 0;

private:

};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_REQUEST_HANDLER_H_

