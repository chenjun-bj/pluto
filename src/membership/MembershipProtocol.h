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
#include <memory>

#include "stdinclude.h"
#include "memberlist.h"
#include "messages.h"

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

class MembershipProtocol : 
   public std::enable_shared_from_this<MembershipProtocol> 
{
public:
    MembershipProtocol() {
    }
    virtual ~MembershipProtocol() {
    }

    virtual int handle_messages(Message* msg) = 0;
    // Protocol sends heartbeat message periodically and 
    // check member health 
    virtual int handle_timer(int id) = 0;
    // self up, should introduce self to group
    virtual int node_up() = 0;
    // self down, should notify group that I'm leaving
    virtual int node_down() = 0;
    // detect (other) node errors
    virtual int detect_node_error() = 0;
    // disseminate error info
    virtual int disseminate_error() = 0;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _MEMBER_SHIP_PROTOCOL_H_

