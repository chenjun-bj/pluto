/**
 *******************************************************************************
 * ClientRequestHandler.h                                                      *
 *                                                                             *
 * Client request handler:                                                     *
 *   - Handle CRUD request from client                                         *
 *******************************************************************************
 */

#ifndef _CLIENT_REQUEST_HANDLER_H_
#define _CLIENT_REQUEST_HANDLER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"
#include "StoreRequestHandler.h"

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

class ClientRequestHandler : public StoreRequestHandler {
public:
    ClientRequestHandler(boost::asio::io_service& io,
                         ConnectionManager& mgr);
    ~ClientRequestHandler();

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

#endif // _CLIENT_REQUEST_HANDLER_H_

