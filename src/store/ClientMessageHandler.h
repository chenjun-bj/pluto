/**
 *******************************************************************************
 * ClientMessageHandler.h                                                      *
 *                                                                             *
 * Client message handler:                                                     *
 *   - Handle CRUD request from client                                         *
 *******************************************************************************
 */

#ifndef _CLIENT_MESSAGE_HANDLER_H_
#define _CLIENT_MESSAGE_HANDLER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreMessage.h"
#include "StoreMessageHandler.h"
//#include "Connection.h"

#include <map>
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

class ClientMessageHandler : public StoreMessageHandler {
public:
    ClientMessageHandler(boost::asio::io_service& io,
                         ConnectionManager& mgr,
                         StoreManager & store,
                         ConfigPortal * pcfg);
    ~ClientMessageHandler();

    virtual void handle_time_event();
protected:
    virtual int handle_create_request(CreatRequestMessage* pmsg);
    virtual int handle_create_response(CreatResponseMessage* pmsg);

    virtual int handle_read_request(ReadRequestMessage* pmsg);
    virtual int handle_read_response(ReadResponseMessage* pmsg);

    virtual int handle_update_request(UpdateRequestMessage* pmsg);
    virtual int handle_update_response(UpdateResponseMessage* pmsg);

    virtual int handle_delete_request(DeleteRequestMessage* pmsg);
    virtual int handle_delete_response(DeleteResponseMessage* pmsg);

    void handle_store_cud_complete(int rc, unsigned long long txid);
    void handle_store_r_complete(int rc, unsigned long long txid, 
                                 unsigned char* data, size_t sz);
    void handle_clt_crud_complete(int rc, StoreMessage* pmsg, unsigned long long txid);
private:
    void add_pending_tran(StoreMessage * pmsg, ClientTransaction* clt_trn);

    int prepare_node_tran(const std::vector<struct MemberEntry >&, 
                          ClientTransaction *,
                          std::map<boost::asio::ip::tcp::endpoint, StoreClient* > &);

    void handle_node_reply(std::map<unsigned long long, ClientTransaction* >::iterator& );
    enum class CheckOperation : int {
        NOP = 0,
        SEND_RESP,
        DELETE,
        ERROR
    };
    CheckOperation check_clnt_tran(ClientTransaction* pclt_trn);

    StoreMessage* construct_client_resp_msg(ClientTransaction *, MsgStatus);

    boost::asio::ip::tcp::endpoint get_node_endpoint(const struct MemberEntry& e); 
    boost::asio::ip::tcp::endpoint get_self_endpoint() const;

private:
    boost::asio::io_service::strand  m_strand;

    std::map<unsigned long long, ClientTransaction* > m_pending_tran;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _CLIENT_REQUEST_HANDLER_H_

