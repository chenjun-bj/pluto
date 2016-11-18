/**
 *******************************************************************************
 * StoreMsgFact.h                                                              *
 *                                                                             *
 * Store message factory:                                                      *
 *   - Contructs messges from raw buffer received from network                 *
 *******************************************************************************
 */

#ifndef _STORE_MSG_FACT_H_
#define _STORE_MSG_FACT_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "StoreMessage.h"
#include "CreatMessage.h"
#include "ReadMessage.h"
#include "UpdateMessage.h"
#include "DeleteMessage.h"

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

class StoreMessageFactory {
public:
    StoreMessageFactory() {; }
    ~StoreMessageFactory() {; }

    std::tuple< boost::tribool, StoreMessage *> extract(unsigned char* buf, size_t size);

};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_MSG_FACT_H_

