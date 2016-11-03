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
#include <map>
#include <array>
#include <vector>
#include <string>
#include <utility>
#include <memory>

#include <boost/asio.hpp>

#include "stdinclude.h"
#include "memberlist.h"
#include "messages.h"
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

   void do_receive();
 
   void do_send(Message * pmsg);

private:
   void handle_period_timer(); 

   typedef std::pair<std::string, unsigned short> bufkey;
   typedef std::map< bufkey,  std::vector<unsigned char>,
                    bool (*)(const bufkey& l, const bufkey& r) > buftype;

   unsigned char* get_buffer(const boost::asio::ip::udp::endpoint & sender, size_t & sz);
   void append_buffer(const boost::asio::ip::udp::endpoint & sender, 
                      unsigned char* buf, size_t sz);
   void empty_buffer(const boost::asio::ip::udp::endpoint & sender);
private:
   bool           m_done;

#define MAX_RCV_BUF_LEN   8192
   std::array<unsigned char, MAX_RCV_BUF_LEN > m_buf;
   boost::asio::ip::udp::endpoint m_sender;

   buftype        m_rcvbuf;

   ConfigPortal * m_pcfg;

   std::shared_ptr<MembershipProtocol> m_prot;

   MembershipMessageFactory m_fact;

   // The number of threads that will call io_service::run().
   //std::size_t m_thread_pool_sz;

   // The io_service used to perform asynchronous operations.
   boost::asio::io_service m_io;

   // The signal_set is used to register for process termination notifications.
   boost::asio::signal_set m_signals;

   // Acceptor used to listen for incoming connections.
   boost::asio::ip::udp::socket m_udpsock;

   boost::asio::deadline_timer m_t;  // timer

   std::map<unsigned long long, Message* > m_snd_que;
   unsigned long long          m_snd_txid;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _MEMBER_SHIP_SERVER_H_

