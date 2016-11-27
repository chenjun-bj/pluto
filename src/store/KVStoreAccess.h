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

#include <vector>
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
    KVStoreAsyncAccessor(boost::asio::io_service& io,
                         KVStore& store) :
        m_strand(io),
        m_store(store) {
    }
    virtual ~KVStoreAsyncAccessor() {
    }

    template<typename RD_HANDLER > 
    void async_read(const std::string& key, int replica_type,
                    RD_HANDLER handler) {
        m_strand.post([&, this](){
                          std::vector<unsigned char> v;
                          int rc = m_store.do_read(key, replica_type, v);
                          handler(rc, v.data(), v.size());
                      });
    }

    template<typename WR_HANDLER > 
    void async_write(const std::string& key, int replica_type,
                     const unsigned char* value, const size_t sz,
                     WR_HANDLER handler)
    {
        // TODO: memory pointed by value must be exist when handler called
        m_strand.post([&, this](){
                          std::vector<unsigned char> v;
                          v.resize(sz);
                          memcpy(v.data(), value, sz);
                          int rc = m_store.do_write(key, replica_type, v);
                          handler(rc);
                      });
    }

    template<typename UP_HANDLER > 
    void async_update(const std::string& key, int replica_type,
                      const unsigned char* value, const size_t sz,
                      UP_HANDLER handler) {
        m_strand.post([&, this](){
                          std::vector<unsigned char> v;
                          v.resize(sz);
                          memcpy(v.data(), value, sz);
                          int rc = m_store.do_update(key, replica_type, v);
                          handler(rc);
                      });
    }

    template<typename DEL_HANDLER > 
    void async_delete(const std::string& key, int replica_type,
                      DEL_HANDLER handler) {
        m_strand.post([&, this](){
                          int rc = m_store.do_delete(key, replica_type);
                          handler(rc);
                      });
    }
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

