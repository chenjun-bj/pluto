/**
 *******************************************************************************
 * ConnectionManager.cpp                                                       *
 *                                                                             *
 * Tcp connection manager:                                                     *
 *   - Manage TCP connections                                                  *
 *******************************************************************************
 */

#ifndef _CONNECTION_MANAGER_H_
#define _CONNECTION_MANAGER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */

#include <memory>
#include <map>

#include <boost/asio.hpp>

#include "stdinclude.h"
#include "StoreMessage.h"

#include "Connection.h"

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

class ConnectionManager {
public:
    explicit ConnectionManager(boost::asio::io_service & io);

    void start(Connection_ptr conn);

    void stop(Connection_ptr conn);

    void stop_all();

private:
    typedef std::map< boost::asio::ip::tcp::endpoint, Connection_ptr > 
            CM_MAP;
private:
    // User strand to protect m_conn_map
    boost::asio::io_service::strand m_strand;

    CM_MAP                          m_conn_map;
};


/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _CONNECTION_MANAGER_H_

