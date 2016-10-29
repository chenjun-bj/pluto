/**
 *******************************************************************************
 * MembershipMsgFact.h                                                         *
 *                                                                             *
 * Membership message factory:                                                 *
 *   - Contructs messges from raw buffer received from network                 *
 *******************************************************************************
 */

#ifndef _MEMBER_MSG_FACT_H_
#define _MEMBER_MSG_FACT_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "JoinReqMsg.h"
#include "JoinRespMsg.h"
#include "HeartbeatMsg.h"
#include "PeerLeaveMsg.h"

#include <tuple>
#include <boost/logic/tribool.hpp>

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

class MembershipMessageFactory {
public:
    MembershipMessageFactory() {; }
    ~MembershipMessageFactory() {; }

    std::tuple< boost::tribool, Message *> extract(unsigned char* buf, size_t size);

};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _MEMBER_MSG_FACT_H_

