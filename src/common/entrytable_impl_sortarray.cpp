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

#include "stdinclude.h"

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

void entry_impl_sortarray::clear()
{
    m_tab->member_cnt = 0;
    memset(m_mement, 0, sizeof(struct MemberEntry) * m_sz_slot);
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

void entry_impl_sortarray::update(const struct MemberEntry& e)
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
        m_mement[i].heartbeat = tmp.heartbeat;
        m_mement[i].tm_lasthb = tmp.tm_lasthb;
    }
    else {
        getlog()->sendlog(LogLevel::ERROR, "ERROR, update node not found\n");
        // TODO: remove following debug code
        #if 0
        getlog()->sendlog(LogLevel::ERROR, "update node:\n");
        dump_node(tmp);
        getlog()->sendlog(LogLevel::ERROR, "list nodes:\n");
        for (uint32 i=0; i<m_tab->member_cnt; i++) {
            dump_node(m_mement[i]);
        }
        #endif
    }
}

int entry_impl_sortarray::get_node_heartbeat(struct MemberEntry & e)
{
    std::size_t hashkey = entry_hash(e);
    e.hashcode = hashkey;

    uint32 i;
    for (i=0; i<m_tab->member_cnt; i++) {
        if (entry_equal(e, m_mement[i])) {
            break;
        } 
    }
    if (i != m_tab->member_cnt) {
        e.heartbeat = m_mement[i].heartbeat;
        return 0;
    }

    return -1;
}
 
void entry_impl_sortarray::bulk_add(const std::vector< struct MemberEntry > & nodes)
{
    if (nodes.size() == 0) return;

    std::vector< struct MemberEntry > v = nodes;
    std::sort(v.begin(), v.end(), entry_less);

    std::vector< struct MemberEntry > r;

    uint32 i, j;
    for (i=j=0; (i<v.size()) && (j<m_tab->member_cnt); ) {
        if (entry_less(m_mement[j], v[i])) {
            r.push_back(m_mement[j]);
            j++;
        }
        else if (entry_less(v[i], m_mement[j])) {
            r.push_back(v[i]);
            i++;
        }
        else {
            // if equal, use new added value
            r.push_back(v[i]);
            i++;
            j++;
        }
    }

    while (i<v.size()) {
       r.push_back(v[i]);
       i++;
    }

    while (j<m_tab->member_cnt) {
       r.push_back(m_mement[j]);
       j++;
    }

    memcpy(m_mement, r.data(), r.size() * sizeof(struct MemberEntry));
    m_tab->member_cnt = r.size();
}

void entry_impl_sortarray::bulk_update(const std::vector< struct MemberEntry > & nodes,
                                       time_t now )
{
    if (nodes.size() == 0) return;

    std::vector< struct MemberEntry > v = nodes;
    std::sort(v.begin(), v.end(), entry_less);

    std::vector< struct MemberEntry > r;

    uint32 i, j;
    for (i=j=0; (i<v.size()) && (j<m_tab->member_cnt); ) {
        if (entry_less(m_mement[j], v[i])) {
            j++;
        }
        else if (entry_less(v[i], m_mement[j])) {
            // not exist in list, insert
            v[i].tm_lasthb = now;
            r.push_back(v[i]);
            i++;
        }
        else {
            // equal
            v[i].tm_lasthb = now;
            m_mement[j] = v[i];
            i++;
            j++;
        }
    }

    while (i<v.size()) {
        r.push_back(v[i]);
    }

    // TODO: remove following debug code
    #if 0
    if (r.size()>0) {
        getlog()->sendlog(LogLevel::ERROR, "update node failed:\n");
        for (auto && e : r) {
            dump_node(e);
        }
        getlog()->sendlog(LogLevel::ERROR, "list nodes:\n");
        for (uint32 i=0; i<m_tab->member_cnt; i++) {
            dump_node(m_mement[i]);
        }
    }
    #endif
    /*
    if (r.size()>0) {
        bulk_add(r);
    }*/
}

std::vector<std::pair<bool, int64> > entry_impl_sortarray::bulk_get(
                                         const std::vector< struct MemberEntry > & g)
{
    std::vector<std::pair<bool, int64> > rc;
    if (g.size() == 0) return rc;

    std::vector<struct MemberEntry > v(g);
    std::sort(v.begin(), v.end(), entry_less);

    uint32 i, j;
    for (i=j=0; (i<v.size()) && (j<m_tab->member_cnt); ) {
        if (entry_less(m_mement[j], v[i])) {
            j++;
        }
        else if (entry_less(v[i], m_mement[j])) {
            rc.push_back(std::make_pair(false, -1));
            i++;
        }
        else {
            // equal
            rc.push_back(std::make_pair(true, m_mement[j].heartbeat));
            i++;
            j++;
        }
    }
    while (i<v.size()) {
        rc.push_back(std::make_pair(false, -1));
    }

    return rc;
}

const struct MemberEntry& entry_impl_sortarray::operator[](int i) const
{
    if (i<0 || (size_t)i>m_tab->member_cnt) {
        throw std::out_of_range("operator[]: " + std::to_string(i));
    }
    return m_mement[i];
}


