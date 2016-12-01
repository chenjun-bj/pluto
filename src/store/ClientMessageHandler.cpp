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
                                           ConfigPortal * pcfg,
                                           StoreMessageHandler * psuper) :
   StoreMessageHandler(io, mgr, store, pcfg),
   m_super_hdlr(psuper),
   m_strand(io)
{
}

ClientMessageHandler::~ClientMessageHandler()
{
}

int ClientMessageHandler::handle_create_request(CreatRequestMessage* pmsg)
{
    // Find who is in charge of the key, and then send server request message to
    // these nodes.
    if (pmsg == nullptr) return -1;
    async_find_nodes(pmsg->get_key(), [this, pmsg](const std::vector<MemberEntry > &v) {
            ClientTransaction clt(pmsg, ClientTransaction::REQUEST_TYPE::CREAT, v);
            add_pending_tran(pmsg, clt);
            for (auto&& n : v) {
                if (is_self(n)) {
                }
                else {
                    // Construct server request messages
                    // Send to node n
                }
            }
        }); 
    return 0;
}

int ClientMessageHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    return 0;
}

int ClientMessageHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

int ClientMessageHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    if (pmsg == nullptr) return -1;
    return 0;
}

