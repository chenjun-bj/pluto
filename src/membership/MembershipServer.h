/**
 *******************************************************************************
 * MembershipServer.h                                                          *
 *                                                                             *
 * Membership server:                                                          *
 *   - Membership server to perform netwrok operations                         *
 *******************************************************************************
 */

#ifndef _MEMBER_SHIP_SERVER_H_
#define _MEMBER_SHIP_SERVER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */

#include <boost/asio.hpp>

#include "stdinclude.h"
#include "memberlist.h"
#include "MembershipMsgFact.h"
#include "MembershipProtocol.h"
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

class MembershipServer {
public:
   explicit MembershipServer(ConfigPortal * pcfg, 
                             MemberList   * pmemlist,
                             size_t thread_pool_size /*Not support*/);
 
   void run();

private:
   void do_receive();
 
   void do_send();
 
   // Handle a request to stop the server.
   void handle_stop();

private:
   ConfigPortal * m_pcfg;

   MembershipProtocol * m_prot;

   MembershipMessageFactory m_fact;

   // The number of threads that will call io_service::run().
   std::size_t m_thread_pool_sz;

   // The io_service used to perform asynchronous operations.
   boost::asio::io_service m_io_service;

   // The signal_set is used to register for process termination notifications.
   boost::asio::signal_set m_signals;

   // Acceptor used to listen for incoming connections.
   boost::asio::ip::udp::socket m_udpsock;

};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _MEMBER_SHIP_SERVER_H_

