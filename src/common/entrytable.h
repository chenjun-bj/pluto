/**
 *******************************************************************************
 * entrytable.h                                                                *
 *                                                                             *
 * Member entry table                                                          *
 *   -- API to access member entries                                           *
 *******************************************************************************
 */

#ifndef _MEMBER_ENTRY_TAB_H_
#define _MEMBER_ENTRY_TAB_H_

/*
 *******************************************************************************
 *  Headers                                                                    *
 *******************************************************************************
 */
#include <ctime>
#include <cstring>

#include <boost/functional/hash.hpp>

#include <sys/types.h>
#include <sys/socket.h>

#include "pltypes.h"
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

struct st_entry {
    uint64 member_cnt;
    int32  ent_free_start;
    int32  ent_free_end;
    int32  ent_hash[1]; // size is ring_size
};

struct MemberEntry {
    uint64  id;        // assign by entry_table
    uint64  hashcode;  // computed by entry_table
    uint64  heartbeat;
    uint64  tm_lasthb; // time in seconds
    uint32  af;
    uint32  type;
    uint8   address[16];
    uint16  portnumber;
    uint16  reserved;
    int32   next;
};

class entry_table {
public:
    virtual ~entry_table() {
    }
   
    // TODO: provide attach function to attach memory already initialized
    virtual void initialize(void* addr, std::size_t max_size, bool create = false) = 0;
    virtual void insert(const struct MemberEntry & e) = 0;
    virtual void erase(const struct MemberEntry & e) = 0;
    virtual void update(const struct MemberEntry & e, uint64 hb, 
                        uint64 now = std::time(NULL)) = 0;

    virtual const struct MemberEntry& operator[](int i) const = 0;
    virtual std::size_t size() const = 0;
protected:
private:
};

class entry_iterator {
public:
    entry_iterator(const entry_table & tab) : tab_(tab), idx_(0) {
    }
    entry_iterator(const entry_table & tab, bool end) : tab_(tab), idx_(tab.size()) {
    }

    const struct MemberEntry operator*() const {
        return tab_[idx_];
    
    }
    entry_iterator& operator++() {
        idx_++;
        return *this;
    }
    entry_iterator operator++(int) { 
        entry_iterator cur = *this;
        idx_++;
        return cur;
    }

    bool operator!=(const entry_iterator& other) const {
        return !(*this == other);
    }
    bool operator==(const entry_iterator& other) const  {
        if (&tab_ != &other.tab_) {
            return false;
        }
        if (idx_ != other.idx_) {
            return false;
        }
        return true;
    }

private:
    const entry_table& tab_;
    int                idx_;
};

/*
 *******************************************************************************
 *  Inline functions                                                           *
 *******************************************************************************
 */
 
inline std::size_t entry_hash(const struct MemberEntry& e)
{
    std::size_t seed = 0;
    boost::hash_combine(seed, e.af);
    boost::hash_combine(seed, e.portnumber);
    boost::hash_range(seed, e.address, e.address+16);

    return seed; 
}

inline bool entry_less(const struct MemberEntry& l, const struct MemberEntry& r)
{
    // TOOD: optimize if l,r already compute hash code
    std::size_t hl = entry_hash(l);
    std::size_t hr = entry_hash(r);
    if (hl < hr) {
        return true;
    }
    else if (hl > hr) {
        return false;
    }
    else {
        if (l.af < r.af) {
            return true;
        }
        else if (l.af > r.af) {
            return false;
        }

        int addrsize = l.af == AF_INET ? 4 : 16;
        int addcmp = std::memcmp(l.address, r.address, addrsize);
        if (addcmp<0) {
            return true;
        }
        else if (addcmp>0) {
            return false;
        }

        return l.portnumber < r.portnumber;
    }
}

inline bool entry_equal(const struct MemberEntry& l, const struct MemberEntry& r)
{
    return (!entry_less(l, r) && !entry_less(r, l));
}

#endif // _MEMBER_ENTRY_TAB_H_

