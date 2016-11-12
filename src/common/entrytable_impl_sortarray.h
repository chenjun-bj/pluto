/**
 *******************************************************************************
 * entrytable.h                                                                *
 *                                                                             *
 * Member entry table                                                          *
 *   -- API to access member entries                                           *
 *******************************************************************************
 */

#ifndef _MEMBER_ENTRY_SORTED_ARRAY_H_
#define _MEMBER_ENTRY_SORTED_ARRAY_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <ctime>
#include <cstring>

#include <queue>
#include <tuple>

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


class entry_impl_sortarray : public entry_table {
public:
    entry_impl_sortarray(void* addr, std::size_t max_size, bool create=false);
    ~entry_impl_sortarray() {
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

    const struct MemberEntry& operator[](int i) const;

    static std::size_t get_required_size(std::size_t ring_size);
    std::size_t size() const {
        return m_tab->member_cnt;
    }
protected:
    //typedef std::tuple<uint64, uint32> HeapElem;
    //typedef std::priority_queue<HeapElem, std::vector<HeapElem>, 
    //                   bool (*)(const HeapElem&, const HeapElem&) > EntHeap;
private:
    std::size_t       m_sz_slot;
    struct st_entry * m_tab;
    struct MemberEntry * m_mement;
    //EntHeap           m_heap;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */
 
#endif // _MEMBER_ENTRY_SORTED_ARRAY_H_

