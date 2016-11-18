/**
 *******************************************************************************
 * CreatMessage.h                                                              *
 *                                                                             *
 * Client/Server Create request/response message                               *
 *******************************************************************************
 */

#ifndef _CREAT_MSG_COMMON_H_
#define _CREAT_MSG_COMMON_H_

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

class CreatRequestMessage : public KVReqMessage {
public:
    CreatRequestMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KVReqMessage(buf, sz, managebuf) {
    };
  
    CreatRequestMessage(MessageOriginator originator,
                        int64 txid) :
        KVReqMessage(MsgType::CREATREQ, originator, txid) {
    }

    ~CreatRequestMessage() {
    }
private:
};

class CreatResponseMessage : public KVRespMessage {
public:
    CreatResponseMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KVRespMessage(buf, sz, managebuf) {
    }
    CreatResponseMessage(MessageOriginator originator,
                          int64 txid,
                          MsgStatus status) :
       KVRespMessage(MsgType::CREATRESP, originator, txid, status) {
    }
    ~CreatResponseMessage() {
    }
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _CREAT_MSG_COMMON_H_

