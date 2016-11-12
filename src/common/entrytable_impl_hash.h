/**
 *******************************************************************************
 * entrytable.h                                                                *
 *                                                                             *
 * Member entry table                                                          *
 *   -- API to access member entries                                           *
 *******************************************************************************
 */

#ifndef _MEMBER_ENTRY_HASH_H_
#define _MEMBER_ENTRY_HASH_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <ctime>
#include <cstring>

#include "pltypes.h"
#include "entrytable.h"

/*
 *******************************************************************************
 *  Macros                                                                     *
 *******************************************************************************
 */

 
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


class entry_impl_hash : public entry_table {
public:
    entry_impl_hash(void* addr, std::size_t max_size, bool create=false);
    ~entry_impl_hash() {
    }

    void initialize(void* addr, std::size_t max_size, bool create=false);
    void insert(const struct MemberEntry & e);
    void erase(const struct MemberEntry & e);
    void update(const struct MemberEntry & e);
    int  get_node_heartbeat(struct MemberEntry & e);
    void bulk_add(const std::vector< struct MemberEntry > &);
    void bulk_update(const std::vector< struct MemberEntry > &, 
                             time_t now = std::time(NULL));
    std::vector<std::pair<bool, int64> > bulk_get(
                            const std::vector< struct MemberEntry > &);
    void clear();
    int32 compute_index(const struct MemberEntry &e) ;

    const struct MemberEntry& operator[](int i) const;

    static std::size_t get_required_size(std::size_t ring_size);
    std::size_t size() const {
        return m_tab->member_cnt;
    }
    //template<typename T> friend class entry_iterator;
protected:
private:
    std::size_t       m_sz_slot;
    std::size_t       m_sz_hash;
    struct st_entry * m_tab;
    int32           * m_hash;
    struct MemberEntry * m_mement;
};

#if 0
template<> class entry_iterator< entry_impl_hash > {
    enum PTR_VAL {
        NIL_PTR = -1,
    };
public:
    entry_iterator(const entry_impl_hash & tab) : 
        ent_tab(tab),
        hash_idx(-1),
        slot_idx(-1) 
    {
        find_next_slot();
    }
    entry_iterator(const entry_impl_hash & tab, bool end) :
        ent_tab(tab),
        hash_idx(tab.m_sz_hash),
        slot_idx(-1) 
    {
    }

    /**
     * begin():
     *   (hash_idx, slot_idx) points to first element
     * end():
     *   hash_idx = ent_tab.m_sz_hash
     *   slot_idx = NIL_PTR
     */
    const struct MemberEntry operator*() const 
    {
    }

    entry_iterator<entry_impl_hash>& operator++() 
    {
    }

    entry_iterator<entry_impl_hash> operator++(int) 
    {
    }

    bool operator!=(const entry_iterator<entry_impl_hash>& other) const
    {
    }
    bool operator==(const entry_iterator<entry_impl_hash>& other) const
    {
    }

protected:
    int32 find_next_slot() 
    {
    }
private:
    const entry_impl_hash ent_tab;
    int32 hash_idx; // range [0, hash_size)
    int32 slot_idx; // range [NIL_PTR, elem_size)
};

#endif

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */
 
#endif // _MEMBER_ENTRY_HASH_H_

