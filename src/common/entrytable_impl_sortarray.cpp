/**
 *******************************************************************************
 * entrytable.cpp                                                              *
 *                                                                             *
 * member entry table                                                          *
 *******************************************************************************
 */

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */

#include <stdexcept>
#include <functional>
#include <algorithm>

#include <cstring>

#include "entrytable_impl_sortarray.h"

#define NIL_PTR -1

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
std::size_t entry_impl_sortarray::get_required_size(std::size_t ring_size) 
{
    std::size_t size = sizeof(struct MemberEntry) * ring_size;
    return size;
}

entry_impl_sortarray::entry_impl_sortarray(void* addr, std::size_t max_size, 
                                           bool create) :
   m_sz_slot(max_size),
   m_tab(nullptr),
   m_mement(nullptr)
{
    if (addr) {
        initialize(addr, max_size, create);
    }
}

void entry_impl_sortarray::initialize(void* addr, std::size_t max_size, bool create)
{
    if (addr==nullptr) {
        throw std::invalid_argument("nullptr");
    }
    if (max_size==0) {
        throw std::underflow_error("size zero");
    }

    m_sz_slot = max_size;

    m_tab    = (struct st_entry*)addr;
    m_mement = (struct MemberEntry*)(m_tab->ent_hash);

    if (create) {
        m_tab->member_cnt = 0;
    }

    /* for attach */
    if (m_tab->member_cnt != 0) {
    }
}

void entry_impl_sortarray::insert(const struct MemberEntry& e)
{
    if (m_tab->member_cnt >= m_sz_slot) {
        throw std::overflow_error("entry table full");
    }

    MemberEntry tmp = e;
    std::size_t hashkey = entry_hash(tmp);
    tmp.hashcode = hashkey;

    uint32 i;
    for (i=0; i<m_tab->member_cnt; i++) {
        if (entry_less(tmp, m_mement[i])) {
            break;
        } 
    }
    if (i != m_tab->member_cnt) {
        uint32 mvcnt = m_tab->member_cnt - i;
        memmove(m_mement+i+1, m_mement+i, mvcnt * sizeof(struct MemberEntry));
    }
    m_mement[i] = tmp;

    m_tab->member_cnt++;
}

void entry_impl_sortarray::erase(const struct MemberEntry& e)
{
    MemberEntry tmp = e;
    std::size_t hashkey = entry_hash(tmp);
    tmp.hashcode = hashkey;

    uint32 i;
    for (i=0; i<m_tab->member_cnt; i++) {
        if (entry_equal(tmp, m_mement[i])) {
            break;
        } 
    }

    if (i != m_tab->member_cnt) {
        if (i+1 < m_tab->member_cnt) {
            uint32 mvcnt = m_tab->member_cnt - i - 1;
            memmove(m_mement+i, m_mement+i+1, mvcnt * sizeof(struct MemberEntry));
        }
        m_tab->member_cnt--;
        memset(m_mement+m_tab->member_cnt, 0, sizeof(struct MemberEntry));
    }

}

void entry_impl_sortarray::update(const struct MemberEntry& e, uint64 hb, uint64 now)
{
    MemberEntry tmp = e;
    std::size_t hashkey = entry_hash(tmp);
    tmp.hashcode = hashkey;

    uint32 i;
    for (i=0; i<m_tab->member_cnt; i++) {
        if (entry_equal(tmp, m_mement[i])) {
            break;
        } 
    }
    if (i != m_tab->member_cnt) {
        m_mement[i].heartbeat = hb;
        m_mement[i].tm_lasthb = now;
    }
}

const struct MemberEntry& entry_impl_sortarray::operator[](int i) const
{
    if (i<0 || i>m_tab->member_cnt) {
        throw std::out_of_range("operator[]: " + std::to_string(i));
    }
    return m_mement[i];
}


