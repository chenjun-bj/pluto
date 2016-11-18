/**
 *******************************************************************************
 * UpdateMessage.h                                                             *
 *                                                                             *
 * Client/Server Updatee request/response message                              *
 *******************************************************************************
 */

#ifndef _UPDATE_MSG_COMMON_H_
#define _UPDATE_MSG_COMMON_H_

/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */
#include <string>
#include <stdexcept>

#include "stdinclude.h"
#include "KVMessage.h"
#include "plexcept.h"

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Class declaraction                                                          *
 *******************************************************************************
 */

class UpdateRequestMessage : public KVReqMessage {
public:
    UpdateRequestMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KVReqMessage(buf, sz, managebuf) {
    };
  
    UpdateRequestMessage(MessageOriginator originator,
                        int64 txid) :
        KVReqMessage(MsgType::UPDATEREQ, originator, txid) {
    }

    ~UpdateRequestMessage() {
    }
private:
};

class UpdateResponseMessage : public KVRespMessage {
public:
    UpdateResponseMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KVRespMessage(buf, sz, managebuf) {
    }
    UpdateResponseMessage(MessageOriginator originator,
                          int64 txid,
                          MsgStatus status) :
       KVRespMessage(MsgType::UPDATERESP, originator, txid, status) {
    }
    ~UpdateResponseMessage() {
    }
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _UPDATE_MSG_COMMON_H_

