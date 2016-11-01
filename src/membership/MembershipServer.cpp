/**
 *******************************************************************************
 * MembershipServer.cpp                                                        *
 *                                                                             *
 * Membership server:                                                          *
 *   - Membership server to perform netwrok operations                         *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include <boost/asio.hpp>
#include "MembershipServer.h"
#include "GossipProtocol.h"

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

MembershipServer::MembershipServer(ConfigPortal * pcfg, 
                                   MemberList   * pmemlist,
                                   size_t thread_pool_size /*Not support*/) :
   m_done(true),
   m_rcvbuf([](const bufkey &l, const bufkey &r)->bool {
              if (l.first < r.first ) {
                  return true;
              }
              else if (l.first > r.first) {
                  return false;
              }
              else {
                  if (l.second <= r.second) {
                      return true;
                  }
                  return false;
              }
            }),
   m_pcfg(pcfg),
   m_prot(nullptr),
   m_fact(),
   m_io(),
   m_signals(m_io),
   m_udpsock(m_io),
   m_t(m_io)
{

    if (m_pcfg->get_protocol() == "GOSSIP") {
        m_prot = std::make_shared<GossipProtocol> (pmemlist, pcfg);
    }
    else {
        throw config_error("Protocol not support");
    }

    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
#if defined(SIGQUIT)
    m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
    //m_signals.async_wait(boost::bind(&MembershipServer::handle_stop, this));
    m_signals.async_wait([this](const boost::system::error_code ec, int signum) {
            m_done = true;
            m_udpsock.cancel();
            m_t.cancel();
            m_udpsock.close();
        });

    // Bind to address
    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(m_pcfg->get_bindip(), 
                                                ec);
    boost::asio::detail::throw_error(ec, "ip");

    ip::udp::endpoint bind_addr(addr, m_pcfg->get_bindport());

    const ip::udp::endpoint::protocol_type protocol = bind_addr.protocol();
    m_udpsock.open(protocol, ec);
    boost::asio::detail::throw_error(ec, "open");
    m_udpsock.bind(bind_addr, ec);
    boost::asio::detail::throw_error(ec, "bind");

    // period timer
    m_t.expires_from_now(boost::posix_time::seconds(m_pcfg->get_membership_period()));
    m_t.async_wait(boost::bind(&MembershipServer::handle_period_timer, this));
    
    m_prot->node_up();

    do_receive();
}
 
void MembershipServer::run()
{
    m_done = false;
    m_io.run();
}

void MembershipServer::do_receive()
{
    m_udpsock.async_receive_from(
        boost::asio::buffer(m_buf), m_sender,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec && bytes_recvd > 0)
          {
            size_t size;
            boost::tribool result;
            Message * pmsg;
            unsigned char* data = get_buffer(m_sender, size);
            if (data!=nullptr) {
                std::tie(result, pmsg) = m_fact.extract(data, size);
            } else {
                std::tie(result, pmsg) = m_fact.extract(m_buf.data(), bytes_recvd);
            }
            if (boost::indeterminate(result)) {
                append_buffer(m_sender, m_buf.data(), bytes_recvd);
                do_receive();
            }
            else {
                empty_buffer(m_sender);
                if (result) {
                    m_prot->handle_messages(pmsg);
                }
            }
          }
        });

}

void MembershipServer::do_send(Message * pmsg)
{
}

void MembershipServer::handle_period_timer()
{
    if (m_done) return;
    m_prot->handle_timer(0 /*TODO: add timer id*/);
    m_t.expires_from_now(boost::posix_time::seconds(m_pcfg->get_membership_period()));
    m_t.async_wait(boost::bind(&MembershipServer::handle_period_timer, this));
}

unsigned char* MembershipServer::get_buffer(const ip::udp::endpoint & sender, 
                                            size_t & sz)
{
    return nullptr;
}

void MembershipServer::append_buffer(const ip::udp::endpoint & sender, 
                                     unsigned char* buf, size_t sz)
{
}

void MembershipServer::empty_buffer(const ip::udp::endpoint & sender)
{
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


