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

#include <algorithm>
#include <iterator>

#include <boost/asio.hpp>
#include "MembershipServer.h"
#include "GossipProtocol.h"

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
                  if (l.second < r.second) {
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
          // Too complicate for a lambda function
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
                if (result) {
                    pmsg->set_source(m_sender.address(), m_sender.port());
                    m_prot->handle_messages(pmsg);
                }
                // !!! the memory is freed !!!
                empty_buffer(m_sender);
            }
          }
        });

}

void MembershipServer::do_send(Message * pmsg)
{
    if (pmsg == nullptr) return;

    pair<ip::address, unsigned short> dest = pmsg->get_destination();
    ip::udp::endpoint dest_end(dest.first, dest.second);
    m_udpsock.async_send_to(
              boost::asio::buffer(pmsg->get_raw(), pmsg->get_size()),
              dest_end,
              [this](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
           });
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
    string ip = sender.address().to_string();
    unsigned short port = sender.port();

    auto&& it = m_rcvbuf.find(make_pair(ip, port));
    if (it != m_rcvbuf.end()) {
        sz = it->second.size();
        return it->second.data();
    }

    return nullptr;
}

void MembershipServer::append_buffer(const ip::udp::endpoint & sender, 
                                     unsigned char* buf, size_t sz)
{
    string ip = sender.address().to_string();
    unsigned short port = sender.port();

    auto key = make_pair(ip, port);
    auto&& it = m_rcvbuf.find(key);
    if (it != m_rcvbuf.end()) {
        copy(buf, buf+sz, back_inserter(it->second));
    }
    else {
        m_rcvbuf[key] = move(vector<unsigned char>(buf, buf+sz));
    }
}

void MembershipServer::empty_buffer(const ip::udp::endpoint & sender)
{
    string ip = sender.address().to_string();
    unsigned short port = sender.port();

    auto key = make_pair(ip, port);
    m_rcvbuf.erase(key);
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


