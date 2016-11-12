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

#include <climits>

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
   m_t(m_io),
   m_snd_que(),
   m_snd_txid(0),
   m_multicast_que(),
   m_multicast_txid(0)
{

    if (m_pcfg->get_protocol() == "GOSSIP") {
        m_prot = std::make_shared<GossipProtocol> (pmemlist, pcfg, this);
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
            getlog()->sendlog(LogLevel::DEBUG, "Membership server received terminate signal\n"); 
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
    getlog()->sendlog(LogLevel::DEBUG, "Membership server exit\n"); 
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

                    getlog()->sendlog(LogLevel::DEBUG, "Membership server received message begin:\n");
                    if (getlog()->is_level_allowed(LogLevel::DEBUG)) {
                        pmsg->dump(getlog()->get_print_handle(), getlog()->is_level_allowed(LogLevel::TRACE));
                    }
                    getlog()->sendlog(LogLevel::DEBUG, "Membership server received message end\n");

                    m_prot->handle_messages(pmsg);
                }
                // !!! the memory is freed !!!
                empty_buffer(m_sender);
                do_receive();
            }
          }
        });

}

void MembershipServer::do_send(Message * pmsg)
{
    getlog()->sendlog(LogLevel::DEBUG, "Membership send message=0x%x\n", pmsg);
    if (pmsg == nullptr) return;

    pair<ip::address, unsigned short> dest = pmsg->get_destination();
    ip::udp::endpoint dest_end(dest.first, dest.second);

    unsigned long long txid = m_snd_txid++;
    m_snd_que[txid] = pmsg;

    if (m_snd_txid == ULLONG_MAX) {
        m_snd_txid = 0;
        // TODO: what about if message of txid 0 has not sent complete?
    }
 
    getlog()->sendlog(LogLevel::DEBUG, "send message txid=%d\n", txid);
    getlog()->sendlog(LogLevel::DEBUG, "send message begin:\n");
    if (getlog()->is_level_allowed(LogLevel::DEBUG)) {
        pmsg->dump(getlog()->get_print_handle(), getlog()->is_level_allowed(LogLevel::TRACE));
    }
    getlog()->sendlog(LogLevel::DEBUG, "send message end\n");

    m_udpsock.async_send_to(
              boost::asio::buffer(pmsg->get_raw(), pmsg->get_size()),
              dest_end,
              [this, txid](boost::system::error_code /*ec*/, std::size_t /*bytes_sent*/) {
                  getlog()->sendlog(LogLevel::DEBUG, "send message complete txid=%d\n", txid);
                  Message * todel = m_snd_que[txid];
                  if (todel) {
                      delete todel;
                  }
                  m_snd_que.erase(txid);
           });
}

void MembershipServer::do_multicast(const vector<ip::udp::endpoint > &ends, Message * pmsg)
{
    getlog()->sendlog(LogLevel::DEBUG, "Membership send multicast message=0x%x\n", pmsg);
    if (pmsg == nullptr) return;

    if (ends.size()==0) {
        getlog()->sendlog(LogLevel::DEBUG, "Membership send multicast no endpoints\n");
        delete pmsg;
        return;
    }

    unsigned long long txid = m_multicast_txid++;
    if (m_multicast_txid == ULLONG_MAX) {
        m_multicast_txid = 0;
        // TODO: what about if message of txid 0 has not sent complete?
    }

    getlog()->sendlog(LogLevel::DEBUG, "multicast message txid=%d, endpoints count %d\n", 
                                       txid,
                                       ends.size());

    m_multicast_que[txid] = make_pair(ends.size(), pmsg);
    auto multi_cmp = [this, txid](boost::system::error_code ec, std::size_t /*bytes_sent*/) {
                        multicast_info ctl = m_multicast_que[txid];
                        int cnt         = ctl.first;
                        Message * todel = ctl.second;
                        getlog()->sendlog(LogLevel::DEBUG, "multicast message txid=%d one endpoint complete, remain %d\n", 
                                                           txid, cnt-1);
                        if (ec) {
                            getlog()->sendlog(LogLevel::DEBUG, "multicast message txid=%d failed, ec=%d\n", 
                                                               txid, ec);
                        }
                        if (--cnt == 0) {
                            getlog()->sendlog(LogLevel::DEBUG, "multicast message done txid=%d\n", txid);
                            if (todel) {
                                delete todel;
                            }
                            m_multicast_que.erase(txid);
                        }
                        else {
                            m_multicast_que[txid] = make_pair(cnt, todel);
                        }
                    };

    getlog()->sendlog(LogLevel::DEBUG, "multicast message begin:\n");
    if (getlog()->is_level_allowed(LogLevel::DEBUG)) {
        pmsg->dump(getlog()->get_print_handle(), getlog()->is_level_allowed(LogLevel::TRACE));
    }
    getlog()->sendlog(LogLevel::DEBUG, "multicast message end\n");

    for (auto&& end : ends) {
        getlog()->sendlog(LogLevel::DEBUG, "multicast to %s:%d\n", 
                                           end.address().to_string().c_str(),
                                           end.port());
        m_udpsock.async_send_to(
                  boost::asio::buffer(pmsg->get_raw(), pmsg->get_size()),
                  end,
                  multi_cmp);
    }
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


