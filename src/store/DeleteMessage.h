/**
 *******************************************************************************
 * DeleteMessage.h                                                             *
 *                                                                             *
 * Client/Server read request/response message                                 *
 *******************************************************************************
 */

#ifndef _DELETE_MSG_COMMON_H_
#define _DELETE_MSG_COMMON_H_

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

class DeleteRequestMessage : public KeyReqMessage {
public:
    DeleteRequestMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KeyReqMessage(buf, sz, managebuf) {
    };
  
    DeleteRequestMessage(MessageOriginator originator,
                       int64 txid) :
        KeyReqMessage(MsgType::DELETEREQ, originator, txid) {
    }

    ~DeleteRequestMessage() {
    }
private:
};

class DeleteResponseMessage : public KeyRespMessage {
public:
    DeleteResponseMessage(unsigned char* buf, const size_t sz, bool managebuf = false) :
       KeyRespMessage(buf, sz, managebuf) {
    }

    DeleteResponseMessage(MessageOriginator originator,
                        int64 txid,
                        MsgStatus status) :
       KeyRespMessage(MsgType::DELETERESP, originator, txid, status) {
    }
    ~DeleteResponseMessage() {
    }
};

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _DELETE_MSG_COMMON_H_

