/**
 *******************************************************************************
 * MembershipMsgFact.cpp                                                       *
 *                                                                             *
 * Membership message factory definition                                       *
 *   - Contructs messges from raw buffer received from network                 *
 *******************************************************************************
 */


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

#include "MembershipMsgFact.h"

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

std::tuple<boost::tribool, Message* > 
   MembershipMessageFactory :: extract(unsigned char* buf, 
                                       size_t size)
{
    if (buf == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Null buffer passed for parse");
        return std::make_tuple(false, nullptr);
    }

    boost::tribool result;
    size_t  msglen;
    MsgType msgtype;
    Message * pmsg = nullptr;

    try {
        std::tie(result, msglen, msgtype) = try_parse(buf, size);
        if (result) {
            switch(msgtype) {
            case MsgType::JOINREQ:
                pmsg = new JoinRequestMessage(buf, msglen, false);
                break;
            case MsgType::JOINRESP:
                pmsg = new JoinResponseMessage(buf, msglen, false);
                break;
            case MsgType::HEARTBEAT:
                pmsg = new HeartbeatMessage(buf, msglen, false);
                break;
            case MsgType::PEERLEAVE:
                pmsg = new PeerLeaveMessage(buf, msglen, false);
                break;
            default:
                // Invalid message received
                pmsg = nullptr;
                result = false;
                getlog()->sendlog(LogLevel::ERROR, "Message type not support '%s'\n", get_desc_msgtype(msgtype).c_str());
                break;
            }
            if (pmsg) {
                pmsg->parse_msg();
            }
        } else if (!result) {
            // Error
            getlog()->sendlog(LogLevel::ERROR, "Message try parse failed\n");
        } else {
            // indeterminate state
        }
    }
    catch (parse_error & e) {
        getlog()->sendlog(LogLevel::ERROR, "Exception occurs during try parse: '%s'\n", e.what());
        if (pmsg) {
            delete pmsg;
        }
        result = false;
    }
    catch (std::exception & e) {
        getlog()->sendlog(LogLevel::ERROR, "Unexpected exception: '%s'\n", e.what());
        if (pmsg) {
            delete pmsg;
        }
        result = false;
    }

    return std::make_tuple(result, pmsg);
}

