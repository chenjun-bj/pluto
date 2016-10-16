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

#include "memberlist.h"

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
MemberList::MemberList(void* addr) : 
   m_paddr(reinterpret_cast<Membership*>(addr))
{
}

MemberList::~MemberList()
{
}

bool MemberList::attach(void* addr)
{
    if (addr) {
        m_paddr = reinterpret_cast<struct Membership*>(addr);
        if (m_paddr->magic_number != PL_SHM_MAGIC) {
            return false;
        }
        return true;
    }
    return false;
}


void MemberList::detach()
{
    m_paddr = nullptr;
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

bool  MemberList::set_running_status(uint32 status)
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
uint32 MemberList::get_running_status() const
{
    if (m_paddr) {
        return m_paddr->running_status;
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
    return -1;
}

