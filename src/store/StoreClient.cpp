/**
 *******************************************************************************
 * StoreClient.cpp                                                             *
 *                                                                             *
 * Store client:                                                               *
 *   - Store client to that call server for CRUD                               *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreClient.h"

#include <array>
#include <algorithm>
#include <iterator>

#include <boost/bind.hpp>

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
StoreClient::StoreClient(ConfigPortal * pcfg,
                         boost::asio::io_service & io) :
   m_pcfg(pcfg),
   m_sock(io),
   m_rcv_buf(),
   m_buf(),
   m_msgfact()
{
}

template<typename HANDLER>
void StoreClient::async_call(boost::asio::ip::tcp::endpoint & endpoint,
                             StoreMessage* preq,
                             HANDLER cmpl_handler)
{
    if (preq==nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Client aysnc_call got nullptr\n");
        cmpl_handler(ClientErrorCode::ERROR_INVALID_PARAM, nullptr);
        return;
    }

    m_sock.async_connect(endpoint, 
                         boost::bind(&StoreClient::handle_connect<HANDLER>, this, 
                                     boost::asio::placeholders::error, preq,
                                     cmpl_handler));
}

template<typename HANDLER>
void StoreClient::handle_connect(const boost::system::error_code& ec,
                                 StoreMessage* preq,
                                 HANDLER cmpl_handler)
{
    if (!ec) {
        boost::asio::async_write(m_sock, boost::asio::buffer(preq->get_raw(), preq->get_size()),
                                 boost::bind(&StoreClient::handle_write<HANDLER>, this,
                                             boost::asio::placeholders::error,
                                             cmpl_handler));
                                    
    }
    else {
        cmpl_handler(ClientErrorCode::ERROR_IO, nullptr);
    }
}

template<typename HANDLER>
void StoreClient::handle_write(const boost::system::error_code& ec,
                               HANDLER cmpl_handler)
{
    if (!ec) {
        do_read(cmpl_handler);
    }
    else {
        cmpl_handler(ClientErrorCode::ERROR_IO, nullptr);
    }
}

template<typename HANDLER>
void StoreClient::do_read(HANDLER cmpl_handler)
{
    m_sock.async_read_some(boost::asio::buffer(m_buf), 
                           [this, cmpl_handler](const boost::system::error_code& ec,
                                                size_t bytes_read) {
                               if (!ec) {
                                   StoreMessage * presp = nullptr;
                                   boost::tribool result;
                                   copy(m_buf.data(), m_buf.data()+bytes_read, back_inserter(m_rcv_buf));
                                   std::tie(result, presp) = m_msgfact.extract(m_rcv_buf.data(), m_rcv_buf.size());
                                   if (result) {
                                       cmpl_handler(ClientErrorCode::SUCCESS, presp);
                                   }
                                   else if (!result) {
                                       cmpl_handler(ClientErrorCode::ERROR_MSG, nullptr);
                                   }
                                   else {
                                       do_read(cmpl_handler);
                                   }
                               }
                               else {
                                   cmpl_handler(ClientErrorCode::ERROR_IO, nullptr);
                               }
                           });
}

/* eof */

