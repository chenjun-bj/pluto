/**
 *******************************************************************************
 * Connection.cpp                                                              *
 *                                                                             *
 * Tcp connection:                                                             *
 *   - Represents a TCP connection                                             *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include <algorithm>
#include <iterator>
#include <thread>

#include <climits>

#include <boost/asio.hpp>
#include "Connection.h"
#include "ConnectionManager.h"

using namespace boost::asio;
using namespace std;

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

Connection::Connection(ip::tcp::socket sock,
                       io_service& io, 
                       ConnectionManager& conn_mgr,
                       StoreMessageHandler& handler,
                       StoreMessageFactory & fact) :
   m_strand(io),
   m_socket(io),
   m_conn_mgr(conn_mgr),
   m_handler(handler),
   m_fact(fact),
   m_buffer(),
   m_rcv_buf()
{

}
 
void Connection::start()
{
    do_read();
}

void Connection::stop()
{
    // TODO: close is not dispatched by strand, is it valid???
    m_socket.close();
}

void Connection::do_read()
{
    // Perform async read, which different Connection may run in different thread
    // although it is called in ConnectionManager thread
    // The strand is protected concurrent request on the socket, such as read and close

    auto self(shared_from_this());
    m_socket.async_read_some(buffer(m_buffer), m_strand.wrap(
        [this, self](boost::system::error_code ec, size_t bytes_read) 
        {
            if (!ec && bytes_read > 0) {
                boost::tribool result;
                StoreMessage * pmsg;
               
                copy(m_buffer.data(), m_buffer.data()+bytes_read, back_inserter(m_rcv_buf)); 
                tie(result, pmsg) = m_fact.extract(m_rcv_buf.data(), m_rcv_buf.size());
                if (result) {
                    pmsg->set_source(m_socket.remote_endpoint().address(),
                                     m_socket.remote_endpoint().port());
                    // TODO: pmsg holds pointer to connection, be carefully for error handling
                    pmsg->set_connection(self);

                    getlog()->sendlog(LogLevel::DEBUG, "Store recevied message begin:\n");
                    if (getlog()->is_level_allowed(LogLevel::DEBUG)) {
                        pmsg->dump(getlog()->get_print_handle(), getlog()->is_level_allowed(LogLevel::TRACE));
                    }
                    getlog()->sendlog(LogLevel::DEBUG, "Store recevied message end\n");

                    m_handler.handle_message(pmsg);
                }
                else if (!result) {
                    // here we got error, stop myself, connection manager will erase the pointer to myself
                    // after that, no pointer pointed to this, it will be deleted
                    m_conn_mgr.stop(self);
                }
                else {
                    // indeterminate state
                    do_read();
                }
            }
        }));
}

void Connection::do_write(StoreMessage * pmsg, bool del_msg)
{
    if (pmsg == nullptr) return;

    auto self(shared_from_this());
    boost::asio::async_write(m_socket, buffer(pmsg->get_raw(), pmsg->get_size()), m_strand.wrap(
        [this, self, pmsg, del_msg](boost::system::error_code ec, size_t bytes_write) 
        {
            if (del_msg) {
                delete pmsg;
            }
            if (ec) {
                getlog()->sendlog(LogLevel::ERROR, "Connection '%s:%d' send failed, error=%s\n", 
                                  m_socket.remote_endpoint().address().to_string().c_str(),
                                  m_socket.remote_endpoint().port(),
                                  
                                  ec.message().c_str());
                m_conn_mgr.stop(self);
            }
        }));
}
/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


