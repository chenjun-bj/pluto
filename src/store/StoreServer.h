/**
 *******************************************************************************
 * StoreServer.h                                                               *
 *                                                                             *
 * Store server:                                                               *
 *   - Store server to perform netwrok operations                              *
 *******************************************************************************
 */

#ifndef _STORE_SERVER_H_
#define _STORE_SERVER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <map>
#include <array>
#include <vector>
#include <string>
#include <utility>
#include <memory>

#include <boost/asio.hpp>

#include "stdinclude.h"
#include "memberlist.h"
#include "StoreMessage.h"
#include "StoreMsgFact.h"
#include "StoreMessageHandler.h"

#include "StoreManager.h"

#include "Connection.h"
#include "ConnectionManager.h"

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

class StoreServer {
public:
   explicit StoreServer(ConfigPortal * pcfg, 
                        MemberList   * pmemlist,
                        size_t thread_pool_sz = 200);
 
   void run();

   //void do_receive();
 
   //void do_send(StoreMessage * pmsg);

protected:
   void start_accept();

   void handle_period_timer();
private:
   bool           m_done;

   // The number of threads that will call io_service::run().
   std::size_t             m_thread_pool_sz;

   ConfigPortal*           m_pcfg;

   // The io_service used to perform asynchronous operations.
   boost::asio::io_service m_io;

   // The signal_set is used to register for process termination notifications.
   boost::asio::signal_set m_signals;

   // Acceptor used to listen for incoming connections.
   boost::asio::ip::tcp::acceptor m_acceptor;

   boost::asio::ip::tcp::socket   m_new_sock;

   ConnectionManager              m_conn_mgr;

   StoreMessageFactory            m_fact;

   StoreManager                   m_store;

   StoreMessageHandler            m_handler;
   //Connection_ptr                 m_new_connection;

   boost::asio::deadline_timer    m_timer;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_SERVER_H_

