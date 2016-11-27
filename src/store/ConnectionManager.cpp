/**
 *******************************************************************************
 * ConnectionManagerManager.cpp                                                       *
 *                                                                             *
 * Tcp connection manager:                                                     *
 *   - Manage TCP connections                                                  *
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

ConnectionManager::ConnectionManager(io_service& io) :
   m_strand(io),
   m_conn_map()
{

}

void ConnectionManager::start(Connection_ptr conn)
{
    m_strand.post([this, conn](){
                     m_conn_map.insert(make_pair(conn->socket().remote_endpoint(), conn));
                     // conn->start() suppose to start an asynchronous operation
                     // so that other threads has a chance to run concurrently
                     conn->start();
                  });
}
 
void ConnectionManager::stop(Connection_ptr conn)
{
    m_strand.post([this, conn](){
                     conn->stop();
                     m_conn_map.erase(conn->socket().remote_endpoint());
                  });
}

void ConnectionManager::stop_all()
{
    m_strand.post([this](){
                     for (auto&& conn : m_conn_map) { 
                         conn.second->stop();
                     }
                     m_conn_map.clear();
                  });
}

void ConnectionManager::send_message(const boost::asio::ip::tcp::endpoint& endpoint, 
                                     StoreMessage * pmsg,
                                     bool del_msg)
{
    m_strand.post([this, endpoint, pmsg, del_msg]() {
                      CM_MAP::iterator it = m_conn_map.find(endpoint);
                      if (it != m_conn_map.end()) {
                          Connection * conn = it->second.get();
                          if (conn) {
                              conn->do_write(pmsg, del_msg);
                              return;
                          }
                      }
                      // The message is not sent
                      if (del_msg) {
                          delete pmsg;
                      }
                  });
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


