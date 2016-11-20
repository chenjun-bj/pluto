/**
 *******************************************************************************
 * KVStoreAsyncAccess.h                                                        *
 *                                                                             *
 * Key value store accessor:                                                   *
 *   - Access to KV store                                                      *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"

#include "KVStoreAccess.h"

#include <string>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

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

KVStoreAsyncAccessor::KVStoreAsyncAccessor(io_service& io,
                                           KVStore& store) :
    m_strand(io),
    m_store(store)
{
}

KVStoreAsyncAccessor::~KVStoreAsyncAccessor()
{
}

template<typename RD_HANDLER >
int KVStoreAsyncAccessor::async_read(const std::string& key, int replica_type,
                                     RD_HANDLER handler)
{
    m_strand.post([&, this](){
                      size_t sz = 8192;
                      unsigned char buf[8192] = { '\0' };
                      int rc = m_store.do_read(key, replica_type, buf, sz);
                      handler(rc, buf, sz);
                  });
    return 0;
}

template<typename WR_HANDLER >
int KVStoreAsyncAccessor::async_write(const std::string& key, int replica_type,
                                      const unsigned char* value, const size_t sz,
                                      WR_HANDLER handler)
{
    // TODO: memory pointed by value must be exist when handler called
    m_strand.post([&, this](){
                      int rc = m_store.do_write(key, replica_type, value, sz);
                      handler(rc);
                  });
    return 0;
}

template<typename UP_HANDLER >
int KVStoreAsyncAccessor::async_update(const std::string& key, int replica_type,
                                       const unsigned char* value, const size_t sz,
                                       UP_HANDLER handler)
{
    m_strand.post([&, this](){
                      int rc = m_store.do_update(key, replica_type, value, sz);
                      handler(rc);
                  });
    return 0;
}

template<typename DEL_HANDLER >
int KVStoreAsyncAccessor::async_delete(const std::string& key, int replica_type,
                                       DEL_HANDLER handler)
{
    m_strand.post([&, this](){
                      size_t sz = 8192;
                      unsigned char buf[8192] = { '\0' };
                      int rc = m_store.do_delete(key, replica_type, buf, sz);
                      handler(rc, buf, sz);
                  });
    return 0;
}

