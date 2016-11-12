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
#include <string>

#include <cstring>

#include "entrytable_impl_hash.h"

#define NIL_PTR     -1

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
std::size_t entry_impl_hash::get_required_size(std::size_t ring_size)
{
    size_t size = sizeof(int32) * ring_size * 2 +  // Hash array, 2 times of ring size
                  sizeof(struct MemberEntry) * ring_size; // Entry array

    return size;
}

entry_impl_hash::entry_impl_hash(void* addr, std::size_t max_size, bool create) :
   m_sz_slot(0),
   m_sz_hash(0),
   m_tab(nullptr),
   m_hash(nullptr),
   m_mement(nullptr)
{
    if (addr) {
        initialize(addr, max_size, create);
    }
}

void entry_impl_hash::initialize(void* addr, std::size_t max_size, bool create)
{
    if (addr==nullptr) {
        throw std::invalid_argument("nullptr");
    }
    if (max_size==0) {
        throw std::underflow_error("size zero");
    }

    m_sz_slot = max_size;
    m_sz_hash = max_size * 2;

    m_tab    = (struct st_entry*)addr;
    m_hash   = m_tab->ent_hash;
    m_mement = (struct MemberEntry*)(m_hash + m_sz_hash);

    if (create) {
        clear();
    }
}

void entry_impl_hash::clear()
{
    uint32 i;
    for (i=0; i<m_sz_slot; i++) {
        m_mement[i].next = i+1;
    }
    m_mement[i-1].next = NIL_PTR;

    for (i=0; i<m_sz_hash; i++) {
        m_hash[i] = NIL_PTR;
    }

    m_tab->member_cnt     = 0;
    m_tab->ent_free_start = 0;
}

void entry_impl_hash::insert(const struct MemberEntry& e)
{
    if (m_tab->member_cnt >= m_sz_slot) {
        throw std::overflow_error("entry table full");
    }

    int32 free_slot = m_tab->ent_free_start;
    if (free_slot == NIL_PTR) {
        throw std::overflow_error("entry table full");
    }

    m_tab->ent_free_start = m_mement[free_slot].next;
    memcpy(&(m_mement[free_slot]), &e, sizeof(struct MemberEntry));

    int32 hk = compute_index(e);

    int32 idx = m_hash[hk];
    int32 i, pre;
    for (i = idx, pre = NIL_PTR; 
         i != NIL_PTR; 
         pre = i, i = m_mement[i].next) { 
        if (!entry_less(e, m_mement[i])) {
            break;
        };
    }

    if (pre == NIL_PTR) {
        m_hash[hk] = free_slot;
    }
    else {
        m_mement[pre].next = free_slot;
    }

    m_mement[free_slot].next     = i;
    //m_mement[free_slot].hashcode = hk;

    m_tab->member_cnt++;
}

void entry_impl_hash::erase(const struct MemberEntry& e)
{
    int32 hk = compute_index(e);
    int32 idx, pre;
   
    for (idx=m_hash[hk], pre = NIL_PTR; 
         idx != NIL_PTR; 
         pre = idx, idx = m_mement[idx].next) {
        if (entry_equal(m_mement[idx], e)) {
            break;
        } 
    }

    if (idx == NIL_PTR) {
        // Not treat this as error
        return;
    }

    if (pre == NIL_PTR) {
        m_hash[hk] = m_mement[idx].next;
    }
    else {
        m_mement[pre].next = m_mement[idx].next;
    }

    m_mement[idx].next = m_tab->ent_free_start;
    m_tab->ent_free_start = idx;

    m_tab->member_cnt--;
}

void entry_impl_hash::update(const struct MemberEntry& e)
{
    int32 hk = compute_index(e);
    int32 idx;

    for (idx = m_hash[hk]; idx != NIL_PTR; idx = m_mement[idx].next) {
        if (entry_equal(m_mement[idx], e)) {
            break;
        } 
    }

    if (idx == NIL_PTR) {
        return;
    }

    m_mement[idx].heartbeat = e.heartbeat;
    m_mement[idx].tm_lasthb = e.tm_lasthb;
}

int entry_impl_hash::get_node_heartbeat(struct MemberEntry & e)
{
    int32 hk = compute_index(e);
    int32 idx;

    for (idx = m_hash[hk]; idx != NIL_PTR; idx = m_mement[idx].next) {
        if (entry_equal(m_mement[idx], e)) {
            break;
        } 
    }

    if (idx == NIL_PTR) {
        return -1;
    }

    e.heartbeat = m_mement[idx].heartbeat;

    return 0;
}

void entry_impl_hash::bulk_add(const std::vector< struct MemberEntry > & nodes)
{
    for (auto&& e : nodes) {
        insert(e);
    }
}

void entry_impl_hash::bulk_update(const std::vector< struct MemberEntry > & nodes, 
                                  time_t now)
{
    for (auto&& e : nodes) {
        struct MemberEntry tmp=e;
        tmp.tm_lasthb = now;
        if (get_node_heartbeat(tmp) == -1) {
            // nop
            //insert(tmp);
        } else {
            update(tmp);
        }
    }
}

std::vector<std::pair<bool, int64> > entry_impl_hash::bulk_get(const std::vector< struct MemberEntry > & nodes)
{
    std::vector<std::pair<bool, int64> > rc;

    for (auto& e : nodes) {
       struct MemberEntry tmp = e;
       if (get_node_heartbeat(tmp) == -1) {
           rc.push_back(std::make_pair(false, -1));
       }
       else {
           rc.push_back(std::make_pair(true, tmp.heartbeat));
       }
    }

    return rc;
}

const struct MemberEntry& entry_impl_hash::operator[](int i) const
{
    if (i<0 || (size_t)i>m_tab->member_cnt) {
        throw std::out_of_range("opertor[] " + std::to_string(i));
    }
    int cnt=0;
    for (unsigned int hash_idx=0; hash_idx < m_sz_hash; hash_idx++) {
        if (m_hash[hash_idx] != NIL_PTR) {
            for (int slot_idx=m_hash[hash_idx]; slot_idx != NIL_PTR;
                     slot_idx = m_mement[slot_idx].next) {
                //
                if (cnt == i) {
                    return m_mement[slot_idx];
                }
                cnt++;
            }
        }
    }

    throw std::runtime_error("internal error");
}

int32 entry_impl_hash::compute_index(const struct MemberEntry& e)
{
    if (m_sz_hash==0) {
        throw std::underflow_error("size zero");
    }
    return entry_hash(e) % m_sz_hash;
}

#if 0
entry_iterator::entry_iterator(const entry_impl_hash & tab) :
    ent_tab(tab),
    hash_idx(-1),
    slot_idx(NIL_PTR)
{
    find_next_slot();
}

entry_iterator::entry_iterator(const entry_impl_hash & tab, bool end) :
    ent_tab(tab),
    hash_idx(ent_tab.m_sz_hash),
    slot_idx(NIL_PTR)
{
}

const struct MemberEntry entry_iterator::operator*() const
{
    if (slot_idx == NIL_PTR) {
        throw std::range_error("slot null");
    }
    return ent_tab.m_mement[slot_idx];
}

int32 entry_iterator::find_next_slot()
{
    if (slot_idx != NIL_PTR) {
        slot_idx = ent_tab.m_mement[slot_idx].next;
    }
    if (slot_idx == NIL_PTR) {
        for (hash_idx++ ;hash_idx < ent_tab.m_sz_hash ; hash_idx++) {
            if (ent_tab.m_hash[hash_idx] != NIL_PTR) {
                slot_idx = ent_tab.m_hash[hash_idx];
                break;
            }
        }
    }
    return slot_idx;
}

entry_iterator& entry_iterator::operator++() 
{
    find_next_slot();
    return *this;
}

entry_iterator entry_iterator::operator++(int) 
{
    entry_iterator cur = *this;
    find_next_slot();
    return cur;
}

bool entry_iterator::operator!=(const entry_iterator& other) const
{
    return !(*this == other);
}

bool entry_iterator::operator==(const entry_iterator& other) const
{
    return ((ent_tab.m_mement == other.ent_tab.m_mement) && 
            (hash_idx == other.hash_idx) &&
            (slot_idx == other.slot_idx));
}

#endif

