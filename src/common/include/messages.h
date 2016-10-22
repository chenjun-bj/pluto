/**
 *******************************************************************************
 * messages.h                                                                  *
 *                                                                             *
 * Message types:                                                              *
 *******************************************************************************
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_


/**
 *******************************************************************************
 * Headers                                                                     *
 *******************************************************************************
 */

/**
 *******************************************************************************
 * Constants                                                                   *
 *******************************************************************************
 */
enum MsgType{
    JOINREQ,
    JOINRESP,
    HEARTBEATMSG,
    PEERLEAVEMSG,
    CREATREQ,
    CREATRESP,
    UPDATEREQ,
    UPDATERESP,
    READREQ,
    READRESP,
    DELETEREQ,
    DELETERESP
}

typedef struct {
    int          magic;
    int          version;
    enum MsgType type;
    int          reserved;
} MsgCommonHdr;

/**
 *******************************************************************************
 * Function declaractions                                                      *
 *******************************************************************************
 */

#endif // _MESSAGES_H_

