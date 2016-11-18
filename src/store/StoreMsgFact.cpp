/**
 *******************************************************************************
 * StoreMsgFact.cpp                                                            *
 *                                                                             *
 * Store message factory definition                                            *
 *   - Contructs messges from raw buffer received from network                 *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "CreatMessage.h"
#include "ReadMessage.h"
#include "UpdateMessage.h"
#include "DeleteMessage.h"

#include "StoreMsgFact.h"

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

std::tuple<boost::tribool, StoreMessage* > 
   StoreMessageFactory :: extract(unsigned char* buf, 
                                  size_t size)
{
    if (buf == nullptr) {
        getlog()->sendlog(LogLevel::ERROR, "Null buffer passed for parse");
        return std::make_tuple(false, nullptr);
    }

    bool    result;
    size_t  msglen;
    MsgType msgtype;
    StoreMessage * pmsg = nullptr;

    try {
        std::tie(result, msglen, msgtype) = try_parse(buf, size);
        if (result) {
            switch(msgtype) {
            case MsgType::CREATREQ:
                pmsg = new CreatRequestMessage(buf, msglen, false);
                break;
            case MsgType::CREATRESP:
                pmsg = new CreatResponseMessage(buf, msglen, false);
                break;
            case MsgType::READREQ:
                pmsg = new ReadRequestMessage(buf, msglen, false);
                break;
            case MsgType::READRESP:
                pmsg = new ReadResponseMessage(buf, msglen, false);
                break;
            case MsgType::UPDATEREQ:
                pmsg = new UpdateRequestMessage(buf, msglen, false);
                break;
            case MsgType::UPDATERESP:
                pmsg = new UpdateResponseMessage(buf, msglen, false);
                break;
            case MsgType::DELETEREQ:
                pmsg = new DeleteRequestMessage(buf, msglen, false);
                break;
            case MsgType::DELETERESP:
                pmsg = new DeleteResponseMessage(buf, msglen, false);
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

