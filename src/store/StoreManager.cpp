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
   m_has_my_replicas(),
   m_has_replicas_of(),
   m_ring_strand(io),
   m_clnt_strand(io)
{
    string self_ip(m_pconfig->get_bindip());                                   
 
    m_self_port = m_pconfig->get_bindport();                                             
    ip::address self_addr = ip::address::from_string(self_ip); 

    memset(m_self_rawip, '\0', PL_IPv6_ADDR_LEN); 
    if (self_addr.is_v6()) {                                                           
        ip::address_v6::bytes_type rawaddr = self_addr.to_v6().to_bytes();             
        m_self_af = AF_INET6;
        memcpy(m_self_rawip, rawaddr.data(), PL_IPv6_ADDR_LEN);                          
    }                                                                                    
    else {                                                                               
        ip::address_v4::bytes_type rawaddr = self_addr.to_v4().to_bytes();             
        m_self_af = AF_INET;              
        memcpy(m_self_rawip, rawaddr.data(), PL_IPv6_ADDR_LEN);                          
    }
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
    //std::copy(m_pmember->begin(), m_pmember->end(), std::back_inserter( cur_memlist));
    for (auto&& m : *m_pmember) {
        cur_memlist.push_back(m);
    }
    // !!! UNLOCK here !!!

    m_ring_strand.post([this, cur_memlist]() {
        bool change = false;
        // TODO: lock ring !!!
        if (cur_memlist.size() != m_ring.size()) {
            change = true;
        }
        else {
            for (size_t i=0; i<cur_memlist.size(); i++) {
                if (cur_memlist[i] != m_ring[i]) {
                    change=true;
                    break;
                }
            }
        }

        if (change) {
            m_ring.clear();
            std::copy(cur_memlist.begin(), cur_memlist.end(), std::back_inserter(m_ring));
            //m_ring = cur_memlist;
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
 
    long long pos = hash_code % m_pconfig->get_ringsize();

    // !!! TODO: LOCK !!!   
    size_t node_cnt = m_ring.size();
    if (node_cnt >= PLUTO_NODE_REPLICAS_NUM) {
        size_t i = 0;
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
    // stabilization protocol is called by ring stand,
    // no need to 'lock'
    size_t i; 
    bool change = false;
    vector<MemberEntry > l;
    vector<MemberEntry > r;

    find_neighbors(l, r);

    for (i=0; i<m_has_replicas_of.size(); i++) {
        if (i>=PLUTO_NODE_REPLICAS_NUM) break;
        if (!is_node_in_ring(m_has_replicas_of[i])) {
            // Node leave 
            handle_neighbor_leave(m_has_replicas_of[i], PLUTO_NODE_REPLICAS_NUM-i);
        }
    }

    change = false;
    for (i=0; i<PLUTO_NODE_REPLICAS_NUM; i++) {
        if (i>=m_has_replicas_of.size()) { 
            change = true;
            break;
        }
        if (m_has_replicas_of[i] != l[i]) {
            // left neighbor changes 
            change = true;
            remove_left_neighbor_replicas(PLUTO_NODE_REPLICAS_NUM-i);
        }
    }

    if (change) {
        m_has_replicas_of.swap(l);
    }
 
    change = false;
    for (i=0; i<PLUTO_NODE_REPLICAS_NUM; i++) {
        if (i>=m_has_my_replicas.size()) { 
            change = true;
            break;
        }
        if (m_has_my_replicas[i] != r[i]) {
            // right neighbor changes
            change = true;
            send_right_neighbor_replicas(r[i], 0); // 0 is PRIMARY
        }
    } 

    if (change) {
        m_has_my_replicas.swap(r);
    }
}

void StoreManager::find_neighbors(std::vector<MemberEntry >& left,
                                  std::vector<MemberEntry >& right)
{
    if (m_ring.size() < PLUTO_NODE_REPLICAS_NUM) {
        getlog()->sendlog(LogLevel::INFO, "Find neighbors, not enough nodes\n");
        return;
    }

    struct MemberEntry self;
                                                                                                  
    memset(&self, 0, sizeof(self));                                                               
    self.af = m_self_af;
    if (self.af == AF_INET) {
        memcpy(self.address, m_self_rawip, PL_IPv4_ADDR_LEN);
    }
    else {
        memcpy(self.address, m_self_rawip, PL_IPv6_ADDR_LEN);
    }
    self.type       = SOCK_STREAM;
    self.portnumber = m_self_port;

    int ring_sz  = m_ring.size();
    int i=0;
    int self_idx = -1;

    for (;i<ring_sz; i++) {
        if (self == m_ring[i]) {
            self_idx = i;
            break;
        }
    }

    if (self_idx<0) {
        getlog()->sendlog(LogLevel::FATAL, "Find neighbors, self node is not in member\n");
        return;
    }

    for (i=1; i<PLUTO_NODE_REPLICAS_NUM; i++) {
        int idx = (self_idx + i) % ring_sz;
        right.push_back((*m_pmember)[idx]);
    }

    for (i=1; i<PLUTO_NODE_REPLICAS_NUM; i++) {
        int idx = (self_idx - PLUTO_NODE_REPLICAS_NUM + i + ring_sz) % ring_sz;
        left.push_back((*m_pmember)[idx]);
    }
}

bool StoreManager::is_node_in_ring(const MemberEntry& node)
{
    for (auto& n : m_ring) {
        if (n==node) {
            return true;
        }
    }
    return false;
}

void StoreManager::handle_neighbor_leave(const MemberEntry& node, int replica_type)
{
    // left neighbor remove, send data owned by that node to new members
    // multiple nodes may detect, thus the data maybe added multiple times
    m_store_acc.async_get(replica_type, true,
                          [this](int rc, map<string, vector<unsigned char> >& v) {
            // I hate use asynchronous way this much
            for (auto& it : v) {
                vector<MemberEntry > rep = get_nodes(it.first);
                // send data to target replicas 
            }
        });
}

void StoreManager::remove_left_neighbor_replicas(int replica_type)
{
    m_store_acc.async_delete(replica_type, [](int rc) {
        (void)rc;
    });
}

void StoreManager::send_right_neighbor_replicas(const MemberEntry& node, int replica_type)
{
    m_store_acc.async_get(0, true,
                          [=](int rc, map<string, vector<unsigned char> >& v) {
            // I hate use asynchronous way this much
            for (auto it : v) {
                // send data to node as replica_type 
            }
        });
}

