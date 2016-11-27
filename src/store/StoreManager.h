/**
 *******************************************************************************
 * StoreManager.h                                                              *
 *                                                                             *
 * Store Manager:                                                              *
 *   - Perform CRUD operations                                                 *
 *******************************************************************************
 */

#ifndef _STORE_MANAGER_H_
#define _STORE_MANAGER_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "KVStore.h"
#include "KVStoreAccess.h"
#include "MemberList.h"

#include <string>
#include <vector>

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

class StoreManager {
public:
    StoreManager(boost::asio::io_service & io,
                 MemberList * pmemlst,
                 ConfigPortal * pcfg);
    ~StoreManager();

    void update_ring();

    std::vector<struct MemberEntry > get_nodes(const std::string& key );

    template<typename RD_HANDLER > 
    void async_read(const std::string& key, int replica_type,
                    RD_HANDLER handler) {
        m_store_acc.async_read(key, replica_type, handler);
    }

    template<typename WR_HANDLER > 
    void async_creat(const std::string& key, int replica_type,
                     const unsigned char* value, const size_t sz,
                     WR_HANDLER handler) {
        m_store_acc.async_write(key, replica_type, value, sz, handler);
    }

    template<typename UP_HANDLER > 
    void async_update(const std::string& key, int replica_type,
                      const unsigned char* value, const size_t sz,
                      UP_HANDLER handler) {
        m_store_acc.async_update(key, replica_type, value, sz, handler);
    }

    template<typename DEL_HANDLER > 
    void async_delete(const std::string& key, int replica_type,
                      DEL_HANDLER handler) {
        m_store_acc.async_delete(key, replica_type, handler);
    }

    // Synchronous operations, which is NOT supported
    int sync_read(const std::string& key, int replica_type,
                  unsigned char* value, size_t & sz);
    int sync_create(const std::string& key, int replica_type,
                    const unsigned char* value, 
                    const size_t & sz);
    int sync_update(const std::string& key, int replica_type,
                    const unsigned char* value, 
                    const size_t & sz);
    int sync_delete(const std::string& key, int replica_type);
private:
    void stabilization_protocol();
private:
    KVStore               m_store;
    KVStoreAsyncAccessor  m_store_acc;
    MemberList   *        m_pmember;
    ConfigPortal *        m_pconfig;

    std::vector<MemberEntry > m_ring;

    boost::asio::io_service::strand m_ring_strand;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_MANAGER_H_

