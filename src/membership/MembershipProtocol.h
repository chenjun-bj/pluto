/**
 *******************************************************************************
 * MembershipProtocol.h                                                        *
 *                                                                             *
 * Membership protocol :                                                       *
 *   - The membership protocol, it defines required interface that to maintain * 
 *     membership. Mainly there're two tasks: error detection & dissemination  *
 *******************************************************************************
 */

#ifndef _MEMBER_SHIP_PROTOCOL_H_
#define _MEMBER_SHIP_PROTOCOL_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "memberlist.h"

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

class MembershipProtocol{
public:
    MembershipProtocol();
    virtual ~MembershipProtocol();

    int handleMessages(unsigned char* buf, size_t len) = 0;
    int handleTimerEvent(int id) = 0;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _MEMBER_SHIP_PROTOCOL_H_

