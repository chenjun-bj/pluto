/**
 *******************************************************************************
 * KVStoreAsyncAccess.h                                                        *
 *                                                                             *
 * Key value store accessor:                                                   *
 *   - Access to KV store                                                      *
 *******************************************************************************
 */

#ifndef _KV_STORE_ASYNC_ACCESSOR_H_
#define _KV_STORE_ASYNC_ACCESSOR_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "KVStore.h"

#include <string>

#include <boost/asio.hpp>

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

class KVStoreAsyncAccessor {
public:
    KVStoreAsyncAccessor(boost::asio::io_service&,
                         KVStore& );
    virtual ~KVStoreAsyncAccessor();

    template<typename RD_HANDLER > 
    int async_read(const std::string& key, int replica_type,
                   RD_HANDLER handler);
    template<typename WR_HANDLER > 
    int async_write(const std::string& key, int replica_type,
                    const unsigned char* value, const size_t sz,
                    WR_HANDLER handler);
    template<typename UP_HANDLER > 
    int async_update(const std::string& key, int replica_type,
                     const unsigned char* value, const size_t sz,
                     UP_HANDLER handler);
    template<typename DEL_HANDLER > 
    int async_delete(const std::string& key, int replica_type,
                     DEL_HANDLER handler);
private:
    boost::asio::io_service::strand m_strand;
    KVStore&                        m_store;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _KV_STORE_ASYNC_ACCESSOR_H_

