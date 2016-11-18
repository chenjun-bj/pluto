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
   m_acceptor(m_io)
{

    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
#if defined(SIGQUIT)
    m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
    m_signals.async_wait([this](const boost::system::error_code ec, int signum) {
            m_done = true;
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

    //start_accept();
}
 
void StoreServer::run()
{
    m_done = false;

    vector<shared_ptr<thread> > thrds;
    for (size_t i=0; i<m_thread_pool_sz; i++) {
        shared_ptr<thread> thrd(new thread([this](){ m_io.run(); }));
        thrds.push_back(thrd);
    }

    for (size_t i=0; i<thrds.size(); i++) {
        thrds[i]->join();
    }

    m_done = true;
    getlog()->sendlog(LogLevel::DEBUG, "Store server exit\n"); 
}

void StoreServer::do_receive()
{
}

void StoreServer::do_send(StoreMessage * pmsg)
{
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


