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
                        boost::asio::io_service& io);

   /* HANDLER signature:
      void (int, StoreMessage*&);
    */ 
   template<typename HANDLER> 
   void async_call(boost::asio::ip::tcp::endpoint &, 
                   StoreMessage* preq, 
                   HANDLER cmpl_handler);
protected:
   template<typename HANDLER> 
   void handle_connect(const boost::system::error_code& ec, 
                       StoreMessage* preq,
                       HANDLER cmpl_handler);
   template<typename HANDLER> 
   void handle_write(const boost::system::error_code& ec, 
                     HANDLER cmpl_handler);
   template<typename HANDLER> 
   void do_read(HANDLER cmpl_handler);
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

