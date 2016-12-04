/**
 *******************************************************************************
 * StoreManager.cpp                                                            *
 *                                                                             *
 * Store Manager                                                               *
 *   - Perform CRUD operation                                                  *
 *******************************************************************************
 */


/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include "stdinclude.h"
#include "StoreManager.h"

#include <string>
#include <functional>
#include <algorithm>
#include <iterator>
#include <vector>

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

StoreManager::StoreManager(io_service & io,
                           MemberList * pmemlst,
                           ConfigPortal * pcfg) :
   m_store(),
   m_store_acc(io, m_store),
   m_pmember(pmemlst),
   m_pconfig(pcfg),
   m_ring(),
   m_ring_strand(io),
   m_clnt_strand(io)
{
}

StoreManager::~StoreManager()
{
}

void StoreManager :: update_ring()
{
    // TODO: MemberList is updated by Membership protocol (another process)
    //       so the access should be protected by a lock

    std::vector< MemberEntry > cur_memlist;

    // !!! TODO: LOCK member list !!! 
    std::copy(m_pmember->begin(), m_pmember->end(), std::back_inserter( cur_memlist));
    // !!! UNLOCK here !!!

    m_ring_strand.post([this, cur_memlist]() {
        bool change = false;
        // TODO: lock ring !!!
        if (cur_memlist.size() != m_ring.size()) {
            change = true;
        }
        else {
            for (int i=0; i<cur_memlist.size(); i++) {
                if (cur_memlist[i] != m_ring[i]) {
                    change=true;
                    break;
                }
            }
        }

        if (change) {
            m_ring.clear();
            std::copy(cur_memlist.begin(), cur_memlist.end(), std::back_inserter(m_ring));
            // run stablization protocol
            stabilization_protocol();
        }});
}

vector<struct MemberEntry > StoreManager :: get_nodes(const string& key)
{
    vector<struct MemberEntry > v;

    std::hash<string> hashfunc;
    size_t hash_code = hashfunc(key);
  
    if (m_ring.size() == 0) return v;
 
    size_t pos = hash_code % m_pconfig->get_ringsize();

    // !!! TODO: LOCK !!!   
    size_t node_cnt = m_ring.size();
    if (node_cnt >= PLUTO_NODE_REPLICAS_NUM) {
        int i = 0;
        if (pos > m_ring[node_cnt-1].hashcode % m_pconfig->get_ringsize()) {
            i = 0;
        } else {
            for (i=0; i<node_cnt; i++) {
                if (pos <= m_ring[i].hashcode % m_pconfig->get_ringsize()) {
                    // push back
                    break;
                }
            }
        }
        for (; v.size()<PLUTO_NODE_REPLICAS_NUM; i++) {
            v.push_back(m_ring[i%node_cnt]);
        }
    }
  
    return v;
}

void StoreManager::stabilization_protocol()
{
}

// Synchronous operation is NOT supported
int StoreManager::sync_read(const std::string& key, int replica_type,
                            unsigned char* value, size_t & sz)
{
    return PLERROR;
}

int StoreManager::sync_create(const std::string& key, int replica_type,
                              const unsigned char* value, 
                              const size_t & sz)
{
    return PLERROR;
}

int StoreManager::sync_update(const std::string& key, int replica_type,
                              const unsigned char* value, 
                              const size_t & sz)
{
    return PLERROR;
}

int StoreManager::sync_delete(const std::string& key, int replica_type)
{
    return PLERROR;
}

