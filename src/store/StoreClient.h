/**
 *******************************************************************************
 * StoreClient.h                                                               *
 *                                                                             *
 * Store client:                                                               *
 *   - Store client to that call server for CRUD                               *
 *******************************************************************************
 */

#ifndef _STORE_CLIENT_H_
#define _STORE_CLIENT_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <array>
#include <vector>
#include <boost/asio.hpp>

#include "stdinclude.h"
#include "StoreMessage.h"
#include "StoreMsgFact.h"

/*
 *******************************************************************************
 *  Forward declaraction                                                       *
 *******************************************************************************
 */
enum class ClientErrorCode : int {
    SUCCESS = 0,
    GENERIC_ERROR,
    ERROR_INVALID_PARAM,
    ERROR_IO,
    ERROR_MSG
};

/*
 *******************************************************************************
 *  Class declaraction                                                         *
 *******************************************************************************
 */

class StoreClient {
public:
   explicit StoreClient(ConfigPortal * pcfg, 
                        boost::asio::io_service& io) :
      m_pcfg(pcfg),
      m_sock(io),
      m_rcv_buf(),
      m_buf(),
      m_msgfact()
   {
       m_pcfg->get_message_timeout();
   }

   /* HANDLER signature:
      void (const ClientErrorCode&, StoreMessage*);
    */ 
   template<typename HANDLER> 
   void async_call(boost::asio::ip::tcp::endpoint & ep, 
                   StoreMessage* preq, 
                   HANDLER cmpl_handler) {
       if (preq==nullptr) {
           getlog()->sendlog(LogLevel::ERROR, "Client aysnc_call got nullptr\n");
           cmpl_handler(ClientErrorCode::ERROR_INVALID_PARAM, nullptr);
           return;
       }

       m_sock.async_connect(ep, 
                            boost::bind(&StoreClient::handle_connect<HANDLER>, this, 
                                        boost::asio::placeholders::error, preq,
                                        cmpl_handler));
   }
protected:
   template<typename HANDLER> 
   void handle_connect(const boost::system::error_code& ec, 
                       StoreMessage* preq,
                       HANDLER cmpl_handler) {
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
   void handle_write(const boost::system::error_code& ec, 
                     HANDLER cmpl_handler) {
        if (!ec) {
            do_read(cmpl_handler);
        }
        else {
            cmpl_handler(ClientErrorCode::ERROR_IO, nullptr);
        }
   }

   template<typename HANDLER> 
   void do_read(HANDLER cmpl_handler) {
       m_sock.async_read_some(boost::asio::buffer(m_buf), 
                              [this, cmpl_handler](const boost::system::error_code& ec,
                                                   size_t bytes_read) {
                                  if (!ec) {
                                      StoreMessage * presp = nullptr;
                                      boost::tribool result;
                                      std::copy(m_buf.data(), m_buf.data()+bytes_read, back_inserter(m_rcv_buf));
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
private:
   ConfigPortal*           m_pcfg;

   boost::asio::ip::tcp::socket m_sock;

   std::vector<unsigned char>   m_rcv_buf;

   std::array<unsigned char, 4096> m_buf;
   StoreMessageFactory          m_msgfact;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_CLIENT_H_

