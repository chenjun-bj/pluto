/**
 *******************************************************************************
 * ReadMessage.h                                                               *
 *                                                                             *
 * Client/Server read request/response message                                 *
 *******************************************************************************
 */

#ifndef _READ_MSG_COMMON_H_
#define _READ_MSG_COMMON_H_

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

class ReadRequestMessage : public KeyReqMessage {
public:
    ReadRequestMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KeyReqMessage(buf, sz, managebuf) {
    };
  
    ReadRequestMessage(MessageOriginator originator,
                       int64 txid) :
        KeyReqMessage(MsgType::READREQ, originator, txid) {
    }

    ~ReadRequestMessage() {
    }
private:
};

class ReadResponseMessage : public KeyRespMessage {
public:
    ReadResponseMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KeyRespMessage(buf, sz, managebuf) {
    }

    ReadResponseMessage(MessageOriginator originator,
                        int64 txid,
                        MsgStatus status) :
       KeyRespMessage(MsgType::READRESP, originator, txid, status) {
    }
    ~ReadResponseMessage() {
    }
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _READ_MSG_COMMON_H_

