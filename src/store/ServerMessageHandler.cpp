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

#include <vector>
#include <boost/asio.hpp>

using namespace std;
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
    if (pmsg == nullptr) return -1;

    std::string key = pmsg->get_key();
    size_t       sz = pmsg->get_value_length();
    std::vector<unsigned char> value(sz);

    pmsg->get_value(value.data(), sz);

    m_store.async_creat(key, pmsg->get_replica_type(), 
                        value.data(), sz, 
                        [this, pmsg](int rc){
                            CreatResponseMessage * presp = nullptr;
                            if (rc) {
                                presp = new CreatResponseMessage(MessageOriginator::Server,
                                                                 pmsg->get_txid(),
                                                                 MsgStatus::ERROR);
                            }
                            else {
                                presp = new CreatResponseMessage(MessageOriginator::Server,
                                                                 pmsg->get_txid(),
                                                                 MsgStatus::OK);
                            }
                            set_resp_info_from_req(presp, pmsg);
                            presp->build_msg();
                            send_message(presp);
                        });
    return 0;
}

int ServerMessageHandler::handle_create_response(CreatResponseMessage* pmsg)
{
    // TODO: need get corresponding request from response's txid,
    //       this may require global resource, lock/strand is needed
    return 0;
}

int ServerMessageHandler::handle_read_request(ReadRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;

    std::string key = pmsg->get_key();

    m_store.async_read(key, pmsg->get_replica_type(),
                       [this, pmsg](int rc, const unsigned char* data, const size_t sz) {
                           ReadResponseMessage * presp = nullptr;
                           if (rc) {
                               presp = new ReadResponseMessage(MessageOriginator::Server,
                                                               pmsg->get_txid(),
                                                               MsgStatus::ERROR);
                               //presp->set_value(
                           }
                           else {
                               presp = new ReadResponseMessage(MessageOriginator::Server,
                                                               pmsg->get_txid(),
                                                               MsgStatus::OK);
                               presp->set_value(data, sz);
                           }
                           set_resp_info_from_req(presp, pmsg);
                           presp->build_msg();
                           send_message(presp);
                       });
    return 0;
}

int ServerMessageHandler::handle_read_response(ReadResponseMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_update_request(UpdateRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;

    std::string key = pmsg->get_key();
    size_t       sz = pmsg->get_value_length();
    std::vector<unsigned char> value(sz);

    pmsg->get_value(value.data(), sz);

    m_store.async_update(key, pmsg->get_replica_type(), 
                         value.data(), sz, 
                         [this, pmsg](int rc) {
                            UpdateResponseMessage * presp = nullptr;
                            if (rc) {
                                presp = new UpdateResponseMessage(MessageOriginator::Server,
                                                                 pmsg->get_txid(),
                                                                 MsgStatus::ERROR);
                            }
                            else {
                                presp = new UpdateResponseMessage(MessageOriginator::Server,
                                                                 pmsg->get_txid(),
                                                                 MsgStatus::OK);
                            }
                            set_resp_info_from_req(presp, pmsg);
                            presp->build_msg();
                            send_message(presp);
                        });
    return 0;
}

int ServerMessageHandler::handle_update_response(UpdateResponseMessage* pmsg)
{
    return 0;
}

int ServerMessageHandler::handle_delete_request(DeleteRequestMessage* pmsg)
{
    if (pmsg == nullptr) return -1;

    std::string key = pmsg->get_key();

    m_store.async_delete(key, pmsg->get_replica_type(),
                         [this, pmsg](int rc) {
                             DeleteResponseMessage * presp = nullptr;
                             if (rc) {
                                 presp = new DeleteResponseMessage(MessageOriginator::Server,
                                                                   pmsg->get_txid(),
                                                                   MsgStatus::ERROR);
                             }
                             else {
                                 presp = new DeleteResponseMessage(MessageOriginator::Server,
                                                                   pmsg->get_txid(),
                                                                   MsgStatus::OK);
                             }
                             set_resp_info_from_req(presp, pmsg);
                             presp->build_msg();
                             send_message(presp);
                         });
    return 0;
}

int ServerMessageHandler::handle_delete_response(DeleteResponseMessage* pmsg)
{
    return 0;
}

