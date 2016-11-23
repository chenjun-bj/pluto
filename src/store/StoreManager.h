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

    std::vector<struct MemberEntry > get_nodes(const std::string& key );

    int store_read(const std::string& key, int replica_type,
                   unsigned char* value, size_t & sz);
    int store_create(const std::string& key, int replica_type,
                    const unsigned char* value, 
                    const size_t & sz);
    int store_update(const std::string& key, int replica_type,
                     const unsigned char* value, 
                     const size_t & sz);
    int store_delete(const std::string& key, int replica_type);
private:
private:
    KVStore               m_store;
    KVStoreAsyncAccessor  m_store_acc;
    MemberList   *        m_pmember;
    ConfigPortal *        m_pconfig;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_MANAGER_H_

