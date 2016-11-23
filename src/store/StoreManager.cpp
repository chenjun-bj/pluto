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
   m_pconfig(pcfg)
{
}

StoreManager::~StoreManager()
{
}

vector<struct MemberEntry > StoreManager :: get_nodes(const string& key)
{
    // TODO: MemberList is updated by Membership protocol (another process)
    //       so the access should be protected by a lock

    vector<struct MemberEntry > v;

    std::hash<string> hashfunc;
    size_t hash_code = hashfunc(key);
   
    size_t pos = hash_code % m_pconfig->get_ringsize();

    // !!! TODO: LOCK !!!   
    size_t node_cnt = m_pmember->size();
    if (node_cnt >= PLUTO_NODE_REPLICAS_NUM) {
        int i = 0;
        if (pos > (*m_pmember)[node_cnt-1].hashcode % m_pconfig->get_ringsize()) {
            i = 0;
        } else {
            for (i=0; i<node_cnt; i++) {
                const MemberEntry & e = (*m_pmember)[i];
                if (pos <= e.hashcode % m_pconfig->get_ringsize()) {
                    // push back
                    break;
                }
            }
        }
        for (; v.size()<PLUTO_NODE_REPLICAS_NUM; i++) {
            v.push_back((*m_pmember)[i%m_pconfig->get_ringsize()]);
        }
    }
  
    return v;
}

int StoreManager::store_read(const std::string& key, int replica_type,
                             unsigned char* value, size_t & sz)
{
    return 0;
}

int StoreManager::store_create(const std::string& key, int replica_type,
                               const unsigned char* value, 
                               const size_t & sz)
{
    return 0;
}

int StoreManager::store_update(const std::string& key, int replica_type,
                               const unsigned char* value, 
                               const size_t & sz)
{
    return 0;
}

int StoreManager::store_delete(const std::string& key, int replica_type)
{
    return 0;
}

