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
int KVStoreAsyncAccessor::async_read(const std::string& key, 
                                     RD_HANDLER handler)
{
    return 0;
}

template<typename WR_HANDLER >
int KVStoreAsyncAccessor::async_write(const std::string& key, 
                                      const unsigned char* value, const size_t sz,
                                      WR_HANDLER handler)
{
    return 0;
}

template<typename UP_HANDLER >
int KVStoreAsyncAccessor::async_update(const std::string& key,
                                       const unsigned char* value, const size_t sz,
                                       UP_HANDLER handler)
{
    return 0;
}

template<typename DEL_HANDLER >
int KVStoreAsyncAccessor::async_delete(const std::string& key, 
                                       DEL_HANDLER handler)
{
    return 0;
}

