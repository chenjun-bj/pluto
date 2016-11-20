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
}

void ConnectionManager::stop_all()
{
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


