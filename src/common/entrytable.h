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

#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>

#include <sys/types.h>
#include <sys/socket.h>

#include "stdinclude.h"
#include "pltypes.h"
#include "pladdress.h"
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
    int64   hashcode;  // computed by entry_table
    int64   heartbeat; // -1 indicates start over
    uint64  tm_lasthb; // time in seconds
    int32   af;
    int32   type;
    uint8   address[PL_IPv6_ADDR_LEN]; // IPv6: 16 bytes, IPv4: 4 bytes
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
    virtual void update(const struct MemberEntry & e) = 0;
    virtual int  get_node_heartbeat(struct MemberEntry & e) = 0;
    virtual void bulk_add(const std::vector< struct MemberEntry > &) = 0;
    // Add not found data into memeber list
    virtual void bulk_update(const std::vector< struct MemberEntry > &, 
                             time_t now = std::time(NULL)) = 0;
    virtual std::vector<std::pair<bool, int64> > bulk_get(
                                           const std::vector< struct MemberEntry > &) = 0;

    virtual void clear() = 0;

    virtual const struct MemberEntry& operator[](int i) const = 0;
    virtual std::size_t size() const = 0;

    void dump_node(const struct MemberEntry & e) {
        getlog()->sendlog(LogLevel::DEBUG, "Member\n");
        getlog()->sendlog(LogLevel::DEBUG, "hash      = %lx\n", e.hashcode);
        getlog()->sendlog(LogLevel::DEBUG, "id        = %lx\n", e.id);
        getlog()->sendlog(LogLevel::DEBUG, "heartbeat = %lx\n", e.heartbeat);
        getlog()->sendlog(LogLevel::DEBUG, "tm_lasthb = %lx\n", e.tm_lasthb);
        getlog()->sendlog(LogLevel::DEBUG, "af        = %d\n",  e.af);
        getlog()->sendlog(LogLevel::DEBUG, "type      = %d\n",  e.type);
        getlog()->sendlog(LogLevel::DEBUG, "port      = %d\n",  e.portnumber);
        getlog()->sendlog(LogLevel::DEBUG, "addr      = ");
        unsigned int i;
        for (i=0; i< PL_IPv6_ADDR_LEN - 1; i++) {
            getlog()->sendlog(LogLevel::DEBUG, "%X.", e.address[i]);
        }
        getlog()->sendlog(LogLevel::DEBUG, "%X\n", e.address[i]);
    }
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
    int addrsize = e.af == AF_INET ? 4 : 16;
    boost::hash_range(seed, e.address, e.address+addrsize);

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

inline bool operator<(const struct MemberEntry& l, const struct MemberEntry& r)
{
    return entry_less(l, r);
}

inline bool operator==(const struct MemberEntry& l, const struct MemberEntry& r)
{
    return entry_equal(l, r);
}

inline bool operator!=(const struct MemberEntry& l, const struct MemberEntry& r)
{
    return !(l==r);
}

#endif // _MEMBER_ENTRY_TAB_H_

