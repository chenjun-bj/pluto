/**
 *******************************************************************************
 * StoreServer.cpp                                                             *
 *                                                                             *
 * Store server:                                                               *
 *   - Store server to perform netwrok operations                              *
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
#include "StoreServer.h"
#include "KVMessage.h"

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

StoreServer::StoreServer(ConfigPortal * pcfg, 
                         MemberList   * pmemlist,
                         size_t pool_sz) :
   m_done(true),
   m_thread_pool_sz(pool_sz),
   m_pcfg(pcfg),
   m_io(),
   m_signals(m_io),
   m_acceptor(m_io),
   m_new_sock(m_io),
   m_conn_mgr(m_io),
   m_fact(),
   m_store(m_io, pmemlist, pcfg),
   m_handler(m_io, m_conn_mgr, m_store, pcfg, true),
   m_timer(m_io)
{

    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
#if defined(SIGQUIT)
    m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
    m_signals.async_wait([this](const boost::system::error_code ec, int signum) {
            m_done = true;

            m_timer.cancel();

            m_acceptor.close();
            m_conn_mgr.stop_all();

            getlog()->sendlog(LogLevel::DEBUG, "Store server received terminate signal\n"); 
        });

    // Bind to address
    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(m_pcfg->get_bindip(), 
                                                ec);
    boost::asio::detail::throw_error(ec, "ip");

    ip::tcp::endpoint bind_addr(addr, m_pcfg->get_bindport());

    m_acceptor.open(bind_addr.protocol());
    m_acceptor.set_option(ip::tcp::acceptor::reuse_address(true));
    m_acceptor.bind(bind_addr);
    m_acceptor.listen();

    start_accept();

    m_timer.expires_from_now(boost::posix_time::seconds(m_pcfg->get_membership_period()));
    m_timer.async_wait(boost::bind(&StoreServer::handle_period_timer, this));
}
 
void StoreServer::run()
{
    m_done = false;

    vector<shared_ptr<thread> > thrds;
    for (size_t i=0; i<m_thread_pool_sz; i++) {
        thrds.push_back(make_shared<thread >([this](){ m_io.run(); }));
    }

    for (size_t i=0; i<thrds.size(); i++) {
        thrds[i]->join();
    }

    m_done = true;
    getlog()->sendlog(LogLevel::DEBUG, "Store server exit\n"); 
}

void StoreServer::start_accept()
{
    m_acceptor.async_accept(m_new_sock,
                            [this](const boost::system::error_code & ec) {
                                if (!m_acceptor.is_open()) {
                                }
                                if (!ec) {
                                    m_conn_mgr.start(make_shared<Connection >(
                                        move(m_new_sock), m_io, m_conn_mgr, m_handler, m_fact));
                                }
                                start_accept();
                            });
}

void StoreServer::handle_period_timer()
{
    if (m_done) return;
   
    m_handler.handle_time_event();
    m_store.update_ring();
 
    m_timer.expires_from_now(boost::posix_time::seconds(m_pcfg->get_membership_period()));
    m_timer.async_wait(boost::bind(&StoreServer::handle_period_timer, this));
}

//void StoreServer::do_receive()
//{
//}
//
//void StoreServer::do_send(StoreMessage * pmsg)
//{
//}
//
/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


