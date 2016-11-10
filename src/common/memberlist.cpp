/**
 *******************************************************************************
 * memberlist.cpp                                                              *
 *                                                                             *
 * member list                                                                 *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */

#include <cstddef>
#include <stdexcept>

#include "memberlist.h"
#include "pladdress.h"

#include "entrytable_impl_hash.h"
#include "entrytable_impl_sortarray.h"

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

size_t MemberList::get_required_size(size_t ring_size)
{
    size_t size = sizeof(struct Membership) -  
                  sizeof(int32)  + // Membership contains one int32 entry
                  entry_impl_sortarray::get_required_size(ring_size);

    return size;
}

MemberList::MemberList() : 
   m_paddr(nullptr),
   m_ptab(nullptr)
{
}

MemberList::~MemberList()
{
    if (m_ptab) {
        delete m_ptab;
    }
}

bool MemberList::create(void* addr, key_t ipckey, size_t ring_size)
{
    if (addr) {
        m_paddr = reinterpret_cast<struct Membership*>(addr);
        if (m_paddr->magic_number != PL_SHM_MAGIC) {
            return false;
        }
        m_paddr->ring_size = ring_size;
        m_paddr->ipckey    = ipckey;
        return init_entry_table(true);
    }
    return false;
}

bool MemberList::attach(void* addr)
{
    if (addr) {
        m_paddr = reinterpret_cast<struct Membership*>(addr);
        if (m_paddr->magic_number != PL_SHM_MAGIC) {
            return false;
        }
        return init_entry_table();
    }
    return false;
}

void MemberList::detach()
{
    m_paddr = nullptr;
    if (m_ptab) {
        delete m_ptab;
        m_ptab = nullptr; 
    }
}

void MemberList::clear()
{
    if (m_ptab) {
        m_ptab->clear();
    }
}

bool MemberList::init_entry_table(bool create)
{
    if (m_paddr == nullptr) {
        return false;
    }

    void * entry_ptr = (void*)m_paddr + offsetof(Membership, ent_tab);
    m_ptab = new entry_impl_sortarray(entry_ptr, m_paddr->ring_size, create);

    return true;
}

entry_iterator MemberList::begin()
{
    return entry_iterator(*m_ptab);
}

entry_iterator MemberList::end()
{
    return entry_iterator(*m_ptab, true);
}

void MemberList::add_node(int af, const uint8 * addr, unsigned short port,
                          int64 hb, time_t now)
{
    if (!valid_node_addr(af, SOCK_STREAM, addr, port)) {
        throw std::invalid_argument("Invalid node address");
    }

    if (m_ptab == nullptr) {
        throw std::logic_error("table not initialized");
    }

    struct MemberEntry node;

    memset(&node, 0, sizeof(node));
    node.af = af;
    if (af == AF_INET) {
        memcpy(node.address, addr, PL_IPv4_ADDR_LEN);
    }
    else {
        memcpy(node.address, addr, PL_IPv6_ADDR_LEN);
    }
    node.portnumber = port;
    node.heartbeat = hb;
    node.tm_lasthb = now;

    m_ptab->insert(node);
}

void MemberList::del_node(int af, const uint8 * addr, unsigned short port)
{
    if (!valid_node_addr(af, SOCK_STREAM, addr, port)) {
        throw std::invalid_argument("Invalid node address");
    }

    if (m_ptab == nullptr) {
        throw std::logic_error("table not initialized");
    }

    struct MemberEntry node;

    memset(&node, 0, sizeof(node));
    node.af = af;
    if (af == AF_INET) {
        memcpy(node.address, addr, PL_IPv4_ADDR_LEN);
    }
    else {
        memcpy(node.address, addr, PL_IPv6_ADDR_LEN);
    }
    node.portnumber = port;

    m_ptab->erase(node);
}

void MemberList::update_node_heartbeat(int af, const uint8 * addr, unsigned short port,
                                       int64 hb,
                                       time_t now)
{
    if (!valid_node_addr(af, SOCK_STREAM, addr, port)) {
        throw std::invalid_argument("Invalid node address");
    }

    if (m_ptab == nullptr) {
        throw std::logic_error("table not initialized");
    }

    struct MemberEntry node;

    memset(&node, 0, sizeof(node));
    node.af = af;
    if (af == AF_INET) {
        memcpy(node.address, addr, PL_IPv4_ADDR_LEN);
    }
    else {
        memcpy(node.address, addr, PL_IPv6_ADDR_LEN);
    }
    node.portnumber = port;
    node.heartbeat = hb;
    node.tm_lasthb = now;

    m_ptab->update(node);
}

int MemberList::get_node_heartbeat(int af, const uint8 * addr, unsigned short port,
                                   int64* hb)
{
    if (!valid_node_addr(af, SOCK_STREAM, addr, port)) {
        return -1;
    }

    if (m_ptab == nullptr) {
        return -1;
    }

    struct MemberEntry node;

    memset(&node, 0, sizeof(node));
    node.af = af;
    if (af == AF_INET) {
        memcpy(node.address, addr, PL_IPv4_ADDR_LEN);
    }
    else {
        memcpy(node.address, addr, PL_IPv6_ADDR_LEN);
    }
    node.portnumber = port;

    int rc = m_ptab->get_node_heartbeat(node);
    *hb = node.heartbeat;

    return rc;
}

void MemberList::bulk_add(const std::vector< struct MemberEntry > & nodes)
{
    m_ptab->bulk_add(nodes);
}

void MemberList::bulk_update(const std::vector< struct MemberEntry > &nodes, time_t now)
{
    m_ptab->bulk_update(nodes, now);
}

std::vector<bool> MemberList::bulk_get(std::vector< struct MemberEntry > & nodes)
{
    return std::move(m_ptab->bulk_get(nodes));
}

uint64 MemberList::get_magic_number() const
{
    if (m_paddr) {
        return m_paddr->magic_number;
    }
    return -1;
}

bool  MemberList::set_ipckey(key_t key)
{
    if (m_paddr) {
        m_paddr->ipckey = key;
        return true;
    }
    return false;
}

key_t  MemberList::get_ipckey() const
{
    if (m_paddr) {
        return m_paddr->ipckey;
    }
    return -1;
}

bool  MemberList::set_running_status(uint16 status)
{
    if ((status != PL_RUNNING_ST_RUN) && (status != PL_RUNNING_ST_STOP)) {
        return false;
    }

    if (m_paddr) {
        m_paddr->running_status = status;
        return true;
    }
    return false;
}

uint16 MemberList::get_running_status() const
{
    if (m_paddr) {
        return m_paddr->running_status;
    }
    return -1;
}

bool  MemberList::set_membership_status(uint8 status)
{
    if (valid_child_status(status) && m_paddr) {
        m_paddr->st_member = status;
        return true;
    }
    return false;
}
uint8 MemberList::get_membership_status() const
{
    if (m_paddr) {
        return m_paddr->st_member;
    }
    return -1;
}

bool  MemberList::set_store_status(uint8 status)
{
    if (valid_child_status(status) && m_paddr) {
        m_paddr->st_store = status;
        return true;
    }
    return false;
}
uint8 MemberList::get_store_status() const
{
    if (m_paddr) {
        return m_paddr->st_store;
    }
    return -1;
}

bool  MemberList::set_pid_member(pid_t pid)
{
    if (m_paddr) {
        m_paddr->pid_member = pid;
        return true;
    }
    return false;
}

pid_t  MemberList::get_pid_member() const
{
    if (m_paddr) {
        return m_paddr->pid_member;
    }
    return -1;
}

bool  MemberList::set_pid_store(pid_t pid)
{
    if (m_paddr) {
        m_paddr->pid_store = pid;
        return true;
    }
    return false;
}

pid_t  MemberList::get_pid_store()  const
{
    if (m_paddr) {
        return m_paddr->pid_store;
    }
    return -1;
}

pthread_mutex_t* MemberList::get_mutex() const
{
    if (m_paddr) {
        return &(m_paddr->mutex);
    }
    return nullptr;
}

bool  MemberList::set_ring_size(uint64 size)
{
    if (m_paddr) {
        m_paddr->ring_size = size;
        return true;
    }
    return false;
}

uint64 MemberList::get_ring_size() const
{
    if (m_paddr) {
        return m_paddr->ring_size;
    }
    return 0;
}

