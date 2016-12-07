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
#include "pladdress.h"
#include "KVStore.h"
#include "KVStoreAccess.h"
#include "memberlist.h"
#include "ClientTransaction.h"

#include <string>
#include <vector>
#include <functional>

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

    template<typename H >
    void async_get_nodes(const std::string& key, H handler ) {
        std::hash<std::string> hashfunc;
        size_t hash_code = hashfunc(key);
        m_ring_strand.post([this, hash_code, handler]() {
                              std::vector< MemberEntry > v;
                              if (m_ring.size() == 0) {
                                  handler(v);
                                  return;
                              }
                              size_t pos = hash_code % m_pconfig->get_ringsize();
                              size_t node_cnt = m_ring.size();
                              if (node_cnt >= PLUTO_NODE_REPLICAS_NUM) {
                                  int i = 0;
                                  if (pos > m_ring[node_cnt-1].hashcode % m_pconfig->get_ringsize()) {
                                      i=0;
                                  } else {
                                      for (i=0; i<node_cnt; i++) {
                                          if (pos <= m_ring[i].hashcode % m_pconfig->get_ringsize()) {
                                              // push back
                                              break;
                                          }
                                      }
                                  }
                              }
                              handler(v);
                           });
    }

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
                  unsigned char* value, size_t & sz) {
        return PLERROR;
    }
    int sync_create(const std::string& key, int replica_type,
                    const unsigned char* value, 
                    const size_t & sz) {
        return PLERROR;
    }
    int sync_update(const std::string& key, int replica_type,
                    const unsigned char* value, 
                    const size_t & sz) {
        return PLERROR;
    }
    int sync_delete(const std::string& key, int replica_type) {
        return PLERROR;
    }
private:
    // Following functions are called by ring strand!!!
    std::vector<struct MemberEntry > get_nodes(const std::string& key );
    void stabilization_protocol();

    void find_neighbors(std::vector<MemberEntry >& left, 
                        std::vector<MemberEntry >& right);
    bool is_node_in_ring(const MemberEntry& node);

    void handle_neighbor_leave(const MemberEntry& node, int replica_type);
    void remove_left_neighbor_replicas(int replica_type);
    void send_right_neighbor_replicas(const MemberEntry& node, int replica_type);
private:
    KVStore               m_store;
    KVStoreAsyncAccessor  m_store_acc;
    MemberList   *        m_pmember;
    ConfigPortal *        m_pconfig;

    std::vector<MemberEntry > m_ring;

    // For stablization protocol
    std::vector<MemberEntry > m_has_my_replicas;
    std::vector<MemberEntry > m_has_replicas_of;

    boost::asio::io_service::strand m_ring_strand;
    boost::asio::io_service::strand m_clnt_strand;

    // self address
    int            m_self_af;                                                            
    unsigned char  m_self_rawip[PL_IPv6_ADDR_LEN];                                       
    unsigned short m_self_port; 
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */

#endif // _STORE_MANAGER_H_

