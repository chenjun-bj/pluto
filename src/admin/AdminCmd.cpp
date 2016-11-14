/**
 *******************************************************************************
 * AdminCmd.cpp                                                                *
 *                                                                             *
 * Node administration commands implementation                                 *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <iostream>

#include <boost/asio.hpp>
#include <boost/format.hpp>

#include <sys/socket.h>

#include "AdminCmd.h"

using namespace std;
using namespace boost::asio;

using boost::format;
using boost::io::group;

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

cmdproc::~cmdproc()
{
}

void cmdproc::register_cmd(const std::string& cmd, admincmd * pcmd)
{
}

void cmdproc::handle_input(const std::string& line) 
{
}

int lst_member::operator()(const std::string& )
{
    if (m_pmlst == nullptr) {
        return PLERROR;
    }

    format fmter("Address: %1% Port: %2% Heartbeat: %3% Last_tm: %4%");
    for (auto&& v : *m_pmlst) {
        ip::address addr = rawip2address(v.af, v.address);
        fmter % addr.to_string() % v.portnumber % v.heartbeat % v.tm_lasthb;
        cout << fmter << endl;
    }

    return PLSUCCESS;
}

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */


