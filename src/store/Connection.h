/**
 *******************************************************************************
 * Connection.h                                                                *
 *                                                                             *
 * Tcp connection:                                                             *
 *   - Represents a TCP connection                                             *
 *******************************************************************************
 */

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */

#include <memory>

#include <boost/asio.hpp>

#include "stdinclude.h"
#include "StoreMessage.h"
#include "StoreMsgFact.h"
#include "StoreMessageHandler.h"

#define MAX_RCV_BUF_LEN      8196

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

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(const Connection& ) = delete;
    Connection& operator=(const Connection& ) = delete;

    explicit Connection(boost::asio::ip::tcp::socket sock,
                        boost::asio::io_service & io,
                        ConnectionManager & conn_mgr,
                        StoreMessageHandler& handler,
                        StoreMessageFactory& fact);

    void start();

    void stop();

    const boost::asio::ip::tcp::socket& socket() {
        return m_socket;
    }

    void do_write(StoreMessage * pmsg, bool del_msg=true);

protected:
    void do_read();

private:
    boost::asio::io_service::strand m_strand;

    boost::asio::ip::tcp::socket    m_socket;

    ConnectionManager &             m_conn_mgr;

    StoreMessageHandler &           m_handler;

    StoreMessageFactory &           m_fact;

    std::array<unsigned char, MAX_RCV_BUF_LEN > m_buffer;

    std::vector<unsigned char>      m_rcv_buf;
};

typedef std::shared_ptr< Connection > Connection_ptr;

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _CONNECTION_H_

