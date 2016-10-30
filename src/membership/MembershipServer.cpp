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

#include <boost/asio.hpp>
#include "MembershipServer.h"

using namespace boost::asio;

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
   m_pcfg(nullptr),
   m_prot(nullptr),
   m_signals(m_io_service),
   m_udpsock(m_io_service)
{
    m_signals.add(SIGINT);
    m_signals.add(SIGTERM);
#if defined(SIGQUIT)
    m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
    m_signals.async_wait(boost::bind(&MembershipServer::handle_stop, this));

    // TODO: Bind to address

    do_receive();
}
 
void MembershipServer::run()
{
    m_io_service.run();
}

void MembershipServer::do_receive()
{
    unsigned char data[1024] = { '\0' };
    ip::udp::endpoint sender;
    m_udpsock.async_receive_from(
        boost::asio::buffer(data, 1024), sender,
        [this, &data, sender](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec && bytes_recvd > 0)
          {
            boost::tribool result;
            Message * pmsg;
            std::tie(result, pmsg) = m_fact.extract((unsigned char*)data, bytes_recvd);
            if (boost::indeterminate(result)) {
                do_receive();
            }
            else {
                m_prot->handle_messages(pmsg);
            }
          }
        });

}

void MembershipServer::do_send()
{
}

void MembershipServer::handle_stop()
{
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


